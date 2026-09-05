/** Include any configuration and things that commonly change here,
 * or really anything intended to be used across the entire file
 */

#pragma once

//Example:
//#define uintptr_t ptr

namespace settings {
    constexpr int WINDOW_HEIGHT = 1080;
    constexpr int WINDOW_WIDTH = 1920;
    constexpr std::string name = "Window Name";
}

namespace offsets {
    inline uintptr_t prossessBaseOffset = 0x1234;
    inline uintptr_t relativeOffsetOfVector = 0x4321;
}