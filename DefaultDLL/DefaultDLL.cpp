// DefaultDLL.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

__declspec(dllexport)
int add(int a, int b) {
	return a - b;
}
