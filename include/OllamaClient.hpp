#pragma once
#include <string>
#include <vector>
#include "Types.hpp"

namespace OllamaClient {
    bool chat(const std::string& model, std::vector<json>& history, std::string& err);
}