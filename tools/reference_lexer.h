#define REF_LEXER_ZPAD 65

typedef struct Ref_Lexer
{
	u8* contents;
	u8* cursor;
} Ref_Lexer;

bool
RefLexFile(String input, Virtual_Array* token_array, Virtual_Array* string_array, Token** first_token, u32* token_count)
{
	ASSERT(input.len > REF_LEXER_ZPAD && IsZero(input.data + input.len-REF_LEXER_ZPAD, REF_LEXER_ZPAD));

	VA_EnsureCommitted(token_array, input.len/8);

	Ref_Lexer lexer = (Ref_Lexer){
		.contents = input.data,
		.cursor   = input.data,
	};

	*first_token = VA_EndPointer(token_array);

	for (;;)
	{
		Token* token = VA_Push(token_array);

		for (;;)
		{
			while ((u8)(*lexer.cursor-1) < (u8)0x20) ++lexer.cursor;

			if (lexer.cursor[0] == '/' && lexer.cursor[1] == '*')
			{
				lexer.cursor += 2;

				while (*lexer.cursor != 0 && !(lexer.cursor[0] == '*' && lexer.cursor[1] == '/')) ++lexer.cursor;

				if (*lexer.cursor == 0)
				{
					//// ERROR: Unterminated block comment
					NOT_IMPLEMENTED;
				}
				else
				{
					lexer.cursor += 2;
				}
			}
			else if (lexer.cursor[0] == '/' && lexer.cursor[1] == '/')
			{
				while (*lexer.cursor != 0 && *lexer.cursor != '\n') ++lexer.cursor;
			}
			else break;
		}

		u32 offset = (u32)(lexer.cursor - lexer.contents);

		if (Char_IsAlpha(*lexer.cursor) || *lexer.cursor == '_')
		{
			u8* start = lexer.cursor;

			while (Char_IsAlpha(*lexer.cursor) || Char_IsDigit(*lexer.cursor) || *lexer.cursor == '_') ++lexer.cursor;

			u64 len = lexer.cursor - start;

			if (len > ~(u16)0)
			{
				//// ERROR: Identifier is too long
				NOT_IMPLEMENTED;
			}

			*token = (Token){
				.kind   = Token_Ident,
				.len    = (u16)len,
				.offset = offset,
			};
		}
		else if (lexer.cursor[0] == '0' && (lexer.cursor[1] == 'x' || lexer.cursor[1] == 'h'))
		{
			bool is_hex_float = (lexer.cursor[1] == 'h');

			lexer.cursor += 2;

			umm digit_count = 0;
			u64 value = 0;

			for (;;)
			{
				if (Char_IsDigit(*lexer.cursor))
				{
					value <<= 4;
					value  |= *lexer.cursor & 0xF;
					++digit_count;
					++lexer.cursor;
				}
				else if (Char_IsHexAlpha(*lexer.cursor))
				{
					value <<= 4;
					value  |= 9 + (*lexer.cursor & 0x1F);
					++digit_count;
					++lexer.cursor;
				}
				else if (*lexer.cursor == '_')
				{
					++lexer.cursor;
				}
				else break;
			}

			if (is_hex_float)
			{
				if (digit_count == 16)
				{
					*token = (Token){
						.kind   = Token_Float,
						.len    = digit_count,
						.offset = offset,
					};

					*(Token_Data*)VA_Push(token_array) = (Token_Data){ .floating = (F64_Bits){ .bits = value }.f };
				}
				else if (digit_count == 8)
				{
					*token = (Token){
						.kind   = Token_Float,
						.len    = digit_count,
						.offset = offset,
					};

					*(Token_Data*)VA_Push(token_array) = (Token_Data){ .floating = (F32_Bits){ .bits = (u32)value }.f };
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
					*token = (Token){
						.kind   = Token_Int,
						.len    = digit_count,
						.offset = offset,
					};

					*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = value };
				}
			}
		}
		else if (Char_IsDigit(*lexer.cursor))
		{
			umm digit_count = 0;
			u64 value = 0;

			u8* start = lexer.cursor;

			for (;;)
			{
				if (Char_IsDigit(*lexer.cursor))
				{
					value = value*10 + (*lexer.cursor&0xF);
					++digit_count;
					++lexer.cursor;
				}
				else if (*lexer.cursor == '_')
				{
					++lexer.cursor;
				}
				else break;
			}

			if (*lexer.cursor == '.' || (*lexer.cursor&0xDF) == 'E')
			{
				char buffer[256] = {0};
				umm buf_cur = 0;

				umm len = lexer.cursor - start;
				for (umm i = 0; i < len; ++i)
				{
					if (start[i] != '_')
					{
						ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
						buffer[buf_cur++] = start[i];
					}
				}

				if (*lexer.cursor == '.')
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = '.';
					++lexer.cursor;

					umm fraction_digit_count = 0;
					for (;;)
					{
						if (Char_IsDigit(*lexer.cursor))
						{
							ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
							buffer[buf_cur++] = *lexer.cursor;
							++fraction_digit_count;
							++lexer.cursor;
						}
						else if (*lexer.cursor == '_')
						{
							++lexer.cursor;
						}
						else break;
					}

					if (fraction_digit_count == 0)
					{
						//// ERROR: Missing digits
						NOT_IMPLEMENTED;
					}
				}

				if ((*lexer.cursor&0xDF) == 'E')
				{
					ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
					buffer[buf_cur++] = *lexer.cursor;
					++lexer.cursor;

					if (*lexer.cursor == '+' || *lexer.cursor == '-')
					{
						ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
						buffer[buf_cur++] = *lexer.cursor;
						++lexer.cursor;
					}

					umm exp_digit_count = 0;
					for (;;)
					{
						if (Char_IsDigit(*lexer.cursor))
						{
							ASSERT(buf_cur < ARRAY_LEN(buffer)-1);
							buffer[buf_cur++] = *lexer.cursor;
							++exp_digit_count;
							++lexer.cursor;
						}
						else if (*lexer.cursor == '_')
						{
							++lexer.cursor;
						}
						else break;
					}

					if (exp_digit_count == 0)
					{
						//// ERROR: Missing digits
						NOT_IMPLEMENTED;
					}
				}
				
				*token = (Token){
					.kind   = Token_Float,
					.len    = 0, // TODO: can anything useful be encoded in "len"?
					.offset = offset,
				};

				*(Token_Data*)VA_Push(token_array) = (Token_Data){ .floating = strtod(buffer, 0) };
			}
			else
			{
				if (digit_count <= 19)
				{
					*token = (Token){
						.kind   = Token_Int,
						.len    = digit_count,
						.offset = offset,
					};

					*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = value };
				}
				else
				{
					u64 value_lo = 0;
					u64 value_hi = 0;

					for (u8* scan = start; scan != lexer.cursor; ++scan)
					{
						if (*scan != '_')
						{
							u64 lo_hi;
							value_lo = _umul128(value_lo, 10, &lo_hi);
							value_hi = _umul128(value_hi, 10, &(u64){0});

							u8 c1 = _addcarry_u64(0, value_lo, *scan&0xF, &value_lo);
							_addcarry_u64(c1, value_hi, lo_hi, &value_hi);
						}
					}

					if (value_hi == 0)
					{
						*token = (Token){
							.kind   = Token_Int,
							.len    = digit_count,
							.offset = offset,
						};

						*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = value_lo };
					}
					else
					{
						*token = (Token){
							.kind   = Token_Int128,
							.len    = digit_count,
							.offset = offset,
						};

						*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = value_lo };
						*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = value_hi };
					}
				}
			}
		}
		else if (*lexer.cursor == '"' || *lexer.cursor == '\'')
		{
			u8 terminator = *lexer.cursor;

			++lexer.cursor;
			u8* start = lexer.cursor;
			for (;;)
			{
				if (*lexer.cursor == 0 || *lexer.cursor == terminator) break;
				else if (*lexer.cursor == '\\') lexer.cursor += 2;
				else                             lexer.cursor += 1;
			}
			u8* end = lexer.cursor;

			if (*lexer.cursor == 0)
			{
				//// ERROR: Unterminated string literal
				NOT_IMPLEMENTED;
			}
			else
			{
				++lexer.cursor; // NOTE: Skip terminator

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

				if (terminator == '\'')
				{
					if (string.len != 1)
					{
						//// ERROR: Character literal must contain exactly one character
						NOT_IMPLEMENTED;
					}
					else
					{
						*token = (Token){
							.kind   = Token_Char,
							.len    = 0,
							.offset = offset,
						};

						// TODO: This wastes bytes but is clearer than storing the char in "len" (which is probably fine because char literals are almost never used)
						//       Consider changing this
						*(Token_Data*)VA_Push(token_array) = (Token_Data){ .integer = *string.data };
					}
				}
				else
				{
					if (string.len > ~(u16)0)
					{
						//// ERROR: String is too long
						NOT_IMPLEMENTED;
					}
					else
					{
						*token = (Token){
							.kind   = Token_String,
							.len    = (u16)string.len,
							.offset = offset,
						};

						*(Token_Data*)VA_Push(token_array) = (Token_Data){ .string = string.data };
					}
				}
			}
		}
		else if (*lexer.cursor == 0)
		{
			*token = (Token){
				.kind   = Token_EOF,
				.offset = offset,
			};

			break;
		}
		else
		{
			u8 c = *lexer.cursor;
			++lexer.cursor;

			u8 c1_eq = (lexer.cursor[0] == '=');
			u8 c1_c  = (lexer.cursor[0] == c);
			u8 c2_eq = (lexer.cursor[1] == '=');

			switch (c)
			{
				case '(':  *token = (Token){ .kind = Token_OpenParen,    .offset = offset }; break;
				case ')':  *token = (Token){ .kind = Token_CloseParen,   .offset = offset }; break;
				case '[':  *token = (Token){ .kind = Token_OpenBracket,  .offset = offset }; break;
				case ']':  *token = (Token){ .kind = Token_CloseBracket, .offset = offset }; break;
				case '{':  *token = (Token){ .kind = Token_OpenBrace,    .offset = offset }; break;
				case '}':  *token = (Token){ .kind = Token_CloseBrace,   .offset = offset }; break;
				case '#':  *token = (Token){ .kind = Token_Pound,        .offset = offset }; break;
				case '$':  *token = (Token){ .kind = Token_Cash,         .offset = offset }; break;
				case ',':  *token = (Token){ .kind = Token_Comma,        .offset = offset }; break;
				case ':':  *token = (Token){ .kind = Token_Colon,        .offset = offset }; break;
				case ';':  *token = (Token){ .kind = Token_Semicolon,    .offset = offset }; break;
				case '?':  *token = (Token){ .kind = Token_QMark,        .offset = offset }; break;
				case '@':  *token = (Token){ .kind = Token_At,           .offset = offset }; break;
				case '^':  *token = (Token){ .kind = Token_Hat,          .offset = offset }; break;
				case '.':  *token = (Token){ .kind = Token_Dot,          .offset = offset }; break;

				#define SINGLE_OR_SINGLE_EQ(SINGLE, SINGLE_EQ)                                  \
					{                                                                             \
						*token = (Token){ .kind = (c1_eq ? SINGLE_EQ : SINGLE), .offset = offset }; \
						lexer.cursor += c1_eq;                                                      \
					}

				case '/': SINGLE_OR_SINGLE_EQ(Token_Slash,   Token_SlashEq);   break;
				case '!': SINGLE_OR_SINGLE_EQ(Token_Bang,    Token_BangEq);    break;
				case '%': SINGLE_OR_SINGLE_EQ(Token_Percent, Token_PercentEq); break;
				case '*': SINGLE_OR_SINGLE_EQ(Token_Star,    Token_StarEq);    break;
				case '=': SINGLE_OR_SINGLE_EQ(Token_Eq,      Token_EqEq);      break;
				case '~': SINGLE_OR_SINGLE_EQ(Token_Tilde,   Token_TildeEq);   break;
				#undef SINGLE_OR_SINGLE_EQ

				#define SINGLE_OR_SINGLE_EQ_OR_DOUBLE(SINGLE, SINGLE_EQ, DOUBLE)                  \
					{                                                                               \
						if (c1_c)                                                                     \
						{                                                                             \
							*token = (Token){ .kind = DOUBLE, .offset = offset };                       \
							lexer.cursor += 1;                                                          \
						}                                                                             \
						else                                                                          \
						{                                                                             \
							*token = (Token){ .kind = (c1_eq ? SINGLE_EQ : SINGLE), .offset = offset }; \
							lexer.cursor += c1_eq;                                                      \
						}                                                                             \
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
							*token = (Token){ .kind = (c2_eq ? DOUBLE_EQ : DOUBLE), .offset = offset };        \
							lexer.cursor += 1 + c2_eq;                                                         \
						}                                                                                    \
						else                                                                                 \
						{                                                                                    \
							*token = (Token){ .kind = (c1_eq ? SINGLE_EQ : SINGLE), .offset = offset };        \
							lexer.cursor += c1_eq;                                                             \
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
	}

	*token_count = (u32)((Token*)VA_EndPointer(token_array) - *first_token);

	return true; // TODO: Error reporting
}
