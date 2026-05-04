#include "core/ToolManager.hpp"
#include "core/HistoryManager.hpp"
#include "core/Types.hpp"
#include "util/SystemUtils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace
{
    using json = nlohmann::json;

    json parseArguments(const std::string &raw_args)
    {
        if (raw_args.empty())
        {
            return json::object();
        }

        json parsed = json::parse(raw_args, nullptr, false);
        if (parsed.is_object())
        {
            return parsed;
        }

        return json::object();
    }

    void executeToolCall(const ToolCall &tool_call)
    {
        const std::string call_id = tool_call.id;
        const std::string tool_name = tool_call.name;
        const json args = parseArguments(tool_call.arguments);

        if (tool_name == "run_shell")
        {
            const std::string command = args.value("command", "");
            const bool pipe_output = args.value("pipe_output", true);

            if (command.empty())
            {
                ExecResult invalid_result{2, "Tool call is missing a valid command argument."};
                HistoryManager::addToolResult(call_id, tool_name, invalid_result);
                return;
            }

            std::cout << "[System] Executing: " << command << "\n";
            const ExecResult result = SystemUtils::execute(command, pipe_output);
            HistoryManager::addToolResult(call_id, tool_name, result);
        }
        else if (tool_name == "read_file")
        {
            const std::string path = args.value("path", "");
            ExecResult result;

            std::ifstream file(path);
            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                result.exit_code = 0;
                result.output = buffer.str();
            }
            else
            {
                result.exit_code = 1;
                result.output = "Error: Could not open file for reading: " + path;
            }

            std::cout << "[System] Read file: " << path << "\n";
            HistoryManager::addToolResult(call_id, tool_name, result);
        }
        else if (tool_name == "write_file")
        {
            const std::string path = args.value("path", "");
            const std::string content = args.value("content", "");
            ExecResult result;

            std::ofstream file(path);
            if (file.is_open())
            {
                file << content;
                result.exit_code = 0;
                result.output = "Successfully wrote " + std::to_string(content.size()) + " bytes to " + path;
            }
            else
            {
                result.exit_code = 1;
                result.output = "Error: Could not open file for writing: " + path;
            }

            std::cout << "[System] Wrote file: " << path << "\n";
            HistoryManager::addToolResult(call_id, tool_name, result);
        }
    }
}

namespace ToolManager
{

    void executeToolCalls(const std::vector<ToolCall> &tool_calls)
    {

        for (const auto &tc : tool_calls)
        {
            executeToolCall(tc);
        }
    }

}