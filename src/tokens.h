#define TOKEN_KIND__BLOCK(N) ((N) << 3)
#define TOKEN_KIND__BINARY_TO_ASS(K) ((K) | 0x40)
#define TOKEN_KIND__ASS_TO_BINARY(K) ((K) & 0xBF)
#define TOKEN_KIND__PRECEDENCE(K) (((K) >> 3) & 0x7)
#define TOKEN_KIND__IS_BINARY(K) ((umm)((K) - Token__FirstBinary) < (umm)(Token__PastLastBinary - Token__FirstBinary))
#define TOKEN_KIND__IS_ASS(K) ((umm)((K) - Token__FirstBinaryAssignment) < (umm)(Token__PastLastBinaryAssignment - Token__FirstBinaryAssignment))

#define KEYWORD_LIST                                       \
	FIRST_KEYWORD(     Token_Asm,      'a', 's', "asm")      \
	REMAINING_KEYWORDS(Token_Break,    'b', 'r', "break")    \
	REMAINING_KEYWORDS(Token_Case,     'c', 'a', "case")     \
	REMAINING_KEYWORDS(Token_Continue, 'c', 'o', "continue") \
	REMAINING_KEYWORDS(Token_Defer,    'd', 'e', "defer")    \
	REMAINING_KEYWORDS(Token_Else,     'e', 'l', "else")     \
	REMAINING_KEYWORDS(Token_Enum,     'e', 'n', "enum")     \
	REMAINING_KEYWORDS(Token_False,    'f', 'a', "false")    \
	REMAINING_KEYWORDS(Token_For,      'f', 'o', "for")      \
	REMAINING_KEYWORDS(Token_If,       'i', 'f', "if")       \
	REMAINING_KEYWORDS(Token_In,       'i', 'n', "in")       \
	REMAINING_KEYWORDS(Token_Proc,     'p', 'r', "proc")     \
	REMAINING_KEYWORDS(Token_Return,   'r', 'e', "return")   \
	REMAINING_KEYWORDS(Token_Struct,   's', 't', "struct")   \
	REMAINING_KEYWORDS(Token_Switch,   's', 'w', "switch")   \
	REMAINING_KEYWORDS(Token_True,     't', 'r', "true")     \
	REMAINING_KEYWORDS(Token_Union,    'u', 'n', "union")    \
	REMAINING_KEYWORDS(Token_Using,    'u', 's', "using")    \
	REMAINING_KEYWORDS(Token_When,     'w', 'h', "when")     \

typedef enum Token_Kind
{
	Token_EOF   = 0,
	Token_Error = 1,

	Token_Ident,
	Token_String,
	Token_Char,
	Token_Int,
	Token_Int128,
	Token_Float,

	Token_Pound        = '#',
	Token_Cash         = '$',
	Token_OpenParen    = '(',
	Token_CloseParen   = ')',
	Token_Comma        = ',',
	Token_Dot          = '.',
	Token_Colon        = ':',
	Token_Semicolon    = ';',
	Token_QMark        = '?',
	Token_At           = '@',
	Token_OpenBracket  = '[',
	Token_CloseBracket = ']',
	Token_Hat          = '^',
	Token_OpenBrace    = '{',
	Token_CloseBrace   = '}',

	Token_Bang,
	Token_MinusMinus,
	Token_PlusPlus,

	Token__FirstBinary = TOKEN_KIND__BLOCK((0x80 >> 3) + 1),
	Token__FirstMulLevel = Token__FirstBinary,
	Token_Star = Token__FirstMulLevel,
	Token_Slash,
	Token_Percent,
	Token_And,
	Token_LtLt,
	Token_GtGt,
	Token__PastLastMulLevel,

	Token__FirstAddLevel = TOKEN_KIND__BLOCK((0x80 >> 3) + 2),
	Token_Plus = Token__FirstAddLevel,
	Token_Minus,
	Token_Or,
	Token_Tilde,
	Token__PastLastAddLevel,

	Token__FirstCmpLevel = TOKEN_KIND__BLOCK((0x80 >> 3) + 3),
	Token_EqEq = Token__FirstCmpLevel,
	Token_BangEq,
	Token_Lt,
	Token_LtEq,
	Token_Gt,
	Token_GtEq,
	Token__PastLastCmpLevel,

	Token__FirstAndLevel = TOKEN_KIND__BLOCK((0x80 >> 3) + 4),
	Token_AndAnd = Token__FirstAndLevel,
	Token__PastLastAndLevel,

	Token__FirstOrLevel = TOKEN_KIND__BLOCK((0x80 >> 3) + 5),
	Token_OrOr = Token__FirstOrLevel,
	Token__PastLastOrLevel,
	Token__PastLastBinary = Token__PastLastOrLevel,

	Token__FirstBinaryAssignment = TOKEN_KIND__BLOCK((0xC0 >> 3) + 1),
	Token__FirstMulLevelAssignment = Token__FirstBinaryAssignment,
	Token_StarEq = Token__FirstMulLevelAssignment,
	Token_SlashEq,
	Token_PercentEq,
	Token_AndEq,
	Token_LtLtEq,
	Token_GtGtEq,
	Token__PastLastMulLevelAssignment,

	Token__FirstAddLevelAssignment = TOKEN_KIND__BLOCK((0xC0 >> 3) + 2),
	Token_PlusEq = Token__FirstAddLevelAssignment,
	Token_MinusEq,
	Token_OrEq,
	Token_TildeEq,
	Token__PastLastAddLevelAssignment,

	Token_Eq,
	Token__PastLastBinaryAssignment,

	Token__FirstKeyword,
#define FIRST_KEYWORD(T, C0, C1, S) T = Token__FirstKeyword,
#define REMAINING_KEYWORDS(T, C0, C1, S) T,
	KEYWORD_LIST
#undef FIRST_KEYWORD
#undef REMAINING_KEYWORDS
	Token__PastLastKeyword,
} Token_Kind;

