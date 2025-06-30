#pragma once

#include <windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/common.h"

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


bool
Char_IsDigit(u8 c)
{
	return ((u8)(c - '0') < (u8)10);
}

bool
Char_IsAlpha(u8 c)
{
	return ((u8)((c & 0xDF) - 'A') <= (u8)('Z' - 'A'));
}

bool
Char_IsHexAlpha(u8 c)
{
	return ((u8)((c & 0xDF) - 'A') <= (u8)('F' - 'A'));
}

u32
WideStrLen(wchar_t* str)
{
	u32 i = 0;

	if (str != 0)
	{
		while (str[i] != 0) ++i;
	}

	return i;
}

bool
WideStrEqual(wchar_t* a, wchar_t* b)
{
	if (a == 0 || b == 0) return false;
	else
	{
		while (*a != 0 && *b != 0 && *a == *b) ++a, ++b;

		return (*a == *b);
	}
}

typedef struct Path_Builder
{
	wchar_t* buffer;
	u32 buffer_len;
	u32 buffer_cap;
} Path_Builder;

typedef u32 Path_Marker;

Path_Builder
PathBuilder_Init(u32 cap)
{
	wchar_t* buffer = malloc((u64)cap*sizeof(wchar_t));
	ASSERT(buffer != 0);

	return (Path_Builder){
		.buffer     = buffer,
		.buffer_len = 0,
		.buffer_cap = cap,
	};
}

Path_Marker
PathBuilder_Push(Path_Builder* builder, wchar_t* addend)
{
	Path_Marker marker = builder->buffer_len;

	u32 addend_len = WideStrLen(addend);

	ASSERT((u64)builder->buffer_len + 1 + addend_len + 1 <= builder->buffer_cap);

	if (builder->buffer_len > 0) builder->buffer[builder->buffer_len++] = L'\\';

	memcpy(builder->buffer + builder->buffer_len, addend, (addend_len+1)*sizeof(wchar_t));

	builder->buffer_len += addend_len;

	if (builder->buffer[builder->buffer_len-1] == '\\' || builder->buffer[builder->buffer_len-1] == '/')
	{
		--builder->buffer_len;
	}

	builder->buffer[builder->buffer_len] = 0;

	return marker;
}

void
PathBuilder_PopToMarker(Path_Builder* builder, Path_Marker marker)
{
	ASSERT(builder->buffer_len >= marker);

	builder->buffer_len = marker;
	builder->buffer[builder->buffer_len] = 0;
}

wchar_t*
PathBuilder_CurrentPath(Path_Builder* builder)
{
	return builder->buffer;
}

typedef struct Token_Stat
{
	u64 num;
	u64 sum;
	u64 min;
	u64 max;
} Token_Stat;

Token_Stat
TokenStat_Init()
{
	return (Token_Stat){
		.num = 0,
		.sum = 0,
		.min = ~0ULL,
		.max = 0,
	};
}

Token_Stat
TokenStat_Combine(Token_Stat a, Token_Stat b)
{
	return (Token_Stat){
		.num   = a.num + b.num,
		.sum   = a.sum + b.sum,
		.min   = MIN(a.min, b.min),
		.max   = MAX(a.max, b.max),
	};
}

void
TokenStat_Add(Token_Stat* token_stat, u64 len)
{
	if (len > 0)
	{
		token_stat->num += 1;
		token_stat->sum += len;
		token_stat->min  = MIN(token_stat->min, len);
		token_stat->max  = MAX(token_stat->max, len);
	}
}

typedef struct Code_Stats
{
	Token_Stat lines;
	Token_Stat whitespace;
	Token_Stat single_line_comments;
	Token_Stat block_comments;
	Token_Stat idents;
	Token_Stat keywords;
	Token_Stat string_literals;
	Token_Stat char_literals;
	Token_Stat int_literals;
	Token_Stat hex_int_literals;
	Token_Stat float_literals;
	Token_Stat hex_float_literals;
	Token_Stat misc;
	Token_Stat includes;

	u64 bytes;
	u64 byte_freq[256];
} Code_Stats;

Code_Stats
CodeStats_Init()
{
	return (Code_Stats){
		.lines                = TokenStat_Init(),
		.whitespace           = TokenStat_Init(),
		.single_line_comments = TokenStat_Init(),
		.block_comments       = TokenStat_Init(),
		.idents               = TokenStat_Init(),
		.keywords             = TokenStat_Init(),
		.string_literals      = TokenStat_Init(),
		.char_literals        = TokenStat_Init(),
		.int_literals         = TokenStat_Init(),
		.hex_int_literals     = TokenStat_Init(),
		.float_literals       = TokenStat_Init(),
		.hex_float_literals   = TokenStat_Init(),
		.misc                 = TokenStat_Init(),
		.includes             = TokenStat_Init(),
		.bytes                = 0,
		.byte_freq            = {0},
	};
}

