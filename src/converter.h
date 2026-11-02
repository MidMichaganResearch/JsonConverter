#pragma once

#include <filesystem>

#include "core/conversion_result.h"

namespace plotconv {
	/// Converts one raw simulation file into Plotly JSON.
	///
	/// The file type is chosen from the file name; the parameters
	/// (node count, conformance flag...) are read from the header.
	ConversionResult convertFile(const std::filesystem::path& inputPath);
}
