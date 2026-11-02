#pragma once

#include <string_view>
#include <vector>

#include "handlers/type_handler.h"

namespace plotconv {
    /// Description of one file type whose data is a sequence of matrices
    /// (one per output step), drawn as an animated surface.
    ///
    /// This is plain data on purpose. If a new type needs behaviour that these
    /// fields cannot express, write a new TypeHandler instead of adding fields here.
    struct MatrixFramesType
    {
        std::string_view fileKey;
        std::string_view xLabel;
        std::string_view yLabel;
        std::string_view zLabel;

        // The header's NODES value is the extent of one matrix dimension.
        bool nodesAreColumns;
        int otherExtent;
    };

    /// The table of supported matrix-frame types.
    const std::vector<MatrixFramesType>& matrixFramesTypes();

    /// Generic handler driven by one MatrixFramesType.
    class MatrixFramesHandler final : public TypeHandler
    {
    public:
        explicit MatrixFramesHandler(MatrixFramesType type);

        std::string_view name() const override;
        bool matches(std::string_view upperFileName) const override;
        ConversionResult convert(ConversionContext& context) const override;

    private:
        MatrixFramesType type_;
    };
}
