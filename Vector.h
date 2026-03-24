#ifndef VECTOR_H
#define VECTOR_H

#pragma warning(disable: 4996) // Disable the "ExAllocatePoolWithTag is deprecated" warning

#define POOLTAG 'KVEC'

#define STATUS_HEAP_ALLOCATION_FAILED ((NTSTATUS)'VEC1')
#define STATUS_EMPTY ((NTSTATUS)'VEC2')

template <typename T>
class Vector
{
	T* m_data{};
	SIZE_T m_size{};
	SIZE_T m_capacity{};

	VOID init();

	SIZE_T type_size{ sizeof(T) };
	SIZE_T byte_size() { return m_size * sizeof(T); }
	SIZE_T byte_capacity() { return m_capacity * sizeof(T); }
public:
	Vector() : Vector(0) {}
	Vector(SIZE_T initial_size, const T& val = T{})
	{
		init();
		increase_capacity(initial_size);
		resize(initial_size, val);
	}

	// Rule of five
	~Vector()
	{
		if (m_data)
		{
			Free(m_data);
		}
	}

	Vector(const Vector& other) // copy constructor
	{
		if (other.m_capacity)
		{
			// deep copy
			m_data = Allocate(other.m_capacity * sizeof(T));
			if (!m_data)
				ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception
			RtlCopyMemory(m_data, other.m_data, other.m_size); // no need to copy all capacity, size is enough

			m_size = other.m_size;
			m_capacity = other.m_capacity;
		}
	}

	Vector& operator=(const Vector& other) //copy assignment operator
	{
		if (this == &other)
			return *this;

		if (!other.empty())
		{
			// deep copy
			T* temp = Allocate(other.m_capacity * sizeof(T)); // this can throw
			if (!temp)
				ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception
			RtlCopyMemory(m_data, other.m_data, other.m_size * sizeof(T)); // no need to copy all capacity, size is enough


			if (m_capacity) //discard existing memory
				Free(m_data);

			m_data = temp;
			m_size = other.m_size;
			m_capacity = other.m_capacity;
		}
		else
		{
			//shallow copy
			m_data = other.m_data;
			m_size = other.m_size;
			m_capacity = other.m_capacity;
		}

		return *this;
	}

