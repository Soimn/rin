typedef struct Checker
{
} Checker;

typedef struct Check_Result
{
	bool valid;
} Check_Result;

#define CHECK_FAILED (Check_Result){ .valid = false }

static ---
Checker__CheckExpr(Checker* state, AST_Header* expr)
{
	if (expr->kind >= ASTKind__FirstBinaryExpr && expr->kind < ASTKind__PastLastBinaryExpr)
	{
		switch (expr->kind)
		{
			case ASTKind_Mul:
			case ASTKind_Div:
			case ASTKind_Add:
			case ASTKind_Sub:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Rem:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_And:
			case ASTKind_Or:
			case ASTKind_Xor:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_Shl:
			case ASTKind_Shr:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_CmpEq:
			case ASTKind_CmpNeq:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_CmpLt:
			case ASTKind_CmpLtEq:
			case ASTKind_CmpGt:
			case ASTKind_CmpGtEq:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_LAnd:
			case ASTKind_LOr:
			{
				NOT_IMPLEMENTED;
			} break;

			default:
			{
				//// ERROR: Illegal AST kind
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			} break;
		}
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
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_SliceOf:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_ArrayOf:
			{
				NOT_IMPLEMENTED;
			} break;

			case ASTKind_DistinctOf:
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
			{
				NOT_IMPLEMENTED;
			} break;

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
				//// ERROR: Illegal AST kind
				NOT_IMPLEMENTED;
				return CHECK_FAILED;
			} break;
		}
	}
}
