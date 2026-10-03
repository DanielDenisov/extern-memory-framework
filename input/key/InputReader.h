#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <map>

class InputReader {
public:
    InputReader();
    ~InputReader();

    bool Init();
    void Poll();

    // Keyboard & Generic
    bool IsKeyDown(int keycode);
    bool WasKeyPressed(int keycode);
    bool WasKeyReleased(int keycode);

    // Mouse Specific Helpers
    bool IsLMBDown() { return IsKeyDown(BTN_LEFT); }
    bool WasLMBPressed() { return WasKeyPressed(BTN_LEFT); }
    bool WasLMBReleased() { return WasKeyReleased(BTN_LEFT); }

    bool IsRMBDown() { return IsKeyDown(BTN_RIGHT); }
    bool WasRMBPressed() { return WasKeyPressed(BTN_RIGHT); }
    bool WasRMBReleased() { return WasKeyReleased(BTN_RIGHT); }

    bool IsMMBDown() { return IsKeyDown(BTN_MIDDLE); }
    bool WasMMBPressed() { return WasKeyPressed(BTN_MIDDLE); }
    bool WasMMBReleased() { return WasKeyReleased(BTN_MIDDLE); }

private:
    std::vector<int> fds;
    std::map<int, bool> state;
    std::map<int, bool> lastState;
};
