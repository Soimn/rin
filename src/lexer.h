#define LEXER_ZPAD 65

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

static Token
Lexer__ParseHexInt(u8** cursor, u32 offset)
{
	Token token = { .offset = offset };

	// NOTE: this is to make MSVC happy so that it actually keeps the cursor in a register
	u8* cur = *cursor;

	ASSERT(cur[0] == '0' && (cur[1] == 'x' || cur[1] == 'h'));

	bool is_hex_float = (cur[1] == 'h');

	cur += 2;

	umm digit_count = 0;

	u64 value = 0;
	for (;;)
	{
		if (Char_IsDigit(*cur))
		{
			value <<= 4;
			value  |= *cur & 0xF;
			++digit_count;
			++cur;
		}
		else if (Char_IsHexAlpha(*cur))
		{
			value <<= 4;
			value  |= 9 + (*cur & 0x1F);
			++digit_count;
			++cur;
		}
		else if (*cur == '_')
		{
			++cur;
		}
		else break;
	}

	if (is_hex_float)
	{
		if (digit_count == 16)
		{
			token.kind     = Token_Float;
			token.floating = (F64_Bits){ .bits = value }.f;
		}
		else if (digit_count == 8)
		{
			token.kind     = Token_Float;
			token.floating = (F32_Bits){ .bits = (u32)value }.f;
		}
		else
		{
			//// ERROR: Hex floats can only have 8 or 16 digits (corresponding to IEE 754 32-bit and 64-bit respectively)
			NOT_IMPLEMENTED;
		}
	}
	else
	{
		if (digit_count <= 16)
		{
			token.kind    = Token_Int;
			token.integer = value;
		}
		else
		{
			//// ERROR: Hex int is too large
			NOT_IMPLEMENTED;
		}
	}

	*cursor = cur;
	return token;
}

static Token
Lexer__ParseInt(u8** cursor, u32 offset)
{
	// NOTE: this is to make MSVC happy so that it actually keeps the cursor in a register
	u8* cur = *cursor;

	ASSERT(Char_IsDigit(*cur));

	Token token = { .offset = offset };

	umm digit_count = 0;
	u64 value = 0;

	u8* start = cur;

	for (;;)
	{
		if (Char_IsDigit(*cur))
		{
			value = value*10 + (*cur&0xF);
			++digit_count;
			++cur;
		}
		else if (*cur == '_')
		{
			++cur;
		}
		else break;
	}

	if (*cur != '.' && (*cur&0xDF) != 'E')
	{
		if (digit_count < 19)
		{
			token.kind    = Token_Int;
			token.integer = value;
		}
		else
		{
			value = 0;

			for (u8* scan = start; scan != cur; ++scan)
			{
				if (*scan == '_') continue;

				u64 hi_product;
				u64 value_x10 = _umul128(value, 10, &hi_product);
				u8 carry = _addcarry_u64(0, value_x10, *scan&0xF, &value);

				if (hi_product != 0 || carry != 0)
				{
					//// ERROR: Overflow
					NOT_IMPLEMENTED;
				}
			}

			token.kind    = Token_Int;
			token.integer = value;
		}
	}
	else
	{
		char buffer[128] = {0};
		umm buf_cur = 0;

		for (u8* scan = start; scan < cur; ++scan)
		{
			if (*scan != '_')
			{
				ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
				buffer[buf_cur++] = *scan;
			}
		}

		if (*cur == '.')
		{
			ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
			buffer[buf_cur++] = '.';
			++cur;

			umm fraction_digit_count = 0;
			for (;;)
			{
				if (Char_IsDigit(*cur))
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = *cur;
					++fraction_digit_count;
					++cur;
				}
				else if (*cur == '_')
				{
					++cur;
				}
				else break;
			}

			if (fraction_digit_count == 0)
			{
				//// ERROR: Missing digits
				NOT_IMPLEMENTED;
			}
		}

		if ((*cur&0xDF) == 'E')
		{
			ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
			buffer[buf_cur++] = *cur;
			++cur;

			if (*cur == '+' || *cur == '-')
			{
				ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
				buffer[buf_cur++] = *cur;
				++cur;
			}

			umm exp_digit_count = 0;
			for (;;)
			{
				if (Char_IsDigit(*cur))
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = *cur;
					++exp_digit_count;
					++cur;
				}
				else if (*cur == '_')
				{
					++cur;
				}
				else break;
			}

			if (exp_digit_count == 0)
			{
				//// ERROR: Missing digits
				NOT_IMPLEMENTED;
			}
		}

		token.kind = Token_Float;
		token.floating = strtod(buffer, 0); // TODO: replace
	}

	*cursor = cur;
	return token;
}

