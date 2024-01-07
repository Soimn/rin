typedef struct Parser
{
  Token* tokens;
  u32 token_idx;
  u32 token_count;
} Parser;

static void*
PushNode(Parser* state, Text_Pos pos, AST_Kind kind)
{
  AST* node = 0;
  NOT_IMPLEMENTED;

  node->kind     = kind;
  node->text_pos = pos;

  return node;
}

static Token_Kind
GetToken(Parser* state)
{
  ASSERT(state->token_idx < state->token_count);
  return state->tokens[state->token_idx].kind;
}

static Token_Kind
PeekToken(Parser* state)
{
  ASSERT(state->token_idx < state->token_count && state->token_idx + 1 < state->token_count);
  return state->tokens[state->token_idx+1].kind;
}

static Text_Pos
GetTokenPos(Parser* state)
{
  ASSERT(state->token_idx < state->token_count);
  return state->tokens[state->token_idx].pos;
}

static bool
EatToken(Parser* state, Token_Kind kind)
{
  ASSERT(state->token_idx < state->token_count);
  ASSERT(state->tokens[state->token_idx].kind != Token_Invalid);
  ASSERT(state->tokens[state->token_idx].kind != Token_EndOfStream);

  bool result = (state->tokens[state->token_idx].kind == kind);

  state->token_idx += result;

  return result;
}

static Token_Kind
SkipToken(Parser* state)
{
  ASSERT(state->token_idx < state->token_count && state->token_idx + 1 < state->token_count);
  ASSERT(state->tokens[state->token_idx].kind != Token_Invalid);
  ASSERT(state->tokens[state->token_idx].kind != Token_EndOfStream);
  
  state->token_idx += 1;

  return state->tokens[state->token_idx].kind;
}

static bool ParseExpression(Parser* state, AST** expr);

static bool
ParsePrimaryExpression(Parser* state, AST** expr)
{
  Token_Kind kind = GetToken(state);
  Text_Pos pos    = GetTokenPos(state);

  if (kind == Token_Ident)
  {
    Ident_Expr* ident = PushNode(state, pos, AST_Ident);
    ident->value = state->tokens[state->token_idx].ident; // TODO

    *expr = &ident->header;
    SkipToken(state);
  }
  else if (kind == Token_String)
  {
    String_Expr* string = PushNode(state, pos, AST_String);
    string->value = state->tokens[state->token_idx].string; // TODO
    
    *expr = &string->header;
    SkipToken(state);
  }
  else if (kind == Token_Int)
  {
    Int_Expr* integer = PushNode(state, pos, AST_Int);
    integer->value = state->tokens[state->token_idx].integer; // TODO
    
    *expr = &integer->header;
    SkipToken(state);
  }
  else if (kind == Token_Float)
  {
    Float_Expr* floating = PushNode(state, pos, AST_Float);
    floating->value = state->tokens[state->token_idx].floating; // TODO

    *expr = &floating->header;
    SkipToken(state);
  }
  else if (kind == Token_True || kind == Token_False)
  {
  	Bool_Expr* bool_expr = PushNode(state, pos, AST_Bool);
    bool_expr->value = (kind == Token_True);

    *expr = &bool_expr->header;
    SkipToken(state);
  }
  else if (EatToken(state, Token_Proc))
  {
  	NOT_IMPLEMENTED;
  }
  else if (EatToken(state, Token_Struct))
  {
  	NOT_IMPLEMENTED;
  }
  else if (EatToken(state, Token_OpenParen))
  {
    AST* inner = 0;
    if (!ParseExpression(state, &inner)) return false;
    else
    {
      Compound_Expr* compound = PushNode(state, pos, AST_Compound);
      compound->inner = inner;

      *expr = &compound->header;
    }
  }
  else
  {
    //// ERROR: Missing primary expression
    NOT_IMPLEMENTED;
  }

  return true;
}

static bool
ParseTypePrefixExpression(Parser* state, AST** expr)
{
  for (;;)
  {
    Text_Pos pos = GetTokenPos(state);

    if (EatToken(state, Token_Hat))
    {
      Unary_Expr* pointer_type = PushNode(state, pos, AST_PointerType);
      expr = &pointer_type->expr;
    }
    else if (EatToken(state, Token_OpenBracket))
    {
      if (EatToken(state, Token_CloseBracket))
      {
        Unary_Expr* slice_type = PushNode(state, pos, AST_SliceType);
        expr = &slice_type->expr;
      }
      else
      {
        AST* size = 0;
        if (!ParseExpression(state, &size)) return false;
        else
        {
          Array_Type_Expr* array_type = PushNode(state, pos, AST_ArrayType);
          array_type->size = size;

          expr = &array_type->type;
        }
      }
    }
    else break;
  }

  return ParsePrimaryExpression(state, expr);
}

