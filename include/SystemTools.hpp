#pragma once
#include <string>
#include "Types.hpp"

namespace SystemTools {
    ExecResult execute(const std::string& command, bool pipe_output);
}