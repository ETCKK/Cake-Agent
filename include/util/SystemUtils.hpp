#pragma once

#include <string>

#include "core/Types.hpp"

namespace SystemUtils {
    ExecResult execute(const std::string& command, bool pipe_output);
}
