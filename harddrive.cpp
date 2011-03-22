// harddrive.c RAMHDD
//Information "FOR ME"
/*  Oni WDM(h1, p66)
DriverObject: 
	DeviceObject  				//linked list of device objects (1 for evere device supported by driver)
	DriverExtension->AddDevice 	//AddDevice function pointer (create device object)
	HardwareDatabase 			//registry path to directory of this device(unused in WDM)
	FastIoDispatch 				//used for FS and net drivers
	DriverInit 					// ???
	DriverStartIo 				//point to driver's function, that proccess i/o requests (by i/o Manager) (Oni WDM (h5,??))
	DriverUnload 				//deinitialize
	MajorFunction 				//table of pointers on the functions that proccess every of 20+ types of i/o requests
*/
/* Oni WDM(h1, p69)
DeviceObject (created by IoCreateDevice routine):
	DriverObject 				//link to the DriverObject that creating this DeviceObject
	NextDevice					//link to the next DeviceObject that belongs to the same DriverObject (not interesting for WDM)
	CurrentIrp 					//IRP queue (not interesting for WDM, because WDM must be realize their own queue)
	Flags						//bit flags (Oni WDM(h1, 69) t2.2)
	Characteristics				//additional bit flags (Oni WDM(h1, 70) t2.3)
	DeviceExtension 			//pointer on sruct that contain information about specific device
	DeviceType					//enum,  argument in the call of IoCreateDevice (look WDK!!!)
	StackSize					// ?? number of DeviceObject in stack ?? (not interesing in WMD, use IoAttachDeviceToDeviceStack)
	AlingmentRequirement 		//alingment of data buffets 2^n-1
*/
/* DeviceTree 2.20
functions supported by physical HDD:
	IRP_MJ_CREATE
	IRP_MJ_CLOSE
	IRP_MJ_READ
	IRP_MJ_WRITE
	IRP_MJ_FLUSH_BUFFERS
	IRP_MJ_DEVICE_CONTROL
	IRP_MJ_INTERNAL_DEVICE_CONTROL
	IRP_MJ_SHUTDOWN
	IRP_MJ_QUERY_POWER
	IRP_MJ_SET_POWER
*/	
/*
Plug'n'Play
	IRP_MJ_PNP
	IRP_MJ_POWER
	IRP_MJ_SYSTEM_CONTROL
*/
#include <ntddk.h>

//this struct will be changed with time... but before i must understand, which information should be there for my needs...
typedef struct _DEVICE_EXTENSION{
	PDEVICE_OBJECT	DeviceObject;
	PDEVICE_OBJECT	LowerDeviceObject;	//pointer in stack on the device below 
	PDEVICE_OBJECT	Pdo;				//pointer on the DeviceObject
	UNICODE_STRING	ifname;				//device interface name
	IO_REMOVE_LOCK	RemoveLock;			//for safety delete device with call IoDeleteDevice
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//ms-help://MS.WDK.v10.7600.091201/DevTest_g/hh/DevTest_g/staticdv_50914dab-fd5f-4301-8c0a-18402b101282.xml.htm
DRIVER_INITIALIZE	DriverEntry;
DRIVER_ADD_DEVICE	myAddDevice;
DRIVER_UNLOAD		DriverUnload;

NTSTATUS		DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS		myAddDevice(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT pdo);
VOID			DriverUnload(IN PDRIVER_OBJECT DriverObject);

UNICODE_STRING	DeviceName;
UNICODE_STRING	SymbolicLinkName;

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, myAddDevice)
#pragma alloc_text(PAGE, DriverUnload)

//RegistryPath, needed in WMI (Oni WDM (h10, ??))
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverExtension->AddDevice = myAddDevice;
	DriverObject->DriverUnload = DriverUnload;
	
	//
	//
	//
	DbgPrint("DriverEntry routine");
	 
	return STATUS_SUCCESS;
}
NTSTATUS myAddDevice(IN PDRIVER_OBJECT DriverObject,IN PDEVICE_OBJECT pdo)
{
	PDEVICE_OBJECT fdo;
	PDEVICE_EXTENSION dve;
	NTSTATUS status;
	DbgPrint("myAddDevice routine begin");
	//Oni(h1, p85) dynamic name if device more than 1
	/*static LONG lastindex = -1;
	LONG devindex = InterlockedIncrement(&lastindex);
	WCHAR name[32]
	_snwprintf(name, arraysize(name), L"\\Device\\RAMHDD%2.2d", devindex);
	RtlInitUnicodeString(&devname, name);*/
	//??
	RtlInitUnicodeString(&DeviceName, L"\\Device\\RAMHDD");
	//creating DeviceObject
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &fdo);
	dve = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	dve->DeviceObject = fdo;
	dve->Pdo = pdo;
	DbgPrint("=RAMHDD= FDO %d, DevExt=%d",fdo,dve);
	RtlInitUnicodeString(&SymbolicLinkName, L"\\Device\\RAMHDD");
	dve->ifname =  SymbolicLinkName;
	status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);
	//IoInitializeRemoveLock(&dve->RemoveLock, 0, 0, 0);
	//dve->devstate = STATE_INITIALIZED;
	//IoInitializeDpcRequest(fdo, DpcForIsr);
	fdo->Flags = DO_DIRECT_IO | DO_POWER_PAGABLE;
	dve->LowerDeviceObject = IoAttachDeviceToDeviceStack(fdo, pdo);
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	DbgPrint("myAddDevice routine end");
	return STATUS_SUCCESS;
}
VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("DriverUnload routine");
}