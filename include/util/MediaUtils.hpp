#pragma once

#include <string>
#include <vector>

namespace MediaUtils {
    std::vector<unsigned char> readFile(const std::string& file_path);
    std::string encodeBase64(const std::vector<unsigned char>& data);
}
