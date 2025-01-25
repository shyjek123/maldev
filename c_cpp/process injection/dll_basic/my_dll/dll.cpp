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

