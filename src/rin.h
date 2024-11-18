#include <intrin.h>

typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

#define S8_MIN  0x80
#define S16_MIN 0x8000
#define S32_MIN 0x80000000
#define S64_MIN 0x8000000000000000LL
#define S8_MAX  0x7F
#define S16_MAX 0x7FFF
#define S32_MAX 0x7FFFFFFF
#define S64_MAX 0x7FFFFFFFFFFFFFFFLL

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFFULL

typedef s64 smm;
typedef u64 umm;

#define SMM_MIN S64_MIN
#define SMM_MAX S64_MAX
#define UMM_MAX U64_MAX

typedef u8 bool;
#define true 1
#define false 0

typedef float f32;
typedef double f64;

#ifdef _WIN32
#define IS_DEBUGGER_PRESENT() IsDebuggerPresent()
#else
#define IS_DEBUGGER_PRESENT() false
#endif

void AssertHandler(char* file, int line, char* expr);
#define ASSERT(EX) ((EX) ? 1 : (IS_DEBUGGER_PRESENT() ? *(volatile int*)0 = 0 : AssertHandler(__FILE__, __LINE__, #EX)))
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(0[A]))

#define ALIGNOF(T) _alignof(T)

typedef union F32_Bits
{
  u32 bits;
  f32 f;
} F32_Bits;

typedef union F64_Bits
{
  u64 bits;
  f64 f;
} F64_Bits;

typedef u32 Ident;

static void* ReserveMemory(umm size, bool do_commit);
static void CommitMemory(void* base, umm size);
static void ReleaseMemory(void* reserve_base);

typedef struct Entity
{
	// file
	// dependencies?
	// symbol table?
	// constant stack?
	struct AST* ast;
} Entity;

#include "memory.h"
#include "string.h"
#include "tokens.h"
#include "ast.h"
#include "string_interning.h"
#include "lexer.h"
#include "parser.h"
