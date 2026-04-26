#pragma once

#include <string>
#include <vector>

namespace AssistantManager {
    void runAssistant();
    const std::string& getCurrentModel();
    const std::vector<std::string>& getValidModels();
    bool setCurrentModel(const std::string& model);
}
