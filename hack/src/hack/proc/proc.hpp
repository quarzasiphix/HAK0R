#pragma once
#include <common.hpp>

namespace hack {
	class proc {
		DWORD pid{};
		HANDLE h_proc{};

		std::wstring m_name;
		DWORD GetProcessIdByName(const std::wstring& name);

		struct thread {
			thread(proc p_proc);
			~thread();
			HANDLE get_thread() { return h_thread; }
		private:
			proc& proc;

			HANDLE h_thread{};

			HMODULE h_module = nullptr;
			DWORD cbNeeded = 0;

			MODULEINFO module_info = { 0 };

			CONTEXT context{};

			DWORD targetMainThreadId{};
			DWORD GetMainThreadId(DWORD processId);

		};

	public:
		thread* m_thred;

		proc(std::wstring name);

		DWORD get_pid() { return pid; }
		HANDLE get_proc() { return h_proc; }

		template <typename T>
		bool readProcMem(LPVOID address, T& value) {
			if (!ReadProcessMemory(h_proc, address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed reading" << std::endl;
				return false;
			}
			return true;
		}

		template <typename T>
		bool writeProcMem(LPVOID address, T& value) {
			if (!WriteProcessMemory(h_proc, address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed writing" << std::endl;
				return false;
			}
			return true;
		}

	};

	/*
	class process {
		std::wstring m_name;

		DWORD pid{};

		HANDLE h_process{};

		HMODULE h_module = nullptr;
		DWORD cbNeeded = 0;

		MODULEINFO module_info = { 0 };

		uintptr_t startAddress;
		uintptr_t endAddress;

		//DWORD GetProcessIdByName(const std::wstring& name);

		struct thread {

			thread(process p_proc);
			~thread();
			HANDLE get_thread() { return h_thread; }

		private:
			process& proc;

			HANDLE h_thread{};

			CONTEXT context{};

			DWORD targetMainThreadId{};
			DWORD GetMainThreadId(DWORD processId);

		};
	public:
		DWORD get_pid() { return pid; }
		HANDLE get_proc() { return h_process; }

		thread* m_thread;

		template <typename T>
		bool readProcMem(long long address, T& value) {
			if (!ReadProcessMemory(h_process, (LPCVOID)address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed reading" << std::endl;
				return false;
			}
			return true;
		}

		template <typename T>
		bool writeProcMem(long long address, T& value) {
			if (!WriteProcessMemory(h_process, (LPCVOID)address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed writing" << std::endl;
				return false;
			}
			return true;
		}

		process(std::wstring name);

		~process();
	};
	*/
}