#include "headers.h"

extern PTEB getTEB(void);
extern DWORD CustomError(void);

int main(void) {

	info("getting the TEB");
	PTEB pTEB = getTEB();
	okay("TEB: 0x%p", pTEB);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 4);

	if (hProcess == NULL) {
		okay("[OpenProcess] Failed (this is good)");
		info("[CustomError] Error : 0x%lx", CustomError());
		info("[GetLastError] Error: 0x%lx", GetLastError());
		if (CustomError() == GetLastError()) {
			okay("Value matches, custom error function working properly");
			return EXIT_SUCCESS;
		}
		warn("Values dont match: 0x%lx", GetLastError());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}