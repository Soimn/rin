typedef u32 Text_Pos;

#define TOKEN__BLOCK(N) ((N) << 4)

typedef enum Token_Kind
{
  Token_Invalid     = 0,
  Token_EndOfStream = 1,

  Token_OpenParen,
  Token_CloseParen,
  Token_OpenBracket,
  Token_CloseBracket,
  Token_OpenBrace,
  Token_CloseBrace,
  Token_Qmark,
  Token_Colon,
  Token_Comma,
  Token_Semicolon,
  Token_Hat,
  Token_Pound,
  Token_Cash,
  Token_At,

  Token_Period,
  Token_Elipsis,
  Token_PeriodParen,
  Token_PeriodBracket,
  Token_PeriodBrace,
  Token_TripleMinus,
  Token_Arrow,

  Token_Underscore,

  Token_Ident,
  Token_String,
  Token_Int,
  Token_Float,

  Token_Not,
  Token_Equals,

  Token__FirstBinaryOp = TOKEN__BLOCK(9),
  Token__FirstMulLevel = Token__FirstBinaryOp,
  Token_Star = Token__FirstMulLevel,
  Token_Slash,
  Token_Mod,
  Token_And,
  Token_LeLe,
  Token_GeGe,
  Token_GeGeGe,
  Token__PastLastMulLevel,

  Token__FirstAddLevel = TOKEN__BLOCK(10),
  Token_Plus = Token__FirstAddLevel,
  Token_Minus,
  Token_Or,
  Token_Tilde,
  Token__PastLastAddLevel,

  Token__FirstCmpLevel = TOKEN__BLOCK(11),
  Token_EqualsEQ = Token__FirstCmpLevel,
  Token_NotEQ,
  Token_Le,
  Token_LeEQ,
  Token_Ge,
  Token_GeEQ,
  Token__PastLastCmpLevel,

  Token__FirstAndLevel = TOKEN__BLOCK(12),
  Token_AndAnd = Token__FirstAndLevel,
  Token__PastAndLevel,

  Token__FirstOrLevel = TOKEN__BLOCK(13),
  Token_OrOr = Token__FirstOrLevel,
  Token__PastLastOrLevel,
  Token__PastLastBinaryOp = Token__PastLastOrLevel,

  Token__FirstBinaryAssignment = TOKEN__BLOCK(27),
  Token__FirstMulLevelAssignment = Token__FirstBinaryAssignment,
  Token_StarEQ = Token__FirstMulLevelAssignment,
  Token_SlashEQ,
  Token_ModEQ,
  Token_AndEQ,
  Token_LeLeEQ,
  Token_GeGeEQ,
  Token_GeGeGeEQ,
  Token__PastLastMulLevelAssignment,

  Token__FirstAddLevelAssignment = TOKEN__BLOCK(28),
  Token_PlusEQ = Token__FirstAddLevelAssignment,
  Token_MinusEQ,
  Token_OrEQ,
  Token_TildeEQ,
  Token__PastLastAddLevelAssignment,

  Token__PastLastBinaryAssignment = Token__PastLastAddLevelAssignment,

  Token__FirstKeyword,
  Token_True = Token__FirstKeyword,
  Token_False,
  Token_If,
  Token_Else,
  Token_While,
  Token_For,
  Token_Break,
  Token_Continue,
  Token_Return,
  Token_Proc,
  Token_Struct,
  Token__PastLastKeyword,
} Token_Kind;

#define KEYWORD_STRING(E, S) [E - Token__FirstKeyword] = { .data = (S), .size = sizeof(S)-1 }
String Keywords[] = {
  KEYWORD_STRING(Token_True,     "true"),
  KEYWORD_STRING(Token_False,    "false"),
  KEYWORD_STRING(Token_If,       "if"),
  KEYWORD_STRING(Token_Else,     "else"),
  KEYWORD_STRING(Token_While,    "while"),
  KEYWORD_STRING(Token_For,      "for"),
  KEYWORD_STRING(Token_Break,    "break"),
  KEYWORD_STRING(Token_Continue, "continue"),
  KEYWORD_STRING(Token_Return,   "return"),
  KEYWORD_STRING(Token_Proc,     "proc"),
  KEYWORD_STRING(Token_Struct,   "struct"),
};
#undef KEYWORD_STRING
STATIC_ASSERT(ARRAY_SIZE(Keywords) == Token__PastLastKeyword-Token__FirstKeyword);

