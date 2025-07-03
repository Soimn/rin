#define LEXER_ZPAD 65

typedef struct Lexer
{
	u8* cursor;
	u8* contents;
} Lexer;

static Token
Lexer__Error(Lexer* lexer, const char* msg, u8* at, u32 line)
{
	(void)lexer, (void)msg, (void)at, (void)line;

	// TODO
	*(volatile int*)0 = 0;

	return (Token){
		.kind = Token_Error,
	};
}

static bool
Lexer__ParseHexInt(Lexer* lexer, Token* token, u32 line)
{
	ASSERT(lexer->cursor[0] == '0' && (lexer->cursor[1] == 'x' || lexer->cursor[1] == 'h'));

	u8* start = lexer->cursor;

	bool is_hex_float = (lexer->cursor[1] == 'h');

	lexer->cursor += 2;

	umm digit_count = 0;

	u64 value = 0;
	for (;;)
	{
		if (Char_IsDigit(*lexer->cursor))
		{
			value <<= 4;
			value  |= *lexer->cursor & 0xF;
			++digit_count;
			++lexer->cursor;
		}
		else if (Char_IsHexAlpha(*lexer->cursor))
		{
			value <<= 4;
			value  |= 9 + (*lexer->cursor & 0x1F);
			++digit_count;
			++lexer->cursor;
		}
		else if (*lexer->cursor == '_')
		{
			++lexer->cursor;
		}
		else break;
	}

	if (is_hex_float)
	{
		if (digit_count == 16)
		{
			token->kind     = Token_Float;
			token->floating = (F64_Bits){ .bits = value }.f;
		}
		else if (digit_count == 8)
		{
			token->kind     = Token_Float;
			token->floating = (F32_Bits){ .bits = (u32)value }.f;
		}
		else
		{
			//// ERROR
			Lexer__Error(lexer, "Hexadecimal floating point literals can only have 8 or 16 digits (corresponding to IEE 754 32-bit and 64-bit respectively)", start, line);
			return false;
		}
	}
	else
	{
		if (digit_count <= 16)
		{
			token->kind    = Token_Int;
			token->integer = value;
		}
		else
		{
			//// ERROR
			Lexer__Error(lexer, "Hexadecimal integer literal is too large to fit in 64 bits", start, line);
			return false;
		}
	}

	return true;
}

static bool
Lexer__ParseInt(Lexer* lexer, Token* token, u32 line)
{
	ASSERT(Char_IsDigit(*lexer->cursor));

	umm digit_count = 0;
	u64 value = 0;

	u8* start = lexer->cursor;

	for (;;)
	{
		if (Char_IsDigit(*lexer->cursor))
		{
			value = value*10 + (*lexer->cursor&0xF);
			++digit_count;
			++lexer->cursor;
		}
		else if (*lexer->cursor == '_')
		{
			++lexer->cursor;
		}
		else break;
	}

	if (*lexer->cursor != '.' && (*lexer->cursor&0xDF) != 'E')
	{
		if (digit_count < 19)
		{
			token->kind    = Token_Int;
			token->integer = value;
		}
		else
		{
			value = 0;

			for (u8* scan = start; scan != lexer->cursor; ++scan)
			{
				if (*scan == '_') continue;

				u64 hi_product;
				u64 value_x10 = _umul128(value, 10, &hi_product);
				u8 carry = _addcarry_u64(0, value_x10, *scan&0xF, &value);

				if (hi_product != 0 || carry != 0)
				{
					//// ERROR
					Lexer__Error(lexer, "Integer literal is too large to fit in 64 bits", start, line);
					return false;
				}
			}

			token->kind    = Token_Int;
			token->integer = value;
		}
	}
	else
	{
		char buffer[128] = {0};
		umm buf_cur = 0;

		for (u8* scan = start; scan < lexer->cursor; ++scan)
		{
			if (*scan != '_')
			{
				ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
				buffer[buf_cur++] = *scan;
			}
		}

		if (*lexer->cursor == '.')
		{
			ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
			buffer[buf_cur++] = '.';
			++lexer->cursor;

			umm fraction_digit_count = 0;
			for (;;)
			{
				if (Char_IsDigit(*lexer->cursor))
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = *lexer->cursor;
					++fraction_digit_count;
					++lexer->cursor;
				}
				else if (*lexer->cursor == '_')
				{
					++lexer->cursor;
				}
				else break;
			}

			if (fraction_digit_count == 0)
			{
				//// ERROR
				Lexer__Error(lexer, "Missing digits after decimal point in floating point literal", lexer->cursor, line);
				return false;
			}
		}

		if ((*lexer->cursor&0xDF) == 'E')
		{
			ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
			buffer[buf_cur++] = *lexer->cursor;
			++lexer->cursor;

			if (*lexer->cursor == '+' || *lexer->cursor == '-')
			{
				ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
				buffer[buf_cur++] = *lexer->cursor;
				++lexer->cursor;
			}

			umm exp_digit_count = 0;
			for (;;)
			{
				if (Char_IsDigit(*lexer->cursor))
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = *lexer->cursor;
					++exp_digit_count;
					++lexer->cursor;
				}
				else if (*lexer->cursor == '_')
				{
					++lexer->cursor;
				}
				else break;
			}

			if (exp_digit_count == 0)
			{
				//// ERROR
				Lexer__Error(lexer, "Missing digits of exponent in floating point literal", lexer->cursor, line);
				return false;
			}
		}

		token->kind     = Token_Float;
		token->floating = strtod(buffer, 0); // TODO: replace
	}

	return true;
}

