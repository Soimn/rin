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

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

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
	u64 whitespace;
	u64 ws_min;
	u64 ws_max;
	u64 ws_sum;
	u64 ident_min;
	u64 ident_max;
	u64 ident_sum;
	u64 string_min;
	u64 string_max;
	u64 string_sum;
	u64 misc;
	u64 misc_sum;
	u64 comments;
	u64 comment_sum;
	u64 preprocs;
	u64 preproc_sum;
} Stats;

u8*
SkipSingleComment(u8* scan, Stats* stats)
{
	assert(scan[0] == '/' && scan[1] == '/');

	u8* start = scan;

	while (*scan != 0 && *scan != '\n')
	{
		if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
		{
			++stats->lines;
			scan += 3;
		}
		else ++scan;
	}

	++stats->comments;
	stats->comment_sum += scan - start;

	return scan;
}

u8*
SkipBlockComment(u8* scan, Stats* stats)
{
	assert(scan[0] == '/' && scan[1] == '*');

	u8* start = scan;

	scan += 2;

	while (*scan != 0 && !(scan[0] == '*' && scan[1] == '/'))
	{
		if (*scan == '\n') ++stats->lines;
		++scan;
	}

	assert(scan[0] == '*' && scan[1] == '/');
	scan += 2;

	++stats->comments;
	stats->comment_sum += scan - start;

	return scan;
}

