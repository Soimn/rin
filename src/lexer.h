#define TOKEN_BLOCK(IDX) ((IDX) << 4)

typedef enum Token_Kind
{
	Token_Invalid = 0,

  Token_String,
  Token_Char,   // TODO
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
  Token_Arrow,
  Token_PlusPlus,   // TODO
  Token_MinusMinus, // TODO
  Token_TripleMinus,
  Token_Tilde,
  
  Token__FirstMulLevel = TOKEN_BLOCK(1),
  Token_Star = Token__FirstMulLevel,
  Token_Slash,
  Token_Mod,
  Token_And,
  Token_LeLe,
  Token_GeGe,
  Token_GeGeGe,
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN_BLOCK(2),
  Token_Plus = Token__FirstAddLevel,
  Token_Minus,
  Token_Pipe,
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_BLOCK(3),
  Token_EqualsEQ,
  Token_Le,
  Token_LeEQ,
  Token_Ge,
  Token_GeEQ,
  Token__PastLastCmpLevel,

  Token__FirstAndLevel = TOKEN_BLOCK(4),
  Token_AndAnd = Token__FirstAndLevel,
  Token__PastLastAndLevel,

  Token__FirstOrLevel = TOKEN_BLOCK(5),
  Token_OrOr = Token__FirstOrLevel,
  Token__PastLastOrLevel,

  Token_Qmark,
} Token_Kind;

STATIC_ASSERT(Token__PastLastMulLevel <= Token__FirstAddLevel);
STATIC_ASSERT(Token__PastLastAddLevel <= Token__FirstCmpLevel);
