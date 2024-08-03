inline void*
Align(void* ptr, u8 alignment)
{
  return (void*)(((umm)ptr + (alignment-1)) & ~(umm)(alignment-1));
}

typedef struct Arena
{
  umm offset;
  umm committed;
  u8 memory[];
} Arena;

typedef struct Arena_Marker
{
  umm val;
} Arena_Marker;

static Arena*
Arena_Create()
{
  NOT_IMPLEMENTED;
}

static void
Arena_Destroy(Arena* arena)
{
  NOT_IMPLEMENTED;
}

static void*
Arena_Push(Arena* arena, umm size, u8 alignment)
{
  ASSERT(alignment != 0 && (alignment & (alignment-1)) == 0);

  arena->offset = (arena->offset + (alignment-1)) & ~(umm)(alignment-1);

  void* result = arena->memory + arena->offset;

  arena->offset += size;

  if (arena->offset > arena->committed)
  {
    NOT_IMPLEMENTED;
    CommitMemory(arena->memory + arena->committed, ((arena->offset - arena->committed) + (ARENA_PAGE_SIZE-1)) & ~(ARENA_PAGE_SIZE-1));
  }

  return result;
}

static void
Arena_Pop(Arena* arena, umm size)
{
  ASSERT(arena->offset >= size);
  arena->offset -= size;
}

static Arena_Marker
Arena_GetMarker(Arena* arena)
{
  return (Arena_Marker){ .val = arena->offset };
}

static void
Arena_PopToMarker(Arena* arena, Arena_Marker marker)
{
  ASSERT(arena->offset > marker.val);
  arena->offset = marker.val;
}

static void*
Arena_GetPointer(Arena* arena)
{
  return arena->memory + arena->offset;
}

static void
Arena_Clear(Arena* arena)
{
  arena->offset = 0;
}
