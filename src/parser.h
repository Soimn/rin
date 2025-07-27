typedef struct Parser
{
	Token* token;
	Virtual_Array* ast_array;
	u8* file_contents;
} Parser;

static Token
Parser__GetToken(Parser* state)
{
	return *state->token;
}

static Token
Parser__PeekToken(Parser* state)
{
	// TODO: Make sure this is legal in all cases.
	//       Should not be a problem if Token_EOF is present and the
	//       parser only peeks one token, but I feel like not having
	//       a guarantee is kind of yucky
	return *(state->token + 1);
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
Parser__PushNode(Parser* state, AST_Kind kind, umm size_div_4)
{
	AST_Header* node = VA_PushN(state->ast_array, (u32)size_div_4);

	Zero(node, size_div_4*4);

	node->kind = kind;

	return node;
}

#define GET_TOKEN() Parser__GetToken(state)
#define PEEK_TOKEN() Parser__PeekToken(state)
#define GET_TOKEN_DATA() Parser__GetTokenData(state)
#define NEXT_TOKEN() Parser__NextToken(state)
#define EAT_TOKEN(K) Parser__EatToken(state, (K))
#define PUSH_NODE(N) (AST_##N*)Parser__PushNode(state, ASTKind_##N, sizeof(AST_##N) / 4)
#define PUSH_GROUPED_NODE(T, K) (T*)Parser__PushNode(state, K, sizeof(T) / 4)

static bool Parser__ParseExpr(Parser* state, AST_Header** expr);
static bool Parser__ParseTypePrefixExpr(Parser* state, AST_Header** expr);
static bool Parser__ParseBlock(Parser* state, AST_Linked_Header** block);
static bool Parser__ParseStatement(Parser* state, AST_Linked_Header** statement, bool expect_semicolon);

static bool
Parser__ParseArguments(Parser* state, AST_Linked_Header** args)
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

		if (link == 0) *args = &node->linked_header;
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
		AST_Linked_Header* args = 0;
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
Parser__ParseProc(Parser* state, AST_Header** expr)
{
	ASSERT(GET_TOKEN().kind == Token_Proc);
	NEXT_TOKEN();

	if (!EAT_TOKEN(Token_OpenParen))
	{
		//// ERROR: Missing parameter list
		NOT_IMPLEMENTED;
		return false;
	}

	if (GET_TOKEN().kind == Token_CloseParen)
	{
		//// ERROR: Missing parameters
		NOT_IMPLEMENTED;
		return false;
	}

	AST_Parameter* params = 0;
	if (GET_TOKEN().kind != Token_CloseParen)
	{
		AST_Header* first = 0;
		if (!Parser__ParseExpr(state, &first)) return false;

		if (GET_TOKEN().kind == Token_Comma || GET_TOKEN().kind == Token_CloseParen)
		{
			// NOTE: This is the type only variant for the parameter list i.e. proc(int, float, string)
			//       and not the normal proc(name0: type0, name1: type1)

			AST_Parameter* first_node = PUSH_NODE(Parameter);
			first_node->next  = ASTPtr_Nil;
			first_node->name  = ASTPtr_Nil;
			first_node->value = ASTPtr_Nil;
			ASTPtr_SetToPtr(&first_node->type, first);

			params = first_node;
			AST_Ptr* param_link = &first_node->next;

			for (;;)
			{
				if (GET_TOKEN().kind == Token_Colon)
				{
					//// ERROR: Illegal to mix type only and name, type, val proc parameter variations
					NOT_IMPLEMENTED;
					return false;
				}

				if (!EAT_TOKEN(Token_Comma)) break;

				AST_Header* type = 0;
				if (!Parser__ParseExpr(state, &type)) return false;

				AST_Parameter* node = PUSH_NODE(Parameter);
				node->next  = ASTPtr_Nil;
				node->name  = ASTPtr_Nil;
				node->value = ASTPtr_Nil;
				ASTPtr_SetToPtr(&node->type, type);

				ASTPtr_SetToPtr(param_link, node);
				param_link = &node->next;
			}
		}
		else
		{
			AST_Header* name = first;
			AST_Ptr* param_link = 0;

			for (;;)
			{
				AST_Header* type  = 0;
				AST_Header* value = 0;

				if (!EAT_TOKEN(Token_Colon))
				{
					//// ERROR: Missing type of parameter
					NOT_IMPLEMENTED;
					return false;
				}

				if (GET_TOKEN().kind != Token_Eq)
				{
					if (!Parser__ParseExpr(state, &type)) return false;
				}

				if (EAT_TOKEN(Token_Eq))
				{
					if (!Parser__ParseExpr(state, &value)) return false;
				}

				AST_Parameter* node = PUSH_NODE(Parameter);
				node->next = ASTPtr_Nil;
				ASTPtr_SetToPtr(&node->name, name);
				ASTPtr_SetToPtr(&node->type, type);
				ASTPtr_SetToPtr(&node->value, value);

				if (param_link == 0) params = node;
				else                 ASTPtr_SetToPtr(param_link, node);
				param_link = &node->next;

				if (!EAT_TOKEN(Token_Colon)) break;

				if (!Parser__ParseExpr(state, &name)) return false;
				continue;
			}
		}
	}

	if (!EAT_TOKEN(Token_CloseParen))
	{
		//// ERROR: Missing closing paren after parameter list
		NOT_IMPLEMENTED;
		return false;
	}

	AST_ReturnVal* return_vals = 0;
	{
		if (GET_TOKEN().kind == Token_Minus && PEEK_TOKEN().kind == Token_Gt)
		{
			NEXT_TOKEN();
			NEXT_TOKEN();

			if (GET_TOKEN().kind != Token_OpenParen)
			{
				AST_Header* type = 0;
				if (!Parser__ParseExpr(state, &type)) return false;

				AST_ReturnVal* node = PUSH_NODE(ReturnVal);
				node->next = ASTPtr_Nil;
				node->name = ASTPtr_Nil;
				ASTPtr_SetToPtr(&node->type, type);

				return_vals = node;
			}
			else
			{
				ASSERT(GET_TOKEN().kind == Token_OpenParen);
				NEXT_TOKEN();

				AST_Header* first = 0;
				if (!Parser__ParseExpr(state, &first)) return false;

				if (GET_TOKEN().kind == Token_Comma || GET_TOKEN().kind == Token_CloseParen)
				{
					// NOTE: This is the type only variant for the return val list i.e. (int, float, string)
					//       and not the normal (name0: type0, name1: type1)

					AST_ReturnVal* first_node = PUSH_NODE(ReturnVal);
					first_node->next  = ASTPtr_Nil;
					first_node->name  = ASTPtr_Nil;
					ASTPtr_SetToPtr(&first_node->type, first);

					return_vals = first_node;
					AST_Ptr* return_val_link = &first_node->next;

					for (;;)
					{
						if (GET_TOKEN().kind == Token_Colon)
						{
							//// ERROR: Illegal to mix type only and name, type return value variations
							NOT_IMPLEMENTED;
							return false;
						}

						if (!EAT_TOKEN(Token_Comma)) break;

						AST_Header* type = 0;
						if (!Parser__ParseExpr(state, &type)) return false;

						AST_ReturnVal* node = PUSH_NODE(ReturnVal);
						node->next  = ASTPtr_Nil;
						node->name  = ASTPtr_Nil;
						ASTPtr_SetToPtr(&node->type, type);

						ASTPtr_SetToPtr(return_val_link, node);
						return_val_link = &node->next;
					}
				}
				else
				{
					AST_Header* name = first;
					AST_Ptr* return_val_link = 0;

					for (;;)
					{
						AST_Header* type = 0;

						if (!EAT_TOKEN(Token_Colon))
						{
							//// ERROR: Missing type of return value
							NOT_IMPLEMENTED;
							return false;
						}

						if (GET_TOKEN().kind != Token_Eq)
						{
							if (!Parser__ParseExpr(state, &type)) return false;
						}

						if (GET_TOKEN().kind == Token_Eq)
						{
							//// ERROR: Return values do not support default values
							NOT_IMPLEMENTED;
							return false;
						}

						AST_ReturnVal* node = PUSH_NODE(ReturnVal);
						node->next = ASTPtr_Nil;
						ASTPtr_SetToPtr(&node->name, name);
						ASTPtr_SetToPtr(&node->type, type);

						if (return_val_link == 0) return_vals = node;
						else                      ASTPtr_SetToPtr(return_val_link, node);
						return_val_link = &node->next;

						if (!EAT_TOKEN(Token_Colon)) break;

						if (!Parser__ParseExpr(state, &name)) return false;
						continue;
					}
				}

				if (!EAT_TOKEN(Token_CloseParen))
				{
					//// ERROR: Missing closing paren after return value list
					NOT_IMPLEMENTED;
					return false;
				}
			}
		}
	}

	if (GET_TOKEN().kind != Token_OpenBrace)
	{
		AST_ProcType* node = PUSH_NODE(ProcType);
		ASTPtr_SetToPtr(&node->params, params);
		ASTPtr_SetToPtr(&node->return_vals, return_vals);

		*expr = &node->header;
	}
	else
	{
		ASSERT(GET_TOKEN().kind == Token_OpenBrace);

		AST_Linked_Header* body = 0;
		if (!Parser__ParseBlock(state, &body)) return false;

		AST_ProcLit* node = PUSH_NODE(ProcLit);
		ASTPtr_SetToPtr(&node->params, params);
		ASTPtr_SetToPtr(&node->return_vals, return_vals);
		ASTPtr_SetToPtr(&node->body, body);

		*expr = &node->header;
	}

	return true;
}

static bool
Parser__ParseStruct(Parser* state, AST_Header** expr)
{
	ASSERT(GET_TOKEN().kind == Token_Struct);
	NEXT_TOKEN();

	if (GET_TOKEN().kind != Token_OpenBrace)
	{
		//// ERROR: Missing struct body
		NOT_IMPLEMENTED;
		return false;
	}

	AST_Linked_Header* body = 0;
	if (!Parser__ParseBlock(state, &body)) return false;

	AST_StructType* node = 0;
	ASTPtr_SetToPtr(&node->body, body);

	*expr = &node->header;

	return true;
}

static bool
Parser__ParseEnum(Parser* state, AST_Header** expr)
{
	ASSERT(GET_TOKEN().kind == Token_Enum);
	NEXT_TOKEN();

	AST_Header* elem_type = 0;
	if (GET_TOKEN().kind != Token_OpenBrace)
	{
		if (!Parser__ParseExpr(state, &elem_type)) return false;
	}

	if (GET_TOKEN().kind != Token_OpenBrace)
	{
		//// ERROR: Missing enum body
		NOT_IMPLEMENTED;
		return false;
	}

	AST_Linked_Header* body = 0;
	if (!Parser__ParseBlock(state, &body)) return false;

	AST_EnumType* node = 0;
	ASTPtr_SetToPtr(&node->elem_type, elem_type);
	ASTPtr_SetToPtr(&node->body, body);

	*expr = &node->header;

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
		if (!Parser__ParseProc(state, expr)) return false;
	}
	else if (token_kind == Token_Struct)
	{
		if (!Parser__ParseStruct(state, expr)) return false;
	}
	else if (token_kind == Token_Enum)
	{
		if (!Parser__ParseEnum(state, expr)) return false;
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
		else if (EAT_TOKEN(Token_Distinct))
		{
			AST_DistinctOf* node = PUSH_NODE(DistinctOf);
			node->elem_type = ASTPtr_Nil;

			if (link == 0) *expr = &node->header;
			else           ASTPtr_SetToPtr(link, node);

			link = &node->elem_type;
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
			AST_Linked_Header* args = 0;
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

static bool
Parser__ParseBlock(Parser* state, AST_Linked_Header** block)
{
	ASSERT(GET_TOKEN().kind == Token_OpenBrace);
	NEXT_TOKEN();

	AST_Linked_Header* statements = 0;
	AST_Ptr* statement_link       = 0;

	for (;;)
	{
		// NOTE: Skip stray semicolons before trying to parse a statement because this
		//       avoids special casing the "statement_link" linking
		while (EAT_TOKEN(Token_Semicolon));

		if (EAT_TOKEN(Token_CloseBrace)) break;

		AST_Linked_Header* statement = 0;
		if (!Parser__ParseStatement(state, &statement, true)) return false;

		if (statement_link == 0) statements = statement;
		else                     ASTPtr_SetToPtr(statement_link, statement);

		statement_link = &statement->next;
	}

	AST_Block* node = PUSH_NODE(Block);
	node->next  = ASTPtr_Nil;
	node->label = ASTPtr_Nil;
	ASTPtr_SetToPtr(&node->statements, statements);

	*block = &node->linked_header;

	return true;
}

static bool
Parser__ParseExprList(Parser* state, AST_Linked_Header** exprs)
{
	AST_Ptr* expr_link = 0;

	for (;;)
	{
		AST_Expr* node = PUSH_NODE(Expr);
		node->next = ASTPtr_Nil;

		AST_Header* expr = 0;
		if (!Parser__ParseExpr(state, &expr)) return false;

		ASTPtr_SetToPtr(&node->expr, expr);

		if (expr_link == 0) *exprs = &node->linked_header;
		else                ASTPtr_SetToPtr(expr_link, node);

		expr_link = &node->next;

		if (EAT_TOKEN(Token_Comma)) continue;
		else                        break;
	}

	return true;
}

static bool
Parser__ParseStatement(Parser* state, AST_Linked_Header** statement, bool expect_semicolon)
{
	if (EAT_TOKEN(Token_Colon))
	{
		AST_Header* label = 0;
		if (!Parser__ParseExpr(state, &label)) return false;

		AST_Linked_Header* stmnt = 0;
		if (!Parser__ParseStatement(state, &stmnt, expect_semicolon)) return false;

		if      (stmnt->kind == ASTKind_Block) ASTPtr_SetToPtr(&((AST_Block*)stmnt)->label, label);
		else if (stmnt->kind == ASTKind_If)    ASTPtr_SetToPtr(&((AST_If*)stmnt)->label, label);
		else if (stmnt->kind == ASTKind_For)   ASTPtr_SetToPtr(&((AST_For*)stmnt)->label, label);
		else if (stmnt->kind == ASTKind_While) ASTPtr_SetToPtr(&((AST_While*)stmnt)->label, label);
		else
		{
			//// ERROR: Labels can only be applied to blocks, ifs, for and while loops
			NOT_IMPLEMENTED;
			return false;
		}

		*statement = stmnt;
	}
	else if (GET_TOKEN().kind == Token_OpenBrace)
	{
		if (!Parser__ParseBlock(state, statement)) return false;
	}
	else if (EAT_TOKEN(Token_If))
	{
		if (!EAT_TOKEN(Token_OpenParen))
		{
			//// ERROR: Missing paren before if condition
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Linked_Header* condition = 0;
		if (!Parser__ParseStatement(state, &condition, false)) return false;

		if (!EAT_TOKEN(Token_CloseParen))
		{
			//// ERROR: Missing closing paren after if condition
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Linked_Header* true_branch = 0;
		if (!Parser__ParseStatement(state, &true_branch, true)) return false;

		AST_Linked_Header* false_branch = 0;
		if (EAT_TOKEN(Token_Else))
		{
			if (!Parser__ParseStatement(state, &false_branch, true)) return false;
		}

		AST_If* node = PUSH_NODE(If);
		node->next  = ASTPtr_Nil;
		node->label = ASTPtr_Nil;
		ASTPtr_SetToPtr(&node->condition, condition);
		ASTPtr_SetToPtr(&node->true_branch, true_branch);
		ASTPtr_SetToPtr(&node->false_branch, false_branch);

		*statement = &node->linked_header;
	}
	else if (GET_TOKEN().kind == Token_Else)
	{
		//// ERROR: Illegal else without matching if
		NOT_IMPLEMENTED;
		return false;
	}
	else if (EAT_TOKEN(Token_While))
	{
		if (!EAT_TOKEN(Token_OpenParen))
		{
			//// ERROR: Missing open paren before while loop header
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Linked_Header* init      = 0;
		AST_Linked_Header* condition = 0;
		AST_Linked_Header* step      = 0;

		AST_Linked_Header* first = 0;
		if (!Parser__ParseStatement(state, &first, false)) return false;

		if (GET_TOKEN().kind != Token_Semicolon)
		{
			// NOTE: This is the condition only variant of the while loop i.e. while (a == b)
			condition = first;
		}
		else
		{
			// NOTE: This is the c style for loop variant of the while loop i.e. while (i := 0; i < 10; ++i)
			ASSERT(GET_TOKEN().kind == Token_Semicolon);
			NEXT_TOKEN();

			init = first;

			if (!Parser__ParseStatement(state, &condition, false)) return false;

			if (!EAT_TOKEN(Token_Semicolon))
			{
				//// ERROR: Missing semicolon after condition in init, condition, step variant while loop
				NOT_IMPLEMENTED;
				return false;
			}

			if (!Parser__ParseStatement(state, &step, false)) return false;
		}

		if (!EAT_TOKEN(Token_CloseParen))
		{
			//// ERROR: Missing close paren after while loop header
			NOT_IMPLEMENTED;
			return false;
		}

		NOT_IMPLEMENTED;
	}
	else if (EAT_TOKEN(Token_For))
	{
		// TODO
		NOT_IMPLEMENTED;
	}
	else if (EAT_TOKEN(Token_Break))
	{
		AST_Header* label = 0;
		if (GET_TOKEN().kind != Token_Semicolon)
		{
			if (!Parser__ParseExpr(state, &label)) return false;
		}

		if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
		{
			//// ERROR: Missing terminating semicolon after break statement
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Break* node = PUSH_NODE(Break);
		node->next = ASTPtr_Nil;
		ASTPtr_SetToPtr(&node->label, label);

		*statement = &node->linked_header;
	}
	else if (EAT_TOKEN(Token_Continue))
	{
		AST_Header* label = 0;
		if (GET_TOKEN().kind != Token_Semicolon)
		{
			if (!Parser__ParseExpr(state, &label)) return false;
		}

		if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
		{
			//// ERROR: Missing terminating semicolon after continue statement
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Continue* node = PUSH_NODE(Continue);
		node->next = ASTPtr_Nil;
		ASTPtr_SetToPtr(&node->label, label);

		*statement = &node->linked_header;
	}
	else if (EAT_TOKEN(Token_Return))
	{
		AST_Linked_Header* args = 0;
		if (GET_TOKEN().kind != Token_Semicolon)
		{
			if (!Parser__ParseArguments(state, &args)) return false;
		}

		if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
		{
			//// ERROR: Missing terminating semicolon after return statement
			NOT_IMPLEMENTED;
			return false;
		}

		AST_Return* node = PUSH_NODE(Return);
		node->next = ASTPtr_Nil;
		ASTPtr_SetToPtr(&node->args, args);

		*statement = &node->linked_header;
	}
	else
	{
		AST_Linked_Header* exprs = 0;
		if (!Parser__ParseExprList(state, &exprs)) return false;

		if (EAT_TOKEN(Token_Colon))
		{
			AST_Linked_Header* names  = exprs;
			AST_Linked_Header* types  = 0;
			AST_Linked_Header* values = 0;

			if (GET_TOKEN().kind != Token_Eq && GET_TOKEN().kind != Token_Colon)
			{
				if (!Parser__ParseExprList(state, &types)) return false;
			}

			if (EAT_TOKEN(Token_Colon))
			{
				if (!Parser__ParseExprList(state, &values)) return false;

				if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
				{
					//// ERROR: Missing terminating semicolon after constant declaration
					NOT_IMPLEMENTED;
					return false;
				}

				AST_Constant* node = PUSH_NODE(Constant);
				node->next = ASTPtr_Nil;
				ASTPtr_SetToPtr(&node->names, names);
				ASTPtr_SetToPtr(&node->types, types);
				ASTPtr_SetToPtr(&node->values, values);

				*statement = &node->linked_header;
			}
			else
			{
				if (EAT_TOKEN(Token_Eq))
				{
					if (!Parser__ParseExprList(state, &values)) return false;
				}

				if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
				{
					//// ERROR: Missing terminating semicolon after variable declaration
					NOT_IMPLEMENTED;
					return false;
				}

				AST_Variable* node = PUSH_NODE(Variable);
				node->next = ASTPtr_Nil;
				ASTPtr_SetToPtr(&node->names, names);
				ASTPtr_SetToPtr(&node->types, types);
				ASTPtr_SetToPtr(&node->values, values);

				*statement = &node->linked_header;
			}
		}
		else if (TOKEN_KIND__IS_ASS(GET_TOKEN().kind))
		{
			Token_Kind token_kind = GET_TOKEN().kind;
			NEXT_TOKEN();

			AST_Kind ass_op = ASTKind_Invalid;
			if (token_kind != Token_Eq)
			{
				ass_op = AST_KIND__BINARY_FROM_TOKEN(TOKEN_KIND__ASS_TO_BINARY(token_kind));
			}

			AST_Linked_Header* lhs = exprs;
			AST_Linked_Header* rhs = 0;

			if (!Parser__ParseExprList(state, &rhs)) return false;

			if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
			{
				//// ERROR: Missing terminating semicolon after assignment statement
				NOT_IMPLEMENTED;
				return false;
			}

			AST_Assignment* node = PUSH_NODE(Assignment);
			node->next = ASTPtr_Nil;
			node->op   = ass_op;
			ASTPtr_SetToPtr(&node->lhs, lhs);
			ASTPtr_SetToPtr(&node->rhs, rhs);

			*statement = &node->linked_header;
		}
		else
		{
			*statement = exprs;

			if (expect_semicolon && !EAT_TOKEN(Token_Semicolon))
			{
				//// ERROR: Missing terminating semicolon after expression statement
				NOT_IMPLEMENTED;
				return false;
			}
		}
	}

	return true;
}

static bool
ParseFile(u8* file_contents, Token* tokens, u32 tokens_len, Virtual_Array* ast_array, AST_Linked_Header** ast)
{
	ASSERT(tokens_len > 0 && tokens[tokens_len-1].kind == Token_EOF);

	Parser* state = &(Parser){
		.token         = tokens,
		.ast_array     = ast_array,
		.file_contents = file_contents,
	};

	AST_Ptr* statement_link = 0;

	for (;;)
	{
		// NOTE: Skip stray semicolons before trying to parse a statement because this
		//       avoids special casing the "statement_link" linking
		while (EAT_TOKEN(Token_Semicolon));

		if (GET_TOKEN().kind == Token_EOF) break;

		AST_Linked_Header* statement = 0;
		if (!Parser__ParseStatement(state, &statement, true)) return false;

		if (statement_link == 0) *ast = statement;
		else                     ASTPtr_SetToPtr(statement_link, statement);

		statement_link = &statement->next;
	}

	return true;
}

#undef GET_TOKEN
#undef PEEK_TOKEN
#undef GET_TOKEN_DATA
#undef NEXT_TOKEN
#undef EAT_TOKEN
#undef PUSH_NODE
#undef PUSH_GROUPED_NODE
