#include "common.h"
#include <immintrin.h>
#include "../src/memory.h"
#include "../src/virtual_array.h"
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
	bool encountered_errors = false;

	Virtual_Array ref_token_array = VA_Create(sizeof(Token), 3ULL << 30, 1024);
	Token* ref_tokens   = 0;
	u32 ref_token_count = 0;

	RefLexFile(input, &ref_token_array, &ref_tokens, &ref_token_count); // TODO: Error handling

	Virtual_Array token_array  = VA_Create(sizeof(Token), 3ULL << 30, 1024);
	Virtual_Array string_array = VA_Create(sizeof(u8), 1ULL << 30, 256);
	Token* tokens   = 0;
	u32 token_count = 0;

	LexFile(input, &token_array, &string_array, &tokens, &token_count); // TODO: Error handling

	for (u32 i = 0; i < MIN(token_count, ref_token_count); ++i)
	{
		Token token     = tokens[i];
		Token ref_token = ref_tokens[i];

		if (!TokensEqual(token, ref_token))
		{
			fprintf(stderr, "TOKEN MISMATCH\n");
			fprintf(stderr, "\nlexer output:\n");
			PrintToken(stderr, token);
			fprintf(stderr, "\n\nreference output:\n");
			PrintToken(stderr, ref_token);
			fprintf(stderr, "\n");

			encountered_errors = true;
			break;
		}
	}

	VA_Destroy(&ref_token_array);
	VA_Destroy(&token_array);
	VA_Destroy(&string_array);

	return !encountered_errors;
}

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		fwprintf(stderr, L"ERROR: Invalid number of arguments. Expected: %s [path to test file file]\n", argv[0]);
		return 1;
	}

	wchar_t* test_path = argv[1];

	Stats stats  = {0};
	String input = {0};
	{
		bool succeeded = false;

		HANDLE test_file = CreateFileW(test_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (test_file != INVALID_HANDLE_VALUE)
		{
			u32 zpad = MAX(LEXER_ZPAD, REF_LEXER_ZPAD);

			u32 file_size = GetFileSize(test_file, &(DWORD){0});
			u8* file_data = VirtualAlloc(0, file_size + zpad, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			DWORD bytes_read = 0;
			if (file_data != 0 && ReadFile(test_file, file_data, file_size, &bytes_read, 0) && bytes_read == file_size)
			{
				stats = *(Stats*)file_data;
				input.data = file_data + sizeof(Stats);
				input.len  = file_size - sizeof(Stats) + zpad;

				succeeded = true;
			}

			CloseHandle(test_file);
		}


		if (!succeeded)
		{
			fwprintf(stderr, L"ERROR: Failed to read test file '%s'\n", test_path);
			return 1;
		}
	}

	if (!VerifyLexer(input))
	{
		fprintf(stderr, "Failed verification\n");
		return 1;
	}
	else printf("Passed verification\n");

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);

	u64 lines = stats.post_clean.lines.num;

	printf("\nPacked file throughput (%8.3f MB, %8.3f Mloc)\n", (f64)input.len/(1ULL << 20), (f64)lines/1e6);
	printf("===================================================\n\n");

	printf("    |   time [ms]  | throughput [MB/s] | throughput [Mloc/s]\n");
	printf("----+--------------+-------------------+---------------------\n");

	u64 min_t_dus    = ~(u64)0;
	f64 min_t_mbps   = 0;
	f64 min_t_mlocps = 0;
	u64 max_t_dus    = 0;
	f64 max_t_mbps   = 0;
	f64 max_t_mlocps = 0;
	for (umm i = 0; i < 10; ++i)
	{
		Token* tokens   = 0;
		u32 token_count = 0;

#if 1
		Virtual_Array token_array  = VA_Create(sizeof(Token), 3ULL << 30, 1024);
		Virtual_Array string_array = VA_Create(sizeof(u8), 1ULL << 30, 256);

		VA_EnsureCommitted(&token_array, input.len / 8);
		memset(token_array.data, '0', token_array.committed);

		LARGE_INTEGER start_t;
		QueryPerformanceCounter(&start_t);
#else
		LARGE_INTEGER start_t;
		QueryPerformanceCounter(&start_t);

		Virtual_Array token_array  = VA_Create(sizeof(Token), 3ULL << 30, 1024);
		Virtual_Array string_array = VA_Create(sizeof(u8), 1ULL << 30, 256);
#endif

		LexFile(input, &token_array, &string_array, &tokens, &token_count);

		LARGE_INTEGER end_t;
		QueryPerformanceCounter(&end_t);

		VA_Destroy(&token_array);
		VA_Destroy(&string_array);

		u64 t = end_t.QuadPart - start_t.QuadPart;

		u64 t_dus = (10000000*t)/perf_freq.QuadPart;

		f64 throughput = ((f64)input.len*perf_freq.QuadPart)/t;
		f64 mlocps     = (((f64)lines*perf_freq.QuadPart)/t)/1e6;

		ASSERT((u64)(0.5 + (throughput*t)/perf_freq.QuadPart) == (u64)input.len);
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

		printf(" %2llu | %4llu.%04llu ms |   %8.3f MB/s   |   %8.3f Mloc/s\n", i, t_dus/10000, t_dus%10000, throughput/(1ULL << 20), mlocps);
	}

	printf("best:  %4llu.%04llu ms, %8.3f MB/s, %8.3f Mloc/s\n", min_t_dus/10000, min_t_dus%10000, min_t_mbps, min_t_mlocps);
	printf("worst: %4llu.%04llu ms, %8.3f MB/s, %8.3f Mloc/s\n", max_t_dus/10000, max_t_dus%10000, max_t_mbps, max_t_mlocps);

	return 0;
}
