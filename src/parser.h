typedef struct Parser
{
  Arena* ast_arena;
  Token token;
  Lexer lexer;
} Parser;

static Token
Parser__GetToken(Parser* state)
{
  return state->token;
}

static Token
Parser__NextToken(Parser* state)
{
  state->token = Lexer_NextToken(&state->lexer);
  return state->token;
}

static bool
Parser__EatToken(Parser* state, Token_Kind kind)
{
  bool result = false;

  if (state->token.kind == kind)
  {
    result = true;
    state->token = Lexer_NextToken(&state->lexer);
  }

  return result;
}

#define GET_TOKEN() Parser__GetToken(state)
#define NEXT_TOKEN() Parser__NextToken(state)
#define EAT_TOKEN(K) Parser__EatToken(state, (K))

static void*
Parser__PushExpr(Parser* state, umm size, u8 alignment, AST_Kind kind)
{
  AST* result = Arena_Push(state->ast_arena, size, alignment);

  result->kind = kind;
  result->next = 0;

  return result;
}

#define PUSH_EXPR(T, K) Parser__PushExpr(state, sizeof(T), _alignof(T), (K))

static void*
Parser__PushStmnt(Parser* state, umm size, u8 alignment, AST_Kind kind)
{
  AST* result = Arena_Push(state->ast_arena, size, alignment);

  result->kind = kind;
  result->next = 0;

  return result;
}

#define PUSH_STMNT(T, K) Parser__PushStmnt(state, sizeof(T), _alignof(T), (K))

// TODO: Temp
#define PARSER_ERROR() __debugbreak()

static bool Parser__ParseExpression(Parser* state, AST** expr);
static bool Parser__ParsePrimaryExpression(Parser* state, AST** expr);
static bool Parser__ParseBlock(Parser* state, AST** block);

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

    if (EAT_TOKEN(Token_Comma)) continue;
    else                        break;
  }

  return true;
}

