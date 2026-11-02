#pragma once

#include <string>
#include <vector>

#include "core/json_writer.h"

namespace plotconv {

    /// <summary>
    /// Everything needed to draw a 2D line plot: several series sharing the same x values.
    /// </summary>
    struct LineFigure
    {
        std::string title;
        std::string xLabel;
        std::string yLabel;

        std::string xValues;
        std::vector<std::string> seriesNames;
        std::vector<std::string> yValues;
    };

    /// Writes the complete line plot file: {"data": [...], "layout": {...}}.
    void writeLineFigure(JsonWriter& writer, const LineFigure& figure);

}
