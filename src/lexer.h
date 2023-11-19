#define TOKEN_BLOCK(IDX) ((IDX) << 4)

typedef enum Token_Kind
{
	Token_Invalid = 0,
	Token_EndOfFile,
  
  Token__FirstMulLevel = TOKEN_BLOCK(2),
  Token_Star = Token__FirstMulLevel,
  Token_Slash,
  Token_Mod,
  Token_And,
  Token_LeLe,
  Token_GeGe,
  Token_GeGeGe,
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN_BLOCK(3),
  Token_Plus = Token__FirstAddLevel,
  Token_Minus,
  Token_Or,
  Token_Tilde,
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_BLOCK(4),
  Token_EqualsEQ = Token__FirstCmpLevel,
	Token_NotEQ,
  Token_Le,
  Token_LeEQ,
  Token_Ge,
  Token_GeEQ,
  Token__PastLastCmpLevel,

  Token__FirstAndLevel = TOKEN_BLOCK(5),
  Token_AndAnd = Token__FirstAndLevel,
  Token__PastLastAndLevel,

  Token__FirstOrLevel = TOKEN_BLOCK(6),
  Token_OrOr = Token__FirstOrLevel,
  Token__PastLastOrLevel,

	Token__FirstAssignment = TOKEN_BLOCK(10),
  Token__FirstMulLevelAssignment = Token__FirstAssignment,
  Token_StarEQ = Token__FirstMulLevelAssignment,
  Token_SlashEQ,
  Token_ModEQ,
  Token_AndEQ,
  Token_LeLeEQ,
  Token_GeGeEQ,
  Token_GeGeGeEQ,
  Token__PastLastMulLevelAssignment,

  Token__FirstAddLevelAssignment = TOKEN_BLOCK(11),
  Token_PlusEQ = Token__FirstAddLevelAssignment,
  Token_MinusEQ,
  Token_OrEQ,
	Token_TildeEQ,
  Token__PastLastAddLevelAssignment,

  Token__FirstAndLevelAssignment = TOKEN_BLOCK(13),
  Token_AndAndEQ = Token__FirstAndLevelAssignment,
  Token__PastLastAndLevelAssignment,

  Token__FirstOrLevelAssignment = TOKEN_BLOCK(14),
  Token_OrOrEQ = Token__FirstOrLevelAssignment,
  Token__PastLastOrLevelAssignment,
	Token__PastLastAssignment = Token__PastLastOrLevelAssignment,

	Token__FirstKeyword = Token__PastLastAssignment,
	Token_True,
	Token_False,
	Token_If,
	Token_While,
	Token_When,
	Token_Break,
	Token_Continue,
	Token_Return,
	Token_Struct,
	Token_Union,
	Token_Proc,
	Token__PastLastKeyword,

	Token_Int,
	Token_Float,
  Token_String,
	Token_Ident,

  Token_OpenParen,
  Token_CloseParen,
  Token_OpenBracket,
  Token_CloseBracket,
  Token_OpenBrace,
  Token_CloseBrace,
  Token_Period,
	Token_Elipsis,
  Token_Colon,
  Token_Comma,
  Token_Semicolon,
  Token_Hat,
  Token_At,
	Token_Pound,
	Token_Cash,
	Token_Underscore,
  Token_Arrow,
  Token_PlusPlus,
  Token_MinusMinus,
  Token_TripleMinus,
  Token_Qmark,

	Token_Equals = TOKEN_BLOCK(36),
	Token_Not,
} Token_Kind;

STATIC_ASSERT(Token__PastLastMulLevel           <= Token__FirstAddLevel);
STATIC_ASSERT(Token__PastLastAddLevel           <= Token__FirstCmpLevel);
STATIC_ASSERT(Token__PastLastCmpLevel           <= Token__FirstAndLevel);
STATIC_ASSERT(Token__PastLastAndLevel           <= Token__FirstOrLevel);
STATIC_ASSERT(Token__PastLastOrLevel            <= Token__FirstAssignment);
STATIC_ASSERT(Token__PastLastMulLevelAssignment <= Token__FirstAddLevelAssignment);
STATIC_ASSERT(Token__PastLastAddLevelAssignment <= Token__FirstAndLevelAssignment);
STATIC_ASSERT(Token__PastLastAndLevelAssignment <= Token__FirstOrLevelAssignment);