static bool
Parser__ParseCommaSeparatedExpressions(Parser* state, AST** expr)
{
  for (AST** next_expr = expr;; next_expr = &(*next_expr)->next)
  {
    if (!Parser__ParseExpression(state, next_expr)) return false;

    if (EAT_TOKEN(Token_Comma)) continue;
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
    AST_Ident_Expr* ident_expr = PUSH_EXPR(AST_Ident_Expr, AST_Ident);
    ident_expr->ident = token.ident;

    NEXT_TOKEN();

    *expr = &ident_expr->header;
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
  else if (EAT_TOKEN(Token_OpenParen))
  {
    AST* inner;
    if (!Parser__ParseExpression(state, &inner)) return false;

    AST_Compound_Expr* compound = PUSH_EXPR(AST_Compound_Expr, AST_Compound);
    compound->inner = inner;

    *expr = &compound->header;

    if (!EAT_TOKEN(Token_CloseParen))
    {
      //// ERROR: Missing close paren
      PARSER_ERROR();
      return false;
    }
  }
  else if (EAT_TOKEN(Token_Proc))
  {
    AST* params   = 0;
    AST* ret_types = 0;

    if (!EAT_TOKEN(Token_OpenParen))
    {
      //// ERROR: Missing parameter list
      PARSER_ERROR();
      return false;
    }

    if (GET_TOKEN().kind != Token_CloseParen)
    {
      for (AST** next_param = &params;; next_param = &(*next_param)->next)
      {
        AST* names;
        AST* type  = 0;
        AST* value = 0;

        if (!Parser__ParseCommaSeparatedExpressions(state, &names)) return false;

        if (!EAT_TOKEN(Token_Colon))
        {
          //// ERROR: Missing type of parameters
          PARSER_ERROR();
          return false;
        }

        if (GET_TOKEN().kind != Token_EQ)
        {
          if (!Parser__ParseExpression(state, &type)) return false;
        }

        if (EAT_TOKEN(Token_EQ))
        {
          if (!Parser__ParseExpression(state, &value)) return false;
        }

        AST_Parameter* param = PUSH_EXPR(AST_Parameter, AST_Param);
        param->names = names;
        param->type  = type;
        param->value = value;

        *next_param = &param->header;

        if (EAT_TOKEN(Token_Comma)) continue;
        else                        break;
      }
    }

    if (!EAT_TOKEN(Token_CloseParen))
    {
      //// ERROR: Missing closing paren
      PARSER_ERROR();
      return false;
    }

    if (EAT_TOKEN(Token_Arrow))
    {
      if (GET_TOKEN().kind != Token_OpenParen)
      {
        if (!Parser__ParseExpression(state, &ret_types)) return false;
      }
      else
      {
        NEXT_TOKEN(); // NOTE: Skip open paren

        for (AST** next_type = &ret_types;; next_type = &(*next_type)->next)
        {
          AST* names;
          AST* type  = 0;
          AST* value = 0;

          if (!Parser__ParseCommaSeparatedExpressions(state, &names)) return false;

          if (!EAT_TOKEN(Token_Colon))
          {
            //// ERROR: Missing type of parameters
            PARSER_ERROR();
            return false;
          }

          if (GET_TOKEN().kind != Token_EQ)
          {
            if (!Parser__ParseExpression(state, &type)) return false;
          }

          if (EAT_TOKEN(Token_EQ))
          {
            if (!Parser__ParseExpression(state, &value)) return false;
          }

          AST_Return_Type* ret_type = PUSH_EXPR(AST_Return_Type, AST_RetType);
          ret_type->names = names;
          ret_type->type  = type;
          ret_type->value = value;

          *next_type = &ret_type->header;

          if (EAT_TOKEN(Token_Comma)) continue;
          else                        break;
        }

        if (!EAT_TOKEN(Token_CloseParen))
        {
          //// ERROR: Missing close paren
          PARSER_ERROR();
          return false;
        }
      }
    }

    if (GET_TOKEN().kind == Token_OpenBrace)
    {
      AST* body;
      if (!Parser__ParseBlock(state, &body)) return false;

      AST_Proc_Lit_Expr* proc_lit = PUSH_EXPR(AST_Proc_Lit_Expr, AST_ProcLit);
      proc_lit->params    = params;
      proc_lit->ret_types = ret_types;
      proc_lit->body      = body;

      *expr = &proc_lit->header;
    }
    else
    {
      AST_Proc_Type_Expr* proc_type = PUSH_EXPR(AST_Proc_Type_Expr, AST_ProcType);
      proc_type->params    = params;
      proc_type->ret_types = ret_types;

      *expr = &proc_type->header;
    }
  }
  else if (EAT_TOKEN(Token_Struct))
  {
    if (GET_TOKEN().kind != Token_OpenBrace)
    {
      //// ERROR: Missing struct body
      PARSER_ERROR();
      return false;
    }
    
    AST* body;
    if (!Parser__ParseBlock(state, &body)) return false;

    AST_Struct_Type_Expr* struct_type = PUSH_EXPR(AST_Struct_Type_Expr, AST_StructType);
    struct_type->body = body;

    *expr = &struct_type->header;
  }
  else
  {
    if (token.kind == Token_Invalid)
    {
      //// ERROR
      PARSER_ERROR();
      return false;
    }
    else
    {
      //// ERROR: Missing primary expression
      PARSER_ERROR();
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
        PARSER_ERROR();
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
        PARSER_ERROR();
        return false;
      }
    }
    else if (EAT_TOKEN(Token_Dot))
    {
      AST* name;
      if (!Parser__ParsePrimaryExpression(state, &name)) return false;

      AST_Member_Expr* member = PUSH_EXPR(AST_Member_Expr, AST_Member);
      member->operand = *expr;
      member->name    = name;

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
        PARSER_ERROR();
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

        *expr = &slice_of->header;
        expr = &slice_of->operand;
      }
      else
      {
        AST* size;
        if (!Parser__ParseExpression(state, &size)) return false;

        if (!EAT_TOKEN(Token_CloseBracket))
        {
          //// ERROR: Missing closing bracket after "array of" prefix operator
          PARSER_ERROR();
          return false;
        }

        AST_Array_Of_Expr* array_of = PUSH_EXPR(AST_Array_Of_Expr, AST_ArrayOf);
        array_of->size = size;

        *expr = &array_of->header;
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

        *expr = &prefix_expr->header;
        expr = &prefix_expr->operand;
      }
    }
  }

  return Parser__ParsePostfixExpression(state, expr);
}

