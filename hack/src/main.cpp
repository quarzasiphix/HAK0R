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

std::vector<int> StringToBytes(const std::string& str)
{
    std::vector<int> bytes;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ' ')) {
        if (token == "?") {
            bytes.push_back(-1);
        }
        else {
            bytes.push_back(std::stoi(token, nullptr, 16));
        }
    }
    return bytes;
}

void* FindPattern(HANDLE hProcess, uintptr_t start, uintptr_t end, const char* pattern) {
    const char* p = pattern;
    void* first_match = nullptr;

    // Calculate the length of the memory region to read
    size_t length = end - start;

    // Allocate a buffer to hold the memory read from the external process
    std::vector<char> buffer(length);

    // Read the memory from the external process into the buffer
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProcess, (LPCVOID)start, &buffer[0], length, &bytesRead) || bytesRead != length) {
        return nullptr;
    }

    // Search for the pattern in the buffer
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] == *p || *p == '?') {
            if (!first_match) {
                first_match = (void*)(start + i);
            }
            ++p;
            if (!*p) {
                return first_match;
            }
        }
        else {
            p = pattern;
            first_match = nullptr;
        }
    }

    return nullptr;
}

void* patternScan(char* base, size_t size, char* pattern, char* mask) {
    size_t patternLength = strlen(pattern);
    for (unsigned int i = 0; i < size - patternLength; i++) {
        bool found = true;
        for (unsigned int j = 0; j < patternLength; j++) { // fix: changed '>' to '<'
            if (mask[j] != '?' && pattern[j] != *(base + i + j)) {
                found = false;
                break;
            }
        }
        if (found) return (void*)(base + i);
    }

    return nullptr; // fix: use nullptr instead of 0
}

void* exPatternScan(HANDLE hproc, uintptr_t begin, uintptr_t end, char* pattern, char* mask) {
    uintptr_t currentchunk = begin;
    SIZE_T bytesRead;

    while (currentchunk < end) {
        char buffer[4096];

        DWORD oldprotect;
        VirtualProtectEx(hproc, (void*)currentchunk, sizeof(buffer), PAGE_READWRITE, &oldprotect); // fix: changed protection to PAGE_READWRITE
        ReadProcessMemory(hproc, (void*)currentchunk, buffer, sizeof(buffer), &bytesRead); // fix: removed unnecessary '&'
        VirtualProtectEx(hproc, (void*)currentchunk, sizeof(buffer), oldprotect, NULL);

        if (bytesRead == 0) return nullptr;

        void* internalAddress = patternScan(buffer, bytesRead, pattern, mask); // fix: pass buffer instead of &buffer

        if (internalAddress != nullptr) {
            uintptr_t offsetFromBuffer = (uintptr_t)internalAddress - (uintptr_t)buffer;
            return (void*)(currentchunk + offsetFromBuffer);
        }
        else currentchunk += bytesRead; // fix: use += instead of + to update currentchunk
    }
    return nullptr; // fix: use nullptr instead of 0
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
    
    // Get the module handle for the target module in the target process
    HMODULE hModule = nullptr;
    DWORD cbNeeded = 0;
    if (!EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
        std::cerr << "Failed to enumerate process modules: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return 0;
    }

    // Allocate memory to store the module information
    MODULEINFO module_info = { 0 };
    if (!GetModuleInformation(hProcess, hModule, &module_info, sizeof(module_info))) {
        std::cerr << "Failed to get module information: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return 0;
    }

    // Calculate the start and end addresses of the module in the target process
    // Determine the size of the module and set the end address to search
    uintptr_t startAddress = (uintptr_t)module_info.lpBaseOfDll;
    uintptr_t endAddress = startAddress + module_info.SizeOfImage;

    const char* sig = "83 f8 ? 75 ? 66 0f 1f 84 00";

    char pattern[30] = "\x2a\x00";
    char mask[20] = "xx";
    void* addres = exPatternScan(hProcess, startAddress, endAddress, pattern, mask);
    if (addres == NULL) {
        printf("fucked up finding sig, %s", sig);
        std::cout << "\ntype in memory address instead? \n y/n: ";
    }
    else {
        int test = 0;
        if (ReadProcessMemory(hProcess, addres, &test, sizeof(test), NULL)) {
            std::cout << "Variable value: " << test << std::endl;
            std::cout << "Variable address: " << std::hex << reinterpret_cast<uintptr_t>(addres) << std::endl;

            std::cout << "\nAddress is within module bounds." << std::endl << "is this correct? y/n: ";
        }
        else {
            std::cerr << "Failed to read variable: " << GetLastError() << std::endl;
        }
    }

    void* address;
    std::string input;
    std::cin >> input;
    if(input != "y") {
        long long adresinput;
        std::cout << "memory addres: ";
        std::cin >> std::hex >> adresinput;
        address = reinterpret_cast<LPVOID>(adresinput);
    }
    else {
        address = reinterpret_cast<LPVOID>(addres);
    }

    int value = 1;
    while (value != 0) {
        if (!ReadProcessMemory(hProcess, address, &value, sizeof(value), NULL)) printf("failed reading");
        std::cout << "value: " << value << std::endl;
        for (int i = 1; i < 11; i++) {
            if (!ReadProcessMemory(hProcess, address, &value, sizeof(value), NULL)) printf("failed reading");
            std::cout << "loop: " << i << " value: " << value << std::endl;
            if(!WriteProcessMemory(hProcess, address, &i, sizeof(i), NULL)) printf("failed writing");
            Sleep(500);
        }

        printf("written\n");
    }

    printf("bye\n");
    // Close the handle to the process
    CloseHandle(hProcess);

    return 0;
}