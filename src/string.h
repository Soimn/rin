static inline bool
Char_IsAlpha(u8 c)
{
  return (u8)((c&0xDF) - 'A') <= (u8)('Z' - 'A');
}

static inline bool
Char_IsDigit(u8 c)
{
  return (u8)(c - '0') < (u8)10;
}

static bool
String_Match(String s0, String s1)
{
  bool result = (s0.size == s1.size);

  for (umm i = 0; i < s0.size && result; ++i) result = (s0.data[i] == s1.data[i]);

  return result;
}
