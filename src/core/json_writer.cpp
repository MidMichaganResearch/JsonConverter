#include "core/json_writer.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include "core/number_token.h"

namespace plotconv {

    namespace {
		// Appends a Unicode escape sequence for the given code unit to the output string.
        void appendUnicodeEscape(std::string& out, unsigned unit)
        {
            static const char kHex[] = "0123456789ABCDEF";

            out += "\\u";
            out.push_back(kHex[(unit >> 12) & 0xF]);
            out.push_back(kHex[(unit >> 8) & 0xF]);
            out.push_back(kHex[(unit >> 4) & 0xF]);
            out.push_back(kHex[unit & 0xF]);
        }

		// Appends a code point to the output string, using one or two Unicode escape sequences as needed.
        void appendCodePoint(std::string& out, uint32_t codePoint)
        {
            if (codePoint <= 0xFFFF)
            {
                appendUnicodeEscape(out, codePoint);
                return;
            }

            codePoint -= 0x10000;
            appendUnicodeEscape(out, 0xD800 + (codePoint >> 10));
            appendUnicodeEscape(out, 0xDC00 + (codePoint & 0x3FF));
        }

        // Returns the length of the UTF-8 sequence starting at text[index] and stores its code point, or 0 when the bytes are not valid UTF-8.
        size_t decodeUtf8(std::string_view text, size_t index, uint32_t& codePoint)
        {
            const unsigned char lead = static_cast<unsigned char>(text[index]);

            size_t length = 0;
            uint32_t minimum = 0;

            if ((lead & 0xE0) == 0xC0)
            {
                length = 2;
                codePoint = lead & 0x1F;
                minimum = 0x80;
            }
            else if ((lead & 0xF0) == 0xE0)
            {
                length = 3;
                codePoint = lead & 0x0F;
                minimum = 0x800;
            }
            else if ((lead & 0xF8) == 0xF0)
            {
                length = 4;
                codePoint = lead & 0x07;
                minimum = 0x10000;
            }
            else
            {
                return 0;
            }

            if (index + length > text.size())
            {
                return 0;
            }

            for (size_t i = 1; i < length; ++i)
            {
                const unsigned char next = static_cast<unsigned char>(text[index + i]);
                if ((next & 0xC0) != 0x80)
                {
                    return 0;
                }
                codePoint = (codePoint << 6) | (next & 0x3F);
            }

            const bool overlong = codePoint < minimum;
            const bool surrogate = codePoint >= 0xD800 && codePoint <= 0xDFFF;
            if (overlong || surrogate || codePoint > 0x10FFFF)
            {
                return 0;
            }

            return length;
        }

		// Formats a double value as a string with enough precision to round-trip back to the same value.
        std::string formatDouble(double value)
        {
            char buffer[48];

            for (int precision = 15; precision <= 17; ++precision)
            {
                std::snprintf(buffer, sizeof(buffer), "%.*g", precision, value);
                if (std::strtod(buffer, nullptr) == value)
                {
                    break;
                }
            }

            return buffer;
        }
    }

    /// <summary>
	/// Initializes an instance of the JsonWriter class that writes to the given string.
    /// </summary>
    /// <param name="output"> The string to write the JSON data to. </param>
    JsonWriter::JsonWriter(std::string& output) : out_(output) {}

	// Writes any necessary punctuation before writing a value, and checks for errors in the current state.
    void JsonWriter::beforeValue()
    {
        if (stack_.empty())
        {
            if (rootWritten_)
            {
                throw std::logic_error("JsonWriter: the document already has a root value");
            }
            rootWritten_ = true;
            return;
        }

        Level& top = stack_.back();

        if (top.isObject)
        {
            if (!keyPending_)
            {
                throw std::logic_error("JsonWriter: a value inside an object needs key() first");
            }
            keyPending_ = false;
            return;
        }

        if (top.hasItems)
        {
            out_.push_back(',');
        }
        top.hasItems = true;
    }

	// Writes the beginning of a JSON object, and pushes a new level onto the stack.
    void JsonWriter::beginObject()
    {
        beforeValue();
        out_.push_back('{');
        stack_.push_back({true, false});
    }