// TODO: Meassure perf diff between discriminated union vs header/body vs kind + LUT
typedef struct Token
{
  Token_Kind kind;
  Text_Pos pos;

  union
  {
    s128 integer;
    f64 floating;
    String ident;
    String string;
  };
} Token;

typedef struct Token_Array
{
  Token* tokens;
  u32 token_count;
} Token_Array;

// NOTE: All ASCII values below 0x21 (except 0) are treated as whitespace
// NOTE: Content must be padded with 4 null terminators
static bool
LexFile(u8* content, Text_Pos init_pos, Arena token_arena, Token_Array* token_array)
{
  *token_array = (Token_Array){
    .tokens      = Arena_PushAlignment(token_arena, ALIGNOF(Token)),
    .token_count = 0,
  };

  for (u8* cursor = content;;)
  {
    for (;;)
    {
      if ((u8)(*cursor-1) < 0x20) ++cursor;
      else if (cursor[0] == '/' && cursor[1] == '/')
      {
        // NOTE: Knock out 2's bit to make line feed less than horizontal tab (lf 10->8, tab 9->9, null 0->0)
        while ((*cursor&0xFD) > 0x08) ++cursor;
      }
      else if (cursor[0] == '/' && cursor[1] == '*')
      {
        cursor += 2;
        uint nesting = 1;

        for (;;)
        {
          if (cursor[0] == '/' && cursor[1] == '*')
          {
            nesting += 1;
            cursor  += 2;
          }
          else if (cursor[0] == '*' && cursor[1] == '/')
          {
            nesting -= 1;
            if (nesting == 0) break;

            cursor  += 2;
          }
          else if (*cursor != 0) ++cursor;
          else                   break;
        }

        if (*cursor != 0) cursor += sizeof("*/")-1;
        else
        {
          //// ERROR: Unterminated block comment
          return false;
        }
      }
      else break;
    }

    if (*cursor == 0) break;
    else
    {
      Token* token = Arena_PushBytes(token_arena, sizeof(Token)); // NOTE: base is already aligned
      token->kind = Token_Invalid;
      token->pos  = init_pos + (Text_Pos)(cursor - content);

      u8 c[4];
      for (uint i = 0; i < 4; ++i) c[i] = cursor[i];

      switch (c[0])
      {
        case '(': token->kind = Token_OpenParen;    break;
        case ')': token->kind = Token_CloseParen;   break;
        case '[': token->kind = Token_OpenBracket;  break;
        case ']': token->kind = Token_CloseBracket; break;
        case '{': token->kind = Token_OpenBrace;    break;
        case '}': token->kind = Token_CloseBrace;   break;
        case '?': token->kind = Token_Qmark;        break;
        case ':': token->kind = Token_Colon;        break;
        case ',': token->kind = Token_Comma;        break;
        case ';': token->kind = Token_Semicolon;    break;
        case '^': token->kind = Token_Hat;          break;
        case '#': token->kind = Token_Pound;        break;
        case '$': token->kind = Token_Cash;         break;
        case '@': token->kind = Token_At;           break;

        case '.':
        {
          if      (c[1] == '.') token->kind = Token_Elipsis,       cursor += 1;
          else if (c[1] == '(') token->kind = Token_PeriodParen,   cursor += 1;
          else if (c[1] == '[') token->kind = Token_PeriodBracket, cursor += 1;
          else if (c[1] == '{') token->kind = Token_PeriodBrace,   cursor += 1;
          else                  token->kind = Token_Period;
        } break;

        case '*':
        {
          if (c[1] == '=') token->kind = Token_StarEQ, cursor += 1;
          else             token->kind = Token_Star;
        } break;

        case '/':
        {
          if (c[1] == '=') token->kind = Token_SlashEQ, cursor += 1;
          else             token->kind = Token_Slash;
        } break;

        case '%':
        {
          if (c[1] == '=') token->kind = Token_ModEQ, cursor += 1;
          else             token->kind = Token_Mod;
        } break;

        case '&':
        {
          if      (c[1] == '=') token->kind = Token_AndEQ,  cursor += 1;
          else if (c[1] == '&') token->kind = Token_AndAnd, cursor += 1;
          else                  token->kind = Token_And;
        } break;

        case '<':
        {
          if      (c[1] == '<' && c[2] == '=') token->kind = Token_LeLeEQ, cursor += 2;
          else if (c[1] == '<')                token->kind = Token_LeLe,   cursor += 1;
          else if (c[1] == '=')                token->kind = Token_LeEQ,   cursor += 1;
          else                                 token->kind = Token_Le;
        } break;

        case '>':
        {
          if      (c[1] == '>' && c[2] == '>' && c[3] == '=') token->kind = Token_GeGeEQ, cursor += 3;
          else if (c[1] == '>' && c[2] == '>')                token->kind = Token_GeGe,   cursor += 2;
          else if (c[1] == '>' && c[2] == '=')                token->kind = Token_GeGeEQ, cursor += 2;
          else if (c[1] == '>')                               token->kind = Token_GeGe,   cursor += 1;
          else if (c[1] == '=')                               token->kind = Token_GeEQ,   cursor += 1;
          else                                                token->kind = Token_Ge;
        } break;

        case '+':
        {
          if (c[1] == '=') token->kind = Token_PlusEQ, cursor += 1;
          else             token->kind = Token_Plus;
        } break;

        case '-':
        {
          if      (c[1] == '-' && c[2] == '-') token->kind = Token_TripleMinus, cursor += 2;
          else if (c[1] == '>')                token->kind = Token_Arrow,       cursor += 1;
          else if (c[1] == '=')                token->kind = Token_MinusEQ,     cursor += 1;
          else                                 token->kind = Token_Minus;
        } break;

        case '|':
        {
          if      (c[1] == '|') token->kind = Token_OrOr, cursor += 1;
          else if (c[1] == '=') token->kind = Token_OrEQ, cursor += 1;
          else                  token->kind = Token_Or;
        } break;

        case '~':
        {
          if (c[1] == '=') token->kind = Token_TildeEQ, cursor += 1;
          else             token->kind = Token_Tilde;
        } break;

        case '!':
        {
          if (c[1] == '=') token->kind = Token_NotEQ, cursor += 1;
          else             token->kind = Token_Not;
        } break;

        default:
        {
          if (Char_IsAlpha(c[0]) || c[0] == '_')
          {
            // NOTE: c[0] is the character before cursor
            String identifier = { .data = cursor-1, .size = 0 };

            for (; Char_IsAlpha(*cursor) || Char_IsDigit(*cursor) || *cursor == '_'; ++cursor);
            identifier.size = cursor - identifier.data;

            if (identifier.size == 1 && c[0] == '_') token->kind = Token_Underscore;
            else
            {
              Token_Kind kind = Token_Ident;
              
              for (uint i = 0; i < ARRAY_SIZE(Keywords); ++i)
              {
                if (String_Match(identifier, Keywords[i]))
                {
                  kind = i + Token__FirstKeyword;
                  break;
                }
              }

              token->kind  = kind;
              token->ident = identifier;
            }
          }
          else if (Char_IsDigit(c[0]))
          {
            if (c[0] == '0' && c[1] == 'b')
            {
              cursor += 1;

              uint digit_count = 0;
              u64 parts[2]     = {0};
              for (;; ++cursor)
              {
                if      (*cursor == '_')  continue;
                else if (*cursor-'0' > 1) break;
                else
                {
                  parts[digit_count >> 6] |= ((u64)(*cursor & 0x1) << (digit_count & 63));
                  digit_count             += 1;
                }
              }

              if (digit_count > 128)
              {
                //// ERROR: Binary integer literal has too many bits
                return false;
              }
              else if (digit_count == 0)
              {
                //// ERROR: Binary integer literal has no digits
                return false;
              }
              else
              {
                token->kind    = Token_Int;
                token->integer = S128_FromBits(parts[0], parts[1]);
              }
            }
            else if (c[0] == '0' && (c[1] == 'x' || c[1] == 'h'))
            {
              bool is_float = (c[1] == 'h');
              cursor += 1;

              uint digit_count = 0;
              u64 parts[2]     = {0};
              for (;; ++cursor)
              {
                u64 digit;
                if      (*cursor == '_')                              continue;
                else if (Char_IsDigit(*cursor))                       digit = *cursor&0xF;
                else if ((u8)((*cursor&0xDF)-'A') <= (u8)('F' - 'A')) digit = 9 + (*cursor & 0x07);

                parts[digit_count >> 4] |= digit << ((digit_count & 15) << 2);
              }

              if (is_float)
              {
                if (digit_count == 16)
                {
                  token->kind     = Token_Float;
                  token->floating = (F64_Bits){ .bits = parts[0] }.f;
                }
                else if (digit_count == 8)
                {
                  token->kind     = Token_Float;
                  token->floating = (F32_Bits){ .bits = (u32)parts[0] }.f;
                }
                else
                {
                  //// ERROR: Hex float literal must have exactly 8 or 16 digits (for f32 and f64 respectively)
                  return false;
                }
              }
              else
              {
                if (digit_count > 16)
                {
                  //// ERROR: Hex integer literal has too many digits
                  return false;
                }
                else if (digit_count == 0)
                {
                  //// ERROR: Hex integer literal has no digits
                  return false;
                }
                else
                {
                  token->kind    = Token_Int;
                  token->integer = S128_FromBits(parts[0], parts[1]);
                }
              }
            }
            else
            {
              uint digit_count = 1;
              u64 lo           = c[0]&0xF;
              for (; digit_count < 19; ++cursor)
              {
                u8 digit;
                if      (*cursor == '_')        continue;
                else if (Char_IsDigit(*cursor)) digit = *cursor&0xF;
                else                            break;

                lo           = lo*10 + digit;
                digit_count += 1;
              }

              s128 integer = S128_FromBits(lo, 0);
              if (Char_IsDigit(*cursor))
              {
                for (;; ++cursor)
                {
                  u8 digit;
                  if      (*cursor == '_')        continue;
                  else if (Char_IsDigit(*cursor)) digit = *cursor&0xF;
                  else                            break;
                  
                  bool overflow = false;
                  integer = S128_Mul(integer, S128_FromU64(10), &overflow);
                  integer = S128_Add(integer, S128_FromU64(digit), &overflow);

                  if (overflow)
                  {
                    //// ERROR: Integer literal is too large
                    return false;
                  }
                }
              }

              bool is_float = (*cursor == '.' || (*cursor&0xDF) == 'E');
              if (!is_float)
              {
                token->kind    = Token_Int;
                token->integer = integer;
              }
              else
              {
                u64 fraction = 0;
                if (*cursor == '.')
                {
                  uint fraction_digit_count = 0;
                  for (; fraction_digit_count < 19; ++cursor)
                  {
                    u8 digit;
                    if      (*cursor == '_')        continue;
                    else if (Char_IsDigit(*cursor)) digit = *cursor&0xF;
                    else                            break;
                    
                    fraction              = fraction*10 + digit;
                    fraction_digit_count += 1;
                  }

                  if (Char_IsDigit(*cursor))
                  {
                    //// ERROR: Too many digits in fractional part of floating point literal
                    return false;
                  }
                }

                s64 exponent = 0;
                if ((*cursor&0xDF) == 'E')
                {
                  cursor += 1;

                  bool is_neg = (*cursor == '-');
                  cursor += (*cursor == '-' || *cursor == '+');

                  uint exponent_digit_count = 0;
                  for (; exponent_digit_count < 4; ++cursor)
                  {
                    u8 digit;
                    if      (*cursor == '_')        continue;
                    else if (Char_IsDigit(*cursor)) digit = *cursor&0xF;
                    else                            break;
                    
                    exponent              = exponent*10 + digit;
                    exponent_digit_count += 1;
                  }

                  if (is_neg) exponent = -exponent;
                  
                  if (Char_IsDigit(*cursor))
                  {
                    //// ERROR: Too many digits in exponent of floating point literal
                    return false;
                  }
                }

                // TODO: Assemble float
                NOT_IMPLEMENTED;
              }
            }
          }
          else if (c[0] == '"')
          {
            String raw_string = { .data = cursor, .size = 0 };

            while (*cursor != 0 && *cursor != '"')
            {
              if (cursor[0] == '\\' && cursor[1] != 0) cursor += 2;
              else                                     cursor += 1;
            }

            if (*cursor != '"')
            {
              //// ERROR: Unterminated string literal
              return false;
            }
            else
            {
              raw_string.size = cursor - raw_string.data;

              // TODO: escaped characters

              token->kind   = Token_String;
              token->string = raw_string; // TODO: where to store strings?
            }
          }
          else
          {
            //// ERROR: Unknown symbol
            return false;
          }
        } break;
      }
    }
  }

  return true;
}
