#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ExecResult {
    int exit_code = -1;
    std::string output;
};