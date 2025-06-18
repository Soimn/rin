typedef struct Lexer
{
	u8* cursor;
} Lexer;

Token
Lexer_NextToken(Lexer* lexer)
{
	/// Skip all whitespace and comments
	{
		for (;;)
		{
			while (*lexer->cursor == ' ' || *lexer->cursor == '\t' || *lexer->cursor == '\r') ++lexer->cursor;

			if (*lexer->cursor == 0) break;
			else if (*lexer->cursor == '\n')
			{
			}
		}
	}
}


Source_File
LexFile(u8* contents, Token_Array* tokens)
{
}
