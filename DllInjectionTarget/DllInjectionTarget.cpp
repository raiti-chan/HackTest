#include "pch.h"
#include "../DefaultDLL/DefaultDLL.h"
#include <iostream>
#include <Windows.h>

int main() {
	int a, b;
loop:
	a = rand() % 10;
	b = rand() % 10;
	printf("%d + %d = %d\n", a, b, add(a, b));
	Sleep(200);
	goto loop;

}