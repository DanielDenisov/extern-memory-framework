//Need to be here or else a linker error type thing will be thrown
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include "memory.h"

namespace PID {
    #include <filesystem>
    #include <fstream>
    namespace fs = std::filesystem;

    //Finds the PID for an process.
    // The exact executable naming convention seen in your Wine process dump
    pid_t FindProcessPID(std::string procName) {
        const std::string target_exe = procName;

        for (const auto& entry : fs::directory_iterator("/proc")) {
            if (!entry.is_directory()) continue;

            std::string pid_str = entry.path().filename().string();
            if (pid_str.empty() || !std::isdigit(pid_str[0])) continue;

            try {
                pid_t pid = std::stoi(pid_str);
                std::string cmdline_path = "/proc/" + pid_str + "/cmdline";

                std::ifstream file(cmdline_path, std::ios::binary);
                if (!file.is_open()) continue;

                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());

                if (content.empty()) continue;

                // Find str in binary
                if (content.find(target_exe) != std::string::npos) {
                    std::cout << "[+] Found game process inside Flatpak! PID: " << pid
                            << " (" << target_exe << ")" << std::endl;
                    return pid;
                }
            } catch (...) {
                continue;
            }
        }

        std::cerr << "[-] Game process (" << target_exe << ") not found!" << std::endl;
        return 0;
    }
}

namespace BaseAddress{
    long FindBaseAddressOfWineProcess(pid_t pid, std::string procName) {
        return 0x140000000; //For me its always been this accross
        //many processes.
    }


    uintptr_t GetProcessBaseAddress(pid_t pid, const std::string& name) {
        std::ifstream file("/proc/" + std::to_string(pid) + "/maps");
        if (!file.is_open()) return 0;

        std::string line;
        while (std::getline(file, line)) {
            if (line.find(name) == std::string::npos) continue;

            std::stringstream ss(line);
            std::string range, perms;
            ss >> range >> perms;

            // Ensure it's the executable segment
            if (perms.find('x') != std::string::npos) {
                size_t dash = range.find('-');
                if (dash != std::string::npos) {
                    return std::stoull(range.substr(0, dash), nullptr, 16);
                    }
            }
        }
        return 0;
    }
}
