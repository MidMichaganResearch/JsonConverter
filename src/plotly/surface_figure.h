#pragma once

#include <cstddef>
#include <string>

#include "core/json_writer.h"
#include "plotly/axis_range.h"

namespace plotconv {
    /// Everything needed to draw an animated 3D surface: one matrix per frame, with play/stop buttons and a slider.
    struct SurfaceFigure
    {
        std::string title;
        std::string xLabel;
        std::string yLabel;
        std::string zLabel;

        AxisRange xRange;
        AxisRange yRange;
        AxisRange zRange;

        std::string colorMinToken;
        std::string colorMaxToken;

        std::string firstFrameZ;
        std::string framesJson;
        size_t frameCount = 0;
        std::string sliderPrefix;
    };

    /// Writes the complete Plotly figure: {"data": [...], "layout": {...}, "frames": [...]}.
    void writeSurfaceFigure(JsonWriter& writer, const SurfaceFigure& surface);
}
