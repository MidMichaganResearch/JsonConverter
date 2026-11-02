#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "handlers/type_handler.h"

namespace plotconv {

    /// <summary>
    /// Description of one file type whose data is a set of columns drawn as curves of a 2D line plot.
    /// </summary>
    struct LineSeriesType
    {
        std::string_view fileKey;
        std::string_view xLabel;
        std::string_view yLabel;
        bool xFromFirstColumn;
        std::string_view titleSuffix;
        size_t ignoredLeadingLines;
    };

    /// The table of supported line series types (line_series_types.cpp).
    const std::vector<LineSeriesType>& lineSeriesTypes();

    /// <summary>
    /// Generic handler driven by one LineSeriesType.
    /// </summary>
    class LineSeriesHandler final : public TypeHandler
    {
    public:
        explicit LineSeriesHandler(LineSeriesType type);

        std::string_view name() const override;
        bool matches(std::string_view upperFileName) const override;
        ConversionResult convert(ConversionContext& context) const override;

    private:
        LineSeriesType type_;
    };

}
