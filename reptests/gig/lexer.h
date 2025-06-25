typedef struct Lexer
{
	Virtual_Array* tokens;
	Virtual_Array* strings;
	u8* contents;
	u8* cursor;
	u32 line;
} Lexer;

__forceinline static Token Lexer__ParseNumber(Lexer* lexer);
__forceinline static String Lexer__ParseString(Lexer* lexer, u8 terminator);

__forceinline
static Token
Lexer__NextToken(Lexer* lexer)
{
	Token token = { .kind = Token_Invalid };

	/// Skip all whitespace and comments
	{
		u8* cursor = lexer->cursor;
		u32 line   = lexer->line;
		for (;;)
		{
			while ((u8)(*cursor-1) < (u8)0x20)
			{
				line += (*cursor == '\n');
				++cursor;
			}

			if (cursor[0] == '/' && cursor[1] == '/')
			{
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);

					int mask = _mm256_movemask_epi8(_mm256_or_si256(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('\n')), _mm256_cmpeq_epi8(c, _mm256_setzero_si256())));

					if (mask == 0)
					{
						cursor += 32;
						continue;
					}
					else
					{
						unsigned long skip;
						_BitScanForward(&skip, mask);

						cursor += skip;

						break;
					}
				}
			}
			else if (cursor[0] == '/' && cursor[1] == '*')
			{
				cursor += 2;
				for (;;)
				{
					__m256i c = _mm256_loadu_si256((__m256i*)cursor);
					if (_mm256_testz_si256(c, c)) break;

					u32 slash = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('/')));
					u32 star  = _mm256_movemask_epi8(_mm256_cmpeq_epi8(c, _mm256_set1_epi8('*')));

					u32 mask = ((star << 1) & slash);

					unsigned long skip;
					if (_BitScanForward(&skip, mask))
					{
						cursor += skip;
						break;
					}
					else
					{
						cursor += 31;
						continue;
					}
				}

				if (*cursor != 0) ++cursor;
				else
				{
					//// ERROR: Unterminated block comment
					NOT_IMPLEMENTED;
				}
			}
			else break;
		}

		lexer->cursor = cursor;
		lexer->line   = line;
	}

	u32 offset = (u32)(lexer->cursor - lexer->contents);
	u8* start = lexer->cursor;

	token.offset = offset;

	if (Char_IsAlpha(*lexer->cursor) || *lexer->cursor == '_')
	{
		u8* cursor = lexer->cursor;

		__m256i lo_mask      = _mm256_set1_epi8(0xDF);
		__m256i alpha_bias   = _mm256_set1_epi8(0x7F - 'Z');
		__m256i alpha_thresh = _mm256_set1_epi8(0x7E - ('Z' - 'A'));
		__m256i num_bias     = _mm256_set1_epi8(0x7F - '9');
		__m256i num_thresh   = _mm256_set1_epi8(0x7E - ('9' - '0'));
		__m256i underscore   = _mm256_set1_epi8('_');

		__m256i c = _mm256_loadu_si256((__m256i*)cursor);

		__m256i alpha_test      = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c, lo_mask), alpha_bias), alpha_thresh);
		__m256i num_test        = _mm256_cmpgt_epi8(_mm256_add_epi8(c, num_bias), num_thresh);
		__m256i underscore_test = _mm256_cmpeq_epi8(c, underscore);

		int mask = _mm256_movemask_epi8(alpha_test) | _mm256_movemask_epi8(_mm256_or_si256(num_test, underscore_test));

		unsigned long skip;
		_BitScanForward(&skip, mask+1);

		if (mask != -1) cursor += skip;
		else
		{
			for (;;)
			{
				c = _mm256_loadu_si256((__m256i*)cursor);

				alpha_test      = _mm256_cmpgt_epi8(_mm256_add_epi8(_mm256_and_si256(c, lo_mask), alpha_bias), alpha_thresh);
				num_test        = _mm256_cmpgt_epi8(_mm256_add_epi8(c, num_bias), num_thresh);
				underscore_test = _mm256_cmpeq_epi8(c, underscore);

				mask = _mm256_movemask_epi8(alpha_test) | _mm256_movemask_epi8(_mm256_or_si256(num_test, underscore_test));

				if (mask == 0) break;
				else if (mask == -1)
				{
					cursor += 32;
					continue;
				}
				else
				{
					_BitScanForward(&skip, mask+1);

					cursor += skip;
					break;
				}
			}
		}

		lexer->cursor = cursor;

		u32 len = (u32)(lexer->cursor - start);

		if (len > 0xFFFF)
		{
			//// ERROR: Identifier is too long
			NOT_IMPLEMENTED;
		}

		return (Token){
			.kind   = Token_Ident,
			.offset = offset,
			.len    = (u16)len,
			.data   = start,
		};
	}
	else
	{
		// NOTE: There is at least 32 0's at the end of the file => going a bit past the end is not a problem

		u8 c = *lexer->cursor;
		++lexer->cursor;

		u8 c1_eq      = (*lexer->cursor == '=');
		u16 c1_eq_bit = (*lexer->cursor == '=' ? TOKEN_KIND__BINARY_ASS_BIT : 0);

		switch (c)
		{
			case 0: token.kind = Token_EOF; break;

			case '@':
			case '#':
			case '$':
			case '?':
			case ':':
			case ',':
			case ';':
			case '^':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			{
				token.kind = c;
			} break;

			case '*': token.kind = Token_Star    | c1_eq_bit; lexer->cursor += c1_eq; break;
			case '/': token.kind = Token_Slash   | c1_eq_bit; lexer->cursor += c1_eq; break;
			case '%': token.kind = Token_Percent | c1_eq_bit; lexer->cursor += c1_eq; break;
			case '+': token.kind = Token_Plus    | c1_eq_bit; lexer->cursor += c1_eq; break;
			case '~': token.kind = Token_Tilde   | c1_eq_bit; lexer->cursor += c1_eq; break;

			case '=': token.kind = (*lexer->cursor == '=' ? Token_EqEq   : Token_Eq);   lexer->cursor += c1_eq; break;
			case '!': token.kind = (*lexer->cursor == '=' ? Token_BangEq : Token_Bang); lexer->cursor += c1_eq; break;

			case '|':
			{
				token.kind = Token_Or;
				if (*lexer->cursor == '|')
				{
					token.kind = Token_OrOr;
					++lexer->cursor;
				}

				if (*lexer->cursor == '=')
				{
					token.kind |= TOKEN_KIND__BINARY_ASS_BIT;
					++lexer->cursor;
				}
			} break;

			case '&':
			{
				token.kind = Token_And;
				if (*lexer->cursor == '&')
				{
					token.kind = Token_AndAnd;
					++lexer->cursor;
				}

				if (*lexer->cursor == '=')
				{
					token.kind |= TOKEN_KIND__BINARY_ASS_BIT;
					++lexer->cursor;
				}
			} break;

			case '<':
			{
				token.kind = Token_Le;
				if (*lexer->cursor == '<')
				{
					token.kind = Token_LeLe;
					++lexer->cursor;
				}

				if (*lexer->cursor == '=')
				{
					token.kind |= TOKEN_KIND__BINARY_ASS_BIT;
					++lexer->cursor;
				}
			} break;

			case '>':
			{
				token.kind = Token_Ge;
				if (*lexer->cursor == '>')
				{
					token.kind = Token_GeGe;
					++lexer->cursor;
				}
				if (*lexer->cursor == '>')
				{
					token.kind = Token_GeGeGe;
					++lexer->cursor;
				}

				if (*lexer->cursor == '=')
				{
					token.kind |= TOKEN_KIND__BINARY_ASS_BIT;
					++lexer->cursor;
				}
			} break;

			case '-':
			{
				token.kind = Token_Minus;

				if (*lexer->cursor == '=')
				{
					token.kind |= TOKEN_KIND__BINARY_ASS_BIT;
					++lexer->cursor;
				}
				else if (*lexer->cursor == '>')
				{
					token.kind = Token_Arrow;
					++lexer->cursor;
				}
				else if (lexer->cursor[0] == '-' && lexer->cursor[1] == '-')
				{
					token.kind = Token_Strike;
					lexer->cursor += 2;
				}
			} break;

			case '.':
			{
				token.kind = Token_Dot;

				if (*lexer->cursor == '(')
				{
					token.kind = Token_DotParen;
					++lexer->cursor;
				}
				else if (*lexer->cursor == '[')
				{
					token.kind = Token_DotBracket;
					++lexer->cursor;
				}
				else if (*lexer->cursor == '{')
				{
					token.kind = Token_DotBrace;
					++lexer->cursor;
				}
				else if (Char_IsDigit(*lexer->cursor))
				{
					//// ERROR: You are a terrible person. I hate you.
					NOT_IMPLEMENTED;
				}
			} break;

			default:
			{
				lexer->cursor -= 1;
				if (Char_IsDigit(*lexer->cursor))
				{
					token = Lexer__ParseNumber(lexer);
					token.offset = offset;
				}
				else if (*lexer->cursor == '"')
				{
					String string = Lexer__ParseString(lexer, '"');

					if (string.len > 0xFFFF)
					{
						//// ERROR: String literal is too long
						NOT_IMPLEMENTED;
					}

					token.kind   = Token_String;
					token.offset = offset;
					token.len    = (u16)string.len;
					token.data   = string.data;
				}
				else if (*lexer->cursor == '\'')
				{
					String string = Lexer__ParseString(lexer, '\'');

					if (string.len > 0xFFFF)
					{
						//// ERROR: Character literal is too long
						NOT_IMPLEMENTED;
					}

					token.kind   = Token_Char;
					token.offset = offset;
					token.len    = (u16)string.len;
					token.data   = string.data;
				}
				else
				{
					//// ERROR
					NOT_IMPLEMENTED;
				}
			} break;
		}
	}

	return token;
}

