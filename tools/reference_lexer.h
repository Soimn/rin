typedef struct Ref_Lexer
{
	u8* contents;
	u8* cursor;
} Ref_Lexer;

// NOTE: contents must be padded with at least 32 zeros (excluding the terminating 0)
Ref_Lexer
RefLexer_Init(u8* contents)
{
	return (Ref_Lexer){
		.contents = contents,
		.cursor   = contents,
	};
}

Token
RefLexer_NextToken(Ref_Lexer* lexer)
{
	Token token = {0};

	for (;;)
	{
		while ((u8)(*lexer->cursor-1) < (u8)0x20) ++lexer->cursor;

		if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
		{
			lexer->cursor += 2;

			while (*lexer->cursor != 0 && !(lexer->cursor[0] == '*' && lexer->cursor[1] == '/')) ++lexer->cursor;

			if (*lexer->cursor == 0)
			{
				//// ERROR: Unterminated block comment
				NOT_IMPLEMENTED;
			}
			else
			{
				lexer->cursor += 2;
			}
		}
		else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '/')
		{
			while (*lexer->cursor != 0 && *lexer->cursor != '\n') ++lexer->cursor;
		}
		else break;
	}

	token.offset = (u32)(lexer->cursor - lexer->contents);

	if (Char_IsAlpha(*lexer->cursor) || *lexer->cursor == '_')
	{
		u8* start = lexer->cursor;

		while (Char_IsAlpha(*lexer->cursor) || Char_IsDigit(*lexer->cursor) || *lexer->cursor == '_') ++lexer->cursor;

		u64 len = lexer->cursor - start;

		if (len > ~(u16)0)
		{
			//// ERROR: Identifier is too long
			NOT_IMPLEMENTED;
		}

		token.kind = Token_Ident;
		token.len  = (u16)len;
		token.data = start;
	}
	else if (lexer->cursor[0] == '0' && (lexer->cursor[1] == 'x' || lexer->cursor[1] == 'h'))
	{
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
				//// ERROR: Hex float can only be 8 or 16 digits long
				NOT_IMPLEMENTED;
			}
		}
		else
		{
			if (digit_count > 16)
			{
				//// ERROR: Hex literal too large
				NOT_IMPLEMENTED;
			}
			else
			{
				token.kind    = Token_Int;
				token.integer = value;
			}
		}
	}
	else if (Char_IsDigit(*lexer->cursor))
	{
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

		if (*lexer->cursor == '.' || (*lexer->cursor&0xDF) == 'E')
		{
			char buffer[256] = {0};
			umm buf_cur = 0;

			umm len = lexer->cursor - start;
			for (umm i = 0; i < len; ++i)
			{
				if (start[i] != '_')
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = start[i];
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

			token.kind     = Token_Float;
			token.floating = strtod(buffer, 0);
		}
		else
		{
			if (digit_count <= 19)
			{
				token.kind    = Token_Int;
				token.integer = value;
			}
			else
			{
				umm len = lexer->cursor - start;
				value = 0;
				for (umm i = 0; i < len; ++i)
				{
					if (start[i] != '_')
					{
						u64 hi_product;
						u64 value_x10 = _umul128(value, 10, &hi_product);
						u8 carry = _addcarry_u64(0, value_x10, start[i]&0xF, &value);

						if (hi_product || carry)
						{
							//// ERROR: Overflow
							NOT_IMPLEMENTED;
						}
					}
				}

				token.kind    = Token_Int;
				token.integer = value;
			}
		}
	}
	else if (*lexer->cursor == '"' || *lexer->cursor == '\'')
	{
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
	}
	else
	{
		u8 c = *lexer->cursor;
		++lexer->cursor; // NOTE: valid since the file is padded with zeros

		u8 c1_eq = (lexer->cursor[0] == '=');
		u8 c1_c  = (lexer->cursor[0] == c);
		u8 c2_eq = (lexer->cursor[1] == '=');

		switch (c)
		{
			case 0: token.kind = Token_EOF; break;

			case '(':  token.kind = Token_OpenParen;    break;
			case ')':  token.kind = Token_CloseParen;   break;
			case '[':  token.kind = Token_OpenBracket;  break;
			case ']':  token.kind = Token_CloseBracket; break;
			case '{':  token.kind = Token_OpenBrace;    break;
			case '}':  token.kind = Token_CloseBrace;   break;
			case '#':  token.kind = Token_Pound;        break;
			case '$':  token.kind = Token_Cash;         break;
			case ',':  token.kind = Token_Comma;        break;
			case ':':  token.kind = Token_Colon;        break;
			case ';':  token.kind = Token_Semicolon;    break;
			case '?':  token.kind = Token_QMark;        break;
			case '@':  token.kind = Token_At;           break;
			case '^':  token.kind = Token_Hat;          break;
			case '.':  token.kind = Token_Dot;          break;

			#define SINGLE_OR_SINGLE_EQ(SINGLE, SINGLE_EQ)     \
				{                                                \
					token.kind     = (c1_eq ? SINGLE_EQ : SINGLE); \
					lexer->cursor += c1_eq;                        \
				}

			case '/': SINGLE_OR_SINGLE_EQ(Token_Slash,   Token_SlashEq);   break;
			case '!': SINGLE_OR_SINGLE_EQ(Token_Bang,    Token_BangEq);    break;
			case '%': SINGLE_OR_SINGLE_EQ(Token_Percent, Token_PercentEq); break;
			case '*': SINGLE_OR_SINGLE_EQ(Token_Star,    Token_StarEq);    break;
			case '=': SINGLE_OR_SINGLE_EQ(Token_Eq,      Token_EqEq);      break;
			case '~': SINGLE_OR_SINGLE_EQ(Token_Tilde,   Token_TildeEq);   break;
			#undef SINGLE_OR_SINGLE_EQ

			#define SINGLE_OR_SINGLE_EQ_OR_DOUBLE(SINGLE, SINGLE_EQ, DOUBLE) \
				{                                                              \
					if (c1_c)                                                    \
					{                                                            \
						token.kind     = DOUBLE;                                   \
						lexer->cursor += 1;                                        \
					}                                                            \
					else                                                         \
					{                                                            \
						token.kind     = (c1_eq ? SINGLE_EQ : SINGLE);             \
						lexer->cursor += c1_eq;                                    \
					}                                                            \
				}

			case '+': SINGLE_OR_SINGLE_EQ_OR_DOUBLE(Token_Plus,  Token_PlusEq,  Token_PlusPlus);   break;
			case '-': SINGLE_OR_SINGLE_EQ_OR_DOUBLE(Token_Minus, Token_MinusEq, Token_MinusMinus); break;
			case '&': SINGLE_OR_SINGLE_EQ_OR_DOUBLE(Token_And,   Token_AndEq,   Token_AndAnd);     break;
			case '|': SINGLE_OR_SINGLE_EQ_OR_DOUBLE(Token_Or,    Token_OrEq,    Token_OrOr);       break;
			#undef SINGLE_OR_SINGLE_EQ_OR_DOUBLE

			#define SINGLE_OR_SINGLE_EQ_OR_DOUBLE_OR_DOUBLE_EQ(SINGLE, SINGLE_EQ, DOUBLE, DOUBLE_EQ) \
				{                                                                                      \
					if (c1_c)                                                                            \
					{                                                                                    \
						token.kind     = (c2_eq ? DOUBLE_EQ : DOUBLE);                                     \
						lexer->cursor += 1 + c2_eq;                                                        \
					}                                                                                    \
					else                                                                                 \
					{                                                                                    \
						token.kind     = (c1_eq ? SINGLE_EQ : SINGLE);                                     \
						lexer->cursor += c1_eq;                                                            \
					}                                                                                    \
				}

			case '<': SINGLE_OR_SINGLE_EQ_OR_DOUBLE_OR_DOUBLE_EQ(Token_Lt,  Token_LtEq,  Token_LtLt,   Token_LtLtEq);   break;
			case '>': SINGLE_OR_SINGLE_EQ_OR_DOUBLE_OR_DOUBLE_EQ(Token_Gt,  Token_GtEq,  Token_GtGt,   Token_GtGtEq);   break;
			#undef SINGLE_OR_SINGLE_EQ_OR_DOUBLE_OR_DOUBLE_EQ

			// \, ` or >= 0x7E
			default:
			{
				//// ERROR: Unknown symbol
				NOT_IMPLEMENTED;
			} break;
		}
	}

	return token;
}
