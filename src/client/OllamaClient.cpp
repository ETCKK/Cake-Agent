#include "client/OllamaClient.hpp"
#include "core/HistoryManager.hpp"
#include "core/ToolManager.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace
{
    using json = nlohmann::json;
    constexpr const char *OLLAMA_CHAT_URL = "http://localhost:11434/api/chat";

    size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        auto *buffer = static_cast<std::string *>(userp);
        buffer->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
    }

    json toolArgumentsForOllama(const std::string &arguments)
    {
        if (arguments.empty())
        {
            return json::object();
        }

        json parsed = json::parse(arguments, nullptr, false);
        if (!parsed.is_discarded())
        {
            return parsed;
        }

        return arguments;
    }

    json messageToOllama(const Message &message)
    {
        json payload;
        payload["role"] = message.role;
        payload["content"] = message.content;

        if (!message.name.empty())
        {
            payload["name"] = message.name;
        }

        if (!message.tool_call_id.empty())
        {
            payload["tool_call_id"] = message.tool_call_id;
        }

        if (!message.images.empty())
        {
            payload["images"] = message.images;
        }

        if (!message.tool_calls.empty())
        {
            json tool_calls = json::array();
            for (const auto &tool_call : message.tool_calls)
            {
                tool_calls.push_back({{"id", tool_call.id},
                                      {"type", "function"},
                                      {"function", {{"name", tool_call.name}, {"arguments", toolArgumentsForOllama(tool_call.arguments)}}}});
            }
            payload["tool_calls"] = tool_calls;
        }

        return payload;
    }

    Message ollamaToMessage(const json &response_message)
    {
        Message reply;
        reply.role = response_message.value("role", "assistant");
        if (response_message.contains("content"))
        {
            if (response_message["content"].is_string())
            {
                reply.content = response_message["content"].get<std::string>();
            }
            else
            {
                reply.content = response_message["content"].dump();
            }
        }
        reply.name = response_message.value("name", "");
        reply.tool_call_id = response_message.value("tool_call_id", "");

        if (response_message.contains("tool_calls") && response_message["tool_calls"].is_array())
        {
            for (const auto &tc : response_message["tool_calls"])
            {
                ToolCall tool_call;
                tool_call.id = tc.value("id", "");
                if (tc.contains("function"))
                {
                    const json function_data = tc["function"];
                    tool_call.name = function_data.value("name", "");
                    if (function_data.contains("arguments"))
                    {
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

    json buildPayload(const std::string &model, const std::vector<Message> &history)
    {
        json payload;
        payload["model"] = model;
        payload["stream"] = false;
        payload["tools"] = ToolManager::tools();
        payload["messages"] = json::array();

        for (const auto &msg : history)
        {
            json message = messageToOllama(msg);
            payload["messages"].push_back(message);
        }

        return payload;
    }

    bool parseResponse(const std::string &response, Message &reply, std::string &err)
    {
        json parsed_response = json::parse(response, nullptr, false);
        if (parsed_response.is_discarded())
        {
            err = "Ollama returned invalid JSON";
            return false;
        }

        if (parsed_response.contains("error"))
        {
            err = parsed_response["error"].value("message", "Ollama API error");
            return false;
        }

        json message = parsed_response.value("message", json::object());
        if (!message.contains("content") || message["content"].is_null())
        {
            message["content"] = "";
        }

        reply = ollamaToMessage(message);
        return true;
    }
}

namespace OllamaClient
{

    bool chat(const std::string &model, const std::vector<Message> &history, Message &reply, std::string &err)
    {
        CURL *curl = curl_easy_init();
        std::string response;

        if (!curl)
        {
            err = "Failed to initialize CURL";
            return false;
        }

        const std::string payload = buildPayload(model, history).dump();

        curl_easy_setopt(curl, CURLOPT_URL, OLLAMA_CHAT_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

        const CURLcode status = curl_easy_perform(curl);
        if (status != CURLE_OK)
        {
            err = curl_easy_strerror(status);
            curl_easy_cleanup(curl);
            return false;
        }

        const bool ok = parseResponse(response, reply, err);
        if (ok)
        {
            HistoryManager::addAssistantMessage(reply);
        }
        curl_easy_cleanup(curl);
        return ok;
    }

}
