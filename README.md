# Cake-Agent

Cake-Agent is a local-first AI assistant built in C++. It uses **Native Tool Calling** to interact with your Linux system through the Ollama API.

## Current Status

- [x] Use tool calling to execute commands
- [ ] Support for loading other models
- [ ] Support for multimodal history and attachments (images and audio)
- [ ] Support for Windows
- [ ] Maybe much more?

## Required Dependencies

- **CMake** for the build system
- **Ollama** for the local LLM API
- For now, specifically the **Gemma4 E2B** model
- **libcurl** for networking/API requests
- **nlohmann_json** for JSON parsing

### Install Dependencies (Linux)

Install **CMake** and required libraries:
```bash
# Arch
sudo pacman -S cmake curl nlohmann-json

# Debian / Ubuntu
sudo apt install cmake libcurl4-openssl-dev nlohmann-json3-dev

# Fedora
sudo dnf install cmake libcurl-devel nlohmann-json-devel
```

Install **Ollama**:
(You can use your package manager if you prefer)
```bash
curl -fsSL https://ollama.com/install.sh | sh
```

Download the model:
```bash
ollama pull gemma4:e2b
```

## Build

Make sure you have the required dependencies installed.

Clone the project using:
```bash
git clone https://github.com/ETCKK/Cake-Agent.git
cd Cake-Agent
```

Build with the following command:
```bash
cmake -S . -B build
cmake --build build -j
```

## Run

In Linux, simply execute the binary under "/build":
```bash
./build/cake-agent
```

Keep in mind that the first reply may be delayed by ~30-60 seconds because of the model loading.