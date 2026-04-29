#include "util/SystemUtils.hpp"

#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>

namespace SystemUtils {

ExecResult execute(const std::string& command, const bool pipe_output) {
    ExecResult result;

    if (!pipe_output) {
        const std::string background_command = "nohup " + command + " > /dev/null 2>&1 &";
        result.exit_code = std::system(background_command.c_str());
        result.output = "[Background Process Started]";
        return result;
    }

    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        return {-1, "Pipe Error"};
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result.output += buffer;
    }

    const int status = pclose(pipe);
    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else {
        result.exit_code = status;
    }

    return result;
}

}
