#include "handlers/matrix_frames_handler.h"

namespace plotconv {

    /// <summary>
	/// Sets the properties for each type of matrix frames file, including the file key, axis labels, and the size of each frame.
    /// </summary>
    /// <returns></returns>
    const std::vector<MatrixFramesType>& matrixFramesTypes()
    {
        static const std::vector<MatrixFramesType> types = {
            // key | x label | y label | z label | nodesAreColumns | otherExtent
            {"FACEPRES",           "Circumferential Node", "Axial Node",           "Pressure",         true,  101},
            {"FACE_STRESS",        "Axial Node",           "Circumferential Node", "Von-Mises Stress", false, 101},
            {"TOP_STRESS",         "Radial Side Node",     "Circumferential Node", "Von-Mises Stress", false, 101},
            {"BOT_STRESS",         "Radial Side Node",     "Circumferential Node", "Von-Mises Stress", false, 101},
            {"BRA_CYCLE",          "Circumferential Node", "Radial Side Node",     "Pressure",         true,  101},
            {"TRA_CYCLE",          "Circumferential Node", "Radial Side Node",     "Pressure",         true,  101},
            {"BOTTOMPRES_CONFORM", "Circumferential Node", "Radial Side Node",     "Pressure",         true,  101},
            {"TOPPRES_CONFORM",    "Circumferential Node", "Radial Side Node",     "Pressure",         true,  101},
        };
        return types;
    }
}
