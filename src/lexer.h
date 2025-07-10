#define LEXER_ZPAD 65

#define DB 0x100
#define EQ 0x80
static s16 Lexer_LUT[512] = {
	['#']      = -1,
	['#' | DB] = -1,
	['#' | EQ] = -1,
	['$']      = -1,
	['$' | DB] = -1,
	['$' | EQ] = -1,
	['(']      = -1,
	['(' | DB] = -1,
	['(' | EQ] = -1,
	[')']      = -1,
	[')' | DB] = -1,
	[')' | EQ] = -1,
	[',']      = -1,
	[',' | DB] = -1,
	[',' | EQ] = -1,
	['.']      = -1,
	['.' | DB] = -1,
	['.' | EQ] = -1,
	[':']      = -1,
	[':' | DB] = -1,
	[':' | EQ] = -1,
	[';']      = -1,
	[';' | DB] = -1,
	[';' | EQ] = -1,
	['?']      = -1,
	['?' | DB] = -1,
	['?' | EQ] = -1,
	['@']      = -1,
	['@' | DB] = -1,
	['@' | EQ] = -1,
	['[']      = -1,
	['[' | DB] = -1,
	['[' | EQ] = -1,
	[']']      = -1,
	[']' | DB] = -1,
	[']' | EQ] = -1,
	['^']      = -1,
	['^' | DB] = -1,
	['^' | EQ] = -1,
	['{']      = -1,
	['{' | DB] = -1,
	['{' | EQ] = -1,
	['}']      = -1,
	['}' | DB] = -1,
	['}' | EQ] = -1,

	['%']           = (Token_Percent    << 2) | 1,
	['%' | DB]      = (Token_Percent    << 2) | 1,
	['%' | EQ]      = (Token_PercentEq  << 2) | 2,
	['*']           = (Token_Star       << 2) | 1,
	['*' | DB]      = (Token_Star       << 2) | 1,
	['*' | EQ]      = (Token_StarEq     << 2) | 2,
	['/']           = (Token_Slash      << 2) | 1,
	['/' | DB]      = (Token_Slash      << 2) | 1,
	['/' | EQ]      = (Token_SlashEq    << 2) | 2,
	['~']           = (Token_Tilde      << 2) | 1,
	['~' | DB]      = (Token_Tilde      << 2) | 1,
	['~' | EQ]      = (Token_TildeEq    << 2) | 2,
	['&']           = (Token_And        << 2) | 1,
	['&' | DB]      = (Token_AndAnd     << 2) | 2,
	['&' | EQ]      = (Token_AndEq      << 2) | 2,
	['|']           = (Token_Or         << 2) | 1,
	['|' | DB]      = (Token_OrOr       << 2) | 2,
	['|' | EQ]      = (Token_OrEq       << 2) | 2,
	['<']           = (Token_Lt         << 2) | 1,
	['<' | DB]      = (Token_LtLt       << 2) | 2,
	['<' | EQ]      = (Token_LtEq       << 2) | 2,
	['>']           = (Token_Gt         << 2) | 1,
	['>' | DB]      = (Token_GtGt       << 2) | 2,
	['>' | EQ]      = (Token_GtEq       << 2) | 2,
	['-']           = (Token_Minus      << 2) | 1,
	['-' | DB]      = (Token_MinusMinus << 2) | 2,
	['-' | EQ]      = (Token_MinusEq    << 2) | 2,
	['+']           = (Token_Plus       << 2) | 1,
	['+' | DB]      = (Token_PlusPlus   << 2) | 2,
	['+' | EQ]      = (Token_PlusEq     << 2) | 2,
	['=']           = (Token_Eq         << 2) | 1,
	['=' | EQ]      = (Token_EqEq       << 2) | 2,
	['=' | DB]      = (Token_EqEq       << 2) | 2,
	['=' | DB | EQ] = (Token_EqEq       << 2) | 2,
	['!']           = (Token_Bang       << 2) | 1,
	['!' | DB]      = (Token_Bang       << 2) | 1,
	['!' | EQ]      = (Token_BangEq     << 2) | 2,
};
#undef DB
#undef EQ

static __declspec(align(32)) u8 Lexer__Keywords[][32] = {
#define FIRST_KEYWORD(T, C0, C1, S) [T - Token__FirstKeyword] = S,
#define REMAINING_KEYWORDS(T, C0, C1, S) [T - Token__FirstKeyword] = S,
	KEYWORD_LIST
#undef FIRST_KEYWORD
#undef REMAINING_KEYWORDS
};

