#include <Windows.h>
#include <iostream>

int main() {
	HMODULE dllDemo = LoadLibraryA("DllDemo.dll");
	if (!dllDemo) {
		std::cout << "Can't find DLL!, expecting DLL named DllDemo.dll";
		return -1;
	}

	FARPROC dllMain = GetProcAddress(dllDemo, "main");
	dllMain();
	return 0;
}