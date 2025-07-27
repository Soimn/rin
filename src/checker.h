typedef struct Checker
{
	bool stub;
} Checker;

typedef struct Check_Result
{
	Typeid type;
	// type
	// value kind (l or r)
	// value
	// succeeded
} Check_Result;

static Check_Result
Checker__CheckExpr(Checker* state, AST_Header* expr)
{
	if (expr->kind >= ASTKind__FirstBinaryExpr && expr->kind < ASTKind__PastLastBinaryExpr)
	{
		NOT_IMPLEMENTED;
	}
	else
	{
		switch (expr->kind)
		{
			case ASTKind_Ident:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_String:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Char:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Int:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Int128:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Float:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Bool:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Compound:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_ProcType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_ProcLit:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_StructType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_EnumType:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_PointerTo:
			case ASTKind_SliceOf:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_ArrayOf:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Deref:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Call:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Index:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Slice:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Member:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_StructLit:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Pos:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Neg:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Not:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_LNot:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Ref:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Inc:
			case ASTKind_Dec:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Conditional:
			{
				NOT_IMPLEMENTED;
			} break;

			default:
			{
				//// ERROR: This should never happen with parsed input
				NOT_IMPLEMENTED;
			} break;
		}
	}
}
