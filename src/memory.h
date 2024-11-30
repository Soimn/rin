static void*
Align(void* ptr, u8 alignment)
{
  return (void*)(((umm)ptr + (alignment-1)) & ~(alignment-1));
}

static void
Copy(void* dst, void* src, umm size)
{
  for (umm i = 0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}

static void
Move(void* dst, void* src, umm size)
{
  u8* bdst = (u8*)dst;
  u8* bsrc = (u8*)src;

  if (bdst < bsrc) for (umm i = 0;      i < size; ++i) bdst[i] = bsrc[i];
  else             for (umm i = size-1; i < size; --i) bdst[i] = bsrc[i];
}

typedef struct
{
	u64 offset;
	u64 committed;
} *Arena;

void*
Arena_Push(Arena arena, umm size, u8 alignment)
{
	return 0;
}
