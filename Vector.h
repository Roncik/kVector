#pragma once

#define POOLTAG 'KVEC'

#define STATUS_HEAP_ALLOCATION_FAILED ((NTSTATUS)'VEC1')
#define STATUS_EMPTY ((NTSTATUS)'VEC2')

template <typename T>
class Vector
{
	T* m_data{};
	SIZE_T m_size{};
	SIZE_T m_capacity{};


	const SIZE_T type_size{ sizeof(T) };
	constexpr SIZE_T byte_size() { return m_size * sizeof(T); }
	constexpr SIZE_T byte_capacity() { return m_capacity * sizeof(T); }
public:
	Vector() = default;

	// Rule of five
	~Vector()
	{
		if (m_data)
		{
			ExFreePool2(m_data, POOLTAG, NULL, NULL);
		}
	}

	Vector(const Vector& other) // copy constructor
	{
		if (other.m_capacity)
		{
			// deep copy
			m_data = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.m_capacity * sizeof(T), POOLTAG);
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

		if (other.m_capacity)
		{
			// deep copy
			T* temp = ExAllocatePool2(POOL_FLAG_NON_PAGED, other.m_capacity * sizeof(T), POOLTAG); // this can throw
			if (!temp)
				ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception
			RtlCopyMemory(m_data, other.m_data, other.m_size * sizeof(T)); // no need to copy all capacity, size is enough


			if (m_capacity) //discard existing memory
				ExFreePool2(m_data, POOLTAG, NULL, NULL);

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

	Vector(Vector&& other) noexcept //move constructor
	{
		//no std::swap available so trivial method used
		
		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;

		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	Vector& operator=(Vector&& other) noexcept //move assignment operator
	{ 
		if (this == &other)
			return *this;

		if (m_capacity) //discard existing memory
			ExFreePool2(m_data, POOLTAG, NULL, NULL);

		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;

		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;

		return *this;
	}

	SIZE_T size() const noexcept { return m_size; }
	SIZE_T capacity() const noexcept { return m_capacity; }

	T& at(ULONG idx) const
	{
		if (idx > (m_size - 1))
		{
			ExRaiseStatus(STATUS_INDEX_OUT_OF_BOUNDS);// SEH exception
		}

		return m_data[idx];
	}

	VOID push_back(const T& object)
	{
		++m_size;
		
		if (m_size > m_capacity)
			increase_capacity(m_size);

		m_data[m_size - 1] = object;
	}

	VOID push_front(const T& object)
	{
		++m_size;

		if (m_size > m_capacity)
			increase_capacity(m_size);

		RtlMoveMemory(&m_data[1], m_data, (m_size - 1) * sizeof(T));

		m_data[0] = object;
	}

	const T& front() const
	{
		if (m_size > 0)
			return m_data[0];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	T& front()
	{
		if (m_size > 0)
			return m_data[0];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	const T& back() const
	{
		if (m_size > 0)
			return m_data[m_size - 1];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	T& back()
	{
		if (m_size > 0)
			return m_data[m_size - 1];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	T& operator[](SIZE_T idx)
	{
		return m_data[idx];
	}

	T operator[](SIZE_T idx) const
	{
		return m_data[idx];
	}

	const T* data() const noexcept
	{
		return m_data;
	}

	T* data() noexcept
	{
		return m_data;
	}

	VOID swap(Vector& other) noexcept
	{
		auto swap = []<typename T>(T& first, T& second) -> void
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

	BOOLEAN empty() noexcept
	{
		return !m_size;
	}

	VOID clear()
	{
		if (m_data)
		{
			ExFreePool2(m_data, POOLTAG, NULL, NULL);
		}
		m_size = 0;
		m_capacity = 0;
		m_data = 0;
	}


private:

	VOID increase_capacity(SIZE_T required_size)
	{

		if (required_size <= m_capacity) // sanity check
			return;
		else if (m_capacity == 0)
			m_capacity = 1;

		while (m_capacity < required_size)
			m_capacity *= 2;

		T* new_buffer = static_cast<T*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, byte_capacity(), POOLTAG));
		if (!new_buffer)
			ExRaiseStatus(STATUS_HEAP_ALLOCATION_FAILED);// SEH exception

		if (m_data)
		{
			RtlCopyMemory(new_buffer, m_data, byte_size());
			ExFreePool2(m_data, POOLTAG, NULL, NULL);
		}

		m_data = new_buffer;
	}
};

