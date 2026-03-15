#pragma once

#define POOLTAG 'KVEC'

class Vector
{
	PVOID data{};
	SIZE_T size{};
	SIZE_T capacity{};


public:
	Vector() = default;

	// Rule of five
	~Vector()
	{
		if (data)
		{
			ExFreePool(data);
		}
	}

	Vector(const Vector& other) // copy constructor
	{
		if (other.capacity)
		{
			// deep copy
			data = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.capacity, POOLTAG);
			memcpy(data, other.data, other.size); // no need to copy all capacity, size is enough

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
			if (capacity) //discard existing memory
				ExFreePool2(data, POOLTAG, NULL, NULL);
			
			// deep copy
			data = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.capacity, POOLTAG);
			memcpy(data, other.data, other.size); // no need to copy all capacity, size is enough

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

	PVOID at(ULONG idx, ULONG type_size);

};

