#pragma once

#include <filesystem>
#include <string_view>

#include "core/conversion_result.h"
#include "core/file_header.h"
#include "core/line_reader.h"

namespace plotconv {

    /// What the orchestrator hands to a handler: the file's header 
    /// and the reader positioned at the first data line.
    struct ConversionContext
    {
        const std::filesystem::path& path;
        const FileHeader& header;
        LineReader& reader;
    };

    /// One supported kind of raw file. Handlers are independent of each other:
    /// adding one never requires touching another.
    class TypeHandler
    {
    public:
        virtual ~TypeHandler() = default;

        /// Short name for diagnostics.
        virtual std::string_view name() const = 0;

        /// `upperFileName` is the file name in upper case.
        virtual bool matches(std::string_view upperFileName) const = 0;

        /// Reads the data and produces the finished Plotly JSON.
        /// Must not write anything until the whole file has been validated.
        virtual ConversionResult convert(ConversionContext& context) const = 0;
    };
}  
