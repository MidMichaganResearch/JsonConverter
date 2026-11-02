#pragma once

#include <cstddef>
#include <istream>
#include <string>
#include <utility>

namespace plotconv {

/// Reads a text file line by line, tracking line numbers for error messages.
/// Handles Windows line endings, and lets the header reader hand back the
/// first data line it had to look at, so the file is still read only once.
class LineReader
{
    public:
        explicit LineReader(std::istream& input) : input_(input) {}

        /// Reads the next line (without the line terminator).
        bool next(std::string& line)
        {
            if (hasPending_)
            {
                line = std::move(pending_);
                hasPending_ = false;
                ++lineNumber_;
                return true;
            }

            if (!std::getline(input_, line))
            {
                return false;
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            ++lineNumber_;
            return true;
        }

        /// Returns the line most recently read so that next() yields it again.
        void pushBack(std::string line)
        {
            pending_ = std::move(line);
            hasPending_ = true;
            --lineNumber_;
        }

        /// 1-based number of the line most recently returned by next().
        size_t lineNumber() const { return lineNumber_; }

    private:
        std::istream& input_;
        std::string pending_;
        bool hasPending_ = false;
        size_t lineNumber_ = 0;
    };
}