static Token
Lexer__ParseString(u8** cursor, u32 offset, Virtual_Array* string_array)
{
	// NOTE: this is to make MSVC happy so that it actually keeps the cursor in a register
	u8* cur = *cursor;

	ASSERT(*cur == '"' || *cur == '\'');

	Token token = { .offset = offset };

	u8 terminator = *cur;

	++cur;
	u8* start = cur;
	for (;;)
	{
		if (*cur == 0 || *cur == terminator) break;
		else if (*cur == '\\') cur += 2;
		else                   cur += 1;
	}
	u8* end = cur;

	if (*cur == 0)
	{
		//// ERROR: Unterminated string literal
		NOT_IMPLEMENTED;
	}
	else
	{
		++cur; // NOTE: Skip terminator

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
				//// ERROR: Illegal shit in string literal
				NOT_IMPLEMENTED;
			}
			else if (raw[i] != '\\')
			{
				string.data[string.len++] = raw[i++];
			}
			else
			{
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
								//// ERROR: Missing digits in \x
								NOT_IMPLEMENTED;
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
								//// ERROR: Missing digits in \u
								NOT_IMPLEMENTED;
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
							Lexer__Error("Unicode codepoint is outside of UTF-8 range", , line);
							NOT_IMPLEMENTED;
						}
					} break;

					default:
					{
						//// ERROR
						Lexer__Error("Illegal escape sequence", , line);
						NOT_IMPLEMENTED;
					} break;
				}
			}
		}

		if (string.len > ~(u16)0)
		{
			//// ERROR
			Lexer__Error("String is too long", start, line);
			NOT_IMPLEMENTED;
		}

		token.kind = (terminator == '"' ? Token_String : Token_Char);
		token.len  = (u16)string.len;
		token.data = string.data;
	}

	*cursor = cur;
	return token;
}

