#define TOKEN_BLOCK(IDX) ((IDX) << 4)

typedef enum Token_Kind
{
	Token_Invalid = 0,

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
	Token_Not,
	Token_Equals,
  Token_Qmark,
  
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
  Token_Pipe,
  Token_Tilde,
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_BLOCK(4),
  Token_EqualsEQ,
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

	Token__FirstAssignment = Token_BLOCK(10),
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
  Token_PipeEQ,
	Token_TildeEQ,
  Token__PastLastAddLevelAssignment,

  Token__FirstAndLevelAssignment = TOKEN_BLOCK(13),
  Token_AndAndEQ = Token__FirstAndLevelAssignment,
  Token__PastLastAndLevelAssignment,

  Token__FirstOrLevelAssignment = TOKEN_BLOCK(14),
  Token_OrOrEQ = Token__FirstOrLevelAssignment,
  Token__PastLastOrLevelAssignment,
	Token__PastLastAssignment = Token__PastLastOrLevelAssignment,
} Token_Kind;

STATIC_ASSERT(Token__PastLastMulLevel           <= Token__FirstAddLevel);
STATIC_ASSERT(Token__PastLastAddLevel           <= Token__FirstCmpLevel);
STATIC_ASSERT(Token__PastLastCmpLevel           <= Token__FirstOrLevel);
STATIC_ASSERT(Token__PastLastOrLevel            <= Token__FirstAndLevel);
STATIC_ASSERT(Token__PastLastOrLevel            <= Token__FirstAssignment);
STATIC_ASSERT(Token__PastLastMulLevelAssignment <= Token__FirstAddLevelAssignment);
STATIC_ASSERT(Token__PastLastAddLevelAssignment <= Token__FirstCmpLevelAssignment);
STATIC_ASSERT(Token__PastLastCmpLevelAssignment <= Token__FirstOrLevelAssignment);
STATIC_ASSERT(Token__PastLastOrLevelAssignment  <= Token__FirstAndLevelAssignment);

typedef struct Token
{
	Token_Kind kind;
} Token;
