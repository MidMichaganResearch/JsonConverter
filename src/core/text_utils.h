#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace plotconv {
	/// Removes leading and trailing whitespace.
	std::string_view trim(std::string_view text);

	/// ASCII upper-case copy (file names and headers are plain ASCII).
	std::string toUpper(std::string_view text);

	bool startsWith(std::string_view text, std::string_view prefix);

	/// Splits a line into tokens separated by spaces, tabs or commas.
	/// Empty tokens are skipped, so a trailing comma is harmless.
	/// The tokens point into `line`, which must outlive them.
	void splitTokens(std::string_view line, std::vector<std::string_view>& tokens);

	/// Splits on commas only, so names such as "TIMESTEP (s)" stay in one piece.
	/// Every piece is trimmed.
	std::vector<std::string> splitOnCommas(std::string_view line);

	std::string toTitleCase(std::string_view text);
	std::string labelWithUnits(std::string_view label, std::string_view units);
}
