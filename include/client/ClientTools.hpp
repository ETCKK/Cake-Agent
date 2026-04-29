#pragma once

#include <nlohmann/json.hpp>

namespace ClientTools {

inline nlohmann::json run_shell() {
    return nlohmann::json{
        {"type", "function"},
        {"function", {
            {"name", "run_shell"},
            {"description", "Execute a Linux command."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"command", {{"type", "string"}}},
                    {"pipe_output", {{"type", "boolean"}, {"description", "Whether to capture and return the command's output. Don't set this to true if the command doesn't terminate after execution to avoid hanging the assistant."}}}
                }},
                {"required", nlohmann::json::array({"command", "pipe_output"})}
            }}
        }}
    };
}

inline nlohmann::json read_file() {
    return nlohmann::json{
        {"type", "function"},
        {"function", {
            {"name", "read_file"},
            {"description", "Read the raw contents of a file directly. Prefer this over 'cat' for reliability."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"path", {{"type", "string"}, {"description", "Absolute or relative path to the file"}}}
                }},
                {"required", nlohmann::json::array({"path"})}
            }}
        }}
    };
}

inline nlohmann::json write_file() {
    return nlohmann::json{
        {"type", "function"},
        {"function", {
            {"name", "write_file"},
            {"description", "Write text directly to a file, overwriting it. Use this instead of 'echo' to avoid shell escaping issues."},
            {"parameters", {
                {"type", "object"},
                {"properties", {
                    {"path", {{"type", "string"}}},
                    {"content", {{"type", "string"}, {"description", "The exact content to write into the file"}}}
                }},
                {"required", nlohmann::json::array({"path", "content"})}
            }}
        }}
    };
}

inline nlohmann::json tools() {
    return nlohmann::json::array({run_shell(), read_file(), write_file()});
}

}