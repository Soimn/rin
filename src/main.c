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

#include <stdio.h>

#include "rin.h"

void
AssertHandler(char* file, int line, char* expr)
{
	fprintf(stderr, "ASSERTION FAILED\n%s(%d): %s\n", file, line, expr);
	ExitProcess(-1);
}

void*
ReserveMemory(umm size)
{
	void* result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);

	if (result == 0)
	{
		//// ERROR: Out of memory
		ExitProcess(-1);
	}

	return result;
}

void
CommitMemory(void* base, umm size)
{
	if (VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE) == 0)
	{
		//// ERROR: Out of memory
		ExitProcess(-1);
	}
}

void
ReleaseMemory(void* base)
{
	VirtualFree(base, 0, MEM_RELEASE);
}

int
main(int argc, char** argv)
{
	return 0;
}
