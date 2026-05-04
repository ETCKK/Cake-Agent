#pragma once

#include <string>
#include <vector>

#include "core/Types.hpp"

namespace HistoryManager
{
    void init(const std::string &system_prompt = kDefaultSystemPrompt);

    void attachImage(const std::string &image_base64);
    void addUserMessage(const std::string &text);

    void addAssistantMessage(const Message &message);
    void addToolResult(const std::string &tool_call_id, const std::string &tool_name, const ExecResult &result);

    const std::vector<Message> &getHistory();
}
