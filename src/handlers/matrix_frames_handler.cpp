#include "handlers/matrix_frames_handler.h"

#include <stdexcept>
#include <utility>

#include "core/json_writer.h"
#include "core/data_row.h"
#include "core/text_utils.h"
#include "core/value_extremes.h"
#include "plotly/surface_figure.h"

namespace plotconv {

    namespace {

		// Data structure for the frames read from the raw file, already written for the plot file.
        struct FrameSet
        {
            std::string firstFrameZ;
            std::string framesJson;
            size_t frameCount = 0;
            ValueExtremes extremes;
            std::vector<std::string> warnings;
        };

        /// <summary>
        /// Describes the current line for error reporting.
        /// </summary>
        /// <param name="reader"> The line reader. </param>
        std::string describeLine(const LineReader& reader)
        {
            return "line " + std::to_string(reader.lineNumber()) + ": ";
        }

        /// <summary>
        /// Reads consecutive frames of "rows" by "columns" values until the end of the file.
        /// </summary>
        /// <remarks>
        /// Every value is validated and copied as text; it is also parsed to a double only to track the extremes.
        /// A trailing incomplete frame is dropped with a warning, and its values do not count towards the extremes.
        /// </remarks>
        /// <param name="reader"> The line reader. </param>
        /// <param name="rows"> The number of rows in each frame. </param>
        /// <param name="columns"> The number of columns in each frame. </param>
        /// <param name="frames"> The frame set to populate. </param>
        std::string readFrames(LineReader& reader, size_t rows, size_t columns, FrameSet& frames)
        {
			// Prepares the writer for the animation frames of the plot.
            JsonWriter framesWriter(frames.framesJson);
            framesWriter.beginArray();

			// Sets the variables to hold the current raw line, the matrix of the current frame, and its parsed values.
            std::string line;
            std::string matrixJson;
            DataRow row;

            while (true)
            {
				// Clears the previous frame's matrix and starts the matrix of the current frame.
                matrixJson.clear();
                JsonWriter matrix(matrixJson);
                matrix.beginArray();

				// Prepares an object to track the extremes of the values for color scaling in the current frame.
                ValueExtremes frameExtremes;
                size_t rowsRead = 0;

				// Iterates through each raw row in the current frame, and adds them to its matrix.
                while (rowsRead < rows && reader.next(line))
                {
					// Skips empty lines.
                    if (trim(line).empty())
                    {
                        continue;
                    }

					// Parses the raw values of the row, stopping the conversion if the row is not valid.
                    const std::string rowError = parseDataRow(line, columns, row);
                    if (!rowError.empty())
                    {
                        return describeLine(reader) + rowError;
                    }

					// Iterates through each column in the row, writing the value to the JSON representation and updating the extremes for color scaling.
                    matrix.beginArray();
                    for (size_t column = 0; column < columns; ++column)
                    {
                        matrix.writeNumberToken(row.tokens[column]);
                        frameExtremes.add(row.values[column], row.tokens[column]);
                    }
                    matrix.endArray();

                    // Counts the row as part of the current frame.
                    ++rowsRead;
                }

				// Verifies if no rows were read for the current frame.
                if (rowsRead == 0)
                {
                    break;
                }

				// Verifies if the current frame is incomplete.
                if (rowsRead < rows)
                {
                    frames.warnings.push_back(
                        "the last frame is incomplete (" + std::to_string(rowsRead) + " of " + std::to_string(rows)
                        + " rows) and was ignored");
                    break;
                }

                matrix.endArray();

				// Stores the first frame's Z values for the surface figure, if this is the first frame being processed.
                if (frames.frameCount == 0)
                {
                    frames.firstFrameZ = matrixJson;
                }

				// Adds the current frame to the animation frames of the plot.
                framesWriter.beginObject();
                framesWriter.key("name");
                framesWriter.writeString("f" + std::to_string(frames.frameCount));
                framesWriter.key("traces");
                framesWriter.beginArray();
                framesWriter.writeInteger(0);
                framesWriter.endArray();
                framesWriter.key("data");
                framesWriter.beginArray();
                framesWriter.beginObject();
                framesWriter.key("z");
                framesWriter.writeRaw(matrixJson);
                framesWriter.endObject();
                framesWriter.endArray();
                framesWriter.endObject();

				// Merges the extremes of the current frame into the overall extremes for color scaling, and increments the frame count.
                frames.extremes.merge(frameExtremes);
                ++frames.frameCount;
            }

            framesWriter.endArray();
            return {};
        }
    }

