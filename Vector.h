#pragma once

#define POOLTAG 'KVEC'

#define STATUS_HEAP_ALLOCATION_FAILED ((NTSTATUS)'VEC1')

template <typename T>
class Vector
{
	T* data{};
	SIZE_T size{};
	SIZE_T capacity{};
	const SIZE_T type_size = sizeof(T);
	constexpr SIZE_T byte_size() { return size * sizeof(T); }
	constexpr SIZE_T byte_capacity() { return capacity * sizeof(T); }


public:
	Vector() = default;

	// Rule of five
	~Vector()
	{
		if (data)
		{
			ExFreePool2(data, POOLTAG, NULL, NULL);
		}
	}

	Vector(const Vector& other) // copy constructor
	{
		if (other.capacity)
		{
			// deep copy
			data = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.capacity * sizeof(T), POOLTAG);
			if (!data)
				ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception
			RtlCopyMemory(data, other.data, other.size); // no need to copy all capacity, size is enough

			size = other.size;
			capacity = other.capacity;
		}
	}

	Vector& operator=(const Vector& other) //copy assignment operator
	{
		if (this == &other)
			return *this;

		if (other.capacity)
		{
			// deep copy
			T* temp = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.capacity * sizeof(T), POOLTAG); // this can throw
			if (!temp)
				ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception
			RtlCopyMemory(data, other.data, other.size * sizeof(T)); // no need to copy all capacity, size is enough


			if (capacity) //discard existing memory
				ExFreePool2(data, POOLTAG, NULL, NULL);

			data = temp;
			size = other.size;
			capacity = other.capacity;
		}
		else
		{
			//shallow copy
			data = other.data;
			size = other.size;
			capacity = other.capacity;
		}
		
		return *this;
	}

	Vector(Vector&& other) noexcept //move constructor
	{
		//no std::swap available so trivial method used
		
		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}

	Vector& operator=(Vector&& other) noexcept //move assignment operator
	{ 
		if (this == &other)
			return *this;

		if (capacity) //discard existing memory
			ExFreePool2(data, POOLTAG, NULL, NULL);

		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;

		return *this;
	}

	T& at(ULONG idx) const
	{
		if (idx > (size - 1))
		{
			ExRaiseStatus(STATUS_INDEX_OUT_OF_BOUNDS);// SEH exception
		}

		return data[idx];
	}

	VOID push_back(const T& object)
	{
		++size;
		
		if (size > capacity)
			increase_capacity(size);

		data[size - 1] = object;
	}

	VOID push_front(const T& object)
	{
		++size;

		if (size > capacity)
			increase_capacity(size);

		RtlMoveMemory(&data[1], data, (size - 1) * sizeof(T));

		data[0] = object;
	}

private:

	VOID increase_capacity(SIZE_T required_size)
	{

		if (required_size <= capacity) // sanity check
			return;
		else if (capacity == 0)
			capacity = 1;

		while (capacity < required_size)
			capacity *= 2;

		T* new_buffer = static_cast<T*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, byte_capacity(), POOLTAG));
		if (!new_buffer)
			ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception

		if (data)
		{
			RtlCopyMemory(new_buffer, data, byte_size());
			ExFreePool2(data, POOLTAG, NULL, NULL);
		}

		data = new_buffer;
	}
};

