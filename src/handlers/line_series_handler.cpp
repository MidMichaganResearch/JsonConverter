#include "handlers/line_series_handler.h"

#include <stdexcept>
#include <utility>

#include "core/data_row.h"
#include "core/json_writer.h"
#include "core/text_utils.h"
#include "plotly/line_figure.h"

namespace plotconv {

    namespace {
        /// <summary>
        /// Obtains the column names from the header line, in title case.
        /// </summary>
        /// <param name="header"> The file header. </param>
        std::vector<std::string> columnNamesFromHeader(const FileHeader& header)
        {
            std::vector<std::string> names;

			// Iterates through the header lines looking for the one that starts the column names.
            for (const std::string& line : header.lines)
            {
                if (!startsWith(toUpper(line), "C1:"))
                {
                    continue;
                }

				// Removes the "Cn:" prefix of each piece and keeps the name.
                for (const std::string& piece : splitOnCommas(line))
                {
                    const size_t colon = piece.find(':');
                    const std::string_view name = (colon == std::string::npos) ? std::string_view(piece)
                        : trim(std::string_view(piece).substr(colon + 1));
                    names.push_back(toTitleCase(name));
                }
                break;
            }
            return names;
        }
    }

    /// <summary>
	/// Handler for converting raw line series files into 2D line plot files, based on the specified line series type.
    /// </summary>
    /// <param name="type"> Type of the line series handler. </param>
    LineSeriesHandler::LineSeriesHandler(LineSeriesType type) : type_(std::move(type)) {}

	// Identifies the name of the handler, which is the expected file key for this handler.
    std::string_view LineSeriesHandler::name() const
    {
        return type_.fileKey;
    }

	// Determines if the given file name matches the expected file key for this handler.
    bool LineSeriesHandler::matches(std::string_view upperFileName) const
    {
        return upperFileName.find(type_.fileKey) != std::string_view::npos;
    }

    /// <summary>
	/// Converts a raw line series file into a 2D line plot file, based on the specified line series type.
    /// </summary>
    /// <param name="context"> The conversion context. </param>
    ConversionResult LineSeriesHandler::convert(ConversionContext& context) const
    {
		// Extracts the header and reader from the conversion context.
        const FileHeader& header = context.header;
        LineReader& reader = context.reader;

		// Sets the variables for the x values, the y values of each curve, and the number of columns, decided by the first raw row.
        std::string xValues;
        JsonWriter xWriter(xValues);
        xWriter.beginArray();

        std::vector<std::string> yValues;
        std::vector<JsonWriter> yWriters;
        size_t columns = 0;
        size_t rows = 0;

        DataRow row;
        std::string line;

		// Reads every raw row and adds its values to the x values and to each curve.
        while (reader.next(line))
        {
			// Skips empty lines, and the leading lines the type ignores.
            if (trim(line).empty() || reader.lineNumber() <= type_.ignoredLeadingLines)
            {
                continue;
            }

			// Parses the raw values of the row, stopping the conversion if the row is not valid.
            const std::string rowError = parseDataRow(line, columns, row);
            if (!rowError.empty())
            {
                return ConversionResult::error("line " + std::to_string(reader.lineNumber()) + ": " + rowError);
            }

			// Verifies if this is the first raw row, which decides the number of columns and of curves.
            if (rows == 0)
            {
                columns = row.tokens.size();
                const size_t firstSeriesColumn = type_.xFromFirstColumn ? 1 : 0;

                if (columns <= firstSeriesColumn)
                {
                    return ConversionResult::error("line " + std::to_string(reader.lineNumber())
                        + ": a row needs at least " + std::to_string(firstSeriesColumn + 1) + " values");
                }

				// Prepares one writer per curve. The strings are sized first so the writers can keep a reference to them.
                yValues.resize(columns - firstSeriesColumn);
                yWriters.reserve(yValues.size());
                for (std::string& values : yValues)
                {
                    yWriters.emplace_back(values);
                    yWriters.back().beginArray();
                }
            }

			// Adds the x value of the row, taken from the first column or from the row number.
            size_t column = 0;
            if (type_.xFromFirstColumn)
            {
                xWriter.writeNumberToken(row.tokens[0]);
                column = 1;
            }
            else
            {
                xWriter.writeInteger(static_cast<long long>(rows));
            }

			// Adds the remaining values of the row, one to each curve.
            for (JsonWriter& yWriter : yWriters)
            {
                yWriter.writeNumberToken(row.tokens[column]);
                ++column;
            }

            ++rows;
        }

		// Verifies that at least one raw row was found.
        if (rows == 0)
        {
            return ConversionResult::error("no data rows found");
        }

        xWriter.endArray();
        for (JsonWriter& yWriter : yWriters)
        {
            yWriter.endArray();
        }

		// Prepares a vector to hold any warnings generated during the conversion process.
        std::vector<std::string> warnings;

		// Sets the curve names from the header; the first name belongs to the x column when x comes from the file.
        std::vector<std::string> names = columnNamesFromHeader(header);
        if (names.size() == columns)
        {
            if (type_.xFromFirstColumn)
            {
                names.erase(names.begin());
            }
        }
		// If the header does not name every column, the curves are numbered instead.
        else
        {
            names.clear();
            for (size_t i = 1; i <= yValues.size(); ++i)
            {
                names.push_back("Series " + std::to_string(i));
            }
            warnings.push_back("the header does not name every column; the curves are named 'Series 1', 'Series 2', ...");
        }

		// Sets the title from the header or the file name, and completes it with the type's suffix.
        std::string title = header.title.empty() ? context.path.stem().string() : header.title;
        if (!type_.titleSuffix.empty())
        {
            title += " " + std::string(type_.titleSuffix);
        }

		// Sets the main values for the line figure.
        LineFigure figure;
        figure.title = std::move(title);
        figure.xLabel = std::string(type_.xLabel);
        figure.yLabel = labelWithUnits(type_.yLabel, header.units);
        figure.xValues = std::move(xValues);
        figure.seriesNames = std::move(names);
        figure.yValues = std::move(yValues);

		// Writes the plot file.
        std::string json;
        JsonWriter writer(json);
        writeLineFigure(writer, figure);

		// Verifies that the plot file was completely written.
        if (!writer.isComplete())
        {
            throw std::logic_error("the generated JSON document is not complete");
        }

        return ConversionResult::success(std::move(json), std::move(warnings));
    }
}
