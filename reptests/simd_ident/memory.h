static void*
Align(void* p, u8 alignment)
{
	return (void*)(((umm)p + (alignment-1)) & -alignment);
}

#define ALIGN(N, A) (((umm)(N) + (umm)(A) - 1) & (umm)-(smm)(A))

#define PAGE_SIZE 4096

static void* ReserveMemory(umm bytes_to_reserve);
static void CommitMemory(void* commit_base, umm bytes_to_commit);
static void FreeMemory(void* reserve_base);

static void
Copy(void* dst, void* src, umm size)
{
	for (umm i = 0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}