__forceinline
static Token
Lexer__ParseNumber(Lexer* lexer)
{
	ASSERT(Char_IsDigit(*lexer->cursor));

	Token token = { .kind = Token_Invalid };

	if (lexer->cursor[0] == '0' && (lexer->cursor[1] == 'x' || lexer->cursor[1] == 'h'))
	{
		bool is_hex_float = (lexer->cursor[1] == 'h');
		lexer->cursor += 2;

		u8* start = lexer->cursor;

		u32 underscores = 0;
		u64 value       = 0;
		for (;;)
		{
			if (Char_IsDigit(*lexer->cursor))
			{
				value = (value << 4) | (*lexer->cursor&0xF);
				++lexer->cursor;
			}
			else if ((u8)((*lexer->cursor&0xDF) - 'A') <= ('F' - 'A'))
			{
				value = (value << 4) | (9 + (*lexer->cursor&0x1F));
				++lexer->cursor;
			}
			else if (*lexer->cursor == '_')
			{
				++underscores;
				++lexer->cursor;
			}
			else break;
		}

		u32 digit_count = (u32)(lexer->cursor - start) - underscores;

		if (is_hex_float)
		{
			// TODO: Should hex floats hint type?
			if (digit_count == 8)
			{
				token.kind     = Token_Float;
				token.floating = (F32_Bits){ .bits = (u32)value }.f;
			}
			else if (digit_count == 16)
			{
				token.kind     = Token_Float;
				token.floating = (F64_Bits){ .bits = value }.f;
			}
			else
			{
				//// ERROR: hex floats can only be 8 or 16 digits long (corresponding to IEE 754 32 and 64 bit floats respectively)
				NOT_IMPLEMENTED;
			}
		}
		else
		{
			if (digit_count > 16)
			{
#if 0
				//// ERROR: hex literal is too large to fit in 64 bits
				NOT_IMPLEMENTED;
#else
				// TODO REMOVE
				token.kind = Token_Int;
#endif
			}
			else
			{
				token.kind    = Token_Int;
				token.integer = value;
			}
		}
	}
	else
	{
		u64 value            = 0;
		u64 underscore_count = 0;

		u8* start = lexer->cursor;
		for (;;)
		{
			if (Char_IsDigit(*lexer->cursor))
			{
				value = value*10 + (*lexer->cursor&0xF);
				++lexer->cursor;
			}
			else if (*lexer->cursor == '_')
			{
				++underscore_count;
				++lexer->cursor;
			}
			else break;
		}

		u64 digit_count = (lexer->cursor - start) - underscore_count;

		if (*lexer->cursor == '.' && (Char_IsDigit(lexer->cursor[1]) || (*lexer->cursor&0xDF) == 'E'))
		{
			NOT_IMPLEMENTED;
		}
		else if (digit_count >= 19) // NOTE: floor(log10(2^64-1)) = 19
		{
			U128 value128 = {0};
			bool overflow = false;

			for (u8* scan = start; scan < lexer->cursor; ++scan)
			{
				if (Char_IsDigit(*lexer->cursor))
				{
					u8 add_carry = 0;
					bool mul_overflow = false;
					value128 = U128_Add(U128_Mul10(value128, &mul_overflow), (U128){ .lo = *lexer->cursor&0xF }, &add_carry);
					overflow = (mul_overflow || add_carry);

					++lexer->cursor;
				}
				else if (*lexer->cursor == '_')
				{
					++lexer->cursor;
				}
				else break;
			}

			if (overflow || value128.hi != 0)
			{
				//// ERROR: does not fit in 64 bits
				NOT_IMPLEMENTED;
			}
			else
			{
				token.kind    = Token_Int;
				token.integer = value128.lo;
			}
		}
		else
		{
			token.kind    = Token_Int;
			token.integer = value;
		}
	}

	return token;
}

