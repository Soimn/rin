#define AST_KIND__BLOCK(N) ((N) << 5)

typedef enum AST_Kind
{
	AST_Invalid = 0,

  AST__FirstExpr,
  AST__FirstPrimitiveLevel = AST__FirstExpr,
  AST_Ident = AST__FirstPrimitiveLevel,
  AST_String,
	AST_Char,
  AST_Int,
  AST_Float,
  AST_Bool,
  AST_Compound,
  AST_ProcType,
  AST_StructType,
  AST_ProcLit,
  AST__PastLastPrimitiveLevel,

  AST__FirstPostfixLevel = AST_KIND__BLOCK(1),
  AST_Deref = AST__FirstPostfixLevel,
  AST_Call,
  AST_Slice,
  AST_Index,
  AST_Member,
  AST_StructLit,
  AST__PastLastPostfixLevel,

  AST__FirstPrefixLevel = AST_KIND__BLOCK(2),
  AST_Pos = AST__FirstPrefixLevel,
  AST_Neg,
  AST_LNot,
  AST_Not,
  AST_Ref,
  AST_PointerTo,
  AST_SliceOf,
  AST_ArrayOf,
  AST__PastLastPrefixLevel,

  AST__FirstBinaryExpr = AST_KIND__BLOCK(3),
  AST__FirstMulLevel = AST__FirstBinaryExpr,
  AST_Mul = AST__FirstMulLevel,
  AST_Div,
  AST_Rem,
  AST_And,
  AST_Shl,
  AST_Shr,
  AST_Sar,
  AST__PastLastMulLevel,

  AST__FirstAddLevel = AST_KIND__BLOCK(4),
  AST_Add = AST__FirstAddLevel,
  AST_Sub,
  AST_Or,
  AST_Xor,
  AST__PastLastAddLevel,

  AST__FirstCmpLevel = AST_KIND__BLOCK(5),
  AST_CmpEq = AST__FirstCmpLevel,
  AST_CmpNeq,
  AST_CmpLe,
  AST_CmpLeEq,
  AST_CmpGe,
  AST_CmpGeEq,
  AST__PastLastCmpLevel,

  AST__FirstLAndLevel = AST_KIND__BLOCK(6),
  AST_LAnd = AST__FirstLAndLevel,
  AST__PastLastLAndLevel,

  AST__FirstLOrLevel = AST_KIND__BLOCK(7),
  AST_LOr = AST__FirstLOrLevel,
  AST__PastLastLOrLevel,

  AST__PastLastBinaryExpr = AST_KIND__BLOCK(8),

  AST_Conditional,

  AST__PastLastExpr,

  AST__FirstDecl,
  AST_Var = AST__FirstDecl,
  AST_Const,
  AST__PastLastDecl,

  AST__FirstStmnt,
  AST_Block = AST__FirstStmnt,
  AST_If,
  AST_While,
  AST_Return,
  AST_Break,
  AST_Continue,

  AST__FirstAssignment = AST_KIND__BLOCK(8 + 3),
  AST__FirstMulLevelAssignment = AST__FirstAssignment,
  AST_MulEq = AST__FirstMulLevelAssignment,
  AST_DivEq,
  AST_RemEq,
  AST_AndEq,
  AST_ShlEq,
  AST_ShrEq,
  AST_SarEq,
  AST__PastLastMulLevelAssignment,

  AST__FirstAddLevelAssignment = AST_KIND__BLOCK(8 + 4),
  AST_AddEq = AST__FirstAddLevelAssignment,
  AST_SubEq,
  AST_OrEq,
  AST_XorEq,
  AST__PastLastAddLevelAssignment,

  // NOTE: 8 + 6 is not used since there are no cmp assignment expressions

  AST__FirstLAndLevelAssignment = AST_KIND__BLOCK(8 + 6),
  AST_LAndEq = AST__FirstLAndLevelAssignment,
  AST__PastLastLAndLevelAssignment,

  AST__FirstLOrLevelAssignment = AST_KIND__BLOCK(8 + 7),
  AST_LOrEq = AST__FirstLOrLevelAssignment,
  AST__PastLastLOrLevelAssignment,

  AST_EQ = AST_KIND__BLOCK(8 + 8),
  AST__PastLastAssignment,
  AST__PastLastStmnt = AST__PastLastAssignment,

  AST_Arg,
  AST__FirstSpecial = AST_Arg,
  AST_Param,
  AST_RetType,
  AST__PastLastSpecial,
} AST_Kind;

typedef u32 AST_RelPtr;

typedef struct AST_Header
{
  AST_Kind kind;
  AST_RelPtr next; // TODO: see if this is actually usefull
} AST_Header;

#define AST_HEADER union { struct AST_Header; AST_Header header; }


// -- Primary expressions

typedef struct AST_Ident_Expr
{
  AST_HEADER;
	// TODO
} AST_Ident_Expr;

typedef struct AST_String_Expr
{
  AST_HEADER;
	// TODO
} AST_String_Expr;