void
GatherStats(Stats* stats, u8* buffer)
{
	++stats->files;

	u8* scan = buffer;
	for (;;)
	{
		{
			u8* ws_start = scan;
			for (;;)
			{
				if (*scan == ' ' || *scan == '\t' || *scan == '\r' || *scan == '\v' || *scan == '\f')
				{
					++scan;
				}
				else if (*scan == '\n' || scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
				{
					++stats->lines;

					scan += (*scan == '\\' ? 3 : 1);
				}
				else break;
			}

			u64 ws_len = scan - ws_start;
			stats->ws_sum += ws_len;
			stats->ws_min = MIN(stats->ws_min, ws_len);
			stats->ws_max = MAX(stats->ws_max, ws_len);
			++stats->whitespace;
		}

		if (*scan == 0) break;
		else if (scan[0] == '/' && scan[1] == '/') scan = SkipSingleComment(scan, stats);
		else if (scan[0] == '/' && scan[1] == '*') scan = SkipBlockComment(scan, stats);
		else if (Char_IsAlpha(*scan) || *scan == '_')
		{
			u8* ident = scan;
			while (Char_IsAlpha(*scan) || Char_IsDigit(*scan) || *scan == '_') ++scan;

			assert(!(scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n' && (Char_IsAlpha(scan[3]) || Char_IsDigit(scan[3]) || scan[3] == '_')));

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

			u64 ident_len = scan - ident;
			stats->ident_sum += ident_len;
			stats->ident_min = MIN(stats->ident_min, ident_len);
			stats->ident_max = MAX(stats->ident_max, ident_len);
		}
		else if (Char_IsDigit(*scan) || *scan == '.')
		{
			if (scan[0] == '0' && (scan[1]&0xDF) == 'X')
			{
				scan += 2;

				while (Char_IsDigit(*scan) || (u8)((*scan&0xDF) - 'A') <= (u8)('F' - 'A')) ++scan;

				if (*scan == '.')
				{
					++scan;
					while (Char_IsDigit(*scan) || (u8)((*scan&0xDF) - 'A') <= (u8)('F' - 'A')) ++scan;

					if ((*scan&0xDF) == 'P')
					{
						++scan;
						if (*scan == '+' || *scan == '-') ++scan;
						while (Char_IsDigit(*scan)) ++scan;
					}
				}
			}
			else if (scan[0] == '.' && !Char_IsDigit(scan[1]))
			{
				++scan;
			}
			else
			{
				while (Char_IsDigit(*scan)) ++scan;

				if (*scan == '.')
				{
					++scan;
					while (Char_IsDigit(*scan)) ++scan;

					if ((*scan&0xDF) == 'E')
					{
						++scan;
						if (*scan == '+' || *scan == '-') ++scan;
						while (Char_IsDigit(*scan)) ++scan;
					}
				}
			}

			u8 u = *scan & 0xDF;
			if      (u == 'U' && (scan[1]&0xDF) == 'L') scan += ((scan[2]&0xDF) == 'L' ? 3 : 2);
			else if (u == 'L')                          scan += ((scan[1]&0xDF) == 'L' ? 2 : 1);
			else if (u == 'F' || u == 'U')              scan += 1;

			++stats->tokens;
		}
		else if (*scan == '"')
		{
			u8* string = scan;

			++scan;
			
			while (*scan != '"')
			{
				assert(*scan != 0);
				if (scan[0] == '\\' && (scan[1] == '"' || scan[1] == '\\'))
				{
					scan += 2;
				}
				else if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
				{
					++stats->lines;
					scan += 3;
				}
				else ++scan;
			}

			++scan;

			++stats->tokens;
			++stats->strings;

			u64 string_len = scan - string;
			stats->string_sum += string_len;
			stats->string_min = MIN(stats->string_min, string_len);
			stats->string_max = MAX(stats->string_max, string_len);
		}
		else if (*scan == '\'')
		{
			++scan;
			
			while (*scan != '\'')
			{
				assert(*scan != 0);
				if (scan[0] == '\\' && (scan[1] == '\'' || scan[1] == '\\'))
				{
					scan += 2;
				}
				else if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
				{
					assert(false);
				}
				else ++scan;
			}

			++scan;

			++stats->tokens;
		}
		/*
		else if (*scan == '#')
		{
			u8* start = scan;

			for (;;)
			{
				u8* ws_start = scan;
				for (;;)
				{
					if (*scan == ' ' || *scan == '\t' || *scan == '\r' || *scan == '\v' || *scan == '\f')
					{
						++scan;
					}
					else if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
					{
						++stats->lines;
						scan += 3;
					}
					else break;
				}

				u64 ws_len = scan - ws_start;
				if (*scan == '\n') ++ws_len;
				stats->ws_sum += ws_len;
				stats->ws_min = MIN(stats->ws_min, ws_len);
				stats->ws_max = MAX(stats->ws_max, ws_len);
				++stats->whitespace;

				if (*scan == '\n')
				{
					++stats->lines;
					++scan;
					break;
				}
				else if (scan[0] == '/' && scan[1] == '/')
				{
					scan = SkipSingleComment(scan, stats);
					break;
				}
				else if (scan[0] == '/' && scan[1] == '*')
				{
					scan = SkipBlockComment(scan, stats);
				}
				else
				{
					if (*scan == 0) break;
					++scan;
				}
			}

			++stats->preprocs;
			stats->preproc_sum += scan - start;
		}
		*/
		else
		{
			u8* start = scan;

			if (scan[0] == scan[1] && (*scan == '+' || *scan == '-' || *scan == '&' || *scan == '|' || *scan == '<' || *scan == '>'))
			{
				scan += 2;
			}
			else if (scan[0] != 0 && scan[1] == '=' && (*scan == '=' || *scan == '!' || *scan == '<' || *scan == '>' ||
						                                      *scan == '*' || *scan == '/' || *scan == '%' || *scan == '&' ||
																									*scan == '+' || *scan == '-' || *scan == '|' || *scan == '^'))
			{
				scan += 2;
			}
			else if ((*scan == '<' || *scan == '>') && scan[0] == scan[1] && scan[2] == '=')
			{
				scan += 3;
			}
			else
			{
				++scan;
			}

			++stats->tokens;
			++stats->misc;
			stats->misc_sum += scan - start;
		}
	}

	stats->bytes += scan - buffer;
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

				//if (StrMatch(find_data.cFileName, L"speakup_dtlk.h")) __debugbreak();

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
	stats.ident_min  = ~0ULL;
	stats.string_min = ~0ULL;
	stats.ws_min     = ~0ULL;
	Crawl(filename, filename_len, &stats, buffer);

	printf("Stats:\n");
	printf("files:      % 10llu\n", stats.files);
	printf("bytes:      % 10llu B (%f MB)\n", stats.bytes, (double)stats.bytes/(1 << 20));
	printf("lines:      % 10llu\n", stats.lines);
	printf("tokens:     % 10llu\n", stats.tokens);
	printf("keywords:   % 10llu\n", stats.keywords);
	printf("idents:     % 10llu (sum: % 9llu B (%5.2f%%), min: %llu, max: % 5llu, avg: %9.6f)\n", stats.idents, stats.ident_sum, 100*(double)stats.ident_sum/stats.bytes, stats.ident_min, stats.ident_max, (double)stats.ident_sum/stats.idents);
	printf("strings:    % 10llu (sum: % 9llu B (%5.2f%%), min: %llu, max: % 5llu, avg: %9.6f)\n", stats.strings, stats.string_sum, 100*(double)stats.string_sum/stats.bytes, stats.string_min, stats.string_max, (double)stats.string_sum/stats.strings);
	printf("whitespace: % 10llu (sum: % 9llu B (%5.2f%%), min: %llu, max: % 5llu, avg: %9.6f)\n", stats.whitespace, stats.ws_sum, 100*(double)stats.ws_sum/stats.bytes, stats.ws_min, stats.ws_max, (double)stats.ws_sum/stats.whitespace);
	printf("comments:   % 10llu (sum: % 9llu B (%5.2f%%))\n", stats.comments, stats.comment_sum, 100*(double)stats.comment_sum/stats.bytes);
	//printf("preproc:    % 10llu (sum: % 9llu B (%5.2f%%))\n", stats.preprocs, stats.preproc_sum, 100*(double)stats.preproc_sum/stats.bytes);
	printf("misc:       % 10llu (sum: % 9llu B (%5.2f%%))\n", stats.misc, stats.misc_sum, 100*(double)stats.misc_sum/stats.bytes);

	return 0;
}