static bool
Lexer__ParseString(Lexer* lexer, Token* token, Virtual_Array* string_array, u32 line)
{
	ASSERT(*lexer->cursor == '"' || *lexer->cursor == '\'');

	u8 terminator = *lexer->cursor;

	++lexer->cursor;
	u8* start = lexer->cursor;
	for (;;)
	{
		if (*lexer->cursor == 0 || *lexer->cursor == terminator) break;
		else if (*lexer->cursor == '\\') lexer->cursor += 2;
		else                             lexer->cursor += 1;
	}
	u8* end = lexer->cursor;

	if (*lexer->cursor == 0)
	{
		//// ERROR
		Lexer__Error(lexer, "Unterminated string literal", start, line);
		return false;
	}
	else
	{
		++lexer->cursor; // NOTE: Skip terminator

		ASSERT(end - start < ~(u32)0);

		u8* raw = start;
		u32 cap = (u32)(end - start);

		String string = {
			.data = VA_PushN(string_array, cap),
			.len  = 0,
		};

		ASSERT(string.data != 0);

		for (umm i = 0; i < cap; )
		{
			if (raw[i] > 0x7F)
			{
				//// ERROR
				Lexer__Error(lexer, "Illegal character in string literal", start + i, line);
				return false;
			}
			else if (raw[i] != '\\')
			{
				string.data[string.len++] = raw[i++];
			}
			else
			{
				u8* esc_start = raw;

				u8 esc = raw[i+1];

				ASSERT(i+1 < cap);
				i += 2;

				switch (esc)
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
					{
						u8 val = 0;

						// NOTE: The terminating " is used as a sentinel to avoid a bounds check
						for (umm j = 0; j < 2; ++j, ++i)
						{
							if (Char_IsDigit(raw[i]))
							{
								val <<= 4;
								val  |= raw[i] & 0xF;
							}
							else if (Char_IsHexAlpha(raw[i]))
							{
								val <<= 4;
								val  |= 9 + (raw[i] & 0x1F);
							}
							else
							{
								//// ERROR
								Lexer__Error(lexer, "Missing digits after \\x in byte escape sequence", esc_start, line); 
								return false;
							}
						}

						string.data[string.len++] = val;
					} break;

					case 'U':
					{
						u32 val = 0;

						// NOTE: The terminating " is used as a sentinel to avoid a bounds check
						for (umm j = 0; j < 6; ++j, ++i)
						{
							if (Char_IsDigit(raw[i]))
							{
								val <<= 4;
								val  |= raw[i] & 0xF;
							}
							else if (Char_IsHexAlpha(raw[i]))
							{
								val <<= 4;
								val  |= 9 + (raw[i] & 0x1F);
							}
							else
							{
								//// ERROR
								Lexer__Error(lexer, "Missing digits after \\U in unicode escape sequence", esc_start, line);
								return false;
							}
						}

						if (val <= 0x7F)
						{
							string.data[string.len++] = (u8)val;
						}
						else if (val <= 0x7FF)
						{
							string.data[string.len++] = 0xC0 | (u8)(val >> 6);
							string.data[string.len++] = 0x80 | (u8)(val & 0x3F);
						}
						else if (val <= 0xFFFF)
						{
							string.data[string.len++] = 0xE0 | (u8)( val >> 12);
							string.data[string.len++] = 0x80 | (u8)((val >>  6) & 0x3F);
							string.data[string.len++] = 0x80 | (u8)( val        & 0x3F);
						}
						else if (val <= 0x10FFFF)
						{
							string.data[string.len++] = 0xF0 | (u8)( val >> 18);
							string.data[string.len++] = 0x80 | (u8)((val >> 12) & 0x3F);
							string.data[string.len++] = 0x80 | (u8)((val >>  6) & 0x3F);
							string.data[string.len++] = 0x80 | (u8)( val        & 0x3F);
						}
						else
						{
							//// ERROR
							Lexer__Error(lexer, "Unicode codepoint is outside of UTF-8 range", esc_start, line);
							return false;
						}
					} break;

					default:
					{
						//// ERROR
						Lexer__Error(lexer, "Illegal escape sequence", esc_start, line);
						return false;
					} break;
				}
			}
		}

		if (string.len > ~(u16)0)
		{
			//// ERROR
			Lexer__Error(lexer, "String is too long", start, line);
			return false;
		}

		token->kind = (terminator == '"' ? Token_String : Token_Char);
		token->len  = (u16)string.len;
		token->data = string.data;
	}

	return true;
}

