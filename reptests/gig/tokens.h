#define TOKEN_KIND__BLOCK(N) ((N) << 5)
#define TOKEN_KIND__BINARY_ASS_BIT (TOKEN_KIND__BLOCK(8))

typedef enum Token_Kind
{
	Token_Invalid = 0,
	Token_EOF,

	Token_At           = '@',
	Token_Pound        = '#',
	Token_Cash         = '$',
	Token_QMark        = '?',
	Token_Colon        = ':',
	Token_Comma        = ',',
	Token_Semicolon    = ';',
	Token_Hat          = '^',
	Token_OpenParen    = '(',
	Token_CloseParen   = ')',
	Token_OpenBracket  = '[',
	Token_CloseBracket = ']',
	Token_OpenBrace    = '{',
	Token_CloseBrace   = '}',

	Token__FirstBinary = TOKEN_KIND__BLOCK(3),
	Token__FirstMulLevel = Token__FirstBinary,
	Token_Star = Token__FirstMulLevel,               // *
	Token_Slash,                                     // /
	Token_Percent,                                   // %
	Token_And,                                       // &
	Token_LeLe,                                      // <<
	Token_GeGe,                                      // >>
	Token_GeGeGe,                                    // >>>
	Token__PastLastMulLevel,

	Token__FirstAddLevel = TOKEN_KIND__BLOCK(4),
	Token_Plus = Token__FirstAddLevel,               // +
	Token_Minus,                                     // -
	Token_Or,                                        // |
	Token_Tilde,                                     // ~
	Token__PastLastAddLevel,

	Token__FirstCmpLevel = TOKEN_KIND__BLOCK(5),
	Token_EqEq = Token__FirstCmpLevel,               // ==
	Token_BangEq,                                    // !=
	Token_Le,                                        // <
	Token_LeEq,                                      // <=
	Token_Ge,                                        // >
	Token_GeEq,                                      // >=
	Token__PastLastCmpLevel,

	Token__FirstLAndLevel = TOKEN_KIND__BLOCK(6),
	Token_AndAnd = Token__FirstLAndLevel,            // &&
	Token__PastLastLAndLevel,

	Token__FirstLOrLevel = TOKEN_KIND__BLOCK(7),
	Token_OrOr = Token__FirstLOrLevel,               // ||
	Token__PastLastLOrLevel,
	Token__PastLastBinary = Token__PastLastLOrLevel,

	Token__FirstBinaryAssignment = TOKEN_KIND__BLOCK(8 + 3),
	Token__FirstMulLevelAssigment = Token__FirstBinaryAssignment,
	Token_StarEq = Token__FirstMulLevelAssigment,    // *=
	Token_SlashEq,                                   // /=
	Token_PercentEq,                                 // %=
	Token_AndEq,                                     // &=
	Token_LeLeEq,                                    // <<=
	Token_GeGeEq,                                    // >>=
	Token_GeGeGeEq,                                  // >>>=
	Token__PastLastMulLevelAssigment,

	Token__FirstAddLevelAssigment = TOKEN_KIND__BLOCK(8 + 4),
	Token_PlusEq = Token__FirstAddLevelAssigment,    // +=
	Token_MinusEq,                                   // -=
	Token_OrEq,                                      // |=
	Token_TildeEq,                                   // ~=
	Token__PastLastAddLevelAssigment,

	Token__FirstLAndLevelAssigment = TOKEN_KIND__BLOCK(8 + 6),
	Token_AndAndEq = Token__FirstLAndLevelAssigment, // &&=
	Token__PastLastLAndLevelAssigment,

	Token__FirstLOrLevelAssigment = TOKEN_KIND__BLOCK(8 + 7),
	Token_OrOrEq = Token__FirstLOrLevelAssigment,    // ||=
	Token__PastLastLOrLevelAssigment,

	Token_Eq = TOKEN_KIND__BLOCK(8 + 8),             // =
	Token__PastLastBinaryAssignment,

	Token_Bang,                                      // !

	Token_Arrow,                                     // ->
	Token_Strike,                                    // ---

	Token_Dot,                                       // .
	Token_DotParen,                                  // .(
	Token_DotBracket,                                // .[
	Token_DotBrace,                                  // .{

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

String Keyword_True     = MS_STRING("true");
String Keyword_False    = MS_STRING("false");
String Keyword_Proc     = MS_STRING("proc");
String Keyword_Struct   = MS_STRING("struct");
String Keyword_If       = MS_STRING("if");
String Keyword_Else     = MS_STRING("else");
String Keyword_While    = MS_STRING("while");
String Keyword_Break    = MS_STRING("break");
String Keyword_Continue = MS_STRING("continue");
String Keyword_Return   = MS_STRING("return");
