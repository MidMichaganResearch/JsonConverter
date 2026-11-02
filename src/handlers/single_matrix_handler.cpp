#include "handlers/single_matrix_handler.h"

#include <stdexcept>
#include <utility>

#include "core/data_row.h"
#include "core/json_writer.h"
#include "core/text_utils.h"
#include "core/value_extremes.h"
#include "plotly/surface_figure.h"

namespace plotconv {

    namespace {
        /// <summary>
		/// Reduces the amount of whitespace in a string to a single space, and trims leading/trailing whitespace.
        /// </summary>
		/// <remarks> Example: "AT TIME:   0.00 HOURS" -> "AT TIME: 0.00 HOURS" </remarks>
        /// <param name="text"> The input string. </param>
        std::string collapseSpaces(std::string_view text)
        {
            std::string result;

			// Sets a flag to determine whether the previous character was whitespace.
            bool previousWasSpace = false;

			// Iterates through each character in the input string, trimming leading/trailing whitespace and reducing consecutive whitespace to a single space.
            for (char c : trim(text))
            {
                const bool isSpace = (c == ' ' || c == '\t');
                if (!(isSpace && previousWasSpace))
                {
                    result.push_back(isSpace ? ' ' : c);
                }
                previousWasSpace = isSpace;
            }
            return result;
        }

        /// <summary>
		/// Obtains the portion of the header text starting from the specified marker, from the first header line that contains it.
        /// </summary>
        /// <param name="header"> Header of the file. </param>
        /// <param name="marker"> The marker to search for in the header. </param>
        std::string headerTextFrom(const FileHeader& header, std::string_view marker)
        {
            const std::string upperMarker = toUpper(marker);

			// Iterates through each line in the header, searching for the specified marker (case-insensitive).
            for (const std::string& line : header.lines)
            {
                const size_t position = toUpper(line).find(upperMarker);
                if (position != std::string::npos)
                {
                    return collapseSpaces(std::string_view(line).substr(position));
                }
            }
            return {};
        }

        /// <summary>
        /// Extracts the row label from the file header when the header specifies what the rows represent.
        /// </summary>
		/// <remarks> Example: "... = WEAR AT AXIAL NODE I, CIRCUMFERENTIAL NODE J" -> "Axial Node". </remarks>
        /// <param name="header"> The file header. </param>
        std::string rowLabelFromHeader(const FileHeader& header)
        {
			// Iterates through each line in the header, searching for specific phrases that indicate the row label.
            for (const std::string& line : header.lines)
            {
                const std::string upper = toUpper(line);

                if (upper.find("AXIAL NODE I") != std::string::npos)
                {
                    return "Axial Node";
                }
                if (upper.find("RADIAL NODE I") != std::string::npos)
                {
                    return "Radial Node";
                }
            }
            return {};
        }
    }

    /// <summary>
	/// Handler for converting a raw single matrix file into a static surface plot file.
    /// </summary>
    /// <param name="type"> Type of the single matrix handler. </param>
    SingleMatrixHandler::SingleMatrixHandler(SingleMatrixType type) : type_(std::move(type)) {}

	// Identifies the name of the handler, which is the expected file key for this handler.
    std::string_view SingleMatrixHandler::name() const
    {
        return type_.fileKey;
    }

	// Determines if the given file name matches the expected file key for this handler.
    bool SingleMatrixHandler::matches(std::string_view upperFileName) const
    {
        return upperFileName.find(type_.fileKey) != std::string_view::npos;
    }

    /// <summary>
	/// Converts a raw single matrix file into a static surface plot file, based on the specified single matrix type.
    /// </summary>
    /// <param name="context"> The conversion context. </param>
    ConversionResult SingleMatrixHandler::convert(ConversionContext& context) const
    {
		// Extracts the header and reader from the conversion context.
        const FileHeader& header = context.header;
        LineReader& reader = context.reader;

		// Sets the initial number of columns based on the header parameters, if available. If not, it defaults to 0 and will be determined by the first data row. 
        size_t columns = header.parameters ? static_cast<size_t>(header.parameters->nodes) : 0;
        size_t rows = 0;

		// Prepares the writer for the surface matrix of the plot.
        std::string matrixJson;
        JsonWriter matrix(matrixJson);

        matrix.beginArray();

		// Prepares an object to hold the parsed data row and track the extremes of the values for color scaling.
        DataRow row;
        ValueExtremes extremes;
        std::string line;

		// Iterates through each line in the file, reading every raw row and adding it to the surface matrix.
        while (reader.next(line))
        {
			// Skips empty lines.
            if (trim(line).empty())
            {
                continue;
            }

			// Parses the current line into a data row, checking for errors.
            const std::string rowError = parseDataRow(line, columns, row);
            
			// Verifies if there was an error parsing the row.
            if (!rowError.empty())
            {
                return ConversionResult::error("line " + std::to_string(reader.lineNumber()) + ": " + rowError);
            }

			// Updates the number of columns based on the parsed row.
            columns = row.tokens.size();

			// Iterates through each column in the row, writing the value to the JSON representation and updating the extremes for color scaling.
            matrix.beginArray();
            for (size_t column = 0; column < columns; ++column)
            {
                matrix.writeNumberToken(row.tokens[column]);
                extremes.add(row.values[column], row.tokens[column]);
            }
            matrix.endArray();

			// Increments the row count after successfully processing a data row.
            ++rows;
        }

		// Verifies that at least one data row was found.
        if (rows == 0)
        {
            return ConversionResult::error("no data rows found");
        }

        matrix.endArray();

		// Prepares a vector to hold any warnings generated during the conversion process.
        std::vector<std::string> warnings;

		// Determines the Y-axis label based on the handler type or the file header.
        std::string yLabel = std::string(type_.yLabel);
        
		// Verifies if the Y-axis label is empty.
        if (yLabel.empty())
        {
            yLabel = rowLabelFromHeader(header);
            if (yLabel.empty())
            {
                yLabel = "Node";
                warnings.push_back("the header does not say what the rows are; the Y axis is labelled 'Node'");
            }
        }

		// Determines the title of the surface figure based on the file header or the file name.
        std::string title = header.title.empty() ? context.path.stem().string() : header.title;

		// Verifies if a title suffix marker is specified in the handler type.
        if (!type_.titleSuffixMarker.empty())
        {
			// Extracts the title suffix from the header based on the specified marker and appends it to the title if found.
            const std::string suffix = headerTextFrom(header, type_.titleSuffixMarker);
            if (!suffix.empty())
            {
                title += " " + suffix;
            }
        }

		// Sets the main values for the surface figure, including the title, labels, ranges and the surface matrix.
        SurfaceFigure surface;
        surface.title = std::move(title);
        surface.xLabel = std::string(type_.xLabel);
        surface.yLabel = std::move(yLabel);
        surface.zLabel = labelWithUnits(type_.zLabel, header.units);
        surface.xRange = paddedRange(0.0, static_cast<double>(columns) - 1.0);
        surface.yRange = paddedRange(0.0, static_cast<double>(rows) - 1.0);
        surface.zRange = paddedRange(extremes.min(), extremes.max());
        surface.colorMinToken = extremes.minToken();
        surface.colorMaxToken = extremes.maxToken();
        surface.firstFrameZ = std::move(matrixJson);

        // Writes the plot file.
        std::string json;
        JsonWriter writer(json);
        writeSurfaceFigure(writer, surface);

        // Verifies that the plot file was completely written.
        if (!writer.isComplete())
        {
            throw std::logic_error("the generated JSON document is not complete");
        }

        return ConversionResult::success(std::move(json), std::move(warnings));
    }
}
