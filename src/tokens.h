#define TOKEN_KIND__BLOCK_SIZE_LG2 6
#define TOKEN_KIND__BLOCK(N) ((N) << TOKEN_KIND__BLOCK_SIZE_LG2)
#define TOKEN_KIND__BLOCK_IDX(N) ((N) >> TOKEN_KIND__BLOCK_SIZE_LG2)
#define TOKEN_KIND__BLOCK_OFFSET(N) ((N) & ~(TOKEN_KIND__BLOCK_SIZE_LG2-1))
#define TOKEN_KIND__ASS_BIT TOKEN_KIND__BLOCK(8)

#define TOKEN_KIND__IS_BINARY(K) ((u32)((K) - Token__FirstBinary) <= (u32)(Token__PastLastBinary - Token__FirstBinary))
#define TOKEN_KIND__IS_BINARY_ASSIGNMENT(K) ((u32)((K) - Token__FirstAssignment) <= (u32)(Token__PastLastAssignment - Token__FirstAssignment))

// NOTE: String interning depends on Token_Ident, Token_Blank and all keywords having a value that fits in 16 bits (or in 16 bits from a constant offset)
typedef enum Token_Kind
{
  Token_Invalid = 0,
  Token_EOF,

  Token__FirstBinary = TOKEN_KIND__BLOCK(3),
  Token__FirstMulLevel = Token__FirstBinary,
  Token_Mul = Token__FirstMulLevel,                       // *
  Token_Div,                                              // /
  Token_Rem,                                              // %
  Token_And,                                              // &
  Token_Shl,                                              // <<
  Token_Shr,                                              // >>
  Token_Sar,                                              // >>>
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN_KIND__BLOCK(4),
  Token_Add = Token__FirstAddLevel,                       // +
  Token_Sub,                                              // -
  Token_Or,                                               // |
  Token_Xor,                                              // ~
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_KIND__BLOCK(5),
  Token_EQEQ = Token__FirstCmpLevel,                      // ==
  Token_LNotEQ,                                           // !=
  Token_Le,                                               // <
  Token_LeEQ,                                             // <=
  Token_Ge,                                               // >
  Token_GeEQ,                                             // >=
  Token__PastLastCmpLevel,

  Token__FirstLAndLevel = TOKEN_KIND__BLOCK(6),
  Token_LAnd = Token__FirstLAndLevel,                     // &&
  Token__PastLastLAndLevel,

  Token__FirstLOrLevel = TOKEN_KIND__BLOCK(7),
  Token_LOr = Token__FirstLOrLevel,                       // ||
  Token__PastLastLOrLevel,
  Token__PastLastBinary = Token__PastLastLOrLevel,

  Token__FirstAssignment = TOKEN_KIND__BLOCK(8 + 3),
  Token__FirstMulLevelAssignment = Token__FirstAssignment,
  Token_MulEQ = Token__FirstMulLevelAssignment,           // *=
  Token_DivEQ,                                            // /=
  Token_RemEQ,                                            // %=
  Token_AndEQ,                                            // &=
  Token_ShlEQ,                                            // <<=
  Token_ShrEQ,                                            // >>=
  Token_SarEQ,                                            // >>>=
  Token__PastLastMulLevelAssignment,

  Token__FirstAddLevelAssignment = TOKEN_KIND__BLOCK(8 + 4),
  Token_AddEQ = Token__FirstAddLevelAssignment,           // +=
  Token_SubEQ,                                            // -=
  Token_OrEQ,                                             // |=
  Token_XorEQ,                                            // ~=
  Token__PastLastAddLevelAssignment,

  // NOTE: 8 + 6 is not used since there are no cmp assignment expressions

  Token__FirstLAndLevelAssignment = TOKEN_KIND__BLOCK(8 + 6),
  Token_LAndEQ = Token__FirstLAndLevelAssignment,         // &&=
  Token__PastLastLAndLevelAssignment,

  Token__FirstLOrLevelAssignment = TOKEN_KIND__BLOCK(8 + 7),
  Token_LOrEQ = Token__FirstLOrLevelAssignment,           // ||=
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
  Token_Char,
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
