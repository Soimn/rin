typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;
typedef s64 smm;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
typedef u64 umm;

typedef u8 bool;
#define true 1
#define false 0

typedef float f32;
typedef union F32_Bits { f32 f; u32 bits; } F32_Bits;

typedef double f64;
typedef union F64_Bits { f64 f; u64 bits; } F64_Bits;

typedef struct String
{
	u8* data;
	u32 len;
} String;

#define STRING(S) ((String){ .data = (u8*)(S), .len = sizeof(S)-1 })
#define MS_STRING(S) { .data = (u8*)(S), .len = sizeof(S)-1 }

__declspec(dllimport) int __stdcall IsDebuggerPresent();
static void AssertionFailed(const char* file, int line, const char* expr);
#define ASSERT(EX) ((EX) ? 1 : (IsDebuggerPresent() ? (*(volatile int*)0 = 0) : (AssertionFailed(__FILE__, __LINE__, #EX), 0)))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

static void* ReserveMemory(umm bytes_to_reserve);
static void CommitMemory(void* commit_base, umm bytes_to_commit);
static void FreeMemory(void* reserve_base);

#define ALIGN(N, A) (((umm)(N) + (umm)(A)-1) & (umm)-(smm)(A))
#define PAGE_SIZE 4096

#define ARRAY_LEN(A) (sizeof(A) / sizeof(0[A]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define FORCE_INLINE __forceinline
