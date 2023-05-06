#pragma once
#include <common.hpp>

struct proc {
    std::wstring name;
    DWORD pid;
    HANDLE hProcess;
    LPVOID address;

    DWORD pBaseAddress; //Base Address of exe
    DWORD pSize;

    uintptr_t GetAddressFromSignature(std::vector<int> signature);
};

