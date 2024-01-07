static void*
Align(void* ptr, u8 alignment)
{
  ASSERT(IS_POW2(alignment));
  return (void*)(((umm)ptr + (alignment-1)) & ~(alignment-1));
}

static void
Copy(void* src, void* dst, umm size)
{
  for (uint i = 0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}

static void
Move(void* src, void* dst, umm size)
{
  if ((u8*)src < (u8*)dst) for (uint i = size-1; i < size; --i) ((u8*)dst)[i] = ((u8*)src)[i];
  else                     for (uint i =      0; i < size; ++i) ((u8*)dst)[i] = ((u8*)src)[i];
}

static void
Zero(void* ptr, umm size)
{
  for (uint i = 0; i < size; ++i) ((u8*)ptr)[i] = 0;
}

#define ZeroStruct(S) Zero((S), sizeof(0[S]))

typedef struct Arena_
{
  u64 cursor;
  u64 committed;
  u64 reserved;
  u8 memory[];
} Arena_;

typedef void Arena;
typedef void* Arena_Marker;

void
Arena__CommitMoreMemory(Arena_* arena)
{
  __declspec(dllimport) void* VirtualAlloc(void*, unsigned __int64, unsigned long, unsigned long);

  ASSERT(UMM_MAX - arena->cursor > 4096);
  void* result = VirtualAlloc(arena, arena->cursor + 4096, 0x00001000/*MEM_COMMIT*/, 0x04/*PAGE_READWRITE*/);

  // TODO: handle oom
  ASSERT(result != 0);
}

Arena*
Arena_Create()
{
}

void
Arena_Destroy()
{
}

void*
Arena_Push(Arena* arena_handle, umm size, u8 alignment)
{
  ASSERT(size != 0);
  ASSERT(IS_POW2(alignment));

  Arena_* arena = arena_handle;

  umm aligned_cursor = (arena->cursor + (alignment-1)) & ~(alignment-1);
  ASSERT(size < UMM_MAX - aligned_cursor);

  void* result  = &arena->memory[aligned_cursor];
  arena->cursor = aligned_cursor + size;

  ASSERT(arena->cursor <= arena->reserved);
  if (arena->cursor > arena->committed) Arena__CommitMoreMemory(arena);

  return result;
}

void*
Arena_PushBytes(Arena* arena_handle, umm size)
{
  ASSERT(size != 0);

  Arena_* arena = arena_handle;

  void* result = &arena->memory[arena->cursor];
  arena->cursor += size;

  ASSERT(arena->cursor <= arena->reserved);
  if (arena->cursor > arena->committed) Arena__CommitMoreMemory(arena);

  return result;
}

void*
Arena_PushAlignment(Arena* arena_handle, u8 alignment)
{
  ASSERT(IS_POW2(alignment));

  Arena_* arena = arena_handle;

  umm aligned_cursor = (arena->cursor + (alignment-1)) & ~(alignment-1);

  void* result = &arena->memory[aligned_cursor];
  arena->cursor = aligned_cursor;

  ASSERT(arena->cursor <= arena->reserved);
  if (arena->cursor > arena->committed) Arena__CommitMoreMemory(arena);

  return result;
}

void
Arena_Pop(Arena* arena_handle, umm size)
{
  Arena_* arena = arena_handle;

  ASSERT(size <= arena->cursor - sizeof(Arena));
  arena->cursor -= size;
}

Arena_Marker
Arena_GetMarker(Arena* arena_handle)
{
  Arena_* arena = arena_handle;

  return (Arena_Marker)arena->cursor;
}

void
Arena_PopToMarker(Arena* arena_handle, Arena_Marker marker)
{
  Arena_* arena = arena_handle;

  ASSERT(arena->cursor >= (u64)marker);
  arena->cursor = (u64)marker;
}

void
Arena_Clear(Arena* arena_handle)
{
  Arena_* arena = arena_handle;

  arena->cursor = sizeof(Arena);
}
