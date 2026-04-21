#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

struct ExecResult {
    int exit_code = -1;
    std::string output;
};

struct Message {
    std::string role;
    json content = "";
    std::string name;
    std::string tool_call_id;
    json metadata = nullptr;
    json tool_calls = json::array();

    Message() = default;
    Message(std::string r, json c) : role(std::move(r)), content(std::move(c)) {}
};

inline void to_json(json& j, const Message& m) {
    j = {
        {"role", m.role},
        {"content", m.content.is_null() ? json("") : m.content}
    };

    if (!m.name.empty()) {
        j["name"] = m.name;
    }
    if (!m.tool_call_id.empty()) {
        j["tool_call_id"] = m.tool_call_id;
    }
    if (!m.metadata.is_null()) {
        j["metadata"] = m.metadata;
    }
    if (m.tool_calls.is_array() && !m.tool_calls.empty()) {
        j["tool_calls"] = m.tool_calls;
    }
}

inline void from_json(const json& j, Message& m) {
    m.role = j.value("role", "");
    if (j.contains("content")) {
        m.content = j["content"];
    } else {
        m.content = "";
    }

    m.name = j.value("name", "");
    m.tool_call_id = j.value("tool_call_id", "");
    m.metadata = j.contains("metadata") ? j["metadata"] : json(nullptr);
    m.tool_calls = j.contains("tool_calls") ? j["tool_calls"] : json::array();
}