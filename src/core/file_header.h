#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/line_reader.h"

namespace plotconv {
    /// Fields are matched by the start of their name (some files truncate or
    /// decorate them), so a header with or without RPM reads correctly.
    struct SimulationParameters
    {
        int nodes = 0;
        std::optional<double> timestepSeconds;
        std::optional<double> rpm;
        std::optional<int> stepsPerOutput;
        std::optional<bool> conformance;
    };

    struct FileHeader
    {
        std::string title;
        std::string units;
        std::vector<std::string> lines;
        std::optional<SimulationParameters> parameters;
    };

    /// Reads the header and leaves `reader` positioned at the first data line.
    ///
    /// The header ends at the first line made only of numbers. If the line before it starts with "NODES",
    /// that numeric line is the parameter line and is consumed; otherwise it already is data and is
    /// handed back to the reader. Header length therefore does not need to be known in advance.
    bool readFileHeader(LineReader& reader, FileHeader& header, std::string& error);
}
