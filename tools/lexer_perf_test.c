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

#include <stdlib.h>
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
typedef union F32_Bits { f32 f; u32 bits; } F32_Bits;

typedef double f64;
typedef union F64_Bits { f64 f; u64 bits; } F64_Bits;

static void AssertionFailed(const char* file, int line, const char* expr);
#define ASSERT(EX) ((EX) ? 1 : (IsDebuggerPresent() ? (*(volatile int*)0 = 0) : (AssertionFailed(__FILE__, __LINE__, #EX), 0)))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ODIN_LEXER_PERF_TEST
#include "../src/memory.h"
#include "../src/string.h"
#include "../src/virtual_array.h"
#include "../src/tokens.h"
#include "../src/lexer.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

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

u32
StrCopy(wchar_t* dst, wchar_t* src)
{
	u32 i = 0;

	for (;;)
	{
		dst[i] = src[i];
		if (src[i] == 0) break;

		++i;
	}

	return i;
}

bool
StrMatch(wchar_t* a, wchar_t* b)
{
	u32 i = 0;
	while (a[i] != 0 && a[i] == b[i]) ++i;

	return (a[i] == 0 && a[i] == b[i]);
}

u64 TotalTime = 0;

void
Crawl(wchar_t* filename, u32 filename_len, Virtual_Array* files, Virtual_Array* tokens, Virtual_Array* idents, Virtual_Array* strings)
{

	WIN32_FIND_DATAW find_data = {0};

	StrCopy(filename + filename_len, L"\\*");

	HANDLE find_handle = FindFirstFileExW(filename, FindExInfoBasic, &find_data, FindExSearchNameMatch, 0, 0);
	ASSERT(find_handle != INVALID_HANDLE_VALUE);

	filename[filename_len] = 0;

	for (;;)
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!StrMatch(&find_data.cFileName[0], L".") && !StrMatch(&find_data.cFileName[0], L".."))
			{
				filename[filename_len] = L'\\';
				u32 new_filename_len = filename_len + 1 + StrCopy(filename + filename_len + 1, &find_data.cFileName[0]);

				Crawl(filename, new_filename_len, files, tokens, idents, strings);

				filename[filename_len] = 0;
			}
		}
		else
		{
			wchar_t* ext = &find_data.cFileName[0];
			
			for (wchar_t* scan = &find_data.cFileName[0]; *scan != 0; ++scan)
			{
				if (*scan == '.') ext = scan + 1;
			}

			if (StrMatch(ext, L"c") || StrMatch(ext, L"cc") || StrMatch(ext, L"cpp") || StrMatch(ext, L"h") || StrMatch(ext, L"hh") || StrMatch(ext, L"hpp") || StrMatch(ext, L"odin"))
			{
				filename[filename_len] = L'\\';
				StrCopy(filename + filename_len + 1, &find_data.cFileName[0]);

				HANDLE file_handle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				ASSERT(file_handle != INVALID_HANDLE_VALUE);

				u8* buffer = VA_PushN(files, find_data.nFileSizeLow + 1);

				DWORD read = 0;
				ASSERT(ReadFile(file_handle, buffer, find_data.nFileSizeLow, &read, 0) && read == find_data.nFileSizeLow);
				buffer[find_data.nFileSizeLow] = 0;

				CloseHandle(file_handle);

				filename[filename_len] = 0;

				Token* first_token = 0;
				u32 token_count    = 0;
				LARGE_INTEGER start;
				QueryPerformanceCounter(&start);
				LexFile(tokens, idents, strings, buffer, &first_token, &token_count);
				LARGE_INTEGER end;
				QueryPerformanceCounter(&end);
				TotalTime += end.QuadPart - start.QuadPart;
			}
		}

		if (FindNextFileW(find_handle, &find_data)) continue;
		else
		{
			ASSERT(GetLastError() == ERROR_NO_MORE_FILES);
			break;
		}
	}

	FindClose(find_handle);
}

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Invalid Arguments. Expected: lexer_perf_test [path to directory]\n");
		return 1;
	}
	
	Virtual_Array files   = VA_Init(sizeof(u8), 1U << 30);
	Virtual_Array tokens  = VA_Init(sizeof(Token), ~0U - 4096);
	Virtual_Array idents  = VA_Init(sizeof(u8), 1U << 30);
	Virtual_Array strings = VA_Init(sizeof(u8), 1U << 30);

	u32 filename_cap = ALIGN(MAX_PATH, PAGE_SIZE) + PAGE_SIZE;
	wchar_t* filename = VirtualAlloc(0, filename_cap, MEM_RESERVE, PAGE_READWRITE);
	VirtualAlloc(filename, filename_cap, MEM_COMMIT, PAGE_READWRITE);

	u32 filename_len = StrCopy(filename, argv[1]);

	if (filename[filename_len-1] == L'\\')
	{
		filename_len -= 1;
		filename[filename_len] = 0;
	}

	for (umm i = 0; i < 100; ++i) Crawl(filename, filename_len, &files, &tokens, &idents, &strings);

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);
	double time = (double)TotalTime/perf_freq.QuadPart;
	printf("%llu, %f, %f GB/s\n", TotalTime, time, ((double)files.offset/(1ULL << 30))/time);

	return 0;
}