	Vector(Vector&& other) noexcept : m_data{ other.m_data }, m_size{ other.m_size }, m_capacity{ other.m_capacity } //move constructor
	{
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	Vector& operator=(Vector&& other) noexcept //move assignment operator
	{ 
		if (this == &other)
			return *this;

		if (m_capacity) //discard existing memory
		{
			__try
			{
				Free(m_data);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				//ExFreePool2 raised a SEH exception
				//this is likely a fatal error but we don't want to crash so for now ignore the exception and move on(accept the memory leak)
				//TODO: handle this
			}
		}

		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;

		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;

		return *this;
	}

	SIZE_T size() const noexcept;

	SIZE_T capacity() const noexcept;

	T& at(ULONG idx) const;

	VOID push_back(const T& object);

	VOID push_front(const T& object);

	T& front();
	//const Treference front() is called for const vectors for read-only access 
	const T& front() const;

	T& back();
	//const Treference back() is called for const vectors for read-only access 
	const T& back() const;

	T& operator[](SIZE_T idx);
	//const Treference operator[] is called for const vectors for read-only access
	const T& operator[](SIZE_T idx) const;

	const T* data() const noexcept;
	T* data() noexcept;

	VOID swap(Vector& other) noexcept;

	BOOLEAN empty() noexcept;

	VOID clear();

	VOID resize(SIZE_T new_size, const T& val = T{});

	T* begin();
	const T* begin() const;

	T* end();
	const T* end() const;


private:
	using ExAllocatePool2_t = PVOID(*)(POOL_FLAGS Flags, SIZE_T NumberOfBytes, ULONG Tag);
	inline static ExAllocatePool2_t pExAllocatePool2{ nullptr };
	using ExFreePool2_t = VOID(*)(PVOID P, ULONG Tag, PCPOOL_EXTENDED_PARAMETER ExtendedParameters, ULONG ExtendedParametersCount);
	inline static ExFreePool2_t pExFreePool2{ nullptr };
	inline static BOOLEAN vector_initialized{ FALSE };

	VOID increase_capacity(SIZE_T required_size);

	PVOID Allocate(SIZE_T size);
	VOID Free(PVOID P);
};

template<typename T>
inline VOID Vector<T>::init()
{
	/*
	This init guarantees that every version of Vector class created at runtime will be initialized:

	We need to check at runtime if the kernel version has ExAllocatePool2
	If ExAllocatePool2 isn't found use the old API(ExAllocatePoolWithTag)
	*/
	
	
	if (vector_initialized)
		return;
	
	// Try to get ExAllocatePool2
	{
		UNICODE_STRING routineName;
		RtlInitUnicodeString(&routineName, L"ExAllocatePool2");
		pExAllocatePool2 = (ExAllocatePool2_t)MmGetSystemRoutineAddress(&routineName);
	}

	// Try to get ExFreePool2 if ExAllocatePool2 is present
	if (pExAllocatePool2)
	{
		UNICODE_STRING routineName;
		RtlInitUnicodeString(&routineName, L"ExFreePool2");
		pExFreePool2 = (ExFreePool2_t)MmGetSystemRoutineAddress(&routineName);
	}

	vector_initialized = TRUE;
}

template<typename T>
inline SIZE_T Vector<T>::size() const noexcept
{
	return m_size;
}

template<typename T>
inline SIZE_T Vector<T>::capacity() const noexcept
{
	return m_capacity;
}

template<typename T>
inline T& Vector<T>::at(ULONG idx) const
{
	if (idx > (m_size - 1))
	{
		ExRaiseStatus(STATUS_INDEX_OUT_OF_BOUNDS);// SEH exception
	}

	return m_data[idx];
}

template<typename T>
inline VOID Vector<T>::push_back(const T& object)
{
	++m_size;

	if (m_size > m_capacity)
		increase_capacity(m_size);

	m_data[m_size - 1] = object;
}

template<typename T>
inline VOID Vector<T>::push_front(const T& object)
{
	++m_size;

	if (m_size > m_capacity)
		increase_capacity(m_size);

	RtlMoveMemory(&m_data[1], m_data, (m_size - 1) * sizeof(T));

	m_data[0] = object;
}

template<typename T>
inline T& Vector<T>::front()
{
	if (!empty())
		return m_data[0];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline const T& Vector<T>::front() const
{
	if (!empty())
		return m_data[0];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline T& Vector<T>::back()
{
	if (!empty())
		return m_data[m_size - 1];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline const T& Vector<T>::back() const
{
	if (!empty())
		return m_data[m_size - 1];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline const T& Vector<T>::operator[](SIZE_T idx) const
{
	return m_data[idx];
}

template<typename T>
inline T& Vector<T>::operator[](SIZE_T idx)
{
	return m_data[idx];
}


template<typename T>
inline const T* Vector<T>::data() const noexcept
{
	return m_data;
}

template<typename T>
inline T* Vector<T>::data() noexcept
{
	return m_data;
}

template<typename T>
inline VOID Vector<T>::swap(Vector& other) noexcept
{
	auto swap = []<typename T>(T & first, T & second) -> void
	{
		if (&first == &second)
			return;
		first ^= second;
		second ^= first;
		first ^= second;
	};

	swap(m_data, other.m_data);
	swap(m_capacity, other.m_capacity);
	swap(m_size, other.m_size);
}

template<typename T>
inline BOOLEAN Vector<T>::empty() noexcept
{
	return !m_size;
}

template<typename T>
inline VOID Vector<T>::clear()
{
	if (m_data)
	{
		Free(m_data);
	}
	m_size = 0;
	m_capacity = 0;
	m_data = 0;
}

template<typename T>
inline VOID Vector<T>::resize(SIZE_T new_size, const T& val)
{
	if (new_size < m_size) // Requested size is smaller than the current size
	{
		for (SIZE_T i = new_size; i < m_size; ++i)
		{
			m_data[i].~T();
		}
		m_size = new_size;
	}
	else if (new_size > m_size) // Requested size is larger than the current size
	{
		if (new_size > m_capacity)
			increase_capacity(new_size);

		for (SIZE_T i = m_size; i < new_size; ++i)
		{
			// there is no placement new in kernel?
			//new (&m_data[i]) T(val);
			m_data[i] = T(val);
		}
		m_size = new_size;
	}
}

template<typename T>
inline T* Vector<T>::begin()
{
	if (!empty())
		return m_data;
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline const T* Vector<T>::begin() const
{
	if (!empty())
		return m_data;
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline T* Vector<T>::end()
{
	if (!empty())
		return &m_data[m_size];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline const T* Vector<T>::end() const
{
	if (!empty())
		return &m_data[m_size];
	else
		ExRaiseStatus(STATUS_EMPTY);
}

template<typename T>
inline VOID Vector<T>::increase_capacity(SIZE_T required_size)
{

	if (required_size <= m_capacity) // sanity check
		return;
	else if (m_capacity == 0)
		m_capacity = 1;

	while (m_capacity < required_size)
		m_capacity *= 2;

	T* new_buffer = static_cast<T*>(Allocate(byte_capacity()));
	if (!new_buffer)
		ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception

	if (m_data)
	{
		RtlCopyMemory(new_buffer, m_data, byte_size());
		Free(m_data);
	}

	m_data = new_buffer;
}

template<typename T>
inline PVOID Vector<T>::Allocate(SIZE_T size)
{
	if (pExAllocatePool2)
	{
		return pExAllocatePool2(POOL_FLAG_NON_PAGED, size, POOLTAG);
	}
	else
	{
		return ExAllocatePoolWithTag(NonPagedPool, size, POOLTAG);
	}
}

template<typename T>
inline VOID Vector<T>::Free(PVOID P)
{
	if (pExFreePool2)
	{
		pExFreePool2(P, POOLTAG, NULL, NULL);
	}
	else
	{
		ExFreePoolWithTag(P, POOLTAG);
	}
}

#endif