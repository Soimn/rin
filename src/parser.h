typedef struct Parser
{
	u8* file_contents;
	Token* token;
} Parser;

static Token
Parser__GetToken(Parser* state)
{
	return *state->token;
}

static Token_Data
Parser__GetTokenData(Parser* state)
{
	union { Token token; Token_Data token_data; } val = { .token = *state->token };
	return val.token_data;
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

static void*
Parser__PushNode(Parser* state, AST_Kind kind, umm size)
{
	NOT_IMPLEMENTED;
	AST_Header* node = 0;

	Zero(node, size);

	node->kind = kind;

	return node;
}

#define GET_TOKEN() Parser__GetToken(state)
#define GET_TOKEN_DATA() Parser__GetTokenData(state)
#define NEXT_TOKEN() Parser__NextToken(state)
#define EAT_TOKEN(K) Parser__EatToken(state, (K))
#define PUSH_NODE(N) (AST_##N*)Parser__PushNode(state, ASTKind_##N, sizeof(AST_##N))
#define PUSH_GROUPED_NODE(T, K) (T*)Parser__PushNode(state, K, sizeof(T))

static bool Parser__ParseExpr(Parser* state, AST_Header** expr);
static bool Parser__ParseTypePrefixExpr(Parser* state, AST_Header** expr);

static bool
Parser__ParseArguments(Parser* state, AST_Header** args)
{
	AST_Ptr* link = 0;

	for (;;)
	{
		AST_Header* name  = 0;
		AST_Header* value = 0;

		if (!Parser__ParseExpr(state, &value)) return false;

		if (EAT_TOKEN(Token_Eq))
		{
			name = value;

			if (!Parser__ParseExpr(state, &value)) return false;
		}

		AST_Argument* node = PUSH_NODE(Argument);

		node->next = ASTPtr_Nil;
		ASTPtr_SetToPtr(&node->value, value);

		if (name == 0) node->name = ASTPtr_Nil;
		else           ASTPtr_SetToPtr(&node->name, name);

		if (link == 0) *args = &node->header;
		else           ASTPtr_SetToPtr(link, node);

		link = &node->next;

		if (EAT_TOKEN(Token_Comma)) continue;
		else                        break;
	}

	return false;
}

static bool
Parser__ParseDotStuff(Parser* state, AST_Header* lhs, AST_Header** expr)
{
	ASSERT(GET_TOKEN().kind == Token_Dot);
	NEXT_TOKEN();

	if (EAT_TOKEN(Token_OpenBrace))
	{
		AST_Header* args = 0;
		if (!Parser__ParseArguments(state, &args)) return false;

		if (!EAT_TOKEN(Token_CloseBrace))
		{
			//// ERROR: Missing closing brace after struct lit args
			NOT_IMPLEMENTED;
			return false;
		}

		AST_StructLit* node = PUSH_NODE(StructLit);
		ASTPtr_SetToPtr(&node->type, lhs);
		ASTPtr_SetToPtr(&node->args, args);

		*expr = &node->header;
	}
	else
	{
		AST_Header* name = 0;
		if (!Parser__ParseTypePrefixExpr(state, &name)) return false;

		AST_Member* node = PUSH_NODE(Member);
		ASTPtr_SetToPtr(&node->expr, lhs);
		ASTPtr_SetToPtr(&node->name, name);

		*expr = &node->header;
	}

	return true;
}

static bool
Parser__ParsePrimaryExpr(Parser* state, AST_Header** expr)
{
	Token_Kind token_kind = GET_TOKEN().kind;

	if (token_kind == Token_Ident)
	{
		Token token = GET_TOKEN();
		NEXT_TOKEN();

		AST_Ident* node = PUSH_NODE(Ident);
		node->len  = token.len;
		node->data = state->file_contents + token.offset;
	}
	else if (token_kind == Token_String)
	{
		Token token = GET_TOKEN();
		NEXT_TOKEN();
		Token_Data token_data = GET_TOKEN_DATA();
		NEXT_TOKEN();

		AST_String* node = PUSH_NODE(String);
		node->len  = token.len;
		node->data = token_data.string;
	}
	else if (token_kind == Token_Char)
	{
		NEXT_TOKEN();

		AST_Char* node = PUSH_NODE(Char);
		node->value = (u8)GET_TOKEN_DATA().integer;

		NEXT_TOKEN();

		*expr = &node->header;
	}
	else if (token_kind == Token_Int)
	{
		NEXT_TOKEN();

		AST_Int* node = PUSH_NODE(Int);
		node->value = GET_TOKEN_DATA().integer;

		NEXT_TOKEN();

		*expr = &node->header;
	}
	else if (token_kind == Token_Int128)
	{
		NEXT_TOKEN();

		AST_Int128* node = PUSH_NODE(Int128);

		node->value_lo= GET_TOKEN_DATA().integer;
		NEXT_TOKEN();

		node->value_hi = GET_TOKEN_DATA().integer;
		NEXT_TOKEN();

		*expr = &node->header;
	}
	else if (token_kind == Token_Float)
	{
		NEXT_TOKEN();

		AST_Float* node = PUSH_NODE(Float);
		node->value = GET_TOKEN_DATA().floating;

		NEXT_TOKEN();

		*expr = &node->header;
	}
	else if (token_kind == Token_True || token_kind == Token_False)
	{
		AST_Bool* node = PUSH_NODE(Bool);
		node->value = (token_kind == Token_True);

		NEXT_TOKEN();

		*expr = &node->header;
	}
	else if (token_kind == Token_OpenParen)
	{
		NEXT_TOKEN();

		AST_Header* inner = 0;
		if (!Parser__ParseExpr(state, &inner)) return false;

		if (!EAT_TOKEN(Token_CloseParen))
		{
			//// ERROR: Missing closing paren after compound expr
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Compound* node = PUSH_NODE(Compound);
		ASTPtr_SetToPtr(&node->inner_expr, inner);

		*expr = &node->header;
	}
	else if (token_kind == Token_Proc)
	{
		NEXT_TOKEN();

		if (!EAT_TOKEN(Token_OpenParen))
		{
			//// ERROR: Missing parameter list
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Header* params = 0;

		for (;;)
		{
			proc(a, b, c := A())

			AST_* node = 0;
			node->next = ASTPtr_Nil;
			NOT_IMPLEMENTED;

			if (param_link == 0) params = node;
			else                 ASTPtr_SetToPtr(param_link, node);

			param_link = &node->next;

			if (EAT_TOKEN(Token_Comma)) continue;
			else                        break;
		}

		if (!EAT_TOKEN(Token_CloseParen))
		{
			//// ERROR: Missing closing paren after parameter list
			NOT_IMPLEMENTED;
			return false;
		}

		NOT_IMPLEMENTED;
	}
	else if (token_kind == Token_Struct)
	{
		NOT_IMPLEMENTED;
	}
	else if (token_kind == Token_Enum)
	{
		NOT_IMPLEMENTED;
	}
	else if (token_kind == Token_Dot)
	{
		if (!Parser__ParseDotStuff(state, 0, expr)) return false;
	}
	else
	{
		//// ERROR: Missing primary expression
		NOT_IMPLEMENTED;
		return false;
	}

	return true;
}

static bool
Parser__ParseTypePrefixExpr(Parser* state, AST_Header** expr)
{
	AST_Ptr* link = 0;

	for (;;)
	{
		if (EAT_TOKEN(Token_Hat))
		{
			AST_PointerTo* node = PUSH_NODE(PointerTo);
			node->elem_type = ASTPtr_Nil;

			if (link == 0) *expr = &node->header;
			else           ASTPtr_SetToPtr(link, node);
			
			link = &node->elem_type;
		}
		else if (EAT_TOKEN(Token_OpenBracket))
		{
			if (EAT_TOKEN(Token_CloseBracket))
			{
				AST_SliceOf* node = PUSH_NODE(SliceOf);
				node->elem_type = ASTPtr_Nil;

				if (link == 0) *expr = &node->header;
				else           ASTPtr_SetToPtr(link, node);

				link = &node->elem_type;
			}
			else
			{
				AST_Header* len = 0;

				if (EAT_TOKEN(Token_QMark))
				{
					// NOTE: [?]T signify static arrays of a computed size, these are distinguished from regular arrays by setting the "len" field to nil
					len = 0;
				}
				else
				{
					if (!Parser__ParseExpr(state, &len)) return false;
				}

				if (!EAT_TOKEN(Token_CloseBracket))
				{
					//// ERROR: Missing closing bracket after len in array type prefix
					NOT_IMPLEMENTED;
					return false;
				}

				AST_ArrayOf* node = PUSH_NODE(ArrayOf);
				node->elem_type = ASTPtr_Nil;
				if (len == 0) node->len = ASTPtr_Nil;
				else          ASTPtr_SetToPtr(&node->len, len);

				if (link == 0) *expr = &node->header;
				else           ASTPtr_SetToPtr(link, node);

				link = &node->elem_type;
			}
		}
		else break;
	}

	AST_Header* operand = 0;
	if (!Parser__ParsePrimaryExpr(state, &operand)) return false;

	if (link == 0) *expr = operand;
	else           ASTPtr_SetToPtr(link, operand);

	return true;
}

static bool
Parser__ParsePostfixExpr(Parser* state, AST_Header** expr)
{
	if (!Parser__ParseTypePrefixExpr(state, expr)) return false;

	for (;;)
	{
		if (EAT_TOKEN(Token_Hat))
		{
			AST_Deref* node = PUSH_NODE(Deref);
			ASTPtr_SetToPtr(&node->expr, *expr);
			*expr = &node->header;
		}
		else if (EAT_TOKEN(Token_OpenParen))
		{
			AST_Header* args = 0;
			if (!Parser__ParseArguments(state, &args)) return false;

			if (!EAT_TOKEN(Token_CloseParen))
			{
				//// ERROR: Missing closing paren after args to call expr
				NOT_IMPLEMENTED;
				return false;
			}

			AST_Call* node = PUSH_NODE(Call);
			ASTPtr_SetToPtr(&node->expr, *expr);
			ASTPtr_SetToPtr(&node->args, args);

			*expr = &node->header;
		}
		else if (GET_TOKEN().kind == Token_Dot)
		{
			if (!Parser__ParseDotStuff(state, *expr, expr)) return false;
		}
		else if (EAT_TOKEN(Token_OpenBracket))
		{
			AST_Header* index = 0;
			if (!Parser__ParseTypePrefixExpr(state, &index)) return false;

			if (EAT_TOKEN(Token_Colon))
			{
				AST_Header* past_end_index = 0;
				if (!Parser__ParseTypePrefixExpr(state, &past_end_index)) return false;

				if (!EAT_TOKEN(Token_CloseBracket))
				{
					//// ERROR: Missing closing bracket after slice interval
					NOT_IMPLEMENTED;
					return false;
				}

				AST_Slice* node = PUSH_NODE(Slice);
				ASTPtr_SetToPtr(&node->expr, *expr);
				ASTPtr_SetToPtr(&node->start_index, index);
				ASTPtr_SetToPtr(&node->past_end_index, past_end_index);

				*expr = &node->header;
			}
			else
			{
				if (!EAT_TOKEN(Token_CloseBracket))
				{
					//// ERROR: Missing closing bracket after index
					NOT_IMPLEMENTED;
					return false;
				}

				AST_Index* node = PUSH_NODE(Index);
				ASTPtr_SetToPtr(&node->expr, *expr);
				ASTPtr_SetToPtr(&node->index, index);

				*expr = &node->header;
			}
		}
		else break;
	}

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
			AST_Prefix_Expr* node = PUSH_GROUPED_NODE(AST_Prefix_Expr, kind);

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

		AST_Binary_Expr* node = PUSH_GROUPED_NODE(AST_Binary_Expr, kind);
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

		AST_Conditional* node = PUSH_NODE(Conditional);
		ASTPtr_SetToPtr(&node->condition, condition);
		ASTPtr_SetToPtr(&node->true_val, true_val);
		ASTPtr_SetToPtr(&node->false_val, false_val);
	}

	return true;
}

#undef GET_TOKEN
#undef GET_TOKEN_DATA
#undef NEXT_TOKEN
#undef EAT_TOKEN
#undef PUSH_NODE
#undef PUSH_GROUPED_NODE
