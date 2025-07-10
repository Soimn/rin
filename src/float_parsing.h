// based on the explanation of the Eisel-Lemire algorithm by https://nigeltao.github.io/blog/2020/eisel-lemire.html

typedef enum Parse_Float_Error
{
	ParseFloatError_Success = 0,
	ParseFloatError_MissingLeadingDigits,
	ParseFloatError_MissingDecimalPoint,
	ParseFloatError_MissingTrailingDigits,
	ParseFloatError_MissingDigitsAfterExponent,
	ParseFloatError_TooManyDigitsInMantissa,
	ParseFloatError_TooManyDigitsInExponent,
} Parse_Float_Error;

static Parse_Float_Error
ParseFloat(u8* start, u32* skip, f64* value)
{
	u8* cursor = start;

	if (!Char_IsDigit(*cursor))
	{
		//// ERROR
		return ParseFloatError_MissingLeadingDigits;
	}

	smm digit_count         = 0;
	smm digits_before_point = -1;

	u64 mantissa = 0;

	for (;;)
	{
		if (Char_IsDigit(*cursor))
		{
			u64 hi;
			u8 carry = _addcarry_u64(0, _umul128(mantissa, 10, &hi), *cursor&0xF, &mantissa);

			if (hi || carry)
			{
				//// ERROR
				return ParseFloatError_TooManyDigitsInMantissa;
			}

			++digit_count;
			++cursor;
		}
		else if (*cursor == '_')
		{
			++cursor;
		}
		else if (*cursor == '.' && digits_before_point == -1)
		{
			digits_before_point = digit_count;
			++cursor;
		}
		else break;
	}

	if (digits_before_point == -1)
	{
		//// ERROR
		return ParseFloatError_MissingDecimalPoint;
	}

	smm digits_after_point = digit_count - digits_before_point;

	if (digits_after_point == 0)
	{
		//// ERROR
		return ParseFloatError_MissingTrailingDigits;
	}

	smm exponent = 0;
	bool exponent_is_negative = false;
	if ((*cursor&0xDF) == 'E')
	{
		++cursor;
		exponent_is_negative = (*cursor == '-');
		if (*cursor == '-' || *cursor == '+') ++cursor;

		umm exponent_digit_count = 0;

		for (;;)
		{
			if (Char_IsDigit(*cursor))
			{
				exponent = exponent*10 + (*cursor&0xF);

				if (exponent > 9999) // NOTE: just an early check to avoid hassle that should never be a problem
				{
					//// ERROR
					return ParseFloatError_TooManyDigitsInExponent;
				}

				++exponent_digit_count;
				++cursor;
			}
			else if (*cursor == '_')
			{
				++cursor;
			}
			else break;
		}

		if (exponent_digit_count == 0)
		{
			//// ERROR
			return ParseFloatError_MissingDigitsAfterExponent;
		}
	}

	exponent = (exponent_is_negative ? -exponent : exponent) - digits_after_point;

	f64 float_value = 0;

	if (mantissa == 0)
	{
		float_value = 0;
	}
	else if (mantissa < (1ULL << 53) && exponent >= -22 && exponent <= 22)
	{
		static f64 small_powers_of_10[23] = {
			1.0e000, 1.0e+01, 1.0e+02, 1.0e+03, 1.0e+04,
			1.0e+05, 1.0e+06, 1.0e+07, 1.0e+08, 1.0e+09,
			1.0e+10, 1.0e+11, 1.0e+12, 1.0e+13, 1.0e+14,
			1.0e+15, 1.0e+16, 1.0e+17, 1.0e+18, 1.0e+19,
			1.0e+20, 1.0e+21, 1.0e+22
		};
		
		if (exponent < 0) float_value = (f64)mantissa/small_powers_of_10[-exponent];
		else              float_value = (f64)mantissa*small_powers_of_10[exponent];
	}
	else
	{
		// TODO: This is supposed to either be Eisel-Lemire or a fallback, but I just gave up
		umm abs_exponent = (umm)(exponent < 0 ? -exponent : exponent);

		char buffer[sizeof("xxxxxxxxxxxxxxxxxxxxe+xxxx")] = {
			'0' + (mantissa/10000000000000000000ULL) % 10,
			'0' + (mantissa/1000000000000000000ULL) % 10,
			'0' + (mantissa/100000000000000000ULL) % 10,
			'0' + (mantissa/10000000000000000ULL) % 10,
			'0' + (mantissa/1000000000000000ULL) % 10,
			'0' + (mantissa/100000000000000ULL) % 10,
			'0' + (mantissa/10000000000000ULL) % 10,
			'0' + (mantissa/1000000000000ULL) % 10,
			'0' + (mantissa/100000000000ULL) % 10,
			'0' + (mantissa/10000000000ULL) % 10,
			'0' + (mantissa/1000000000ULL) % 10,
			'0' + (mantissa/100000000ULL) % 10,
			'0' + (mantissa/10000000ULL) % 10,
			'0' + (mantissa/1000000ULL) % 10,
			'0' + (mantissa/100000ULL) % 10,
			'0' + (mantissa/10000ULL) % 10,
			'0' + (mantissa/1000ULL) % 10,
			'0' + (mantissa/100ULL) % 10,
			'0' + (mantissa/10ULL) % 10,
			'0' + (mantissa/1ULL) % 10,
			'e',
			(exponent < 0 ? '-' : '+'),
			'0' + (abs_exponent/1000) % 10,
			'0' + (abs_exponent/100) % 10,
			'0' + (abs_exponent/10) % 10,
			'0' + (abs_exponent/1) % 10,
			0,
		};

		float_value = strtod(buffer, 0);
	}

	*skip = (u32)(cursor - start);
	*value = float_value;
	return ParseFloatError_Success;
}
