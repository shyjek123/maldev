#include "headers.h"


extern PPEB GetPEB(void);
extern BYTE pebPatcher(void);

int main(void) {
	info("Getting the PEB");

	PPEB pPEB = GetPEB();
	okay("PEB: 0x%p", pPEB);

	if (pPEB->BeingDebugged != 0) {
		info("[PEB->BeingDebugged: 0x%d]", pPEB->BeingDebugged);
		warn("debugger detected program shutting down");
		info("patching the PEB");
		pebPatcher();

		if (pPEB->BeingDebugged != 0) {
			warn("something went wrong");
			return EXIT_FAILURE;
		}
		okay("PEB should have been patched");
		return EXIT_FAILURE;
	}
	info("[PEB->BeingDebugged: 0x%d]", pPEB->BeingDebugged);
	MessageBoxA(NULL, "WASSUP", "HELLO WORLD", MB_ICONEXCLAMATION);
	return EXIT_SUCCESS;
}