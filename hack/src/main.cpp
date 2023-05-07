#include <common.hpp>

void Parse(char* combo, char* pattern, char* mask)
{
    char lastChar = ' ';
    unsigned int j = 0;

    for (unsigned int i = 0; i < strlen(combo); i++)
    {
        if ((combo[i] == '?' || combo[i] == '*') && (lastChar != '?' && lastChar != '*'))
        {
            pattern[j] = mask[j] = '?';
            j++;
        }

        else if (isspace(lastChar))
        {
            pattern[j] = lastChar = (char)strtol(&combo[i], 0, 16);
            mask[j] = 'x';
            j++;
        }
        lastChar = combo[i];
    }
    pattern[j] = mask[j] = '\0';
}

DWORD GetProcessIdByName(const std::wstring& name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return 0;
    }

    DWORD pid = 0;
    do {
        if (name.compare(entry.szExeFile) == 0) {
            pid = entry.th32ProcessID;
            break;
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return pid;
}
   
DWORD_PTR FindSignature(HANDLE processHandle, const char* signature, const char* mask) {
    // Get the module information for the main module of the specified process
    MODULEINFO moduleInfo;
    HMODULE moduleHandle = nullptr;
    if (EnumProcessModules(processHandle, &moduleHandle, sizeof(moduleHandle), nullptr)) {
        GetModuleInformation(processHandle, moduleHandle, &moduleInfo, sizeof(moduleInfo));
    }
    else {
        std::cerr << "Failed to get module handle." << std::endl;
        return 0;
    }

    // Search for the signature in the module memory
    const char* moduleStart = reinterpret_cast<const char*>(moduleInfo.lpBaseOfDll);
    const char* moduleEnd = moduleStart + moduleInfo.SizeOfImage - strlen(mask);
    for (const char* p = moduleStart; p < moduleEnd; ++p) {
        bool found = true;
        for (size_t i = 0; i < strlen(mask); ++i) {
            if (mask[i] != '?' && p[i] != signature[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<DWORD_PTR>(p);
        }
    }

    std::cerr << "Signature not found." << std::endl;
    return 0;
}

int main() {
    // Find the window with the specified class name or window title
    std::wstring name = L"victim.exe";
    DWORD pid = GetProcessIdByName(name);
    printf("finding process..");
    while (pid == 0) {
        pid = GetProcessIdByName(name);
        Sleep(300);
    }
    system("cls");

    // Open the target process with PROCESS_VM_WRITE access
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        std::cout << "Failed to open process" << std::endl;
        return 1;
    }
    
    LPVOID address{};

    HMODULE moduleHandle{};
    EnumProcessModules(hProcess, &moduleHandle, sizeof(moduleHandle), nullptr); 
        MODULEINFO moduleInfo{};
        GetModuleInformation(hProcess, moduleHandle, &moduleInfo, sizeof(moduleInfo));

        char signature[100];
        char mask[100];

        char combo[100] = "\x89\x44\x24\x00\xff\x15 xxx ? xx";

        Parse(combo, signature, mask);

        // Calculate the start address and size of the module
        DWORD_PTR startAddress = reinterpret_cast<DWORD_PTR>(moduleInfo.lpBaseOfDll);
        DWORD_PTR endAddress = startAddress + moduleInfo.SizeOfImage;

        DWORD_PTR signatureAddress = FindSignature(hProcess, signature, mask);
        if (signatureAddress != 0) {
            std::cout << "Signature found at address: " << std::hex << signatureAddress << std::endl;
            std::string input;
            if (signatureAddress >= startAddress && signatureAddress <= endAddress) {
                std::cout << "Address is within module bounds." << std::endl << "is this correct? y/n: ";
                std::cin >> input;
                if (input[1] == 'y') address = reinterpret_cast<LPVOID>(signatureAddress);
                else {
                    long long addres = 1;
                    std::cout << "memory addres: ";
                    std::cin >> std::hex >> addres;
                    address = reinterpret_cast<LPVOID>(addres);
                    std::cout << addres << std::endl << "start: ";
                    if (addres == 1) {
                        std::cout << "memory addres: ";
                        std::cin >> std::hex >> addres;
                        address = reinterpret_cast<LPVOID>(addres);
                    }
                }
            }
        }
    


    int value = 1;
    while (value != 0) {
        if (!ReadProcessMemory(hProcess, address, &value, sizeof(value), NULL)) return 0;
        std::cout << "value: " << value << std::endl;
        for (int i = 1; i < 11; i++) {
            if (!ReadProcessMemory(hProcess, address, &value, sizeof(value), NULL)) return 0;
            std::cout << "loop: " << i << " value: " << value << std::endl;
            if(!WriteProcessMemory(hProcess, address, &i, sizeof(i), NULL)) return 0;
            Sleep(500);
        }

        printf("written\n");
    }

    printf("bye\n");
    // Close the handle to the process
    CloseHandle(hProcess);

    return 0;
}