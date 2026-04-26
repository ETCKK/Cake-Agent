#include "core/InputManager.hpp"
#include "core/AssistantManager.hpp"
#include "core/HistoryManager.hpp"
#include "util/MediaUtils.hpp"

#include <iostream>
#include <string>

namespace InputManager {

bool input(const std::string& raw_input) {
    if (raw_input.empty()) {
        return true;
    }

    if (raw_input == "/exit" || raw_input == "/quit") {
        return false;
    }

    if (raw_input == "/model") {
        std::cout << "Current model: " << AssistantManager::getCurrentModel() << "\n";
        std::cout << "Accepted models:\n";
        for (const auto& model : AssistantManager::getValidModels()) {
            std::cout << "- " << model << "\n";
        }
        return true;
    }

    const std::string model_prefix = "/model ";
    if (raw_input.rfind(model_prefix, 0) == 0) {
        const std::string model_name = raw_input.substr(model_prefix.size());
        if (model_name.empty()) {
            std::cerr << "Usage: /model <model_name>\n";
            return true;
        }

        if (!AssistantManager::setCurrentModel(model_name)) {
            std::cerr << "Unsupported model: " << model_name << "\n";
            std::cerr << "Accepted models:\n";
            for (const auto& model : AssistantManager::getValidModels()) {
                std::cerr << "- " << model << "\n";
            }
            return true;
        }

        std::cout << "Model switched to: " << AssistantManager::getCurrentModel() << "\n";
        return true;
    }

    if (AssistantManager::getCurrentModel() == "None") {
        std::cerr << "No model selected. Use /model <model_name> to select a model.\n";
        return true;
    }

    const std::string image_prefix = "/image ";
    if (raw_input.rfind(image_prefix, 0) == 0) {
        const std::string image_path = raw_input.substr(image_prefix.size());
        if (image_path.empty()) {
            return true;
        }

        const auto data = MediaUtils::readFile(image_path);
        if (data.empty()) {
            std::cerr << "Failed to read image: " << image_path << "\n";
            return true;
        }

        HistoryManager::attachImage(MediaUtils::encodeBase64(data));
        std::cout << "Attached image: " << image_path << "\n";
        return true;
    }

    HistoryManager::addUserMessage(raw_input);

    return true;
}

} // namespace InputManager
