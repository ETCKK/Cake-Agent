#pragma once

#include <vector>
#include "core/Types.hpp"

namespace ToolManager {
    void executeToolCalls(const std::vector<ToolCall>& tool_calls);
}