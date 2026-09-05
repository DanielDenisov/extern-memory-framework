#include <iostream>

#include "memory/memory.h"
#include "draw/Window.h"
#include "config.h"
#include "util/targetStructs.h"


int main() {

    //Find & Set PID
    GMemoryManager.setPID(PID::FindProcessPID("target.exe"));

    //Find & Set Base Address
    GMemoryManager.setBaseAddress(BaseAddress::FindBaseAddressOfWineProcess(
        GMemoryManager.getPID(), "target.exe"));

    //Examples
    auto processBase = ReadMemory<uintptr_t>(GMemoryManager.getBaseAddress() + offsets::prossessBaseOffset);
    auto VectorStruct = ReadMemory<TargetVector<int>>(processBase + offsets::relativeOffsetOfVector);
    //You get the idea, also look over buffered reading + writing

}
