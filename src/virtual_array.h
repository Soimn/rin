typedef struct Virtual_Array
{
	u8* data;
	u32 offset;
	u32 committed;
	u16 elem_size;
	u16 commit_pace;
	u32 reserved;
} Virtual_Array;

#define VA_PAGE_SIZE_LG2 12
#define VA_PAGE_SIZE (1 << VA_PAGE_SIZE_LG2)

static Virtual_Array
VA_Create(u16 elem_size, u32 to_reserve, u16 commit_pace)
{
	ASSERT(to_reserve % VA_PAGE_SIZE == 0);
	ASSERT((to_reserve/VA_PAGE_SIZE) % commit_pace == 0);

	u8* data = ReserveMemory(to_reserve);
	CommitMemory(data, commit_pace << VA_PAGE_SIZE_LG2);

	return (Virtual_Array){
		.data           = data,
		.offset         = 0,
		.committed      = commit_pace,
		.elem_size      = elem_size,
		.commit_pace    = commit_pace,
		.reserved       = to_reserve,
	};
}

static void
VA_Destroy(Virtual_Array* array)
{
	FreeMemory(array->data);
	*array = (Virtual_Array){0};
}

static void
VA_EnsureCommitted(Virtual_Array* array, u32 n)
{
	u64 must_be_committed = (u64)array->offset + n*array->elem_size;

	if (must_be_committed > array->committed)
	{
		u64 to_commit = ALIGN(must_be_committed - array->committed, (array->commit_pace << VA_PAGE_SIZE_LG2));

		u8* commit_base = &array->data[array->committed];
		
		if ((u64)array->committed + to_commit > array->reserved)
		{
			//// ERROR: Out of memory
			NOT_IMPLEMENTED;
		}

		CommitMemory(commit_base, to_commit);
		array->committed += (u32)to_commit;
	}
}

static void*
VA_Push(Virtual_Array* array)
{
	void* result = &array->data[array->offset];

	array->offset += array->elem_size;

	if (array->offset > array->committed)
	{
		VA_EnsureCommitted(array, 1);
	}

	return result;
}

static void*
VA_PushN(Virtual_Array* array, u32 n)
{
	void* result = &array->data[array->offset];

	array->offset += n*array->elem_size;

	if (array->offset > array->committed)
	{
		VA_EnsureCommitted(array, n);
	}

	return result;
}

static void*
VA_EndPointer(Virtual_Array* array)
{
	return &array->data[array->offset];
}
