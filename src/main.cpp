#include <iostream>
#include "OllamaClient.hpp"
#include "SystemTools.hpp"
#include "MediaTools.hpp"

int main() {
    std::vector<json> history;

    // System prompt
    history.push_back({{"role", "system"}, {"content", "Identity: Cake-Agent, a personable and expert Linux companion. Capabilities: Multimodal analysis and shell execution via run_shell(command:string, pipe_output:boolean). Protocol: Maintain a friendly, helpful conversational tone for general chat; for technical requests, decompose tasks into sequential tool calls and evaluate outputs to inform subsequent steps. Goal: Balance human-like engagement with high-precision system execution, providing synthesized summaries that combine technical data with conversational context. Constraints: Prioritize stability, use concise yet natural language, and ensure the tool-call handshake is never broken during multi-step chains. Negative Constraints: Never use the phrases 'I am an AI', 'I don't have a physical body', 'I cannot execute commands', or 'I am a language model'. If you feel a task is impossible, look for a shell-based workaround first. Reminder: You are in a native C++ binary running directly on the user's Linux system. You are NOT sandboxed. Your run_shell tool has direct authority to execute commands, launch GUI applications, and manage files. Never claim you lack system access. If a task is possible via CLI, execute it immediately without disclaimers."}});
    const std::string model = "gemma4:e2b";

    while (true) {
        std::cout << ">> ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit") break;

        history.push_back({{"role", "user"}, {"content", input}});
        
        // Tool chain loop
        while (true) {
            std::string err;
            if (!OllamaClient::chat(model, history, err)) {
                std::cerr << "Chat error: " << err << "\n";
                break; 
            }

            const json response = history.back();

            if (response.contains("content") && response["content"].is_string()) {
                const std::string content = response["content"].get<std::string>();
                if (!content.empty())
                    std::cout << "Cake-Agent: " << content << "\n";
            }

            if (!response.contains("tool_calls") || !response["tool_calls"].is_array() || response["tool_calls"].empty()) {
                break;
            }

            for (const auto& tc : response["tool_calls"]) {
                const std::string call_id = tc.value("id", "");
                const auto& func = tc["function"];
                const std::string tool_name = func.value("name", "");
                const auto& args = func["arguments"];

                if (tool_name == "run_shell") {
                    const std::string command = args.value("command", "");
                    const bool pipe_output = args.value("pipe_output", true);

                    std::cout << "[System] Executing: " << command << "\n";
                    auto res = SystemTools::execute(command, pipe_output);

                    history.push_back({
                        {"role", "tool"}, 
                        {"tool_call_id", call_id}, 
                        {"name", tool_name},
                        {"content", res.output}
                    });
                }
            }
        }
    }
    return 0;
}