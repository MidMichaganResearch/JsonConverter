#include "parser_factory.h"

#include "../parsers/pressure_conform_parser.h"
#include "../parsers/ring_forces_parser.h"
#include "../parsers/parser_face_wear.h"
#include "../parsers/RingPositionCycleParser.h"
#include "../parsers/StrainEnergyConformParser.h"
#include "../parsers/FaceClearanceConformParser.h"
#include "../parsers/SurfaceAnimParser.h"
#include "../parsers/Scatter3dAnimParser.h"
#include <cctype> // Required for std::toupper

/// <summary>
/// Determines the appropriate parser to create based on the provided file path.
/// </summary>
/// <param name="filepath"> Path of the file to parse. </param>
std::unique_ptr<BaseParser>
ParserFactory::create(
    const std::string& filepath)
{

    // Work on an uppercase copy for case-insensitive matching so we don't attempt to modify the const input parameter.
    std::string upper_filepath = filepath;
    for (char& c : upper_filepath) {
        c = std::toupper(static_cast<unsigned char>(c));
    }

	// Case for when the file is determined to be a Face Clearance Conform graph.
    if (upper_filepath.find("FACE_CLEARANCE_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            FaceClearanceConformParser>(filepath);
    }

	// Case for when the file is determined to be a Position Cycle graph. This is used for both conformance and cycle position graphs.
    if (upper_filepath.find("POS_") != std::string::npos)
    {
        return std::make_unique<
            RingPositionCycleParser>(filepath);
    }

	// Case for when the file is determined to be a Wear graph.
    if (upper_filepath.find("_WEAR") != std::string::npos)
    {
        return std::make_unique<
            FaceWearParser>(
                filepath);
    }

	// Case for when the file is determined to be a Pressure Conform graph.
    if (upper_filepath.find("PRES_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            PressureConformParser>(filepath);
    }

	// Case for when the file is determined to be a Strain Energy Conform graph.
    if (upper_filepath.find("STRAIN_ENERGY_CONFORM") != std::string::npos)
    {
        return std::make_unique<
            StrainEnergyConformParser>(filepath);
    }

	// Case for when the file is determined to be a Ring Forces graph.
    if (upper_filepath.find("RING_FORCES") != std::string::npos)
    {
        return std::make_unique<
            RingForcesParser>(
                filepath);
    }

	// Case for when the file is determined to be a Surface Animation graph.
    if (upper_filepath.find("_CYCLE") != std::string::npos)
    {
        return std::make_unique<
            SurfaceAnimParser>(
                filepath);
    }

    return nullptr;
}