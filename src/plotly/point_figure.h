#pragma once

#include <cstddef>
#include <string>

#include "core/json_writer.h"
#include "plotly/axis_range.h"

namespace plotconv {

    /// <summary>
    /// The four value arrays of one frame of a 3D point plot, already serialized.
    /// </summary>
    struct PointValues
    {
        std::string x;
        std::string y;
        std::string z;
        std::string color;
    };

    /// <summary>
    /// Everything needed to draw an animated 3D point plot: the points of every frame joined by a line,
    /// colored by a fourth value, with play/stop buttons and a slider.
    /// </summary>
    struct PointFigure
    {
        std::string title;
        std::string xLabel;
        std::string yLabel;
        std::string zLabel;
        std::string colorLabel;
        std::string colorUnits;

        AxisRange xRange;
        AxisRange yRange;
        AxisRange zRange;

        std::string colorMinToken;
        std::string colorMaxToken;

        PointValues firstFrame;
        std::string framesJson;
        size_t frameCount = 0;
        std::string sliderPrefix;
    };

    /// Writes the complete 3D point plot file: {"data": [...], "layout": {...}, "frames": [...]}.
    void writePointFigure(JsonWriter& writer, const PointFigure& figure);

    /// Writes one entry of the "frames" array, named f<index>.
    void writePointFrame(JsonWriter& writer, size_t index, const PointValues& values);

}
