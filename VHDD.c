#include "VHDD.h"
#include "ntintsafe.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VHDDEvtDriverDeviceAdd)
//#pragma alloc_text(PAGE, VHDDEvtCleanUpCallback)
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PCUNICODE_STRING RegistryPath)
{
	WDF_DRIVER_CONFIG Config;

	KdPrint("DriverEntry routine");

	WDF_DRIVER_CONFIG_INIT(&Config, VHDDEvtDriverDeviceAdd);
	
return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &Config, WDF_NO_HANDLE);
}

NTSTATUS VHDDEvtDriverDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit)
{
	WDFDEVICE Device;
	WDF_OBJECT_ATTRIBUTES DeviceAttributes;
	NTSTATUS status;
	PDEVICE_EXTENSION pDeviceExtension;
	DECLARE_CONST_UNICODE_STRING(DevName, MY_DEVICE_NAME);

	PAGED_CODE();
    UNREFERENCED_PARAMETER(Driver);
	
	KdPrint("VHDDEvtDriverDeviceAdd routine");
	
	if(!NT_SUCCESS(status = WdfDeviceInitAssignName(DeviceInit, &DevName))) return status;
	
	WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
	WdfDeviceInitSetExclusive(DeviceInit, FALSE);
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect); //??WdfDeviceIoBuffered??
	
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&DeviceAttributes, DEVICE_EXTENSION);
	//DeviceAttributes.EvtCleanupCallback = VHDDEvtCleanUpCallback;
	if(!NT_SUCCESS(status = WdfDeviceCreate(&DeviceInit, &DeviceAttributes, &Device))) return status;
	pDeviceExtension = DeviceGetExtension(Device);
	
	
	return STATUS_SUCCESS;
}
/*
VOID VHDDEvtCleanUpCallback(IN WDFOBJECT Object)
{
	KdPrint("VHDDEvtCleanUpCallback routine");
}
*/