STATIC_ASSERT((Token_Equals ^ 512) == Token_EqualsEQ);
STATIC_ASSERT((Token_Not    ^ 512) == Token_NotEQ);

typedef struct Token
{
	Token_Kind kind;
	u32 offset;
	u32 offset_to_line;
	u32 line;
	u32 size;

	union
	{
		Identifier ident;
		String_Lit string;
		i128 integer;
		f64 floating;
	};
} Token;

// NOTE: Assumes files are no longer than 4 GB
// NOTE: This function is written in a very convoluted, hopefully mostly branchless, way.
//       Although this is probably a bad idea for both readability and maintainability,
//       as well as probably being harder for the compiler to optimize, it is fun to write,
//       so it will probably stay this way, regardless of the problems. Additionally, it
//       would probably be smarter to do a prepass to determine token boundaries and then
//       parse identifier, strings and numbers, instead of doing everything in one go.
//       However, I am too lazy to go that far, and none of this really matters since
//       the lexer is such a small part of the compilation process.
static void
LexFile(u8* contents)
{
	u8* cursor = contents;

	u32 offset_to_line = 0;
	u32 line           = 1;

	for (;;)
	{
		for (;;)
		{
			if (*cursor == '\n')
			{
				cursor += 1;

				offset_to_line = (u32)(cursor - contents);
				line          += 1;
			}
			else if (Char_IsWhitespace(*cursor))
			{
				cursor += 1;
			}
			else if (cursor[0] == '/' && cursor[1] == '/')
			{
				cursor += 2;
				while (*cursor != 0 && *cursor != '\n') ++cursor;
			}
			else if (cursor[0] == '/' && cursor[1] == '*')
			{
				cursor += 2;
				for (uint nesting = 1; nesting > 0;)
				{
					if      (cursor[0] == '/' && cursor[1] == '*') cursor += 2, nesting += 1;
					else if (cursor[0] == '*' && cursor[1] == '/') cursor += 2, nesting -= 1;
					else if (*cursor != 0)                         cursor += 1;
					else
					{
						//// ERROR: Unterminated block comment
						NOT_IMPLEMENTED;
					}
				}
			}
			else break;
		}

		Token* token = 0;
		token->kind           = Token_Invalid;
		token->offset         = (u32)(cursor - contents);
		token->offset_to_line = offset_to_line;
		token->line           = line;

		u8 c[4] = {0};
		for (uint i = 0; i < ARRAY_SIZE(c) && cursor[i] != 0; ++i) c[i] = cursor[i];

		cursor += (c[0] != 0);

		bool c1_rep  = (c[1] == c[0]);
		bool c_eq[2] = {c[1] == '"', c[2] == '='};
		u32 c_bit[2] = {c_eq[1] << 3, c_eq[2] << 3};
		switch (c[0])
		{
			case 0: token->kind = Token_EndOfFile; break;

			case '(': token->kind = Token_OpenParen;    break;
			case ')': token->kind = Token_CloseParen;   break;
			case '[': token->kind = Token_OpenBracket;  break;
			case ']': token->kind = Token_CloseBracket; break;
			case '{': token->kind = Token_OpenBrace;    break;
			case '}': token->kind = Token_CloseBrace;   break;
			case ':': token->kind = Token_Colon;        break;
			case ',': token->kind = Token_Comma;        break;
			case ';': token->kind = Token_Semicolon;    break;
			case '^': token->kind = Token_Hat;          break;
			case '@': token->kind = Token_At;           break;
			case '#': token->kind = Token_Pound;        break;
			case '$': token->kind = Token_Cash;         break;
			case '?': token->kind = Token_Qmark;        break;
		
			case '*': token->kind = Token_Star  | c_bit[1]; cursor += c_eq[1]; break;
			case '/': token->kind = Token_Slash | c_bit[1]; cursor += c_eq[1]; break;
			case '%': token->kind = Token_Mod   | c_bit[1]; cursor += c_eq[1]; break;
			case '~': token->kind = Token_Tilde | c_bit[1]; cursor += c_eq[1]; break;

			case '.': token->kind = Token_Period + c1_rep; cursor += c1_rep; break;

			case '+': token->kind = (c1_rep ? Token_PlusPlus : Token_Plus) | c_bit[1]; cursor += c_bit[1]; break;

			case '&': token->kind = (c1_rep ? Token_AndAnd : Token_And) | c_bit[c1_rep]; cursor += (c1_rep + c_eq[c1_rep]); break;
			case '|': token->kind = (c1_rep ? Token_OrOr   : Token_Or)  | c_bit[c1_rep]; cursor += (c1_rep + c_eq[c1_rep]); break;

			case '!': token->kind = Token_Not    ^ (c_bit[1] << 6); cursor += c_bit[1]; break;
			case '=': token->kind = Token_Equals ^ (c_bit[1] << 6); cursor += c_bit[1]; break;
			
			case '<':
			{
				if (c1_rep)
				{
					token->kind = Token_LeLe | c_bit[2];
					cursor     += c1_rep + c_eq[2];
				}
				else
				{
					token->kind = Token_Le + c_eq[1];
					cursor     += c_eq[1];
				}
			} break;

			case '>':
			{
				if (c1_rep)
				{
					bool triple = (c[2] == '>');
					bool eq = (c[2 + triple] == '=');

					token->kind = (Token_GeGe + triple) | (eq << 3);
					cursor     += c1_rep + triple + eq;
				}
				else
				{
					token->kind = Token_Ge + c_eq[1];
					cursor     += c_eq[1];
				}
			} break;

			case '-':
			{
				if (c1_rep)
				{
					bool c2_rep = (c[2] == '-');
					token->kind = Token_MinusMinus + c2_rep;
					cursor     += 1 + c2_rep;
				}
				else
				{
					bool arrow = (c[1] == '>');
					token->kind = (arrow ? Token_Arrow : Token_Minus) | c_bit[1];
					cursor     += c_bit[1] + arrow;
				}
			} break;

			default:
			{
				if (Char_IsAlpha(c[0]) || c[0] == '_')
				{
					String ident = { .data = cursor };
					while (Char_IsAlpha(*cursor) || Char_IsDigit(*cursor) || *cursor == '_') ++cursor;
					ident.size = cursor - ident.data;

					if (ident.size == 1 && *ident.data == '_') token->kind = Token_Underscore;
					else
					{
						// TODO: intern ident and check for keyword
						NOT_IMPLEMENTED;
					}
				}
				else if (Char_IsDigit(c[0]))
				{
					if (c[0] == '0' && (c[1] == 'x' || c[1] == 'h'))
					{
						bool is_float = (c[1] == 'h');
						++cursor;

						i128 value;
						uint digit_count = 0;
						for (;; ++cursor)
						{
							u8 digit;
							if (*cursor == '_') continue;
							else if (Char_IsHexAlpha(*cursor)) digit = (*cursor&0xF) + 9;
							else if (Char_IsDigit(*cursor))    digit = *cursor&0xF;
							else break;

							value.hi = (value.hi << 4) | (value.lo >> 60)
							value.lo = (value.lo << 4) | digit;
							++digit_count;
						}

						if (is_float)
						{
							if (digit_count == 8)
							{
								token->kind = Token_Float;
								token->floating = (F32_Bits){ .bits = (u32)value.lo }.f;
							}
							else if (digit_count == 16)
							{
								token->kind = Token_Float;
								token->floating = (F64_Bits){ .bits = value.lo }.f;
							}
							else
							{
								//// ERROR: Invalid number of digits for hexadecimal float literal, must be either 8 or 16
								NOT_IMPLEMENTED;
							}
						}
						else
						{
							if (digit_count == 0)
							{
								//// ERROR: No digits after hexadecimal prefix in integer literal
								NOT_IMPLEMENTED;
							}
							else if (digit_count > 32)
							{
								//// ERROR: Too many digits in hexadecimal integer literal
								NOT_IMPLEMENTED;
							}
							else
							{
								token->kind    = Token_Int;
								token->integer = value;
							}
						}
					}
					else
					{
						NOT_IMPLEMENTED;
					}
				}
				else if (c[0] == '"')
				{
					String raw_string = { .data = cursor };

					while (*cursor != 0 && *cursor != '"') cursor += 1 + (cursor[0] == '\\' && cursor[1] != 0);

					if (*cursor == 0)
					{
						//// ERROR: unterminated string literal
						NOT_IMPLEMENTED;
					}
					else
					{
						raw_string.size = cursor - raw_string.data;
						++cursor;

						NOT_IMPLEMENTED;
					}
				}
				else
				{
					//// ERROR: Invalid character
					NOT_IMPLEMENTED;
				}
			} break;
		}

		token->size = (u32)(cursor - contents) - token->offset;
	}
}
