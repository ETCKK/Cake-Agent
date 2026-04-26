#pragma once

#include <string>
#include <vector>

#include "core/Types.hpp"

namespace OllamaClient {
    bool chat(const std::string& model, const std::vector<Message>& history, Message& reply, std::string& err);
}
