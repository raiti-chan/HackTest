// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include <tuple>
#include <tchar.h>

WNDPROC default_procedure_ptr = nullptr;

HWND find_main_window(DWORD process_id) {
	using type_enum_window_lparam = std::tuple<DWORD, HWND*>;

	HWND h_mainwindow = nullptr;
	type_enum_window_lparam lparam_value = { process_id, &h_mainwindow };

	EnumWindows([](HWND h_window, LPARAM lparam)->BOOL {
		auto[lv_target_processid, lv_p_result_h_mainwindow] = *(reinterpret_cast<type_enum_window_lparam*>(lparam));
		DWORD lv_process_id = 0;
		GetWindowThreadProcessId(h_window, &lv_process_id);
		if (lv_process_id == lv_target_processid) {
			TCHAR target_windowclass[64];
			RealGetWindowClass(h_window, target_windowclass, 64);
			if (lstrcmpW(target_windowclass, _T("Notepad"))) return true;
			*lv_p_result_h_mainwindow = h_window;
			return false;
		}
		return true;
	}, reinterpret_cast<LPARAM>(&lparam_value));

	return h_mainwindow;
}

LRESULT CALLBACK new_proc(HWND h_window, UINT param, WPARAM w_param, LPARAM l_param) {
	std::cout << "new_proc\n";
	return default_procedure_ptr(h_window, param, w_param, l_param);
}

BOOL APIENTRY DllMain(HMODULE, DWORD  ul_reason_for_call, LPVOID) {
	FILE *out = nullptr, *err = nullptr;
	bool return_flag = false;
	__try {
		if (ul_reason_for_call != DLL_PROCESS_ATTACH) __leave;
		DWORD process_id = GetCurrentProcessId();
		AllocConsole();
		freopen_s(&out, "CONOUT$", "w", stdout);
		freopen_s(&err, "CONOUT$", "w", stderr);

		std::cout << "Current ProcessID : " << process_id << " hex = " << std::hex << process_id << "\n";
		
		HWND h_window = find_main_window(process_id);
		if (h_window == nullptr) {
			std::cerr << "Not found MainWindow!\n ";
			__leave;
		}
		std::cout << "MainWindow Handle : " << std::hex << h_window << "\n";

		HMENU h_menu = GetMenu(h_window);
		if (h_menu == nullptr) {
			std::cerr << "Not found menu!\n";
			__leave;
		}
		std::cout << "Menu Handle : " << std::hex << h_menu << "\n";

		HMENU h_menu_1 = GetSubMenu(h_menu, 0);
		if (h_menu_1 == nullptr) {
			std::cerr << "Not found menu_1!\n";
			__leave;
		}
		std::cout << "Menu_1 Handle : " << std::hex << h_menu_1 << "\n";
		
		if (!InsertMenu(h_menu_1, 10, MF_BYPOSITION | MFT_STRING, 128, _T("NullNull"))) {
			std::cerr << "Failed Insert Menu!\n";
			__leave;
		}
		std::cout << "Success Insert Menu!\n";

		default_procedure_ptr = reinterpret_cast<WNDPROC>(GetClassLongPtr(h_window, GCLP_WNDPROC));
		if (default_procedure_ptr == nullptr) {
			std::cerr << "Failed GetClassLongPtr!\n";
			__leave;
		}
		std::cout << "Default Proc ptr : " << std::hex << reinterpret_cast<void*>(default_procedure_ptr) << "\n";

		LONG_PTR old_value = SetClassLongPtr(h_window, GCLP_WNDPROC, reinterpret_cast<LONG_PTR>(&new_proc));
		if (old_value == 0) {
			std::cerr << "Failed SetClassLongPtr!\n";
			__leave;
		}
		std::cout << "Success Switch WindowProcedure : " << std::hex << old_value << " -> " << std::hex << reinterpret_cast<void*>(&new_proc) << "\n";

		std::cout << SetClassLongPtr(h_window, GCLP_HCURSOR, 0);

		return_flag = true;

	} __finally {
		//system("pause");
		//if (out) fclose(out);
		//if (err) fclose(err);
		//FreeConsole();
	}
	return return_flag;
}

