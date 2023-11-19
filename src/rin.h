typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

#define I8_MIN   (i8)(0x80)
#define I16_MIN (i16)(0x8000)
#define I32_MIN (i32)(0x80000000)
#define I64_MIN (i64)(0x8000000000000000)

#define I8_MAX   (i8)(0x7F)
#define I16_MAX (i16)(0x7FFF)
#define I32_MAX (i32)(0x7FFFFFFF)
#define I64_MAX (i64)(0x7FFFFFFFFFFFFFFF)

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#define U8_MAX   (u8)(0xFF)
#define U16_MAX (u16)(0xFFFF)
#define U32_MAX (u32)(0xFFFFFFFF)
#define U64_MAX (u64)(0xFFFFFFFFFFFFFFFF)

typedef i64 imm;
typedef u64 umm;

typedef u64 uint;

typedef u8 bool;
#define true 1
#define false 0

typedef float f32;
typedef double f64;

typedef struct String
{
	u8* data;
	umm size;
} String;

#define CONCAT__1(X, Y) X##Y
#define CONCAT__2(X, Y) CONCAT__1(X, Y)
#define CONCAT__3(X, Y) CONCAT__2(X, Y)
#define CONCAT__4(X, Y) CONCAT__3(X, Y)
#define CONCAT__5(X, Y) CONCAT__4(X, Y)
#define CONCAT__6(X, Y) CONCAT__5(X, Y)
#define CONCAT(X, Y) CONCAT__6(X, Y)

#define STATIC_ASSERT(EX) static struct { int static_assert_failed : ((EX) ? 1 : -1); } CONCAT(StaticAssert_, CONCAT(__LINE__, CONCAT(_, __COUNT__)))
#define ASSERT(EX) ((EX) ? 1 : (__debugbreak(), *(volatile int*)0 = 0))
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

typedef struct Identifier
{
	u32 _value;
} Identifier;

typedef struct String_Lit
{
	u32 _value;
} String_Lit;

#include "string.h"
#include "int.h"
#include "memory.h"
#include "lexer.h"
#include "ast.h"
