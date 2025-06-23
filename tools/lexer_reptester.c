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

#include <immintrin.h>

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
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Invalid numer of arguments. Usage: lexer_reptest [path to source file]\n");
		return 1;
	}

	HANDLE in = CreateFileW(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	DWORD file_size = GetFileSize(in, &(DWORD){0});

	umm zpad = 64;
	u8* contents = malloc(file_size+zpad);

	DWORD bytes_read = 0;
	if (contents == 0 || !ReadFile(in, contents, file_size, &bytes_read, 0) || bytes_read != file_size)
	{
		fprintf(stderr, "Failed to read input file\n");
		contents = 0;
	}

	CloseHandle(in);

	if (contents != 0)
	{
		for (umm i = 0; i < zpad; ++i) contents[file_size + i] = 0;

		u64 lines = 0;
		for (u8* scan = contents; *scan != 0; ++scan)
		{
			lines += (*scan == '\n');
		}

		LARGE_INTEGER perf_freq;
		QueryPerformanceFrequency(&perf_freq);

		Virtual_Array tokens  = VA_Create(sizeof(Token), ~0U - 4096);
		Virtual_Array idents  = VA_Create(sizeof(u8), ~0U - 4096);
		Virtual_Array strings = VA_Create(sizeof(u8), ~0U - 4096);

		CommitMemory(tokens.data, tokens.reserved); tokens.committed = tokens.reserved;
		CommitMemory(idents.data, idents.reserved); idents.committed = idents.reserved;
		CommitMemory(strings.data, strings.reserved); strings.committed = strings.reserved;
		memset(tokens.data, '0', tokens.committed);
		memset(idents.data, '0', idents.committed);
		memset(strings.data, '0', strings.committed);

		u64 min_dus        = ~0ULL;
		f64 max_throughput = 0;
		f64 max_mlocps     = 0;
		for (umm i = 0; i < 10; ++i)
		{

			LARGE_INTEGER start_t;
			QueryPerformanceCounter(&start_t);

			Token* first_token = 0;
			u32 token_count    = 0;
			LexFile(&tokens, &idents, &strings, contents, &first_token, &token_count);

			LARGE_INTEGER end_t;
			QueryPerformanceCounter(&end_t);

			tokens.offset = 0;
			idents.offset = 0;
			strings.offset = 0;

			u64 t = end_t.QuadPart - start_t.QuadPart;

			u64 t_dus = (10000000*t)/perf_freq.QuadPart;

			f64 throughput = ((f64)file_size*perf_freq.QuadPart)/t;
			f64 mlocps     = (((f64)lines*perf_freq.QuadPart)/t)/1000000;

			ASSERT((u64)(0.5 + (throughput*t)/perf_freq.QuadPart) == (u64)file_size);
			ASSERT((u64)(0.5 + ((mlocps*1000000)*t)/perf_freq.QuadPart) == lines);

			if (i == 0)
			{
				wprintf(L"input file: %s (%f MB, %f Mloc)\n", argv[1], (f64)file_size/(1ULL << 20), (f64)lines/1000000);
				printf("\n run |    time      | throughput (MB) | throughput (lines of code)\n");
				printf("-----+--------------+-----------------+-----------------------------\n");
			}

			printf("  %2llu | %4llu.%04llu ms |  % 8.3f MB/s  | % 8.3f Mloc/s\n", i, t_dus/10000, t_dus%10000, throughput/(1ULL << 20), mlocps);

			if (t_dus < min_dus)
			{
				min_dus        = t_dus;
				max_throughput = throughput;
				max_mlocps     = mlocps;
			}
		}

		printf("min: %4llu.%04llu ms, %f MB/s, %f Mloc/s\n", min_dus/10000, min_dus%10000, max_throughput/(1ULL << 20), max_mlocps);
	}

	return 0;
}
