bool
Char_IsDigit(u8 c)
{
	return ((u8)(c - '0') < (u8)10);
}

bool
Char_IsAlpha(u8 c)
{
	return ((u8)((c & 0xDF) - 'A') <= (u8)('Z' - 'A'));
}

typedef struct String
{
	u8* data;
	u32 len;
} String;

#define MS_STRING(S) { .data = (u8*)(S), .len = sizeof(S)-1 }
#define STRING(S) (String){ .data = (u8*)(S), .len = sizeof(S)-1 }

bool
String_Equal(String a, String b)
{
	bool result = (a.len == b.len);

	for (u64 i = 0; i < a.len && result; ++i) result = (a.data[i] == b.data[i]);

	return result;
}
