#include <stdio.h>
#include <stdint.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s64 smm;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u64 umm;

typedef u8 bool;
#define true 1
#define false 0

typedef float f32;
typedef double f64;

typedef struct String
{
	u8* data;
	u32 len;
} String;

#define MS_STRING(S) { .data = (u8*)(S), .len = sizeof(S)-1 }
#define STRING(S) (String){ .data = (u8*)(S), .len = sizeof(S)-1 }

bool
String_Match(String a, String b)
{
	bool result = (a.len == b.len);

	for (u64 i = 0; i < a.len && result; ++i) result = (a.data[i] == b.data[i]);

	return result;
}

#include "tokens.h"
#include "lexer.h"

int
main(int argc, char** argv)
{
	(void)argc, (void)argv;

	return 0;
}
