#ifndef _VHDD_H
#define _VHDD_H

#include <ntddk.h>
#include <wdf.h>
#include <ntstrsafe.h>
#include <wdfroletypes.h>
#define MY_DEVICE_NAME L"\\Device\\VHDD"

typedef struct _DEVICE_EXTENSION{
	ULONG			Size;
	PDEVICE_OBJECT	DeviceObject;
	PDEVICE_OBJECT	LowerDeviceObject;	//pointer in stack on the device below 
	PDEVICE_OBJECT	Pdo;				//pointer on the DeviceObject
	UNICODE_STRING	ifname;				//device interface name
	IO_REMOVE_LOCK	RemoveLock;			//for safety delete device with call IoDeleteDevice
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, DeviceGetExtension)

typedef struct _QUEUE_EXTENSION{
	ULONG			Size;
	PDEVICE_EXTENSION DeviceExtension;
} QUEUE_EXTENSION, *PQUEUE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION, QueueGetExtension)

EVT_WDF_DRIVER_DEVICE_ADD VHDDEvtDriverDeviceAdd;
//EVT_WDF_DRIVER_UNLOAD VHDDEvtDriverUnload;
EVT_WDF_OBJECT_CONTEXT_CLEANUP VHDDEvtCleanupCallback;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  VHDDEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_WRITE  VHDDEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_READ  VHDDEvtIoRead;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PCUNICODE_STRING RegistryPath);
NTSTATUS VHDDEvtDriverDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);
//VOID VHDDEvtDriverUnload(IN WDFDRIVER  Driver);
VOID VHDDEvtCleanupCallback(IN WDFOBJECT Object);
VOID VHDDEvtIoDeviceControl(IN WDFQUEUE  Queue, IN WDFREQUEST  Request, IN size_t OutputBufferLength, IN size_t InputBufferLength, IN ULONG  IoControlCode);
VOID VHDDEvtIoRead(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length);
VOID VHDDEvtIoWrite(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length);
#endif