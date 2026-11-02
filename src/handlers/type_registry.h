#pragma once

#include <memory>
#include <vector>

#include "handlers/type_handler.h"

namespace plotconv {
	/// Every supported file type, in no particular order (a file must match exactly one).
	const std::vector<std::unique_ptr<TypeHandler>>& registeredHandlers();
}
