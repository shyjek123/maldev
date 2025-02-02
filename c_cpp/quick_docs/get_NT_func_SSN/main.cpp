#include "header.h"

// Gets the SSN of an NTAPI function
// Remember to include the required modules and definitions when using
DWORD GetSSN(IN HMODULE hNTDLL, IN LPCSTR NtFunction)
{

	DWORD NtFunctionSSN = NULL;
	UINT_PTR NtFunctionAddress = NULL;

	info("trying to get the address of %s...", NtFunction);
	NtFunctionAddress = (UINT_PTR)GetProcAddress(hNTDLL, NtFunction);

	if (NtFunctionAddress == NULL)
	{
		warn("failed to get the address of %s", NtFunction);
		return NULL;
	}

	okay("got the address of %s!", NtFunction);
	info("getting SSN of %s...", NtFunction);
	NtFunctionSSN = ((PBYTE)(NtFunctionAddress + 4))[0];
	okay("\\___[\n\t| %s\n\t| 0x%p+0x4\n\t|____________________0x%lx]\n", NtFunction, NtFunctionAddress, NtFunctionSSN);
	return NtFunctionSSN;
}

// Gets the SSN and the Syscall addr of an NTAPI function
// note: a syscall is a syscall so a syscall for NtOPenProcess will work the same as a syscall for NtCreateThread
VOID Get_SSN_SYSCALL(
	IN HMODULE hNTDLL,
	IN LPCSTR NtFunction,
	OUT DWORD *SSN,
	OUT UINT_PTR *Syscall)
{

	UINT_PTR NtFunctionAddress = NULL;
	BYTE SyscallOpcode[2] = {0x0F, 0x05};

	info("beginning indirect prelude...");
	info("trying to get the address of %s...", NtFunction);
	NtFunctionAddress = (UINT_PTR)GetProcAddress(hNTDLL, NtFunction);

	if (NtFunctionAddress == NULL)
	{
		warn("[GetProcAddress] failed, error: 0x%lx", GetLastError());
		return NULL;
	}

	okay("got the address of %s! (0x%p)", NtFunction, NtFunctionAddress);
	*SSN = ((PBYTE)(NtFunctionAddress + 4))[0];
	*Syscall = NtFunctionAddress + 0x12;

	if (memcmp(SyscallOpcode, *Syscall, sizeof(SyscallOpcode)) == 0)
	{
		okay("syscall signature (0x0F, 0x05) matched, found a valid syscall instruction!");
	}
	else
	{
		warn("expected syscall signature: 0x0f,0x05 didn't match.");
		return NULL;
	}

	okay("got the SSN of %s (0x%lx)", NtFunction, *SSN);
	printf("\n\t| %s ", NtFunction);
	printf("\n\t|\n\t| ADDRESS\t| 0x%p\n\t| SYSCALL\t| 0x%p\n\t| SSN\t\t| 0x%lx\n\t|____________________________________\n\n", NtFunctionAddress, *Syscall, *SSN);
}

/*
Example of how .asm file definition looks like:

define that it is data, var must be extern type
.data
EXTERN NtCreateThreadExSSN:DWORD
mimic the syscall instruction and place the name with the second mov instruction
.code
NtCreateThreadEx PROC
		mov r10, rcx
		mov eax, NtCreateThreadExSSN
		syscall
		ret
NtCreateThreadEx ENDP

make sure the varibale is also defined in the file where the SSN function is used like this:
DWORD NtCreateThreadExSSN;

Example of how .asm file definition looks like:

define that it is data, var must be extern type
.data
EXTERN NtCreateThreadExSSN:DWORD
EXTERN mySyscall:QWORD

place the SSN within the second mov instruction and jump to the syscall address and place the SSN within the second mov instruction

.code
NtCreateThreadEx PROC
		mov r10, rcx
		mov eax, NtCreateThreadExSSN
		jmp qword ptr [mySyscall]
		ret
NtCreateThreadEx ENDP

make sure the varibale is also defined in the file where the SSN function is used like this:
DWORD NtCreateThreadExSSN;
QWORD mySyscall;
*/