typedef struct AST_Char_Expr
{
  AST_HEADER;
	// TODO
} AST_Char_Expr;

typedef struct AST_Int_Expr
{
  AST_HEADER;
  u64 value;
} AST_Int_Expr;

typedef struct AST_Float_Expr
{
  AST_HEADER;
  f64 value;
} AST_Float_Expr;

typedef struct AST_Bool_Expr
{
  AST_HEADER;
  bool value;
} AST_Bool_Expr;

typedef struct AST_Compound_Expr
{
  AST_HEADER;
  AST_RelPtr inner;
} AST_Compound_Expr;

typedef struct AST_Proc_Type_Expr
{
  AST_HEADER;
  AST_RelPtr params;
  AST_RelPtr ret_types;
} AST_Proc_Type_Expr;

typedef struct AST_Proc_Lit_Expr
{
  AST_HEADER;
  AST_RelPtr params;
  AST_RelPtr ret_types;
  AST_RelPtr body; // NOTE: if body is 0 this is a forward decl
} AST_Proc_Lit_Expr;

typedef struct AST_Struct_Type_Expr
{
  AST_HEADER;
  AST_RelPtr body;
} AST_Struct_Type_Expr;


// -- Postix expressions

typedef struct AST_Postfix_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
} AST_Postfix_Expr;

typedef struct AST_Index_Expr
{
  AST_HEADER;
  AST_RelPtr array;
  AST_RelPtr idx;
} AST_Index_Expr;

typedef struct AST_Slice_Expr
{
  AST_HEADER;
  AST_RelPtr array;
  AST_RelPtr start_idx;
  AST_RelPtr past_last_idx;
} AST_Slice_Expr;

// NOTE: Call or Cast
typedef struct AST_Call_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
  AST_RelPtr args;
} AST_Call_Expr;

typedef struct AST_Member_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
  AST_RelPtr name;
} AST_Member_Expr;

typedef struct AST_Struct_Lit_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
  AST_RelPtr args;
} AST_Struct_Lit_Expr;


// -- Prefix expressions

typedef struct AST_Prefix_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
} AST_Prefix_Expr;

typedef struct AST_Array_Of_Expr
{
  AST_HEADER;
  AST_RelPtr operand;
  AST_RelPtr size;
} AST_Array_Of_Expr;


// -- Binary expressions

typedef struct AST_Binary_Expr
{
  AST_HEADER;
  AST_RelPtr left;
  AST_RelPtr right;
} AST_Binary_Expr;


// -- Ternary expressions

typedef struct AST_Conditional_Expr
{
  AST_HEADER;
  AST_RelPtr condition;
  AST_RelPtr true_expr;
  AST_RelPtr false_expr;
} AST_Conditional_Expr;


// -- Declarations

typedef struct AST_Var_Decl
{
  AST_HEADER;
  AST_RelPtr names;
  AST_RelPtr type;
  AST_RelPtr values;
  bool is_uninitialized;
} AST_Var_Decl;

typedef struct AST_Const_Decl
{
  AST_HEADER;
  AST_RelPtr names;
  AST_RelPtr type;
  AST_RelPtr values;
} AST_Const_Decl;


// -- Statements

typedef struct AST_Block_Stmnt
{
  AST_HEADER;
  AST_RelPtr label;
  AST_RelPtr body;
} AST_Block_Stmnt;

typedef struct AST_If_Stmnt
{
  AST_HEADER;
  AST_RelPtr label;
  AST_RelPtr condition;
  AST_RelPtr true_body;
  AST_RelPtr false_body;
} AST_If_Stmnt;

typedef struct AST_While_Stmnt
{
  AST_HEADER;
  AST_RelPtr label;
  AST_RelPtr condition;
  AST_RelPtr body;
} AST_While_Stmnt;

typedef struct AST_Return_Stmnt
{
  AST_HEADER;
  AST_RelPtr args;
} AST_Return_Stmnt;

typedef struct AST_Break_Stmnt
{
  AST_HEADER;
  AST_RelPtr label;
} AST_Break_Stmnt;

typedef struct AST_Continue_Stmnt
{
  AST_HEADER;
  AST_RelPtr label;
} AST_Continue_Stmnt;

typedef struct AST_Assignment_Stmnt
{
  AST_HEADER;
  AST_RelPtr lhs;
  AST_RelPtr rhs;
} AST_Assignment_Stmnt;


// -- Special

typedef struct AST_Argument
{
  AST_HEADER;
  AST_RelPtr value;
  AST_RelPtr name;
} AST_Argument;

typedef struct AST_Parameter
{
  AST_HEADER;
  AST_RelPtr names;
  AST_RelPtr type;
  AST_RelPtr value;
} AST_Parameter;

typedef struct AST_Return_Type
{
  AST_HEADER;
  AST_RelPtr names;
  AST_RelPtr type;
  AST_RelPtr value;
} AST_Return_Type;
