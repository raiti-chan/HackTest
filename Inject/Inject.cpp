#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <Psapi.h>

void out_error() {
	char* p_msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&p_msg_buf), 0, nullptr);
	std::cout << "Error : " << p_msg_buf << "\n";
	LocalFree(p_msg_buf);
}

void inject(DWORD process_id, const char* const dll_name) {
	HANDLE h_process = nullptr;
	char* remote_char_ptr = nullptr;
	PTHREAD_START_ROUTINE thread_start_routine_ptr = nullptr;
	HANDLE h_remote_thread = nullptr;
	std::cout << "start inject!\n";
	__try {
		SetLastError(NO_ERROR);
		std::cout << "Open Process Handle.\n";
		h_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, process_id);
		if (h_process == nullptr) {
			out_error();
			__leave;
		}
		std::cout << "Alloc Remote Memory.\n";
		remote_char_ptr = reinterpret_cast<char*>(VirtualAllocEx(h_process, nullptr, strlen(dll_name) + 1, MEM_COMMIT, PAGE_READWRITE));
		if (remote_char_ptr == nullptr) {
			out_error();
			__leave;
		}
		std::cout << "Write Remote Memory.\n";
		if (!WriteProcessMemory(h_process, remote_char_ptr, dll_name, strlen(dll_name) + 1, nullptr)) {
			out_error();
			__leave;
		}
		std::cout << "Get LoadLibraryA Address.\n";
		thread_start_routine_ptr = reinterpret_cast<PTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA"));
		if (thread_start_routine_ptr == nullptr) {
			out_error();
			__leave;
		}
		std::cout << "Create Remote Thread.\n";
		h_remote_thread = CreateRemoteThread(h_process, nullptr, 0, thread_start_routine_ptr, remote_char_ptr, 0, nullptr);
		if (h_remote_thread == nullptr) {
			out_error();
			__leave;
		}
		std::cout << "Wait Remote Thread.\n";
		WaitForSingleObject(h_process, INFINITE);
		std::cout << "Success.\n";
	} __finally{
		if (remote_char_ptr) VirtualFreeEx(h_process, remote_char_ptr, 0, MEM_RELEASE);
		if (h_remote_thread) CloseHandle(h_remote_thread);
		if (h_process) CloseHandle(h_process);
	}
}


int main(int argc, char** argv) {

	if (argc != 3) {
		std::cerr 
		<< "Argument Error.\n"
		<< "Inject process_id dll_name\n";
		return 1;
	}

	DWORD id = atoi(argv[1]);
	if ((id == 0)) {
		id = GetCurrentProcessId();
		std::cout << "Self Injection. ProcessId = " << id << "\n";
	} else {
		HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
		if (h_process == nullptr) {
			out_error();
			return 1;
		}
		char name[MAX_PATH];
		GetModuleBaseName(static_cast<HMODULE>(h_process), nullptr, name, _countof(name));
		std::cout << "Cross Injection. ProcessId = " << id << " Target = " << name << "\n";
		CloseHandle(h_process);
	}
	inject(id, argv[2]);
}
