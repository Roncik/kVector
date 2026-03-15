#include "pch.h"
#include "Vector.h"

PVOID Vector::at(ULONG idx, ULONG type_size)
{
	if (type_size * (idx + 1) > size)
	{
		ExRaiseStatus(STATUS_INDEX_OUT_OF_BOUNDS);// SEH exception
	}

	return reinterpret_cast<PVOID>((reinterpret_cast<ULONGLONG>(data) + type_size * idx)); // return the address of the desired object
}
