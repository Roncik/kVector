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


	Vector<ULONG> vec;
	for (int i = 0; i < 12; ++i)
		vec.push_back(i);

	DbgPrintEx(0, 0, "%ld\n", vec.at(6));
	DbgPrintEx(0, 0, "%ld\n", vec.at(8));
	DbgPrintEx(0, 0, "%ld\n", vec.at(0));
	DbgPrintEx(0, 0, "%ld\n", vec.at(11));
	__try
	{
		DbgPrintEx(0, 0, "%ld\n", vec.at(12));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrintEx(0, 0, "exception \"STATUS_INDEX_OUT_OF_BOUNDS\" thrown\n");
	}

	DbgPrintEx(0, 0, "%ld\n", vec.at(0));
	vec.push_front(67);
	DbgPrintEx(0, 0, "%ld\n", vec.at(0));
	DbgPrintEx(0, 0, "%ld\n", vec.at(1));
	DbgPrintEx(0, 0, "%ld\n", vec.at(2));

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