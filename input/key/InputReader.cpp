#include "InputReader.h"
#include <sys/ioctl.h>
#include <linux/input.h>
#include <cstring>
#include <cerrno>

#define BITS_PER_LONG (8 * sizeof(long))
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define TEST_BIT(nr, addr) ((((addr)[BIT_WORD(nr)]) >> ((nr) % BITS_PER_LONG)) & 1)

InputReader::InputReader() {}
InputReader::~InputReader() { for (int fd : fds) { if (fd >= 0) close(fd); } }

bool InputReader::Init() {
    DIR* dir = opendir("/dev/input");
    if (!dir) {
        std::cerr << "[-] Critical: Failed to open /dev/input. Error: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "[Diagnostic] Successfully opened /dev/input. Scanning nodes..." << std::endl;

    struct dirent* entry;
    int event_node_count = 0;

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            std::cout << "[Diagnostic] Visible entry: " << entry->d_name << std::endl;
        }

        if (strncmp(entry->d_name, "event", 5) == 0) {
            event_node_count++;
            std::string path = std::string("/dev/input/") + entry->d_name;
            int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);

            if (fd < 0) {
                std::cout << "  [-] Failed to open " << path << ". Error: " << strerror(errno) << std::endl;
                continue;
            }

            unsigned long evtype_bitmask[DIV_ROUND_UP(EV_MAX, BITS_PER_LONG)] = {0};
            unsigned long key_bitmask[DIV_ROUND_UP(KEY_MAX, BITS_PER_LONG)] = {0};

            if (ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask) >= 0) {
                // Mice and keyboards both use EV_KEY for buttons/keys
                if (TEST_BIT(EV_KEY, evtype_bitmask)) {

                    // Fetch specifically what keys/buttons this device is capable of sending
                    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) >= 0) {

                        // Check if it's a keyboard (has normal keys) OR a mouse (has mouse buttons)
                        bool is_keyboard = TEST_BIT(KEY_A, key_bitmask) || TEST_BIT(KEY_1, key_bitmask);
                        bool is_mouse = TEST_BIT(BTN_LEFT, key_bitmask) || TEST_BIT(BTN_RIGHT, key_bitmask);

                        if (is_keyboard || is_mouse) {
                            std::cout << "[+] Accepted device: " << path
                                      << (is_keyboard ? " (Keyboard)" : "")
                                      << (is_mouse ? " (Mouse)" : "") << std::endl;
                            fds.push_back(fd);
                        } else {
                            std::cout << "  [-] Device " << path << " rejected: No relevant keys or mouse buttons found." << std::endl;
                            close(fd);
                        }
                    } else {
                        // Fallback: If we can't read specific keys, keep it anyway just in case
                        fds.push_back(fd);
                    }
                } else {
                    std::cout << "  [-] Device " << path << " rejected: Does not support EV_KEY" << std::endl;
                    close(fd);
                }
            } else {
                std::cout << "  [-] ioctl failed for " << path << ". Error: " << strerror(errno) << std::endl;
                close(fd);
            }
        }
    }
    closedir(dir);

    std::cout << "[Diagnostic] Total 'event*' strings matched: " << event_node_count << std::endl;
    std::cout << "[+] Found " << fds.size() << " input event devices" << std::endl;
    return fds.size() > 0;
}

void InputReader::Poll() {
    // 1. Snapshot the previous frame's exact state
    // Note: Using an iterator loop since state is a std::map, your original loop
    // trying to iterate to 512 would skip keys/buttons with codes higher than 512.
    lastState = state;

    struct input_event ie;
    for (int fd : fds) {
        while (read(fd, &ie, sizeof(struct input_event)) > 0) {
            if (ie.type == EV_KEY) {
                if (ie.value == 1) {
                    state[ie.code] = true;
                } else if (ie.value == 0) {
                    state[ie.code] = false;
                }
            }
        }
    }
}

// Returns true as long as the key/button is held down
bool InputReader::IsKeyDown(int keycode) {
    return state[keycode];
}

// FIX: Returns true ONLY on the exact frame the key was first pressed
bool InputReader::WasKeyPressed(int keycode) {
    return state[keycode] && !lastState[keycode];
}

// Returns true ONLY on the exact frame the key was released
bool InputReader::WasKeyReleased(int keycode) {
    return !state[keycode] && lastState[keycode];
}