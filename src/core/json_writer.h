#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace plotconv {

/// Minimal streaming JSON writer.
///
/// It owns the punctuation (commas, colons, quotes, escaping) so that callers
/// only describe structure:
///
///     writer.beginObject();
///     writer.key("name");  writer.writeString("x");
///     writer.endObject();
class JsonWriter
{
    public:
        explicit JsonWriter(std::string& output);

        void beginObject();
        void endObject();
        void beginArray();
        void endArray();

        /// Starts a member of the current object. Must be followed by exactly one value.
        void key(std::string_view name);

        /// Quoted and escaped. Non-ASCII text is written as \uXXXX so the output is pure ASCII.
        void writeString(std::string_view text);
        void writeBool(bool value);
        void writeNull();
        void writeInteger(long long value);

        /// Computed value, written with the fewest digits that read back as the same double.
        void writeNumber(double value);

        /// Copies a number verbatim.
        void writeNumberToken(std::string_view token);

        /// Inserts already-serialized JSON (a value) as is.
        void writeRaw(std::string_view json);

        /// True once exactly one root value has been written and everything is closed.
        bool isComplete() const;

    private:
        struct Level
        {
            bool isObject;
            bool hasItems;
        };

        void beforeValue();
        void writeQuoted(std::string_view text);

        std::string& out_;
        std::vector<Level> stack_;
        bool keyPending_ = false;
        bool rootWritten_ = false;
    };

}
