#pragma once
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

typedef NTSTATUS(NTAPI* PFN_NT_QUERY_INFORMATION_PROCESS)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

HMODULE hNtDLL;