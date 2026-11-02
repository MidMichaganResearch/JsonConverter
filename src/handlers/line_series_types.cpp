#include "handlers/line_series_handler.h"

namespace plotconv {

    /// <summary>
	/// Sets the properties for each type of line series file, including the file key, axis labels, where x comes from, and title fixes.
    /// </summary>
    const std::vector<LineSeriesType>& lineSeriesTypes()
    {
        // RING_FORCES ignores the first 5 lines, as the old converter did; with a 3 line header this drops the first two crank angles.
        // STRAIN_ENERGY_CONFORM completes its title, which the file cuts after "DURING".
        static const std::vector<LineSeriesType> types = {
            // key | x label | y label | x from first column | title suffix | ignored leading lines
            {"RING_FORCES",           "Crank Angle", "Force",         true,  "",            5},
            {"STRAIN_ENERGY_CONFORM", "Output Step", "Strain Energy", false, "CONFORMANCE", 0},
        };
        return types;
    }
}
