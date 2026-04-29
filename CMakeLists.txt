cmake_minimum_required(VERSION 3.10)
project(Cake-Agent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(include)

add_executable(cake-agent 
    src/main.cpp 
    src/core/InputManager.cpp
    src/core/HistoryManager.cpp
    src/core/AssistantManager.cpp
    src/core/ToolManager.cpp
    src/util/MediaUtils.cpp
    src/util/SystemUtils.cpp
    src/client/OllamaClient.cpp
    src/client/OpenAIClient.cpp
)

target_link_libraries(cake-agent PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)