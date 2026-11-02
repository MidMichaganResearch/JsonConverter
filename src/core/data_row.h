#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace plotconv {

/// The numbers of one data line, validated and ready to be written.
struct DataRow
{
    std::vector<std::string> tokens;
    std::vector<double> values;

    std::vector<std::string_view> rawTokens;
};

/// Splits `line` into numbers and validates each one (see toJsonNumber).
/// `expectedColumns` of 0 accepts any count of at least one value.
std::string parseDataRow(std::string_view line, size_t expectedColumns, DataRow& row);

}
