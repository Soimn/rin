#define LEXER_ZPAD 65

typedef struct Lexer
{
	u8* contents;
	u8* cursor;
	Virtual_Array* strings;
} Lexer;

static Token Lexer__ParseHexInt(Lexer* lexer, u32 offset);
static Token Lexer__ParseInt(Lexer* lexer, u32 offset);
static Token Lexer__ParseString(Lexer* lexer, u32 offset);

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

	['!'] = 8 | 1,
	['%'] = 8 | 1,
	['*'] = 8 | 1,
	['/'] = 8 | 1,
	['~'] = 8 | 1,

	['='] = 8 | 1 | 2,
	['&'] = 8 | 1 | 2,
	['|'] = 8 | 1 | 2,
	['+'] = 8 | 1 | 2,
	['-'] = 8 | 1 | 2,

	['<'] = 8 | 1 | 2 | 4,
	['>'] = 8 | 1 | 2 | 4,
};

static Token
Lexer__ParseHexInt(Lexer* lexer, u32 offset)
{
	Token token = { .offset = offset };

	ASSERT(lexer->cursor[0] == '0' && (lexer->cursor[1] == 'x' || lexer->cursor[1] == 'h'));

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

	return token;
}

static Token
Lexer__ParseInt(Lexer* lexer, u32 offset)
{
	ASSERT(Char_IsDigit(*lexer->cursor));

	Token token = { .offset = offset };

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
			token.kind    = Token_Int;
			token.integer = value;
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
				//// ERROR: Missing digits
				NOT_IMPLEMENTED;
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
				//// ERROR: Missing digits
				NOT_IMPLEMENTED;
			}
		}

		token.kind = Token_Float;
		token.floating = strtod(buffer, 0); // TODO: replace
	}

	return token;
}

static Token
Lexer__ParseString(Lexer* lexer, u32 offset)
{
	ASSERT(*lexer->cursor == '"' || *lexer->cursor == '\'');

	Token token = { .offset = offset };

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
		//// ERROR: Unterminated string literal
		NOT_IMPLEMENTED;
	}
	else
	{
		++lexer->cursor; // NOTE: Skip terminator

		ASSERT(end - start < ~(u32)0);

		u8* raw = start;
		u32 cap = (u32)(end - start);

		String string = {
			.data = malloc(cap),
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
							//// ERROR: Out of UTF-8 range
							NOT_IMPLEMENTED;
						}
					} break;

					default:
					{
						//// ERROR: Illegal escape sequence
						NOT_IMPLEMENTED;
					} break;
				}
			}
		}

		if (string.len > ~(u16)0)
		{
			//// ERROR: String is too long
			NOT_IMPLEMENTED;
		}

		token.kind = (terminator == '"' ? Token_String : Token_Char);
		token.len  = (u16)string.len;
		token.data = string.data;
	}

	return token;
}

static bool
LexFile(String input, Virtual_Array* tokens, Virtual_Array* strings, Token** first_token, u32* token_count)
{
	ASSERT(input.len > LEXER_ZPAD && IsZeroed(input.data + input.len-LEXER_ZPAD, LEXER_ZPAD));

	Lexer lexer = {
		.contents = input.data,
		.cursor   = input.data,
		.strings  = strings,
	};

	VA_EnsureCommitted(tokens, input.len/8);

	*first_token = VA_EndPointer(tokens);

	__m256i hex_df       = _mm256_set1_epi8(0xDF);
	__m256i alpha_bias   = _mm256_set1_epi8(0x7F - 'Z');
	__m256i alpha_thresh = _mm256_set1_epi8(0x7E - ('Z' - 'A'));
	__m256i digit_bias   = _mm256_set1_epi8(0x7F - '9');
	__m256i digit_thresh = _mm256_set1_epi8(0x7E - ('9' - '0'));
	__m256i underscore   = _mm256_set1_epi8('_');

	for (;;)
	{
		Token* token = VA_Push(tokens);

		for (;;)
		{
			while ((u8)(*lexer.cursor-1) < (u8)0x20) ++lexer.cursor;

			if (lexer.cursor[0] == '/' && lexer.cursor[1] == '*')
			{
				lexer.cursor += 2;

				unsigned long skip;
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)lexer.cursor);

					u32 slash_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('/')));
					u32 star_mask  = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('*')));

					u32 mask = (star_mask << 1) & slash_mask;

					if (_BitScanForward(&skip, mask)) break;
					{
						if (_mm256_testz_si256(c, c))
						{
							//// ERROR: Unterminated block comment
							NOT_IMPLEMENTED;
						}
						else
						{
							lexer.cursor += 31; // there might be a * at the end, so only skip 31
							continue;
						}
					}
				}

				lexer.cursor += skip + 1;
			}
			else if (lexer.cursor[0] == '/' && lexer.cursor[1] == '/')
			{
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)lexer.cursor);
					if (_mm256_testz_si256(c, c)) break;

					u32 newline_mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('\n')));

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
		token->offset = offset;

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
				token->kind = Token_Ident;
				token->len  = (u16)ident_skip;
				token->data = lexer.cursor;
				lexer.cursor += ident_skip;
			}
			else
			{
				u8* start = lexer.cursor;

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
					//// ERROR: Identifier is too long
					NOT_IMPLEMENTED;
				}

				token->kind = Token_Ident;
				token->len  = (u16)len;
				token->data = start;
			}
		}
		else
		{
			u8 c = lexer.cursor[0];
			u8 c1_c  = (lexer.cursor[1] == c);
			u8 c1_eq = (lexer.cursor[1] == '=');

			token->kind     = c;
			lexer.cursor += 1;

			s8 pattern = Lexer__PatternTable[c];
			if (pattern >= 0)
			{
				if (pattern != 0)
				{
					u8 ext = c1_eq + c1_c + c1_c;
					u8 resolved_pattern = pattern & ext;

					token->kind    |= (resolved_pattern << 7);
					lexer.cursor += (resolved_pattern != 0);

					if ((pattern&4) && c1_c && *lexer.cursor == '=')
					{
						token->kind    |= 0x80;
						lexer.cursor += 1;
					}
				}
				else // ", ', 0-9, \, ` or >= 0x7F
				{
					lexer.cursor -= 1;

					if (lexer.cursor[0] == '0' && (lexer.cursor[1] == 'x' || lexer.cursor[1] == 'h'))
					{
						*token = Lexer__ParseHexInt(&lexer, offset);
					}
					else if (Char_IsDigit(lexer.cursor[0]))
					{
						*token = Lexer__ParseInt(&lexer, offset);
					}
					else if (lexer.cursor[0] == '"' || lexer.cursor[0] == '\'')
					{
						*token = Lexer__ParseString(&lexer, offset);
					}
					else if (lexer.cursor[0] == 0)
					{
						token->kind = Token_EOF;
					}
					else
					{
						//// ERROR: Unknown symbol
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
