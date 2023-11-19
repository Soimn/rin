#define EXPR_BLOCK(IDX) ((IDX) << 4)

typedef enum Expr_Kind
{
	Expr_Invalid = 0,

	Expr__FirstTypePrefix,
	Expr_PointerTo = Expr__FirstTypePrefix,
	Expr_SliceOf,
	Expr_ArrayOf,
	Expr__PastLastTypePrefix,

	Expr__FirstPostfix,
	Expr_Subscript = Expr__FirstPostfix,
	Expr_Slice,
	Expr_Call,
	Expr_Member,
	Expr_Deref,
	Expr_StructLit,
	Expr_ArrayLit,
	Expr__PastLastPostfix,

	Expr__FirstPrefix,
	Expr_Pos = Expr__FirstPrefix,
	Expr_Neg,
	Expr_BitNot,
	Expr_Not,
	Expr_Ref,
	Expr__PastLastPrefix,

	Expr__FirstBinary = EXPR_BLOCK(2),
	Expr__FirstMulLevel = Expr__FirstBinary,
	Expr_Mul = Expr__FirstMulLevel,
	Expr_Div,
	Expr_Mod,
	Expr_BitAnd,
	Expr_BitShr,
	Expr_BitSar,
	Expr_BitShl,
	Expr__PastLastMulLevel,

	Expr__FirstAddLevel = EXPR_BLOCK(3),
	Expr_Add = Expr__FirstAddLevel,
	Expr_Sub,
	Expr_BitOr,
	Expr_BitXor,
	Expr__PastLastAddLevel,

	Expr__FirstCmpLevel = EXPR_BLOCK(4),
	Expr_CmpEQ = Expr__FirstCmpLevel,
	Expr_CmpNEQ,
	Expr_CmpLe,
	Expr_CmpLeEQ,
	Expr_CmpGe,
	Expr_CmpGeEQ,
	Expr__PastLastCmpLevel,

	Expr__FirstAndLevel = EXPR_BLOCK(5),
	Expr_And = Expr__FirstAndLevel,
	Expr__PastLastAndLevel,

	Expr__FirstOrLevel = EXPR_BLOCK(6),
	Expr_Or = Expr__FirstOrLevel,
	Expr__PastLastOrLevel,
	Expr__PastLastBinary = Expr__PastLastOrLevel,

	Expr__FirstCondLevel,
	Expr_Cond = Expr__FirstCondLevel,
	Expr__PastLastCondLevel,

	Expr_Ident,
	Expr_String,
	Expr_Int, // TODO:
	Expr_Comp,
} Expr_Kind;

STATIC_ASSERT(Expr__PastLastPostfix  <= Expr__FirstBinary);
STATIC_ASSERT(Expr__PastLastMulLevel <= Expr__FirstAddLevel);
STATIC_ASSERT(Expr__PastLastAddLevel <= Expr__FirstCmpLevel);
STATIC_ASSERT(Expr__PastLastCmpLevel <= Expr__FirstAndLevel);
STATIC_ASSERT(Expr__PastLastAndLevel <= Expr__FirstOrLevel);

typedef struct Expr Expr;
typedef struct Expr
{
	Expr_Kind kind;

	union
	{
		Identifier ident_expr;
		String_Lit string_expr;

		Expr* unary_expr;

		struct 
		{
			Expr* left;
			Expr* right;
		} binary_expr;

		struct
		{
			Expr* type;
			Expr* size;
		} array_of_expr;

		struct
		{
			Expr* array;
			Expr* idx;
		} subcript_expr;

		struct
		{
			Expr* array;
			Expr* start_idx;
			Expr* past_end_idx;
		} slice_expr;

		struct
		{
			Expr* pointer;
			// args
		} call_expr;

		struct
		{
			Expr* collection;
			Expr* member;
		} member_expr;

		struct
		{
			Expr* type;
			// args
		} struct_lit_expr;

		struct
		{
			Expr* type;
			// args
		} array_lit_expr;
	};
} Expr;

typedef enum Stmnt_Kind
{
	Stmnt_Block,
	Stmnt_Var,
	Stmnt_Const,
	Stmnt_If,
	Stmnt_While,
	Stmnt_Break,
	Stmnt_Continue,
	Stmnt_Return,
	Stmnt_Expr,
	Stmnt_Assign,
} Stmnt_Kind;

typedef struct Stmnt Stmnt;
typedef struct Stmnt
{
	Stmnt_Kind kind;
} Stmnt;