struct { String token_name; String keyword; } TokenKind_KeywordStrings[] = {
#define FIRST_KEYWORD(T, C0, C1, S) { .token_name = MS_STRING(STRINGIFY(T)), .keyword = MS_STRING(S) },
#define REMAINING_KEYWORDS(T, C0, C1, S) { .token_name = MS_STRING(STRINGIFY(T)), .keyword = MS_STRING(S) },
	KEYWORD_LIST
#undef FIRST_KEYWORD
#undef REMAINING_KEYWORDS
};

typedef __declspec(align(8)) struct Token
{
	u16 kind;
	u16 len;
	u32 offset;
} Token;

typedef union Token_Data
{
	u8* string;
	u64 integer;
	f64 floating;
} Token_Data;

static_assert(sizeof(Token) == sizeof(Token_Data) && _Alignof(Token) == _Alignof(Token_Data), "Token and Token_Data must have the same size and alignment for packing to work");

String
TokenKind__ToString(Token_Kind kind)
{
	switch (kind)
	{
		case Token_EOF:          return STRING("Token_EOF");          break;
		case Token_Error:        return STRING("Token_Error");        break;
		case Token_Bang:         return STRING("Token_Bang");         break;
		case Token_Pound:        return STRING("Token_Pound");        break;
		case Token_Cash:         return STRING("Token_Cash");         break;
		case Token_Percent:      return STRING("Token_Percent");      break;
		case Token_And:          return STRING("Token_And");          break;
		case Token_OpenParen:    return STRING("Token_OpenParen");    break;
		case Token_CloseParen:   return STRING("Token_CloseParen");   break;
		case Token_Star:         return STRING("Token_Star");         break;
		case Token_Plus:         return STRING("Token_Plus");         break;
		case Token_Comma:        return STRING("Token_Comma");        break;
		case Token_Minus:        return STRING("Token_Minus");        break;
		case Token_Dot:          return STRING("Token_Dot");          break;
		case Token_Slash:        return STRING("Token_Slash");        break;
		case Token_Colon:        return STRING("Token_Colon");        break;
		case Token_Semicolon:    return STRING("Token_Semicolon");    break;
		case Token_Lt:           return STRING("Token_Lt");           break;
		case Token_Eq:           return STRING("Token_Eq");           break;
		case Token_Gt:           return STRING("Token_Gt");           break;
		case Token_QMark:        return STRING("Token_QMark");        break;
		case Token_At:           return STRING("Token_At");           break;
		case Token_OpenBracket:  return STRING("Token_OpenBracket");  break;
		case Token_CloseBracket: return STRING("Token_CloseBracket"); break;
		case Token_Hat:          return STRING("Token_Hat");          break;
		case Token_OpenBrace:    return STRING("Token_OpenBrace");    break;
		case Token_Or:           return STRING("Token_Or");           break;
		case Token_CloseBrace:   return STRING("Token_CloseBrace");   break;
		case Token_Tilde:        return STRING("Token_Tilde");        break;
		case Token_BangEq:       return STRING("Token_BangEq");       break;
		case Token_PercentEq:    return STRING("Token_PercentEq");    break;
		case Token_StarEq:       return STRING("Token_StarEq");       break;
		case Token_SlashEq:      return STRING("Token_SlashEq");      break;
		case Token_EqEq:         return STRING("Token_EqEq");         break;
		case Token_TildeEq:      return STRING("Token_TildeEq");      break;
		case Token_AndEq:        return STRING("Token_AndEq");        break;
		case Token_OrEq:         return STRING("Token_OrEq");         break;
		case Token_PlusEq:       return STRING("Token_PlusEq");       break;
		case Token_MinusEq:      return STRING("Token_MinusEq");      break;
		case Token_AndAnd:       return STRING("Token_AndAnd");       break;
		case Token_OrOr:         return STRING("Token_OrOr");         break;
		case Token_PlusPlus:     return STRING("Token_PlusPlus");     break;
		case Token_MinusMinus:   return STRING("Token_MinusMinus");   break;
		case Token_LtEq:         return STRING("Token_LtEq");         break;
		case Token_GtEq:         return STRING("Token_GtEq");         break;
		case Token_LtLt:         return STRING("Token_LtLt");         break;
		case Token_GtGt:         return STRING("Token_GtGt");         break;
		case Token_LtLtEq:       return STRING("Token_LtLtEq");       break;
		case Token_GtGtEq:       return STRING("Token_GtGtEq");       break;
		case Token_Ident:        return STRING("Token_Ident");        break;
		case Token_String:       return STRING("Token_String");       break;
		case Token_Char:         return STRING("Token_Char");         break;
		case Token_Int:          return STRING("Token_Int");          break;
		case Token_Int128:       return STRING("Token_Int128");       break;
		case Token_Float:        return STRING("Token_Float");        break;
		default:
		{
			if (kind >= Token__FirstKeyword && kind < Token__PastLastKeyword)
			{
				return TokenKind_KeywordStrings[kind - Token__FirstKeyword].token_name;
			}
			else return STRING("NOT_A_TOKEN");
		} break;
	}
}

