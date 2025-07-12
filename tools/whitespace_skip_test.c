#include "common.h"
#include <immintrin.h>

__forceinline static u8*
SkipNonWhitespace(u8* cursor)
{
	if (cursor[0] == '/' && cursor[1] == '/')
	{
		while (*cursor != 0 && *cursor != '\n') ++cursor;
		++cursor; // NOTE: Skipping 1 zero is fine since there are at least 65 padded at the end
	}
	else if (cursor[0] == '/' && cursor[1] == '*')
	{
		cursor += 2;
		while (*cursor != 0 && !(cursor[0] == '*' && cursor[1] == '/')) ++cursor;
		cursor += 2; // NOTE: Skipping 2 zeros is fine since the file is padded
	}
	else if (*cursor == '"' || *cursor == '\'')
	{
		u8 terminator = *cursor;
		++cursor;

		while (*cursor != 0 && *cursor != terminator)
		{
			if (*cursor == '\\') ++cursor; // NOTE: Skipping a \ at the end of the file is fine, since the file is padded
			++cursor;
		}
	}
	else
	{
		while ((u8)*cursor > (u8)0x20) ++cursor;
	}

	return cursor;
}

umm
SimpleSkip(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSkipBR1(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		cursor += ((u8)*cursor <= (u8)0x20);

		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSkipBR2(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);

		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSkipBR3(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);

		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSkipBR4(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);

		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSkipBR5(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	for (;;)
	{
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);
		cursor += ((u8)*cursor <= (u8)0x20);

		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleSIMDSkip(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	__m128i hex_21 = _mm_set1_epi8(0x21);
	__m128i zero   = _mm_setzero_si128();

	for (;;)
	{
		__m128i c = _mm_loadu_si128((__m128i*)cursor);
		if (_mm_testz_si128(c, c)) break;

		u32 mask = _mm_movemask_epi8(_mm_cmpgt_epi8(hex_21, c));

		if (mask == 0xFFFF)
		{
			cursor += 16;
			continue;
		}

		unsigned long skip;
		_BitScanForward(&skip, mask+1);
		cursor += skip;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

umm
SimpleAVXSkip(u8* in)
{
	umm result = 0;

	u8* cursor = in;

	__m256i hex_21 = _mm256_set1_epi8(0x21);
	__m256i zero   = _mm256_setzero_si256();

	for (;;)
	{
		__m256i c = _mm256_loadu_si256((__m256i*)cursor);
		if (_mm256_testz_si256(c, c)) break;

		u32 mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(hex_21, c));

		if (mask == 0xFFFFFFFF)
		{
			cursor += 32;
			continue;
		}

		unsigned long skip;
		_BitScanForward(&skip, mask+1);
		cursor += skip;

		if (*cursor == 0) break;
		cursor = SkipNonWhitespace(cursor);
		++result;
	}

	return result;
}

typedef struct Test_Case
{
	umm (*func)(u8* in);
	char* name;
} Test_Case;

Test_Case TestCases[] = {
	{ .func = SimpleSkip,     .name = "SimpleSkip"     },
	{ .func = SimpleSkipBR1,  .name = "SimpleSkipBR1"  },
	{ .func = SimpleSkipBR2,  .name = "SimpleSkipBR2"  },
	{ .func = SimpleSkipBR3,  .name = "SimpleSkipBR3"  },
	{ .func = SimpleSkipBR4,  .name = "SimpleSkipBR4"  },
	{ .func = SimpleSkipBR5,  .name = "SimpleSkipBR5"  },
	{ .func = SimpleSIMDSkip, .name = "SimpleSIMDSkip" },
	{ .func = SimpleAVXSkip,  .name = "SimpleAVXSkip"  },
};

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		//// ERROR
		fwprintf(stderr, L"ERROR: Invalid number of arguments. Expected: %s [path to test file]\n", argv[0]);
		return 1;
	}

	u8* in     = 0;
	umm in_len = 0;
	{
		bool succeeded = false;

		wchar_t* in_path = argv[1];

		HANDLE in_file = CreateFileW(in_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (in_file != INVALID_HANDLE_VALUE)
		{
			DWORD file_size = GetFileSize(in_file, &(DWORD){0});

			umm zpad = 65;
			in = VirtualAlloc(0, file_size + zpad, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			DWORD bytes_read = 0;
			if (in != 0 && ReadFile(in_file, in, file_size, &bytes_read, 0) && bytes_read == file_size)
			{
				for (umm i = 0; i < file_size && in[i] != 0; ++i) ++in_len;

				for (umm i = 0; i < zpad; ++i) in[in_len + i] = 0;

				succeeded = true;
			}

			CloseHandle(in_file);
		}

		if (!succeeded)
		{
			fwprintf(stderr, L"ERROR: failed to read '%s'\n", in_path);
			return 1;
		}
	}

	umm expected_result = SimpleSkip(in);

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);

	f64 mbs = (f64)in_len/(1 << 20);
	printf("file size: %8.3f MB\n", mbs);

	for (umm i = 0; i < ARRAY_LEN(TestCases); ++i)
	{
		Test_Case* test_case = &TestCases[i];

		u64 min_t = ~(u64)0;

		for (umm tries = 0; tries < 20; ++tries)
		{
			LARGE_INTEGER start_t;
			QueryPerformanceCounter(&start_t);

			umm result = test_case->func(in);

			LARGE_INTEGER end_t;
			QueryPerformanceCounter(&end_t);

			u64 t = end_t.QuadPart - start_t.QuadPart;

			if (result != expected_result) fprintf(stderr, "%llu %llu\n", result, expected_result);
			ASSERT(result == expected_result);

			if (t < min_t)
			{
				min_t = t;
				tries = 0;
			}
		}

		f64 ms   = (1000.0*min_t)/perf_freq.QuadPart;
		f64 mbps = (mbs*perf_freq.QuadPart)/min_t;
		printf("%14s %20llu %8.3f ms %8.3f MB/s\n", test_case->name, min_t, ms, mbps);
	}

	return 0;
}
