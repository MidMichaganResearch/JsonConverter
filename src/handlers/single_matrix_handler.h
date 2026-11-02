#pragma once

#include <string_view>
#include <vector>

#include "handlers/type_handler.h"

namespace plotconv {
    /// Description of one file type whose data is a single matrix, drawn as a static surface.
    /// Like MatrixFramesType, this is plain data: behaviour that these fields cannot
    /// express belongs in a new TypeHandler.
    struct SingleMatrixType
    {
        std::string_view fileKey;
        std::string_view xLabel;
        std::string_view yLabel;

        std::string_view zLabel;
        std::string_view titleSuffixMarker;
    };

    /// The table of supported single-matrix types.
    const std::vector<SingleMatrixType>& singleMatrixTypes();

    /// Generic handler driven by one SingleMatrixType.
    ///
    /// The number of columns comes from the header's NODES value when it has one, otherwise from the first data row; every row must then have that many values.
    /// The number of rows is whatever the file contains.
    class SingleMatrixHandler final : public TypeHandler
    {
    public:
        explicit SingleMatrixHandler(SingleMatrixType type);

        std::string_view name() const override;
        bool matches(std::string_view upperFileName) const override;
        ConversionResult convert(ConversionContext& context) const override;

    private:
        SingleMatrixType type_;
    };

}
