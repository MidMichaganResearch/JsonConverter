#pragma once

#include <string>
#include <utility>
#include <vector>

namespace plotconv {
    enum class ConversionStatus
    {
        Ok,
        UnsupportedFile,
        Error
    };

    /// Outcome of converting one raw file.
    /// The library never prints and never lets an exception escape: everything the
    /// caller needs to know is in this object.
    struct ConversionResult
    {
        ConversionStatus status = ConversionStatus::Error;
        std::string json;
        std::string message;
       std::vector<std::string> warnings;

        bool ok() const { return status == ConversionStatus::Ok; }

        static ConversionResult success(std::string json, std::vector<std::string> warnings = {})
        {
            ConversionResult result;
            result.status = ConversionStatus::Ok;
            result.json = std::move(json);
            result.warnings = std::move(warnings);
            return result;
        }

        static ConversionResult unsupported(std::string message)
        {
            ConversionResult result;
            result.status = ConversionStatus::UnsupportedFile;
            result.message = std::move(message);
            return result;
        }

        static ConversionResult error(std::string message)
        {
            ConversionResult result;
            result.status = ConversionStatus::Error;
            result.message = std::move(message);
            return result;
        }
    };
}
