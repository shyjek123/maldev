//How to use message box function
/*
URL: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox
    Function Syntax:
        int MessageBox(
            [in, optional] HWND    hWnd,
            [in, optional] LPCTSTR lpText,
            [in, optional] LPCTSTR lpCaption,
            [in]           UINT    uType
            );
*/
#include <windows.h>
int main(void) {
    MessageBoxW(
        NULL,
        L"Hello world",
        L"my first message box",
        MB_YESNOCANCEL
        );
    return EXIT_SUCCESS;
}