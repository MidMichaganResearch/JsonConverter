#include "handlers/point_frames_handler.h"

#include <cmath>
#include <stdexcept>
#include <utility>

#include "core/data_row.h"
#include "core/json_writer.h"
#include "core/text_utils.h"
#include "core/value_extremes.h"
#include "plotly/point_figure.h"

namespace plotconv {

    namespace {
		// Each raw row holds x, y, z and the color value of one point.
        constexpr size_t kColumns = 4;

		// Frames read from the raw file, already written for the plot file.
        struct PointFrameSet
        {
            PointValues firstFrame;
            std::string framesJson;
            size_t frameCount = 0;
            ValueExtremes x, y, z, color;
            std::vector<std::string> warnings;

			// Sanity checks of the coordinates, reported as warnings once every frame is read.
            size_t pointsBeyondLimit = 0;
            std::string firstPointBeyondLimit;
            size_t framesBeyondLimit = 0;
            size_t firstFrameBeyondLimit = 0;
        };

        /// <summary>
        /// Reads consecutive frames of "points" rows until the end of the raw file.
        /// </summary>
        /// <param name="reader"> The line reader. </param>
        /// <param name="points"> The number of points in each frame. </param>
        /// <param name="type"> Type of the point frames handler, for the warning limits. </param>
        /// <param name="frames"> The frame set to populate. </param>
        std::string readPointFrames(LineReader& reader, size_t points, const PointFramesType& type, PointFrameSet& frames)
        {
			// Prepares the writer for the animation frames of the plot.
            JsonWriter framesWriter(frames.framesJson);
            framesWriter.beginArray();

            std::string line;
            DataRow row;

            while (true)
            {
				// Starts the four value arrays of the current frame.
                PointValues values;
                JsonWriter xWriter(values.x), yWriter(values.y), zWriter(values.z), colorWriter(values.color);
                xWriter.beginArray();
                yWriter.beginArray();
                zWriter.beginArray();
                colorWriter.beginArray();

				// Prepares the extremes and sanity checks of the current frame, merged only if the frame is complete.
                ValueExtremes xFrame, yFrame, zFrame, colorFrame;
                size_t pointsBeyondLimit = 0;
                std::string firstPointBeyondLimit;
                bool frameBeyondLimit = false;
                size_t pointsRead = 0;

				// Reads the raw rows of the current frame, one point per row.
                while (pointsRead < points && reader.next(line))
                {
					// Skips empty lines.
                    if (trim(line).empty())
                    {
                        continue;
                    }

					// Parses the raw values of the row, stopping the conversion if the row is not valid.
                    const std::string rowError = parseDataRow(line, kColumns, row);
                    if (!rowError.empty())
                    {
                        return "line " + std::to_string(reader.lineNumber()) + ": " + rowError;
                    }

					// Adds the point to the frame.
                    xWriter.writeNumberToken(row.tokens[0]);
                    yWriter.writeNumberToken(row.tokens[1]);
                    zWriter.writeNumberToken(row.tokens[2]);
                    colorWriter.writeNumberToken(row.tokens[3]);

                    xFrame.add(row.values[0], row.tokens[0]);
                    yFrame.add(row.values[1], row.tokens[1]);
                    zFrame.add(row.values[2], row.tokens[2]);
                    colorFrame.add(row.values[3], row.tokens[3]);

					// Checks the coordinates of the point against the warning limits.
                    const double largest = std::fmax(std::fabs(row.values[0]),
                                                     std::fmax(std::fabs(row.values[1]), std::fabs(row.values[2])));
                    if (largest > type.nodeWarningLimit)
                    {
                        if (pointsBeyondLimit == 0)
                        {
                            firstPointBeyondLimit = "frame " + std::to_string(frames.frameCount) + ", node " + std::to_string(pointsRead);
                        }
                        ++pointsBeyondLimit;
                    }
                    if (largest > type.frameWarningLimit)
                    {
                        frameBeyondLimit = true;
                    }

                    ++pointsRead;
                }

				// Verifies if no rows were read for the current frame, which means the raw file has ended.
                if (pointsRead == 0)
                {
                    break;
                }

				// Verifies if the current frame is incomplete.
                if (pointsRead < points)
                {
                    frames.warnings.push_back("the last frame is incomplete (" + std::to_string(pointsRead) + " of "
                        + std::to_string(points) + " points) and was ignored");
                    break;
                }

                xWriter.endArray();
                yWriter.endArray();
                zWriter.endArray();
                colorWriter.endArray();

				// Adds the current frame to the animation frames of the plot.
                writePointFrame(framesWriter, frames.frameCount, values);

				// Merges the extremes and sanity checks of the complete frame into the totals.
                frames.x.merge(xFrame);
                frames.y.merge(yFrame);
                frames.z.merge(zFrame);
                frames.color.merge(colorFrame);

                if (pointsBeyondLimit > 0 && frames.pointsBeyondLimit == 0)
                {
                    frames.firstPointBeyondLimit = firstPointBeyondLimit;
                }
                frames.pointsBeyondLimit += pointsBeyondLimit;

                if (frameBeyondLimit)
                {
                    if (frames.framesBeyondLimit == 0)
                    {
                        frames.firstFrameBeyondLimit = frames.frameCount;
                    }
                    ++frames.framesBeyondLimit;
                }

				// Stores the first frame, shown before the animation starts.
                if (frames.frameCount == 0)
                {
                    frames.firstFrame = std::move(values);
                }
                ++frames.frameCount;
            }

            framesWriter.endArray();
            return {};
        }

