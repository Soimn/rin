typedef enum Token_Kind
{
	Token_EOF   = 0,
	Token_Ident,
	Token_String,
	Token_Char,
	Token_Int,
	Token_Float,
	Token_Error = 0x8000,

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

	Token_Percent      = '%' | 0x080, // BOP
	Token_PercentEq    = '%' | 0x180, // ASS

	Token_Star         = '*' | 0x080, // BOP
	Token_StarEq       = '*' | 0x180, // ASS

	Token_Slash        = '/' | 0x080, // BOP
	Token_SlashEq      = '/' | 0x180, // ASS

	Token_Tilde        = '~' | 0x080, // BOP
	Token_TildeEq      = '~' | 0x180, // ASS

	Token_And          = '&' | 0x080, // BOP
	Token_AndEq        = '&' | 0x180, // ASS
	Token_AndAnd       = '&' | 0x280, // BOP

	Token_Or           = '|' | 0x080, // BOP
	Token_OrEq         = '|' | 0x180, // ASS
	Token_OrOr         = '|' | 0x280, // BOP

	Token_Lt           = '<' | 0x080, // BOP
	Token_LtEq         = '<' | 0x480, // BOP
	Token_LtLt         = '<' | 0x280, // BOP
	Token_LtLtEq       = '<' | 0x380, // ASS

	Token_Gt           = '>' | 0x080, // BOP
	Token_GtEq         = '>' | 0x480, // BOP
	Token_GtGt         = '>' | 0x280, // BOP
	Token_GtGtEq       = '>' | 0x380, // ASS

	Token_Minus        = '-' | 0x880, // BOP
	Token_MinusEq      = '-' | 0x980, // ASS
	Token_MinusMinus   = '-' | 0xA80, // --

	Token_Plus         = '+' | 0x880, // BOP
	Token_PlusEq       = '+' | 0x980, // ASS
	Token_PlusPlus     = '+' | 0xA80, // --

	Token_Eq           = '=' | 0x000, // ASS
	Token_EqEq         = '=' | 0x300, // BOP

	Token_Bang         = '!' | 0x000, // --
	Token_BangEq       = '!' | 0x100, // BOP
} Token_Kind;

#define TOKEN_IS_BINARY_OP(K) ((((K) & 0x180) == 0x080 && ((K) < 0xA00)) || (K) == Token_EqEq || (K) == Token_BangEq)

#define TOKEN_IS_BINARY_ASS(K) ((K) == Token_Eq || (((K) & 0x180) == 0x180))

static_assert(TOKEN_IS_BINARY_OP(Token_Percent), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Percent), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_PercentEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_PercentEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Star), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Star), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_StarEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_StarEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Slash), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Slash), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_SlashEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_SlashEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Tilde), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Tilde), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_TildeEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_TildeEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_And), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_And), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_AndEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_AndEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_AndAnd), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_AndAnd), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Or), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Or), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_OrEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_OrEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_OrOr), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_OrOr), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Lt), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Lt), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_LtEq), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_LtEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_LtLt), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_LtLt), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_LtLtEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_LtLtEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Gt), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Gt), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_GtEq), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_GtEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_GtGt), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_GtGt), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_GtGtEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_GtGtEq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Minus), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Minus), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_MinusEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_MinusEq), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_MinusMinus), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_MinusMinus), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_Plus), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Plus), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_PlusEq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_PlusEq), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_PlusPlus), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_PlusPlus), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_Eq), "TOKEN_IS_* macros are broken");
static_assert(TOKEN_IS_BINARY_ASS(Token_Eq), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_EqEq), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_EqEq), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_Bang), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_Bang), "TOKEN_IS_* macros are broken");

static_assert(TOKEN_IS_BINARY_OP(Token_BangEq), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_ASS(Token_BangEq), "TOKEN_IS_* macros are broken");

static_assert(!TOKEN_IS_BINARY_OP(Token_Pound)        && !TOKEN_IS_BINARY_ASS(Token_Pound),        "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Cash)         && !TOKEN_IS_BINARY_ASS(Token_Cash),         "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_OpenParen)    && !TOKEN_IS_BINARY_ASS(Token_OpenParen),    "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_CloseParen)   && !TOKEN_IS_BINARY_ASS(Token_CloseParen),   "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Comma)        && !TOKEN_IS_BINARY_ASS(Token_Comma),        "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Dot)          && !TOKEN_IS_BINARY_ASS(Token_Dot),          "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Colon)        && !TOKEN_IS_BINARY_ASS(Token_Colon),        "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Semicolon)    && !TOKEN_IS_BINARY_ASS(Token_Semicolon),    "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_QMark)        && !TOKEN_IS_BINARY_ASS(Token_QMark),        "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_At)           && !TOKEN_IS_BINARY_ASS(Token_At),           "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_OpenBracket)  && !TOKEN_IS_BINARY_ASS(Token_OpenBracket),  "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_CloseBracket) && !TOKEN_IS_BINARY_ASS(Token_CloseBracket), "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Hat)          && !TOKEN_IS_BINARY_ASS(Token_Hat),          "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_OpenBrace)    && !TOKEN_IS_BINARY_ASS(Token_OpenBrace),    "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_CloseBrace)   && !TOKEN_IS_BINARY_ASS(Token_CloseBrace),   "TOKEN_IS_* macros are broken");
static_assert(!TOKEN_IS_BINARY_OP(Token_EOF)          && !TOKEN_IS_BINARY_ASS(Token_EOF),          "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Ident)        && !TOKEN_IS_BINARY_ASS(Token_Ident),        "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_String)       && !TOKEN_IS_BINARY_ASS(Token_String),       "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Char)         && !TOKEN_IS_BINARY_ASS(Token_Char),         "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Int)          && !TOKEN_IS_BINARY_ASS(Token_Int),          "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Float)        && !TOKEN_IS_BINARY_ASS(Token_Float),        "TOKEN_IS_* macros are borken");
static_assert(!TOKEN_IS_BINARY_OP(Token_Error)        && !TOKEN_IS_BINARY_ASS(Token_Error),        "TOKEN_IS_* macros are borken");

typedef struct Token
{
	u16 kind;
	u16 len;
	u32 offset;
	union
	{
		u8* data;
		u64 integer;
		f64 floating;
	};
} Token;

String
TokenKind__ToString(Token_Kind kind)
{
	switch (kind)
	{
		case Token_EOF:          return STRING("Token_EOF");          break;
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
		case Token_Float:        return STRING("Token_Float");        break;
		default:                 return STRING("NOT_A_TOKEN");        break;
	}
}
