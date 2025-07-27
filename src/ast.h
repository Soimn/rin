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
	ASTKind_Int128,
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
	ASTKind_DistinctOf,
	ASTKind__PastLastTypePrefixExpr,

	ASTKind__FirstPostfixExpr,
	ASTKind_Deref = ASTKind__FirstPostfixExpr,
	ASTKind_Call,
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

	ASTKind_Argument,
	ASTKind_Parameter,
	ASTKind_ReturnVal,

	ASTKind__FirstStatement,
	ASTKind_Block = ASTKind__FirstStatement,
	ASTKind_If,
	ASTKind_While,
	ASTKind_For,
	ASTKind_Break,
	ASTKind_Continue,
	ASTKind_Return,
	ASTKind_Expr,
	ASTKind_Assignment,
	ASTKind__PastLastStatement,

	ASTKind__FirstDeclaration,
	ASTKind_Constant = ASTKind__FirstDeclaration,
	ASTKind_Variable,
	ASTKind__PastLastDeclaration,
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

typedef __declspec(align(4)) struct AST_Linked_Header
{
	AST_HEADER;
	AST_Ptr next;
} AST_Linked_Header;

#define AST_LINKED_HEADER union { struct AST_Linked_Header; AST_Linked_Header linked_header; }

// -- Primary expressions

#pragma pack(push, 4)
typedef struct AST_Ident
{
	AST_HEADER;
	u32 len;
	u8* data;
} AST_Ident;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct AST_String
{
	AST_HEADER;
	u32 len;
	u8* data;
} AST_String;
#pragma pack(pop)

typedef struct AST_Char
{
	AST_HEADER;
	u8 value;
} AST_Char;

#pragma pack(push, 4)
typedef struct AST_Int
{
	AST_HEADER;
	u64 value;
} AST_Int;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct AST_Int128
{
	AST_HEADER;
	u64 value_lo;
	u64 value_hi;
} AST_Int128;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct AST_Float
{
	AST_HEADER;
	f64 value;
} AST_Float;
#pragma pack(pop)

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

typedef struct AST_ProcType
{
	AST_HEADER;
	AST_Ptr params;
	AST_Ptr return_vals;
} AST_ProcType;

typedef struct AST_ProcLit
{
	AST_HEADER;
	AST_Ptr params;
	AST_Ptr return_vals;
	AST_Ptr body;
} AST_ProcLit;

typedef struct AST_StructType
{
	AST_HEADER;
	AST_Ptr body;
} AST_StructType;

typedef struct AST_EnumType
{
	AST_HEADER;
	AST_Ptr elem_type;
	AST_Ptr body;
} AST_EnumType;

// -- Type prefix level

typedef struct AST_PointerTo
{
	AST_HEADER;
	AST_Ptr elem_type;
} AST_PointerTo;

typedef struct AST_SliceOf
{
	AST_HEADER;
	AST_Ptr elem_type;
} AST_SliceOf;

typedef struct AST_ArrayOf
{
	AST_HEADER;
	AST_Ptr len;
	AST_Ptr elem_type;
} AST_ArrayOf;

typedef struct AST_DistinctOf
{
	AST_HEADER;
	AST_Ptr elem_type;
} AST_DistinctOf;

// -- Postfix

typedef struct AST_Deref
{
	AST_HEADER;
	AST_Ptr expr;
} AST_Deref;

typedef struct AST_Call
{
	AST_HEADER;
	AST_Ptr expr;
	AST_Ptr args;
} AST_Call;

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

typedef struct AST_StructLit
{
	AST_HEADER;
	AST_Ptr type;
	AST_Ptr args;
} AST_StructLit;

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

// --

typedef struct AST_Argument
{
	AST_LINKED_HEADER;
	AST_Ptr name;
	AST_Ptr value;
} AST_Argument;

typedef struct AST_Parameter
{
	AST_LINKED_HEADER;
	AST_Ptr name;
	AST_Ptr type;
	AST_Ptr value;
} AST_Parameter;

typedef struct AST_ReturnVal
{
	AST_LINKED_HEADER;
	AST_Ptr name;
	AST_Ptr type;
} AST_ReturnVal;

// -- Statements
typedef struct AST_Block
{
	AST_LINKED_HEADER;
	AST_Ptr label;
	AST_Ptr statements;
} AST_Block;

typedef struct AST_If
{
	AST_LINKED_HEADER;
	AST_Ptr label;
	AST_Ptr condition;
	AST_Ptr true_branch;
	AST_Ptr false_branch;
} AST_If;

typedef struct AST_While
{
	AST_LINKED_HEADER;
	AST_Ptr label;
	AST_Ptr init;
	AST_Ptr condition;
	AST_Ptr step;
	AST_Ptr body;
} AST_While;

typedef struct AST_For
{
	AST_LINKED_HEADER;
	AST_Ptr label;
	// TODO
} AST_For;

typedef struct AST_Break
{
	AST_LINKED_HEADER;
	AST_Ptr label;
} AST_Break;

typedef struct AST_Continue
{
	AST_LINKED_HEADER;
	AST_Ptr label;
} AST_Continue;

typedef struct AST_Return
{
	AST_LINKED_HEADER;
	AST_Ptr args;
} AST_Return;

typedef struct AST_Expr
{
	AST_LINKED_HEADER;
	AST_Ptr expr;
} AST_Expr;

typedef struct AST_Assignment
{
	AST_LINKED_HEADER;
	AST_Kind op;
	AST_Ptr lhs;
	AST_Ptr rhs;
} AST_Assignment;

// -- Declarations
typedef struct AST_Constant
{
	AST_LINKED_HEADER;
	AST_Ptr names;
	AST_Ptr types;
	AST_Ptr values;
} AST_Constant;

typedef struct AST_Variable
{
	AST_LINKED_HEADER;
	AST_Ptr names;
	AST_Ptr types;
	AST_Ptr values;
} AST_Variable;
