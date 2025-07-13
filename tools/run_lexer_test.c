#include "common.h"
#include <immintrin.h>
#include "../src/float_parsing.h"
#include "../src/memory.h"
#include "../src/virtual_array.h"
#include "../src/tokens.h"
#include "../src/lexer.h"
#include "reference_lexer.h"

bool
VerifyLexer(String input)
{
	bool encountered_errors = false;

	Virtual_Array ref_token_array  = VA_Create(sizeof(Token), 3ULL << 30, 1024);
	Virtual_Array ref_string_array = VA_Create(sizeof(u8), 1ULL << 30, 256);
	Token* ref_tokens   = 0;
	u32 ref_token_count = 0;

	bool ref_succeeded = RefLexFile(input, &ref_token_array, &ref_string_array, &ref_tokens, &ref_token_count);
	ASSERT(ref_succeeded);

	Virtual_Array token_array  = VA_Create(sizeof(Token), 3ULL << 30, 1024);
	Virtual_Array string_array = VA_Create(sizeof(u8), 1ULL << 30, 256);
	Token* tokens   = 0;
	u32 token_count = 0;

	bool succeeded = LexFile(input, &token_array, &string_array, &tokens, &token_count);
	ASSERT(succeeded);

	for (u32 i = 0; i < MIN(token_count, ref_token_count); ++i)
	{
		Token token = tokens[i];
		Token ref_token = ref_tokens[i];

		String token_string     = TokenKind__ToString(token.kind);
		String ref_token_string = TokenKind__ToString(ref_token.kind);
		
		if (token.kind != ref_token.kind || token.len != ref_token.len || (token.kind != Token_EOF && token.offset != ref_token.offset))
		{
			fprintf(stderr, "TOKEN MISMATCH\n");
			fprintf(stderr, "got:      kind = %18.*s, len = %5u, offset = %10u\n", (int)token_string.len, (char*)token_string.data, token.len, token.offset);
			fprintf(stderr, "expected: kind = %18.*s, len = %5u, offset = %10u\n", (int)ref_token_string.len, (char*)ref_token_string.data, ref_token.len, ref_token.offset);
			encountered_errors = true;
			break;
		}

		Token_Data* token_data     = (Token_Data*)&tokens[i+1];
		Token_Data* ref_token_data = (Token_Data*)&ref_tokens[i+1];

		if ((token.kind == Token_Int || token.kind == Token_Char) && token_data->integer != ref_token_data->integer)
		{
			fprintf(stderr, "DATA MISMATCH\n");
			fprintf(stderr, "got:      kind = %18.*s, len = %5u, offset = %10u, data = %20llu\n", (int)token_string.len, (char*)token_string.data, token.len, token.offset, token_data->integer);
			fprintf(stderr, "expected: kind = %18.*s, len = %5u, offset = %10u, data = %20llu\n", (int)ref_token_string.len, (char*)ref_token_string.data, ref_token.len, ref_token.offset, ref_token_data->integer);
			encountered_errors = true;
			break;
		}
		else if (token.kind == Token_Float && token_data->floating != ref_token_data->floating)
		{
			fprintf(stderr, "DATA MISMATCH\n");
			fprintf(stderr, "got:      kind = %18.*s, len = %5u, offset = %10u, data = %.16f\n", (int)token_string.len, (char*)token_string.data, token.len, token.offset, token_data->floating);
			fprintf(stderr, "expected: kind = %18.*s, len = %5u, offset = %10u, data = %.16f\n", (int)ref_token_string.len, (char*)ref_token_string.data, ref_token.len, ref_token.offset, ref_token_data->floating);
			encountered_errors = true;
			break;
		}
		else if (token.kind == Token_String && strncmp((char*)token_data->string, (char*)ref_token_data->string, token.len) != 0)
		{
			fprintf(stderr, "DATA MISMATCH\n");
			fprintf(stderr, "got:      kind = %18.*s, len = %5u, offset = %10u, data =\"%.*s\"\n", (int)token_string.len, (char*)token_string.data, token.len, token.offset, (int)ref_token.len, (char*)token_data->string);
			fprintf(stderr, "expected: kind = %18.*s, len = %5u, offset = %10u, data =\"%.*s\"\n", (int)ref_token_string.len, (char*)ref_token_string.data, ref_token.len, ref_token.offset, ref_token.len, ref_token_data->string);
			encountered_errors = true;
			break;
		}

		if (token.kind == Token_Int || token.kind == Token_Char || token.kind == Token_Float || token.kind == Token_String)
		{
			i += 1;
		}
	}

	if (!encountered_errors && token_count != ref_token_count)
	{
		fprintf(stderr, "TOKEN COUNT MISMATCH\n");
		fprintf(stderr, "got:      %10u\n", token_count);
		fprintf(stderr, "expected: %10u\n", ref_token_count);
		encountered_errors = true;
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

#if 0
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
