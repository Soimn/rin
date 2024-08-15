#define AST_KIND__BLOCK_SIZE_LG2 6
#define AST_KIND__BLOCK(N) ((N) << AST_KIND__BLOCK_SIZE_LG2)
#define AST_KIND__BLOCK_IDX(N) ((N) >> AST_KIND__BLOCK_SIZE_LG2)
#define AST_KIND__BLOCK_OFFSET(N) ((N) & ~(AST_KIND__BLOCK_SIZE_LG2-1))
#define AST_KIND__FirstBinaryBlockIDX 3
#define AST_KIND__PastLastBinaryBlockIDX 8

typedef enum AST_Kind
{
  AST_Invalid = 0,

  AST__FirstExpr,
  AST__FirstPrimitiveLevel = AST__FirstExpr,
  AST_Ident,
  AST_String,
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
  AST_CmpEQ = AST__FirstCmpLevel,
  AST_CmpNeq,
  AST_CmpLe,
  AST_CmpLeEQ,
  AST_CmpGe,
  AST_CmpGeEQ,
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
  AST_MulEQ = AST__FirstMulLevelAssignment,
  AST_DivEQ,
  AST_RemEQ,
  AST_AndEQ,
  AST_ShlEQ,
  AST_ShrEQ,
  AST_SarEQ,
  AST__PastLastMulLevelAssignment,

  AST__FirstAddLevelAssignment = AST_KIND__BLOCK(8 + 4),
  AST_AddEQ = AST__FirstAddLevelAssignment,
  AST_SubEQ,
  AST_OrEQ,
  AST_XorEQ,
  AST__PastLastAddLevelAssignment,

  // NOTE: 8 + 6 is not used since there are no cmp assignment expressions

  AST__FirstLAndLevelAssignment = AST_KIND__BLOCK(8 + 6),
  AST_LAndEQ = AST__FirstLAndLevelAssignment,
  AST__PastLastLAndLevelAssignment,

  AST__FirstLOrLevelAssignment = AST_KIND__BLOCK(8 + 7),
  AST_LOrEQ = AST__FirstLOrLevelAssignment,
  AST__PastLastLOrLevelAssignment,
  AST__PastLastBinaryAssignment = AST__PastLastLOrLevelAssignment,
  AST__PastLastStmnt = AST__PastLastBinaryAssignment,

  AST__FirstSpecial,
  AST_Arg = AST__FirstSpecial,
  AST__PastLastSpecial,
} AST_Kind;

typedef struct AST_Header
{
  AST_Kind kind;
  struct AST_Header* next; // TODO: see if this is actually usefull
} AST_Header;

#define AST_HEADER union { struct AST_Header; AST_Header header; }

typedef AST_Header AST;


// -- Primary expressions

typedef struct AST_Ident_Expr
{
  AST_HEADER;
  Ident ident;
} AST_Ident_Expr;

typedef struct AST_String_Expr
{
  AST_HEADER;
  // TODO:
} AST_String_Expr;

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
  AST* inner;
} AST_Compound_Expr;

typedef struct AST_Proc_Type_Expr
{
  AST_HEADER;
  // TODO:
} AST_Proc_Type_Expr;

typedef struct AST_Proc_Lit_Expr
{
  AST_HEADER;
  // TODO:
} AST_Proc_Lit_Expr;

typedef struct AST_Struct_Type_Expr
{
  AST_HEADER;
  AST* body;
} AST_Struct_Type_Expr;


// -- Postix expressions

typedef struct AST_Deref_Expr
{
  AST_HEADER;
  AST* operand;
} AST_Deref_Expr;

typedef struct AST_Index_Expr
{
  AST_HEADER;
  AST* array;
  AST* idx;
} AST_Index_Expr;

typedef struct AST_Slice_Expr
{
  AST_HEADER;
  AST* array;
  AST* start_idx;
  AST* past_last_idx;
} AST_Slice_Expr;

// NOTE: Call or Cast
typedef struct AST_Call_Expr
{
  AST_HEADER;
  AST* operand;
  AST* args;
} AST_Call_Expr;

typedef struct AST_Member_Expr
{
  AST_HEADER;
  AST* operand;
  Ident name;
} AST_Member_Expr;

typedef struct AST_Struct_Lit_Expr
{
  AST_HEADER;
  AST* operand;
  AST* args;
} AST_Struct_Lit_Expr;


// -- Prefix expressions

typedef struct AST_Prefix_Expr
{
  AST_HEADER;
  AST* operand;
} AST_Prefix_Expr;

typedef struct AST_Array_Of_Expr
{
  AST_HEADER;
  AST* operand;
  AST* size;
} AST_Array_Of_Expr;


// -- Binary expressions

typedef struct AST_Binary_Expr
{
  AST_HEADER;
  AST* left;
  AST* right;
} AST_Binary_Expr;


// -- Ternary expressions

typedef struct AST_Conditional_Expr
{
  AST_HEADER;
  AST* condition;
  AST* true_expr;
  AST* false_expr;
} AST_Conditional_Expr;


// -- Declarations

typedef struct AST_Var_Decl
{
  AST_HEADER;
  AST* names;
  AST* type;
  AST* values;
  bool is_uninitialized;
} AST_Var_Decl;

typedef struct AST_Const_Decl
{
  AST_HEADER;
  AST* names;
  AST* type;
  AST* values;
} AST_Const_Decl;


// -- Statements

typedef struct AST_Block_Stmnt
{
  AST_HEADER;
  AST* label;
  AST* body;
} AST_Block_Stmnt;

typedef struct AST_If_Stmnt
{
  AST_HEADER;
  AST* label;
  AST* condition;
  AST* true_body;
  AST* false_body;
} AST_If_Stmnt;

typedef struct AST_While_Stmnt
{
  AST_HEADER;
  AST* label;
  AST* condition;
  AST* body;
} AST_While_Stmnt;

typedef struct AST_Return_Stmnt
{
  AST_HEADER;
  AST* args;
} AST_Return_Stmnt;

typedef struct AST_Break_Stmnt
{
  AST_HEADER;
  AST* label;
} AST_Break_Stmnt;

typedef struct AST_Continue_Stmnt
{
  AST_HEADER;
  AST* label;
} AST_Continue_Stmnt;

typedef struct AST_Assignment_Stmnt
{
  AST_HEADER;
  AST* lhs;
  AST* rhs;
} AST_Assignment_Stmnt;


// -- Special

typedef struct AST_Argument
{
  AST_HEADER;
  AST* value;
  AST* name;
} AST_Argument;
