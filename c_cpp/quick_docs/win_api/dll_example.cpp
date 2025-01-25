//How to create a standard DLL (Dynamic-Link-Library)
//It is just a switch statement that calls a piece of code to run in response to a certain action (Reason).
//Like a messagebox or some other piece of code that may be more malicous, like one
//that deletes the computer's operating system ;)
#include "windows.h"

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  Reason, LPVOID lpReserved) {
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxW(
            NULL,
            L"Hello world",
            L"my first DLL to inject :)",
            MB_YESNOCANCEL
        );
    }
    return TRUE;
}

