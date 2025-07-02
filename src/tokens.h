typedef enum Token_Kind
{
	Token_EOF = 0,

	Token_Bang         = '!',
	Token_Pound        = '#',
	Token_Cash         = '$',
	Token_Percent      = '%',
	Token_And          = '&',
	Token_OpenParen    = '(',
	Token_CloseParen   = ')',
	Token_Star         = '*',
	Token_Plus         = '+',
	Token_Comma        = ',',
	Token_Minus        = '-',
	Token_Dot          = '.',
	Token_Slash        = '/',
	Token_Colon        = ':',
	Token_Semicolon    = ';',
	Token_Lt           = '<',
	Token_Eq           = '=',
	Token_Gt           = '>',
	Token_QMark        = '?',
	Token_At           = '@',
	Token_OpenBracket  = '[',
	//Token_Backslash    = '\\', -- illegal
	Token_CloseBracket = ']',
	Token_Hat          = '^',
	//Token_Backtick     = '`',  -- illegal
	Token_OpenBrace    = '{',
	Token_Or           = '|',
	Token_CloseBrace   = '}',
	Token_Tilde        = '~',

	Token_BangEq       = '!' | 0x80,
	Token_PercentEq    = '%' | 0x80,
	Token_StarEq       = '*' | 0x80,
	Token_SlashEq      = '/' | 0x80,
	Token_TildeEq      = '~' | 0x80,

	Token_AndEq        = '&' | 0x80,
	Token_OrEq         = '|' | 0x80,
	Token_PlusEq       = '+' | 0x80,
	Token_MinusEq      = '-' | 0x80,

	Token_EqEq         = '=' | 0x180,

	Token_AndAnd       = '&' | 0x100,
	Token_OrOr         = '|' | 0x100,
	Token_PlusPlus     = '+' | 0x100,
	Token_MinusMinus   = '-' | 0x100,

	Token_LtEq         = '<' | 0x80,
	Token_GtEq         = '>' | 0x80,

	Token_LtLt         = '<' | 0x100,
	Token_GtGt         = '>' | 0x100,

	Token_LtLtEq       = '<' | 0x180,
	Token_GtGtEq       = '>' | 0x180,

	Token_Ident,
	Token_String,
	Token_Char,
	Token_Int,
	Token_Float,
} Token_Kind;

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
