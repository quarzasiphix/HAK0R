#pragma once
#include <common.hpp>

namespace hack {
	class process {
		std::wstring m_name;

		DWORD pid;

		HANDLE h_process{};

		HMODULE h_module = nullptr;
		DWORD cbNeeded = 0;

		MODULEINFO module_info = { 0 };

		uintptr_t startAddress;
		uintptr_t endAddress;

		DWORD GetProcessIdByName(const std::wstring& name);

		struct thread {
			thread(process p_proc);
			~thread();

			process& proc;

			HANDLE h_thread{};

			CONTEXT context{};

			DWORD targetMainThreadId{};
			DWORD GetMainThreadId(DWORD processId);
		};

	public:
		thread* m_thread;

		template <typename T>
		bool readProcMem(uintptr_t address, T& value) {
			if (!ReadProcessMemory(h_process, (LPVOID)address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed reading" << std::endl;
				return false;
			}
			return true;
		}

		template <typename T>
		bool writeProcMem(uintptr_t address, T& value) {
			if (!WriteProcessMemory(h_process, (LPVOID)address, &value, sizeof(value), NULL)) {
				std::cout << "\nFailed writing" << std::endl;
				return false;
			}
			return true;
		}

		process(std::wstring name);

		~process();
	};
}