typedef struct String
{
  u8* data;
  u64 len;
} String;

#define STRING(S) (String){ .data = (S), .len = sizeof(S)-1 }
#define MS_STRING(S) { .data = (S), .len = sizeof(S)-1 }

static bool
Char_IsAlpha(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= ('Z' - 'A'));
}

static bool
Char_IsDigit(u8 c)
{
  return ((u8)(c - '0') < (u8)10);
}

static bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= ('F' - 'A'));
}

static u8
Char_UncheckedToUpper(u8 c)
{
  return c & 0xDF;
}

static umm
CString_Len(u8* cstring)
{
  umm result = 0;

  for (u8* scan = cstring; *scan != 0; ++scan) ++result;

  return result;
}

static bool
CString_Match(u8* cs0, u8* cs1)
{
  umm i = 0;

  while (cs0[i] != 0 && cs0[i] == cs1[i]) ++i;

  return (cs0[i] == cs1[i]);
}

static bool
String_Match(String s0, String s1)
{
  bool result = (s0.len == s1.len);

  for (umm i = 0; i < s0.len && result; ++i) result = (s0.data[i] == s1.data[i]);

  return result;
}
