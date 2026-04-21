#include "OpenAIClient.hpp"
#include <curl/curl.h>
#include <cstdlib>

namespace OpenAIClient
{
    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    bool chat(const std::string &model, std::vector<Message> &history, std::string &err)
    {
        CURL *curl = curl_easy_init();
        std::string response;
        struct curl_slist *headers = nullptr;

        if (!curl)
        {
            err = "Failed to initialize CURL";
            return false;
        }

        const char *api_key = std::getenv("OPENAI_API_KEY");
        if (!api_key || std::string(api_key).empty())
        {
            err = "OPENAI_API_KEY is not set";
            curl_easy_cleanup(curl);
            return false;
        }

        json payload;
        payload["model"] = model;
        payload["stream"] = false;
        payload["tool_choice"] = "auto";

        // Define the Tools
        payload["tools"] = {{
            {"type", "function"},
            {"function", {
                {"name", "run_shell"}, 
                {"description", "Execute a Linux command."}, 
                {"parameters", {
                    {"type", "object"}, 
                    {"properties", {
                        {"command", {{"type", "string"}}}, 
                        {"pipe_output", {{"type", "boolean"}}}
                    }}, 
                    {"required", {"command", "pipe_output"}}
                }}
            }}
        }};

        // Build the messages payload
        for (const auto &msg : history)
        {
            json m = msg;
            
            if (!m.contains("content") || m["content"].is_null()) {
                m["content"] = "";
            }

            // OpenAI expects tool call function arguments as JSON strings.
            if (m.contains("tool_calls") && m["tool_calls"].is_array()) {
                for (auto &tc : m["tool_calls"]) {
                    if (tc.contains("function") && tc["function"].contains("arguments")) {
                        auto &args = tc["function"]["arguments"];
                        if (!args.is_string()) {
                            args = args.dump();
                        }
                    }
                }
            }

            payload["messages"].push_back(m);
        }

        std::string data = payload.dump();
        std::string auth_header = std::string("Authorization: Bearer ") + api_key;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

        auto status = curl_easy_perform(curl);
        if (status != CURLE_OK)
        {
            err = curl_easy_strerror(status);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }

        json res_json = json::parse(response, nullptr, false);
        if (res_json.is_discarded())
        {
            err = "OpenAI returned invalid JSON";
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }

        if (res_json.contains("error"))
        {
            err = res_json["error"].value("message", "OpenAI API error");
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }

        if (!res_json.contains("choices") || !res_json["choices"].is_array() || res_json["choices"].empty())
        {
            err = "OpenAI response missing choices";
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }

        json reply = res_json["choices"][0].value("message", json::object());

        if (!reply.contains("content") || reply["content"].is_null()) {
            reply["content"] = "";
        }

        // Convert OpenAI tool call arguments into JSON objects for local handling.
        if (reply.contains("tool_calls") && reply["tool_calls"].is_array())
        {
            for (auto &tc : reply["tool_calls"])
            {
                if (tc.contains("function") && tc["function"].contains("arguments"))
                {
                    auto &raw_args = tc["function"]["arguments"];
                    if (raw_args.is_string())
                    {
                        json parsed_args = json::parse(raw_args.get<std::string>(), nullptr, false);
                        if (!parsed_args.is_discarded()) {
                            tc["function"]["arguments"] = parsed_args;
                        }
                    }
                }
            }
        }
        
        history.push_back(reply.get<Message>());
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return true;
    }
}