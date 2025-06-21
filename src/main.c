// NOTE: Constraints
// max total file size: ~~4GB~~ 1GB => max token memory 2GB, max AST memory 4GB
// linux like source code qualities:
// - 40% of tokens are identifiers
// - 8.6 bytes per token
// - 16 byte tokens requires <= 1.86x file contents of memory
// - 13.0% of identifiers are keywords
// - 51% of bytes are identifiers
// - 24% of bytes are whitespace
// offset/line/col are rarely used and can be recomputed on demand
// tab, space, carriage return and line feed are the only valid whitespace characters, everything else is an error
//
// Stats:
// files:           61467
// bytes:      1391639876 B (1327.171207 MB)
// lines:        35511800
// tokens:      161930156
// keywords:      8470484
// idents:       65023478 (sum: 710969750 B (51.09%), min: 1, max:   152, avg: 10.934047)
// strings:       1446808 (sum:  29686380 B ( 2.13%), min: 2, max: 20670, avg: 20.518535)
// whitespace:  164252444 (sum: 335754156 B (24.13%), min: 0, max:   298, avg:  2.044135)
// comments:      2260821 (sum: 174382823 B (12.53%))
// misc:         79865229 (sum:  81539078 B ( 5.86%))
#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
typedef union F32_Bits { f32 f; u32 bits; } F32_Bits;

typedef double f64;
typedef union F64_Bits { f64 f; u64 bits; } F64_Bits;

static void AssertionFailed(const char* file, int line, const char* expr);
#define ASSERT(EX) ((EX) ? 1 : (IsDebuggerPresent() ? (*(volatile int*)0 = 0) : (AssertionFailed(__FILE__, __LINE__, #EX), 0)))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#include "memory.h"
#include "strings.h"
#include "virtual_array.h"
#include "u128.h"
#include "tokens.h"
#include "lexer.h"

static void
AssertionFailed(const char* file, int line, const char* expr)
{
	fprintf(stderr, "%s(%d): Assertion '%s' failed\n", file, line, expr);
	exit(1);
}

static void*
ReserveMemory(umm bytes_to_reserve)
{
	bytes_to_reserve = ALIGN(bytes_to_reserve, PAGE_SIZE);

	void* result = 0;

	result = VirtualAlloc(0, bytes_to_reserve, MEM_RESERVE, PAGE_READWRITE);

	if (result == 0)
	{
		//// ERROR: Out of memory
		NOT_IMPLEMENTED;
	}

	return result;
}

static void
CommitMemory(void* commit_base, umm bytes_to_commit)
{
	bytes_to_commit = ALIGN(bytes_to_commit, PAGE_SIZE);

	void* result = 0;

	result = VirtualAlloc(commit_base, bytes_to_commit, MEM_COMMIT, PAGE_READWRITE);

	if (result == 0)
	{
		//// ERROR: Out of memory
		NOT_IMPLEMENTED;
	}
}

static void
FreeMemory(void* reserve_base)
{
	VirtualFree(reserve_base, 0, MEM_RELEASE);
}

int
main(int argc, char** argv)
{
	(void)argc, (void)argv;

	return 0;
}
