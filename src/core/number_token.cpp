#include "core/number_token.h"

#include <cstdlib>

#include "core/text_utils.h"

namespace plotconv {

    namespace {
		// Determines if a character is a digit (0-9).
        bool isDigit(char c)
        {
            return c >= '0' && c <= '9';
        }
        
        /// <summary>
		/// Inserts a missing 'E' in a Fortran-style number string if the exponent is present but lacks the 'E' character.
        /// </summary>
		/// <remarks> This is only used when the mantissa has a decimal point and the exponent has exactly three digits.</remarks>
        /// <param name="text"> The number string to process. </param>
        void insertMissingExponentLetter(std::string& text)
        {
            // Verifies if the value is missing an exponent letter.
            if (text.find_first_of("Ee") != std::string::npos)
            {
                return;
            }

			// Verifies if the string contains a sign character ('+' or '-') after the first character, which indicates the presence of an exponent.
            const size_t sign = text.find_first_of("+-", 1);
            if (sign == std::string::npos || !isDigit(text[sign - 1]))
            {
                return;
            }

			// Verifies if the string contains a decimal point before the exponent sign.
            const size_t dot = text.find('.');
            if (dot == std::string::npos || dot > sign)
            {
                return;
            }

			// Verifies if the exponent has exactly three digits after the sign.
            if (text.size() - sign - 1 != 3)
            {
                return;
            }

			// Iterates through the characters after the sign to ensure they are all digits.
            for (size_t i = sign + 1; i < text.size(); ++i)
            {
                if (!isDigit(text[i]))
                {
                    return;
                }
            }

            // If all the verifications pass, inserts the exponent letter.
            text.insert(sign, 1, 'E');
        }

        /// <summary>
		/// Inserts a leading zero in a number string if it starts with a decimal point.
        /// </summary>
        /// <param name="text"> The number string to process. </param>
        void insertMissingLeadingZero(std::string& text)
        {
			// Determines the position to check for a leading decimal point, accounting for an optional negative sign.
            const size_t position = (!text.empty() && text[0] == '-') ? 1 : 0;

			// Verifies if the character at the determined position is a decimal point and inserts a leading zero if necessary.
            if (position < text.size() && text[position] == '.')
            {
                text.insert(position, 1, '0');
            }
        }

        /// <summary>
		/// Inserts a zero after the decimal point in a number string if it is missing, ensuring that there is at least one digit after the decimal point.
        /// </summary>
        /// <param name="text"> The number string to process. </param>
        void insertMissingFractionDigit(std::string& text)
        {
			// Verifies if the string contains a decimal point.
            const size_t dot = text.find('.');
            if (dot == std::string::npos)
            {
                return;
            }

			// Verifies if the decimal point is the last character in the string or if the character following it is not a digit, and inserts a zero if necessary.
            if (dot + 1 >= text.size() || !isDigit(text[dot + 1]))
            {
                text.insert(dot + 1, 1, '0');
            }
        }
    }

	// Determines if the given string represents a valid JSON number according to the JSON specification.
    bool isJsonNumber(std::string_view text)
    {
		// Sets the initial index to determine the position in the string and retrieves the size of the string.
        size_t i = 0;
        const size_t size = text.size();

		// Verifies if the string starts with a negative sign and increments the index if it does.
        if (i < size && text[i] == '-')
        {
            ++i;
        }

		// Verifies if the string is empty or if the index has reached the end of the string.
        if (i >= size)
        {
            return false;
        }

		// Verifies if the string starts with a zero or a non-zero digit and processes the integer part of the number accordingly.
        if (text[i] == '0')
        {
            ++i;
        }
		// If the string starts with a non-zero digit, it continues to process the integer part of the number.
        else if (text[i] >= '1' && text[i] <= '9')
        {
            while (i < size && isDigit(text[i]))
            {
                ++i;
            }
        }
		// And if the string does not start with a valid digit, it stops processing.
        else
        {
            return false;
        }
        
		// Verifies if the string contains a decimal point and processes the fractional part of the number accordingly.
        if (i < size && text[i] == '.')
        {
            ++i;
            const size_t start = i;
            while (i < size && isDigit(text[i]))
            {
                ++i;
            }
            if (i == start)
            {
                return false;
            }
        }

		// Verifies if the string contains an exponent part, and processes it accordingly.
        if (i < size && (text[i] == 'e' || text[i] == 'E'))
        {
            ++i;
            if (i < size && (text[i] == '+' || text[i] == '-'))
            {
                ++i;
            }
            const size_t start = i;
            while (i < size && isDigit(text[i]))
            {
                ++i;
            }
            if (i == start)
            {
                return false;
            }
        }

        return i == size;
    }

    // Repairs the notation of a raw value so it can be copied to the plot file without changing its digits.
    bool toJsonNumber(std::string_view raw, std::string& out)
    {
		// Trims whitespace from the input string and assigns it to the output string.
        raw = trim(raw);
        out.assign(raw.data(), raw.size());

		// Verifies if the output string is empty after trimming whitespace.
        if (out.empty())
        {
            return false;
        }

		// Verifies if the output string is already a valid JSON number.
        if (isJsonNumber(out))
        {
            return true;
        }

		// Iterates through the characters in the output string, replacing the used exponent letter for the appropriate format for JSON numbers.
        for (char& c : out)
        {
            if (c == 'D' || c == 'd')
            {
                c = 'E';
                break;
            }
        }

		// Verifies if the output string starts with a '+' character and removes it if present.
        if (out.front() == '+')
        {
            out.erase(0, 1);
        }

		// Normalizes the number string by inserting a missing exponent letter, leading zero, and fraction digit if necessary, and then checks if it is a valid JSON number.
        insertMissingExponentLetter(out);
        insertMissingLeadingZero(out);
        insertMissingFractionDigit(out);

        return isJsonNumber(out);
    }

	// Converts a checked raw value to a number, used only for the axis ranges and the color scale.
    double jsonNumberToDouble(const std::string& token)
    {
        return std::strtod(token.c_str(), nullptr);
    }
}
