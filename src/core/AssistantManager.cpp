#include "core/AssistantManager.hpp"
#include "client/OllamaClient.hpp"
#include "client/OpenAIClient.hpp"
#include "core/HistoryManager.hpp"
#include "core/ToolManager.hpp"
#include "core/Types.hpp"
#include "util/TerminalUtils.hpp"
#include <iostream>
#include <algorithm>

namespace
{

    std::string current_model = "None";
    const std::vector<std::string> VALID_MODELS = {
        "openai:gpt-4o-mini",
        "ollama:gemma4:e2b",
        "ollama:gemma4:e4b"};

    bool chat(const std::string &model, Message &reply, std::string &err)
    {
        const std::string openai_prefix = "openai:";
        const std::string ollama_prefix = "ollama:";

        if (model.rfind(openai_prefix, 0) == 0)
        {
            const std::string openai_model = model.substr(openai_prefix.size());
            return OpenAIClient::chat(openai_model, HistoryManager::getHistory(), reply, err);
        }

        if (model.rfind(ollama_prefix, 0) == 0)
        {
            const std::string ollama_model = model.substr(ollama_prefix.size());
            return OllamaClient::chat(ollama_model, HistoryManager::getHistory(), reply, err);
        }

        return false;
    }

    void chainAssistant(const std::string &model)
    {
        while (true)
        {
            Message reply;
            std::string err;

            TerminalUtils::startSpinner("Thinking...");

            bool success = chat(model, reply, err);

            TerminalUtils::stopSpinner();

            if (!success)
            {
                std::cerr << "Chat error: " << err << "\n";
                return;
            }

            if (reply.tool_calls.empty())
            {
                return;
            }

            ToolManager::executeToolCalls(reply.tool_calls);
        }
    }
}

namespace AssistantManager
{

    const std::vector<std::string> &getValidModels()
    {
        return VALID_MODELS;
    }

    const std::string &getCurrentModel()
    {
        return current_model;
    }

    bool setCurrentModel(const std::string &model)
    {
        const auto it = std::find(VALID_MODELS.begin(), VALID_MODELS.end(), model);
        if (it == VALID_MODELS.end())
        {
            return false;
        }

        current_model = model;
        return true;
    }

    void runAssistant()
    {
        const auto &history = HistoryManager::getHistory();
        if (!history.empty() && history.back().role == "user")
        {
            chainAssistant(current_model);

            const auto &updated_history = HistoryManager::getHistory();
            if (!updated_history.empty())
            {
                const Message &last_message = updated_history.back();
                if (last_message.role == "assistant" && !last_message.content.empty())
                {
                    std::cout << "Cake-Agent" << ": " << last_message.content << "\n";
                }
            }
        }
    }

}