static_assert(Token__PastLastKeyword < 0xFF, "overflow");

static_assert(Token__PastLastMulLevel <= Token__FirstAddLevel, "token block overlap");
static_assert(Token__PastLastAddLevel <= Token__FirstCmpLevel, "token block overlap");
static_assert(Token__PastLastCmpLevel <= Token__FirstAndLevel, "token block overlap");
static_assert(Token__PastLastAndLevel <= Token__FirstOrLevel,  "token block overlap");

static_assert(!TOKEN_KIND__IS_BINARY(Token_EOF)          && !TOKEN_KIND__IS_ASS(Token_EOF),          "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Error)        && !TOKEN_KIND__IS_ASS(Token_Error),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Ident)        && !TOKEN_KIND__IS_ASS(Token_Ident),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_String)       && !TOKEN_KIND__IS_ASS(Token_String),       "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Char)         && !TOKEN_KIND__IS_ASS(Token_Char),         "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Int)          && !TOKEN_KIND__IS_ASS(Token_Int),          "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Int128)       && !TOKEN_KIND__IS_ASS(Token_Int128),       "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Float)        && !TOKEN_KIND__IS_ASS(Token_Float),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Pound)        && !TOKEN_KIND__IS_ASS(Token_Pound),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Cash)         && !TOKEN_KIND__IS_ASS(Token_Cash),         "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_OpenParen)    && !TOKEN_KIND__IS_ASS(Token_OpenParen),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_CloseParen)   && !TOKEN_KIND__IS_ASS(Token_CloseParen),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Comma)        && !TOKEN_KIND__IS_ASS(Token_Comma),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Dot)          && !TOKEN_KIND__IS_ASS(Token_Dot),          "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Colon)        && !TOKEN_KIND__IS_ASS(Token_Colon),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Semicolon)    && !TOKEN_KIND__IS_ASS(Token_Semicolon),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_QMark)        && !TOKEN_KIND__IS_ASS(Token_QMark),        "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_At)           && !TOKEN_KIND__IS_ASS(Token_At),           "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_OpenBracket)  && !TOKEN_KIND__IS_ASS(Token_OpenBracket),  "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_CloseBracket) && !TOKEN_KIND__IS_ASS(Token_CloseBracket), "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Hat)          && !TOKEN_KIND__IS_ASS(Token_Hat),          "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_OpenBrace)    && !TOKEN_KIND__IS_ASS(Token_OpenBrace),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_CloseBrace)   && !TOKEN_KIND__IS_ASS(Token_CloseBrace),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Bang)         && !TOKEN_KIND__IS_ASS(Token_Bang),         "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_MinusMinus)   && !TOKEN_KIND__IS_ASS(Token_MinusMinus),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_PlusPlus)     && !TOKEN_KIND__IS_ASS(Token_PlusPlus),     "TOKEN_KIND_IS_* macros are broken");