        /// <summary>
        /// Turns a number into text for a warning message.
        /// </summary>
        std::string limitText(double limit)
        {
            std::string text = std::to_string(limit);
            text.erase(text.find_last_not_of('0') + 1);
            if (!text.empty() && text.back() == '.')
            {
                text.pop_back();
            }
            return text;
        }
    }

    /// <summary>
	/// Handler for converting raw point frames files into animated 3D point plot files, based on the specified point frames type.
    /// </summary>
    /// <param name="type"> Type of the point frames handler. </param>
    PointFramesHandler::PointFramesHandler(PointFramesType type) : type_(std::move(type)) {}

	// Identifies the name of the handler, which is the expected file key for this handler.
    std::string_view PointFramesHandler::name() const
    {
        return type_.fileKey;
    }

	// Determines if the given file name matches the expected file key for this handler.
    bool PointFramesHandler::matches(std::string_view upperFileName) const
    {
        return upperFileName.find(type_.fileKey) != std::string_view::npos;
    }

    /// <summary>
	/// Converts a raw point frames file into an animated 3D point plot file, based on the specified point frames type.
    /// </summary>
    /// <param name="context"> The conversion context. </param>
    ConversionResult PointFramesHandler::convert(ConversionContext& context) const
    {
		// Extracts the header from the conversion context.
        const FileHeader& header = context.header;

		// Verifies if the header has the parameter line, which gives the number of points per frame.
        if (!header.parameters)
        {
            return ConversionResult::error("the header has no NODES / parameter line");
        }

        const size_t points = static_cast<size_t>(header.parameters->nodes);
        PointFrameSet frames;

		// Reads every frame of the raw file, stopping the conversion if a row is not valid.
        const std::string readError = readPointFrames(context.reader, points, type_, frames);
        if (!readError.empty())
        {
            return ConversionResult::error(readError);
        }

		// Verifies if at least one complete frame was found in the file.
        if (frames.frameCount == 0)
        {
            return ConversionResult::error("no complete frame found (expected " + std::to_string(points) + " rows of "
                + std::to_string(kColumns) + " values)");
        }

		// Reports the sanity checks of the coordinates as warnings; they do not change the plot.
        if (frames.pointsBeyondLimit > 0)
        {
            frames.warnings.push_back(std::to_string(frames.pointsBeyondLimit) + " points have a coordinate beyond +/-"
                + limitText(type_.nodeWarningLimit) + " (first at " + frames.firstPointBeyondLimit + ")");
        }
        if (frames.framesBeyondLimit > 0)
        {
            frames.warnings.push_back(std::to_string(frames.framesBeyondLimit) + " frames are suspicious, with a coordinate beyond +/-"
                + limitText(type_.frameWarningLimit) + " (first is frame " + std::to_string(frames.firstFrameBeyondLimit) + ")");
        }

        // Cycle files advance in crank angle; conformance files are just numbered frames.
        const bool isCycle = header.parameters->conformance.has_value() && !*header.parameters->conformance;

		// Sets the main values for the point figure.
        PointFigure figure;
        figure.title = header.title.empty() ? context.path.stem().string() : header.title;
        figure.xLabel = labelWithUnits(type_.xLabel, header.units);
        figure.yLabel = labelWithUnits(type_.yLabel, header.units);
        figure.zLabel = labelWithUnits(type_.zLabel, header.units);
        figure.colorLabel = std::string(type_.colorLabel);
        figure.colorUnits = std::string(type_.colorUnits);
        figure.xRange = paddedRange(frames.x.min(), frames.x.max());
        figure.yRange = paddedRange(frames.y.min(), frames.y.max());
        figure.zRange = paddedRange(frames.z.min(), frames.z.max());
        figure.colorMinToken = frames.color.minToken();
        figure.colorMaxToken = frames.color.maxToken();
        figure.firstFrame = std::move(frames.firstFrame);
        figure.framesJson = std::move(frames.framesJson);
        figure.frameCount = frames.frameCount;
        figure.sliderPrefix = isCycle ? "Crank Angle: " : "Frame: ";

		// Writes the plot file.
        std::string json;
        JsonWriter writer(json);
        writePointFigure(writer, figure);

		// Verifies that the plot file was completely written.
        if (!writer.isComplete())
        {
            throw std::logic_error("the generated JSON document is not complete");
        }

        return ConversionResult::success(std::move(json), std::move(frames.warnings));
    }
}
