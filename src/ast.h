#define AST_KIND__BLOCK_SIZE_LG2 6
#define AST_KIND__BLOCK(N) ((N) << AST_KIND__BLOCK_SIZE_LG2)
#define AST_KIND__BLOCK_IDX(N) ((N) >> AST_KIND__BLOCK_SIZE_LG2)
#define AST_KIND__BLOCK_OFFSET(N) ((N) & ~(AST_KIND__BLOCK_SIZE_LG2-1))

typedef enum AST_Kind
{
  AST__FirstExpr = AST_KIND__BLOCK(0),
  AST__FirstPrimitiveLevel = AST__FirstExpr,
  AST_Ident,
  AST_String,
  AST_Int,
  AST_Float,
  AST_Bool,
  AST_Compound,
  AST_StructType,
  AST_ProcType,
  AST_ProcLit,
  AST__PastLastPrimitiveLevel,

  AST__FirstTypePrefixLevel = AST_KIND__BLOCK(1),
  AST_PointerTo = AST__FirstTypePrefixLevel,
  AST_SliceOf,
  AST_ArrayOf,
  AST__PastLastTypePrefixLevel,

  AST__FirstPostfixLevel = AST_KIND__BLOCK(2),
  AST_Call = AST__FirstPostfixLevel,
  AST_Member,
  AST_Slice,
  AST_Subscript,
  AST_StructLit,
  AST_ArrayLit,
  AST_Cast,
  AST__PastLastPostfixLevel,

  AST__FirstPrefixLevel = AST_KIND__BLOCK(3),
  AST_Pos = AST__FirstPrefixLevel,
  AST_Neg,
  AST_LNot,
  AST_Not,
  AST_Ref,
  AST__PastLastPrefixLevel,

  AST__FirstBinaryExpr = AST_KIND__BLOCK(4),
  AST__FirstMulLevel = AST__FirstBinaryExpr,
  AST_Mul = AST__FirstMulLevel,
  AST_Div,
  AST_Rem,
  AST_And,
  AST_Shl,
  AST_Shr,
  AST_Sar,
  AST__PastLastMulLevel,

  AST__FirstAddLevel = AST_KIND__BLOCK(5),
  AST_Add = AST__FirstAddLevel,
  AST_Sub,
  AST_Or,
  AST_Xor,
  AST__PastLastAddLevel,

  AST__FirstCmpLevel = AST_KIND__BLOCK(6),
  AST_CmpEQ = AST__FirstCmpLevel,
  AST_CmpNeq,
  AST_CmpLe,
  AST_CmpLeEQ,
  AST_CmpGe,
  AST_CmpGeEQ,
  AST__PastLastCmpLevel,

  AST__FirstLAndLevel = AST_KIND__BLOCK(7),
  AST_LAnd = AST__FirstLAndLevel,
  AST__PastLastLAndLevel,

  AST__FirstLOrLevel = AST_KIND__BLOCK(8),
  AST_LOr = AST__FirstLOrLevel,
  AST__PastLastLOrLevel,

  AST__PastLastBinaryExpr = AST_KIND__BLOCK(9),

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
  AST_Assignment,
  AST__PastLastStmnt,
} AST_Kind;

typedef u32 ASTRP;
#define ASTRP_DEREF(BASE, RELPTR) (AST*)((u32*)(BASE) + (RELPTR))

typedef struct AST_Header
{
  AST_Kind kind;
  ASTRP next;
  u32 file_offset;
  u16 file_idx;
  // u16 padding
} AST_Header;

#define AST_HEADER union { AST_Header header; struct AST_Header; }

typedef struct AST
{
  AST_HEADER;
} AST;

typedef struct AST_Binary_Expr
{
  AST_HEADER;
  ASTRP left;
  ASTRP right;
} AST_Binary_Expr;

/*
 
^ []
^ [] . .{} .[] .()
+ - ! ~ &
* / % & >> << >>>
+ - | ~
== != < <= > >=
&&
||

*/