#if 0
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
        u32 peek_block_idx = AST_KIND__BLOCK_IDX(peek_kind);

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
#elif 0
static bool
Parser__ParseBinaryExpression(Parser* state, AST** expr)
{
  if (!Parser__ParsePrefixExpression(state, expr)) return false;

  while (TOKEN_KIND__IS_BINARY(GET_TOKEN().kind))
  {
    AST_Kind op_kind = (AST_Kind)GET_TOKEN().kind;
    umm op_block_idx = AST_KIND__BLOCK_IDX(op_kind);
    NEXT_TOKEN();

    AST* right;
    if (!Parser__ParsePrefixExpression(state, &right)) return false;

    AST** slot = expr;
    while (AST_KIND__BLOCK_IDX((*slot)->kind) > op_block_idx) slot = &((AST_Binary_Expr*)*slot)->right;

    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, op_kind);
    op->left  = *slot;
    op->right = right;

    *slot = &op->header;
  }

  return true;
}
#else
static bool
Parser__ParseMulLevelExpression(Parser* state, AST** expr)
{
  if (!Parser__ParsePrefixExpression(state, expr)) return false;

  while ((u32)(GET_TOKEN().kind - Token__FirstMulLevel) <= (u32)(Token__PastLastMulLevel - Token__FirstMulLevel))
  {
    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, (AST_Kind)GET_TOKEN().kind);
    op->left = *expr;

    NEXT_TOKEN();

    if (!Parser__ParsePrefixExpression(state, &op->right)) return false;

    *expr = &op->header;
  }

  return true;
}
static bool
Parser__ParseAddLevelExpression(Parser* state, AST** expr)
{
  if (!Parser__ParseMulLevelExpression(state, expr)) return false;

  while ((u32)(GET_TOKEN().kind - Token__FirstAddLevel) <= (u32)(Token__PastLastAddLevel - Token__FirstAddLevel))
  {
    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, (AST_Kind)GET_TOKEN().kind);
    op->left = *expr;

    NEXT_TOKEN();

    if (!Parser__ParseMulLevelExpression(state, &op->right)) return false;

    *expr = &op->header;
  }

  return true;
}

static bool
Parser__ParseCmpLevelExpression(Parser* state, AST** expr)
{
  if (!Parser__ParseAddLevelExpression(state, expr)) return false;

  while ((u32)(GET_TOKEN().kind - Token__FirstCmpLevel) <= (u32)(Token__PastLastCmpLevel - Token__FirstCmpLevel))
  {
    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, (AST_Kind)GET_TOKEN().kind);
    op->left = *expr;

    NEXT_TOKEN();

    if (!Parser__ParseAddLevelExpression(state, &op->right)) return false;

    *expr = &op->header;
  }

  return true;
}

static bool
Parser__ParseLAndLevelExpression(Parser* state, AST** expr)
{
  if (!Parser__ParseCmpLevelExpression(state, expr)) return false;

  while (EAT_TOKEN(Token_LAnd))
  {
    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, AST_LAnd);
    op->left = *expr;

    if (!Parser__ParseCmpLevelExpression(state, &op->right)) return false;

    *expr = &op->header;
  }

  return true;
}

static bool
Parser__ParseBinaryExpression(Parser* state, AST** expr)
{
  if (!Parser__ParseLAndLevelExpression(state, expr)) return false;

  while (EAT_TOKEN(Token_LOr))
  {
    AST_Binary_Expr* op = PUSH_EXPR(AST_Binary_Expr, AST_LOr);
    op->left = *expr;

    if (!Parser__ParseLAndLevelExpression(state, &op->right)) return false;

    *expr = &op->header;
  }

  return true;
}
#endif

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
      PARSER_ERROR();
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

