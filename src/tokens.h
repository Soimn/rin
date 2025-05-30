#define TOKEN_KIND__BLOCK_SIZE_LG2 6
#define TOKEN_KIND__BLOCK(N) ((N) << TOKEN_KIND__BLOCK_SIZE_LG2)
#define TOKEN_KIND__BLOCK_IDX(N) ((N) >> TOKEN_KIND__BLOCK_SIZE_LG2)
#define TOKEN_KIND__ASS_BIT TOKEN_KIND__BLOCK(8)

#define TOKEN_KIND__IS_BINARY(K) ((u32)((K) - Token__FirstBinary) <= (u32)(Token__PastLastBinary - Token__FirstBinary))
#define TOKEN_KIND__IS_BINARY_ASSIGNMENT(K) ((u32)((K) - Token__FirstAssignment) <= (u32)(Token__PastLastAssignment - Token__FirstAssignment))

typedef enum Token_Kind
{
  Token_Invalid = 0,
  Token_EOF,

  Token_Mul = TOKEN_KIND__BLOCK(3),                       // *
  Token__FirstBinary = Token_Mul,
  Token__FirstMulLevel = Token__FirstBinary,
  Token_Div,                                              // /
  Token_Rem,                                              // %
  Token_And,                                              // &
  Token_Shl,                                              // <<
  Token_Shr,                                              // >>
  Token_Sar,                                              // >>>
  Token__PastLastMulLevel,

  Token_Add = TOKEN_KIND__BLOCK(4),                       // +
  Token__FirstAddLevel = Token_Add,
  Token_Sub,                                              // -
  Token_Or,                                               // |
  Token_Xor,                                              // ~
  Token__PastLastAddLevel,

  Token_EQEQ = TOKEN_KIND__BLOCK(5),                      // ==
  Token__FirstCmpLevel = Token_EQEQ,
  Token_LNotEQ,                                           // !=
  Token_Le,                                               // <
  Token_LeEQ,                                             // <=
  Token_Ge,                                               // >
  Token_GeEQ,                                             // >=
  Token__PastLastCmpLevel,

  Token_LAnd = TOKEN_KIND__BLOCK(6),                      // &&
  Token__FirstLAndLevel = Token_LAnd,
  Token__PastLastLAndLevel,

  Token_LOr = TOKEN_KIND__BLOCK(7),                       // ||
  Token__FirstLOrLevel = Token_LOr,
  Token__PastLastLOrLevel,
  Token__PastLastBinary = Token__PastLastLOrLevel,

  Token_MulEQ = TOKEN_KIND__BLOCK(8 + 3),                 // *=
  Token__FirstAssignment = Token_MulEQ,
  Token__FirstMulLevelAssignment = Token__FirstAssignment,
  Token_DivEQ,                                            // /=
  Token_RemEQ,                                            // %=
  Token_AndEQ,                                            // &=
  Token_ShlEQ,                                            // <<=
  Token_ShrEQ,                                            // >>=
  Token_SarEQ,                                            // >>>=
  Token__PastLastMulLevelAssignment,

  Token_AddEQ = TOKEN_KIND__BLOCK(8 + 4),                 // +=
  Token__FirstAddLevelAssignment = Token_AddEQ,
  Token_SubEQ,                                            // -=
  Token_OrEQ,                                             // |=
  Token_XorEQ,                                            // ~=
  Token__PastLastAddLevelAssignment,

  // NOTE: 8 + 6 is not used since there are no cmp assignment expressions

  Token_LAndEQ = TOKEN_KIND__BLOCK(8 + 6),                // &&=
  Token__FirstLAndLevelAssignment = Token_LAndEQ,
  Token__PastLastLAndLevelAssignment,

  Token_LOrEQ = TOKEN_KIND__BLOCK(8 + 7),                 // ||=
  Token__FirstLOrLevelAssignment = Token_LOrEQ,
  Token__PastLastLOrLevelAssignment,

  Token_EQ,                                               // =
  Token__PastLastAssignment,

  Token_LNot,                                             // !
  Token_Cash,                                             // $
  Token_QMark,                                            // ?
  Token_Colon,                                            // :
  Token_Comma,                                            // ,
  Token_Semicolon,                                        // ;
  Token_Hat,                                              // ^
  Token_OpenParen,                                        // (
  Token_CloseParen,                                       // )
  Token_OpenBracket,                                      // [
  Token_CloseBracket,                                     // ]
  Token_OpenBrace,                                        // {
  Token_CloseBrace,                                       // }

  Token_Arrow,                                            // ->
	Token_Dec,                                              // --
	Token_StrikeOut,                                        // ---

	Token_Inc,                                              // ++

  Token_Dot,                                              // .
  Token_DotParen,                                         // .(
  Token_DotBracket,                                       // .[
  Token_DotBrace,                                         // .{

  Token__FirstKeyword,
  Token_True = Token__FirstKeyword,
  Token_False,
  Token_If,
  Token_Else,
  Token_While,
  Token_Return,
  Token_Proc,
  Token_Struct,
  Token_Break,
  Token_Continue,
  Token__PastLastKeyword,

  Token_Blank,                                            // _
  Token_Ident,
  Token_String,
  Token_Int,
  Token_Float,
} Token_Kind;

typedef struct Token
{
  Token_Kind kind;
  u32 offset;
  u32 len;
  u32 line;
  u32 col;

  union
  {
    u64 integer;
    f64 floating;
    Ident ident;
  };
} Token;

String Token_KeywordStrings[Token__PastLastKeyword - Token__FirstKeyword] = {
  [Token_True     - Token__FirstKeyword] = MS_STRING("true"),
  [Token_False    - Token__FirstKeyword] = MS_STRING("false"),
  [Token_If       - Token__FirstKeyword] = MS_STRING("if"),
  [Token_Else     - Token__FirstKeyword] = MS_STRING("else"),
  [Token_While    - Token__FirstKeyword] = MS_STRING("while"),
  [Token_Return   - Token__FirstKeyword] = MS_STRING("return"),
  [Token_Proc     - Token__FirstKeyword] = MS_STRING("proc"),
  [Token_Struct   - Token__FirstKeyword] = MS_STRING("struct"),
  [Token_Break    - Token__FirstKeyword] = MS_STRING("break"),
  [Token_Continue - Token__FirstKeyword] = MS_STRING("continue"),
};

Ident Token_KeywordIdents[Token__PastLastKeyword - Token__FirstKeyword] = {0};