void
CodeStats_Combine(Code_Stats* stats, Code_Stats addend)
{
	stats->lines                = TokenStat_Combine(stats->lines, addend.lines);
	stats->whitespace           = TokenStat_Combine(stats->whitespace, addend.whitespace);
	stats->single_line_comments = TokenStat_Combine(stats->single_line_comments, addend.single_line_comments);
	stats->block_comments       = TokenStat_Combine(stats->block_comments, addend.block_comments);
	stats->idents               = TokenStat_Combine(stats->idents, addend.idents);
	stats->keywords             = TokenStat_Combine(stats->keywords, addend.keywords);
	stats->string_literals      = TokenStat_Combine(stats->string_literals, addend.string_literals);
	stats->char_literals        = TokenStat_Combine(stats->char_literals, addend.char_literals);
	stats->int_literals         = TokenStat_Combine(stats->int_literals, addend.int_literals);
	stats->hex_int_literals     = TokenStat_Combine(stats->hex_int_literals, addend.hex_int_literals);
	stats->float_literals       = TokenStat_Combine(stats->float_literals, addend.float_literals);
	stats->hex_float_literals   = TokenStat_Combine(stats->hex_float_literals, addend.hex_float_literals);
	stats->misc                 = TokenStat_Combine(stats->misc, addend.misc);
	stats->includes             = TokenStat_Combine(stats->includes, addend.includes);

	stats->bytes += addend.bytes;
	for (umm i = 0; i < ARRAY_LEN(stats->byte_freq); ++i) stats->byte_freq[i] += addend.byte_freq[i];
}

void
CodeStats__ConsumeChar(Code_Stats* stats, u8** line_start, u8** scan)
{
	stats->bytes             += 1;
	stats->byte_freq[**scan] += 1;

	if (**scan == '\n')
	{
		TokenStat_Add(&stats->lines, (*scan+1) - *line_start);
		*line_start = *scan + 1;
	}

	*scan += 1;
}

void
CodeStats__Advance(Code_Stats* stats, u8** line_start, u8** scan)
{
	CodeStats__ConsumeChar(stats, line_start, scan);

	if ((*scan)[0] == '\\' && (*scan)[1] == '\r' && (*scan)[2] == '\n')
	{
		CodeStats__ConsumeChar(stats, line_start, scan);
		CodeStats__ConsumeChar(stats, line_start, scan);
		CodeStats__ConsumeChar(stats, line_start, scan);
	}
}

