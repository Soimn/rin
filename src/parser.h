typedef struct Parser
{
	Token* token;
} Parser;

static Token
Parser__GetToken(Parser* state)
{
	return *state->token;
}

static void
Parser__NextToken(Parser* state)
{
	++state->token;
}

static bool
Parser__EatToken(Parser* state, Token_Kind kind)
{
	if (Parser__GetToken(state).kind == kind)
	{
		Parser__NextToken(state);
		return true;
	}
	else return false;
}

#define GET_TOKEN() Parser__GetToken(state)
#define NEXT_TOKEN() Parser__NextToken(state)
#define EAT_TOKEN(K) Parser__EatToken(state, (K))

static bool
Parser__ParsePostfixExpr(Parser* state, AST_Header** expr)
{
	NOT_IMPLEMENTED;
	return true;
}

static bool
Parser__ParsePrefixExpr(Parser* state, AST_Header** expr)
{
	AST_Ptr* link = 0;

	for (;;)
	{
		AST_Kind kind = ASTKind_Invalid;

		switch (GET_TOKEN().kind)
		{
			case Token_Plus:       kind = ASTKind_Pos;  break;
			case Token_Minus:      kind = ASTKind_Neg;  break;
			case Token_Tilde:      kind = ASTKind_Not;  break;
			case Token_Bang:       kind = ASTKind_LNot; break;
			case Token_And:        kind = ASTKind_Ref;  break;
			case Token_PlusPlus:   kind = ASTKind_Inc;  break;
			case Token_MinusMinus: kind = ASTKind_Dec;  break;
		}

		if (kind == ASTKind_Invalid) break;
		else
		{
			AST_Prefix_Expr* node = 0;

			if (link == 0) *expr = &node->header;
			else           ASTPtr_SetToPtr(link, node);

			link = &node->operand;
		}
	}

	AST_Header* operand = 0;
	if (!Parser__ParsePostfixExpr(state, &operand)) return false;

	if (link == 0) *expr = operand;
	else           ASTPtr_SetToPtr(link, operand);

	return true;
}

static bool
Parser__ParseBinaryExpr(Parser* state, AST_Header** expr)
{
	if (!Parser__ParsePrefixExpr(state, expr)) return false;

	struct { AST_Binary_Expr* node; u32 precedence; } stack[1 + AST_KIND__BINARY_PRECEDENCE_LEVELS] = {0};
	stack[0].node       = 0;
	stack[0].precedence = ~(u64)0;
	umm stack_cur = 1;
	stack[stack_cur].node       = (AST_Binary_Expr*)*expr; // NOTE: This is safe since the precedence is set to 0
	stack[stack_cur].precedence = 0;

	while (TOKEN_KIND__IS_BINARY(GET_TOKEN().kind))
	{
		Token_Kind token_kind = GET_TOKEN().kind;
		NEXT_TOKEN();

		AST_Kind kind  = AST_KIND__BINARY_FROM_TOKEN(token_kind);
		u32 precedence = AST_KIND__BINARY_PRECEDENCE(kind);

		while (precedence >= stack[stack_cur-1].precedence)
		{
			--stack_cur;
			ASTPtr_SetToPtr(&stack[stack_cur].node->rhs, stack[stack_cur+1].node);
		}

		AST_Header* lhs = &stack[stack_cur].node->header;

		if (precedence < stack[stack_cur].precedence)
		{
			lhs = ASTPtr_ToPtr(&stack[stack_cur].node->rhs);
			stack[stack_cur].node->rhs = ASTPtr_Nil;

			++stack_cur;
			ASSERT(stack_cur < ARRAY_LEN(stack));
		}

		AST_Header* rhs = 0;
		if (!Parser__ParsePrefixExpr(state, &rhs)) return false;

		AST_Binary_Expr* node = 0;
		ASTPtr_SetToPtr(&node->lhs, lhs);
		ASTPtr_SetToPtr(&node->rhs, rhs);

		stack[stack_cur].node       = node;
		stack[stack_cur].precedence = precedence;
	}

	return true;
}

static bool
Parser__ParseExpr(Parser* state, AST_Header** expr)
{
	if (!Parser__ParseBinaryExpr(state, expr)) return false;

	if (EAT_TOKEN(Token_QMark))
	{
		AST_Header* condition = *expr;
		AST_Header* true_val  = 0;
		AST_Header* false_val = 0;

		if (!Parser__ParseBinaryExpr(state, &true_val)) return false;

		if (!EAT_TOKEN(Token_Colon))
		{
			//// ERROR: Missing : after condition ? true_val
			NOT_IMPLEMENTED;
			return false;
		}

		if (!Parser__ParseBinaryExpr(state, &false_val)) return false;

		AST_Conditional* node = 0;
		ASTPtr_SetToPtr(&node->condition, condition);
		ASTPtr_SetToPtr(&node->true_val, true_val);
		ASTPtr_SetToPtr(&node->false_val, false_val);
	}

	return true;
}

#undef GET_TOKEN
#undef NEXT_TOKEN
#undef EAT_TOKEN
