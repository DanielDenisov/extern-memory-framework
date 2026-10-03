# Extern Process Memory Reader
Framework for **reading memory** from a process (either for Native Linux OR Wine/Proton)
Also inlcudes output via **window or overlay** and input via **TUI/terminus**
The framewok is EXCLUSIVE to **wayland** due to the Window/Overlay and the decision to preserve privacy (XServer allows any window to capture the display of any other window, this is a structural flaw in Xo1rg itself witch is a result of its initial use as a Client/Mainframe archetecture).

## Usecase & Features
Currently, there exists no common/widespread method by witch to R/W memory through a process/framework thats standerdized

### Features:
 - R/W memory with primitives and structures
 - Output via a Window or Overlay (configurable)
 - TUI to control user settings while preserving output (cout, cerr)
 - PID isolation (this framework is invisible to target)
 - Window isolation (this framework's Window cannot be captured)

## Setup/Install

### 1) Must use a Linux distro running **Wayland**
The system was inteneded for Plamsa/KDE Wayland Desktop Environment
It can be used on other Wayland based DEs/Compsitors like Hyperland, but requires custom window rules

### 2) HIGHLY RECOMENDED: Install target under **Flatpak**
Flatpak not only preserves privacy (like wayland), but it puts the target in an isolated environment.
 - Flatpak isolates processes such that processes cannot access files outside of its container (so cannot see this Framework)
 - Flatpak isolates the target process from other processes (PID isolation)
 
 Installing under Flatpak hides the framework compeatly from the target
#### Consiquences of NOT installing Flatpak
Process will be alerted to the presense of the Framework & will be able to read all files on the system
#### Flatpak and nested installing (Lutrus/Steam)
If your tagret is being installed through Lutrus or Steam, that installer should **ALSO** be installed under Flatpak

### 3) Clone Repo
Since the Framework depends on IMGUI, it needs to be cloned recusivly
```
git clone --recurse-submodules https://github.com/DanielDenisov/extern-memory-framework.git
```

### 4) Building

#### Using CLion (recomended)


#### Using Terminal
```cmake...```

### 5) Running
Once built, the file is outputted to ```...```.
Before running, you need to add the program to ...
The program must be ran as root.

There is a provided `setup.sh`
Run to stat the program
```
bash start.sh

```
