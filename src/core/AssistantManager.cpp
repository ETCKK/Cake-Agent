#include "core/AssistantManager.hpp"
#include "client/OllamaClient.hpp"
#include "client/OpenAIClient.hpp"
#include "core/HistoryManager.hpp"
#include "core/Types.hpp"
#include "util/SystemUtils.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <algorithm>

namespace {
using json = nlohmann::json;

std::string current_model = "None";
const std::vector<std::string> VALID_MODELS = {
    "openai:gpt-4o-mini",
    "ollama:gemma4:e2b",
    "ollama:gemma4:e4b"
};

json parseFunctionArguments(const std::string& raw_args) {
    if (raw_args.empty()) {
        return json::object();
    }

    json parsed = json::parse(raw_args, nullptr, false);
    if (parsed.is_object()) {
        return parsed;
    }

    return json::object();
}

bool chat(const std::string& model, Message& reply, std::string& err) {
    const std::string openai_prefix = "openai:";
    const std::string ollama_prefix = "ollama:";

    if (model.rfind(openai_prefix, 0) == 0) {
        const std::string openai_model = model.substr(openai_prefix.size());
        return OpenAIClient::chat(openai_model, HistoryManager::getHistory(), reply, err);
    }

    if (model.rfind(ollama_prefix, 0) == 0) {
        const std::string ollama_model = model.substr(ollama_prefix.size());
        return OllamaClient::chat(ollama_model, HistoryManager::getHistory(), reply, err);
    }

    return false;
}

void executeToolCalls(const std::vector<ToolCall>& tool_calls) {

    for (const auto& tc : tool_calls) {
        const std::string call_id = tc.id;
        const std::string tool_name = tc.name;

        if (tool_name != "run_shell") {
            continue;
        }

        const json args = parseFunctionArguments(tc.arguments);
        const std::string command = args.value("command", "");
        const bool pipe_output = args.value("pipe_output", true);

        if (command.empty()) {
            ExecResult invalid_result;
            invalid_result.exit_code = 2;
            invalid_result.output = "Tool call is missing a valid command argument.";
            HistoryManager::addToolResult(call_id, tool_name, invalid_result);
            continue;
        }

        std::cout << "[System] Executing: " << command << "\n";
        const ExecResult result = SystemUtils::execute(command, pipe_output);
        HistoryManager::addToolResult(call_id, tool_name, result);
    }
}

void chainAssistant(const std::string& model) {
    while (true) {
        Message reply;
        std::string err;
        if (!chat(model, reply, err)) {
            std::cerr << "Chat error: " << err << "\n";
            return;
        }

        if (reply.tool_calls.empty()) {
            return;
        }

        executeToolCalls(reply.tool_calls);
    }
}
} // namespace

namespace AssistantManager {

const std::vector<std::string>& getValidModels() {
    return VALID_MODELS;
}

const std::string& getCurrentModel() {
    return current_model;
}

bool setCurrentModel(const std::string& model) {
    const auto it = std::find(VALID_MODELS.begin(), VALID_MODELS.end(), model);
    if (it == VALID_MODELS.end()) {
        return false;
    }

    current_model = model;
    return true;
}

void runAssistant() {
    const auto& history = HistoryManager::getHistory();
        if (!history.empty() && history.back().role == "user") {
            chainAssistant(current_model);

            const auto& updated_history = HistoryManager::getHistory();
            if (!updated_history.empty()) {
                const Message& last_message = updated_history.back();
                if (last_message.role == "assistant" && !last_message.content.empty()) {
                    std::cout << "Cake-Agent" << ": " << last_message.content << "\n";
                }
            }
        }
}

}