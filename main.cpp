#include "pch.h"


#define DEVICE_NAME L"\\Device\\kvector"
#define SYMBOLIC_NAME L"\\DosDevices\\kvector"

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path);
DRIVER_UNLOAD DriverUnload;

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	//UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);

	UNICODE_STRING device_name{}, symbolic_name{};

	RtlInitUnicodeString(&device_name, DEVICE_NAME);
	RtlInitUnicodeString(&symbolic_name, SYMBOLIC_NAME);

	NTSTATUS status{};

	PDEVICE_OBJECT device_object{};
	status = IoCreateDevice(driver_object, NULL, &device_name, FILE_DEVICE_UNKNOWN, NULL, FALSE, &device_object);
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

	driver_object->DriverUnload = DriverUnload;


	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT driver_object)
{
	UNICODE_STRING symbolic_name;
	RtlInitUnicodeString(&symbolic_name, SYMBOLIC_NAME);

	IoDeleteSymbolicLink(&symbolic_name);
	IoDeleteDevice(driver_object->DeviceObject);
	DbgPrintEx(0, 0, "Driver Unloaded\n");
}