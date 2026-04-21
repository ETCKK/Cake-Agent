#pragma once
#include <string>
#include <vector>
#include "Types.hpp"

namespace OpenAIClient {
    bool chat(const std::string& model, std::vector<Message>& history, std::string& err);
}