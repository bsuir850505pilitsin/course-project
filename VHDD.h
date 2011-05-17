#ifndef _VHDD_H
#define _VHDD_H

#include <ntddk.h>
#include <ntddvol.h>
#include <ntdddisk.h>
#include <wdf.h>
#include <ntstrsafe.h>
#include <wdfroletypes.h>
#include <ntddstor.h>
#include <WdfControl.h>
#include <mountmgr.h>
#define NT_DEVICE_NAME                  L"\\Device\\VHDD"
#define DOS_DEVICE_NAME                 L"\\DosDevices\\"
#define MOUNTDEV_LINK_NAME				L"\\DosDevices\\V:"
#define VHDD_TAG						'DDHV'
#define DOS_DEVNAME_LENGTH              (sizeof(DOS_DEVICE_NAME)+sizeof(WCHAR)*10)
#define DRIVE_LETTER_LENGTH             (sizeof(WCHAR)*10)

#define DRIVE_LETTER_BUFFER_SIZE        10
#define DOS_DEVNAME_BUFFER_SIZE         (sizeof(DOS_DEVICE_NAME) / 2) + 10

#define VHDD_MEDIA_TYPE					0xF8					//��� �������� - ������������ ����
#define DIR_ENTRIES_PER_SECTOR          16

#define DEFAULT_DISK_SIZE               (128*1024*1024)			//����������� ������
#define DEFAULT_ROOT_DIR_ENTRIES        512						//������������ ���������� ������ � �������� ��������
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"V:"					//����� �����

typedef struct _DISK_INFO {
    ULONG					DiskSize;           // ������ ����� � ������
    ULONG					RootDirEntries;     // ���������� ������ � �������� ��������
    ULONG					SectorsPerCluster;  // ���������� �������� �� �������
    UNICODE_STRING			DriveLetter;		// ����� �����
} DISK_INFO, *PDISK_INFO;

typedef struct _DEVICE_EXTENSION{
    PUCHAR					DiskImage;                  // ��������� �� ������ ������ �����
    DISK_GEOMETRY			DiskGeometry;               // ��������� �����
    DISK_INFO				DiskRegInfo;                // ��������� ����� (������)
    UNICODE_STRING			SymbolicLink;               // ������������� ��� �����
    WCHAR					DriveLetterBuffer[DRIVE_LETTER_BUFFER_SIZE];
    WCHAR					DosDeviceNameBuffer[DOS_DEVNAME_BUFFER_SIZE];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, DeviceGetExtension)

typedef struct _QUEUE_EXTENSION{
	PDEVICE_EXTENSION	DeviceExtension;
} QUEUE_EXTENSION, *PQUEUE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION, QueueGetExtension)

#pragma pack(1)
///��������� ������������ ������� ��� �������� ������� FAT (FAT12/16)
typedef struct  _BOOT_SECTOR
{
    UCHAR       bsJump[3];          // ������������ ������� �������� � ������������ ����
    CCHAR       bsOemName[8];       // ��� OEM � ��������� ASCII
    USHORT      bsBytesPerSec;      // ���������� ������ � �������
    UCHAR       bsSecPerClus;       // ���������� �������� � �������� (����� ������)
    USHORT      bsResSectors;       // ������ ����������������� ������� � ��������
    UCHAR       bsFATs;             // ���������� ����� FAT
    USHORT      bsRootDirEnts;      // ������������ ���������� ������ � �������� ��������
    USHORT      bsSectors;          // 16-��������� ���������� �������� � �������� �������
    UCHAR       bsMedia;            // ��� �������� (0xF8)
    USHORT      bsFATsecs;          // 16-��������� ������ (� ��������) ������ ����� FAT 
    USHORT      bsSecPerTrack;      // ���������� �������� � ������� (32)
    USHORT      bsHeads;            // ���������� ������� (2)
    ULONG       bsHiddenSecs;       // ���������� �������� ����� ������� ������� (0)
    ULONG       bsHugeSectors;      // 32-��������� ���������� �������� � �������� �������
    UCHAR       bsDriveNumber;      // ����� ����� BIOS INT13h (�� ������������)
    UCHAR       bsReserved1;        // �� ������������
    UCHAR       bsBootSignature;    // ����������� ���������, ������� ����������, ������������� �� ��������� ��� ��������. (0x29)
    ULONG       bsVolumeID;         // �������� ����� ���� (0x12345678)
    CCHAR       bsLabel[11];        // ����� ���� � ��������� ASCII
    CCHAR       bsFileSystemType[8];// ����� ���� �������� ������� � ��������� ASCII 
    CCHAR       bsReserved2[448];   // �� ������������
    UCHAR       bsSig2[2];          // ��������� (0xAA55)
}   BOOT_SECTOR, *PBOOT_SECTOR;
///������ ��������� � ������� FAT
typedef struct  _DIR_ENTRY
{
    UCHAR       deName[8];          // ��� �����
    UCHAR       deExtension[3];     // ���������� �����
    UCHAR       deAttributes;       // �������� �����
    UCHAR       deReserved;         // �� ������������
    USHORT      deTime;             // ����� ��������
    USHORT      deDate;             // ���� ��������
    USHORT      deStartCluster;     // ����� ������� ��������
    ULONG       deFileSize;         // ������ �����
}   DIR_ENTRY, *PDIR_ENTRY;
#pragma pack()
///����� ���� ��������� � ������ ��������
#define DIR_ATTR_READONLY   0x01	// ������ ������ ��� ������
#define DIR_ATTR_HIDDEN     0x02	// ������� ����
#define DIR_ATTR_SYSTEM     0x04	// ��������� ����
#define DIR_ATTR_VOLUME     0x08	// ����� ����
#define DIR_ATTR_DIRECTORY  0x10	// �������
#define DIR_ATTR_ARCHIVE    0x20	// �������� ����

EVT_WDF_DRIVER_DEVICE_ADD VHDDEvtDriverDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP VHDDEvtCleanupCallback;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  VHDDEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_WRITE  VHDDEvtIoWrite;
EVT_WDF_IO_QUEUE_IO_READ  VHDDEvtIoRead;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL VHDDEvtIoInternalDeviceControl;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PCUNICODE_STRING RegistryPath);
NTSTATUS VHDDEvtDriverDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);
VOID VHDDEvtCleanupCallback(IN WDFOBJECT Object);
VOID VHDDEvtIoDeviceControl(IN WDFQUEUE  Queue, IN WDFREQUEST  Request, IN size_t OutputBufferLength, IN size_t InputBufferLength, IN ULONG  IoControlCode);
VOID VHDDEvtIoRead(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length);
VOID VHDDEvtIoWrite(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length);
VOID VHDDQueryDiskRegParameters(IN PWSTR RegistryPath,IN PDISK_INFO DiskRegInfo);
VOID VHDDEvtIoInternalDeviceControl(IN WDFQUEUE  Queue, IN WDFREQUEST  Request, IN size_t OutputBufferLength, IN size_t InputBufferLength, IN ULONG  IoControlCode);
NTSTATUS VHDDFormatDisk(IN PDEVICE_EXTENSION DeviceExtension);
VOID CheckStatus(NTSTATUS Status);
#endif

