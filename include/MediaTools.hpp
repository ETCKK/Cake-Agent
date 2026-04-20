#pragma once
#include <string>
#include <vector>

namespace MediaTools {
    std::vector<unsigned char> readFile(const std::string& filePath);
    std::string encodeBase64(const std::vector<unsigned char>& data);
}