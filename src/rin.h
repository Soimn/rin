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

typedef struct String
{
	u8* data;
	umm size;
} String;

#include "string.h"
#include "memory.h"
#include "lexer.h"
