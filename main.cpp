#include <iostream>

#include "memory/memory.h"
#include "draw/Window.h"
#include "config.h"
#include "util/targetStructs.h"


int main() {

    //Find & Set PID
    GMemoryManager.setPID(PID::FindProcessPID("SquadGame-Win64-Shipping.exe"));

    //Find & Set Base Address
    GMemoryManager.setBaseAddress(BaseAddress::FindBaseAddressOfWineProcess(
        GMemoryManager.getPID(), "SquadGame-Win64-Shipping.exe"));

    std::cout << "PID: " << GMemoryManager.getPID() << " | BA: " << std::hex << GMemoryManager.getBaseAddress() << std::hex << ::std::endl;

    //Examples
    //You get the idea, also look over buffered reading + writing

    Window win(true);

    if (!win.IsValid()) return 1;

    while (true) {
        //Reads
        auto processBase = ReadMemory<uintptr_t>(GMemoryManager.getBaseAddress() + offsets::prossessBaseOffset);
        auto VectorStruct = ReadMemory<TargetVector<int>>(processBase + offsets::relativeOffsetOfVector);

        //Render Loop Begin
        win.RenderBegin();

        DrawTextCentered(1000, 300, IM_COL32(255, 255, 255, 255), "blabla");

        //Render Loop End
        win.RenderEnd();

    }

}
