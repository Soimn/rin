typedef struct Lexer
{
  u8* input;
  u8* cursor;
  u8* start_of_line;
  u32 line;
  Ident_Table* ident_table;
} Lexer;

static Lexer
Lexer_Init(u8* file_content, Ident_Table* ident_table)
{
  return (Lexer){
    .input         = file_content,
    .cursor        = file_content,
    .start_of_line = file_content,
    .line          = 1,
    .ident_table   = ident_table,
  };
}

static Token
Lexer_NextToken(Lexer* lexer)
{
  Token token = { .kind = Token_Invalid };

  bool encountered_errors = false;

  for (;;)
  {
    // NOTE: All characters in the range [0x1, 0x20] are considered whitespace
    while ((u8)(*lexer->cursor-1) < (u8)0x20)
    {
      if (*lexer->cursor == '\n')
      {
        lexer->line         += 1;
        lexer->start_of_line = lexer->cursor + 1;
      }

      ++lexer->cursor;
    }

    if (lexer->cursor[0] == '/' && lexer->cursor[1] == '/')
    {
      lexer->cursor += 2;
      while (*lexer->cursor != 0 && *lexer->cursor != '\n') ++lexer->cursor;
    }
    else if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
    {
      lexer->cursor += 2;

      umm nesting = 1;
      while (nesting > 0)
      {
        if (lexer->cursor[0] == '/' && lexer->cursor[1] == '*')
        {
          lexer->cursor += 2;
          nesting       += 1;
        }
        else if (lexer->cursor[0] == '*' && lexer->cursor[1] == '/')
        {
          lexer->cursor += 2;
          nesting       -= 1;
        }
        else
        {
          if (*lexer->cursor == '\n')
          {
            lexer->line         += 1;
            lexer->start_of_line = lexer->cursor + 1;
          }

          ++lexer->cursor;
        }
      }
    }
    else break;
  }

  u8* start_of_token = lexer->cursor;

  if (*lexer->cursor == 0) token.kind = Token_EOF;
  else
  {
    u8 c = *lexer->cursor;
    lexer->cursor += 1;

    u8 c1_eq      = (*lexer->cursor == '=');
    u32 c1_eq_bit = (*lexer->cursor == '=' ? TOKEN_KIND__ASS_BIT : 0);

    switch (c)
    {
			case '$': token.kind = Token_Cash;         break;
			case '?': token.kind = Token_QMark;        break;
			case ':': token.kind = Token_Colon;        break;
			case ',': token.kind = Token_Comma;        break;
			case ';': token.kind = Token_Semicolon;    break;
			case '^': token.kind = Token_Hat;          break;
			case '(': token.kind = Token_OpenParen;    break;
			case ')': token.kind = Token_CloseParen;   break;
			case '[': token.kind = Token_OpenBracket;  break;
			case ']': token.kind = Token_CloseBracket; break;
			case '{': token.kind = Token_OpenBrace;    break;
			case '}': token.kind = Token_CloseBrace;   break;

      case '*': { token.kind = Token_Mul | c1_eq_bit; lexer->cursor += c1_eq; } break;
      case '/': { token.kind = Token_Div | c1_eq_bit; lexer->cursor += c1_eq; } break;
      case '%': { token.kind = Token_Rem | c1_eq_bit; lexer->cursor += c1_eq; } break;
      case '+': { token.kind = Token_Add | c1_eq_bit; lexer->cursor += c1_eq; } break;
      case '~': { token.kind = Token_Xor | c1_eq_bit; lexer->cursor += c1_eq; } break;

      case '-':
      {
        if (*lexer->cursor == '>')
        {
          token.kind     = Token_Arrow;
          lexer->cursor += 1;
        }
				else if (lexer->cursor[0] == '-' && lexer->cursor[1] == '-')
				{
					token.kind     = Token_StrikeOut;
					lexer->cursor += 2;
				}
        else
        {
          token.kind     = Token_Sub | c1_eq_bit;
          lexer->cursor += c1_eq;
        }
      } break;

      case '=':
      {
        token.kind     = (c1_eq ? Token_EQEQ : Token_EQ);
        lexer->cursor += c1_eq;
      } break;

      case '!':
      {
        token.kind     = (c1_eq ? Token_LNotEQ : Token_LNot);
        lexer->cursor += c1_eq;
      } break;

      case '&':
      {
        if (lexer->cursor[0] == '&')
        {
          if (lexer->cursor[1] == '=')
          {
            token.kind     = Token_LAndEQ;
            lexer->cursor += 2;
          }
          else
          {
            token.kind     = Token_LAnd;
            lexer->cursor += 1;
          }
        }
        else
        {
          token.kind     = Token_And | c1_eq_bit;
          lexer->cursor += c1_eq;
        }
      } break;

      case '|':
      {
        if (lexer->cursor[0] == '|')
        {
          if (lexer->cursor[1] == '=')
          {
            token.kind     = Token_LOrEQ;
            lexer->cursor += 2;
          }
          else
          {
            token.kind     = Token_LOr;
            lexer->cursor += 1;
          }
        }
        else
        {
          token.kind     = Token_Or | c1_eq_bit;
          lexer->cursor += c1_eq;
        }
      } break;

      case '<':
      {
        if (lexer->cursor[0] == '<')
        {
          if (lexer->cursor[1] == '=')
          {
            token.kind = Token_ShlEQ;
            lexer->cursor += 2;
          }
          else
          {
            token.kind = Token_Shl;
            lexer->cursor += 1;
          }
        }
        else
        {
          token.kind     = Token_Le + c1_eq;
          lexer->cursor += c1_eq;
        }
      } break;

      case '>':
      {
        if (lexer->cursor[0] == '>')
        {
          if (lexer->cursor[2] == '>')
          {
            if (lexer->cursor[3] == '=')
            {
              token.kind     = Token_SarEQ;
              lexer->cursor += 3;
            }
            else
            {
              token.kind     = Token_Sar;
              lexer->cursor += 2;
            }
          }
          else if (lexer->cursor[1] == '=')
          {
            token.kind = Token_ShrEQ;
            lexer->cursor += 2;
          }
          else
          {
            token.kind = Token_Shr;
            lexer->cursor += 1;
          }
        }
        else
        {
          token.kind     = Token_Ge + c1_eq;
          lexer->cursor += c1_eq;
        }
      } break;

      case '.':
      {
        if (*lexer->cursor == '(')
        {
          token.kind = Token_DotParen;
          lexer->cursor += 1;
        }
        else if (*lexer->cursor == '[')
        {
          token.kind = Token_DotBracket;
          lexer->cursor += 1;
        }
        else if (*lexer->cursor == '{')
        {
          token.kind = Token_DotBrace;
          lexer->cursor += 1;
        }
        else token.kind = Token_Dot;
      } break;

      default:
      {
        if (c == '_' || Char_IsAlpha(c))
        {
          // TODO: SIMD digest identifiers while hashing

          while (*lexer->cursor == '_' || Char_IsAlpha(*lexer->cursor) || Char_IsDigit(*lexer->cursor)) ++lexer->cursor;

          String ident_string = {
            .data = start_of_token,
            .len  = (u32)(lexer->cursor - start_of_token),
          };

          Token_Kind kind;
          Ident ident = IdentTable_Put(lexer->ident_table, IdentTable_Hash(ident_string), ident_string, &kind);

          token.kind  = kind;
          token.ident = ident;
        }
        else if (Char_IsDigit(c))
        {
          u8 base           = 10;
          bool is_hex_float = false;

          if (c == '0')
          {
            if      (*lexer->cursor == 'x') base = 16;
            else if (*lexer->cursor == 'h') base = 16, is_hex_float = true;
            else if (*lexer->cursor == 'o') base = 8;
            else if (*lexer->cursor == 'b') base = 2;
          }

          u64 value       = c & 0xF;
          umm digit_count = 1;
          if (base != 10)
          {
            value       = 0;
            digit_count = 0;
            lexer->cursor += 1; // NOTE: Skip base prefix
          }

          bool integer_overflow = false;
          for (;; lexer->cursor += 1)
          {
            u8 digit;
            if      (Char_IsDigit(*lexer->cursor)) digit = *lexer->cursor & 0xF;
            else if (Char_IsAlpha(*lexer->cursor)) digit = (*lexer->cursor & 0x1F) + 9;
            else if (*lexer->cursor == '_') continue;
            else                            break;

            if (digit >= base)
            {
              //// ERROR: Invalid base x digit
              encountered_errors = true;
              break;
            }

            u64 old_value = value;

            value *= base;

            // TODO: better way of checking for overflow
            integer_overflow = (integer_overflow || value < old_value || value + digit < value);

            value       += digit;
            digit_count += 1;
          }

          if (!encountered_errors)
          {
            if (base == 10 && (*lexer->cursor == '.' || Char_UncheckedToUpper(*lexer->cursor) == 'E'))
            {
              // TODO: Parse floats
              NOT_IMPLEMENTED;
            }
            else if (is_hex_float)
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
                //// ERROR: Invalid digit count in hex float, must be either 8 or 16
                encountered_errors = true;
              }
            }
            else
            {
              if (digit_count == 0)
              {
                //// ERROR: Missing digits of integer literal
                encountered_errors = true;
              }
              else if (integer_overflow)
              {
                //// ERROR: Integer literal is too large
                encountered_errors = true;
              }
              else
              {
                token.kind    = Token_Int;
                token.integer = value;
              }
            }
          }
        }
        else if (c == '"' || c == '\'')
        {
          NOT_IMPLEMENTED;
        }
        else
        {
          //// ERROR: Unknown token
          encountered_errors = true;
        }
      } break;
    }
  }

  token.offset   = (u32)(start_of_token - lexer->input);
  token.len      = (u32)(lexer->cursor - start_of_token);
  token.line     = lexer->line; // TODO: This is only valid as long as there are no multiline tokens
  token.col      = (u32)(start_of_token - lexer->start_of_line) + 1;

  // TODO: temp
  if (encountered_errors) token.kind = Token_Invalid;

  return token;
}
