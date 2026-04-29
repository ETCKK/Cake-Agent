#include "core/HistoryManager.hpp"
#include "core/InputManager.hpp"
#include "core/AssistantManager.hpp"
#include "util/EnvUtils.hpp"

#include <iostream>
#include <string>

int main() {
    EnvUtils::load();
    HistoryManager::init();
    
    std::cout << "Cake-Agent started. Use /model to see available models" << "\n";

    while (true) {
        std::cout << ">> ";

        std::string raw_input;
        std::getline(std::cin, raw_input);

        if (!InputManager::input(raw_input)) {
            return 0;
        }

        AssistantManager::runAssistant();
    }
}