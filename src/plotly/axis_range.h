#pragma once

namespace plotconv {
    /// Display range of an axis.
    struct AxisRange
    {
        double min = 0.0;
        double max = 0.0;
    };

    /// Widens [min, max] by `fraction` of its span on both sides, so points at the edge are not drawn on the axis line.
    inline AxisRange paddedRange(double min, double max, double fraction = 0.05)
    {
        const double padding = (max - min) * fraction;
        return {min - padding, max + padding};
    }
}
