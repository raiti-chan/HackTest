// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "NewDLL.h"
#include <iostream>
#include <ImageHlp.h>

#pragma comment(lib, "imagehlp.lib")

bool update_iat(PIMAGE_IMPORT_DESCRIPTOR image_inport_descriptror_ptr, HMODULE h_module, char const* dll_name, char const* procname, void* new_proc) {
	PIMAGE_THUNK_DATA image_thunk_i_addres_t_ptr, image_thunk_i_name_t_ptr;
	PIMAGE_IMPORT_BY_NAME import_by_name_ptr;

	for (; image_inport_descriptror_ptr->Name; image_inport_descriptror_ptr++) {
		if (strcmp(reinterpret_cast<char*>(h_module + image_inport_descriptror_ptr->Name), dll_name) != 0) continue;

		image_thunk_i_addres_t_ptr = reinterpret_cast<PIMAGE_THUNK_DATA>(h_module + image_inport_descriptror_ptr->FirstThunk);
		image_thunk_i_name_t_ptr = reinterpret_cast<PIMAGE_THUNK_DATA>(h_module + image_inport_descriptror_ptr->OriginalFirstThunk);
		for (; image_thunk_i_addres_t_ptr->u1.Function; image_thunk_i_addres_t_ptr++, image_thunk_i_name_t_ptr++) {
			if (image_thunk_i_name_t_ptr->u1.AddressOfData & 0x8000000000000000) continue;

			import_by_name_ptr = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(h_module + image_thunk_i_name_t_ptr->u1.AddressOfData);
			if (strcmp(reinterpret_cast<char*>(import_by_name_ptr->Name), procname) == 0) {
				DWORD old_status;
				size_t size_function = sizeof(image_thunk_i_addres_t_ptr->u1.Function);
				VirtualProtect(&image_thunk_i_addres_t_ptr->u1.Function, size_function, PAGE_WRITECOPY, &old_status);
				CopyMemory(&image_thunk_i_addres_t_ptr->u1.Function, &new_proc, size_function);
				VirtualProtect(&image_thunk_i_addres_t_ptr->u1.Function, size_function, old_status, &old_status);
				return true;
			}
		}

	}
	return false;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call != DLL_PROCESS_ATTACH) return true;
	
	HMODULE h_module = GetModuleHandle("DllInjectionTarge.exe");
	if (h_module == nullptr) {
		std::cerr << "Not Found TargetModule!\n";
		return false;
	}
	ULONG size;
	PIMAGE_IMPORT_DESCRIPTOR p_image_import_descriptor
		= reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(ImageDirectoryEntryToDataEx(h_module, true, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, nullptr));
	if (p_image_import_descriptor) {
		std::cerr << "Error! ImageDirectoryEntryToDataEx\n";
		return false;
	}
	update_iat(p_image_import_descriptor, h_module, "DefaultDLL.dll", "?add@@YAHHH@Z", new_add);
	update_iat(p_image_import_descriptor, h_module, "KERNEL32.dll", "Sleep", new_sleep);
	return true;
}

