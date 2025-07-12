typedef struct Parser
{
	Token* tokens;
	u32 token_cur;
} Parser;

static bool
Parser__ParsePrefixExpression(Parser* state)
{
}

static bool
Parser__ParseExpression(Parser* state)
{
	if (!Parser__ParsePrefixExpression(state)) return false;

	while (is_bianry)
	{
		// consume token

		if (!Parser__ParsePrefixExpression(state)) return false;


	}

	return true;
}
