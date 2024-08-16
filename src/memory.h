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

typedef struct Arena
{
  u64 offset;
  u64 committed;
  u8 memory[];
} Arena;

// NOTE: Must be a power of 2
#define ARENA_COMMIT_PACE (1ULL << 14)

typedef struct { u64 value; } Arena_Marker;

static Arena*
Arena_Create(umm reserve_size)
{
  Arena* arena = ReserveMemory(reserve_size, false);

  u64 to_commit = 4096;
  CommitMemory(arena, to_commit);

  *arena = (Arena){
    .offset    = 0,
    .committed = to_commit - sizeof(Arena),
  };

  return arena;
}

static void
Arena_Destroy(Arena** arena)
{
  ReleaseMemory(*arena);
  *arena = 0;
}

static void*
Arena_Push(Arena* arena, umm size, u8 alignment)
{
  u64 aligned_offset = (arena->offset + (u64)(alignment-1)) & ~(u64)(alignment-1);

  arena->offset = aligned_offset + size;

  if (arena->offset > arena->committed)
  {
    u64 to_commit = ((arena->offset - arena->committed) + (ARENA_COMMIT_PACE-1)) & ~(ARENA_COMMIT_PACE-1);

    CommitMemory(arena->memory + arena->committed, to_commit);

    arena->committed += to_commit;
  }

  return arena->memory + aligned_offset;
}

static Arena_Marker
Arena_GetMarker(Arena* arena)
{
  return (Arena_Marker){ .value = arena->offset };
}

static void
Arena_PopToMarker(Arena* arena, Arena_Marker marker)
{
  ASSERT(marker.value < arena->offset);
  arena->offset = marker.value;
}

static void
Arena_Clear(Arena* arena)
{
  arena->offset = 0;
}

static void*
Arena_GetBasePointer(Arena* arena)
{
  return arena->memory;
}

static void*
Arena_GetPointer(Arena* arena)
{
  return arena->memory + arena->offset;
}
