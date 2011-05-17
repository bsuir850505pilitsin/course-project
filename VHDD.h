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

#define VHDD_MEDIA_TYPE					0xF8					//тип носителя - стационарный диск
#define DIR_ENTRIES_PER_SECTOR          16

#define DEFAULT_DISK_SIZE               (128*1024*1024)			//стандартный размер
#define DEFAULT_ROOT_DIR_ENTRIES        512						//максимальное количество файлов в корневом каталоге
#define DEFAULT_SECTORS_PER_CLUSTER     2
#define DEFAULT_DRIVE_LETTER            L"V:"					//Буква диска

typedef struct _DISK_INFO {
    ULONG					DiskSize;           // Размер диска в байтах
    ULONG					RootDirEntries;     // Количество файлов в корневом каталоге
    ULONG					SectorsPerCluster;  // Количество секторов на кластер
    UNICODE_STRING			DriveLetter;		// Буква диска
} DISK_INFO, *PDISK_INFO;

typedef struct _DEVICE_EXTENSION{
    PUCHAR					DiskImage;                  // Указатель на начало образа диска
    DISK_GEOMETRY			DiskGeometry;               // Параметры диска
    DISK_INFO				DiskRegInfo;                // Параметры диска (реестр)
    UNICODE_STRING			SymbolicLink;               // Символическое имя диска
    WCHAR					DriveLetterBuffer[DRIVE_LETTER_BUFFER_SIZE];
    WCHAR					DosDeviceNameBuffer[DOS_DEVNAME_BUFFER_SIZE];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, DeviceGetExtension)

typedef struct _QUEUE_EXTENSION{
	PDEVICE_EXTENSION	DeviceExtension;
} QUEUE_EXTENSION, *PQUEUE_EXTENSION;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_EXTENSION, QueueGetExtension)

#pragma pack(1)
///структура загрузочного сектора для файловой системы FAT (FAT12/16)
typedef struct  _BOOT_SECTOR
{
    UCHAR       bsJump[3];          // Ассемблерная команда перехода к загрузочному коду
    CCHAR       bsOemName[8];       // Имя OEM в кодировке ASCII
    USHORT      bsBytesPerSec;      // Количество байтов в секторе
    UCHAR       bsSecPerClus;       // Количество секторов в кластере (блоке данных)
    USHORT      bsResSectors;       // Размер зарезервированной области в секторах
    UCHAR       bsFATs;             // Количество копий FAT
    USHORT      bsRootDirEnts;      // Максимальное количество файлов в корневом каталоге
    USHORT      bsSectors;          // 16-разрядное количество секторов в файловой системе
    UCHAR       bsMedia;            // Тип носителя (0xF8)
    USHORT      bsFATsecs;          // 16-разрядный размер (в секторах) каждой копии FAT 
    USHORT      bsSecPerTrack;      // Количество секторов в дорожке (32)
    USHORT      bsHeads;            // Количество головок (2)
    ULONG       bsHiddenSecs;       // Количество секторов перед началом раздела (0)
    ULONG       bsHugeSectors;      // 32-разрядное количество секторов в файловой системе
    UCHAR       bsDriveNumber;      // Номер диска BIOS INT13h (не используется)
    UCHAR       bsReserved1;        // не используется
    UCHAR       bsBootSignature;    // Расширенная сигнатура, которая показывает, действительны ли следующие три значения. (0x29)
    ULONG       bsVolumeID;         // Серийный номер тома (0x12345678)
    CCHAR       bsLabel[11];        // Метка тома в кодировке ASCII
    CCHAR       bsFileSystemType[8];// Метка типа файловой системы в кодировке ASCII 
    CCHAR       bsReserved2[448];   // Не используется
    UCHAR       bsSig2[2];          // Сигнатура (0xAA55)
}   BOOT_SECTOR, *PBOOT_SECTOR;
///Записи каталогов в системе FAT
typedef struct  _DIR_ENTRY
{
    UCHAR       deName[8];          // Имя файла
    UCHAR       deExtension[3];     // Расширение файла
    UCHAR       deAttributes;       // Атрибуты файла
    UCHAR       deReserved;         // не используется
    USHORT      deTime;             // Время создания
    USHORT      deDate;             // День создания
    USHORT      deStartCluster;     // Адрес первого кластера
    ULONG       deFileSize;         // Размер файла
}   DIR_ENTRY, *PDIR_ENTRY;
#pragma pack()
///Флаги поля атрибутов в записи каталога
#define DIR_ATTR_READONLY   0x01	// Доступ только для чтения
#define DIR_ATTR_HIDDEN     0x02	// Скрытый файл
#define DIR_ATTR_SYSTEM     0x04	// Системный файл
#define DIR_ATTR_VOLUME     0x08	// Метка тома
#define DIR_ATTR_DIRECTORY  0x10	// Каталог
#define DIR_ATTR_ARCHIVE    0x20	// Архивный файл

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

