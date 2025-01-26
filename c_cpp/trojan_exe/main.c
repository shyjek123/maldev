#include <windows.h>
#pragma comment(lib, "user32.lib")

int main(void)
{
    MessageBox(NULL, "Hello World!", "Hello World!", MB_OK);
    return 0;
}

/*
This is just something I played around with to learn more about masking a potentially malicious exe or file, by being able to change the name or the icon, etd.
*/