static_assert( TOKEN_KIND__IS_BINARY(Token_Star)    && !TOKEN_KIND__IS_ASS(Token_Star),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Slash)   && !TOKEN_KIND__IS_ASS(Token_Slash),   "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Percent) && !TOKEN_KIND__IS_ASS(Token_Percent), "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_And)     && !TOKEN_KIND__IS_ASS(Token_And),     "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_LtLt)    && !TOKEN_KIND__IS_ASS(Token_LtLt),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_GtGt)    && !TOKEN_KIND__IS_ASS(Token_GtGt),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Plus)    && !TOKEN_KIND__IS_ASS(Token_Plus),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Minus)   && !TOKEN_KIND__IS_ASS(Token_Minus),   "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Or)      && !TOKEN_KIND__IS_ASS(Token_Or),      "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Tilde)   && !TOKEN_KIND__IS_ASS(Token_Tilde),   "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_EqEq)    && !TOKEN_KIND__IS_ASS(Token_EqEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_BangEq)  && !TOKEN_KIND__IS_ASS(Token_BangEq),  "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Lt)      && !TOKEN_KIND__IS_ASS(Token_Lt),      "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_LtEq)    && !TOKEN_KIND__IS_ASS(Token_LtEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_Gt)      && !TOKEN_KIND__IS_ASS(Token_Gt),      "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_GtEq)    && !TOKEN_KIND__IS_ASS(Token_GtEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_AndAnd)  && !TOKEN_KIND__IS_ASS(Token_AndAnd),  "TOKEN_KIND_IS_* macros are broken");
static_assert( TOKEN_KIND__IS_BINARY(Token_OrOr)    && !TOKEN_KIND__IS_ASS(Token_OrOr),    "TOKEN_KIND_IS_* macros are broken");

static_assert(!TOKEN_KIND__IS_BINARY(Token_StarEq)    &&  TOKEN_KIND__IS_ASS(Token_StarEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_SlashEq)   &&  TOKEN_KIND__IS_ASS(Token_SlashEq),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_PercentEq) &&  TOKEN_KIND__IS_ASS(Token_PercentEq), "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_AndEq)     &&  TOKEN_KIND__IS_ASS(Token_AndEq),     "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_LtLtEq)    &&  TOKEN_KIND__IS_ASS(Token_LtLtEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_GtGtEq)    &&  TOKEN_KIND__IS_ASS(Token_GtGtEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_PlusEq)    &&  TOKEN_KIND__IS_ASS(Token_PlusEq),    "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_MinusEq)   &&  TOKEN_KIND__IS_ASS(Token_MinusEq),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_OrEq)      &&  TOKEN_KIND__IS_ASS(Token_OrEq),      "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_TildeEq)   &&  TOKEN_KIND__IS_ASS(Token_TildeEq),   "TOKEN_KIND_IS_* macros are broken");
static_assert(!TOKEN_KIND__IS_BINARY(Token_Eq)        &&  TOKEN_KIND__IS_ASS(Token_Eq),        "TOKEN_KIND_IS_* macros are broken");

static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Star)    == Token_StarEq,    "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Slash)   == Token_SlashEq,   "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Percent) == Token_PercentEq, "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_And)     == Token_AndEq,     "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_LtLt)    == Token_LtLtEq,    "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_GtGt)    == Token_GtGtEq,    "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Plus)    == Token_PlusEq,    "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Minus)   == Token_MinusEq,   "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Or)      == Token_OrEq,      "TOKEN_KIND__BINARY_TO_ASS is broken");
static_assert(TOKEN_KIND__BINARY_TO_ASS(Token_Tilde)   == Token_TildeEq,   "TOKEN_KIND__BINARY_TO_ASS is broken");

static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_StarEq)    == Token_Star,    "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_SlashEq)   == Token_Slash,   "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_PercentEq) == Token_Percent, "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_AndEq)     == Token_And,     "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_LtLtEq)    == Token_LtLt,    "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_GtGtEq)    == Token_GtGt,    "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_PlusEq)    == Token_Plus,    "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_MinusEq)   == Token_Minus,   "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_OrEq)      == Token_Or,      "TOKEN_KIND__ASS_TO_BINARY is broken");
static_assert(TOKEN_KIND__ASS_TO_BINARY(Token_TildeEq)   == Token_Tilde,   "TOKEN_KIND__ASS_TO_BINARY is broken");
