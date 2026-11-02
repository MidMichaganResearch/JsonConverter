#pragma once

#include <string_view>
#include <vector>

#include "handlers/type_handler.h"

namespace plotconv {

    /// <summary>
    /// Description of one file type whose data is a sequence of frames of 3D points, each row being
    /// "x, y, z, color value", drawn as an animated 3D point plot. NODES in the header gives the points per frame.
    /// </summary>
    struct PointFramesType
    {
        std::string_view fileKey;
        std::string_view xLabel;
        std::string_view yLabel;
        std::string_view zLabel;
        std::string_view colorLabel;
        std::string_view colorUnits;
        double nodeWarningLimit;
        double frameWarningLimit;
    };

    /// The table of supported point frames types.
    const std::vector<PointFramesType>& pointFramesTypes();

    /// <summary>
    /// Generic handler driven by one PointFramesType.
    /// </summary>
    class PointFramesHandler final : public TypeHandler
    {
    public:
        explicit PointFramesHandler(PointFramesType type);

        std::string_view name() const override;
        bool matches(std::string_view upperFileName) const override;
        ConversionResult convert(ConversionContext& context) const override;

    private:
        PointFramesType type_;
    };

}
