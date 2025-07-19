#define AST_KIND__BINARY_PRECEDENCE_LEVELS 5
#define AST_KIND__BINARY_FROM_TOKEN(K) K
#define AST_KIND__BINARY_PRECEDENCE(K) TOKEN_KIND__PRECEDENCE(K)

typedef enum AST_Kind
{
	ASTKind_Invalid = 0,

	ASTKind__FirstExpr,
	ASTKind__FirstPrimaryExpr = ASTKind__FirstExpr,
	ASTKind_Ident = ASTKind__FirstPrimaryExpr,
	ASTKind_String,
	ASTKind_Char,
	ASTKind_Int,
	ASTKind_Float,
	ASTKind_Bool,
	ASTKind_Compound,
	ASTKind_ProcType,
	ASTKind_ProcLit,
	ASTKind_StructType,
	ASTKind_EnumType,
	ASTKind__PastLastPrimaryExpr,

	ASTKind__FirstTypePrefixExpr,
	ASTKind_PointerTo = ASTKind__FirstTypePrefixExpr,
	ASTKind_SliceOf,
	ASTKind_ArrayOf,
	ASTKind__PastLastTypePrefixExpr,

	ASTKind__FirstPostfixExpr,
	ASTKind_Deref = ASTKind__FirstPostfixExpr,
	ASTKind_Index,
	ASTKind_Slice,
	ASTKind_Member,
	ASTKind_StructLit,
	ASTKind__PastLastPostfixExpr,

	ASTKind__FirstPrefixExpr,
	ASTKind_Pos = ASTKind__FirstPrefixExpr,
	ASTKind_Neg,
	ASTKind_Not,
	ASTKind_LNot,
	ASTKind_Ref,
	ASTKind_Inc,
	ASTKind_Dec,
	ASTKind__PastLastPrefixExpr,

	ASTKind__FirstBinaryExpr = Token__FirstMulLevel,
	ASTKind__FirstMulLevelExpr = ASTKind__FirstBinaryExpr,
	ASTKind_Mul = ASTKind__FirstMulLevelExpr,
	ASTKind_Div,
	ASTKind_Rem,
	ASTKind_And,
	ASTKind_Shl,
	ASTKind_Shr,
	ASTKind__PastLastMulLevelExpr,

	ASTKind__FirstAddLevelExpr = Token__FirstAddLevel,
	ASTKind_Add = ASTKind__FirstAddLevelExpr,
	ASTKind_Sub,
	ASTKind_Or,
	ASTKind_Xor,
	ASTKind__PastLastAddLevelExpr,

	ASTKind__FirstCmpLevelExpr = Token__FirstCmpLevel,
	ASTKind_CmpEq = ASTKind__FirstCmpLevelExpr,
	ASTKind_CmpNeq,
	ASTKind_CmpLt,
	ASTKind_CmpLtEq,
	ASTKind_CmpGt,
	ASTKind_CmpGtEq,
	ASTKind__PastLastCmpLevelExpr,

	ASTKind__FirstLAndLevelExpr = Token__FirstAndLevel,
	ASTKind__LAnd = ASTKind__FirstLAndLevelExpr,
	ASTKind__PastLastLAndLevelExpr,

	ASTKind__FirstLOrLevelExpr = Token__FirstOrLevel,
	ASTKind__LOr = ASTKind__FirstLOrLevelExpr,
	ASTKind__PastLastLOrLevelExpr,
	ASTKind__PastLastBinaryExpr = ASTKind__PastLastLOrLevelExpr,

	ASTKind_Conditional,
	ASTKind__PastLastExpr,
} AST_Kind;

#if 1
typedef struct { s32 off; } AST_Ptr;

#define ASTPtr_ToPtr(ASTPTR) (void*)((ASTPTR) + (ASTPTR)->off)

#define ASTPtr_FromPtr(ASTPTR, PTR) ((AST_Ptr){ (u32)((AST_Ptr*)(PTR) - (ASTPTR)) })