	// Writes the end of a JSON object, and pops the top level from the stack.
    void JsonWriter::endObject()
    {
        if (stack_.empty() || !stack_.back().isObject || keyPending_)
        {
            throw std::logic_error("JsonWriter: endObject() does not match an open object");
        }
        stack_.pop_back();
        out_.push_back('}');
    }

	// Writes the beginning of a JSON array, and pushes a new level onto the stack.
    void JsonWriter::beginArray()
    {
        beforeValue();
        out_.push_back('[');
        stack_.push_back({false, false});
    }

	// Writes the end of a JSON array, and pops the top level from the stack.
    void JsonWriter::endArray()
    {
        if (stack_.empty() || stack_.back().isObject)
        {
            throw std::logic_error("JsonWriter: endArray() does not match an open array");
        }
        stack_.pop_back();
        out_.push_back(']');
    }

	// Writes a key for a JSON object, and sets a flag to indicate that a value is expected next.
    void JsonWriter::key(std::string_view name)
    {
        if (stack_.empty() || !stack_.back().isObject || keyPending_)
        {
            throw std::logic_error("JsonWriter: key() is only valid directly inside an object");
        }

        Level& top = stack_.back();
        if (top.hasItems)
        {
            out_.push_back(',');
        }
        top.hasItems = true;

        writeQuoted(name);
        out_.push_back(':');
        keyPending_ = true;
    }

	// Writes a string value, escaping special characters as needed.
    void JsonWriter::writeString(std::string_view text)
    {
        beforeValue();
        writeQuoted(text);
    }

	// Writes a boolean value as "true" or "false".
    void JsonWriter::writeBool(bool value)
    {
        beforeValue();
        out_ += value ? "true" : "false";
    }

	// Writes a null value as "null".
    void JsonWriter::writeNull()
    {
        beforeValue();
        out_ += "null";
    }

	// Writes an integer value as a string.
    void JsonWriter::writeInteger(long long value)
    {
        beforeValue();
        out_ += std::to_string(value);
    }

	// Writes a double value as a string, throwing an exception for NaN or Infinity.
    void JsonWriter::writeNumber(double value)
    {
        if (!std::isfinite(value))
        {
            throw std::invalid_argument("JsonWriter: NaN and Infinity cannot be written as JSON numbers");
        }

        beforeValue();
        out_ += formatDouble(value);
    }

	// Writes a number token as a string, throwing an exception if it is not a valid JSON number.
    void JsonWriter::writeNumberToken(std::string_view token)
    {
        if (!isJsonNumber(token))
        {
            throw std::invalid_argument("JsonWriter: '" + std::string(token) + "' is not a valid JSON number");
        }

        beforeValue();
        out_.append(token.data(), token.size());
    }

	// Writes raw JSON text to the output, without any escaping or validation.
    void JsonWriter::writeRaw(std::string_view json)
    {
        beforeValue();
        out_.append(json.data(), json.size());
    }

	// Returns true if the JSON document is complete.
    bool JsonWriter::isComplete() const
    {
        return rootWritten_ && stack_.empty() && !keyPending_;
    }

	// Writes a string value with quotes and escapes special characters as needed.
    void JsonWriter::writeQuoted(std::string_view text)
    {
        out_.push_back('"');

        for (size_t i = 0; i < text.size();)
        {
            const unsigned char c = static_cast<unsigned char>(text[i]);

            if (c >= 0x80)
            {
                uint32_t codePoint = 0;
                const size_t length = decodeUtf8(text, i, codePoint);

                if (length == 0)
                {
                    appendUnicodeEscape(out_, 0xFFFD);
                    ++i;
                }
                else
                {
                    appendCodePoint(out_, codePoint);
                    i += length;
                }
                continue;
            }

            switch (c)
            {
            case '"':  out_ += "\\\""; break;
            case '\\': out_ += "\\\\"; break;
            case '\b': out_ += "\\b"; break;
            case '\f': out_ += "\\f"; break;
            case '\n': out_ += "\\n"; break;
            case '\r': out_ += "\\r"; break;
            case '\t': out_ += "\\t"; break;
            default:
                if (c < 0x20)
                {
                    appendUnicodeEscape(out_, c);
                }
                else
                {
                    out_.push_back(static_cast<char>(c));
                }
            }

            ++i;
        }

        out_.push_back('"');
    }
}