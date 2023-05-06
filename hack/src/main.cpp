#include <common.hpp>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == *reinterpret_cast<DWORD*>(lParam)) {
        std::vector<char> title(256);
        GetWindowTextA(hwnd, title.data(), title.size());
        std::cout << "Window found: " << title.data() << std::endl;
        *reinterpret_cast<HWND*>(lParam) = hwnd;
        return FALSE;  // Stop enumeration
    }
    return TRUE;  // Continue enumeration
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
    

int main() {
    // Find the window with the specified class name or window title
    std::wstring name = L"victim.exe";
    DWORD pid = GetProcessIdByName(name);
    if (pid == 0) {
        std::cout << "Failed to get process ID by name" << std::endl;
        return 1;
    }

    // Open the target process with PROCESS_VM_WRITE access
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == NULL) {
        std::cout << "Failed to open process" << std::endl;
        return 1;
    }

    // Write a value to the process's memory
    long long addres = 0;
    std::cout << "input memory addres: ";
    std::cin >> std::hex >> addres;
    LPVOID address = reinterpret_cast<LPVOID>(addres);
    std::cout << addres << std::endl << "start: ";
    std::cin.get();

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