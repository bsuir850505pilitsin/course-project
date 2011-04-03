#ifndef _VHDD_H
#define _VHDD_H

#include <ntddk.h>
#include <wdf.h>
#include <ntstrsafe.h>

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


EVT_WDF_DRIVER_DEVICE_ADD VHDDEvtDriverDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP VHDDEvtCleanUpCallback;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PCUNICODE_STRING RegistryPath);
NTSTATUS VHDDEvtDriverDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);
VOID VHDDEvtCleanUpCallback(IN WDFOBJECT Object);

#endif