#include "core/text_utils.h"

namespace plotconv {

    namespace {
        /// <summary>
        /// Determines whether the specified character is a whitespace character.
        /// </summary>
        bool isWhitespace(char c)
        {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
        }

        /// <summary>
        /// Determines whether the specified character is a separator character (space, tab, comma, or newline).
        /// </summary>
        bool isSeparator(char c)
        {
            return c == ' ' || c == '\t' || c == ',' || c == '\r' || c == '\n';
        }
    }

    /// <summary>
    /// Removes leading and trailing whitespace characters from text strings.
    /// </summary>
    std::string_view trim(std::string_view text)
    {
		// Removes prefix whitespace characters from the text string.
        while (!text.empty() && isWhitespace(text.front()))
        {
            text.remove_prefix(1);
        }

		// Removes suffix whitespace characters from the text string.
        while (!text.empty() && isWhitespace(text.back()))
        {
            text.remove_suffix(1);
        }
        return text;
    }

    /// <summary>
    /// Converts the text strings to uppercase.
    /// </summary>
    std::string toUpper(std::string_view text)
    {
        std::string result(text);

		// Iterates through each character in the result string and converts it to uppercase if it is a lowercase letter.
        for (char& c : result)
        {
            if (c >= 'a' && c <= 'z')
            {
                c = static_cast<char>(c - 'a' + 'A');
            }
        }
        return result;
    }

    /// <summary>
    /// Determines whether the text string starts with the given prefix.
    /// </summary>
    bool startsWith(std::string_view text, std::string_view prefix)
    {
        return text.substr(0, prefix.size()) == prefix;
    }

    /// <summary>
    /// Splits the text string into tokens based on separator characters.
    /// </summary>
    void splitTokens(std::string_view line, std::vector<std::string_view>& tokens)
    {
        tokens.clear();

		// Sets the index to zero and gets the size of the line.
        size_t index = 0;
        const size_t size = line.size();

		// Iterates through the characters in the line and splits it into tokens based on separator characters.
        while (index < size)
        {
			// Skips any leading separator characters.
            while (index < size && isSeparator(line[index]))
            {
                ++index;
            }

            const size_t start = index;

			// Skips any non-separator characters to find the end of the token.
            while (index < size && !isSeparator(line[index]))
            {
                ++index;
            }

			// Verifies if a token was found and adds it to the tokens vector.
            if (index > start)
            {
                tokens.push_back(line.substr(start, index - start));
            }
        }
    }

    /// <summary>
    /// Splits the text string into pieces based on commas, trimming whitespace from each piece.
    /// </summary>
    std::vector<std::string> splitOnCommas(std::string_view line)
    {
        std::vector<std::string> pieces;

		// Sets the starting index to zero.
        size_t start = 0;
        
		// Iterates through the line and splits it into pieces based on commas, trimming whitespace from each piece.
        while (true)
        {
            const size_t comma = line.find(',', start);
            const size_t end = (comma == std::string_view::npos) ? line.size() : comma;

            pieces.emplace_back(trim(line.substr(start, end - start)));

            if (comma == std::string_view::npos)
            {
                break;
            }
            start = comma + 1;
        }

        return pieces;
    }

    /// <summary>
    /// Converts upper case header text to title case, so "GAS PRESSURE FORCE" becomes "Gas Pressure Force".
    /// </summary>
    std::string toTitleCase(std::string_view text)
    {
        std::string result(text);

        // Capitalizes the first letter of each word, and lowers the rest of its letters.
        bool startOfWord = true;
        for (char& c : result)
        {
            const bool isLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            const bool isDigit = c >= '0' && c <= '9';

            if (isLetter)
            {
                if (startOfWord && c >= 'a' && c <= 'z')
                {
                    c = static_cast<char>(c - 'a' + 'A');
                }
                else if (!startOfWord && c >= 'A' && c <= 'Z')
                {
                    c = static_cast<char>(c - 'A' + 'a');
                }
            }
            startOfWord = !isLetter && !isDigit;
        }
        return result;
    }

    /// <summary>
    /// Adds the units read from the header to an axis label, so "Force" and "N" become "Force (N)".
    /// </summary>
    /// <remarks> The label is returned unchanged when the file states no units. </remarks>
    std::string labelWithUnits(std::string_view label, std::string_view units)
    {
        if (units.empty())
        {
            return std::string(label);
        }
        return std::string(label) + " (" + std::string(units) + ")";
    }

}
