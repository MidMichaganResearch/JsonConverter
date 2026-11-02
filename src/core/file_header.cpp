#include "core/file_header.h"

#include <climits>
#include <cmath>

#include "core/number_token.h"
#include "core/text_utils.h"

namespace plotconv {

    namespace {
		// Sets the maximum number of header lines to read, to avoid reading the entire file if it is not a valid data file.
        constexpr size_t kMaxHeaderLines = 20;

		// Determines whether the line holds only numbers, which marks the end of the header.
        bool isNumericLine(std::string_view line)
        {
			// Splits the line into tokens.
            std::vector<std::string_view> tokens;
            splitTokens(line, tokens);

			// Verifies that there are tokens to check.
            if (tokens.empty())
            {
                return false;
            }

			// Iterates through each token and checks if it is a valid JSON number.
            std::string scratch;
            for (std::string_view token : tokens)
            {
                if (!toJsonNumber(token, scratch))
                {
                    return false;
                }
            }
            return true;
        }

		// Cleans the title by removing the "CONTAINS" prefix, "THE" prefix, and any trailing text after a period.
        std::string cleanTitle(std::string_view firstLine)
        {
			// Obtains the text after "CONTAINS" if present, otherwise uses the entire first line.
            std::string_view text = firstLine;

			// Verifies if the first line contains "CONTAINS" and extracts the text after it.
            const size_t marker = toUpper(firstLine).find("CONTAINS");
            if (marker != std::string::npos)
            {
                text = firstLine.substr(marker + 8);
            }
            text = trim(text);

			// Verifies if the text starts with "THE" and removes it if present.
            if (startsWith(toUpper(text), "THE "))
            {
                text = trim(text.substr(4));
            }

			// Iterates through the text to find a period followed by a space or end of line, and trims the text accordingly.
            for (size_t i = 0; i < text.size(); ++i)
            {
                if (text[i] == '.' && (i + 1 == text.size() || text[i + 1] == ' '))
                {
                    text = trim(text.substr(0, i));
                    break;
                }
            }

            return std::string(text.empty() ? trim(firstLine) : text);
        }

		// Converts a double value to an integer if it is a positive whole number within the range of int.
        bool toInteger(double value, int& result)
        {
			// Verifies if the value is a whole number and within the range of int.
            if (value != std::floor(value) || value < 1.0 || value > INT_MAX)
            {
                return false;
            }
            result = static_cast<int>(value);
            return true;
        }

