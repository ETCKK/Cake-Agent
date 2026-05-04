#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

namespace TerminalUtils
{
    // Controls the background thread
    inline std::atomic<bool> is_thinking{false};

    inline void startSpinner(const std::string &message = "Cake-Agent is thinking")
    {
        is_thinking = true;

        // Launch a background thread for the animation
        std::thread spinner_thread([message]()
                                   {
            const std::vector<std::string> frames = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
            size_t i = 0;
            
            while (is_thinking) {
                // \r moves the cursor back to the start of the line
                // \033[K clears the rest of the line
                std::cout << "\r\033[K" << frames[i] << " " << message << std::flush;
                i = (i + 1) % frames.size();
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
            }
            
            // Clear the spinner line when done
            std::cout << "\r\033[K" << std::flush; });

        // Detach so it runs independently
        spinner_thread.detach();
    }

    inline void stopSpinner()
    {
        is_thinking = false;
        // Small delay to ensure the thread sees the change and cleans up the line
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}