__forceinline
static String
Lexer__ParseString(Lexer* lexer, u8 terminator)
{
	ASSERT(*lexer->cursor == terminator);

	++lexer->cursor;
	u8* start = lexer->cursor;

	for (;;)
	{
		while (*lexer->cursor != terminator && *lexer->cursor != '\\' && *lexer->cursor != 0) ++lexer->cursor;

		if (*lexer->cursor == terminator)
		{
			++lexer->cursor;
			break;
		}
		else if (*lexer->cursor == '\\')
		{
			if (lexer->cursor[1] == 0)
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}

			// NOTE: skip just the first character of an escape sequence (\x, \u, and \U doesn't fuck this up)
			lexer->cursor += 2;
		}
		else
		{
			//// ERROR
			NOT_IMPLEMENTED;
		}
	}

	String raw = {
		.data = start,
		.len  = (u32)(lexer->cursor - start),
	};

	String real = {
		.data = VA_PushN(lexer->strings, raw.len),
		.len  = 0,
	};

	umm i = 0;
	while (i < raw.len)
	{
		if (raw.data[i] == '\n')
		{
			//// ERROR: Strings cannot span multiple lines
			NOT_IMPLEMENTED;
		}
		else if (raw.data[i] != '\\') real.data[real.len++] = raw.data[i++];
		else if (raw.len < i+1)
		{
			//// ERROR: Missing escape sequence after backslash
			NOT_IMPLEMENTED;
		}
		else
		{
			++i;
			u8 c = raw.data[i];
			++i;

			switch (c)
			{
				case 'a': real.data[real.len++] = '\a'; break;
				case 'b': real.data[real.len++] = '\b'; break;
				case 'e': real.data[real.len++] = 0x1B; break;
				case 'f': real.data[real.len++] = '\f'; break;
				case 'n': real.data[real.len++] = '\n'; break;
				case 'r': real.data[real.len++] = '\r'; break;
				case 't': real.data[real.len++] = '\t'; break;
				case 'v': real.data[real.len++] = '\v'; break;

				case '\\': real.data[real.len++] = '\\'; break;
				case '\"': real.data[real.len++] = '\"'; break;
				case '\'': real.data[real.len++] = '\''; break;

				default:
				{
					if (Char_IsDigit(c))
					{
						if (raw.len < i + 2 || !((u8)(raw.data[i] - '0') < (u8)9) || !((u8)(raw.data[i+1] - '0') < (u8)9))
						{
							//// ERROR: Missing digits
							NOT_IMPLEMENTED;
						}
						else
						{
							umm value = ((umm)(c - '0') << 6) | ((umm)(raw.data[i] - '0') << 3) | ((umm)(raw.data[i+1] - '0'));
							i += 2;

							if (value > 255)
							{
								//// ERROR: Octal escape sequence is out of range
								NOT_IMPLEMENTED;
							}
							else
							{
								real.data[real.len++] = (u8)value;
							}
						}
					}
					else if ((c&0xDF) == 'U' || c == 'x')
					{
						umm required_digit_count = (c == 'x' ? 2 : (c == 'U' ? 8 : 4));

						if (raw.len < i+required_digit_count)
						{
							//// ERROR: Missing digits
							NOT_IMPLEMENTED;
						}
						else
						{
							u32 value = 0;

							for (umm end_i = i + required_digit_count; i < end_i; ++i)
							{
								if (Char_IsDigit(raw.data[i]))
								{
									value = (value << 4) | (raw.data[i]&0xF);
								}
								else if ((u8)((raw.data[i]&0xDF) - 'A') <= (u8)('F' - 'A'))
								{
									value = (value << 4) | (9 + (raw.data[i]&0x1F));
								}
								else
								{
									//// ERROR: Invalid digit
									NOT_IMPLEMENTED;
								}
							}

							if (value <= 0x7F || c == 'x')
							{
								real.data[real.len++] = (u8)value;
							}
							else if (value <= 0x7FF)
							{
								real.data[real.len++] = 0xC0 | (u8)((value >> 6) & 0x1F);
								real.data[real.len++] = 0x80 | (u8)((value >> 0) & 0x3F);
							}
							else if (value <= 0xFFFF)
							{
								real.data[real.len++] = 0xE0 | (u8)((value >> 12) & 0x0F);
								real.data[real.len++] = 0x80 | (u8)((value >>  6) & 0x3F);
								real.data[real.len++] = 0x80 | (u8)((value >>  0) & 0x3F);
							}
							else if (value <= 0x10FFFF)
							{
								real.data[real.len++] = 0xF0 | (u8)((value >> 18) & 0x07);
								real.data[real.len++] = 0x80 | (u8)((value >> 12) & 0x3F);
								real.data[real.len++] = 0x80 | (u8)((value >>  6) & 0x3F);
								real.data[real.len++] = 0x80 | (u8)((value >>  0) & 0x3F);
							}
							else
							{
								//// ERROR: UTF8 escape sequence is out of range
								NOT_IMPLEMENTED;
							}
						}
					}
					else
					{
						//// ERROR: Illegal escape sequence
						NOT_IMPLEMENTED;
					}
				} break;
			}
		}
	}

	return real;
}

static bool
LexFile(Virtual_Array* tokens, Virtual_Array* strings, u8* contents, Token** first_token, u32* token_count)
{
	bool encountered_errors = false;

	Lexer lexer = {
		.tokens   = tokens,
		.strings  = strings,
		.contents = contents,
		.cursor   = contents,
		.line     = 1,
	};

	*first_token = VA_TopPointer(lexer.tokens);

	for (;;)
	{
		Token* token = VA_Push(lexer.tokens);
		*token = Lexer__NextToken(&lexer);

		if (token->kind == Token_EOF) break;
		else if (token->kind == Token_Invalid)
		{
			//// ERROR
			NOT_IMPLEMENTED;
			encountered_errors = true;
		}
	}

	*token_count = (u32)((Token*)VA_TopPointer(lexer.tokens) - *first_token);

	return !encountered_errors;
}
