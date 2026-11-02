#include "handlers/single_matrix_handler.h"

namespace plotconv {

    /// <summary>
	/// Sets the properties for each type of single matrix file, including the file key, axis labels, and title suffix marker.
    /// </summary>
    const std::vector<SingleMatrixType>& singleMatrixTypes()
    {
        static const std::vector<SingleMatrixType> types = {
            // key | x label | y label (empty: header) | z label | title suffix marker
            {"_WEAR",                  "Circumferential Node", "",            "Wear",      "AT TIME"},
            {"FACE_CLEARANCE_CONFORM", "Circumferential Node", "Output Step", "Clearance", ""       },
        };
        return types;
    }
}