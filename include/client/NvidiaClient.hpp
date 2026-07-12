#pragma once

#include <string>
#include <vector>

#include "core/Types.hpp"

namespace NvidiaClient
{
    bool chat(const std::string &model, const std::vector<Message> &history, Message &reply, std::string &err);
}
