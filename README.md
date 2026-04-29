# Cake-Agent

Cake-Agent is an AI assistant built in C++. It uses **Native Tool Calling** to interact with your Linux system through the Ollama API locally, or through the OpenAI API via an API token.

## Current Status

- [x] Use tool calling to execute commands
- [x] Support for loading other models
- [x] Support for multimodal history and attachments (only images for now)
- [ ] More tools to further allow the agent to operate
- [ ] Proper history management
- [ ] Proper GUI
- [ ] Audio input via speech-to-text
- [ ] Audio output via text-to-speech
- [ ] Support for Windows
- [ ] More?

## Dependencies

### Required Dependencies
- **CMake** for the build system
- **libcurl** for networking/API requests
- **nlohmann_json** for JSON parsing

### Optional Dependencies
- **Ollama** for the local LLM API
- A local model for using **Ollama**

## Install Dependencies (Linux)

### Install Required Dependencies
Install **CMake** and required libraries:
```bash
# Arch
sudo pacman -S cmake curl nlohmann-json

# Debian / Ubuntu
sudo apt install cmake libcurl4-openssl-dev nlohmann-json3-dev

# Fedora
sudo dnf install cmake libcurl-devel nlohmann-json-devel
```

### Install Optional Dependencies
Install **Ollama**:
(You can use your package manager if you prefer)
```bash
curl -fsSL https://ollama.com/install.sh | sh
```

Download a model:
```bash
ollama pull <model-name>
```

## Build & Run

Make sure that you have the required dependencies installed.

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

To run the agent in Linux, simply execute the binary in the build folder:
```bash
./build/cake-agent
```

## Usage

### Local Usage
To use the agent with a local model, you must have **Ollama** installed with a pulled model.

Start the agent:
```bash
./build/cake-agent
```

Then select one of the accepted models:
```text
/model                      // List accepted models
/model ollama:<model-name>  // Set the current model
```

### OpenAI Usage
Create a `.env` file in the project root with your OpenAI API key:
```bash
OPENAI_API_KEY=your_api_key_here
```

Start the agent:
```bash
./build/cake-agent
```

Then select one of the accepted models:
```text
/model                      // List accepted models
/model openai:<model-name>  // Set the current model
```

### Runtime Commands
- `/model` shows the current model and accepted model names.
- `/model <model_name>` switches to a supported model.
- `/image <path>` attaches an image to the next user message.
- `/exit` or `/quit` exits the program.

Keep in mind that the first replies on local LLMs may be delayed by ~30-60 seconds because of the model loading.