static bool
Parser__ParseStatement(Parser* state, bool check_for_semi, AST** stmnt)
{
  if (EAT_TOKEN(Token_Colon))
  {
    AST* label;
    if (!Parser__ParseExpression(state, &label)) return false;

    Token token = GET_TOKEN();
    if (token.kind != Token_OpenBrace && token.kind != Token_If && token.kind != Token_While)
    {
      //// ERROR: Labels may only be applied to block, if and while statements
      PARSER_ERROR();
      return false;
    }

    if (!Parser__ParseStatement(state, true, stmnt)) return false;

    if      ((*stmnt)->kind == AST_Block) ((AST_Block_Stmnt*)*stmnt)->label = label;
    else if ((*stmnt)->kind == AST_If)    ((AST_If_Stmnt*)*stmnt)->label    = label;
    else if ((*stmnt)->kind == AST_While) ((AST_While_Stmnt*)*stmnt)->label = label;
    else UNREACHABLE;
  }
  else if (GET_TOKEN().kind == Token_OpenBrace)
  {
    if (!Parser__ParseBlock(state, stmnt)) return false;
  }
  else if (EAT_TOKEN(Token_If))
  {
    if (!EAT_TOKEN(Token_OpenParen))
    {
      //// ERROR: Missing condition of if statement
      PARSER_ERROR();
      return false;
    }

    AST* condition;
    if (!Parser__ParseExpression(state, &condition)) return false;

    if (!EAT_TOKEN(Token_CloseParen))
    {
      //// ERROR: Missing closing paren
      PARSER_ERROR();
      return false;
    }

    AST* true_body;
    if (!Parser__ParseStatement(state, true, &true_body)) return false;

    AST* false_body = 0;
    if (EAT_TOKEN(Token_Else))
    {
      if (!Parser__ParseStatement(state, true, &false_body)) return false;
    }

    AST_If_Stmnt* if_stmnt = PUSH_STMNT(AST_If_Stmnt, AST_If);
    if_stmnt->label      = 0;
    if_stmnt->condition  = condition;
    if_stmnt->true_body  = true_body;
    if_stmnt->false_body = false_body;

    *stmnt = &if_stmnt->header;
  }
  else if (GET_TOKEN().kind == Token_Else)
  {
    //// ERROR: Else without matching if
    PARSER_ERROR();
    return false;
  }
  else if (EAT_TOKEN(Token_While))
  {
    if (!EAT_TOKEN(Token_OpenParen))
    {
      //// ERROR: Missing condition of while statement
      PARSER_ERROR();
      return false;
    }

    AST* condition;
    if (!Parser__ParseExpression(state, &condition)) return false;

    if (!EAT_TOKEN(Token_CloseParen))
    {
      //// ERROR: Missing closing paren
      PARSER_ERROR();
      return false;
    }

    AST* body;
    if (!Parser__ParseStatement(state, true, &body)) return false;

    AST_While_Stmnt* while_stmnt = PUSH_STMNT(AST_While_Stmnt, AST_While);
    while_stmnt->label     = 0;
    while_stmnt->condition = condition;
    while_stmnt->body      = body;

    *stmnt = &while_stmnt->header;
  }
  else if (EAT_TOKEN(Token_Return))
  {
    AST* args = 0;
    if (GET_TOKEN().kind != Token_Semicolon)
    {
      if (!Parser__ParseArgs(state, &args)) return false;
    }

    AST_Return_Stmnt* return_stmnt = PUSH_STMNT(AST_Return_Stmnt, AST_Return);
    return_stmnt->args = args;

    *stmnt = &return_stmnt->header;
  }
  else if (EAT_TOKEN(Token_Break))
  {
    AST* label = 0;
    if (GET_TOKEN().kind != Token_Semicolon)
    {
      if (!Parser__ParseExpression(state, &label)) return false;
    }

    AST_Break_Stmnt* break_stmnt = PUSH_STMNT(AST_Break_Stmnt, AST_Break);
    break_stmnt->label = label;

    *stmnt = &break_stmnt->header;
  }
  else if (EAT_TOKEN(Token_Continue))
  {
    AST* label = 0;
    if (GET_TOKEN().kind != Token_Semicolon)
    {
      if (!Parser__ParseExpression(state, &label)) return false;
    }

    AST_Continue_Stmnt* continue_stmnt = PUSH_STMNT(AST_Continue_Stmnt, AST_Continue);
    continue_stmnt->label = label;

    *stmnt = &continue_stmnt->header;
  }
  else
  {
    AST* exprs;
    if (!Parser__ParseCommaSeparatedExpressions(state, &exprs)) return false;

    if (EAT_TOKEN(Token_Colon))
    {
      AST* names = exprs;
      AST* type  = 0;

      if (GET_TOKEN().kind != Token_Colon && GET_TOKEN().kind != Token_EQ)
      {
        if (!Parser__ParseExpression(state, &type)) return false;
      }

      if (EAT_TOKEN(Token_Colon))
      {
        AST* values;
        if (!Parser__ParseCommaSeparatedExpressions(state, &values)) return false;

        AST_Const_Decl* const_decl = PUSH_STMNT(AST_Const_Decl, AST_Const);
        const_decl->names  = names;
        const_decl->type   = type;
        const_decl->values = values;

        *stmnt = &const_decl->header;
      }
      else
      {
        AST* values = 0;
        if (EAT_TOKEN(Token_EQ))
        {
          if (!Parser__ParseCommaSeparatedExpressions(state, &values)) return false;
        }

        AST_Var_Decl* var_decl = PUSH_STMNT(AST_Var_Decl, AST_Var);
        var_decl->names  = names;
        var_decl->type   = type;
        var_decl->values = values;

        *stmnt = &var_decl->header;
      }
    }
    else
    {
      if (TOKEN_KIND__IS_BINARY_ASSIGNMENT(GET_TOKEN().kind))
      {
        AST_Kind ass_kind = (AST_Kind)GET_TOKEN().kind;
        NEXT_TOKEN();

        AST* lhs = exprs;
        AST* rhs;
        if (!Parser__ParseCommaSeparatedExpressions(state, &rhs)) return false;

        AST_Assignment_Stmnt* assignment_stmnt = PUSH_STMNT(AST_Assignment_Stmnt, ass_kind);
        assignment_stmnt->lhs = lhs;
        assignment_stmnt->rhs = rhs;

        *stmnt = &assignment_stmnt->header;
      }
      else
      {
        *stmnt = exprs;
      }
    }
  }

  AST_Kind stmnt_kind = (*stmnt)->kind;
  if (check_for_semi &&
      (stmnt_kind != AST_Block && stmnt_kind != AST_If && stmnt_kind != AST_While))
  {
    if (!EAT_TOKEN(Token_Semicolon))
    {
      //// ERROR: Missing semicolon
      PARSER_ERROR();
      return false;
    }
  }

  return true;
}

