#include "handlers/point_frames_handler.h"

namespace plotconv {

    /// <summary>
	/// Sets the properties for each type of point frames file, including the file key, axis labels, the color value and the warning limits.
    /// </summary>
    const std::vector<PointFramesType>& pointFramesTypes()
    {
        static const std::vector<PointFramesType> types = {
            // key | x label | y label | z label | color label | color units | node warning limit | frame warning limit
            {"POS_", "X Position", "Y Position", "Z Position", "Twist", "\xC2\xB0", 10.0, 100.0},
        };
        return types;
    }
}