static bool
ParsePostfixExpression(Parser* state, AST** expr)
{
  if (!ParseTypePrefixExpression(state, expr)) return false;
  else
  {
    for (;;)
    {
      Text_Pos pos = GetTokenPos(state);

      if (EatToken(state, Token_Hat))
      {
        Unary_Expr* deref = PushNode(state, pos, AST_Deref);
        deref->expr = *expr;
        *expr = &deref->header;
      }
      else if (EatToken(state, Token_Period))
      {
        AST* host = *expr;
        AST* name = 0;
        if (!ParseTypePrefixExpression(state, &name)) return false;
        else
        {
          Member_Expr* member = PushNode(state, pos, AST_Member);
          member->host = host;
          member->name = name;

          *expr = &member->header;
        }
      }
      else if (EatToken(state, Token_OpenParen))
      {
        NOT_IMPLEMENTED;
      }
      else if (EatToken(state, Token_OpenBracket))
      {
        NOT_IMPLEMENTED;
      }
      else break;
    }
  }

  return true;
}

static bool
ParsePrefixExpression(Parser* state, AST** expr)
{
  for (;;)
  {
    Unary_Expr* op = 0;
    Text_Pos pos = GetTokenPos(state);
    if      (EatToken(state, Token_Plus))  op = PushNode(state, pos, AST_Pos);
    else if (EatToken(state, Token_Minus)) op = PushNode(state, pos, AST_Neg);
    else if (EatToken(state, Token_Not))   op = PushNode(state, pos, AST_Not);
    else if (EatToken(state, Token_Tilde)) op = PushNode(state, pos, AST_BitNot);
    else if (EatToken(state, Token_And))   op = PushNode(state, pos, AST_Ref);
    else break;

    expr = &op->expr;
  }

  return ParsePostfixExpression(state, expr);
}

static bool
ParseBinaryExpression(Parser* state, AST** expr)
{
  if (!ParsePrefixExpression(state, expr)) return false;
  else
  {
    Token_Kind kind = GetToken(state);
    while (kind >= Token__FirstBinaryOp && kind < Token__PastLastBinaryOp)
    {
      Text_Pos pos = GetTokenPos(state);

      // NOTE: Token kinds for binary ops are equal to the AST kinds
      AST_Kind op_kind = kind;
      uint precedence  = AST__BLOCK_IDX(op_kind);

      AST** spot = expr;
      AST* right = 0;
      if (!ParsePrefixExpression(state, &right)) return false;
      else
      {
        while (precedence < AST__BLOCK_IDX((*spot)->kind))
        {
          ASSERT(AST_IS_BINARY_EXPR((*spot)->kind));
          spot = &((Binary_Expr*)*spot)->right;
        }

        AST* left = *spot;

        Binary_Expr* op = PushNode(state, pos, op_kind);
        op->left  = left;
        op->right = right;

        *spot = &op->header;
      }
    }
  }

  return true;
}

static bool
ParseExpression(Parser* state, AST** expr)
{
  if (!ParseBinaryExpression(state, expr)) return false;
  else
  {
    Text_Pos pos = GetTokenPos(state);
    if (EatToken(state, Token_Qmark))
    {
      AST* condition  = *expr;
      AST* true_expr  = 0;
      AST* false_expr = 0;

      if      (!ParseBinaryExpression(state, &true_expr)) return false;
      else if (!EatToken(state, Token_Colon))
      {
        //// ERROR: Missing separating colon between true and false expressions in conditional expression
        NOT_IMPLEMENTED;
      }
      else if (!ParseBinaryExpression(state, &false_expr)) return false;
      else
      {
        Conditional_Expr* cond_expr = PushNode(state, pos, AST_Conditional);
        cond_expr->condition  = condition;
        cond_expr->true_expr  = true_expr;
        cond_expr->false_expr = false_expr;

        *expr = &cond_expr->header;
      }
    }
  }

  return true;
}
