# Extern Memory Framework


## Design Decisions
Intended to be used on wayland. While can be modified to not, but running any sort of window in X11 is inherently not secure due to the centralized Xorg server that allows every window to view the content of every other window on the system.
Additionally, this program is really designed to be hidden from a possible aggressive target, meaning that the target even knowing about this process existing can be considered a vector of detection, due to this, it's recommended to run under Flatpak were PID isolation exists.

## TODO
1) test out window class
2) create target program?
3) crete readme + cool images of it working