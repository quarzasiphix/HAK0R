#pragma once
#include <common.hpp>

namespace hack {
	//struct shellhook;
	class proc {
		std::wstring m_name;
		DWORD m_pid{};

		HANDLE h_proc{};
		MODULEINFO module_info = { 0 };
		HMODULE h_module = nullptr;
		DWORD cbNeeded = 0;

		uintptr_t startAddress;
		uintptr_t endAddress;

		DWORD GetProcessIdByName(const std::wstring& name);

		struct thread {
			thread(proc& p_proc);
			~thread();
			bool attached = false;
		private:
			proc& proc;

			HANDLE h_thread{};
			DWORD targetMainThreadId{};

			DWORD GetMainThreadId(DWORD processId);
		public:
			CONTEXT context{};
			HANDLE get_thread() { return h_thread; }
			CONTEXT get_context() { return context; }
			DWORD get_mainthreadid() { return targetMainThreadId; }
		};

		struct shellhook {
			proc& m_proc;
			LPVOID remoteMemory;
			bool set_hook;
			template<typename Func>
			shellhook(proc& proc, uintptr_t jmpAddress, Func func) : m_proc(proc) {
				// Allocate executable memory in the target process
				remoteMemory = VirtualAllocEx(m_proc.get_proc(), NULL, sizeof(func), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
				if (!remoteMemory) {
					std::cerr << "Failed to allocate memory in the target process" << std::endl;
					//CloseHandle(m_proc->get_proc());
					set_hook = false;
					return;
				}

				printf("allocated virtual mem\n");

				// Write the custom function code into the allocated memory
				if (!WriteProcessMemory(m_proc.get_proc(), remoteMemory, (LPCVOID)&func, sizeof(func), NULL)) {
					std::cerr << "Failed to write the function code into the target process memory" << std::endl;
					VirtualFreeEx(m_proc.get_proc(), remoteMemory, 0, MEM_RELEASE);
					//CloseHandle(hProcess);
					set_hook = false;
					return;
				}

				printf("added function to virtual mem\n");

				uintptr_t relativeOffset = (uintptr_t)remoteMemory - (jmpAddress + 5);

				// Prepare the new JMP instruction
				unsigned char jmpInstruction[] = {
					0xE9,                                      // JMP instruction
					(relativeOffset & 0xFF),
					((relativeOffset >> 8) & 0xFF),
					((relativeOffset >> 16) & 0xFF),
					((relativeOffset >> 24) & 0xFF)
				};

				// Write the new JMP instruction to the target process memory
				if (!WriteProcessMemory(m_proc.get_proc(), (LPVOID)jmpAddress, jmpInstruction, sizeof(jmpInstruction), NULL)) {
					std::cerr << "Failed to write the JMP instruction" << std::endl;
					VirtualFreeEx(m_proc.get_proc(), remoteMemory, 0, MEM_RELEASE);
					//CloseHandle(hProcess);
					set_hook = false;
					return;
				}

				set_hook = true;
				std::cout << "JMP instruction replaced with the jump to MyFunction" << std::endl;
			}
		};

	public:
		thread* m_thread;
		shellhook* m_hook;

		proc(std::wstring name);
		proc(DWORD pid);
		proc(DWORD pid, std::wstring name);
		~proc();
		void attach();
		bool attached = false;
		int is_access = 0;

		DWORD get_pid() { return m_pid; }
		HANDLE get_proc() { return h_proc; }
		HANDLE get_hmodule() { return h_module; }
		std::wstring get_name() { return m_name; }
		DWORD get_cbneeded() { return cbNeeded; }
		MODULEINFO get_moduleinfo() { return module_info; }
		CONTEXT get_context() { return m_thread->context; }
		uintptr_t get_rsp() { return m_thread->context.Rsp; }

		uintptr_t get_startAddress() { return (uintptr_t)module_info.lpBaseOfDll; }
		uintptr_t get_endAddress() { return get_startAddress() + module_info.SizeOfImage; }

		LPCVOID FindSignatureAddress(const std::vector<BYTE>& signature);

		template<typename Func>
		bool set_shellhook(uintptr_t jmpAddress, Func func) {
			std::cout << "setting hook..\n";
			m_hook = new shellhook(*this, jmpAddress, func);
			if (m_hook == nullptr) return false;
			else return true;
		}

		void get_memoryRegion(uintptr_t& start, uintptr_t& end) {
			start = (uintptr_t)module_info.lpBaseOfDll;
			end = startAddress + module_info.SizeOfImage;
		}

		// Custom exception type for read failures
		struct ReadMemoryException : public std::exception {
			const char* what() const noexcept override {
				return "Failed reading process memory";
			}
		};

		bool read_success = false;
		template <typename T>
		T readProcMem(uintptr_t address) {
			T value = {};
			read_success = false;
			if (!ReadProcessMemory(h_proc, (LPCVOID)address, &value, sizeof(value), NULL)) {
				DWORD errorCode = GetLastError();
				std::cerr << "Failed reading process memory, error: " <<  errorCode;
				throw ReadMemoryException();
			}
			read_success = true;
			return value;
		}

		template <typename T>
		bool writeProcMem(uintptr_t address, T& value) {
			if (!WriteProcessMemory(h_proc, (LPVOID)address, &value, sizeof(value), NULL)) {
				DWORD errorCode = GetLastError();
				//EN_ASSERT("Failed writing memory, error: {0}", errorCode);
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