#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

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

bool
Char_IsAlpha(u8 c)
{
	return ((u8)((c & 0xDF) - 'A') <= (u8)('Z' - 'A'));
}

bool
Char_IsDigit(u8 c)
{
	return ((u8)(c - '0') < (u8)10);
}

typedef struct Stats
{
	u64 files;
	u64 lines;
	u64 bytes;
	u64 tokens;
	u64 idents;
	u64 strings;
	u64 keywords;
} Stats;

void
GatherStats(Stats* stats, u8* buffer)
{
	++stats->files;

	u8* scan = buffer;
	for (;;)
	{
		while (*scan != 0 && (*scan == ' ' || *scan == '\t' || *scan == '\r')) ++scan;

		if (*scan == 0) break;
		else if (*scan == '\n')
		{
			++stats->lines;
			++scan;
		}
		else if (scan[0] == '/' && scan[1] == '/')
		{
			while (*scan != 0 && *scan != '\n') ++scan;
		}
		else if (scan[0] == '/' && scan[1] == '*')
		{
			scan += 2;

			while (*scan != 0 && (scan[0] != '*' || scan[1] != '/'))
			{
				if (*scan == '\n') ++stats->lines;
				++scan;
			}

			if (*scan != 0) scan += 2;
		}
		else if (Char_IsAlpha(*scan) || *scan == '_')
		{
			u8* ident = scan;
			while (Char_IsAlpha(*scan) || Char_IsDigit(*scan) || *scan == '_') ++scan;

			++stats->tokens;
			++stats->idents;

			char* keywords[] = {
				"if",
				"for",
				"while",
				"else",
				"return",
				"break",
				"continue",
				"unsigned",
				"signed",
				"const",
				"volatile",
				"static",
				"typedef",
				"switch",
				"case",
				"default",
				"enum",
				"struct",
				"union",
				"extern",
				"goto",
				"inline",
				"long",
				"short",
				"register",
				"restrict",
			};

			for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); ++i)
			{
				if (strncmp((char*)ident, keywords[i], scan - ident) == 0 && keywords[i][scan - ident] == 0)
				{
					++stats->keywords;
					break;
				}
			}
		}
		else if (Char_IsDigit(*scan))
		{
			while (Char_IsDigit(*scan)) ++scan;
			++stats->tokens;
		}
		else if (*scan == '"')
		{
			++scan;

			while (*scan != 0 && *scan != '"')
			{
				if (scan[0] == '\\' && scan[1] != 0) ++scan;
				++scan;
			}

			assert(*scan != 0);
			++scan;

			++stats->strings;
			++stats->tokens;
		}
		else if (*scan == '\'')
		{
			++scan;

			while (*scan != 0 && *scan != '\'')
			{
				if (scan[0] == '\\' && scan[1] != 0) ++scan;
				++scan;
			}

			assert(*scan != 0);
			++scan;

			++stats->tokens;
		}
		else if (*scan == '#')
		{
			while (*scan != 0 && *scan != '\n')
			{
				if (scan[0] == '/' && scan[1] == '*')
				{
					break;
				}
				else if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
				{
					scan += 2;
				}
				++scan;
			}
		}
		else
		{
			++stats->tokens;
			++scan;
		}
	}

	stats->bytes += (scan - buffer);
}

void
Crawl(wchar_t* filename, u32 filename_len, Stats* stats, u8* buffer)
{

	WIN32_FIND_DATAW find_data = {0};

	StrCopy(filename + filename_len, L"\\*");

	HANDLE find_handle = FindFirstFileExW(filename, FindExInfoBasic, &find_data, FindExSearchNameMatch, 0, 0);
	assert(find_handle != INVALID_HANDLE_VALUE);

	filename[filename_len] = 0;

	for (;;)
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!StrMatch(&find_data.cFileName[0], L".") && !StrMatch(&find_data.cFileName[0], L".."))
			{
				filename[filename_len] = L'\\';
				u32 new_filename_len = filename_len + 1 + StrCopy(filename + filename_len + 1, &find_data.cFileName[0]);

				Crawl(filename, new_filename_len, stats, buffer);

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

			if (StrMatch(ext, L"c") || StrMatch(ext, L"cc") || StrMatch(ext, L"cpp") || StrMatch(ext, L"h") || StrMatch(ext, L"hh") || StrMatch(ext, L"hpp"))
			{
				filename[filename_len] = L'\\';
				StrCopy(filename + filename_len + 1, &find_data.cFileName[0]);

				HANDLE file_handle = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				assert(file_handle != INVALID_HANDLE_VALUE);

				DWORD read = 0;
				assert(ReadFile(file_handle, buffer, find_data.nFileSizeLow, &read, 0) && read == find_data.nFileSizeLow);
				buffer[find_data.nFileSizeLow] = 0;

				CloseHandle(file_handle);

				filename[filename_len] = 0;

				GatherStats(stats, buffer);
			}
		}

		if (FindNextFileW(find_handle, &find_data)) continue;
		else
		{
			assert(GetLastError() == ERROR_NO_MORE_FILES);
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
		fprintf(stderr, "Invalid Arguments. Expected: lex_crawler [path to directory]\n");
		return 1;
	}

	u32 filename_cap = ((MAX_PATH) + 4095) & ~(4096-1);
	wchar_t* filename = VirtualAlloc(0, filename_cap + 4096, MEM_RESERVE, PAGE_READWRITE);
	VirtualAlloc(filename, filename_cap, MEM_COMMIT, PAGE_READWRITE);

	u8* buffer = VirtualAlloc(0, 1 << 30, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	u32 filename_len = StrCopy(filename, argv[1]);

	if (filename[filename_len-1] == L'\\')
	{
		filename_len -= 1;
		filename[filename_len] = 0;
	}

	Stats stats = {0};
	Crawl(filename, filename_len, &stats, buffer);

	printf("Stats:\nfiles: %llu\nlines: %llu\nbytes: %llu\ntokens: %llu\nidents: %llu\nstrings: %llu\nkeywords: %llu\n", stats.files, stats.lines, stats.bytes, stats.tokens, stats.idents, stats.strings, stats.keywords);

	return 0;
}
