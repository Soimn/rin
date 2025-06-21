#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#undef far
#undef near

#include <immintrin.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

typedef double f64;

#define ARRAY_LEN(A) (sizeof(A)/sizeof(0[A]))

static void AssertionFailed(const char* file, int line, const char* expr);
#define ASSERT(EX) ((EX) ? 1 : (IsDebuggerPresent() ? (*(volatile int*)0 = 0) : (AssertionFailed(__FILE__, __LINE__, #EX), 0)))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

static void
AssertionFailed(const char* file, int line, const char* expr)
{
	fprintf(stderr, "%s(%d): Assertion '%s' failed\n", file, line, expr);
	exit(1);
}

static u64
BasicSkip(u8* contents)
{
	u64 as = 0;
	u64 bs = 0;

	for (u8* cursor = contents;;)
	{
		while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

		if (*cursor == 'a')
		{
			++as;
			++cursor;
		}
		else if (*cursor == 'b')
		{
			++bs;
			++cursor;
		}
		else break;
	}

	return as/bs;
}

static u64
BasicSignedSkip(u8* contents)
{
	u64 as = 0;
	u64 bs = 0;

	for (u8* cursor = contents;;)
	{
		while ((s8)(*cursor + 0x5F) > (s8)0x5F) ++cursor;

		if (*cursor == 'a')
		{
			++as;
			++cursor;
		}
		else if (*cursor == 'b')
		{
			++bs;
			++cursor;
		}
		else break;
	}

	return as/bs;
}

static u64
SIMDBasicSignedSkip(u8* contents)
{
	u64 as = 0;
	u64 bs = 0;

	__m256i hex_5f = _mm256_set1_epi8(0x5F);

	for (u8* cursor = contents;;)
	{
		for (;;)
		{
			__m256i c = _mm256_loadu_si256((__m256i*)cursor);
			int whitespace_mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(c, hex_5f), hex_5f));

			if (whitespace_mask == -1)
			{
				cursor += 32;
				continue;
			}
			else
			{
				unsigned long skip;
				if (_BitScanForward(&skip, whitespace_mask+1) == 0) skip = 0;
				cursor += skip;
				break;
			}
		}

		if (*cursor == 'a')
		{
			++as;
			++cursor;
		}
		else if (*cursor == 'b')
		{
			++bs;
			++cursor;
		}
		else break;
	}

	return as/bs;
}

typedef struct Test
{
	char* name;
	u64 (*func)(u8* contents);
} Test;

Test Tests[] = {
	{ .name = "BasicSkip",            .func = BasicSkip           },
	{ .name = "BasicSignedSkip",      .func = BasicSignedSkip     },
	{ .name = "SIMDBasicSignedSkip",  .func = SIMDBasicSignedSkip },
};

int
main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Invalid number of arguments. Expected: %s [max skip to test] [path to output csv]\n", argv[0]);
		return 1;
	}

	umm max_skip = 0;
	{
		u8* scan = (u8*)argv[1];
		for (; *scan != 0; ++scan)
		{
			if ((u8)(*scan - '0') >= (u8)10)
			{
				max_skip = 0;
				break;
			}

			max_skip = max_skip*10 + (*scan&0xF);
		}

		u64 max_allowed_skip_lg2 = 16;
		if (max_skip == 0 || max_skip > (1ULL << max_allowed_skip_lg2) || scan - (u8*)argv[1] >= 19)
		{
			fprintf(stderr, "Max skip must be a valid positive integer in the range [1, 2^%llu]\n", max_allowed_skip_lg2);
			return 1;
		}
	}

	FILE* out = 0;
	{
		if (fopen_s(&out, argv[2], "wb") != 0)
		{
			fprintf(stderr, "Failed to open output file for writing\n");
			return 1;
		}
	}

	u64* results = VirtualAlloc(0, ARRAY_LEN(Tests)*max_skip, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	ASSERT(results != 0);

	u64 text_len = 1ULL << 25;
	u8* text = VirtualAlloc(0, text_len + 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	ASSERT(text != 0);

	text[text_len] = 0;

	u8* skip_template = malloc(max_skip*2 + 2);
	ASSERT(skip_template != 0);

	for (umm i = 1; i <= max_skip; ++i)
	{
		u64 reference_as_div_bs = 0;
		{
			u64 skip_template_len = i*2 + 2;

			memset(skip_template, ' ', i);
			skip_template[i] = 'a';
			memset(skip_template+i+1, ' ', i);
			skip_template[2*i+1] = 'b';

			umm cursor = 0;
			for (; cursor < text_len; cursor += skip_template_len)
			{
				memcpy(text + cursor, skip_template, skip_template_len);
			}

			u64 as = text_len/skip_template_len;
			u64 bs = text_len/skip_template_len;

			umm k = text_len - cursor;
			if (k > i) k = i;
			memset(text + cursor, ' ', k);

			cursor += k;

			if (cursor < text_len)
			{
				text[cursor++] = 'a';
				++as;
			}

			k = text_len - cursor;
			if (k > i) k = i;
			memset(text + cursor, ' ', k);

			cursor += k;

			text[cursor] = 0;

			reference_as_div_bs = as/bs;
		}

		for (umm j = 0; j < ARRAY_LEN(Tests); ++j)
		{
			u64 min_t = ~0ULL;

			for (umm k = 0; k < 10; ++k)
			{
				LARGE_INTEGER start_t;
				QueryPerformanceCounter(&start_t);

				u64 as_div_bs = Tests[j].func(text);

				LARGE_INTEGER end_t;
				QueryPerformanceCounter(&end_t);

				u64 t = end_t.QuadPart - start_t.QuadPart;

				if (t < min_t) min_t = t;

				ASSERT(as_div_bs == reference_as_div_bs);
			}

			results[(i-1)*ARRAY_LEN(Tests) + j] = min_t;
		}
	}

	LARGE_INTEGER perf_freq;
	QueryPerformanceFrequency(&perf_freq);

	fprintf(out, "skip");
	for (umm i = 0; i < ARRAY_LEN(Tests); ++i) fprintf(out, ",%s", Tests[i].name);
	fprintf(out, "\n");

	for (umm i = 1; i <= max_skip; ++i)
	{
		fprintf(out, "%llu", i);

		for (umm j = 0; j < ARRAY_LEN(Tests); ++j)
		{
			u64 ticks = results[(i-1)*ARRAY_LEN(Tests) + j];

			f64 text_size_mb = (f64)text_len/(1ULL << 20);
			//printf("%llu, %f, %f\n", ticks, text_size_mb, (f64)ticks/perf_freq.QuadPart);
			f64 mbs = (f64)(text_size_mb*perf_freq.QuadPart)/ticks;

			ASSERT((u64)(0.5 + (mbs*ticks)/text_size_mb) == (u64)perf_freq.QuadPart);

			fprintf(out, ",% 11.6f", mbs);
		}

		fprintf(out, "\n");
	}

	fclose(out);

	return 0;
}