    /// <summary>
	/// Handler for converting raw matrix frames files into animated surface plot files, based on the specified matrix frames type.
    /// </summary>
    /// <param name="type"></param>
    MatrixFramesHandler::MatrixFramesHandler(MatrixFramesType type) : type_(std::move(type)) {}

    // Identifies the name of the handler, which is the expected file key for this handler.
    std::string_view MatrixFramesHandler::name() const
    {
        return type_.fileKey;
    }

    // Determines if the given file name matches the expected file key for this handler.
    bool MatrixFramesHandler::matches(std::string_view upperFileName) const
    {
        return upperFileName.find(type_.fileKey) != std::string_view::npos;
    }

    /// <summary>
	/// Converts a raw matrix frames file into an animated surface plot file, based on the specified matrix frames type.
    /// </summary>
    /// <param name="context"> The conversion context. </param>
    ConversionResult MatrixFramesHandler::convert(ConversionContext& context) const
    {
		// Extracts the header from the conversion context.
        const FileHeader& header = context.header;

		// Verifies if the header contains the necessary parameters for processing the matrix frames file.
        if (!header.parameters)
        {
            return ConversionResult::error("the header has no NODES / parameter line");
        }

		// Sets the number of nodes and other extent based on the header parameters and the matrix frames type.
        const size_t nodes = static_cast<size_t>(header.parameters->nodes);
        const size_t other = static_cast<size_t>(type_.otherExtent);
        const size_t columns = type_.nodesAreColumns ? nodes : other;
        const size_t rows = type_.nodesAreColumns ? other : nodes;

        FrameSet frames;

		// Reads every frame of the raw file, stopping the conversion if a row is not valid.
        const std::string readError = readFrames(context.reader, rows, columns, frames);
        if (!readError.empty())
        {
            return ConversionResult::error(readError);
        }

		// Verifies if at least one complete frame was found in the file.
        if (frames.frameCount == 0)
        {
            return ConversionResult::error("no complete frame found (expected " + std::to_string(rows)
                + " rows of " + std::to_string(columns) + " values)");
        }

        // Cycle files advance in crank angle; conformance files are just numbered frames.
        const bool isCycle = header.parameters->conformance.has_value() && !*header.parameters->conformance;

		// Sets the main values for the surface figure.
        SurfaceFigure surface;
        surface.title = header.title.empty() ? context.path.stem().string() : header.title;
        surface.xLabel = std::string(type_.xLabel);
        surface.yLabel = std::string(type_.yLabel);
        surface.zLabel = labelWithUnits(type_.zLabel, header.units);
        surface.xRange = paddedRange(0.0, static_cast<double>(columns) - 1.0);
        surface.yRange = paddedRange(0.0, static_cast<double>(rows) - 1.0);
        surface.zRange = paddedRange(frames.extremes.min(), frames.extremes.max());
        surface.colorMinToken = frames.extremes.minToken();
        surface.colorMaxToken = frames.extremes.maxToken();
        surface.firstFrameZ = std::move(frames.firstFrameZ);
        surface.framesJson = std::move(frames.framesJson);
        surface.frameCount = frames.frameCount;
        surface.sliderPrefix = isCycle ? "Crank Angle: " : "Frame: ";

		// Writes the plot file.
        std::string json;
        JsonWriter writer(json);
        writeSurfaceFigure(writer, surface);

		// Verifies that the plot file was completely written.
        if (!writer.isComplete())
        {
            throw std::logic_error("the generated JSON document is not complete");
        }

        return ConversionResult::success(std::move(json), std::move(frames.warnings));
    }
}