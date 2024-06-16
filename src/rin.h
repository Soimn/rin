typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

#define S8_MIN  0x80
#define S16_MIN 0x8000
#define S32_MIN 0x80000000
#define S64_MIN 0x8000000000000000
#define S8_MAX  0x7F
#define S16_MAX 0x7FFF
#define S32_MAX 0x7FFFFFFF
#define S64_MAX 0x7FFFFFFFFFFFFFFF

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

typedef s64 smm;
typedef u64 umm;

#define SMM_MIN S64_MIN
#define SMM_MAX S64_MAX
#define UMM_MAX U64_MAX

typedef u8 bool;
#define true 1
#define false 0

typedef float f32;
typedef double f64;

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

#define ASSERT_MSG(EX, MSG, ...) ((EX) ? 1 : (AssertHandler(__FILE__, __LINE__, #EX, (MSG),##__VA_ARGS__), __debugbreak(), 0))
#define ASSERT(EX) ASSERT_MSG(EX, "")
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

void AssertHandler(const char* file, int line, const char* expr, const char* message, ...);

typedef struct String
{
  u8* data;
  umm len;
} String;

#define STRING(S) (String){ .data = (u8*)(S), .len = sizeof(S)-1 }
#define COMPSTRING(S) { .data = (u8*)(S), .len = sizeof(S)-1 }

bool
String_Match(String a, String b)
{
  bool does_match = (a.len == b.len);

  for (umm i = 0; i < a.len && does_match; ++i) does_match = (a.data[i] == b.data[i]);

  return does_match;
}

bool
Char_IsAlpha(u8 c)
{
  return ((u8)((c&0xDF)-'A') <= (u8)('Z'-'A'));
}

bool
Char_IsDigit(u8 c)
{
  return ((u8)(c-'0') < (u8)10);
}

bool
Char_IsHexAlphaDigit(u8 c)
{
  return ((u8)((c&0xDF)-'A') <= (u8)('F'-'A'));
}

bool
Char_IsHexDigit(u8 c)
{
  return (Char_IsDigit(c) || Char_IsHexAlphaDigit(c));
}

typedef union F32_Bits
{
  f32 f;
  u32 bits;
} F32_Bits;

typedef union F64_Bits
{
  f64 f;
  u64 bits;
} F64_Bits;
