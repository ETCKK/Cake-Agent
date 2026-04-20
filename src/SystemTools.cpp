#include "SystemTools.hpp"
#include <cstdio>
#include <cstdlib>

namespace SystemTools {
    ExecResult execute(const std::string& command, bool pipe_output) {
        ExecResult res;
        if (!pipe_output) {
            std::string bg = "nohup " + command + " > /dev/null 2>&1 &";
            res.exit_code = std::system(bg.c_str());
            res.output = "[Background Process Started]";
            return res;
        }

        FILE* pipe = popen((command + " 2>&1").c_str(), "r");
        if (!pipe) return {-1, "Pipe Error"};
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe)) res.output += buf;
        res.exit_code = pclose(pipe);
        return res;
    }
}