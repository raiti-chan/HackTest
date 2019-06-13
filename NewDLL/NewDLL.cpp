// NewDLL.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "NewDLL.h"

__declspec(dllexport)
int new_add(int a, int b) {
	return a + b;
}


__declspec(dllexport)
void new_sleep(DWORD m) {
	Sleep(m / 2);
}

