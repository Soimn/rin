#define AST__EXPR_BLOCK(N) ((N) << 4)
#define AST__BLOCK_IDX(K) ((K) >> 4)

#define AST_IS_BINARY_EXPR(K) (uint)((K)-AST__FirstBinary) < (uint)(AST__PastLastBinary-AST__FirstBinary)

typedef enum AST_Kind
{
  AST_Invalid = 0,

  AST__FirstExpr = AST__EXPR_BLOCK(1),
  AST__FirstPrimary = AST__FirstExpr,
  AST_Ident = AST__FirstPrimary,
  AST_String,
  AST_Int,
  AST_Float,
  AST_Bool,
  AST_ProcType,
  AST_ProcLit,
  AST_StructType,
  AST_Compound,
  AST__PastLastPrimary,

  AST__FirstTypePrefix = AST__EXPR_BLOCK(2),
  AST_PointerType = AST__FirstTypePrefix, // NOTE: This is a Unary_Expr
  AST_ArrayType,
  AST_SliceType,                          // NOTE: This is a Unary_Expr
  AST__PastLastTypePrefix,

  AST__FirstPostfix = AST__EXPR_BLOCK(3),
  AST_Subscript = AST__FirstPostfix,
  AST_Slice,
  AST_Call,
  AST_Member,
  AST_Deref,                              // NOTE: This is a Unary_Expr
  AST_StructLit,
  AST_ArrayLit,
  AST__PastLastPostfix,

  AST__FirstPrefix = AST__EXPR_BLOCK(4),
  AST_Pos = AST__FirstPrefix,
  AST_Neg,
  AST_BitNot,
  AST_Not,
  AST_Ref,
  AST__PastLastPrefix,

  AST__FirstBinary = AST__EXPR_BLOCK(9),
  AST__FirstMulLevel = AST__FirstBinary,
  AST_Mul = AST__FirstMulLevel,
  AST_Div,
  AST_Mod,
  AST_BitAnd,
  AST_BitShl,
  AST_BitShr,
  AST_BitSar,
  AST__PastLastMulLevel,

  AST__FirstAddLevel = AST__EXPR_BLOCK(10),
  AST_Add = AST__FirstAddLevel,
  AST_Sub,
  AST_BitOr,
  AST_BitXor,
  AST__PastLastAddLevel,

  AST__FirstCmpLevel = AST__EXPR_BLOCK(11),
  AST_CmpEQ = AST__FirstCmpLevel,
  AST_CmpNotEQ,
  AST_CmpLe,
  AST_CmpLeEQ,
  AST_CmpGe,
  AST_CmpGeEQ,
  AST__PastLastCmpLevel,

  AST__FirstAndLevel = AST__EXPR_BLOCK(12),
  AST_And = AST__FirstAndLevel,
  AST__PastLastAndLevel,

  AST__FirstOrLevel = AST__EXPR_BLOCK(13),
  AST_Or = AST__FirstOrLevel,
  AST__PastLastOrLevel,
  AST__PastLastBinary = AST__PastLastOrLevel,

  AST_Conditional = AST__EXPR_BLOCK(14),

  AST__PastLastExpr,

  AST__FirstStmnt,
  AST_Block = AST__FirstStmnt,
  AST_If,
  AST_While,
  AST_Assignment,
  AST__PastLastStmnt,
} AST_Kind;

STATIC_ASSERT(AST__PastLastPrimary    <= AST__FirstTypePrefix);
STATIC_ASSERT(AST__PastLastTypePrefix <= AST__FirstPostfix);
STATIC_ASSERT(AST__PastLastPostfix    <= AST__FirstPrefix);
STATIC_ASSERT(AST__PastLastPrefix     <= AST__FirstBinary);
STATIC_ASSERT(AST__PastLastMulLevel   <= AST__FirstAddLevel);
STATIC_ASSERT(AST__PastLastAddLevel   <= AST__FirstCmpLevel);
STATIC_ASSERT(AST__PastLastCmpLevel   <= AST__FirstAndLevel);
STATIC_ASSERT(AST__PastLastAndLevel   <= AST__FirstOrLevel);

typedef struct AST
{
  AST_Kind kind;
  Text_Pos text_pos;
} AST;

#define AST_HEADER union { AST header; struct AST; }

typedef struct Ident_Expr
{
  AST_HEADER;
  String value;
} Ident_Expr;

typedef struct String_Expr
{
  AST_HEADER;
  String value;
} String_Expr;

typedef struct Int_Expr
{
  AST_HEADER;
  s128 value;
} Int_Expr;

typedef struct Float_Expr
{
  AST_HEADER;
  f64 value;
} Float_Expr;

typedef struct Bool_Expr
{
  AST_HEADER;
  bool value;
} Bool_Expr;

typedef struct Proc_Type_Expr
{
  AST_HEADER;
  // TODO:
} Proc_Type_Expr;

typedef struct Proc_Lit_Expr
{
  AST_HEADER;
  // TODO:
} Proc_Lit_Expr;

typedef struct Compound_Expr
{
  AST_HEADER;
  AST* inner;
} Compound_Expr;

typedef struct Array_Type_Expr
{
  AST_HEADER;
  AST* size;
  AST* type;
} Array_Type_Expr;

typedef struct Subscript_Expr
{
  AST_HEADER;
  AST* array;
  AST* idx;
} Subscript_Expr;

typedef struct Slice_Expr
{
  AST_HEADER;
  AST* array;
  AST* first_idx;
  AST* past_idx;
} Slice_Expr;

typedef struct Member_Expr
{
  AST_HEADER;
  AST* host;
  AST* name;
} Member_Expr;

typedef struct Call_Expr
{
  AST_HEADER;
  // TODO:
} Call_Expr;

typedef struct Struct_Type_Expr
{
  AST_HEADER;
  // TODO:
} Struct_Type_Expr;

typedef struct Array_Lit_Expr
{
  AST_HEADER;
  // TODO:
} Array_Lit_Expr;

typedef struct Unary_Expr
{
  AST_HEADER;
  AST* expr;
} Unary_Expr;

typedef struct Binary_Expr
{
  AST_HEADER;
  AST* left;
  AST* right;
} Binary_Expr;

typedef struct Conditional_Expr
{
  AST_HEADER;
  AST* condition;
  AST* true_expr;
  AST* false_expr;
} Conditional_Expr;

typedef struct Assignment_Stmnt
{
  AST_HEADER;
  AST* lhs;
  AST* rhs;
  AST_Kind op;
} Assignment_Stmnt;