		// Splits the parameter names line (NODES, TIMESTEP (s), ...) on commas, dropping any empty name at the end.
        bool parseParameters(std::string_view namesLine, std::string_view valuesLine,
                             SimulationParameters& parameters, std::string& error)
        {
			// Iterates through the names and values, converting them to uppercase and checking for specific parameter names.
            std::vector<std::string> names = splitOnCommas(namesLine);
            while (!names.empty() && names.back().empty())
            {
                names.pop_back();
            }

			// Splits the values line into tokens and checks if the number of values matches the number of names.
            std::vector<std::string_view> values;
            splitTokens(valuesLine, values);

			// Verifies that the number of names matches the number of values.
            if (names.size() != values.size())
            {
                error = "parameter line has " + std::to_string(values.size()) + " values but the header names "
                    + std::to_string(names.size());
                return false;
            }

			// Sets a flag to track if the "NODES" parameter was found and initializes a token string for conversion.
            bool nodesFound = false;
            std::string token;

			// Iterates through each name-value pair, converting the value to a double and checking for specific parameter names.
            for (size_t i = 0; i < names.size(); ++i)
            {
				// Verifies that the value is a valid JSON number.
                if (!toJsonNumber(values[i], token))
                {
                    error = "parameter value '" + std::string(values[i]) + "' is not a number";
                    return false;
                }

				// Sets the value as a double and converts the name to uppercase for comparison.
                const double value = jsonNumberToDouble(token);
                const std::string name = toUpper(names[i]);

                // Verifies the parameter is for nodes.
                if (startsWith(name, "NODES"))
                {
                    if (!toInteger(value, parameters.nodes))
                    {
                        error = "NODES must be a positive whole number, found '" + std::string(values[i]) + "'";
                        return false;
                    }
                    nodesFound = true;
                }
				// Verifies the parameter is for timestep.
                else if (startsWith(name, "TIMESTEP"))
                {
                    parameters.timestepSeconds = value;
                }
				// Verifies the parameter is for RPM.
                else if (startsWith(name, "RPM"))
                {
                    parameters.rpm = value;
                }
				// Verifies the parameter is for steps per output.
                else if (startsWith(name, "STEPS"))
                {
                    int steps = 0;
                    if (!toInteger(value, steps))
                    {
                        error = "STEPS PER OUTPUT must be a positive whole number, found '" + std::string(values[i]) + "'";
                        return false;
                    }
                    parameters.stepsPerOutput = steps;
                }
				// Verifies the parameter is for conformance.
                else if (startsWith(name, "CONFORMAN"))
                {
                    if (value != 0.0 && value != 1.0)
                    {
                        error = "CONFORMANCE must be 0 or 1, found '" + std::string(values[i]) + "'";
                        return false;
                    }
                    parameters.conformance = (value == 1.0);
                }
            }

			// Verifies that the nodes parameter was found in the header lines.
            if (!nodesFound)
            {
                error = "parameter line has no NODES field";
                return false;
            }

            return true;
        }

		// Finalizes the header by extracting the title and units from the header lines.
        void finishHeader(FileHeader& header)
        {
			// Sets the title from the first line of the header if it is not empty.
            if (!header.lines.empty())
            {
                header.title = cleanTitle(header.lines.front());
            }

			// Iterates through the header lines to find the units line and extracts the units if found.
            for (const std::string& line : header.lines)
            {
                if (startsWith(toUpper(line), "UNITS:"))
                {
                    header.units = std::string(trim(std::string_view(line).substr(6)));
                    break;
                }
            }
        }
    }

	// Reads the file header from the given LineReader and populates the FileHeader structure.
    bool readFileHeader(LineReader& reader, FileHeader& header, std::string& error)
    {
		// Sets the header to an empty state before reading.
        header = FileHeader{};

		// Iterates through the lines of the file.
        std::string line;
        while (reader.next(line))
        {
            // Trims the line and checks if it's empty.
            const std::string_view text = trim(line);

			// Verifies if the line is empty and continues to the next line if it is.
            if (text.empty())
            {
                continue;
            }

			// Verifies if the line is numeric and processes it accordingly.
            if (isNumericLine(text))
            {
				// Sets a flag to indicate if the last line in the header is a parameter line.
                const bool hasParameterLine =
                    !header.lines.empty() && startsWith(toUpper(header.lines.back()), "NODES");

				// Verifies if the last line in the header is a parameter line and parses it if present.
                if (hasParameterLine)
                {
                    SimulationParameters parameters;
                    std::string reason;
                    if (!parseParameters(header.lines.back(), text, parameters, reason))
                    {
                        error = "line " + std::to_string(reader.lineNumber()) + ": " + reason;
                        return false;
                    }
                    header.parameters = parameters;
                }
                // If the last header line is not a parameter line, the numeric line is already raw data, so it is handed back to the reader.
                else
                {
                    reader.pushBack(std::move(line));
                }

                finishHeader(header);
                return true;
            }

			// Adds the line to the header lines and checks if the maximum number of header lines has been exceeded.
            header.lines.emplace_back(text);

			// Verifies if the number of header lines exceeds the maximum allowed.
            if (header.lines.size() > kMaxHeaderLines)
            {
                error = "no numeric data found in the first " + std::to_string(kMaxHeaderLines) + " lines";
                return false;
            }
        }

        error = "the file contains no numeric data";
        return false;
    }
}