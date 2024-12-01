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

#define ARENA_COMMIT_PACE (1ULL << 14)

typedef struct
{
	u64 offset;
	u64 committed;
	u64 reserved;
} *Arena;

Arena
Arena_Create(umm reserve_size)
{
	u64 reserved = (reserve_size + (1ULL << 16) - 1) & -(s64)(1ULL << 16);
	
	Arena arena = ReserveMemory(reserved);
	CommitMemory(arena, ARENA_COMMIT_PACE);
	arena->offset    = sizeof(*arena);
	arena->committed = ARENA_COMMIT_PACE;
	arena->reserved  = reserved;

	return arena;
}

void
Arena_Destroy(Arena* arena)
{
	ReleaseMemory(*arena);
	*arena = 0;
}

void*
Arena_Push(Arena arena, umm size, u8 alignment)
{
	ASSERT(size < (1ULL << 48));

	umm aligned_offset = (arena->offset + (alignment-1)) & -alignment;

	arena->offset = aligned_offset + size;
	if (arena->offset > arena->committed)
	{
		void* base = (u8*)arena + arena->committed;
		umm to_commit = ARENA_COMMIT_PACE;

		arena->committed += to_commit;
		ASSERT(arena->committed <= arena->reserved); // NOTE: Maybe check this in release too, since accidentally committing past reserve is a nasty bug

		CommitMemory(base, to_commit);
	}

	return (u8*)arena + aligned_offset;
}

typedef struct Arena_Marker
{
	u64 _value;
} Arena_Marker;

Arena_Marker
Arena_GetMarker(Arena arena)
{
	return (Arena_Marker){ arena->offset };
}

void
Arena_PopToMarker(Arena arena, Arena_Marker marker)
{
	ASSERT(arena->offset >= marker._value);
	arena->offset = marker._value;
}

void*
Arena_GetBasePointer(Arena arena)
{
	return (u8*)arena + sizeof(*arena);
}

void*
Arena_GetPointer(Arena arena)
{
	return (u8*)arena + arena->offset;
}

void
Arena_Clear(Arena arena)
{
	arena->offset = sizeof(*arena);
}
