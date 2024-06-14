#define TOKEN_BLOCK_LOG_SIZE 5
#define TOKEN_BLOCK(N) ((N) << TOKEN_BLOCK_LOG_SIZE)
#define TOKEN_BLOCK_IDX(N) ((N) >> TOKEN_BLOCK_LOG_SIZE)
#define TOKEN_BLOCK_OFF(N) ((N) & ((1 << TOKEN_BLOCK_LOG_SIZE) - 1))
#define TOKEN_BINARY_TO_BINARYEQ(K) ((K) | TOKEN_BLOCK(8))
#define TOKEN_BINARYEQ_TO_BINARY(K) ((K) & ~TOKEN_BLOCK(8))

typedef enum Token_Kind
{
  Token_Invalid = 0,
  Token_EndOfFile,

  Token__FirstBinary = TOKEN_BLOCK(1),
  Token__FirstMulLevel = Token__FirstBinary,
  Token_Mul = Token__FirstMulLevel,
  Token_Div,
  Token_Rem,
  Token_Shl,
  Token_Shr,
  Token_Sar,
  Token_BitAnd,
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN_BLOCK(2),
  Token_Plus = Token__FirstAddLevel,
  Token_Minus,
  Token_Pipe,
  Token_Tilde,
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_BLOCK(3),
  Token_CmpEq = Token__FirstCmpLevel,
  Token_CmpNeq,
  Token_CmpLe,
  Token_CmpLeEq,
  Token_CmpGt,
  Token_CmpGtEq,
  Token__PastLastCmpLevel,

  Token__FirstAndLevel = TOKEN_BLOCK(4),
  Token_And = Token__FirstAndLevel,
  Token__PastLastAndLevel,

  Token__FirstOrLevel = TOKEN_BLOCK(5),
  Token_Or = Token__FirstOrLevel,
  Token__PastLastOrLevel,
  Token__PastLastBinary,

  Token__FirstBinaryEQ = TOKEN_BLOCK(9),
  Token__FirstMulEQLevel = Token__FirstBinaryEQ,
  Token_MulEQ = Token__FirstMulEQLevel,
  Token_DivEQ,
  Token_RemEQ,
  Token_ShlEQ,
  Token_ShrEQ,
  Token_SarEQ,
  Token_BitAndEQ,
  Token__PastLastMulEQLevel,

  Token__FirstAddEQLevel = TOKEN_BLOCK(10),
  Token_PlusEQ = Token__FirstAddEQLevel,
  Token_MinusEQ,
  Token_PipeEQ,
  Token_TildeEQ,
  Token__PastLastAddEQLevel,

  // NOTE: there are no cmp assignment operators

  Token__FirstAndEQLevel = TOKEN_BLOCK(12),
  Token_AndEQ = Token__FirstAndEQLevel,
  Token__PastLastAndEQLevel,

  Token__FirstOrEQLevel = TOKEN_BLOCK(13),
  Token_OrEQ = Token__FirstOrEQLevel,
  Token__PastLastOrEQLevel,
  Token__PastLastBinaryEQ,

  Token_Equals,
  Token_Cash,
  Token_At,
  Token_Hat,
  Token_Dot,
  Token_Elipsis,
  Token_DotParen,
  Token_DotBracket,
  Token_DotBrace,

  Token_OpenParen,
  Token_CloseParen,
  Token_OpenBracket,
  Token_CloseBracket,
  Token_OpenBrace,
  Token_CloseBrace,

  Token_String,
  Token_Int,
  Token_Float,
  Token_Bool,

  Token_Ident,
  Token__FirstKeyword,
  Token_If = Token__FirstKeyword,
  Token_Else,
  Token_While,
  Token_Return,
  Token_True,
  Token_False,
  Token_Proc,
  Token_Struct,
  Token__PastLastKeyword,

} Token_Kind;

typedef struct Text_Pos
{
  u32 global_offset;
} Text_Pos;

typedef struct Text_Range
{
  union { struct Text_Pos; Text_Pos pos; };
  u32 len;
} Text_Range;

typedef struct Token
{
  Token_Kind kind;
  union { struct Text_Range; Text_Range text_range; };
} Token;

typedef struct Lexer
{
  u8* input;
  u8* cur;
  Text_Pos start;
} Lexer;

// NOTE: input must be terminated with 4 zeros
Lexer
Lexer_Init(u8* input, Text_Pos start)
{
  return (Lexer){
    .input = input,
    .cur   = input,
    .start = start,
  };
}

Token
Lexer_NextToken(Lexer* lexer)
{
  Token token = { .kind = Token_Invalid };

  for (;;)
  {
    while ((u8)(*lexer->cur-1) < (u8)(0x20)) ++lexer->cur;

    if (lexer->cur[0] == '/' && lexer->cur[1] == '/')
    {
      while (*lexer->cur != 0 && *lexer->cur != '\n') ++lexer->cur;
    }
    else if (lexer->cur[0] == '/' && lexer->cur[1] == '*')
    {
      lexer->cur += 2;

      umm nesting = 1;
      while (*lexer->cur != 0 && nesting > 0)
      {
        if (lexer->cur[0] == '/' && lexer->cur[1] == '*')
        {
          nesting    += 1;
          lexer->cur += 2;
        }
        else if (lexer->cur[0] == '*' && lexer->cur[1] == '/')
        {
          nesting    -= 1;
          lexer->cur += 2;
        }
        else ++lexer->cur;
      }

      if (nesting != 0)
      {
        //// ERROR: Unterminated block comment
        NOT_IMPLEMENTED;
        break;
      }
    }
    else break;
  }

  u8* token_start = lexer->cur;
  token.pos = { .global_offset = lexer->start.global_offset + (u32)(token_start - lexer->input) };
  // NOTE: token.len is set right before return
  
  u8 c = *lexer->cur;
  if (c != 0) ++lexer->cur;

  switch (c)
  {
    case 0: token.kind = Token_EndOfFile; break;
  }

  token.len = (u32)(lexer->cur - token_start);

  return token;
}
