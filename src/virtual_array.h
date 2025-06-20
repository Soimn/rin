typedef struct Virtual_Array
{
	u8* data;
	u32 elem_size;
	u32 offset;
	u32 committed;
	u32 reserved;
} Virtual_Array;

static Virtual_Array
VA_Init(u32 elem_size, u32 bytes_to_reserve)
{
	bytes_to_reserve = ALIGN(bytes_to_reserve, PAGE_SIZE);

	u8* data = ReserveMemory(bytes_to_reserve);

	return (Virtual_Array){
		.data      = data,
		.elem_size = elem_size,
		.offset    = 0,
		.committed = 0,
		.reserved  = bytes_to_reserve,
	};
}

static void
VA__CommitOvershoot(Virtual_Array* array)
{
	u32 overshoot = (u32)(array->offset - array->committed) + PAGE_SIZE;

	void* commit_base = &array->data[array->committed];
	u32 to_commit     = ALIGN(overshoot, PAGE_SIZE);
	array->committed += to_commit;
	ASSERT(array->committed < array->reserved);

	CommitMemory(commit_base, to_commit);
}

static void*
VA_Push(Virtual_Array* array)
{
	void* result = &array->data[array->offset];

	array->offset += array->elem_size;

	if (array->offset > array->committed) VA__CommitOvershoot(array);

	return result;
}

static void*
VA_PushN(Virtual_Array* array, u32 num)
{
	void* result = &array->data[array->offset];

	array->offset += array->elem_size*num;

	if (array->offset > array->committed) VA__CommitOvershoot(array);

	return result;
}

static void*
VA_TopPointer(Virtual_Array* array)
{
	return &array->data[array->offset];
}
