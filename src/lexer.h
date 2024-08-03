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
  Arena* string_arena;
  u8* input;
  u8* cur;
  Text_Pos start;
} Lexer;

// NOTE: input must be terminated with 4 zeros
static Lexer
Lexer_Init(Arena* string_arena, u8* input, Text_Pos start)
{
  return (Lexer){
    .string_arena = string_arena,
    .input        = input,
    .cur          = input,
    .start        = start,
  };
}

static Token
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
        bool is_hex_float = false;
        umm base          = 10;

        if (c == 0)
        {
          if      (*lexer->cur == 'x') base = 16;
          else if (*lexer->cur == 'h') base = 16, is_hex_float = true;
          else if (*lexer->cur == 'b') base = 2;

          if (base != 10) ++lexer->cur;
        }

        umm value       = c&0xF;
        umm digit_count = (base == 10);

        for (;;)
        {
          while (*lexer->cur == '_') ++lexer->cur;

          u8 digit;
          if      (Char_IsDigit(*lexer->cur))         digit = *lexer->cur&0xF;
          else if (Char_IsHexAlphaDigit(*lexer->cur)) digit = 9 + (*lexer->cur&0x7);
          else break;

          if (digit >= base)
          {
            //// ERROR: Invalid digit in literal
            NOT_IMPLEMENTED;
          }
          else
          {
            umm old_val = value;

            value *= base;

            if (value < old_val || value + digit < old_val)
            {
              //// ERROR: Literal too large
              NOT_IMPLEMENTED;
            }
            else
            {
              value += digit;
              
              ++digit_count;
              ++lexer->cur;
            }
          }
        }

        if (digit_count == 0)
        {
          //// ERROR: Missing digits
          NOT_IMPLEMENTED;
        }
        else
        {
          if (is_hex_float)
          {
            if (digit_count == 8)
            {
              token.kind     = Token_Float;
              token.floating = (F32_Bits){ .bits = (u32)value }.f;
            }
            else if (digit_count == 16)
            {
              token.kind     = Token_Float;
              token.floating = (F64_Bits){ .bits = value }.f;
            }
            else
            {
              //// ERROR: Invalid digit count in hex float
              NOT_IMPLEMENTED;
            }
          }
          else
          {
            if (*lexer->cur != '.')
            {
              token.kind    = Token_Int;
              token.integer = value;
            }
            else
            {
              // TODO: float parsing
              NOT_IMPLEMENTED;
            }
          }
        }
      }
      else if (c == '"')
      {
        u8 string_start = Arena_GetPointer(lexer->arena);

        while (*lexer->cur != 0 && *lexer->cur != '"')
        {
          u8 chars[4]   = {0};
          umm chars_len = 1;
          
          if (*lexer->cur != '\\')
          {
            chars[0] = *lexer->cur;

            ++lexer->cur;
          }
          else
          {
            ++lexer->cur;

            if (*lexer->cur == 0)
            {
              //// ERROR: Missing escape sequence after backslash
              NOT_IMPLEMENTED;
            }
            else if (Char_IsDigit(*lexer->cur))
            {
              umm value = 0;
              for (umm i = 0; i < 3; ++i)
              {
                if ((u8)(*lexer->cur-'0') < (u8)8)
                {
                  value <<= 3;
                  value  |= *lexer->cur&0xF;

                  ++lexer->cur;
                }
                else
                {
                  //// ERROR: Missing digits
                  NOT_IMPLEMENTED;
                }
              }

              chars[0] = (u8)value;

              if (value > 255)
              {
                //// ERROR: Out of range
                NOT_IMPLEMENTED;
              }
            }
            else if (Char_ToUpperUnconditional(*lexer->cur) == 'U' || *lexer->cur == 'x')
            {
              umm digit_count = 6;
              if (*lexer->cur == 'u') digit_count = 4;
              if (*lexer->cur == 'x') digit_count = 2;
              ++lexer->cur;

              umm value = 0;

              for (umm i = 0; i < digit_count; ++i)
              {
                u8 digit;
                if      (Char_IsDigit(*lexer->cur))         digit = *lexer->cur&0xF;
                else if (Char_IsHexAlphaDigit(*lexer->cur)) digit = 9 + (*lexer->cur&0x7);
                else
                {
                  //// ERROR: Missing digits in escape sequence
                  NOT_IMPLEMENTED;
                }
                else
                {
                  value <<= 4;
                  value  |= digit;

                  ++lexer->cur;
                }
              }

              if (digit_count == 2) chars[0] = (u8)value; // NOTE: no translation for \x
              else
              {
                if (value <= 0x7F)
                {
                  chars[0] = (u8)value;
                }
                else if (value <= 0x7FF)
                {
                  chars[0] = 0xC0 | (u8)(value >> 6);
                  chars[1] = 0x80 | (u8)value;
                }
                else if (value <= 0xFFFF)
                {
                  chars[0] = 0xE0 | (u8)(value >> 12);
                  chars[1] = 0x80 | (u8)(value >>  6);
                  chars[2] = 0x80 | (u8)value;
                }
                else if (value <= 0x10FFFF)
                {
                  chars[0] = 0xF0 | (u8)(value >> 18);
                  chars[1] = 0x80 | (u8)(value >> 12);
                  chars[2] = 0x80 | (u8)(value >> 6);
                  chars[3] = 0x80 | (u8)value;
                }
                else
                {
                  //// ERROR: Unicode codepoint out of range for utf8
                  NOT_IMPLEMENTED;
                }
              }
            }
            else
            {
              switch (*lexer->cur)
              {
                case 'a':  chars[0] = '\a'; break;
                case 'b':  chars[0] = '\b'; break;
                case 'f':  chars[0] = '\f'; break;
                case 'n':  chars[0] = '\n'; break;
                case 'r':  chars[0] = '\r'; break;
                case 't':  chars[0] = '\t'; break;
                case 'v':  chars[0] = '\v'; break;
                case '\\': chars[0] = '\\'; break;
                case '\'': chars[0] = '\''; break;
                case '"':  chars[0] = '"';  break;

                default:
                {
                  //// ERROR: Invalid escape sequence
                  NOT_IMPLEMENTED;
                } break;
              }

              ++lexer->cur;
            }
          }
          
          u8* dst = Arena_Push(lexer->string_arena, chars_len, 1);
          for (umm i = 0; i < chars_len; ++i) dst[i] = chars[i];
        }

        if (*lexer->cur != '"')
        {
          //// ERROR: Unterminated string literal
          NOT_IMPLEMENTED;
        }
        else
        {
          ++lexer->cur;

          String string = {
            .data = string_start,
            .len  = Arena_GetPointer(lexer->arena) - string_start
          };

          token.kind   = Token_String;
          token.string = string;
        }
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
