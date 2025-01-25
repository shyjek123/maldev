#pragma once

#include <windows.h>
#include <stdio.h>
#include <ImageHlp.h>
#include <assert.h> 
#include <Psapi.h>
#include <winternl.h>

#pragma comment(lib, "imagehlp")
#pragma comment(lib, "ntdll")

#define okay(msg, ...) printf("[+]OKAY " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[i]INFO " msg "\n", ##__VA_ARGS__)
#define warn(msg, ...) printf("[!]ERROR " msg "\n", ##__VA_ARGS__)

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

BOOL HollowProc(
    char* processImage, 
    char* replacementImage, 
    wchar_t currentDirectoryPath
);

BOOL Get_SSN_SYSCALL(
    IN HMODULE hNTDLL,
    IN LPCSTR NtFunction,
    OUT DWORD* SSN,
    OUT UINT_PTR* Syscall
);

BOOL Cleanup(
    IN BOOL state
);
