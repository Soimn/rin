#include "common.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

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

bool
IsCFile(wchar_t* filename)
{
	wchar_t* ext = filename;
	for (wchar_t* scan = filename; *scan != 0; ++scan)
	{
		if (*scan == '.')
		{
			ext = scan + 1;
		}
	}

	return (WideStrEqual(ext, L"c") || WideStrEqual(ext, L"h"));
}

void
Clean__Accept(u8* contents, u32* read_cursor, u32* write_cursor)
{
	contents[*write_cursor] = contents[*read_cursor];
	*write_cursor += 1;
	*read_cursor += 1;
}

void
Clean__Reject(u32* read_cursor)
{
	*read_cursor += 1;
}

void
Clean__RejectNumberSuffix(u8* contents, u32* read_cursor)
{
	if ((contents[*read_cursor]&0xDF) == 'F')
	{
		*read_cursor += 1;
	}
	else if ((contents[*read_cursor]&0xDF) == 'U' && (contents[*read_cursor+1]&0xDF) == 'L' && (contents[*read_cursor+2]&0xDF) == 'L' ||
					 (contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'L' && (contents[*read_cursor+2]&0xDF) == 'U')
	{
		*read_cursor += 3;
	}
	else if ((contents[*read_cursor]&0xDF) == 'U' && (contents[*read_cursor+1]&0xDF) == 'L' ||
					 (contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'U' ||
					 (contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'L')
	{
		*read_cursor += 2;
	}
	else if ((contents[*read_cursor]&0xDF) == 'U' || (contents[*read_cursor]&0xDF) == 'L')
	{
		*read_cursor += 1;
	}
}

u32
Clean(u8* contents)
{
	u32 read_cursor  = 0;
	u32 write_cursor = 0;

	for (;;)
	{
		if (contents[read_cursor] == '\\' && contents[read_cursor+1] == '\r' && contents[read_cursor+2] == '\n')
		{
			Clean__Reject(&read_cursor);
			Clean__Reject(&read_cursor);
			Clean__Reject(&read_cursor);
		}
		else
		{
			if (contents[read_cursor] == 0)
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
				break;
			}
			else 
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
				continue;
			}
		}
	}

	read_cursor  = 0;
	write_cursor = 0;

	for (;;)
	{
		while ((u8)(contents[read_cursor]-1) < 0x20)
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
		}

		if (contents[read_cursor] == 0)
		{
			contents[write_cursor] = 0;
			break;
		}
		else if (contents[read_cursor] == '/' && contents[read_cursor+1] == '/')
		{
			while (contents[read_cursor] != 0 && contents[read_cursor] != '\n')
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}
		}
		else if (contents[read_cursor] == '/' && contents[read_cursor+1] == '*')
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
			Clean__Accept(contents, &read_cursor, &write_cursor);

			while (contents[read_cursor] != 0 && !(contents[read_cursor] == '*' && contents[read_cursor+1] == '/'))
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}

			if (contents[read_cursor] != 0)
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}
		}
		else if (contents[read_cursor] == '.')
		{
			if (!Char_IsDigit(contents[read_cursor+1])) Clean__Accept(contents, &read_cursor, &write_cursor);
			else                                        Clean__Reject(&read_cursor);
		}
		else if (contents[read_cursor] == '0' && contents[read_cursor+1] == 'x')
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
			Clean__Accept(contents, &read_cursor, &write_cursor);

			while ((u8)((contents[read_cursor] & 0xDF) - 'A') <= (u8)('F' - 'A'))
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}

			if (contents[read_cursor] == '.')
			{
				Clean__Reject(&read_cursor);

				while ((u8)((contents[read_cursor] & 0xDF) - 'A') <= (u8)('F' - 'A'))
				{
					Clean__Reject(&read_cursor);
				}

				if ((contents[read_cursor]&0xDF) == 'P')
				{
					Clean__Reject(&read_cursor);

					if (contents[read_cursor] == '+' || contents[read_cursor] == '-')
					{
						Clean__Reject(&read_cursor);
					}

					while (Char_IsDigit(contents[read_cursor]))
					{
						Clean__Reject(&read_cursor);
					}
				}
			}

			Clean__RejectNumberSuffix(contents, &read_cursor);
		}
		else if (Char_IsDigit(contents[read_cursor]))
		{
			while (Char_IsDigit(contents[read_cursor]))
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}

			if (contents[read_cursor] == '.')
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);

				while (Char_IsDigit(contents[read_cursor]))
				{
					Clean__Accept(contents, &read_cursor, &write_cursor);
				}
			}

			if ((contents[read_cursor]&0xDF) == 'E')
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);

				if (contents[read_cursor] == '+' || contents[read_cursor] == '-')
				{
					Clean__Accept(contents, &read_cursor, &write_cursor);
				}

				while (Char_IsDigit(contents[read_cursor]))
				{
					Clean__Accept(contents, &read_cursor, &write_cursor);
				}
			}

			Clean__RejectNumberSuffix(contents, &read_cursor);
		}
		else if (contents[read_cursor] == '"' || contents[read_cursor] == '\'')
		{
			u8 terminator = contents[read_cursor];

			Clean__Accept(contents, &read_cursor, &write_cursor);

			for (;;)
			{
				if (contents[read_cursor] == terminator || contents[read_cursor] == 0) break;
				else if (contents[read_cursor] != '\\')
				{
					Clean__Accept(contents, &read_cursor, &write_cursor);
				}
				else
				{
					u8 accepted_seqs[] = { 'a', 'b', 'e', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"', 'u', 'U' };

					bool is_accepted = false;
					for (umm i = 0; i < ARRAY_LEN(accepted_seqs); ++i)
					{
						if (contents[read_cursor] == accepted_seqs[i])
						{
							is_accepted = true;
							break;
						}
					}

					if (is_accepted)
					{
						Clean__Accept(contents, &read_cursor, &write_cursor);
						Clean__Accept(contents, &read_cursor, &write_cursor);
					}
					else
					{
						Clean__Reject(&read_cursor);
					}
				}
			}

			if (contents[read_cursor] != 0)
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}
		}
		else if (strncmp((char*)contents + read_cursor, "#include", sizeof("#include")-1) == 0)
		{
			while (contents[read_cursor] != 0 && contents[read_cursor] != '\n') Clean__Reject(&read_cursor);
			if (contents[read_cursor] != 0) Clean__Reject(&read_cursor);
		}
		else if (contents[read_cursor] >= 0x7F)
		{
			Clean__Reject(&read_cursor);
		}
		else
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
		}
	}

	return write_cursor;
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
	else if (((*scan)[0]&0xDF) == 'U' && ((*scan)[1]&0xDF) == 'L' && ((*scan)[2]&0xDF) == 'L' ||
					 ((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'L' && ((*scan)[2]&0xDF) == 'U')
	{
		skip = 3;
	}
	else if (((*scan)[0]&0xDF) == 'U' && ((*scan)[1]&0xDF) == 'L' ||
					 ((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'U' ||
					 ((*scan)[0]&0xDF) == 'L' && ((*scan)[1]&0xDF) == 'L')
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
		else if (Char_IsDigit(*scan) || scan[0] == '.' && Char_IsDigit(scan[1]))
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

void
Crawl(Stats* stats, Path_Builder in_path, Path_Builder out_path, u8* file_buffer, u64 file_buffer_size)
{
	WIN32_FIND_DATAW find_data;
	
	Path_Marker star_marker = PathBuilder_Push(&in_path, L"*");
	HANDLE find_handle = FindFirstFileW(PathBuilder_CurrentPath(&in_path), &find_data);
	PathBuilder_PopToMarker(&in_path, star_marker);

	if (find_handle == INVALID_HANDLE_VALUE)
	{
		fwprintf(stderr, L"WARNING: Failed to enumerate files in '%s'.\n", PathBuilder_CurrentPath(&in_path));
		return;
	}

	for (;;)
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// NOTE: skip ., .. and hidden directories lik .git
			if (find_data.cFileName[0] != L'.')
			{
				Path_Marker marker = PathBuilder_Push(&in_path, find_data.cFileName);
				Crawl(stats, in_path, out_path, file_buffer, file_buffer_size);
				PathBuilder_PopToMarker(&in_path, marker);
			}
		}
		else if (IsCFile(find_data.cFileName))
		{
			Path_Marker in_marker = PathBuilder_Push(&in_path, find_data.cFileName);
			HANDLE in_file = CreateFileW(PathBuilder_CurrentPath(&in_path), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

			if (in_file == INVALID_HANDLE_VALUE)
			{
				fwprintf(stderr, L"WARNING: Failed to open file '%s'.\n", PathBuilder_CurrentPath(&in_path));
			}
			else
			{
				if ((u64)find_data.nFileSizeLow+1 > file_buffer_size || find_data.nFileSizeHigh != 0)
				{
					fwprintf(stderr, L"WARNING: File '%s' is too large.\n", PathBuilder_CurrentPath(&in_path));
				}
				else
				{
					u32 file_size = find_data.nFileSizeLow;

					DWORD bytes_read = 0;
					if (!ReadFile(in_file, file_buffer, file_size, &bytes_read, 0) || bytes_read != file_size)
					{
						fwprintf(stderr, L"WARNING: Failed to read '%s'.\n", PathBuilder_CurrentPath(&in_path));
					}
					else
					{
						file_buffer[file_size] = 0;

						if ((file_buffer[0] == 0xFF && file_buffer[1] == 0xFE) ||
								(file_buffer[0] == 0xFE && file_buffer[1] == 0xFF))
						{
							fwprintf(stderr, L"WARNING: Skipping non UTF-8 file '%s'.\n", PathBuilder_CurrentPath(&in_path));
						}
						else
						{
							Code_Stats pre_clean = CodeStats_Gather(file_buffer);
							u32 write_size = Clean(file_buffer);
							Code_Stats post_clean = CodeStats_Gather(file_buffer);

							wchar_t out_filename[sizeof("0000000000.rlt")] = {
								L'0' + ((stats->file_count / 1000000000) % 10),
								L'0' + ((stats->file_count / 100000000)  % 10),
								L'0' + ((stats->file_count / 10000000)   % 10),
								L'0' + ((stats->file_count / 1000000)    % 10),
								L'0' + ((stats->file_count / 100000)     % 10),
								L'0' + ((stats->file_count / 10000)      % 10),
								L'0' + ((stats->file_count / 1000)       % 10),
								L'0' + ((stats->file_count / 100)        % 10),
								L'0' + ((stats->file_count / 10)         % 10),
								L'0' + ((stats->file_count / 1)          % 10),
								L'.', L'r', L'l', L't', 0
							};

							Path_Marker out_marker = PathBuilder_Push(&out_path, out_filename);
							HANDLE out_file = CreateFileW(PathBuilder_CurrentPath(&out_path), GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

							if (out_file == INVALID_HANDLE_VALUE)
							{
								fwprintf(stderr, L"WARNING: Skipping file due to file error '%s'.\n", PathBuilder_CurrentPath(&in_path));
							}
							else
							{
								DWORD bytes_written = 0;
								if (!WriteFile(out_file, file_buffer, write_size, &bytes_written, 0) || bytes_written != write_size)
								{
									fwprintf(stderr, L"WARNING: Skipping file due to write error '%s'.\n", PathBuilder_CurrentPath(&in_path));
									DeleteFileW(PathBuilder_CurrentPath(&out_path));
								}
								else
								{
									++stats->file_count;
									CodeStats_Combine(&stats->pre_clean, pre_clean);
									CodeStats_Combine(&stats->post_clean, post_clean);
								}
							}

							if (out_file != INVALID_HANDLE_VALUE) CloseHandle(out_file);

							PathBuilder_PopToMarker(&out_path, out_marker);
						}
					}
				}
			}

			if (in_file != INVALID_HANDLE_VALUE) CloseHandle(in_file);

			PathBuilder_PopToMarker(&in_path, in_marker);
		}

		if (FindNextFileW(find_handle, &find_data)) continue;
		else
		{
			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				fwprintf(stderr, L"WARNING: Failed to enumerate all files in '%s'.\n", PathBuilder_CurrentPath(&in_path));
			}

			break;
		}
	}

	FindClose(find_handle);
}

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 3)
	{
		fwprintf(stderr, L"ERROR: Invalid number of arguments.\nExpected: %s [path to directory of c files] [path to output directory]\n", argv[0]);
		return 1;
	}

	wchar_t* in_dir  = argv[1];
	wchar_t* out_dir = argv[2];

	if (!PathIsDirectoryW(in_dir))
	{
		fwprintf(stderr, L"ERROR: Input is not a directory ('%s')\n", in_dir);
		return 1;
	}

	if (!CreateDirectoryW(out_dir, 0))
	{
		int error = GetLastError();
	
		if (error == ERROR_PATH_NOT_FOUND)
		{
			fwprintf(stderr, L"ERROR: This tool is dumb (I am lazy) and cannot create intermediate directories.\nPlease create all intermediate directories in the output directory path :)\nPath provided: '%s'\n", out_dir);
		}
		else if (error == ERROR_ALREADY_EXISTS)
		{
			fwprintf(stderr, L"ERROR: Output directory already exists.\nPath provided: '%s'\n", out_dir);
		}
		else
		{
			fwprintf(stderr, L"ERROR: Failed to create the output directory.\nPath provided: '%s'\n", out_dir);
		}

		return 1;
	}

	Path_Builder in_path = {0};
	in_path.buffer_cap = 2*MAX_PATH;
	in_path.buffer     = VirtualAlloc(0, (in_path.buffer_cap)*sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	Path_Builder out_path = {0};
	out_path.buffer_cap = 2*MAX_PATH;
	out_path.buffer     = VirtualAlloc(0, (out_path.buffer_cap)*sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	u64 file_buffer_size = 1ULL << 32;
	u8* file_buffer = VirtualAlloc(0, file_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (in_path.buffer == 0 || out_path.buffer == 0 || file_buffer == 0)
	{
		fprintf(stderr, "ERROR: Failed to allocate memory\n");
		return 1;
	}

	PathBuilder_Push(&in_path, in_dir);
	PathBuilder_Push(&out_path, out_dir);

	Stats stats = {
		.pre_clean  = CodeStats_Init(),
		.post_clean = CodeStats_Init(),
		.file_count = 0,
	};

	Crawl(&stats, in_path, out_path, file_buffer, file_buffer_size);

	Path_Marker out_marker = PathBuilder_Push(&out_path, L"manifest.rltm");
	HANDLE manifest_file = CreateFileW(PathBuilder_CurrentPath(&out_path), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (manifest_file == INVALID_HANDLE_VALUE)
	{
		fwprintf(stderr, L"ERROR: Failed to create manifest file '%s'.\n", PathBuilder_CurrentPath(&out_path));
	}
	else
	{
#define WRITE_TOKEN_STAT(NAME, VALUE)                                                                                                                                  \
		skip = snprintf(cursor, space, "%20s = { num = %20llu, sum = %20llu, min = %20llu, max = %20llu }\n", (NAME), (VALUE).num, (VALUE).sum, (VALUE).min, (VALUE).max); \
		cursor += skip;                                                                                                                                                    \
		space  -= skip

		size_t write_buffer_size = 1ULL << 16;
		char* write_buffer = malloc(write_buffer_size);
		size_t space = write_buffer_size;
		char* cursor = write_buffer;
		size_t skip = 0;

		if (cursor == 0)
		{
			fprintf(stderr, "ERROR: Failed to allocate memory to serialize manifest\n");
		}
		else
		{
			memcpy(cursor, &stats, sizeof(stats));
			cursor += sizeof(stats);
			space  -= sizeof(stats);

			struct { Code_Stats stat; char* name; } code_stat[] = {
				{stats.pre_clean,  "pre_clean" },
				{stats.post_clean, "post_clean"},
			};

			for (umm j = 0; j < ARRAY_LEN(code_stat); ++j)
			{
				
				skip = snprintf(cursor, space, "\n\n%s\n===========\n", code_stat[j].name);
				cursor += skip;
				space  -= skip;

				WRITE_TOKEN_STAT("lines", code_stat[j].stat.lines);
				WRITE_TOKEN_STAT("whitespace", code_stat[j].stat.whitespace);
				WRITE_TOKEN_STAT("single_line_comments", code_stat[j].stat.single_line_comments);
				WRITE_TOKEN_STAT("block_comments", code_stat[j].stat.block_comments);
				WRITE_TOKEN_STAT("idents", code_stat[j].stat.idents);
				WRITE_TOKEN_STAT("keywords", code_stat[j].stat.keywords);
				WRITE_TOKEN_STAT("string_literals", code_stat[j].stat.string_literals);
				WRITE_TOKEN_STAT("char_literals", code_stat[j].stat.char_literals);
				WRITE_TOKEN_STAT("int_literals", code_stat[j].stat.int_literals);
				WRITE_TOKEN_STAT("hex_int_literals", code_stat[j].stat.hex_int_literals);
				WRITE_TOKEN_STAT("float_literals", code_stat[j].stat.float_literals);
				WRITE_TOKEN_STAT("hex_float_literals", code_stat[j].stat.hex_float_literals);
				WRITE_TOKEN_STAT("misc", code_stat[j].stat.misc);
				WRITE_TOKEN_STAT("includes", code_stat[j].stat.includes);

				skip = snprintf(cursor, space, "\nbytes = %llu\nbyte_freq = {\n", code_stat[j].stat.bytes);
				cursor += skip;
				space  -= skip;

				ASSERT(ARRAY_LEN(code_stat[j].stat.byte_freq) % 8 == 0);
				for (umm i = 0; i < ARRAY_LEN(code_stat[j].stat.byte_freq); i += 8)
				{
					for (umm k = 0; k < 8; ++k)
					{
						u8 c = (u8)(i + k);

						skip = snprintf(cursor, space, (c >= 0x20 && c <= 0x7E ? "   '%c' = %20llu," : "  0x%02X = %20llu,"), c, code_stat[j].stat.byte_freq[c]);
						cursor += skip;
						space  -= skip;
					}

					skip = snprintf(cursor, space, "\n");
					cursor += skip;
					space  -= skip;
				}

				skip = snprintf(cursor, space, "}\n");
				cursor += skip;
				space  -= skip;
			}

			DWORD to_write = (DWORD)(write_buffer_size - space);

			DWORD bytes_written = 0;
			if (!WriteFile(manifest_file, write_buffer, to_write, &bytes_written, 0) || bytes_written != to_write)
			{
				fprintf(stderr, "ERROR: Failed to write manifest\n");
			}
		}

		CloseHandle(manifest_file);
	}

	PathBuilder_PopToMarker(&out_path, out_marker);

	return 0;
}
