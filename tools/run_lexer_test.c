#include "common.h"
#include <immintrin.h>
#include "../src/tokens.h"
#include "../src/lexer.h"
#include "reference_lexer.h"

void
PrintToken(FILE* stream, Token token)
{
	String kind_string = TokenKind__ToString(token.kind);

	fprintf(stream, "kind = %.*s (%u), offset = %u, len = %u, data = %llu (", (int)kind_string.len, kind_string.data, token.kind, token.offset, token.len, (u64)token.data);
	if      (token.kind == Token_Ident) fprintf(stream, "\"%.*s\"", (int)token.len, token.data);
	else if (token.kind == Token_Int)    fprintf(stream, "%llu", token.integer);
	else if (token.kind == Token_Float)  fprintf(stream, "%.16f", token.floating);
	else if (token.kind == Token_String) fprintf(stream, "\"%.*s\"", (int)token.len, token.data);
	else if (token.kind == Token_Char)   fprintf(stream, "'%.*s'", (int)token.len, token.data);
	fprintf(stream, ")");
}

bool
TokensEqual(Token a, Token b)
{
	bool result = false;

	if (a.kind == b.kind && a.offset == b.offset && a.len == b.len)
	{
		if (a.kind == Token_String || a.kind == Token_Char || a.kind == Token_Ident)
		{
			result = (strncmp((char*)a.data, (char*)b.data, a.len) == 0);
		}
		else
		{
			result = (a.data == b.data);
		}
	}

	return result;
}

