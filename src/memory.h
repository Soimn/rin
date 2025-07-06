static bool
IsZero(void* ptr, umm size)
{
	bool result = true;

	for (umm i = 0; i < size && result; ++i)
	{
		result = (((u8*)ptr)[i] == 0);
	}

	return result;
}