#define ASTPtr_SetToPtr(ASTPTR, PTR) (*(ASTPTR) = ASTPtr_FromPtr(ASTPTR, PTR))

#define ASTPtr_Nil (AST_Ptr){0}
#else
typedef void* AST_Ptr;

#define ASTPtr_ToPtr(ASTPTR) (void*)(ASTPTR)

#define ASTPtr_FromPtr(ASTPTR, PTR) (void*)(PTR)

#define ASTPtr_SetToPtr(ASTPTR, PTR) (*(ASTPTR) = (PTR))

#define ASTPtr_Nil 0
#endif

typedef __declspec(align(4)) struct AST_Header
{
	u16 kind;
} AST_Header;

#define AST_HEADER union { struct AST_Header; AST_Header header; }

// -- Primary expressions

typedef struct AST_Ident
{
	AST_HEADER;
	// TODO: How to store idents?
} AST_Ident;

typedef struct AST_String
{
	AST_HEADER;
	// TODO: How to store strings?
} AST_String;

typedef struct AST_Char
{
	AST_HEADER;
	u8 value;
} AST_Char;

typedef struct AST_Int
{
	AST_HEADER;
	u64 value;
} AST_Int;

typedef struct AST_Float
{
	AST_HEADER;
	f64 value;
} AST_Float;

typedef struct AST_Bool
{
	AST_HEADER;
	bool value;
} AST_Bool;

typedef struct AST_Compound
{
	AST_HEADER;
	AST_Ptr inner_expr;
} AST_Compound;

typedef struct AST_Proc_Type
{
	AST_HEADER;
	// TODO: params
	// TODO: return values
} AST_Proc_Type;

typedef struct AST_Proc_Lit
{
	AST_HEADER;
	// TODO: params
	// TODO: return values
	// TODO: body
} AST_Proc_Lit;

typedef struct AST_Struct_Type
{
	AST_HEADER;
	// TODO: members
} AST_Struct_Type;

typedef struct AST_Enum_Type
{
	AST_HEADER;
	// TODO: members
} AST_Enum_Type;

// -- Type prefix level

typedef struct AST_Pointer_To
{
	AST_HEADER;
	AST_Ptr elem_type;
} AST_Pointer_To;

typedef struct AST_Slice_Of
{
	AST_HEADER;
	AST_Ptr elem_type;
} AST_Slice_Of;

typedef struct AST_Array_Of
{
	AST_HEADER;
	AST_Ptr len;
	AST_Ptr elem_type;
} AST_Array_Of;

// -- Postfix

typedef struct AST_Deref
{
	AST_HEADER;
	AST_Ptr expr;
} AST_Deref;

typedef struct AST_Index
{
	AST_HEADER;
	AST_Ptr expr;
	AST_Ptr index;
} AST_Index;

typedef struct AST_Slice
{
	AST_HEADER;
	AST_Ptr expr;
	AST_Ptr start_index;
	AST_Ptr past_end_index;
} AST_Slice;

typedef struct AST_Member
{
	AST_HEADER;
	AST_Ptr expr;
	AST_Ptr name;
} AST_Member;

typedef struct AST_Struct_Lit
{
	AST_HEADER;
	AST_Ptr type;
	// TODO: arguments
} AST_Struct_Lit;

// -- Prefix level

typedef struct AST_Prefix_Expr
{
	AST_HEADER;
	AST_Ptr operand;
} AST_Prefix_Expr;

// -- Binary level

typedef struct AST_Binary_Expr
{
	AST_HEADER;
	AST_Ptr lhs;
	AST_Ptr rhs;
} AST_Binary_Expr;

// -- Ternary level
typedef struct AST_Conditional
{
	AST_HEADER;
	AST_Ptr condition;
	AST_Ptr true_val;
	AST_Ptr false_val;
} AST_Conditional;
