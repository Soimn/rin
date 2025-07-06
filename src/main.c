// NOTE: Constraints
// max total file size: ~~4GB~~ 1GB => max token memory 2GB
// offset/line/col are rarely used and can be recomputed on demand
//
// Stats (taken from create_lexer_test manifest after running it on almost the entirity of linux source code)
//
// total bytes  = 1377560318
// total tokens = 163646974
// tokens per byte = 0.11879477933
// bytes per token = 8.41787834097
// max total file size of 1 GB => 1GB * tokens per byte => 127554923 max tokens
// 16 byte tokens => max token memory footprint of 1.9007 GB => max token memory of 2 GB
// 8 byte symbol tokens & 16 byte data tokens => max token memory footprint of 1.4117 GB => max token memory of 2 GB
//
//                        |     # of occurences    |   total bytes used   | min |   max  |   avg
// -----------------------+------------------------+----------------------+-----+--------+---------
//                  lines |     34829634           | 1377559391 (99.999%) |   2 |  61029 | 39.55136 
//             whitespace |     79709901           |  333749680 (24.228%) |   1 |    298 |  4.18705 
//   single_line_comments |       536179           |   19367355 ( 1.406%) |   3 |    469 | 36.12106 
//         block_comments |      1722994           |  154968627 (11.249%) |   4 | 173195 | 89.94148 
//                 idents |     65149704 (39.811%) |  702669299 (51.008%) |   1 |    152 | 10.78546 
//               keywords |     16206063 ( 9.903%) |   57341598 ( 4.163%) |   1 |      8 |  3.53828 
//        string_literals |      1360654 ( 0.831%) |   28422563 ( 2.063%) |   2 |  18309 | 20.88890 
//          char_literals |        32454 ( 0.020%) |     104375 ( 0.008%) |   2 |    483 |  3.21609 
//           int_literals |      5297763 ( 3.237%) |    8423396 ( 0.611%) |   1 |     23 |  1.58999 
//       hex_int_literals |      7598090 ( 4.643%) |   45613046 ( 3.311%) |   2 |     32 |  6.00323 
//         float_literals |        10498 ( 0.006%) |      44166 ( 0.003%) |   2 |     22 |  4.20709 
//                   misc |     84197811 (51.451%) |   84197811 ( 6.112%) |   1 |      1 |  1.00000 

#include <immintrin.h>
#include <stdint.h>

#include "common.h"
#include "memory.h"
#include "virtual_array.h"
#include "tokens.h"
#include "lexer.h"

// -------------------------------------------------------------------------------------------------------------------------------------

// NOTE: undef before including external code just to be safe
#undef ALIGN
#undef PAGE_SIZE

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

#include <stdio.h>
#include <stdlib.h>

#define ALIGN(N, A) (((umm)(N) + (umm)(A)-1) & (umm)-(smm)(A))
#define PAGE_SIZE 4096

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

int
main(int argc, char** argv)
{
	(void)argc, (void)argv;

	return 0;
}
