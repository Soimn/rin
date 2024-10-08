
// Taken from http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-1a
// Licensed under CC0 Public Domain
static u64
FNV1A(String string)
{
  u64 hash = 14695981039346656037;

  for (umm i = 0; i < string.len; ++i)
  {
    hash ^= string.data[i];
    hash *= 1099511628211;
  }

  return hash;
}

typedef struct Interned_String
{
  u32 len;
  u8 data[];
} Interned_String;

typedef struct Ident_Entry
{
  u64 hash;
  u32 string_idx;
  u16 kind;
} Ident_Entry;

// NOTE: From scraping the Linux kernel and driver source code it seems like
//       4 million unique identifiers is a reasonable cap, doubling this
//       results in a memory footprint of ~128 MB, which is nothing. It
//       therefor seems reasonable to fix the hash table size to 8 M entries.
//       Especially since writing more than 48 Mloc of linux like code is
//       insane.
#define IDENT_TABLE_SIZE_LG2 23

typedef struct Ident_Table
{
  Arena* string_arena;
  u32 entry_count;
  Ident_Entry entries[1ULL << IDENT_TABLE_SIZE_LG2];
} Ident_Table;

#if 0

// TODO: temporary API
static Ident
IdentTable_Put(Ident_Table* table, String string, Token_Kind* kind)
{
  u64 hash = FNV1A(string);
  u64 idx  = ((hash >> IDENT_TABLE_SIZE_LG2) ^ hash) & ((1ULL << IDENT_TABLE_SIZE_LG2) - 1);

  Ident_Entry* entry = &table->entries[idx];

  while (entry->kind != 0)
  {
    if (entry->hash == hash)
    {
      Interned_String* ientry = (Interned_String*)Arena_GetBasePointer(table->string_arena) + entry->string_idx;
      
      String ientry_string = (String){ .data = ientry->data, .len = ientry->len };

      if (String_Match(ientry_string, string))
      {
        break;
      }
    }

    idx = (idx + 1) & ((1ULL << IDENT_TABLE_SIZE_LG2) - 1);
  }

  if (entry->kind == 0)
  {
    Interned_String* istring = Arena_Push(table->string_arena, sizeof(Interned_String) + string.len, _alignof(Interned_String));
    istring->len = (u32)string.len;
    Copy(istring->data, string.data, string.len);

    u32 string_idx = (u32)(istring - (Interned_String*)Arena_GetBasePointer(table->string_arena));

    *entry = (Ident_Entry){
      .hash       = hash,
      .string_idx = string_idx,
      .kind       = Token_Ident,
    };

    table->entry_count += 1;
    ASSERT(table->entry_count < 0.7f*(1ULL << IDENT_TABLE_SIZE_LG2));
  }

  *kind = table->entries[idx].kind;
  return (Ident)idx;
}

#else

#define IDENT_TABLE_BUCKET_SIZE_LG2 12

static Ident
IdentTable_Put(Ident_Table* table, String string, Token_Kind* kind)
{
  u64 hash = FNV1A(string);
  u64 idx  = ((hash >> IDENT_TABLE_BUCKET_SIZE_LG2) ^ hash) & ((1ULL << IDENT_TABLE_BUCKET_SIZE_LG2) - 1);

  for (;; ++idx)
  {
    Ident_Entry* entry = &table->entries[idx];

    if      (entry->kind == 0) break;
    else if (entry->hash == hash)
    {
      Interned_String* ientry = (Interned_String*)Arena_GetBasePointer(table->string_arena) + entry->string_idx;
      
      String ientry_string = (String){ .data = ientry->data, .len = ientry->len };

      if (String_Match(ientry_string, string))
      {
        break;
      }
    }
  }

  Ident_Entry* entry = &table->entries[idx];

  if (entry->kind == 0)
  {
    Interned_String* istring = Arena_Push(table->string_arena, sizeof(Interned_String) + string.len, _alignof(Interned_String));
    istring->len = (u32)string.len;
    Copy(istring->data, string.data, string.len);

    u32 string_idx = (u32)(istring - (Interned_String*)Arena_GetBasePointer(table->string_arena));

    *entry = (Ident_Entry){
      .hash       = hash,
      .string_idx = string_idx,
      .kind       = Token_Ident,
    };

    table->entry_count += 1;
    // NOTE: Ensure one spot is vacant at the end of each bucket, this is to avoid overflow into the next bucket
    ASSERT((idx & ((1ULL << IDENT_TABLE_BUCKET_SIZE_LG2) - 1)) < (1ULL << IDENT_TABLE_BUCKET_SIZE_LG2) - 2);
  }

  *kind = entry->kind;
  return (Ident)idx;
}

#endif

static Ident_Table*
IdentTable_Create(umm string_reserve_size)
{
  Ident_Table* table = ReserveMemory(sizeof(Ident_Table), true);
  table->string_arena = Arena_Create(string_reserve_size);
  table->entry_count  = 0;
  
  for (umm i = 0; i < ARRAY_SIZE(Token_KeywordStrings); ++i)
  {
    Ident ident = IdentTable_Put(table, Token_KeywordStrings[i], &(Token_Kind){0});
    table->entries[ident].kind = (u16)(Token__FirstKeyword + i);
  }

  Ident blank_ident = IdentTable_Put(table, STRING("_"), &(Token_Kind){0});
  table->entries[blank_ident].kind = Token_Blank;

  return table;
}

static void
IdentTable_Destroy(Ident_Table** table)
{
  Arena_Destroy(&(*table)->string_arena);
  ReleaseMemory(*table);
  *table = 0;
}