static bool
Parser__ParseBlock(Parser* state, AST** stmnt)
{
  ASSERT(GET_TOKEN().kind == Token_OpenBrace);
  NEXT_TOKEN();

  AST* body = 0;
  AST** next_stmnt = &body;
  for (;;)
  {
    while (EAT_TOKEN(Token_Semicolon));

    Token token = GET_TOKEN();

    if (token.kind == Token_EOF)
    {
      //// ERROR: Unterminated block
      PARSER_ERROR();
      return false;
    }
    else if (token.kind == Token_CloseBrace)
    {
      NEXT_TOKEN();
      break;
    }
    else
    {
      if (!Parser__ParseStatement(state, true, next_stmnt)) return false;
      next_stmnt = &(*next_stmnt)->next;
    }
  }

  AST_Block_Stmnt* block = PUSH_STMNT(AST_Block_Stmnt, AST_Block);
  block->label = 0;
  block->body  = body;

  *stmnt = &block->header;

  return true;
}

static bool
Parser_ParseFile(Arena* ast_arena, Ident_Table* ident_table, u8* file_contents, AST** ast, u64* debug_lines)
{
  Parser _parser = {
    .ast_arena = ast_arena,
    .lexer     = Lexer_Init(file_contents, ident_table),
  };

  _parser.token = Lexer_NextToken(&_parser.lexer);

  Parser* state = &_parser;

  *ast = 0;
  AST** next_stmnt = ast;

  while (GET_TOKEN().kind != Token_EOF)
  {
    while (EAT_TOKEN(Token_Semicolon));

    if (!Parser__ParseStatement(state, false, next_stmnt)) return false;

    bool require_semicolon = true;

    AST_Kind stmnt_kind = (*next_stmnt)->kind;
    if      (stmnt_kind == AST_Block || stmnt_kind == AST_If || stmnt_kind == AST_While) require_semicolon = false;
    else if (stmnt_kind == AST_Const)
    {
      AST_Const_Decl* const_decl = (AST_Const_Decl*)*next_stmnt;

      if (const_decl->values != 0 && const_decl->values->next == 0 && (const_decl->values->kind == AST_ProcLit ||
                                                                       const_decl->values->kind == AST_StructType))
      {
        require_semicolon = false;
      }
    }

    if (require_semicolon && !EAT_TOKEN(Token_Semicolon))
    {
      //// ERROR: Missing semicolon
      PARSER_ERROR();
      return false;
    }

    next_stmnt = &(*next_stmnt)->next;
  }

  *debug_lines = GET_TOKEN().line;

  return true;
}

#undef GET_TOKEN
#undef NEXT_TOKEN
#undef EAT_TOKEN
#undef PUSH_EXPR
#undef PUSH_STMNT
