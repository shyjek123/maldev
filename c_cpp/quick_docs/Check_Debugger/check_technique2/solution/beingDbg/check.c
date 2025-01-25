#include "headers.h"

int main(void) {
    hNtDLL = LoadLibraryA("ntdll.dll");

    if (!hNtDLL) {
        printf("failed to load NTDLL\n");
        return EXIT_FAILURE;
    }
    printf("Got a handle to NTDLL\n");

    PFN_NT_QUERY_INFORMATION_PROCESS NtQueryInformationProcess =
        (PFN_NT_QUERY_INFORMATION_PROCESS)GetProcAddress(hNtDLL, "NtQueryInformationProcess");

    if (!NtQueryInformationProcess) {
        printf("failed to retrieve the address to the NTAPI FUNCTION: NtQueryInformationProcess");
        FreeLibrary(hNtDLL);
        return EXIT_FAILURE;
    }

    HANDLE hProcess = GetCurrentProcess();
    PROCESS_BASIC_INFORMATION pbi;
    NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);

    if (status != STATUS_SUCCESS) {
        printf("Failed to retrieve info needed got a return status code of 0x%X\n", status);
        FreeLibrary(hNtDLL);
        return EXIT_FAILURE;
    }
    printf("Retrieved a successful status (0x%X)\n", status);

    PVOID pebAddress = pbi.PebBaseAddress;

    PPEB peb = (PPEB)pebAddress;
    
    if (peb->BeingDebugged != 0) {
        printf("being debbuged program shutting down");
        FreeLibrary(hNtDLL);
        return EXIT_FAILURE;
    }

    printf("Coast is clear\nPAYLOAD EXECUTING!");
    MessageBoxA(NULL, "Malware LOL", "I LOVE BUILDING MALWARE!", MB_ICONEXCLAMATION);
    FreeLibrary(hNtDLL);
    return EXIT_SUCCESS;
}