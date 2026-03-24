# kVector
std::vector implementation in Windows Kernel

# Usage
Just import Vector.h, it is a template class so no cpp file.

## Implemented functions
I tried to replicate their semantics as similar to std as I could
- size
- capacity
- at
- push_back
- push_front
- front
- back
- data
- swap
- empty
- clear
- resize
- begin
- end

# Docs
## Exceptions
I used SEH exceptions in place of the normal ones.

They can be caught like this:
```
__try
{

}
__except (EXCEPTION_EXECUTE_HANDLER)
{

}
```

## Memory allocation
I wanted to make sure that on newer kernel versions the newer memory allocation API will be used so I implemented a runtime check to determine whether the newer API(ExAllocatePool2) is present, if not, the older API(ExAllocatePoolWithTag) will be used.

## Example usage
```
	UNICODE_STRING us[5];
	Vector<UNICODE_STRING> usvec{};
	for (int i = 0; i < 5; ++i)
	{
		RtlInitUnicodeString(&us[i], L"Unicode_string!");
		usvec.push_back(us[i]);
	}

	auto print_unicode_string = [](UNICODE_STRING us) -> void
		{
			Vector<WCHAR> buff;
			for (int i = 0; i < us.Length; ++i)
				buff.push_back(us.Buffer[i]);
			buff.push_back(L'\0');
			DbgPrintEx(0, 0, "%ls", buff.data());
		};

	for (const auto& z : usvec)
	{
		print_unicode_string(z);
	}
```
