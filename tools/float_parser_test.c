#include "common.h"
#include <immintrin.h>
#include "../src/float_parsing.h"
#include <stdlib.h>

int
wmain(int argc, wchar_t** argv)
{
	if (argc != 2)
	{
		//// ERROR
		fwprintf(stderr, L"ERROR: Invalid number of arguments. Expected: %s [path to test file]\n", argv[0]);
		return 1;
	}

	u8* in = 0;
	{
		bool successfully_read_file = false;

		wchar_t* in_file_path = argv[1];

		HANDLE in_file = CreateFileW(in_file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (in_file == INVALID_HANDLE_VALUE)
		{
			//// ERROR
			fwprintf(stderr, L"ERROR: Failed to open file '%s'\n", in_file_path);
		}
		else
		{
			DWORD in_file_size = GetFileSize(in_file, &(DWORD){0});

			in = VirtualAlloc(0, in_file_size + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			DWORD bytes_read = 0;
			if (in == 0 || !ReadFile(in_file, in, in_file_size, &bytes_read, 0) || bytes_read != in_file_size)
			{
				//// ERROR
				fwprintf(stderr, L"ERROR: Failed to read file '%s'\n", in_file_path);
			}
			else
			{
				in[in_file_size] = 0;

				successfully_read_file = true;
			}
		}

		if (in_file != INVALID_HANDLE_VALUE) CloseHandle(in_file);

		if(!successfully_read_file) return 1;
	}

	for (u8* cursor = in;;)
	{
		u64 bits          = 0;
		u8* float_start   = 0;
		f64 strtod_result = 0;

		String line = {0};

		{
			bool failed_to_parse = false;

			while ((u8)(*cursor-1) < (u8)0x20) ++cursor;
			if (*cursor == 0) break;

			line.data = cursor;

			while (*cursor != 0 && *cursor != '\r' && *cursor != '\n') ++cursor;

			line.len = (u32)(cursor - line.data);

			for (umm i = 0; i < 16; ++i)
			{
				bits <<= 4;

				if      (Char_IsDigit(line.data[i]))    bits |= line.data[i]&0xF;
				else if (Char_IsHexAlpha(line.data[i])) bits |= 9 + (line.data[i]&0x1F);
				else
				{
					// NOTE: Reading 1 past the end of "line" is fine
					//// ERROR
					failed_to_parse = true;
					break;
				}
			}

			if (!failed_to_parse)
			{
				umm i = 16;
				while (i < line.len && (u8)(line.data[i]-1) < (u8)0x20) ++i;

				if (i >= line.len) failed_to_parse = true;
				else
				{
					float_start = &line.data[i];

					strtod_result = strtod((char*)float_start, 0);

					if (strtod_result == 0 && bits != 0)
					{
						failed_to_parse = true;
					}
					else
					{
						u8* scan = float_start;

						while (*scan == '0') ++scan;

						u8* start = scan;
						while (Char_IsDigit(*scan) || *scan == '.') ++scan;

						if (scan - start > 20)
						{
							failed_to_parse = true;
						}
					}
				}
			}

			if (failed_to_parse)
			{
				fprintf(stderr, "WARNING: Failed to parse test '%.*s'\n", (int)line.len, (char*)line.data);
				continue;
			}
		}

		f64 result = 0;
		Parse_Float_Error error = ParseFloat(float_start, &(u32){0}, &result);

		if (error != ParseFloatError_Success)
		{
			char* reason = 0;
			switch (error)
			{
				case ParseFloatError_MissingLeadingDigits:            reason = "ParseFloatError_MissingLeadingDigits";            break;
				case ParseFloatError_MissingDecimalPoint:             reason = "ParseFloatError_MissingDecimalPoint";             break;
				case ParseFloatError_MissingTrailingDigits:           reason = "ParseFloatError_MissingTrailingDigits";           break;
				case ParseFloatError_MissingDigitsAfterExponent:      reason = "ParseFloatError_MissingDigitsAfterExponent";      break;
				case ParseFloatError_TooManyDigitsInMantissa:         reason = "ParseFloatError_TooManyDigitsInMantissa";         break;
				case ParseFloatError_TooManyDigitsInExponent:         reason = "ParseFloatError_TooManyDigitsInExponent";         break;
				default:                                              reason = "unknown reason";                                  break;
			}

			fprintf(stderr, "ERROR: Failed to convert '%.*s'\n", (int)line.len, (char*)line.data);
			fprintf(stderr, "Reason: %s\n", reason);
		}
		else if (result != strtod_result)
		{
			fprintf(stderr, "ERROR: Failed to convert '%.*s'\n", (int)line.len, (char*)line.data);
			fprintf(stderr, "got: %16llX %.16f\n", (F64_Bits){ .f = result }.bits, result);
		}
	}

	return 0;
}
