#ifndef _VHDD_H
#define _VHDD_H

#include <ntddk.h>
#include <wdf.h>
#include <ntstrsafe.h>
#include <wdfroletypes.h>
#include <ntdddisk.h>
#include <ntddstor.h>
#include <WdfControl.h>
#include <mountmgr.h>
#define NT_DEVICE_NAME                  L"\\Device\\VHDD"
#define DOS_DEVICE_NAME                 L"\\DosDevices\\"

#define VHDD_TAG						'DDHV'
#define DOS_DEVNAME_LENGTH              (sizeof(DOS_DEVICE_NAME)+sizeof(WCHAR)*10)
#define DRIVE_LETTER_LENGTH             (sizeof(WCHAR)*10)

#define DRIVE_LETTER_BUFFER_SIZE        10
#define DOS_DEVNAME_BUFFER_SIZE         (sizeof(DOS_DEVICE_NAME) / 2) + 10

#define VHDD_MEDIA_TYPE              0xF8
#define DIR_ENTRIES_PER_SECTOR          16

#define DEFAULT_DISK_SIZE               (1024*1024*64)     // 1 MB
#define DEFAULT_ROOT_DIR_ENTRIES        512
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"Z:"

typedef struct _DISK_INFO {
    ULONG					DiskSize;           // Ramdisk size in bytes
    ULONG					RootDirEntries;     // No. of root directory entries
    ULONG					SectorsPerCluster;  // Sectors per cluster
    UNICODE_STRING			DriveLetter;		// Drive letter to be used
} DISK_INFO, *PDISK_INFO;

typedef struct _DEVICE_EXTENSION{
    PUCHAR					DiskImage;                  // Pointer to beginning of disk image
    DISK_GEOMETRY			DiskGeometry;               // Drive parameters built by Ramdisk
    DISK_INFO				DiskRegInfo;                // Disk parameters from the registry
    UNICODE_STRING			SymbolicLink;               // Dos symbolic name; Drive letter
	PARTITION_INFORMATION	Partition;
    WCHAR					DriveLetterBuffer[DRIVE_LETTER_BUFFER_SIZE];
    WCHAR					DosDeviceNameBuffer[DOS_DEVNAME_BUFFER_SIZE];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, DeviceGetExtension)

typedef struct _QUEUE_EXTENSION{
	PDEVICE_EXTENSION	DeviceExtension;
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
VOID VHDDQueryDiskRegParameters(IN PWSTR RegistryPath,IN PDISK_INFO DiskRegInfo);

#endif