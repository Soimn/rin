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
  for (;;)
  {
    AST* name = 0;
    AST* value;

    if (!Parser__ParseExpression(state, &value)) return false;

    if (EAT_TOKEN(Token_EQ))
    {
      name = value;
      if (!Parser__ParseExpression(state, &value)) return false;
    }

    AST_Argument* argument = PUSH_EXPR(AST_Argument, AST_Arg);
    argument->name  = name;
    argument->value = value;

    *args = &argument->header;
    args = &argument->next;

    if (EAT_TOKEN(Token_Colon)) continue;
    else                        break;
  }

  return true;
}

static bool
Parser__ParsePrimaryExpression(Parser* state, AST** expr)
{
  Token token = GET_TOKEN();

  if (token.kind == Token_Ident)
  {
    NOT_IMPLEMENTED;
  }
  else if (token.kind == Token_String)
  {
    NOT_IMPLEMENTED;
  }
  else if (token.kind == Token_Int)
  {
    AST_Int_Expr* int_expr = PUSH_EXPR(AST_Int_Expr, AST_Int);
    int_expr->value = token.integer;

    NEXT_TOKEN();

    *expr = &int_expr->header;
  }
  else if (token.kind == Token_Float)
  {
    AST_Float_Expr* float_expr = PUSH_EXPR(AST_Float_Expr, AST_Float);
    float_expr->value = token.floating;

    NEXT_TOKEN();

    *expr = &float_expr->header;
  }
  else if (token.kind == Token_True || token.kind == Token_False)
  {
    AST_Bool_Expr* bool_expr = PUSH_EXPR(AST_Bool_Expr, AST_Bool);
    bool_expr->value = (token.kind == Token_True);

    NEXT_TOKEN();

    *expr = &bool_expr->header;
  }
  else if (token.kind == Token_OpenParen)
  {
    AST* inner;
    if (!Parser__ParseExpression(state, &inner)) return false;

    AST_Compound_Expr* compound = PUSH_EXPR(AST_Compound_Expr, AST_Compound);
    compound->inner = inner;

    *expr = &compound->header;

    if (!EAT_TOKEN(Token_CloseParen))
    {
      //// ERROR: Missing close paren
      return false;
    }
  }
  else if (EAT_TOKEN(Token_Proc))
  {
    NOT_IMPLEMENTED;
  }
  else if (EAT_TOKEN(Token_Struct))
  {
    NOT_IMPLEMENTED;
  }
  else
  {
    if (token.kind == Token_Invalid)
    {
      //// ERROR
      return false;
    }
    else
    {
      //// ERROR: Missing primary expression
      return false;
    }
  }

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
      AST* first = 0;
      if (GET_TOKEN().kind != Token_Colon)
      {
        if (!Parser__ParseExpression(state, &first)) return false;
      }

      if (EAT_TOKEN(Token_Colon))
      {
        AST* second = 0;
        if (GET_TOKEN().kind != Token_CloseBracket)
        {
          if (!Parser__ParseExpression(state, &second)) return false;
        }

        AST_Slice_Expr* slice = PUSH_EXPR(AST_Slice_Expr, AST_Slice);
        slice->array         = *expr;
        slice->start_idx     = first;
        slice->past_last_idx = second;

        *expr = &slice->header;
      }
      else
      {
        AST_Index_Expr* index = PUSH_EXPR(AST_Index_Expr, AST_Index);
        index->array = *expr;
        index->idx   = first;

        *expr = &index->header;
      }

      if (!EAT_TOKEN(Token_CloseBracket))
      {
        //// ERROR: Missing closing bracket
        return false;
      }
    }
    else if (EAT_TOKEN(Token_Dot))
    {
      if (GET_TOKEN().kind != Token_Ident)
      {
        //// ERROR: Missing member name to access
        return false;
      }

      // TODO:
      NOT_IMPLEMENTED;
      NEXT_TOKEN();

      AST_Member_Expr* member = PUSH_EXPR(AST_Member_Expr, AST_Member);
      member->operand = *expr;
      // member->member = ;
      NOT_IMPLEMENTED;

      *expr = &member->header;
    }
    else if (EAT_TOKEN(Token_DotBrace))
    {
      AST* args = 0;
      if (GET_TOKEN().kind != Token_CloseBrace)
      {
        if (!Parser__ParseArgs(state, &args)) return false;
      }

      if (!EAT_TOKEN(Token_CloseBrace))
      {
        //// ERROR: Missing closing brace
        return false;
      }

      AST_Struct_Lit_Expr* struct_lit = PUSH_EXPR(AST_Struct_Lit_Expr, AST_StructLit);
      struct_lit->operand = *expr;
      struct_lit->args    = args;

      *expr = &struct_lit->header;
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
  if (!Parser__ParsePrefixExpression(state, expr)) return false;

  if (TOKEN_KIND__IS_BINARY(GET_TOKEN().kind))
  {
    struct { AST** slot; u32 precedence; } stack[2 + (AST_KIND__PastLastBinaryBlockIDX - AST_KIND__FirstBinaryBlockIDX)];
    stack[0].slot       = 0;
    stack[0].precedence = U32_MAX;
    stack[1].slot       = expr;
    stack[1].precedence = 0;

    u32 stack_idx = 1;

    for (;;)
    {
      AST_Kind op_kind = (AST_Kind)GET_TOKEN().kind;
      u32 op_block_idx = AST_KIND__BLOCK_IDX(op_kind);
      NEXT_TOKEN();

      AST* left = *stack[stack_idx].slot;
      AST* right;
      if (!Parser__ParsePrefixExpression(state, &right)) return false;

      AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, op_kind);
      op->left  = left;
      op->right = right;

      *stack[stack_idx].slot      = &op->header;
      stack[stack_idx].precedence = op_block_idx;

      Token_Kind peek_kind = GET_TOKEN().kind;
      if (!TOKEN_KIND__IS_BINARY(peek_kind)) break;
      else
      {
        u32 peek_block_idx = AST_KIND__BLOCK_IDX(peek_block_idx);

        if (peek_block_idx < op_block_idx)
        {
          ASSERT(stack_idx < ARRAY_SIZE(stack) - 1);

          stack_idx += 1;
          stack[stack_idx].slot = &op->right;
        }
        else if (peek_block_idx > op_block_idx)
        {
          while (stack[stack_idx-1].precedence <= peek_block_idx) stack_idx -= 1;
          ASSERT(stack_idx > 0);
        }
      }
    }
  }

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

static bool
Parser_ParseStatement(Parser* state, AST** stmnt)
{
  NOT_IMPLEMENTED;
  return true;
}

#undef GET_TOKEN
#undef NEXT_TOKEN
#undef EAT_TOKEN
