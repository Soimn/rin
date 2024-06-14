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

#define ASSERT_MSG(EX, MSG, ...) ((EX) ? 1 : (AssertHandler(__FILE__, __LINE__, #EX, (MSG),##__VA_ARGS__), __debugbreak(), 0))
#define ASSERT(EX) ASSERT_MSG(EX, "")
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

void AssertHandler(const char* file, int line, const char* expr);