static bool
LexFile(String input, Virtual_Array* tokens, Virtual_Array* string_array, Token** first_token, u32* token_count)
{
	ASSERT(input.len > LEXER_ZPAD && IsZeroed(input.data + input.len-LEXER_ZPAD, LEXER_ZPAD));

	VA_EnsureCommitted(tokens, input.len/8);

	*first_token = VA_EndPointer(tokens);

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

	for (u8* cursor = input.data;;)
	{
		Token* token = VA_Push(tokens);

		for (;;)
		{
			while ((u8)(*cursor-1) < (u8)0x20)
			{
				line += (*cursor == '\n');
				++cursor;
			}

			if (cursor[0] == '/' && cursor[1] == '*')
			{
				u8* start      = cursor;
				u32 start_line = line;

				cursor += 2;

				unsigned long skip;
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);

					u32 slash_mask   = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, slash));
					u32 star_mask    = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, star));
					u32 newline_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, newline));

					u32 mask = (star_mask << 1) & slash_mask;

					if (_BitScanForward(&skip, mask))
					{
						line += _mm_popcnt_u32(newline_mask & ((1 << skip) - 1));
						cursor += skip + 1;
						break;
					}
					else
					{
						if (_mm256_testz_si256(c, c))
						{
							//// ERROR
							*token = Lexer__Error("Unterminated block comment", start, start_line);
							NOT_IMPLEMENTED; // TODO: How to bail?
						}
						else
						{
							line += _mm_popcnt_u32(newline_mask & 0x7FFFFFFF);

							cursor += 31; // there might be a * at the end, so only skip 31
							continue;
						}
					}
				}
			}
			else if (cursor[0] == '/' && cursor[1] == '/')
			{
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);
					if (_mm256_testz_si256(c, c)) break;

					u32 newline_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, newline));

					unsigned long skip;
					if (_BitScanForward(&skip, newline_mask))
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

		u32 offset = (u32)(cursor - input.data);
		token->offset = offset;

		if (Char_IsAlpha(*cursor) || *cursor == '_')
		{
			__m256i c256 = _mm256_loadu_si256((__m256i*)cursor);
			__m256i alpha = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c256, hex_df), alpha_bias), alpha_thresh);
			__m256i digit = _mm256_cmpgt_epi8(_mm256_add_epi8(c256, digit_bias), digit_thresh);
			__m256i under = _mm256_cmpeq_epi8(c256, underscore);

			u32 ident_mask = _mm256_movemask_epi8(_mm256_or_si256(alpha, _mm256_or_si256(digit, under)));
			
			unsigned long ident_skip = 0;
			if (_BitScanForward(&ident_skip, ident_mask+1))
			{
				token->kind = Token_Ident;
				token->len  = (u16)ident_skip;
				token->data = cursor;
				cursor += ident_skip;
			}
			else
			{
				u8* start      = cursor;
				u32 start_line = line;

				for (;;)
				{
					c256 = _mm256_loadu_si256((__m256i*)cursor);
					alpha = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c256, hex_df), alpha_bias), alpha_thresh);
					digit = _mm256_cmpgt_epi8(_mm256_add_epi8(c256, digit_bias), digit_thresh);
					under = _mm256_cmpeq_epi8(c256, underscore);

					ident_mask = _mm256_movemask_epi8(_mm256_or_si256(alpha, _mm256_or_si256(digit, under)));

					if (ident_mask == 0) break;
					else if (_BitScanForward(&ident_skip, ident_mask+1))
					{
						cursor += ident_skip;
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
					//// ERROR
					*token = Lexer__Error("Identifier is too long", start, start_line);
					NOT_IMPLEMENTED; // TODO: How to bail?
				}
				else
				{
					token->kind = Token_Ident;
					token->len  = (u16)len;
					token->data = start;
				}
			}
		}
		else
		{
			u8 c = cursor[0];
			s8 pattern = Lexer__PatternTable[c];

			u8 c1_c  = (cursor[1] == c);
			u8 c1_eq = (cursor[1] == '=');

			token->kind = c;
			cursor     += 1;

			if (pattern >= 0)
			{
				if (pattern != 0)
				{
					u8 ext = c1_eq + c1_c + c1_c;
					u8 resolved_pattern = pattern & ext;

					token->kind |= (resolved_pattern << 7);
					cursor      += (resolved_pattern != 0);

					if ((pattern&4) && c1_c && *cursor == '=')
					{
						token->kind |= 0x80;
						cursor      += 1;
					}
				}
				else // ", ', 0-9, \, ` or >= 0x7F
				{
					cursor -= 1;

					if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'h'))
					{
						*token = Lexer__ParseHexInt(&cursor, offset, line);
					}
					else if (Char_IsDigit(cursor[0]))
					{
						*token = Lexer__ParseInt(&cursor, offset, line);
					}
					else if (cursor[0] == '"' || cursor[0] == '\'')
					{
						*token = Lexer__ParseString(&cursor, offset, string_array, line);
					}
					else if (cursor[0] == 0)
					{
						token->kind = Token_EOF;
					}
					else
					{
						//// ERROR
						*token = Lexer__Error("Unknown symbol", cursor, line);
						NOT_IMPLEMENTED;
					}
				}
			}
		}

		if (token->kind == Token_EOF) break;
		else                          continue;
	}

	*token_count = (u32)((Token*)VA_EndPointer(tokens) - *first_token);

	return true; // TODO: Error handling
}
