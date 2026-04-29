#include "core/HistoryManager.hpp"

#include <utility>

namespace {
std::vector<Message> history;
std::vector<std::string> attached_images;

Message buildUserMessage(const std::string& text) {
    Message user_message{"user", text};
    user_message.images = attached_images;
    return user_message;
}

void clearAttachedImages() {
    attached_images.clear();
}
}

namespace HistoryManager {

void init(const std::string& system_prompt) {
    history.clear();
    clearAttachedImages();
    history.push_back(Message{"system", system_prompt});
}

void attachImage(const std::string& image_base64) {
    attached_images.push_back(image_base64);
}

void addUserMessage(const std::string& text) {
    history.push_back(buildUserMessage(text));
    clearAttachedImages();
}

void addAssistantMessage(const Message& message) {
    history.push_back(message);
}

void addToolResult(const std::string& tool_call_id, const std::string& tool_name, const ExecResult& result) {
    Message tool_message{"tool", result.output};
    tool_message.tool_call_id = tool_call_id;
    tool_message.name = tool_name;
    tool_message.exit_code = result.exit_code;
    history.push_back(std::move(tool_message));
}

const std::vector<Message>& getHistory() {
    return history;
}

}
