#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "core/json_writer.h"
#include "plotly/axis_range.h"

namespace plotconv {

    /// <summary>
    /// Where the play and stop buttons sit under the plot. Each figure type keeps its own placement.
    /// </summary>
    struct ButtonsPlacement
    {
        double y = 0.0;
        int topPadding = 35;
        std::string_view xAnchor;
    };

    /// "title": {"text": ...}
    void writeTitle(JsonWriter& w, const std::string& text);

    /// The 3D scene shared by the surface and point plots: camera, the three axes with fixed ranges, and a 1:1:1 aspect.
    void writeScene(JsonWriter& w,
                    const std::string& xLabel, const AxisRange& xRange,
                    const std::string& yLabel, const AxisRange& yRange,
                    const std::string& zLabel, const AxisRange& zRange);

    /// The play and stop buttons of an animated plot.
    void writePlayStopButtons(JsonWriter& w, const ButtonsPlacement& placement);

    /// The slider of an animated plot, with one step per frame.
    void writeSlider(JsonWriter& w, size_t frameCount, const std::string& prefix, int topPadding);

}
