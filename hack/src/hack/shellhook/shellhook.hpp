#pragma once
#include <hack/proc/proc.hpp>

namespace hack {
	class proc;
	struct shellhook {
		template<typename Func>
		shellhook(proc& proc, Func func) : m_proc(proc) {
			// Allocate executable memory in the target process
			LPVOID remoteMemory = VirtualAllocEx(m_proc.get_proc(), NULL, sizeof(func), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!remoteMemory)
			{
				std::cerr << "Failed to allocate memory in the target process" << std::endl;
				//CloseHandle(m_proc->get_proc());
				return;
			}

			printf("allocated virtual mem\n");

			// Write the custom function code into the allocated memory
			if (!WriteProcessMemory(m_proc.get_proc(), remoteMemory, (LPCVOID)&func, sizeof(func), NULL))
			{
				std::cerr << "Failed to write the function code into the target process memory" << std::endl;
				VirtualFreeEx(m_proc.get_proc(), remoteMemory, 0, MEM_RELEASE);
				//CloseHandle(hProcess);
				return;
			}

			printf("added function to virtual mem\n");

		}
		proc& m_proc;
	};
}

