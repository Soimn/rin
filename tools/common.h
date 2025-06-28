#pragma once

#include <windows.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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

typedef float f32;
typedef double f64;

typedef u8 bool;
#define true 1
#define false 0

#define ASSERT(EX) ((EX) ? 1 : (IsDebuggerPresent() ? (*(volatile int*)0 = 0) : (AssertionFailed(__FILE__, __LINE__, #EX), 0)))
void AssertionFailed(char* file, int line, char* expr)
{
	fprintf(stderr, "%s(%d): Assertion '%s' failed\n", file, line, expr);
	exit(1);
}

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ARRAY_LEN(A) (sizeof(A) / sizeof(0[A]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

bool
Char_IsDigit(u8 c)
{
	return ((u8)(c - '0') < (u8)10);
}

bool
Char_IsAlpha(u8 c)
{
	return ((u8)((c & 0xDF) - 'A') < (u8)('Z' - 'A'));
}
