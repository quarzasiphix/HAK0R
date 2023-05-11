#pragma once
#include <common.hpp>

namespace hack {
	class proc {
	public:
		std::wstring m_name;

		DWORD pid;

		HANDLE h_process;

		HMODULE h_module = nullptr;
		DWORD cbNeeded = 0;

		MODULEINFO module_info = { 0 };

		uintptr_t startAddress;
		uintptr_t endAddress;

		DWORD GetProcessIdByName(const std::wstring& name);

		struct thread {
			thread(proc proc);
			~thread();

			HANDLE h_thread;

			CONTEXT context;

			DWORD targetMainThreadId;
			DWORD GetMainThreadId(DWORD processId);
		};

		thread* m_thread;

		template <typename T>
		bool readProcMem(uintptr_t address, T& value);

		template <typename T>
		bool writeProcMem(uintptr_t address, T& value);

		proc(std::wstring name);

		~proc();
	};
}