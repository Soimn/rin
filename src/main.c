#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include "rin.h"

#include <stdio.h>

static void
AssertHandler(char* file, int line, char* expr)
{
	char message[1024];

	snprintf(message, sizeof(message), "%s(%d): %s", file, line, expr);

	MessageBoxA(0, message, "Assert", MB_OK | MB_ICONERROR);
	ExitProcess(-1);
}

static void*
ReserveMemory(umm size, bool do_commit)
{
  void* memory = VirtualAlloc(0, size, MEM_RESERVE | (do_commit ? MEM_COMMIT : 0), PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(memory != 0);

  return memory;
}

static void
CommitMemory(void* base, umm size)
{
  void* result = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(result != 0);
}

static void
ReleaseMemory(void* reserve_base)
{
  VirtualFree(reserve_base, 0, MEM_RELEASE);
}

int
main(int argc, char** argv)
{
  return 0;
}
