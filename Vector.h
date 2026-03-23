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


	SIZE_T type_size{ sizeof(T) };
	SIZE_T byte_size() { return m_size * sizeof(T); }
	SIZE_T byte_capacity() { return m_capacity * sizeof(T); }
public:
	Vector() = default;
	Vector(SIZE_T initial_size, const T& val)
	{
		increase_capacity(initial_size);
		resize(initial_size, val);
	}

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

		if (!other.empty())
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
				ExFreePool2(m_data, POOLTAG, NULL, NULL);
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

	T& front()
	{
		if (!empty())
			return m_data[0];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	//const reference front() is for const vectors for read-only access 
	const T& front() const
	{
		if (!empty())
			return m_data[0];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	T& back()
	{
		if (!empty())
			return m_data[m_size - 1];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	//const reference back() is for const vectors for read-only access 
	const T& back() const
	{
		if (!empty())
			return m_data[m_size - 1];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	//const reference operator[] is for const vectors for read-only access
	const T& operator[](SIZE_T idx) const
	{
		return m_data[idx];
	}

	T& operator[](SIZE_T idx)
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

	VOID resize(SIZE_T new_size, const T& val = T{})
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

	T* begin()
	{
		if (!empty())
			return m_data;
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	const T* begin() const
	{
		if (!empty())
			return m_data;
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	T* end()
	{
		if (!empty())
			return &m_data[m_size];
		else
			ExRaiseStatus(STATUS_EMPTY);
	}

	const T* end() const
	{
		if (!empty())
			return &m_data[m_size];
		else
			ExRaiseStatus(STATUS_EMPTY);
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

