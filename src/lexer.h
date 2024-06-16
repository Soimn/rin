#define TOKEN_BLOCK_LOG_SIZE 5
#define TOKEN_BLOCK(N) ((N) << TOKEN_BLOCK_LOG_SIZE)
#define TOKEN_BLOCK_IDX(N) ((N) >> TOKEN_BLOCK_LOG_SIZE)
#define TOKEN_BLOCK_OFF(N) ((N) & ((1 << TOKEN_BLOCK_LOG_SIZE) - 1))
#define TOKEN_BINARYEQ_BIT (TOKEN_BLOCK_LOG_SIZE + 3)
#define TOKEN_BINARY_TO_BINARYEQ(K) ((K) |  ((u32)1 << TOKEN_BINARYEQ_BIT))
#define TOKEN_BINARYEQ_TO_BINARY(K) ((K) & ~((u32)1 << TOKEN_BINARYEQ_BIT))

typedef enum Token_Kind
{
  Token_Invalid = 0,
  Token_EndOfFile,

  Token__FirstBinary = TOKEN_BLOCK(1),
  Token__FirstMulLevel = Token__FirstBinary,
  Token_Mul = Token__FirstMulLevel,            // *
  Token_Div,                                   // /
  Token_Rem,                                   // %
  Token_BitAnd,                                // &
  Token_Shl,                                   // <<
  Token_Shr,                                   // >>
  Token_Sar,                                   // >>>
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN_BLOCK(2),
  Token_Plus = Token__FirstAddLevel,           // +
  Token_Minus,                                 // -
  Token_BitOr,                                 // |
  Token_Tilde,                                 // ~
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN_BLOCK(3),
  Token_CmpEq = Token__FirstCmpLevel,          // ==
  Token_CmpNeq,                                // !=
  Token_CmpLe,                                 // <
  Token_CmpLeEq,                               // <=
  Token_CmpGt,                                 // >
  Token_CmpGtEq,                               // >=
  Token__PastLastCmpLevel,

  Token__FirstAndLevel = TOKEN_BLOCK(4),
  Token_And = Token__FirstAndLevel,            // &&
  Token__PastLastAndLevel,

  Token__FirstOrLevel = TOKEN_BLOCK(5),
  Token_Or = Token__FirstOrLevel,              // ||
  Token__PastLastOrLevel,
  Token__PastLastBinary,

  Token__FirstBinaryEQ = TOKEN_BLOCK(9),
  Token__FirstMulEQLevel = Token__FirstBinaryEQ,
  Token_MulEQ = Token__FirstMulEQLevel,        // *=
  Token_DivEQ,                                 // /=
  Token_RemEQ,                                 // %=
  Token_ShlEQ,                                 // <<=
  Token_ShrEQ,                                 // >>=
  Token_SarEQ,                                 // >>>=
  Token_BitAndEQ,                              // &=
  Token__PastLastMulEQLevel,

  Token__FirstAddEQLevel = TOKEN_BLOCK(10),
  Token_PlusEQ = Token__FirstAddEQLevel,       // +=
  Token_MinusEQ,                               // -=
  Token_BitOrEQ,                               // |=
  Token_TildeEQ,                               // ~=
  Token__PastLastAddEQLevel,

  // NOTE: there are no cmp assignment operators

  Token__FirstAndEQLevel = TOKEN_BLOCK(12),
  Token_AndEQ = Token__FirstAndEQLevel,        // &&=
  Token__PastLastAndEQLevel,

  Token__FirstOrEQLevel = TOKEN_BLOCK(13),
  Token_OrEQ = Token__FirstOrEQLevel,          // ||=
  Token__PastLastOrEQLevel,
  Token__PastLastBinaryEQ,

  Token_Bang,         // !
  Token_PlusPlus,     // ++
  Token_DoubleMinus,  // --
  Token_TripleMinus,  // ---
  Token_Arrow,        // ->
  Token_Equals,       // =
  Token_Cash,         // $
  Token_At,           // @
  Token_Hat,          // ^
  Token_Blank,        // _
  Token_Dot,          // .
  Token_Colon,        // :
  Token_ColonColon,   // ::
  Token_Comma,        // ,
  Token_Semicolon,    // ;
  Token_Elipsis,      // ..
  Token_DotParen,     // .(
  Token_DotBracket,   // .[
  Token_DotBrace,     // .{

  Token_OpenParen,    // (
  Token_CloseParen,   // )
  Token_OpenBracket,  // [
  Token_CloseBracket, // ]
  Token_OpenBrace,    // {
  Token_CloseBrace,   // }

  Token_String,
  Token_Int,
  Token_Float,

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

String KeywordStrings[Token__PastLastKeyword-Token__FirstKeyword] = {
  COMPSTRING("if"),
  COMPSTRING("else"),
  COMPSTRING("while"),
  COMPSTRING("return"),
  COMPSTRING("true"),
  COMPSTRING("false"),
  COMPSTRING("proc"),
  COMPSTRING("struct"),
};

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
  
  union
  {
    String string;
    s64 integer;
    f64 floating;
  };
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
  token.pos = (Text_Pos){ .global_offset = lexer->start.global_offset + (u32)(token_start - lexer->input) };
  // NOTE: token.len is set right before return
  
  u8 c = *lexer->cur;
  if (c != 0) ++lexer->cur;

  bool c1_eq    = (lexer->cur[0] == '=');
  u32 c1_eq_bit = (u32)c1_eq << TOKEN_BINARYEQ_BIT;

  bool c2_eq    = (lexer->cur[1] == '=');
  u32 c2_eq_bit = (u32)c2_eq << TOKEN_BINARYEQ_BIT;

  switch (c)
  {
    case 0: token.kind = Token_EndOfFile; break;

    case '$': token.kind = Token_Cash;         break;
    case '@': token.kind = Token_At;           break;
    case '^': token.kind = Token_Hat;          break;
    case ',': token.kind = Token_Comma;        break;
    case ';': token.kind = Token_Semicolon;    break;
    case '(': token.kind = Token_OpenParen;    break;
    case ')': token.kind = Token_CloseParen;   break;
    case '[': token.kind = Token_OpenBracket;  break;
    case ']': token.kind = Token_CloseBracket; break;
    case '{': token.kind = Token_OpenBrace;    break;
    case '}': token.kind = Token_CloseBrace;   break;

    case '*': token.kind = Token_Mul   | c1_eq_bit, lexer->cur += c1_eq; break;
    case '/': token.kind = Token_Div   | c1_eq_bit; lexer->cur += c1_eq; break;
    case '%': token.kind = Token_Rem   | c1_eq_bit; lexer->cur += c1_eq; break;
    case '~': token.kind = Token_Tilde | c1_eq_bit; lexer->cur += c1_eq; break;

    case '.':
    {
      token.kind = Token_Dot;
      if (*lexer->cur == '.') token.kind = Token_Elipsis;
      if (*lexer->cur == '(') token.kind = Token_DotParen;
      if (*lexer->cur == '[') token.kind = Token_DotBracket;
      if (*lexer->cur == '{') token.kind = Token_DotBrace;

      lexer->cur += (token.kind != Token_Dot);
    } break;

    case ':':
    {
      token.kind = Token_Colon;
      if (*lexer->cur == ':')
      {
        token.kind = Token_ColonColon;
        ++lexer->cur;
      }
    } break;

    case '=':
    {
      token.kind = (c1_eq ? Token_CmpEq : Token_Equals);
      lexer->cur += c1_eq;
    } break;

    case '!':
    {
      token.kind = (c1_eq ? Token_CmpNeq : Token_Bang);
      lexer->cur += c1_eq;
    } break;

    case '&':
    {
      if (*lexer->cur == '&')
      {
        token.kind = Token_And | c2_eq_bit;
        lexer->cur += 1 + c2_eq;
      }
      else
      {
        token.kind = Token_BitAnd | c1_eq_bit;
        lexer->cur += c1_eq;
      }
    } break;

    case '|':
    {
      if (*lexer->cur == '|')
      {
        token.kind = Token_Or | c2_eq_bit;
        lexer->cur += 1 + c2_eq;
      }
      else
      {
        token.kind = Token_BitOr | c1_eq_bit;
        lexer->cur += c1_eq;
      }
    } break;

    case '<':
    {
      if (*lexer->cur == '<')
      {
        token.kind = Token_Shl | c2_eq_bit;
        lexer->cur += 1 + c2_eq;
      }
      else
      {
        token.kind = Token_CmpLe + c1_eq;
        lexer->cur += c1_eq;
      }
    } break;

    case '>':
    {
      if (lexer->cur[0] == '>')
      {
        if (lexer->cur[1] == '>')
        {
          bool c3_eq = (lexer->cur[2] == '=');
          token.kind = Token_Sar | ((u32)c3_eq << TOKEN_BINARYEQ_BIT);
          lexer->cur += 2 + c3_eq;
        }
        else
        {
          token.kind = Token_Shr | c2_eq_bit;
          lexer->cur += 1 + c2_eq;
        }
      }
      else
      {
        token.kind = Token_CmpGt + c1_eq;
        lexer->cur += c1_eq;
      }
    } break;

    case '+':
    {
      if (*lexer->cur == '+')
      {
        token.kind = Token_PlusPlus;
        ++lexer->cur;
      }
      else
      {
        token.kind = Token_Plus | c1_eq_bit;
        lexer->cur += c1_eq;
      }
    }

    case '-':
    {
      if (*lexer->cur == '-')
      {
        token.kind = Token_DoubleMinus;
        lexer->cur += 1;

        if (*lexer->cur == '-')
        {
          token.kind = Token_TripleMinus;
          lexer->cur += 1;
        }
      }
      else if (*lexer->cur == '>')
      {
        token.kind = Token_Arrow;
        lexer->cur += 1;
      }
      else
      {
        token.kind = Token_Minus | c1_eq_bit;
        lexer->cur += c1_eq;
      }
    } break;
  
    default:
    {
      if (Char_IsAlpha(c) || c == '_')
      {
        String ident = { .data = lexer->cur-1 };

        while (Char_IsAlpha(*lexer->cur) || Char_IsDigit(*lexer->cur) || *lexer->cur == '_') ++lexer->cur;

        ident.len = lexer->cur - ident.data;

        if (ident.len == 1 && *ident.data == '_') token.kind = Token_Blank;
        else
        {
          // TODO: replace with perfect hash
          umm i = 0;
          for (; i < ARRAY_SIZE(KeywordStrings); ++i)
          {
            if (String_Match(ident, KeywordStrings[i])) break;
          }

          if (i == ARRAY_SIZE(KeywordStrings)) token.kind = Token_Ident;
          else                                 token.kind = Token__FirstKeyword + i;

          token.string = ident;
        }
      }
      else if (Char_IsDigit(c))
      {
        bool is_float = false;
        umm base      = 10;

        if (c == 0)
        {
          if      (*lexer->cur == 'x') base = 16;
          else if (*lexer->cur == 'h') base = 16, is_float = true;
          else if (*lexer->cur == 'b') base = 2;
        }

        if (base == 16)
        {
          lexer->cur += 1;
          
          umm digit_count = 0;
          u64 value = 0;

          for (;;)
          {
            while (*lexer->cur == '_') ++lexer->cur;

            u8 digit;

            if      (Char_IsDigit(*lexer->cur))         digit = *lexer->cur&0xF;
            else if (Char_IsHexAlphaDigit(*lexer->cur)) digit = (*lexer->cur&0x7) + 9;
            else                                        break;

            value <<= 4;
            value  |= digit;

            ++digit_count;
            ++lexer->cur;
          }

          if (is_float)
          {
            if (digit_count == 16)
            {
              token.kind     = Token_Float;
              token.floating = (F64_Bits){ .bits = value }.f;
            }
            else if (digit_count == 8)
            {
              token.kind     = Token_Float;
              token.floating = (F32_Bits){ .bits = (u32)value }.f;
            }
            else
            {
              //// ERROR: Illegal length of hex float literal. Must be 8 or 16
              NOT_IMPLEMENTED;
            }
          }
          else
          {
            if (digit_count == 0)
            {
              //// ERROR: Missing digits in hex literal
              NOT_IMPLEMENTED;
            }
            else if (digit_count > 16)
            {
              //// ERROR: Hex literal is too large
              NOT_IMPLEMENTED;
            }
            else
            {
              token.kind    = Token_Int;
              token.integer = value;
            }
          }
        }
        else if (base == 2)
        {
          lexer->cur += 1;

          umm digit_count = 0;
          u64 value = 0;

          for (;;)
          {
            while (*lexer->cur == '_') ++lexer->cur;

            if (*lexer->cur == '0' || *lexer->cur == '1')
            {
              value <<= 1;
              value  |= (*lexer->cur == '1');

              ++digit_count;
              ++lexer->cur;
            }
            else if (Char_IsHexDigit(*lexer->cur))
            {
              //// ERROR: Invalid digit in binary literal
              NOT_IMPLEMENTED;
            }
            else break;
          }

          if (digit_count == 0)
          {
            //// ERROR: Missing digits
            NOT_IMPLEMENTED;
          }
          else if (digit_count > 64)
          {
            //// ERROR: Binary literal is too large
            NOT_IMPLEMENTED;
          }
          else
          {
            token.kind    = Token_Int;
            token.integer = value;
          }
        }
        else
        {
          umm digit_count = 1;
          s64 value = c&0xF;

          bool has_overflown = false;

          for (;;)
          {
            while (*lexer->cur == '_') ++lexer->cur;

            if (Char_IsDigit(*lexer->cur))
            {
              s64 old_val = value;

              value *= 10;
              value += *lexer->cur&0xF;

              // TODO: verify this
              has_overflown = (has_overflown || value/10 != old_val);

              ++digit_count;
              ++lexer->cur;
            }
            else if (Char_IsHexAlphaDigit(*lexer->cur))
            {
              //// ERROR: Invalid digit in decimal literal
              NOT_IMPLEMENTED;
            }
            else break;
          }

          if (*lexer->cur == '.')
          {
            NOT_IMPLEMENTED;
          }
          else
          {
            if (digit_count == 0)
            {
              //// ERROR: Missing digits in decimal literal
              NOT_IMPLEMENTED;
            }
            else if (has_overflown)
            {
              //// ERROR: Decimal literal is too large
              NOT_IMPLEMENTED;
            }
            else
            {
              token.kind    = Token_Int;
              token.integer = value;
            }
          }
        }
      }
      else if (c == '"')
      {
        String raw_string = { .data = lexer->cur };

        while (*lexer->cur != 0 && *lexer->cur != '"')
        {
          if (*lexer->cur == '\\' && *lexer->cur != 0) ++lexer->cur;
          ++lexer->cur;
        }

        raw_string.len = lexer->cur - raw_string.data;

        NOT_IMPLEMENTED;
      }
      else
      {
        //// ERROR: Unknown
        NOT_IMPLEMENTED;
      }
    } break;
  }

  token.len = (u32)(lexer->cur - token_start);

  return token;
}
