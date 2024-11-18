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

// NOTE: From scraping the Linux kernel and driver source code it seems like
//       4 million unique identifiers is a reasonable cap, doubling this
//       results in a memory footprint of ~128 MB, which is nothing. It
//       therefor seems reasonable to fix the hash table size to 8 M entries.
//       Especially since writing more than 48 Mloc of linux like code is
//       insane.
#define IDENT_TABLE_SIZE_LG2 23
#define IDENT_TABLE_MASK ((1ULL << IDENT_TABLE_SIZE_LG2) - 1)

typedef struct Interned_String
{
	u32 len;
	u8 data[];
} Interned_String;

static bool
InternedString_MatchString(Interned_String* s0, String s1)
{
	bool result = (s0->len == s1.len);

	for (u32 i = 0; i < s0->len && result; ++i) result = (s0->data[i] == s1.data[i]);

	return result;
}

typedef struct Ident_Table_Entry
{
	u64 hash;
	u32 string_idx;
	Token_Kind kind;
} Ident_Table_Entry;

typedef struct Ident_Table
{
	Arena* string_arena;
	Interned_String* strings;
	Ident_Table_Entry entries[1 << IDENT_TABLE_SIZE_LG2];
} Ident_Table;

u64
IdentTable_Hash(String string)
{
	return FNV1A(string);
}

Ident
IdentTable_Put(Ident_Table* table, u64 hash, String string, Token_Kind* kind)
{
	u32 idx = hash & IDENT_TABLE_MASK;
	if (hash == 0) hash = 1;

	u32 step = 1;
	while (table->entries[idx].hash != 0)
	{
		if (InternedString_MatchString(table->strings + table->entries[idx].string_idx, string))
		{
			break;
		}

		idx = (idx + step++) & IDENT_TABLE_MASK;
	}

	if (table->entries[idx].hash == 0)
	{
		Interned_String* s = Arena_Push(table->string_arena, sizeof(Interned_String) + string.len, ALIGNOF(Interned_String));
		s->len = string.len;
		Copy(s->data, string.data, s->len);

		table->entries[idx] = (Ident_Table_Entry){
			.hash       = hash,
			.string_idx = (u32)(s - table->strings),
			.kind       = Token_Ident,
		};
	}

	*kind = table->entries[idx].kind;

	return (Ident)idx;
}

Ident_Table*
IdentTable_Init(Arena* arena, Arena* string_arena)
{
	Ident_Table* table = Arena_Push(arena, sizeof(Ident_Table), ALIGNOF(Ident_Table));
	*table = (Ident_Table){
		.string_arena = string_arena,
		.strings      = Arena_Push(string_arena, 0, ALIGNOF(Interned_String)),
		.entries      = {0},
	};

	table->entries[IdentTable_Put(table, IdentTable_Hash(STRING("_")), STRING("_"), &(Token_Kind){0})].kind = Token_Blank;

	for (umm i = 0; i < ARRAY_SIZE(Token_KeywordStrings); ++i)
	{
		String s = Token_KeywordStrings[i];
		table->entries[IdentTable_Put(table, IdentTable_Hash(s), s, &(Token_Kind){0})].kind = Token__FirstKeyword + i;
	}

	return table;
}
