#include "common.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

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
Clean__Replace(u8* contents, u32* read_cursor, u32* write_cursor, u8 c)
{
	contents[*write_cursor] = c;
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
	else if (((contents[*read_cursor]&0xDF) == 'U' && (contents[*read_cursor+1]&0xDF) == 'L' && (contents[*read_cursor+2]&0xDF) == 'L') ||
					 ((contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'L' && (contents[*read_cursor+2]&0xDF) == 'U'))
	{
		*read_cursor += 3;
	}
	else if (((contents[*read_cursor]&0xDF) == 'U' && (contents[*read_cursor+1]&0xDF) == 'L') ||
					 ((contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'U') ||
					 ((contents[*read_cursor]&0xDF) == 'L' && (contents[*read_cursor+1]&0xDF) == 'L'))
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
		else if (Char_IsAlpha(contents[read_cursor]) || contents[read_cursor] == '_')
		{
			while (Char_IsAlpha(contents[read_cursor]) || Char_IsDigit(contents[read_cursor]) || contents[read_cursor] == '_')
			{
				Clean__Accept(contents, &read_cursor, &write_cursor);
			}
		}
		else if (strncmp((char*)contents + read_cursor, ".macro", sizeof(".macro")-1) == 0)
		{
			while (contents[read_cursor] != 0 && (contents[read_cursor] != '.' || strncmp((char*)contents + read_cursor, ".endm", sizeof(".endm")-1) != 0))
			{
				Clean__Reject(&read_cursor);
			}

			if (contents[read_cursor] != 0)
			{
				for (umm i = 0; i < sizeof(".endm")-1; ++i) Clean__Reject(&read_cursor);
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

			for (umm i = 0; Char_IsDigit(contents[read_cursor]) || Char_IsHexAlpha(contents[read_cursor]); ++i)
			{
				if (i < 16) Clean__Accept(contents, &read_cursor, &write_cursor);
				else        Clean__Reject(&read_cursor);
			}

			if (contents[read_cursor] == '.')
			{
				Clean__Reject(&read_cursor);

				while (Char_IsDigit(contents[read_cursor]) || Char_IsHexAlpha(contents[read_cursor]))
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
		else if (Char_IsDigit(contents[read_cursor]) && contents[read_cursor+1] == '.' && !Char_IsDigit(contents[read_cursor+2]))
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
			Clean__Reject(&read_cursor);
		}
		else if (Char_IsDigit(contents[read_cursor]) && contents[read_cursor+1] == 'h')
		{
			Clean__Accept(contents, &read_cursor, &write_cursor);
			Clean__Reject(&read_cursor);
		}
		else if (Char_IsDigit(contents[read_cursor]))
		{
			for (umm i = 0; Char_IsDigit(contents[read_cursor]); ++i)
			{
				if (i < 19) Clean__Accept(contents, &read_cursor, &write_cursor);
				else        Clean__Reject(&read_cursor);
			}

			if(contents[read_cursor] == '_')
			{
				Clean__Replace(contents, &read_cursor, &write_cursor, ' ');
			}
			else
			{
				if (contents[read_cursor] == '.')
				{
					if (!Char_IsDigit(contents[read_cursor+1]))
					{
						Clean__Reject(&read_cursor);
					}
					else
					{
						Clean__Accept(contents, &read_cursor, &write_cursor);

						while (Char_IsDigit(contents[read_cursor]))
						{
							Clean__Accept(contents, &read_cursor, &write_cursor);
						}
					}
				}

				if ((contents[read_cursor]&0xDF) == 'E')
				{
					if (Char_IsDigit(contents[read_cursor+1]) || ((contents[read_cursor+1] == '+' || contents[read_cursor+1] == '-') && Char_IsDigit(contents[read_cursor+2])))
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
					else
					{
						Clean__Reject(&read_cursor);
					}
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
					if (contents[read_cursor] > 0x7F) Clean__Reject(&read_cursor);
					else                              Clean__Accept(contents, &read_cursor, &write_cursor);
				}
				else
				{
					u8 accepted_seqs[] = { 'a', 'b', 'e', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"', 'U' };

					bool is_accepted = false;
					for (umm i = 0; i < ARRAY_LEN(accepted_seqs); ++i)
					{
						if (contents[read_cursor+1] == accepted_seqs[i])
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
					else if (contents[read_cursor+1] == 'x')
					{
						if ((Char_IsDigit(contents[read_cursor+2]) || Char_IsHexAlpha(contents[read_cursor+2])) &&
								(Char_IsDigit(contents[read_cursor+3]) || Char_IsHexAlpha(contents[read_cursor+3])))
						{
							Clean__Accept(contents, &read_cursor, &write_cursor);
							Clean__Accept(contents, &read_cursor, &write_cursor);
							Clean__Accept(contents, &read_cursor, &write_cursor);
							Clean__Accept(contents, &read_cursor, &write_cursor);
						}
						else
						{
							Clean__Reject(&read_cursor);
							Clean__Reject(&read_cursor);
						}
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
		else if (contents[read_cursor] >= 0x7F || contents[read_cursor] == '`' || contents[read_cursor] == '\\')
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

void
Crawl(Stats* stats, Path_Builder in_path, u8** file_buffer_cursor, u64* file_buffer_space)
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
				Crawl(stats, in_path, file_buffer_cursor, file_buffer_space);
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
				if ((u64)find_data.nFileSizeLow+1 > *file_buffer_space || find_data.nFileSizeHigh != 0)
				{
					fwprintf(stderr, L"WARNING: File '%s' is too large.\n", PathBuilder_CurrentPath(&in_path));
				}
				else
				{
					u32 file_size = find_data.nFileSizeLow;

					DWORD bytes_read = 0;
					if (!ReadFile(in_file, *file_buffer_cursor, file_size, &bytes_read, 0) || bytes_read != file_size)
					{
						fwprintf(stderr, L"WARNING: Failed to read '%s'.\n", PathBuilder_CurrentPath(&in_path));
					}
					else
					{
						(*file_buffer_cursor)[file_size] = 0;

						if (((*file_buffer_cursor)[0] == 0xFF && (*file_buffer_cursor)[1] == 0xFE) ||
								((*file_buffer_cursor)[0] == 0xFE && (*file_buffer_cursor)[1] == 0xFF))
						{
							fwprintf(stderr, L"WARNING: Skipping non UTF-8 file '%s'.\n", PathBuilder_CurrentPath(&in_path));
						}
						else
						{
							Code_Stats pre_clean = CodeStats_Gather(*file_buffer_cursor);
							u32 write_size = Clean(*file_buffer_cursor);
							Code_Stats post_clean = CodeStats_Gather(*file_buffer_cursor);

							*file_buffer_cursor += write_size;
							*file_buffer_space  -= write_size;

							CodeStats_Combine(&stats->pre_clean, pre_clean);
							CodeStats_Combine(&stats->post_clean, post_clean);
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
		fwprintf(stderr, L"ERROR: Invalid number of arguments.\nExpected: %s [path to directory of c files] [path to output file]\n", argv[0]);
		return 1;
	}

	wchar_t* in_dir   = argv[1];
	wchar_t* out_path = argv[2];

	if (!PathIsDirectoryW(in_dir))
	{
		fwprintf(stderr, L"ERROR: Input is not a directory ('%s')\n", in_dir);
		return 1;
	}

	Path_Builder in_path = PathBuilder_Init(2*MAX_PATH);

	u64 file_buffer_cap = 1ULL << 32;
	u8* file_buffer = VirtualAlloc(0, file_buffer_cap, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (file_buffer == 0)
	{
		fprintf(stderr, "ERROR: Failed to allocate memory\n");
		return 1;
	}

	PathBuilder_Push(&in_path, in_dir);

	u8* file_buffer_cursor = file_buffer;
	u64 file_buffer_space = file_buffer_cap;

	Stats* stats = (Stats*)file_buffer_cursor;
	*stats = (Stats){
		.pre_clean  = CodeStats_Init(),
		.post_clean = CodeStats_Init(),
	};
	file_buffer_cursor += sizeof(Stats);
	file_buffer_space  -= sizeof(Stats);

	Crawl(stats, in_path, &file_buffer_cursor, &file_buffer_space);

	u64 zpad = 65;
	ASSERT(file_buffer_space >= zpad);
	file_buffer_cursor += zpad;
	file_buffer_space  -= zpad;

	HANDLE out_file = CreateFileW(out_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (out_file == INVALID_HANDLE_VALUE)
	{
		fwprintf(stderr, L"ERROR: Failed to create output file at '%s'.\n", out_path);
		return 1;
	}
	else
	{
		u64 file_buffer_size = file_buffer_cap - file_buffer_space;
		ASSERT(file_buffer_size < ~(u32)0);

		DWORD bytes_written = 0;
		if (!WriteFile(out_file, file_buffer, (u32)file_buffer_size, &bytes_written, 0) || bytes_written != file_buffer_size)
		{
			fwprintf(stderr, L"ERROR: Failed to write output file at '%s'.\n", out_path);
			return 1;
		}

		CloseHandle(out_file);
	}

	return 0;
}