bool
VerifyLexer(String input)
{
	Lexer lexer         = Lexer_Init(input.data);
	Ref_Lexer ref_lexer = RefLexer_Init(input.data);

	for (;;)
	{
		Token token     = Lexer_NextToken(&lexer);
		Token ref_token = RefLexer_NextToken(&ref_lexer);

		if (!TokensEqual(token, ref_token))
		{
			fprintf(stderr, "TOKEN MISMATCH\n");
			fprintf(stderr, "\nlexer output:\n");
			PrintToken(stderr, token);
			fprintf(stderr, "\n\nreference output:\n");
			PrintToken(stderr, ref_token);
			fprintf(stderr, "\n");

			return false;
		}

		if (ref_token.kind == Token_EOF) break;
	}

	return true;
}

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		fwprintf(stderr, L"ERROR: Invalid number of arguments. Expected: %s [path to test file directory]\n", argv[0]);
		return 1;
	}

	Path_Builder in_path = PathBuilder_Init(2*MAX_PATH);
	PathBuilder_Push(&in_path, argv[1]);

	Stats stats = {0};
	{
		bool succeeded = false;

		Path_Marker manifest_marker = PathBuilder_Push(&in_path, L"manifest.rltm");
		HANDLE manifest_file = CreateFileW(PathBuilder_CurrentPath(&in_path), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (manifest_file == INVALID_HANDLE_VALUE)
		{
			fwprintf(stderr, L"ERROR: Failed to open manifest file '%s'\n", PathBuilder_CurrentPath(&in_path));
		}
		else
		{
			DWORD bytes_read = 0;
			if (!ReadFile(manifest_file, &stats, sizeof(Stats), &bytes_read, 0) || bytes_read != sizeof(Stats))
			{
				fwprintf(stderr, L"ERROR: Failed to read manifest file '%s'\n", PathBuilder_CurrentPath(&in_path));
			}
			else
			{
				succeeded = true;
			}
		}

		if (manifest_file != INVALID_HANDLE_VALUE) CloseHandle(manifest_file);
		PathBuilder_PopToMarker(&in_path, manifest_marker);

		if (!succeeded)
		{
			return 1;
		}
	}

	u64 sparse_file_buffer_len = 0;
	u64 packed_file_buffer_len = 0;
	u8* sparse_file_buffer = VirtualAlloc(0, (1ULL << 32), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	u8* packed_file_buffer = VirtualAlloc(0, (1ULL << 32), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	String* files          = VirtualAlloc(0, stats.file_count*sizeof(String), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (sparse_file_buffer == 0 || packed_file_buffer == 0 || files == 0)
	{
		fwprintf(stderr, L"ERROR: Failed to allocate memory for files\n");
		return 1;
	}
	else
	{
		u8* sparse_file_cursor = sparse_file_buffer;
		u8* packed_file_cursor = packed_file_buffer;

		for (umm i = 0; i < stats.file_count; ++i)
		{
			bool succeeded = false;

			wchar_t filename[sizeof("0000000000.rlt")] = {
				L'0' + (i / 1000000000) % 10,
				L'0' + (i / 100000000) % 10,
				L'0' + (i / 10000000) % 10,
				L'0' + (i / 1000000) % 10,
				L'0' + (i / 100000) % 10,
				L'0' + (i / 10000) % 10,
				L'0' + (i / 1000) % 10,
				L'0' + (i / 100) % 10,
				L'0' + (i / 10) % 10,
				L'0' + (i / 1) % 10,
				L'.', L'r', L'l', L't', 0
			};

			Path_Marker marker = PathBuilder_Push(&in_path, filename);
			HANDLE file = CreateFileW(PathBuilder_CurrentPath(&in_path), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

			if (file == INVALID_HANDLE_VALUE)
			{
				fwprintf(stderr, L"ERROR: Failed to open test file '%s'\n", PathBuilder_CurrentPath(&in_path));
			}
			else
			{
				DWORD file_size_hi = 0;
				DWORD file_size = GetFileSize(file, &file_size_hi);

				if (file_size_hi != 0 || (umm)(sparse_file_cursor - sparse_file_buffer) + file_size > (1ULL << 32))
				{
					fwprintf(stderr, L"ERROR: Test file is too large '%s'\n", PathBuilder_CurrentPath(&in_path));
				}
				else
				{
					DWORD bytes_read = 0;
					if (!ReadFile(file, packed_file_cursor, file_size, &bytes_read, 0) || bytes_read != file_size)
					{
						fwprintf(stderr, L"ERROR: Failed to read test file '%s'\n", PathBuilder_CurrentPath(&in_path));
					}
					else
					{
						memcpy(sparse_file_cursor, packed_file_cursor, file_size);

						files[i] = (String){ .data = sparse_file_cursor, .len = file_size };

						packed_file_cursor += file_size;
						sparse_file_cursor += file_size + 65; // NOTE: padding each file with zeros
						succeeded = true;
					}
				}
			}

			if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
			PathBuilder_PopToMarker(&in_path, marker);

			if (!succeeded)
			{
				return 1;
			}
		}

		sparse_file_buffer_len = sparse_file_cursor - sparse_file_buffer;
		packed_file_buffer_len = packed_file_cursor - packed_file_buffer;
	}

	ASSERT(packed_file_buffer_len < ~(u32)0);
	String packed_file = { .data = packed_file_buffer, .len = (u32)packed_file_buffer_len };

	ASSERT(packed_file.len == stats.post_clean.bytes);

	if (!VerifyLexer(packed_file))
	{
		fprintf(stderr, "Failed verification\n");
		return 1;
	}
	else printf("Passed verification\n");

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);

	u8* input     = packed_file.data;
	u64 file_size = packed_file.len;
	u64 lines     = stats.post_clean.lines.num;

	printf("\nPacked file throughput (%8.3f MB, %8.3f Mloc)\n", (f64)file_size/(1ULL << 20), (f64)lines/1e6);
	printf("===================================================\n\n");

	printf("    |   time [ms]  | throughput [MB/s] | throughput [Mloc/s]\n");
	printf("----+--------------+-------------------+---------------------\n");

	volatile Token* tokens = VirtualAlloc(0, (1ULL << 32), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memset((void*)tokens, '0', 1ULL << 32);

	u64 min_t_dus    = ~(u64)0;
	f64 min_t_mbps   = 0;
	f64 min_t_mlocps = 0;
	u64 max_t_dus    = 0;
	f64 max_t_mbps   = 0;
	f64 max_t_mlocps = 0;
	for (umm i = 0; i < 10; ++i)
	{
		Lexer lexer = Lexer_Init(input);

		LARGE_INTEGER start_t;
		QueryPerformanceCounter(&start_t);

		for (umm j = 0;; ++j)
		{
			Token token = Lexer_NextToken(&lexer);
			tokens[j] = token;
			if (token.kind == Token_Invalid || token.kind == Token_EOF) break;
		}

		LARGE_INTEGER end_t;
		QueryPerformanceCounter(&end_t);

		u64 t = end_t.QuadPart - start_t.QuadPart;

		u64 t_dus = (10000000*t)/perf_freq.QuadPart;

		f64 throughput = ((f64)file_size*perf_freq.QuadPart)/t;
		f64 mlocps     = (((f64)lines*perf_freq.QuadPart)/t)/1e6;

		ASSERT((u64)(0.5 + (throughput*t)/perf_freq.QuadPart) == (u64)file_size);
		ASSERT((u64)(0.5 + ((mlocps*1e6)*t)/perf_freq.QuadPart) == lines);

		if (t_dus < min_t_dus)
		{
			min_t_dus    = t_dus;
			min_t_mbps   = throughput/(1ULL << 20);
			min_t_mlocps = mlocps;
		}

		if (t_dus > max_t_dus)
		{
			max_t_dus    = t_dus;
			max_t_mbps   = throughput/(1ULL << 20);
			max_t_mlocps = mlocps;
		}

		printf(" %2llu | %4llu.%04llu ms |   % 8.3f MB/s   |   % 8.3f Mloc/s\n", i, t_dus/10000, t_dus%10000, throughput/(1ULL << 20), mlocps);
	}

	printf("best:  %4llu.%04llu ms, % 8.3f MB/s, % 8.3f Mloc/s\n", min_t_dus/10000, min_t_dus%10000, min_t_mbps, min_t_mlocps);
	printf("worst: %4llu.%04llu ms, % 8.3f MB/s, % 8.3f Mloc/s\n", max_t_dus/10000, max_t_dus%10000, max_t_mbps, max_t_mlocps);

	return 0;
}
