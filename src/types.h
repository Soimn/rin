typedef enum Type_Info_Kind
{
	TypeInfoKind_Basic = 0,
	TypeInfoKind_Proc,
	TypeInfoKind_Struct,
	TypeInfoKind_Enum,

	TypeInfoKind_Pointer,
	TypeInfoKind_Slice,
	TypeInfoKind_Array,

	TypeInfoKind__Count,
} Type_Info_Kind;

typedef struct { u32 val; } Typeid;

// NOTE: There are ~65 million identifiers in the linux source code ~= 2^26.
//       Assuming that these are all unique types and have <= 4 variants
//       (i.e. int, int*, int**, int[3]) gives an upper bound of 2^28 types.
//       Using a 28 bit index and packing it with a 4 bit kind gives a typeid
//       that should be able to refer to all types using a flat index and also
//       tell the compiler and user what kind of type it is and how to find the
//       element type.
#define TYPEID_KIND_BITS 4
#define TYPEID_ID_BITS (32 - TYPEID_KIND_BITS)
#define TYPEID_KIND(TID) (Type_Info_Kind)((TID).val >> TYPEID_ID_BITS)
#define TYPEID_ID(TID) ((TID).val & (~(u32)0 >> TYPEID_KIND_BITS))

static_assert(TypeInfoKind__Count < (1 << TYPEID_KIND_BITS), "Too many type kinds");

typedef enum Type_Info_Basic_Kind
{
	TypeInfoBasicKind__FirstSoft,
	TypeInfoBasicKind_SoftInt = TypeInfoBasicKind__FirstSoft,
	TypeInfoBasicKind_SoftFloat,
	TypeInfoBasicKind_SoftBool,
	TypeInfoBasicKind_SoftString,
	TypeInfoBasicKind__PastLastSoft,

	TypeInfoBasicKind_Int,
	TypeInfoBasicKind_Float,
	TypeInfoBasicKind_Bool,
	TypeInfoBasicKind_String,
	TypeInfoBasicKind_Typeid,
} Type_Info_Basic_Kind;

typedef struct TypeInfo_Basic
{
	u8 kind;
	u8 size;
	u16 bit_size;
	Typeid aliased_type;
} TypeInfo_Basic;

// TODO
Typeid Typeid_Typeid     = {0};
Typeid Typeid_SoftInt    = {0};
Typeid Typeid_SoftFloat  = {0};
Typeid Typeid_SoftBool   = {0};
Typeid Typeid_SoftString = {0};

static bool
Typeid_Equal(Typeid a, Typeid b)
{
	return (a.val == b.val);
}

static bool
Typeid_IsInteger(Typeid tid)
{
	NOT_IMPLEMENTED;
	(void)tid;
	return false;
}

static bool
Typeid_IsPointer(Typeid tid)
{
	NOT_IMPLEMENTED;
	(void)tid;
	return false;
}

static Typeid
Typeid_ElementType(Typeid tid)
{
	NOT_IMPLEMENTED;
	(void)tid;
	return (Typeid){0};
}

static bool
Typeid_IsImplicitlyConvertibleToBool(Typeid tid)
{
	NOT_IMPLEMENTED;
	(void)tid;
	return false;
}

static bool
Typeid_HasCommonType(Typeid a, Typeid b, Typeid* common_type)
{
	NOT_IMPLEMENTED;
	(void)a, b, common_type;
	return false;
}

static Typeid
Typeid_PointerTo(Typeid elem_type)
{
	NOT_IMPLEMENTED;
	(void)elem_type;
	return (Typeid){0};
}

static Typeid
Typeid_SliceOf(Typeid elem_type)
{
	NOT_IMPLEMENTED;
	(void)elem_type;
	return (Typeid){0};
}

static Typeid
Typeid_ArrayOf(Typeid elem_type, u32 len)
{
	NOT_IMPLEMENTED;
	(void)elem_type, len;
	return (Typeid){0};
}
