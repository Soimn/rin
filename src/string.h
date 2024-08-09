typedef struct String
{
  u8* data;
  u64 len;
} String;

#define STRING(S) (String){ .data = (S), .len = sizeof(S)-1 }
#define MS_STRING(S) { .data = (S), .len = sizeof(S)-1 }

inline bool
Char_IsAlpha(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= ('Z' - 'A'));
}

inline bool
Char_IsDigit(u8 c)
{
  return ((u8)(c - '0') < (u8)10);
}

inline bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF) - 'A') <= ('F' - 'A'));
}

inline u8
Char_UncheckedToUpper(u8 c)
{
  return c & 0xDF;
}

inline bool
String_Match(String s0, String s1)
{
  bool result = (s0.len == s1.len);

  for (umm i = 0; i < s0.len && result; ++i) result = (s0.data[i] == s1.data[i]);

  return result;
}
