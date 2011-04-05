#include "VHDD.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VHDDEvtDriverDeviceAdd)
#pragma alloc_text(PAGE, VHDDEvtCleanupCallback)
//#pragma alloc_text(PAGE, VHDDEvtDriverUnload)
#endif

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PCUNICODE_STRING RegistryPath
	)
{
	WDF_DRIVER_CONFIG Config;

	DbgPrint("DriverEntry routine");

	WDF_DRIVER_CONFIG_INIT(&Config, VHDDEvtDriverDeviceAdd);
	//Config.EvtDriverUnload = VHDDEvtDriverUnload;
return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &Config, WDF_NO_HANDLE);
}

NTSTATUS VHDDEvtDriverDeviceAdd(
	IN WDFDRIVER Driver,
	IN PWDFDEVICE_INIT DeviceInit
	)
{
	WDFDEVICE Device;
	WDF_OBJECT_ATTRIBUTES DeviceAttributes;
	NTSTATUS status;
	PDEVICE_EXTENSION pDeviceExtension;
	WDF_OBJECT_ATTRIBUTES QueueAttributes;
	WDF_IO_QUEUE_CONFIG IOQueueConfig;
	WDFQUEUE  Queue;
	PQUEUE_EXTENSION  pQueueExtension;
	DECLARE_CONST_UNICODE_STRING(DevName, MY_DEVICE_NAME);
	PAGED_CODE();
    UNREFERENCED_PARAMETER(Driver);
	
	DbgPrint("VHDDEvtDriverDeviceAdd routine");
	
	if(!NT_SUCCESS(status = WdfDeviceInitAssignName(DeviceInit, &DevName)))
	{	
		DbgPrint("return after WdfDeviceInitAssignName");
		return status;
	}
	
	WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
	WdfDeviceInitSetExclusive(DeviceInit, FALSE);
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect); //??WdfDeviceIoBuffered??
	
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&DeviceAttributes, DEVICE_EXTENSION);
	DeviceAttributes.EvtCleanupCallback = VHDDEvtCleanupCallback;
	if(!NT_SUCCESS(status = WdfDeviceCreate(&DeviceInit, &DeviceAttributes, &Device)))
	{	
		DbgPrint("return after WdfDeviceCreate");
		return status;
	}
	pDeviceExtension = DeviceGetExtension(Device);
	
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE( &IOQueueConfig, WdfIoQueueDispatchSequential);
	
	IOQueueConfig.EvtIoDeviceControl = VHDDEvtIoDeviceControl;
    IOQueueConfig.EvtIoRead = VHDDEvtIoRead;
    IOQueueConfig.EvtIoWrite = VHDDEvtIoWrite;
	
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&QueueAttributes, QUEUE_EXTENSION);
	if(!NT_SUCCESS(status = WdfIoQueueCreate(Device, &IOQueueConfig, &QueueAttributes, &Queue)))
	{	
	switch(status){
		case STATUS_INVALID_PARAMETER : 
			DbgPrint("STATUS_INVALID_PARAMETER");
			break;
		case STATUS_INFO_LENGTH_MISMATCH : 
			DbgPrint("STATUS_INFO_LENGTH_MISMATCH");
			break;
		case STATUS_POWER_STATE_INVALID : 
			DbgPrint("STATUS_POWER_STATE_INVALID");
			break;
		case STATUS_INSUFFICIENT_RESOURCES : 
			DbgPrint("STATUS_INSUFFICIENT_RESOURCES");
			break;
		case STATUS_WDF_NO_CALLBACK : 
			DbgPrint("STATUS_WDF_NO_CALLBACK");
			break;
		case STATUS_UNSUCCESSFUL : 
			DbgPrint("STATUS_UNSUCCESSFUL");
			break;
		}
		DbgPrint("return after WdfIoQueueCreate");
		return status;
	}
	pQueueExtension = QueueGetExtension(Queue);
	pQueueExtension->DeviceExtension = pDeviceExtension;
	
	//parameters of VHDD in *_EXTENSION
	
	return STATUS_SUCCESS;
}

VOID VHDDEvtCleanupCallback(
	IN WDFOBJECT Device
	)
{
PAGED_CODE();
	DbgPrint("VHDDEvtCleanUpCallback routine");
}

VOID VHDDEvtIoDeviceControl (
    IN WDFQUEUE  Queue,
    IN WDFREQUEST  Request,
    IN size_t  OutputBufferLength,
    IN size_t  InputBufferLength,
    IN ULONG  IoControlCode
    )
{
	DbgPrint("VHDDEvtIoDeviceControl routine");
}

VOID VHDDEvtIoRead(
	IN WDFQUEUE  Queue,
	IN WDFREQUEST  Request,
	IN size_t  Length
	)
{
	DbgPrint("VHDDEvtIoRead routine");
}

VOID VHDDEvtIoWrite(
	IN WDFQUEUE  Queue,
	IN WDFREQUEST  Request,
	IN size_t  Length
	)
{
	DbgPrint("VHDDEvtIoWrite routine");
}
/*VOID VHDDDriverUnload (
    IN WDFDRIVER  Driver
    )
{
	DbgPrint("VHDDDriverUnload routine");
}
*/