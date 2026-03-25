#include "pch.h"
#include "Vector.h"


#define DEVICE_NAME L"\\Device\\kvector"
#define SYMBOLIC_NAME L"\\DosDevices\\kvector"

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
DRIVER_UNLOAD DriverUnload;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	//UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING device_name{}, symbolic_name{};

	RtlInitUnicodeString(&device_name, DEVICE_NAME);
	RtlInitUnicodeString(&symbolic_name, SYMBOLIC_NAME);

	NTSTATUS status{};

	PDEVICE_OBJECT device_object{};
	status = IoCreateDevice(DriverObject, NULL, &device_name, FILE_DEVICE_UNKNOWN, NULL, FALSE, &device_object);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "IoCreateDevice failed with status 0x%X\n", status);
		RtlFreeUnicodeString(&device_name);
		RtlFreeUnicodeString(&symbolic_name);
		return STATUS_UNSUCCESSFUL;
	}

	status = IoCreateSymbolicLink(&symbolic_name, &device_name);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "IoCreateSymbolicLink failed with status 0x%X\n", status);
		RtlFreeUnicodeString(&device_name);
		RtlFreeUnicodeString(&symbolic_name);
		return STATUS_UNSUCCESSFUL;
	}

	DriverObject->DriverUnload = DriverUnload;

	//Vector<ULONG> vec;

	/*Vector<ULONG> vec;
	for (int i = 0; i < 12; ++i)
		vec.push_back(i);

	for (SIZE_T i = 0; i < vec.size(); ++i)
		DbgPrintEx(0, 0, "%ld\n", vec[i]);

	DbgPrintEx(0, 0, "back: %ld\n", vec.back());
	DbgPrintEx(0, 0, "front: %ld\n", vec.front());

	vec.clear();

	DbgPrintEx(0, 0, "front: %ld\n", vec.front());*/

	UNICODE_STRING us[5];
	Vector<UNICODE_STRING> usvec{};
	for (int i = 0; i < 5; ++i)
	{
		RtlInitUnicodeString(&us[i], L"Unicode_string!");
		usvec.push_back(us[i]);
	}

	//bruh
	auto print_unicode_string = [](UNICODE_STRING us) -> void
		{
			Vector<WCHAR> buff;
			for (int i = 0; i < us.Length; ++i)
				buff.push_back(us.Buffer[i]);
			buff.push_back(L'\0');
			DbgPrintEx(0, 0, "%ls", buff.data());
		};

	for (SIZE_T i = 0; i < usvec.size(); ++i)
	{
		print_unicode_string(usvec[i]);
	}

	for (const auto& z : usvec) // range based for loop  ...hooray
	{
		print_unicode_string(z);
	}


	/*Vector<ULONG> vec(10, 5);
	for (SIZE_T i = 0; i < vec.size(); ++i)
		DbgPrintEx(0, 0, "%ld\n", vec[i]);*/

	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symbolic_name;
	RtlInitUnicodeString(&symbolic_name, SYMBOLIC_NAME);

	IoDeleteSymbolicLink(&symbolic_name);
	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrintEx(0, 0, "Driver Unloaded\n");
}


//#define DEVICE_NAME     L"\\Device\\OpenAV"
//#define SYMBOLIC_NAME   L"\\DosDevices\\OpenAV"
//
//NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
//DRIVER_UNLOAD DriverUnload;
//
//
//NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
//{
//    UNICODE_STRING devName, symName;
//    PDEVICE_OBJECT deviceObject = NULL;
//    NTSTATUS status;
//
//    UNREFERENCED_PARAMETER(RegistryPath);
//
//    RtlInitUnicodeString(&devName, DEVICE_NAME);
//    RtlInitUnicodeString(&symName, SYMBOLIC_NAME);
//
//    status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
//    if (!NT_SUCCESS(status))
//    {
//        DbgPrintEx(0, 0, "IoCreateDevice failed: 0x%X\n", status);
//        return status;
//    }
//
//    status = IoCreateSymbolicLink(&symName, &devName);
//    if (!NT_SUCCESS(status))
//    {
//        DbgPrintEx(0, 0, "IoCreateSymbolicLink failed: 0x%X\n", status);
//        IoDeleteDevice(deviceObject);
//        return status;
//    }
//
//    DriverObject->DriverUnload = DriverUnload;
//
//    // device initialized
//    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
//
//    DbgPrintEx(0, 0, "Driver loaded\n");
//
//    return STATUS_SUCCESS;
//}
//
//VOID DriverUnload(PDRIVER_OBJECT DriverObject)
//{
//    UNICODE_STRING symName;
//    RtlInitUnicodeString(&symName, SYMBOLIC_NAME);
//
//    IoDeleteSymbolicLink(&symName);
//    IoDeleteDevice(DriverObject->DeviceObject);
//    DbgPrintEx(0, 0, "Driver unloaded\n");
//}