static u8 Lexer__KeywordLut[32][32] = {
#define FIRST_KEYWORD(T, C0, C1, S) [C0 - 'a'][C1 - 'a'] = T - Token__FirstKeyword,
#define REMAINING_KEYWORDS(T, C0, C1, S) [C0 - 'a'][C1 - 'a'] = T - Token__FirstKeyword,
	KEYWORD_LIST
#undef FIRST_KEYWORD
#undef REMAINING_KEYWORDS
};

static bool
LexFile(String input, Virtual_Array* token_array, Virtual_Array* string_array, Token** first_token, u32* token_count)
{
	ASSERT(input.len > LEXER_ZPAD && IsZero(input.data + input.len-LEXER_ZPAD, LEXER_ZPAD));

	// NOTE: Linux like source code produces a token per 8.8 bytes on average, expect a similar amount
	VA_EnsureCommitted(token_array, input.len/8);

	// NOTE: Linux like source code contains ~2% strings relative to the total amount of bytes 1/32 is ~3%
	VA_EnsureCommitted(string_array, input.len/32);

	Token* token_start = VA_EndPointer(token_array);

	u8* cursor = input.data;

	__m256i hex_df       = _mm256_set1_epi8(0xDF);
	__m256i alpha_bias   = _mm256_set1_epi8(0x7F - 'Z');
	__m256i alpha_thresh = _mm256_set1_epi8(0x7E - ('Z' - 'A'));
	__m256i digit_bias   = _mm256_set1_epi8(0x7F - '9');
	__m256i digit_thresh = _mm256_set1_epi8(0x7E - ('9' - '0'));
	__m256i underscore   = _mm256_set1_epi8('_');
	__m256i slash        = _mm256_set1_epi8('/');
	__m256i star         = _mm256_set1_epi8('*');
	__m256i newline      = _mm256_set1_epi8('\n');

	for (;;)
	{
		for (;;)
		{
			while ((u8)(*cursor-1) < (u8)0x20) ++cursor;

			if (cursor[0] == '/' && cursor[1] == '*')
			{
				cursor += 2;

				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);
					if (_mm256_testz_si256(c, c))
					{
						//// ERROR: Unterminated block comment
						__debugbreak();
						return false;
					}

					u32 slash_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, slash));
					u32 star_mask  = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, star));

					u32 mask = (star_mask << 1) & slash_mask;

					unsigned long skip;
					_BitScanForward(&skip, mask);

					if (mask == 0)
					{
						cursor += 31; // NOTE: 31 in case of a * at the end of the register
						continue;
					}
					else
					{
						cursor += skip + 1;
						break;
					}
				}
			}
			else if (cursor[0] == '/' && cursor[1] == '/')
			{
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);
					if (_mm256_testz_si256(c, c)) break;

					unsigned long skip;
					if (_BitScanForward(&skip, _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, newline))))
					{
						cursor += skip;
						break;
					}
					else
					{
						cursor += 32;
						continue;
					}
				}
			}
			else break;
		}

		u8 ch  = cursor[0];
		u16 db = (cursor[1] == ch  ? 0x100 : 0);
		u16 eq = (cursor[1] == '=' ?  0x80 : 0);

		s16 lookup = Lexer_LUT[ch | db | eq];

		Token* token = VA_Push(token_array);
		token->offset = (u32)(cursor - input.data);

		if (lookup < 0)
		{
			token->kind = ch;
			cursor     += 1;
		}
		else if (lookup != 0)
		{
			token->kind = (u32)lookup >> 2;

			cursor += lookup & 0x3;

			if ((token->kind == Token_GtGt || token->kind == Token_LtLt) && cursor[0] == '=')
			{
				if (token->kind == Token_GtGt) token->kind = Token_GtGtEq;
				else                           token->kind = Token_LtLtEq;
				++cursor;
			}

			if (ch > 0x7F)
			{
				//// ERROR: Unknown symbol
				__debugbreak();
				return false;
			}
		}
		else
		{
			if (Char_IsAlpha(*cursor) || *cursor == '_')
			{
				__m256i c = _mm256_loadu_si256((__m256i*)cursor);

				u32 alpha_mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c, hex_df), alpha_bias), alpha_thresh));
				u32 digit_mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(c, digit_bias), digit_thresh));
				u32 under_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, underscore));

				u32 mask = alpha_mask | digit_mask | under_mask;

				unsigned long skip;
				if (_BitScanForward(&skip, mask+1))
				{
					cursor += skip;

					umm keyword_idx = Lexer__KeywordLut[((u8*)&c)[0] - 'a'][((u8*)&c)[1] - 'a'];

					// NOTE: based on this answer https://stackoverflow.com/a/77802924
					__m256i index = _mm256_setr_epi8( 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
							                             16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
					__m256i cmp_mask = _mm256_cmpgt_epi8(_mm256_set1_epi8((u8)skip), index);

					c = _mm256_and_si256(c, cmp_mask);

					int match_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_load_si256((__m256i*)Lexer__Keywords[keyword_idx])));

					token->kind = (match_mask == -1 ? (u16)(Token__FirstKeyword + keyword_idx) : Token_Ident);
					token->len  = (u16)skip;
				}
				else
				{
					u8* start = cursor;
					cursor += 32;

					for (;;)
					{
						c = _mm256_loadu_si256((__m256i*)cursor);

						alpha_mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c, hex_df), alpha_bias), alpha_thresh));
						digit_mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(_mm256_add_epi8(c, digit_bias), digit_thresh));
						under_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, underscore));

						mask = alpha_mask | digit_mask | under_mask;

						if (mask == 0) break;
						else if (_BitScanForward(&skip, mask+1))
						{
							cursor += skip;
							break;
						}
						else
						{
							cursor += 32;
							continue;
						}
					}

					u64 len = cursor - start;

					if (len > ~(u16)0)
					{
						//// ERROR: Ident too long
						__debugbreak();
						return false;
					}

					token->kind = Token_Ident;
					token->len  = (u16)len;
				}
			}
			else if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'h'))
			{
				bool is_hex_float = (cursor[1] == 'h');

				u8* start = cursor+2;

				umm underscore_count = 0;
				u64 value = 0;

				++cursor; // NOTE: Skip the 0 in 0x/0h
				for (;;)
				{
					++cursor;
					if (Char_IsDigit(*cursor))
					{
						value <<= 4;
						value  |= *cursor & 0xF;
					}
					else if (Char_IsHexAlpha(*cursor))
					{
						value <<= 4;
						value  |= 9 + (*cursor & 0x1F);
					}
					else if (*cursor == '_')
					{
						++underscore_count;
						continue;
					}
					else break;
				}

				umm digit_count = (cursor - start) - underscore_count;

				if (is_hex_float)
				{
					token->kind = Token_Float;
					token->len  = (u16)digit_count;

					Token_Data* token_data = VA_Push(token_array);

					if      (digit_count == 16) *token_data = (Token_Data){ .floating = (F64_Bits){ .bits =      value }.f };
					else if (digit_count ==  8) *token_data = (Token_Data){ .floating = (F32_Bits){ .bits = (u32)value }.f };
					else
					{
						//// ERROR: Hex floats can only have 8 or 16 digits
						__debugbreak();
						return false;
					}
				}
				else
				{
					if (digit_count == 0)
					{
						//// ERROR: Missing digits in hex int
						__debugbreak();
						return false;
					}
					else if (digit_count <= 16)
					{
						token->kind = Token_Int;
						token->len  = (u16)digit_count;

						Token_Data* token_data = VA_Push(token_array);
						*token_data = (Token_Data){
							.integer = value,
						};
					}
					else
					{
						u64 value_lo = 0;
						u64 value_hi = 0;

						for (u8* scan = start; scan != cursor; ++scan)
						{
							if (*scan == '_') continue;

							if (value_hi & (0xFULL << 60))
							{
								//// ERROR: Integer literal is too large to fit in 128 bits
								__debugbreak();
								return false;
							}

							value_hi <<= 4;
							value_hi  |= value_lo >> 60;
							value_lo <<= 4;

							if (Char_IsDigit(*cursor)) value_lo |= *scan & 0xF;
							else                       value_lo |= 9 + (*scan & 0x1F);
						}

						token->kind = Token_Int128;
						token->len  = (u16)digit_count;

						Token_Data* token_data_lo = VA_Push(token_array);
						*token_data_lo = (Token_Data){
							.integer = value_lo,
						};

						Token_Data* token_data_hi = VA_Push(token_array);
						*token_data_hi = (Token_Data){
							.integer = value_hi,
						};
					}
				}
			}
			else if (Char_IsDigit(*cursor))
			{
				umm digit_count = 0;
				u64 value = 0;

				u8* start = cursor;

				for (;;)
				{
					if (Char_IsDigit(*cursor))
					{
						value = value*10 + (*cursor&0xF);
						++digit_count;
						++cursor;
					}
					else if (*cursor == '_')
					{
						++cursor;
					}
					else break;
				}

				if ((*cursor&0xDF) == 'E')
				{
					//// ERROR: Float literals without a fractional part is illegal
					__debugbreak();
					return false;
				}
				else if (*cursor != '.')
				{
					if (digit_count <= 19)
					{
						token->kind = Token_Int;
						token->len  = (u16)digit_count;

						Token_Data* token_data = VA_Push(token_array);
						*token_data = (Token_Data){
							.integer = value,
						};
					}
					else
					{
						u64 value_lo = 0;
						u64 value_hi = 0;

						for (u8* scan = start; scan != cursor; ++scan)
						{
							if (*scan != '_')
							{
								u64 value_lo_x10_hi;
								u64 value_lo_x10_lo = _umul128(value_lo, 10, &value_lo_x10_hi);
								u64 value_hi_x10_hi;
								u64 value_hi_x10_lo = _umul128(value_hi, 10, &value_hi_x10_hi);

								u8 c1 = _addcarry_u64(0, value_lo_x10_lo, *scan&0xF, &value_lo);
								u8 c2 = _addcarry_u64(c1, value_hi_x10_lo, value_lo_x10_hi, &value_hi);

								if (c2 != 0 || value_hi_x10_hi != 0)
								{
									//// ERROR: Overflow
									__debugbreak();
									return false;
								}
							}
						}

						if (value_hi == 0)
						{
							token->kind = Token_Int;
							token->len  = (u16)digit_count;

							Token_Data* token_data = VA_Push(token_array);
							*token_data = (Token_Data){
								.integer = value_lo,
							};
						}
						else
						{
							token->kind = Token_Int128;
							token->len  = (u16)digit_count;

							Token_Data* token_data_lo = VA_Push(token_array);
							*token_data_lo = (Token_Data){
								.integer = value_lo,
							};

							Token_Data* token_data_hi = VA_Push(token_array);
							*token_data_hi = (Token_Data){
								.integer = value_hi,
							};
						}
					}
				}
				else
				{
					ASSERT(*cursor == '.');

					cursor = start;

					u32 skip        = 0;
					f64 float_value = 0;
					Parse_Float_Error error = ParseFloat(cursor, &skip, &float_value);

					if (error == ParseFloatError_Success)
					{
						token->kind = Token_Float;
						
						Token_Data* token_data = VA_Push(token_array);
						*token_data = (Token_Data){
							.floating = float_value,
						};

						cursor += skip;
					}
					else
					{	
						if (error == ParseFloatError_MissingTrailingDigits)
						{
							//// ERROR: Got decimal point but no digits followed
						}
						else if (error == ParseFloatError_MissingDigitsAfterExponent)
						{
							//// ERROR: Got floating point exponent marker, but no digits followed
						}
						else if (error == ParseFloatError_TooManyDigitsInMantissa)
						{
							//// ERROR: Too many digits in floating point literal
						}
						else if (error == ParseFloatError_TooManyDigitsInExponent)
						{
							//// ERROR: Too many digits in floating point literal exponent
						}
						else
						{
							//// ERROR: Failed to parse float
						}

						__debugbreak();
						return false;
					}
				}
			}
			else if (*cursor == '"' || *cursor == '\'')
			{
				u8 terminator = *cursor;

				u8* start = cursor;
				++cursor;

				while (*cursor != 0 && *cursor != terminator)
				{
					// NOTE: Skipping a null is fine since the file is padded with LEXER_ZPAD nulls
					cursor += (*cursor == '\\' ? 2 : 1);
				}

				if (*cursor == 0)
				{
					//// ERROR: Unterminated string literal
					__debugbreak();
					return false;
				}
				else
				{
					String raw = {
						.data = start+1, // NOTE: start + 1 to skip the leading "
						.len  = (u16)(cursor - (start+1)),
					};

					++cursor; // NOTE: Skip ending "

					String string = {
						.data = VA_PushN(string_array, raw.len),
						.len  = 0,
					};

					for (umm i = 0; i < raw.len;)
					{
						if (raw.data[i] > 0x7F)
						{
							//// ERROR: Non ASCII characters in string literal
							__debugbreak();
							return false;
						}
						else if (raw.data[i] != '\\')
						{
							string.data[string.len++] = raw.data[i++];
						}
						else
						{
							u8 c = raw.data[i+1]; // NOTE: there is always a next character since the string is properly terminated
							i += 2;

							switch (c)
							{
								case 'a':  string.data[string.len++] = '\a'; break;
								case 'b':  string.data[string.len++] = '\b'; break;
								case 'e':  string.data[string.len++] = 0x1B; break;
								case 'f':  string.data[string.len++] = '\f'; break;
								case 'n':  string.data[string.len++] = '\n'; break;
								case 'r':  string.data[string.len++] = '\r'; break;
								case 't':  string.data[string.len++] = '\t'; break;
								case 'v':  string.data[string.len++] = '\v'; break;
								case '\\': string.data[string.len++] = '\\'; break;
								case '\'': string.data[string.len++] = '\''; break;
								case '\"': string.data[string.len++] = '\"'; break;

								case 'x':
								case 'U':
								{
									umm required_digit_count = (c == 'U' ? 6 : 2);

									u64 value = 0;

									for (umm j = 0; j < required_digit_count; ++j, ++i)
									{
										// NOTE: going 1 past the end of "raw" is fine since this will only read the terminating "
										if (Char_IsDigit(raw.data[i]))
										{
											value <<= 4;
											value  |= raw.data[i]&0xF;
										}
										else if (Char_IsHexAlpha(raw.data[i]))
										{
											value <<= 4;
											value  |= 9 + (raw.data[i]&0x1F);
										}
										else
										{
											//// ERROR: Missing digits
											__debugbreak();
											return false;
										}
									}

									if (c == 'x' || value <= 0x7F)
									{
										string.data[string.len++] = (u8)value;
									}
									else if (value <= 0x7FF)
									{
										string.data[string.len++] = 0xC0 | (u8)(value >> 6);
										string.data[string.len++] = 0x80 | (u8)(value & 0x3F);
									}
									else if (value <= 0xFFFF)
									{
										string.data[string.len++] = 0xE0 | (u8)(value >> 12);
										string.data[string.len++] = 0x80 | (u8)((value >> 6) & 0x3F);
										string.data[string.len++] = 0x80 | (u8)(value & 0x3F);
									}
									else if (value <= 0x10FFFF)
									{
										string.data[string.len++] = 0xF0 | (u8)(value >> 18);
										string.data[string.len++] = 0x80 | (u8)((value >> 12) & 0x3F);
										string.data[string.len++] = 0x80 | (u8)((value >>  6) & 0x3F);
										string.data[string.len++] = 0x80 | (u8)(value & 0x3F);
									}
									else
									{
										//// ERROR: Codepoint is out of UTF-8 range
										__debugbreak();
										return false;
									}
								} break;

								default:
								{
									//// ERROR: Illegal escape sequence
									__debugbreak();
									return false;
								} break;
							}
						}
					}

					if (terminator == '\'')
					{
						if (string.len != 1)
						{
							//// ERROR: Character literal must encode exactly one byte
							__debugbreak();
							return false;
						}
						else
						{
							token->kind = Token_Char;

							// NOTE: This wastes 8 bytes but is not a problem since character literals are almost never used
							Token_Data* token_data = VA_Push(token_array);
							*token_data = (Token_Data){
								.integer = string.data[0],
							};
						}
					}
					else
					{
						if (string.len > ~(u16)0)
						{
							//// ERROR: String literal is too long
							__debugbreak();
							return false;
						}
						else
						{
							token->kind = Token_String;
							token->len  = (u16)string.len;

							Token_Data* token_data = VA_Push(token_array);
							*token_data = (Token_Data){
								.string = string.data,
							};
						}
					}
				}
			}
			else if (*cursor == 0)
			{
				token->kind = Token_EOF;
				break;
			}
			else
			{
				//// ERROR: Illegal symbol
				__debugbreak();
				return false;
			}
		}
	}

	Token* token_end = VA_EndPointer(token_array);

	*first_token = token_start;
	*token_count = (u32)(token_end - token_start);

	return true;
}
