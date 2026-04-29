#include "client/OpenAIClient.hpp"
#include "client/ClientTools.hpp"
#include "core/HistoryManager.hpp"

#include <curl/curl.h>
#include <cstdlib>
#include <nlohmann/json.hpp>

namespace {
using json = nlohmann::json;
constexpr const char* OPENAI_CHAT_URL = "https://api.openai.com/v1/chat/completions";

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

json messageToOpenAI(const Message& message) {
    json payload;
    payload["role"] = message.role;

    if (!message.name.empty()) {
        payload["name"] = message.name;
    }

    if (!message.tool_call_id.empty()) {
        payload["tool_call_id"] = message.tool_call_id;
    }

    if (!message.tool_calls.empty()) {
        json tool_calls = json::array();
        for (const auto& tool_call : message.tool_calls) {
            tool_calls.push_back({
                {"id", tool_call.id},
                {"type", "function"},
                {"function", {
                    {"name", tool_call.name},
                    {"arguments", tool_call.arguments}
                }}
            });
        }
        payload["tool_calls"] = tool_calls;
    }

    if (!message.images.empty()) {
        json parts = json::array();
        if (!message.content.empty()) {
            parts.push_back({
                {"type", "text"},
                {"text", message.content}
            });
        }

        for (const auto& image_base64 : message.images) {
            parts.push_back({
                {"type", "image_url"},
                {"image_url", {
                    {"url", std::string("data:image/*;base64,") + image_base64}
                }}
            });
        }

        payload["content"] = parts;
    } else {
        payload["content"] = message.content;
    }

    return payload;
}


Message openAIToMessage(const json& response_message) {
    Message reply;
    reply.role = response_message.value("role", "assistant");
    if (response_message.contains("content")) {
        if (response_message["content"].is_string()) {
            reply.content = response_message["content"].get<std::string>();
        } else {
            reply.content = response_message["content"].dump();
        }
    }
    reply.name = response_message.value("name", "");
    reply.tool_call_id = response_message.value("tool_call_id", "");

    if (response_message.contains("tool_calls") && response_message["tool_calls"].is_array()) {
        for (const auto& tc : response_message["tool_calls"]) {
            ToolCall tool_call;
            tool_call.id = tc.value("id", "");
            if (tc.contains("function")) {
                const json function_data = tc["function"];
                tool_call.name = function_data.value("name", "");
                if (function_data.contains("arguments")) {
                    tool_call.arguments = function_data["arguments"].is_string()
                        ? function_data["arguments"].get<std::string>()
                        : function_data["arguments"].dump();
                }
            }
            reply.tool_calls.push_back(tool_call);
        }
    }

    return reply;
}

json buildPayload(const std::string& model, const std::vector<Message>& history) {
    json payload;
    payload["model"] = model;
    payload["stream"] = false;
    payload["tool_choice"] = "auto";
    payload["tools"] = ClientTools::tools();
    payload["messages"] = json::array();

    for (const auto& msg : history) {
        json message = messageToOpenAI(msg);
        payload["messages"].push_back(message);
    }

    return payload;
}

bool parseResponse(const std::string& response, Message& reply, std::string& err) {
    json parsed_response = json::parse(response, nullptr, false);
    if (parsed_response.is_discarded()) {
        err = "OpenAI returned invalid JSON";
        return false;
    }

    if (parsed_response.contains("error")) {
        err = parsed_response["error"].value("message", "OpenAI API error");
        return false;
    }

    if (!parsed_response.contains("choices") || !parsed_response["choices"].is_array() || parsed_response["choices"].empty()) {
        err = "OpenAI response missing choices";
        return false;
    }

    json message = parsed_response["choices"][0].value("message", json::object());
    if (!message.contains("content") || message["content"].is_null()) {
        message["content"] = "";
    }

    reply = openAIToMessage(message);
    return true;
}
}

namespace OpenAIClient {

bool chat(const std::string& model, const std::vector<Message>& history, Message& reply, std::string& err) {
    CURL* curl = curl_easy_init();
    std::string response;
    curl_slist* headers = nullptr;

    if (!curl) {
        err = "Failed to initialize CURL";
        return false;
    }

    const char* api_key = std::getenv("OPENAI_API_KEY");
    if (!api_key || std::string(api_key).empty()) {
        err = "OPENAI_API_KEY is not set";
        curl_easy_cleanup(curl);
        return false;
    }

    const std::string payload = buildPayload(model, history).dump();
    const std::string auth_header = std::string("Authorization: Bearer ") + api_key;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_CHAT_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    const CURLcode status = curl_easy_perform(curl);
    if (status != CURLE_OK) {
        err = curl_easy_strerror(status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    const bool ok = parseResponse(response, reply, err);
    if (ok) {
        HistoryManager::addAssistantMessage(reply);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return ok;
}

}
