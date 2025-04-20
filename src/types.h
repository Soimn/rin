#if 0
#define TYPEID_NIL 0
typedef u32 Type_ID;

#define TYPEID__INDEX_BITS 27
#define TYPEID_CATEGORY(T) (Type_Category)((T) >> TYPEID__INDEX_BITS)
#define TYPEID_INDEX(T) (umm)((T) & -(1ULL << TYPEID__INDEX_BITS))
#define TYPEID(CATEGORY, INDEX) (((Type_ID)(CATEGORY) << TYPEID__INDEX_BITS) | (Type_ID)(INDEX))

typedef enum Type_Category
{
	TypeCategory_Primitive,
	TypeCategory_Distinct,
	TypeCategory_Pointer,
	TypeCategory_Array,
	TypeCategory_Slice,
	TypeCategory_Struct,
	TypeCategory_Proc,
} Type_Category;

typedef struct Pointer_Type_Info
{
	Type_ID operand_type;
} Pointer_Type_Info;

Type_ID
TypeID_PointerTo(Type_ID type)
{
	return TYPEID(TypeCategory_Pointer, TypeTable_Put(pointer_table, type));
}

bool
TypeID_IsPointer(Type_ID type)
{
	return (TYPEID_CATEGORY(type) == TypeCategory_Pointer);
}
#endif
