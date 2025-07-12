typedef enum AST_Kind
{
	AST_Invalid = 0,

	AST__FirstPrimaryExpr,
	AST_Ident = AST__FirstPrimaryExpr,
	AST_String,
	AST_Char,
	AST_Int,
	AST_Float,
	AST_Bool,
	AST_Compound,
	AST_ProcType,
	AST_ProcLit,
	AST_StructType,
	AST_StructLit,
	AST_EnumType,
	AST_ArrayLit,
	AST_Cast,
	AST__PastLastPrimaryExpr,

	AST__FirstPostfixExpr,
	AST_Deref = AST__FirstPostfixExpr,
	AST_Index,
	AST_Slice,
	AST_Member,
	AST__PastLastPostfixExpr,

	AST__FirstPrefixExpr,
	AST_Pos = AST__FirstPrefixExpr,
	AST_Neg,
	AST_Not,
	AST_LNot,
	AST_Ref,
	AST_PointerTo,
	AST_SliceOf,
	AST_ArrayOf,
	AST__PastLastPrefixExpr,
} AST_Kind;

AST_Kind :: enum
{
	Invalid :: 0;

	_FirstPrimary;
	Ident :: _FirstPrimary;
	String;
	Char;
	Int;
	
	when T == int
	{
		Float;
		Bool;
	}
	else
	{
		F32;
		F64;
		B32;
	}

	Compound;
	ProcType;
	ProcLit;
	StructType;
	StructLit;
	EnumType;
	ArrayLit;
	Cast
	_PastLastPrimaryExpr;
}
