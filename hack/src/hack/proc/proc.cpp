#include "proc.hpp"

namespace hack {
    DWORD process::GetProcessIdByName(const std::wstring& name) {
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


    DWORD process::thread::GetMainThreadId(DWORD processId)
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


    process::thread::~thread() {
        CloseHandle(h_thread);
    }


    process::thread::thread(process p_proc): proc(p_proc) {
        targetMainThreadId = GetMainThreadId(proc.pid);
        if (targetMainThreadId == 0) {
            std::cout << "Failed to get the main thread ID of the target process" << std::endl;
            return;
        }

        h_thread = OpenThread(THREAD_GET_CONTEXT, FALSE, targetMainThreadId);
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

    process::process(std::wstring name) : m_name(name) {
        pid = GetProcessIdByName(m_name);
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

        std::wcout << "attached to process: " << m_name << ", succesfully\n";
    }

    process::~process() {
        delete m_thread;
        CloseHandle(h_process);
    }
}