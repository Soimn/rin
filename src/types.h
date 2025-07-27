typedef enum Type_Info_Kind
{
	TypeInfo_SignedInt,
	TypeInfo_UnsignedInt,
	TypeInfo_Float,
	TypeInfo_Bool,
	TypeInfo_String,

	TypeInfo_Proc,
	TypeInfo_Struct,
	TypeInfo_Enum,

	TypeInfo_Distinct,
	TypeInfo_Pointer,
	TypeInfo_Slice,
	TypeInfo_Array,

	TypeInfoKind__Count,
} Type_Info_Kind;

// NOTE: There are ~65 million identifiers in the linux source code ~= 2^26.
//       Assuming that these are all unique types and have <= 4 variants
//       (i.e. int, int*, int**, int[3]) gives an upper bound of 2^28 types.
//       Using a 28 bit index and packing it with a 4 bit kind gives a typeid
//       that should be able to refer to all types using a flat index and also
//       tell the compiler and user what kind of type it is and how to find the
//       element type.
#define TYPEID_KIND_BITS 4
#define TYPEID_ID_BITS (32 - TYPEID_KIND_BITS)
#define TYPEID_KIND(TID) ((TID) >> TYPEID_ID_BITS)
#define TYPEID_ID(TID) ((TID) & (~(u32)0 >> TYPEID_KIND_BITS))

static_assert(TypeInfoKind__Count < (1 << TYPEID_KIND_BITS), "Too many type kinds");

typedef struct Typeid
{
	u32 value;
} Typeid;

typedef struct Type_Info
{
	Typeid type_id;

	union
	{
		Typeid elem_type;

		struct
		{
			Typeid elem_type;
			u32 len;
		} array;
	};

	Typeid back_edges[TypeInfoKind__Count];
} Type_Info;

static Typeid
Typeid_Distinct(Typeid type)
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_PointerTo(Typeid elem_type)
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_SliceOf(Typeid elem_type)
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_ArrayOf(Typeid elem_type, umm len)
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_Proc()
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_Struct()
{
	NOT_IMPLEMENTED;
}

static Typeid
Typeid_Enum()
{
	NOT_IMPLEMENTED;
}

static Type_Info*
TypeInfo_FromTypeid(Typeid type)
{
	NOT_IMPLEMENTED;
}
