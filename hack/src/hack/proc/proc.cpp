#include "proc.hpp"
namespace hack {
    DWORD proc::GetProcessIdByName(const std::wstring& name) {
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


    DWORD proc::thread::GetMainThreadId(DWORD processId)
    {
        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnapshot == INVALID_HANDLE_VALUE)
        {
            return 0;
        }

        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        DWORD mainThreadId = 0;
        DWORD currentProcessId = GetCurrentProcessId();

        if (Thread32First(hThreadSnapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == processId) {
                    mainThreadId = te32.th32ThreadID;
                    if (te32.th32OwnerProcessID == currentProcessId) {
                        break;  // Found the main thread of the target process in the same process
                    }
                }
            } while (Thread32Next(hThreadSnapshot, &te32));
        }

        CloseHandle(hThreadSnapshot);
        return mainThreadId;
    }

    template <typename T>
    bool proc::writeProcMem(uintptr_t address, T& value) {
        if (!WriteProcessMemory(h_process, (LPVOID)address, &T, sizeof(T), NULL)) {
            printf("failed writing");
            return false;
        } return true;
    }

    template <typename T>
    bool proc::readProcMem(uintptr_t address, T& value) {
        if (!ReadProcessMemory(h_process, (LPVOID)address, &T, sizeof(T), NULL)) {
            printf("failed reading");
            return false;
        } return true;
    }


    proc::thread::~thread() {
        CloseHandle(h_thread);
    }

    proc::thread::thread(proc proc) {
        h_thread = OpenThread(PROCESS_ALL_ACCESS, FALSE, targetMainThreadId);
        if (h_thread == NULL) {
            std::cout << "Failed to open the target thread" << std::endl;
            return;
        }

        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(h_thread, &context)) {
            std::cout << "Failed to get the thread context" << std::endl;
            return;
        }

        printf("thread initialised\n");
    }

    proc::proc(std::wstring name) : m_name(name) {
        DWORD pid = GetProcessIdByName(m_name);
        printf("finding process..");
        while (pid == 0) {
            pid = GetProcessIdByName(name);
            Sleep(300);
        }

        h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (h_process == NULL) {
            std::cout << "Failed to open process" << std::endl;
            return;
        }
        
        m_thread = new thread(*this);

        printf("attached to process: %s , succesfully", m_name.c_str());
    }

    proc::~proc() {
        CloseHandle(h_process);
    }
}