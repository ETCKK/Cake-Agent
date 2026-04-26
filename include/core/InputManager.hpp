#pragma once

#include <string>

namespace InputManager {

// Returns false on /quit or /exit, true otherwise.
bool input(const std::string& raw_input);

} // namespace InputManager