void
CodeStats__SkipNumberSuffix(Code_Stats* stats, u8** line_start, u8** scan)
{
	umm skip = 0;

	if (((*scan)[0]&0xDF) == 'F')
	{
		skip = 1;
	}
	else if ((((*scan)[0]&0xDF) == 'U' && ((*scan)[1]&0xDF) == 'L' && ((*scan)[2]&0xDF) == 'L') ||
					 (((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'L' && ((*scan)[2]&0xDF) == 'U'))
	{
		skip = 3;
	}
	else if ((((*scan)[0]&0xDF) == 'U' && ((*scan)[1]&0xDF) == 'L') ||
					 (((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'U') ||
					 (((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'L'))
	{
		skip = 2;
	}
	else if (((*scan)[0]&0xDF) == 'U' || ((*scan)[0]&0xDF) == 'L')
	{
		skip = 1;
	}

	for (umm i = 0; i < skip; ++i) CodeStats__Advance(stats, line_start, scan);
}

Code_Stats
CodeStats_Gather(u8* contents)
{
	Code_Stats stats = CodeStats_Init();
	u8* line_start = contents;
	u8* scan       = contents;

	if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
	{
		CodeStats__ConsumeChar(&stats, &line_start, &scan);
		CodeStats__ConsumeChar(&stats, &line_start, &scan);
		CodeStats__ConsumeChar(&stats, &line_start, &scan);
	}

	for (;;)
	{
		{
			u8* start = scan;
			while ((u8)(*scan-1) < (u8)0x20) CodeStats__Advance(&stats, &line_start, &scan);
			u8* end = scan;

			TokenStat_Add(&stats.whitespace, end - start);
		}

		if (*scan == 0) break;
		else if (scan[0] == '/' && scan[1] == '/')
		{
			u8* start = scan;
			while (*scan != 0 && *scan != '\n') CodeStats__Advance(&stats, &line_start, &scan);
			u8* end = scan;

			TokenStat_Add(&stats.single_line_comments, end - start);
		}
		else if (scan[0] == '/' && scan[1] == '*')
		{
			u8* start = scan;

			CodeStats__Advance(&stats, &line_start, &scan);
			CodeStats__Advance(&stats, &line_start, &scan);

			while (*scan != 0 && !(scan[0] == '*' && scan[1] == '/'))
			{
				CodeStats__Advance(&stats, &line_start, &scan);
			}

			if (*scan != 0)
			{
				CodeStats__Advance(&stats, &line_start, &scan);
				CodeStats__Advance(&stats, &line_start, &scan);
			}

			u8* end = scan;

			TokenStat_Add(&stats.block_comments, end - start);
		}
		else if (Char_IsAlpha(*scan) || *scan == '_')
		{
			u8* start = scan;
			while (Char_IsAlpha(*scan) || Char_IsDigit(*scan) || *scan == '_') CodeStats__Advance(&stats, &line_start, &scan);
			u8* end = scan;

			TokenStat_Add(&stats.idents, end - start);

			char* keywords[] = {
				"auto",
				"break",
				"case",
				"char",
				"const",
				"continue",
				"default",
				"do",
				"double",
				"else",
				"enum",
				"extern",
				"float",
				"for",
				"goto",
				"if",
				"inline",
				"int",
				"long",
				"register",
				"restrict",
				"return",
				"short",
				"signed",
				"sizeof",
				"static",
				"struct",
				"switch",
				"typedef",
				"union",
				"unsigned",
				"void",
				"volatile",
				"while",
				"_Bool",
				"_Complex",
				"_Imaginary",
			};

			for (umm i = 0; i < ARRAY_LEN(keywords); ++i)
			{
				if (strncmp((char*)start, keywords[i], end-start) == 0)
				{
					TokenStat_Add(&stats.keywords, end - start);
				}
			}
		}
		else if (*scan == '"' || *scan == '\'')
		{
			u8 terminator = *scan;

			u8* start = scan;

			CodeStats__Advance(&stats, &line_start, &scan);

			while (*scan != 0 && *scan != terminator)
			{
				if (scan[0] == '\\' && (scan[1] == '\\' || scan[1] == terminator)) CodeStats__Advance(&stats, &line_start, &scan);
				CodeStats__Advance(&stats, &line_start, &scan);
			}
			
			if (*scan != 0) CodeStats__Advance(&stats, &line_start, &scan);

			u8* end = scan;

			if (terminator == '"') TokenStat_Add(&stats.string_literals, end - start);
			else                   TokenStat_Add(&stats.char_literals, end - start);
		}
		else if (scan[0] == '0' && scan[1] == 'x')
		{
			u8* start = scan;

			bool is_float = false;

			CodeStats__Advance(&stats, &line_start, &scan);
			CodeStats__Advance(&stats, &line_start, &scan);

			while (Char_IsDigit(*scan) || (u8)((*scan&0xDF) - 'A') <= (u8)('F' - 'A')) CodeStats__Advance(&stats, &line_start, &scan);

			if (*scan == '.')
			{
				is_float = true;
				CodeStats__Advance(&stats, &line_start, &scan);

				while (Char_IsDigit(*scan) || (u8)((*scan&0xDF) - 'A') <= (u8)('F' - 'A')) CodeStats__Advance(&stats, &line_start, &scan);

				if ((*scan&0xDF) == 'P')
				{
					CodeStats__Advance(&stats, &line_start, &scan);
					if (*scan == '+' || *scan == '-') CodeStats__Advance(&stats, &line_start, &scan);

					while (Char_IsDigit(*scan)) CodeStats__Advance(&stats, &line_start, &scan);
				}
			}

			CodeStats__SkipNumberSuffix(&stats, &line_start, &scan);

			u8* end = scan;

			if (is_float) TokenStat_Add(&stats.hex_float_literals, end - start);
			else          TokenStat_Add(&stats.hex_int_literals, end - start);
		}
		else if (Char_IsDigit(*scan) || (scan[0] == '.' && Char_IsDigit(scan[1])))
		{
			u8* start = scan;

			bool is_float = false;

			while (Char_IsDigit(*scan)) CodeStats__Advance(&stats, &line_start, &scan);

			if (*scan == '.')
			{
				is_float = true;
				CodeStats__Advance(&stats, &line_start, &scan);

				while (Char_IsDigit(*scan)) CodeStats__Advance(&stats, &line_start, &scan);

				if ((*scan&0xDF) == 'E')
				{
					CodeStats__Advance(&stats, &line_start, &scan);
					if (*scan == '+' || *scan == '-') CodeStats__Advance(&stats, &line_start, &scan);

					while (Char_IsDigit(*scan)) CodeStats__Advance(&stats, &line_start, &scan);
				}
			}

			CodeStats__SkipNumberSuffix(&stats, &line_start, &scan);

			u8* end = scan;

			if (is_float) TokenStat_Add(&stats.float_literals, end - start);
			else          TokenStat_Add(&stats.int_literals, end - start);
		}
		else if (strncmp((char*)scan, "#include", sizeof("#include")-1) == 0)
		{
			u8* start = scan;
			while (*scan != 0 && *scan != '\n') CodeStats__Advance(&stats, &line_start, &scan);

			if (*scan != 0) CodeStats__Advance(&stats, &line_start, &scan);
			u8* end = scan;

			TokenStat_Add(&stats.includes, end - start);
		}
		else
		{
			u8* start = scan;
			CodeStats__Advance(&stats, &line_start, &scan);
			u8* end = scan;

			TokenStat_Add(&stats.misc, end - start);
		}
	}

	return stats;
}

typedef struct Stats
{
	Code_Stats pre_clean;
	Code_Stats post_clean;
	u32 file_count;
} Stats;
