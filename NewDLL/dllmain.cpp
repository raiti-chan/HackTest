// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "NewDLL.h"
#include <iostream>
#include <ImageHlp.h>

#pragma comment(lib, "imagehlp.lib")

void out_error() {
	char* p_msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&p_msg_buf), 0, nullptr);
	std::cout << "Error : " << p_msg_buf << "\n";
	LocalFree(p_msg_buf);
}


bool update_iat(PIMAGE_IMPORT_DESCRIPTOR image_import_descriptor_ptr, HMODULE h_module, char const* dll_name, char const* proc_name, void* new_proc) {
	std::cout << "update_iat " << dll_name << " " << proc_name << "\n";
	char* h_module_address = reinterpret_cast<char*>(h_module);
	for (; image_import_descriptor_ptr->Name; image_import_descriptor_ptr++) {
		if (strcmp(h_module_address + image_import_descriptor_ptr->Name, dll_name) != 0) continue;

		PIMAGE_THUNK_DATA image_thunk_i_address_t_ptr = reinterpret_cast<PIMAGE_THUNK_DATA>(h_module_address + image_import_descriptor_ptr->FirstThunk);
		PIMAGE_THUNK_DATA image_thunk_i_name_t_ptr = reinterpret_cast<PIMAGE_THUNK_DATA>(h_module_address + image_import_descriptor_ptr->OriginalFirstThunk);
		for (; image_thunk_i_address_t_ptr->u1.Function; image_thunk_i_address_t_ptr++, image_thunk_i_name_t_ptr++) {
			if (image_thunk_i_name_t_ptr->u1.AddressOfData & 0x8000000000000000) continue;

			PIMAGE_IMPORT_BY_NAME import_by_name_ptr = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(h_module_address + image_thunk_i_name_t_ptr->u1.AddressOfData);
			if (strcmp(reinterpret_cast<char*>(import_by_name_ptr->Name), proc_name) == 0) {
				DWORD old_status;
				size_t size_function = sizeof(image_thunk_i_address_t_ptr->u1.Function);
				if (!VirtualProtect(&image_thunk_i_address_t_ptr->u1.Function, size_function, PAGE_WRITECOPY, &old_status)) {
					out_error();
					return false;
				}
				CopyMemory(&image_thunk_i_address_t_ptr->u1.Function, &new_proc, size_function);
				if (!VirtualProtect(&image_thunk_i_address_t_ptr->u1.Function, size_function, old_status, &old_status)) {
					out_error();
					return false;
				}
				std::cout << "Success.\n";
				return true;
			}
		}

	}
	std::cerr << "failed.\n";
	return false;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call != DLL_PROCESS_ATTACH) return true;

	std::cout << "Get Module Handle.\n";
	HMODULE h_module = GetModuleHandle("DllInjectionTarget.exe");
	std::cout << "Module Base Address : " << std::hex << h_module << "\n";
	if (h_module == nullptr) {
		out_error();
		return false;
	}
	ULONG size;
	std::cout << "ImageDirectoryEntryToDataEx.\n";
	PIMAGE_IMPORT_DESCRIPTOR p_image_import_descriptor
		= reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToDataEx(h_module, true, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, nullptr));
	std::cout << "IMAGE_IMPORT_DESCRIPTOR Address : " << std::hex << p_image_import_descriptor << "\n";
	if (p_image_import_descriptor == nullptr) {
		out_error();
		return false;
	}
	update_iat(p_image_import_descriptor, h_module, "DefaultDLL.dll", "?add@@YAHHH@Z", &new_add);
	update_iat(p_image_import_descriptor, h_module, "KERNEL32.dll", "Sleep", &new_sleep);
	return true;
}