static bool
LexFile(String input, Virtual_Array* tokens, Virtual_Array* string_array, Token** first_token, u32* token_count)
{
	ASSERT(input.len > LEXER_ZPAD && IsZeroed(input.data + input.len-LEXER_ZPAD, LEXER_ZPAD));

	VA_EnsureCommitted(tokens, input.len/8);

	*first_token = VA_EndPointer(tokens);

	Lexer lexer = {
		.cursor   = input.data,
		.contents = input.data,
	};

	__m256i hex_df       = _mm256_set1_epi8(0xDF);
	__m256i alpha_bias   = _mm256_set1_epi8(0x7F - 'Z');
	__m256i alpha_thresh = _mm256_set1_epi8(0x7E - ('Z' - 'A'));
	__m256i digit_bias   = _mm256_set1_epi8(0x7F - '9');
	__m256i digit_thresh = _mm256_set1_epi8(0x7E - ('9' - '0'));
	__m256i underscore   = _mm256_set1_epi8('_');
	__m256i slash        = _mm256_set1_epi8('/');
	__m256i star         = _mm256_set1_epi8('*');
	__m256i newline      = _mm256_set1_epi8('\n');

	u32 line = 1;

	for (;;)
	{
		Token* token = VA_Push(tokens);

		for (;;)
		{
			while ((u8)(*lexer.cursor-1) < (u8)0x20)
			{
				line += (*lexer.cursor == '\n');
				++lexer.cursor;
			}

			if (lexer.cursor[0] == '/' && lexer.cursor[1] == '*')
			{
				u8* start      = lexer.cursor;
				u32 start_line = line;

				lexer.cursor += 2;

				unsigned long skip;
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)lexer.cursor);

					u32 slash_mask   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, slash));
					u32 star_mask    = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, star));
					u32 newline_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, newline));

					u32 mask = (star_mask << 1) & slash_mask;

					if (_BitScanForward(&skip, mask))
					{
						line += _mm_popcnt_u32(newline_mask & ((1 << skip) - 1));
						lexer.cursor += skip + 1;
						break;
					}
					else
					{
						if (_mm256_testz_si256(c, c))
						{
							//// ERROR
							Lexer__Error(&lexer, "Unterminated block comment", start, start_line);
							return false;
						}
						else
						{
							line += _mm_popcnt_u32(newline_mask & 0x7FFFFFFF);

							lexer.cursor += 31; // there might be a * at the end, so only skip 31
							continue;
						}
					}
				}
			}
			else if (lexer.cursor[0] == '/' && lexer.cursor[1] == '/')
			{
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)lexer.cursor);
					if (_mm256_testz_si256(c, c)) break;

					u32 newline_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, newline));

					unsigned long skip;
					if (_BitScanForward(&skip, newline_mask))
					{
						lexer.cursor += skip;
						break;
					}
					else
					{
						lexer.cursor += 32;
						continue;
					}
				}
			}
			else break;
		}

		u32 offset = (u32)(lexer.cursor - lexer.contents);

		if (Char_IsAlpha(*lexer.cursor) || *lexer.cursor == '_')
		{
			__m256i c256 = _mm256_loadu_si256((__m256i*)lexer.cursor);
			__m256i alpha = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c256, hex_df), alpha_bias), alpha_thresh);
			__m256i digit = _mm256_cmpgt_epi8(_mm256_add_epi8(c256, digit_bias), digit_thresh);
			__m256i under = _mm256_cmpeq_epi8(c256, underscore);

			u32 ident_mask = _mm256_movemask_epi8(_mm256_or_si256(alpha, _mm256_or_si256(digit, under)));
			
			unsigned long ident_skip = 0;
			if (_BitScanForward(&ident_skip, ident_mask+1))
			{
				*token = (Token){
					.kind   = Token_Ident,
					.len    = (u16)ident_skip,
					.offset = offset,
					.data   = lexer.cursor,
				};

				lexer.cursor += ident_skip;
			}
			else
			{
				u8* start      = lexer.cursor;
				u32 start_line = line;

				for (;;)
				{
					c256 = _mm256_loadu_si256((__m256i*)lexer.cursor);
					alpha = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c256, hex_df), alpha_bias), alpha_thresh);
					digit = _mm256_cmpgt_epi8(_mm256_add_epi8(c256, digit_bias), digit_thresh);
					under = _mm256_cmpeq_epi8(c256, underscore);

					ident_mask = _mm256_movemask_epi8(_mm256_or_si256(alpha, _mm256_or_si256(digit, under)));

					if (ident_mask == 0) break;
					else if (_BitScanForward(&ident_skip, ident_mask+1))
					{
						lexer.cursor += ident_skip;
						break;
					}
					else
					{
						lexer.cursor += 32;
						continue;
					}
				}

				u64 len = lexer.cursor - start;
				if (len > ~(u16)0)
				{
					//// ERROR
					Lexer__Error(&lexer, "Identifier is too long", start, start_line);
					return false;
				}
				else
				{
					*token = (Token){
						.kind   = Token_Ident,
						.len    = len,
						.offset = offset,
						.data   = start,
					};
				}
			}
		}
		else
		{
			static s8 Lexer__PatternTable[256] = {
				['#'] = -1,
				['$'] = -1,
				['('] = -1,
				[')'] = -1,
				[','] = -1,
				['.'] = -1,
				[':'] = -1,
				[';'] = -1,
				['?'] = -1,
				['@'] = -1,
				['['] = -1,
				[']'] = -1,
				['^'] = -1,
				['{'] = -1,
				['}'] = -1,

				['!'] = 1,
				['%'] = 1,
				['*'] = 1,
				['/'] = 1,
				['~'] = 1,

				['='] = 1 | 2,
				['&'] = 1 | 2,
				['|'] = 1 | 2,
				['+'] = 1 | 2,
				['-'] = 1 | 2,

				['<'] = 1 | 2 | 4,
				['>'] = 1 | 2 | 4,
			};

			u8 c = lexer.cursor[0];
			s8 pattern = Lexer__PatternTable[c];

			u8 c1_c  = (lexer.cursor[1] == c);
			u8 c1_eq = (lexer.cursor[1] == '=');

			*token = (Token){
				.kind   = c,
				.offset = offset,
			};

			lexer.cursor += 1;

			if (pattern >= 0)
			{
				if (pattern > 0)
				{
					u8 ext = c1_eq + c1_c + c1_c;
					u8 resolved_pattern = pattern & ext;

					token->kind  |= (resolved_pattern << 7);
					lexer.cursor += (resolved_pattern != 0);

					if ((pattern&4) && c1_c && *lexer.cursor == '=')
					{
						token->kind  |= 0x80;
						lexer.cursor += 1;
					}
				}
				else // ", ', 0-9, \, ` or >= 0x7F
				{
					lexer.cursor -= 1;

					if (lexer.cursor[0] == '0' && (lexer.cursor[1] == 'x' || lexer.cursor[1] == 'h'))
					{
						if (!Lexer__ParseHexInt(&lexer, token, line))
						{
							//// ERROR
							return false;
						}
					}
					else if (Char_IsDigit(lexer.cursor[0]))
					{
						if (!Lexer__ParseInt(&lexer, token, line))
						{
							//// ERROR
							return false;
						}
					}
					else if (lexer.cursor[0] == '"' || lexer.cursor[0] == '\'')
					{
						if (!Lexer__ParseString(&lexer, token, string_array, line))
						{
							//// ERROR
							return false;
						}
					}
					else if (lexer.cursor[0] == 0)
					{
						token->kind = Token_EOF;
						break;
					}
					else
					{
						//// ERROR
						Lexer__Error(&lexer, "Unknown symbol", lexer.cursor, line);
						return false;
					}
				}
			}
		}
	}

	*token_count = (u32)((Token*)VA_EndPointer(tokens) - *first_token);

	return true;
}
