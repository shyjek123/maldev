//Opening handle to a process example, I used it to print its priority class int representation
/*Function Syntax:
    HANDLE OpenProcess(
    [in] DWORD dwDesiredAccess,
    [in] BOOL  bInheritHandle,
    [in] DWORD dwProcessId
    );
*/


#include <windows.h>
#include <stdio.h>

int main(int argc, char **argv) {

	if (argc < 2) {
		puts("usage: handles.exe <PID>");
		return -1;
	}

	DWORD PID = atoi(argv[1]);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);

	if (hProcess == NULL) {
		printf("[OpenProcess] failed, error: 0x%lx", GetLastError());
		return -1;
	}

	printf("[%p] got a handle to the process!", hProcess);
	CloseHandle(hProcess);
	(void)getchar();
	return 0;
}