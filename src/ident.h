// TODO: revisit space vs lookup speed tradeoff, use better hash
typedef struct Ident
{
	u8* data;
	u32 len;
	u64 hash;
} Ident;

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

Ident
Ident_FromString(String string)
{
	return (Ident){ .data = string.data, .len = string.len, .hash = FNV1A(string) };
}

bool
Ident_Match(Ident a, Ident b)
{
	bool result = (a.len == b.len && a.hash == b.hash);

	for (u32 i = 0; result && i < a.len; ++i) result = (a.data[i] == b.data[i]);

	return result;
}
