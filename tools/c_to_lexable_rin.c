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

void
Clean(u8* buffer)
{
	u8* scan = buffer;
	for (;;)
	{
		{
			for (;;)
			{
				if (*scan == ' ' || *scan == '\t' || *scan == '\r' || *scan == '\v' || *scan == '\f' || *scan == '\n')
				{
					++scan;
				}
				else if (*scan == '\\')
				{
					*scan = ' ';
					++scan;
				}
				else break;
			}
		}

		if (*scan == 0) break;
		else if (scan[0] == '/' && scan[1] == '/')
		{
			scan += 2;

			while (*scan != 0 && *scan != '\n')
			{
				if (*scan == '\\')
				{
					*scan = ' ';
				}
				else ++scan;
			}
		}
		else if (scan[0] == '/' && scan[1] == '*')
		{
			scan += 2;

			while (*scan != 0 && !(scan[0] == '*' && scan[1] == '/'))
			{
				++scan;
			}

			assert(scan[0] == '*' && scan[1] == '/');
			scan += 2;
		}
		else if (Char_IsAlpha(*scan) || *scan == '_')
		{
			while (Char_IsAlpha(*scan) || Char_IsDigit(*scan) || *scan == '_') ++scan;

			assert(!(scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n' && (Char_IsAlpha(scan[3]) || Char_IsDigit(scan[3]) || scan[3] == '_')));
		}
		else if (Char_IsDigit(*scan) || *scan == '.')
		{
			if (scan[0] == '0' && (scan[1]&0xDF) == 'X')
			{
				scan += 2;

				u8* start = scan;
				while (Char_IsDigit(*scan) || (u8)((*scan&0xDF) - 'A') <= (u8)('F' - 'A')) ++scan;

				if (scan - start > 16)
				{
					scan[-1] = ' ';
				}
				else if (*scan == '.')
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
				while (Char_IsDigit(*scan) || *scan == '_') ++scan;

				if ((*scan&0xDF) == 'H')
				{
					*scan = ' ';
				}
				else
				{
					if (*scan == '.')
					{
						*scan = ' ';

						++scan;
						while (Char_IsDigit(*scan)) ++scan;
					}

					if ((*scan&0xDF) == 'E')
					{
						*scan = ' ';
						++scan;
						if (*scan == '+' || *scan == '-') ++scan;
						while (Char_IsDigit(*scan)) ++scan;
					}
				}
			}
		}
		else if (strncmp((char*)scan, "#include", sizeof("#include")-1) == 0)
		{
			u8* le_pos = scan;
			u8* ge_pos = scan;

			while (*scan != '\n')
			{
				if      (*scan == '<') le_pos = scan;
				else if (*scan == '>') ge_pos = scan;

				++scan;
			}

			if (*le_pos == '<') *le_pos = '"';
			if (*ge_pos == '>') *ge_pos = '"';
		}
		else if (*scan == '"' || *scan == '\'')
		{
			u8 terminator = *scan;
			++scan;
			
			while (*scan != terminator)
			{
				assert(*scan != 0);
				if (scan[0] == '\\' && (scan[1] == terminator || scan[1] == '\\'))
				{
					scan += 2;
				}
				else if (scan[0] == '\\' && scan[1] == '\r' && scan[2] == '\n')
				{
					scan[0] = ' ';
					scan[1] = ' ';
					scan[2] = ' ';
					scan += 3;
				}
				else if (*scan == '\r' || *scan == '\n')
				{
					*scan = ' ';
					++scan;
				}
				else if (scan[0] == '\\')
				{
					u8 accepted_seqs[] = { 'a', 'b', 'e', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"', 'u', 'U' };

					bool is_accepted = false;
					for (umm i = 0; i < sizeof(accepted_seqs)/sizeof(accepted_seqs[0]) && !is_accepted; ++i)
					{
						is_accepted = (scan[1] == accepted_seqs[i]);
					}

					if (!is_accepted)
					{
						*scan = ' ';
					}

					scan += 2;
				}
				else ++scan;
			}

			++scan;
		}
		else
		{
			if (*scan < 0x20 || *scan >= 0x7F || *scan == '`') *scan = ' ';

			++scan;
		}
	}

	for (scan = buffer; *scan != 0; ++scan)
	{
		if (*scan == '\r') *scan = ' ';
	}
}

void
Crawl(wchar_t* filename, u32 filename_len, u8* buffer, HANDLE out)
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

				Crawl(filename, new_filename_len, buffer, out);

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
				if (find_data.nFileSizeLow == 0)
				{
					//fwprintf(stderr, L"%s/%s\n", filename, find_data.cFileName);
				}
				else
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

					Clean(buffer);
					DWORD bytes_written = 0;
					assert(WriteFile(out, buffer, find_data.nFileSizeLow, &bytes_written, 0) && bytes_written == find_data.nFileSizeLow);
				}
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
	if (argc != 3)
	{
		fprintf(stderr, "Invalid Arguments. Expected: c_to_lexable_rin [path to c source directory] [path to output]\n");
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

	HANDLE out = CreateFileW(argv[2], GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	assert(out != INVALID_HANDLE_VALUE);

	Crawl(filename, filename_len, buffer, out);

	assert(CloseHandle(out));

	return 0;
}
