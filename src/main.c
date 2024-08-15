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

void*
ReserveMemory(umm size, bool do_commit)
{
  void* memory = VirtualAlloc(0, size, MEM_RESERVE | (do_commit ? MEM_COMMIT : 0), PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(memory != 0);

  return memory;
}

void
CommitMemory(void* base, umm size)
{
  void* result = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE);

  // TODO: Handle oom
  ASSERT(result != 0);
}

int
main(int argc, char** argv)
{
  Ident_Table* ident_table = IdentTable_Init();
  return 0;
}
