#pragma once

#include <string>
#include <string_view>

namespace plotconv {
	/// True when "text" follows the JSON number grammar exactly.
	bool isJsonNumber(std::string_view text);

	/// Turns a number as written by the simulation program into text that
	/// is a valid JSON number with the same value:
	///   - "D" exponent marker
	///   - missing exponent letter
	///   - leading "+"
	///   - missing leading zero
	///   - decimal point without digits
	bool toJsonNumber(std::string_view raw, std::string& out);

	/// Converts a token already accepted by toJsonNumber() to a double.
	/// Used only for ranges and extremes, never for the values written to the JSON.
	/// Relies on the default "C" locale (the program never calls setlocale).
	double jsonNumberToDouble(const std::string& token);
}
