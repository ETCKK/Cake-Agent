#pragma once

#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>

namespace EnvUtils
{

    inline void trim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                        { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                             { return !std::isspace(ch); })
                    .base(),
                s.end());
    }

    inline void load(const std::string &filename = ".env")
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            trim(line);

            if (line.empty() || line[0] == '#')
                continue;

            auto pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                trim(key);
                trim(value);

                if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                          (value.front() == '\'' && value.back() == '\'')))
                {
                    value = value.substr(1, value.size() - 2);
                }

                setenv(key.c_str(), value.c_str(), 1);
            }
        }
    }
}