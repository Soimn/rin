typedef struct Parser
{
  Lexer lexer;
} Parser;

inline Token
Parser__GetToken(Parser* state)
{
  NOT_IMPLEMENTED;
  return (Token){0};
}

inline Token
Parser__NextToken(Parser* state)
{
  NOT_IMPLEMENTED;
  return (Token){0};
}

inline bool
Parser__EatToken(Parser* state, Token_Kind kind)
{
  NOT_IMPLEMENTED;
  return true;
}

#define GET_TOKEN() Parser__GetToken(state)
#define NEXT_TOKEN() Parser__NextToken(state)
#define EAT_TOKEN(K) Parser__EatToken(state, (K))

static void*
Parser__PushExpr(Parser* state, umm size, u8 alignment, AST_Kind kind)
{
  AST* result = 0;
  NOT_IMPLEMENTED;

  result->kind = kind;
  result->next = 0;

  return result;
}

#define PUSH_EXPR(T, K) Parser__PushExpr(state, sizeof(T), _alignof(T), (K))

static bool Parser__ParseExpression(Parser* state, AST** expr);

static bool
Parser__ParseArgs(Parser* state, AST** args)
{
  NOT_IMPLEMENTED;
  return true;
}

static bool
Parser__ParsePrimaryExpression(Parser* state, AST** expr)
{
  NOT_IMPLEMENTED;
  return true;
}

static bool
Parser__ParsePostfixExpression(Parser* state, AST** expr)
{
  if (!Parser__ParsePrimaryExpression(state, expr)) return false;

  for (;;)
  {
    AST* operand = *expr;

    if (EAT_TOKEN(Token_Hat))
    {
      AST_Deref_Expr* deref = PUSH_EXPR(AST_Deref_Expr, AST_Deref);
      deref->operand = operand;

      *expr = (AST*)deref;
    }
    else if (EAT_TOKEN(Token_OpenParen))
    {
      AST* args = 0;
      if (GET_TOKEN().kind != Token_CloseParen)
      {
        if (!Parser__ParseArgs(state, &args)) return false;
      }

      if (!EAT_TOKEN(Token_CloseParen))
      {
        //// ERROR: Missing closing paren
        return false;
      }

      AST_Call_Expr* call = PUSH_EXPR(AST_Call_Expr, AST_Call);
      call->operand = operand;
      call->args    = args;

      *expr = (AST*)call;
    }
    else if (EAT_TOKEN(Token_OpenBracket))
    {
      NOT_IMPLEMENTED;
    }
    else if (EAT_TOKEN(Token_Dot))
    {
      NOT_IMPLEMENTED;
    }
    else if (EAT_TOKEN(Token_DotBrace))
    {
      NOT_IMPLEMENTED;
    }
    else break;
  }

  return true;
}

static bool
Parser__ParsePrefixExpression(Parser* state, AST** expr)
{
  for (;;)
  {
    if (EAT_TOKEN(Token_OpenBracket))
    {
      if (EAT_TOKEN(Token_CloseBracket))
      {
        AST_Prefix_Expr* slice_of = PUSH_EXPR(AST_Prefix_Expr, AST_SliceOf);

        expr = &slice_of->operand;
      }
      else
      {
        AST* size;
        if (!Parser__ParseExpression(state, &size)) return false;

        if (!EAT_TOKEN(Token_CloseBracket))
        {
          //// ERROR: Missing closing bracket after "array of" prefix operator
          return false;
        }

        AST_Array_Of_Expr* array_of = PUSH_EXPR(AST_Array_Of_Expr, AST_ArrayOf);
        array_of->size = size;

        expr = &array_of->operand;
      }
    }
    else
    {
      AST_Kind kind = AST_Invalid;
      switch (GET_TOKEN().kind)
      {
        case Token_Add:  kind = AST_Pos;       break;
        case Token_Sub:  kind = AST_Neg;       break;
        case Token_LNot: kind = AST_LNot;      break;
        case Token_Xor:  kind = AST_Not;       break;
        case Token_And:  kind = AST_Ref;       break;
        case Token_Hat:  kind = AST_PointerTo; break;
      }

      if (kind == AST_Invalid) break;
      else
      {
        NEXT_TOKEN();

        AST_Prefix_Expr* prefix_expr = PUSH_EXPR(AST_Prefix_Expr, kind);

        expr = &prefix_expr->operand;
      }
    }
  }

  return Parser__ParsePostfixExpression(state, expr);
}

static bool
Parser__ParseBinaryExpression(Parser* state, AST** expr)
{
  NOT_IMPLEMENTED;
  return true;
}

static bool
Parser__ParseExpression(Parser* state, AST** expr)
{
  if (!Parser__ParseBinaryExpression(state, expr)) return false;

  if (EAT_TOKEN(Token_QMark))
  {
    AST* condition = *expr;
    AST* true_expr;
    AST* false_expr;

    if (!Parser__ParseBinaryExpression(state, &true_expr)) return false;

    if (!EAT_TOKEN(Token_Colon))
    {
      //// ERROR: Missing else clause of conditional expression
      return false;
    }

    if (!Parser__ParseBinaryExpression(state, &false_expr)) return false;

    AST_Conditional_Expr* cond = PUSH_EXPR(AST_Conditional_Expr, AST_Conditional);
    cond->condition  = condition;
    cond->true_expr  = true_expr;
    cond->false_expr = false_expr;

    *expr = (AST*)cond;
  }

  return true;
}

// TODO: Stub that can be used to flatten expressions later
static bool
Parser_ParseExpression(Parser* state, AST** expr)
{
  return Parser__ParseExpression(state, expr);
}

#undef GET_TOKEN
#undef NEXT_TOKEN
#undef EAT_TOKEN
