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
			u32 file_size = GetFileSize(test_file, &(DWORD){0});
			u8* file_data = VirtualAlloc(0, file_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			DWORD bytes_read = 0;
			if (file_data != 0 && ReadFile(test_file, file_data, file_size, &bytes_read, 0) && bytes_read == file_size)
			{
				stats = *(Stats*)file_data;
				input.data = file_data + sizeof(Stats);
				input.len  = file_size - sizeof(Stats);

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
		Lexer lexer = Lexer_Init(input.data);

		LARGE_INTEGER start_t;
		QueryPerformanceCounter(&start_t);

		for (umm j = 0;; ++j)
		{
			Token token = Lexer_NextToken(&lexer);
			tokens[j] = token;
			if (token.kind == Token_EOF) break;
		}

		LARGE_INTEGER end_t;
		QueryPerformanceCounter(&end_t);

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
