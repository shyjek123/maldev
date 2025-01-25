// Gets the SSN of an NTAPI function
//Remember to include the required modules and definitions when using
DWORD GetSSN(IN HMODULE hNTDLL, IN LPCSTR NtFunction) {

	DWORD NtFunctionSSN = NULL;
	UINT_PTR NtFunctionAddress = NULL;

	info("trying to get the address of %s...", NtFunction);
	NtFunctionAddress = (UINT_PTR)GetProcAddress(hNTDLL, NtFunction);

	if (NtFunctionAddress == NULL) {
		warn("failed to get the address of %s", NtFunction);
		return NULL;
	}

	okay("got the address of %s!", NtFunction);
	info("getting SSN of %s...", NtFunction);
	NtFunctionSSN = ((PBYTE)(NtFunctionAddress + 4))[0];
	okay("\\___[\n\t| %s\n\t| 0x%p+0x4\n\t|____________________0x%lx]\n", NtFunction, NtFunctionAddress, NtFunctionSSN);
	return NtFunctionSSN;

}