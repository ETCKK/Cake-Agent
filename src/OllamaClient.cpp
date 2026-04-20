#include "OllamaClient.hpp"
#include <curl/curl.h>

namespace OllamaClient
{
    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    bool chat(const std::string &model, std::vector<json> &history, std::string &err)
    {
        CURL *curl = curl_easy_init();
        std::string response;

        if (!curl)
        {
            err = "Failed to initialize CURL";
            return false;
        }

        json payload;
        payload["model"] = model;
        payload["stream"] = false;

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
        for (auto &msg : history)
        {
            json m = msg;
            
            if (!m.contains("content") || m["content"].is_null()) {
                m["content"] = "";
            }

            payload["messages"].push_back(m);
        }

        std::string data = payload.dump();
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

        auto status = curl_easy_perform(curl);
        if (status != CURLE_OK)
        {
            err = curl_easy_strerror(status);
            curl_easy_cleanup(curl);
            return false;
        }

        json res_json = json::parse(response);
        json reply = res_json.value("message", json::object());

        if (!reply.contains("content") || reply["content"].is_null()) {
            reply["content"] = "";
        }

        // Parse tool calls if Ollama accidentally sent them as strings
        if (reply.contains("tool_calls") && reply["tool_calls"].is_array())
        {
            for (auto &tc : reply["tool_calls"])
            {
                if (tc.contains("function") && tc["function"].contains("arguments"))
                {
                    auto &raw_args = tc["function"]["arguments"];
                    if (raw_args.is_string())
                    {
                        tc["function"]["arguments"] = json::parse(raw_args.get<std::string>());
                    }
                }
            }
        }
        
        history.push_back(reply);
        curl_easy_cleanup(curl);
        return true;
    }
}