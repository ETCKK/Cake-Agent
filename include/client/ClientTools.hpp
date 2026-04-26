#pragma once

#include <nlohmann/json.hpp>

namespace ClientTools {

inline nlohmann::json run_shell() {
	return nlohmann::json{
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
				{"required", nlohmann::json::array({"command", "pipe_output"})}
			}}
		}}
	};
}

inline nlohmann::json tools() {
	return nlohmann::json::array({run_shell()});
}

} // namespace ClientTools
