#include "core/data_row.h"

#include "core/number_token.h"
#include "core/text_utils.h"

namespace plotconv {
    /// <summary>
	/// Parses one row of raw values for the plot.
    /// Each value is checked and kept both as written in the raw file (for the plot file) and as a number (for the ranges and the color scale).
    /// </summary>
    /// <param name="line"> The line of text to parse.</param>
    /// <param name="expectedColumns"> The expected number of columns. </param>
    /// <param name="row"> The row to populate with parsed values. </param>
    std::string parseDataRow(std::string_view line, size_t expectedColumns, DataRow& row)
    {
		// Splits the line into tokens and stores them in a vector of string views.
        splitTokens(line, row.rawTokens);

		// Obtains the number of tokens in the row.
        const size_t count = row.rawTokens.size();

		// Verifies that the row is not empty.
        if (count == 0)
        {
            return "the line has no values";
        }

		// Verifies that the number of tokens matches the expected number of columns, if specified.
        if (expectedColumns != 0 && count != expectedColumns)
        {
            return "expected " + std::to_string(expectedColumns) + " values but found " + std::to_string(count);
        }

		// Resizes the tokens and values vectors in the row to match the number of tokens.
        row.tokens.resize(count);
        row.values.resize(count);

		// Iterates over each token, converting it to a JSON number and storing the numeric value in the row.
        for (size_t i = 0; i < count; ++i)
        {
            if (!toJsonNumber(row.rawTokens[i], row.tokens[i]))
            {
                return "value " + std::to_string(i + 1) + " ('" + std::string(row.rawTokens[i])
                    + "') is not a valid number";
            }
            row.values[i] = jsonNumberToDouble(row.tokens[i]);
        }

        return {};
    }
}