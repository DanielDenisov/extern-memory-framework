/** Header for the main functin of the framework.
 * GlobalMemoryManager needs to be in the .h because if its in the .cpp
 * you will run into errors with the linker and not defining what types T can be.
*/

#include <string>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <vector>

class GlobalMemoryManager {
private:
    pid_t ProcessID = 0;
    long BaseAddress = 0x140000000;

private:
////////// "Raw" Functions //////////
    template <typename T>
    T ReadMemory(pid_t pid, long address)
    {
        T buffer;
        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = &buffer;
        local[0].iov_len = sizeof(T);
        remote[0].iov_base = (void*)address;
        remote[0].iov_len = sizeof(T);

        ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
        if (nread != sizeof(T))
        {
            memset(&buffer, 0, sizeof(T));
        }

        return buffer;
    }

    void ReadMemoryBuffer(pid_t pid, long address, void* buffer, size_t size)
    {

        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = buffer;
        local[0].iov_len = size;
        remote[0].iov_base = (void*)address;
        remote[0].iov_len = size;

        ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
        if (nread != (ssize_t)size)
        {
            memset(buffer, 0, size);
        }
    }

    template<typename T>
    bool WriteMemory(pid_t pid, long address, const T& value)
    {
        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = (void*)&value;
        local[0].iov_len = sizeof(T);
        remote[0].iov_base = (void*)address;
        remote[0].iov_len = sizeof(T);

        ssize_t nwrite = process_vm_writev(pid, local, 1, remote, 1, 0);
        return (nwrite == sizeof(T));
    }

public:

////////// Public Functions but not intended for use //////////    
    template<typename T>
    T ReadMemory(uintptr_t address)
    {
        return ReadMemory<T>(ProcessId, address);
    }

    void ReadMemoryBuffer(uintptr_t address, void* buffer, size_t size)
    {
        ReadMemoryBuffer(ProcessId, address, buffer, size);
    }

    template<typename T>
    bool WriteMemory(uintptr_t address, const T& value)
    {
        return WriteMemory<T>(ProcessId, address, value);
    }

////////// Getters and Setters //////////
    void setPID(pid_t pid) {
        GMemoryManager.ProcessID = pid;
    }
    void setBaseAddress(long baseAddress) {
        GMemoryManager.BaseAddress = baseAddress;
    }

    pid_t getPID() {
        return GMemoryManager.ProcessID;
    }
    long getBaseAddress() {
        return GMemoryManager.BaseAddress;
    }
}

GlobalMemoryManager GMemoryManager; //global instance of class

//********** Wrapped Functions for ease of use **********//

//Read's process memory at address, Requires object GMemoryManager to have PID
template<typename T>
T ReadMemory(uintptr_t address)
{
    return GMemoryManager.ReadMemory<T>(address);
}

//Read's process memory at address into a buffer, Requires object GMemoryManager to have PID
void ReadMemoryBuffer(uintptr_t address, void* buffer, size_t size)
{
    GMemoryManager.ReadMemoryBuffer(address, buffer, size);
}

//Writes memory to a process, this can cause target to crash, Requires object GMemoryManager to have PID
template<typename T>
bool WriteMemory(uintptr_t address, const T& value)
{
    return GMemoryManager.WriteMemory<T>(address, value);
}

