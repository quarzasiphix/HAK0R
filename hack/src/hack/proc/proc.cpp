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

    DWORD proc::thread::GetMainThreadId(DWORD processId) {
        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnapshot == INVALID_HANDLE_VALUE)
            return 0;

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

    proc::thread::thread(hack::proc& p_proc) : proc(p_proc) {
        targetMainThreadId = GetMainThreadId(proc.get_pid());
        if (targetMainThreadId == 0) {
            //EN_WARN("Failed to get the main thread ID of the target process");
            return;
        }

        h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, targetMainThreadId);
        if (h_thread == NULL) {
            //EN_WARN("Failed to open target thread");
            return;
        }

        context.ContextFlags = CONTEXT_FULL;
        if (!GetThreadContext(h_thread, &context)) {
            //EN_WARN("Failed to get thread context");
            return;
        }

        attached = true;
    }

    proc::proc(std::wstring name) : m_name(name) {
        m_pid = GetProcessIdByName(name);
        if (this->get_pid() == 0) {
            //EN_WARN("Failed attaching {0} ", name);
            is_access = 2;
            return;
        }


        //Sleep(300);
        //EN_INFO("waiting for process: {0}", name);

        //pid = GetProcessIdByName(name);
        //}

        attach();
        //EN_INFO("Attached to process   : name {0} pid {1} : ", m_name, pid);
    }

    proc::proc(DWORD pid, std::wstring name) : m_pid(pid), m_name(name) {

    }

    proc::proc(DWORD pid) : m_pid(pid) {
        attach();
    }

    proc::thread::~thread() {
        CloseHandle(h_thread);
    }

    proc::~proc() {
        //delete m_thread;
        CloseHandle(h_proc);
    }

    std::string wstringToString(const std::wstring& wstr) {
        int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size == 0) {
            // Error handling
            return "";
        }

        std::string str(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);

        return str;
    }

    void proc::attach() {

        h_proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->get_pid());
        if (h_proc == NULL) {
            if (!m_name.empty()) {
                std::string name = wstringToString(m_name);
                std::cerr << "Not allowed to access process : " << name << ": name";
            }
            //if (this->get_pid() != 0) std::cerr << "not allowed to access process"; //EN_WARN("Not allowed to access process : {0} :", this->get_pid());

            attached = false;
            is_access = 2;
            return;
        }

        m_thread = new thread(*this);

        // Get the module handle for the target module in the target process
        if (!EnumProcessModules(h_proc, &h_module, sizeof(h_module), &cbNeeded)) {
            //EN_WARN("Failed to enumerate process modules");
            return;
        }

        // Allocate memory to store the module information
        if (!GetModuleInformation(h_proc, h_module, &module_info, sizeof(module_info))) {
            //EN_WARN("Failed to get module information: {0}", GetLastError());
            return;
        }

        is_access = 1;
        attached = true;
    }

    /*
    template<typename T>
    bool accessMemory<T>::writeMemory() {
        if (m_proc.writeProcMem<T>(address, write))
            return true;
        return false;
    }

    template<typename T>
    std::any accessMemory<T>::readMemory() {
        return m_proc.readProcMem<T>(address);
    }
    */
}


/*
    proc::thread::thread(proc p_proc)
        : proc(p_proc) {
        targetMainThreadId = GetMainThreadId(p_proc.get_pid());
        if (targetMainThreadId == 0) {
            std::cout << "Failed to get the main thread ID of the target process" << std::endl;
            return;
        }

        h_thread = OpenThread(THREAD_ALL_ACCESS, FALSE, targetMainThreadId);
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


    process::thread::~thread() {
        CloseHandle(h_thread);
    }


    process::process(std::wstring name) : m_name(name) {
        pid = GetProcessIdByName(name);
        std::wcout << "finding... :" << name << ":\n";
        while (pid == 0) {
            pid = GetProcessIdByName(name);
            Sleep(300);
        }

        system("cls");

        h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (h_process == NULL) {
            std::cout << "Failed to open process" << std::endl;
            return;
        }

        DWORD exitCode;
        if (!GetExitCodeProcess(h_process, &exitCode)) {
            // Failed to retrieve exit code, handle may be invalid
            DWORD errorCode = GetLastError();
            std::cout << "Failed to retrieve process exit code: " << errorCode << std::endl;
        }
        else if (exitCode != STILL_ACTIVE) {
            // Process has already exited, handle may be invalid
            std::cout << "Process has already exited" << std::endl;
        }
        else {
            // Process handle is valid and corresponds to the desired process
            std::cout << "Process handle is valid" << std::endl;
        }

        
        m_thread = new thread(*this);

        std::wcout << "attached to process: " << name << " pid: " << pid << " handle: " << h_process << ", succesfully\n";

        LPVOID addy = (LPVOID)0x000000317D2FFA10;

        int read = 1;

        if (!ReadProcessMemory(h_process, addy, &read, sizeof(read), NULL)) {
            DWORD errorCode = GetLastError();
            std::cout << "Failed reading. Error code: " << errorCode << std::endl;
            return;
        }
        else {
            std::cout << "Read value: " << read << "\n";
        }
    }

    process::~process() {
        delete m_thread;
        CloseHandle(h_process);
    }
    */