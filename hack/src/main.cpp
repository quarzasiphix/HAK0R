#include <common.hpp>


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
   
long long FindSignature(void* startAddress, size_t size, const unsigned char* sig, size_t sigSize)
{
    unsigned char* start = reinterpret_cast<unsigned char*>(startAddress);
    unsigned char* end = start + size - sigSize;
    for (unsigned char* p = start; p <= end; ++p)
    {
        if (memcmp(p, sig, sigSize) == 0)
        {
            std::stringstream ss;
            ss << std::hex;
            for (size_t i = 0; i < sizeof(p); ++i)
            {
                ss << static_cast<unsigned int>(p[i]);
            }

            long long result;
            ss >> std::hex >> result;

            return result; // return the address of the match
        }
    }
    return 0; // no match found
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

    /*
    printf("finding sig\n");
    bool is_correct = false;
    int* p = new int(42);
    void* address = FindSignature(p, sizeof(int), signature, sizeof(signature));
    if (address) {
        std::cout << "Found signature at address: " << address << std::endl << "is this correct? true/false: ";
        std::cin >> is_correct;
        Sleep(200);
    }
    else {
        std::cout << "Signature not found." << std::endl;
        return 0;
        // Write a value to the process's memory
    }
    delete p;

    if (is_correct == false) {
        long long addres = 1;
        LPVOID address;
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
        long long add = 0x000008C462FFCF0;
    */

    printf("finding sig\n");
    const unsigned char signature[] = { 0x2a, 0x00 }; // the signature pattern to search for

    int* p = new int(42);
    long long sig = FindSignature(p, sizeof(int), signature, sizeof(signature));
    
    LPVOID address{};

    if (sig != 0) {
        std::cout << "Found signature at address: " << address << std::endl << "is this correct? y\\n\n";
        std::string input;
        std::cin >> input;
        if(input[1] == 'y') address = reinterpret_cast<LPVOID>(sig);
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