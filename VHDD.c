#include "VHDD.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VHDDEvtDriverDeviceAdd)
#pragma alloc_text(PAGE, VHDDEvtCleanupCallback)
#pragma alloc_text(PAGE, VHDDQueryDiskRegParameters)
#pragma alloc_text(PAGE, VHDDFormatDisk)
#endif

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PCUNICODE_STRING RegistryPath
	)
{
	WDF_DRIVER_CONFIG Config;

	DbgPrint("DriverEntry routine");
	WDF_DRIVER_CONFIG_INIT(&Config, VHDDEvtDriverDeviceAdd);
return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &Config, WDF_NO_HANDLE);
}

NTSTATUS VHDDEvtDriverDeviceAdd(
	IN WDFDRIVER Driver,
	IN PWDFDEVICE_INIT DeviceInit
	)
{
	WDFDEVICE				Device;
	WDF_OBJECT_ATTRIBUTES	DeviceAttributes;
	NTSTATUS				status;
	PDEVICE_EXTENSION		pDeviceExtension;
	WDF_OBJECT_ATTRIBUTES	QueueAttributes;
	WDF_IO_QUEUE_CONFIG		IOQueueConfig;
	WDFQUEUE				Queue;
	PQUEUE_EXTENSION		pQueueExtension;
	DECLARE_CONST_UNICODE_STRING(DevName, NT_DEVICE_NAME);

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
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&DeviceAttributes, DEVICE_EXTENSION);
	DeviceAttributes.EvtCleanupCallback = VHDDEvtCleanupCallback;

	if(!NT_SUCCESS(status = WdfDeviceCreate(&DeviceInit, &DeviceAttributes, &Device)))
	{	
		CheckStatus(status);
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
		CheckStatus(status);
		DbgPrint("return after WdfIoQueueCreate");
		return status;
	}
	pQueueExtension = QueueGetExtension(Queue);
	pQueueExtension->DeviceExtension = pDeviceExtension;

    pDeviceExtension->DiskRegInfo.DriveLetter.Buffer =
        (PWSTR) &pDeviceExtension->DriveLetterBuffer;
    pDeviceExtension->DiskRegInfo.DriveLetter.MaximumLength =
        sizeof(pDeviceExtension->DriveLetterBuffer);

    VHDDQueryDiskRegParameters(WdfDriverGetRegistryPath(WdfDeviceGetDriver(Device)),&pDeviceExtension->DiskRegInfo);
	pDeviceExtension->DiskImage != NULL ? DbgPrint("\n\nSUCCESSFULLY ALLOCATED %d BYTES\n\n",(pDeviceExtension->DiskRegInfo.DiskSize/DEFAULT_BYTES_PER_SECTOR*sizeof(PVOID))) : DbgPrint("\n\n\t\tALLOCATED ERROR\n\n");

    if (pDeviceExtension->DiskImage) {
		
        UNICODE_STRING deviceName;
        UNICODE_STRING win32Name;

		VHDDFormatDisk(pDeviceExtension);

        status = STATUS_SUCCESS;

        RtlInitUnicodeString(&win32Name, DOS_DEVICE_NAME);
        RtlInitUnicodeString(&deviceName, NT_DEVICE_NAME);

        pDeviceExtension->SymbolicLink.Buffer = (PWSTR)
            &pDeviceExtension->DosDeviceNameBuffer;
        pDeviceExtension->SymbolicLink.MaximumLength =
            sizeof(pDeviceExtension->DosDeviceNameBuffer);
        pDeviceExtension->SymbolicLink.Length = win32Name.Length;

        RtlCopyUnicodeString(&pDeviceExtension->SymbolicLink, &win32Name);
        RtlAppendUnicodeStringToString(&pDeviceExtension->SymbolicLink,
                                       &pDeviceExtension->DiskRegInfo.DriveLetter);

        status = WdfDeviceCreateSymbolicLink(Device,
                                             &pDeviceExtension->SymbolicLink);
    }

	return status;
}
VOID
VHDDQueryDiskRegParameters(
    IN PWSTR RegistryPath,
    IN PDISK_INFO DiskRegInfo
    )
{
    RTL_QUERY_REGISTRY_TABLE rtlQueryRegTable[5 + 1];  // + 1 для NULL
    NTSTATUS                 Status;
    DISK_INFO                defDiskRegInfo;

    PAGED_CODE();

    ASSERT(RegistryPath != NULL);

    defDiskRegInfo.DiskSize          = DEFAULT_DISK_SIZE;
    defDiskRegInfo.RootDirEntries    = DEFAULT_ROOT_DIR_ENTRIES;
    defDiskRegInfo.SectorsPerCluster = DEFAULT_SECTORS_PER_CLUSTER;

    RtlInitUnicodeString(&defDiskRegInfo.DriveLetter, DEFAULT_DRIVE_LETTER);

    RtlZeroMemory(rtlQueryRegTable, sizeof(rtlQueryRegTable));

    rtlQueryRegTable[0].Flags         = RTL_QUERY_REGISTRY_SUBKEY;
    rtlQueryRegTable[0].Name          = L"Parameters";
    rtlQueryRegTable[0].EntryContext  = NULL;
    rtlQueryRegTable[0].DefaultType   = (ULONG_PTR)NULL;
    rtlQueryRegTable[0].DefaultData   = NULL;
    rtlQueryRegTable[0].DefaultLength = (ULONG_PTR)NULL;

    rtlQueryRegTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTable[1].Name          = L"DiskSize";
    rtlQueryRegTable[1].EntryContext  = &DiskRegInfo->DiskSize;
    rtlQueryRegTable[1].DefaultType   = REG_DWORD;
    rtlQueryRegTable[1].DefaultData   = &defDiskRegInfo.DiskSize;
    rtlQueryRegTable[1].DefaultLength = sizeof(ULONG);

    rtlQueryRegTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTable[2].Name          = L"RootDirEntries";
    rtlQueryRegTable[2].EntryContext  = &DiskRegInfo->RootDirEntries;
    rtlQueryRegTable[2].DefaultType   = REG_DWORD;
    rtlQueryRegTable[2].DefaultData   = &defDiskRegInfo.RootDirEntries;
    rtlQueryRegTable[2].DefaultLength = sizeof(ULONG);

    rtlQueryRegTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTable[3].Name          = L"SectorsPerCluster";
    rtlQueryRegTable[3].EntryContext  = &DiskRegInfo->SectorsPerCluster;
    rtlQueryRegTable[3].DefaultType   = REG_DWORD;
    rtlQueryRegTable[3].DefaultData   = &defDiskRegInfo.SectorsPerCluster;
    rtlQueryRegTable[3].DefaultLength = sizeof(ULONG);

    rtlQueryRegTable[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTable[4].Name          = L"DriveLetter";
    rtlQueryRegTable[4].EntryContext  = &DiskRegInfo->DriveLetter;
    rtlQueryRegTable[4].DefaultType   = REG_SZ;
    rtlQueryRegTable[4].DefaultData   = defDiskRegInfo.DriveLetter.Buffer;
    rtlQueryRegTable[4].DefaultLength = 0;

    Status = RtlQueryRegistryValues(
                 RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                 RegistryPath,
                 rtlQueryRegTable,
                 NULL,
                 NULL
             );

    if (NT_SUCCESS(Status) == FALSE) {

        DiskRegInfo->DiskSize          = defDiskRegInfo.DiskSize;
        DiskRegInfo->RootDirEntries    = defDiskRegInfo.RootDirEntries;
        DiskRegInfo->SectorsPerCluster = defDiskRegInfo.SectorsPerCluster;
        RtlCopyUnicodeString(&DiskRegInfo->DriveLetter, &defDiskRegInfo.DriveLetter);
    }

    DbgPrint("DiskSize          = 0x%lx (%ld)\n", DiskRegInfo->DiskSize, DiskRegInfo->DiskSize);
    DbgPrint("RootDirEntries    = 0x%lx (%ld)\n", DiskRegInfo->RootDirEntries, DiskRegInfo->RootDirEntries);
    DbgPrint("SectorsPerCluster = 0x%lx (%ld)\n", DiskRegInfo->SectorsPerCluster, DiskRegInfo->SectorsPerCluster);
    DbgPrint("DriveLetter       = %wZ\n",   &(DiskRegInfo->DriveLetter));

    return;
}
VOID VHDDEvtCleanupCallback(
	IN WDFOBJECT Device
	)
{
    PDEVICE_EXTENSION pDeviceExtension = DeviceGetExtension(Device);
	UINT32 temp;
    PAGED_CODE();

	DbgPrint("VHDDEvtCleanUpCallback routine");

		for(temp = 1; temp < DISK_IMAGE_SIZE; temp++){
			if(pDeviceExtension->DiskImage[temp] != NULL){
				ExFreePool(pDeviceExtension->DiskImage[temp]);	
				DbgPrint("sector #%d deallocated",temp );
			}
			else DbgPrint("sector #%d is free",temp );
		}
		ExFreePool(pDeviceExtension->DiskImage[0]);
		DbgPrint("sector #0 deallocated");
}
VOID VHDDEvtIoDeviceControl (
    IN WDFQUEUE  Queue,
    IN WDFREQUEST  Request,
    IN size_t  OutputBufferLength,
    IN size_t  InputBufferLength,
    IN ULONG  IoControlCode
    )
{
    NTSTATUS          Status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR         information = 0;
    size_t            bufSize;
    PDEVICE_EXTENSION devExt = QueueGetExtension(Queue)->DeviceExtension;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
	//обработка IOCTL запросов
	switch (IoControlCode) {
		//установить hotplug конфигурацию
	case IOCTL_STORAGE_SET_HOTPLUG_INFO:
		DbgPrint("IOCTL_STORAGE_SET_HOTPLUG_INFO");
		break;
		//получить hotplug конфигурацию
	case IOCTL_STORAGE_GET_HOTPLUG_INFO:
		DbgPrint("IOCTL_STORAGE_GET_HOTPLUG_INFO");
		break;
		//вернуть свойства запоминающего устройства
	case IOCTL_STORAGE_QUERY_PROPERTY:
		DbgPrint("IOCTL_STORAGE_QUERY_PROPERTY");
		break;
		//сообщить о типе, размере и природе раздела диска.
	case IOCTL_DISK_GET_PARTITION_INFO:{

            PPARTITION_INFORMATION outputBuffer;
            PBOOT_SECTOR bootSector = (PBOOT_SECTOR)(devExt->DiskImage[0]);

            information = sizeof(PARTITION_INFORMATION);
			DbgPrint("IOCTL_DISK_GET_PARTITION_INFO");
            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(PARTITION_INFORMATION), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) ) {

                outputBuffer->PartitionType = PARTITION_FAT_16;
                outputBuffer->BootIndicator       = FALSE;
                outputBuffer->RecognizedPartition = TRUE;
                outputBuffer->RewritePartition    = FALSE;
                outputBuffer->StartingOffset.QuadPart = 0;
                outputBuffer->PartitionLength.QuadPart = devExt->DiskRegInfo.DiskSize;
                outputBuffer->HiddenSectors       = (ULONG) (1L);
                outputBuffer->PartitionNumber     = (ULONG) (-1L);

                Status = STATUS_SUCCESS;
            }
        }
        break;
		//изменить тип раздела
	case IOCTL_DISK_GET_PARTITION_INFO_EX:
		DbgPrint("IOCTL_DISK_GET_PARTITION_INFO_EX");
		break;
	case IOCTL_DISK_SET_PARTITION_INFO:
		DbgPrint("IOCTL_DISK_SET_PARTITION_INFO");
		break;
		//форматировать дорожки
	case IOCTL_DISK_FORMAT_TRACKS:
		DbgPrint("IOCTL_DISK_FORMAT_TRACKS");
		Status = STATUS_SUCCESS;
		break;
		//блокировать извлечение носителя
	case IOCTL_DISK_MEDIA_REMOVAL:
		DbgPrint("IOCTL_DISK_MEDIA_REMOVAL");
		Status = STATUS_SUCCESS;
		break;
	case IOCTL_MOUNTMGR_QUERY_POINTS: // сообщить о символической ссылке для тома 
		DbgPrint("IOCTL_MOUNTMGR_QUERY_POINTS"); 
		break; 
		//получить размер диска
	case IOCTL_DISK_GET_LENGTH_INFO:
		DbgPrint("IOCTL_DISK_GET_LENGTH_INFO");
		break;
		//получить ссылочное имя
	case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
		DbgPrint("IOCTL_MOUNTDEV_QUERY_DEVICE_NAME");
		break;
		//вернуть физическое месторасположение одного или более дисков
	case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS:
		DbgPrint("IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS");
		break;
		//сообщить о геометрии диска (количество цилиндров, дорожек, секторов)
	case IOCTL_DISK_GET_MEDIA_TYPES:
    case IOCTL_DISK_GET_DRIVE_GEOMETRY:  {
            PDISK_GEOMETRY outputBuffer;
            information = sizeof(DISK_GEOMETRY);
			DbgPrint("IOCTL_DISK_GET_DRIVE_GEOMETRY / IOCTL_DISK_GET_MEDIA_TYPES");
            Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(DISK_GEOMETRY), &outputBuffer, &bufSize);
            if(NT_SUCCESS(Status) ) {

                RtlCopyMemory(outputBuffer, &(devExt->DiskGeometry), sizeof(DISK_GEOMETRY));
                Status = STATUS_SUCCESS;
            }
        }
        break;
		//проверить, сменился ли носитель (для съемных дисков)
    case IOCTL_DISK_CHECK_VERIFY:
		//проверка можно ли на диск записывать данные
    case IOCTL_DISK_IS_WRITABLE:
			DbgPrint("IOCTL_DISK_IS_WRITABLE / IOCTL_DISK_CHECK_VERIFY");
			Status = STATUS_SUCCESS;
			break;
	case 0x2d5190: 
			DbgPrint("UNKNOWN IOCTL REQUEST");
			Status = STATUS_SUCCESS;
			break;
	default: DbgPrint("VHDDEvtIoDeviceControl - %x - %d", IoControlCode, IoControlCode);
    }

	
	CheckStatus(Status);
    WdfRequestCompleteWithInformation(Request, Status, information);
}
VOID VHDDEvtIoRead(
	IN WDFQUEUE  Queue,
	IN WDFREQUEST  Request,
	IN size_t  Length
	)
{
	PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
	UINT32				   Offset;
    WDFMEMORY              hMemory;

												DbgPrint("VHDDEvtIoRead routine");
    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Read.DeviceOffset/DEFAULT_BYTES_PER_SECTOR;
												DbgPrint("\tREAD OFFSET : \t %d", Parameters.Parameters.Read.DeviceOffset/DEFAULT_BYTES_PER_SECTOR);

        Status = WdfRequestRetrieveOutputMemory(Request, &hMemory);
		if((PVOID)(devExt->DiskImage[ByteOffset.LowPart]) != NULL){
			if(NT_SUCCESS(Status) && ByteOffset.LowPart < DISK_IMAGE_SIZE){
					for(Offset = 0; Offset < Length / DEFAULT_BYTES_PER_SECTOR; Offset++, ByteOffset.LowPart++){
												DbgPrint("===============READ=======================");
												DbgPrint("Length / DEFAULT_BYTES_PER_SECTOR = %d\nByteOffset.LowPart = %d\nOffset = %d", Length / DEFAULT_BYTES_PER_SECTOR, ByteOffset.LowPart, Offset);
												DbgPrint("\t\t&(devExt->DiskImage[%d])\t0x%x", ByteOffset.LowPart, &(devExt->DiskImage[ByteOffset.LowPart]));
												DbgPrint("\t\t\tdevExt->DiskImage[%d]\t0x%x", ByteOffset.LowPart,devExt->DiskImage[ByteOffset.LowPart]);
												DbgPrint("\t\t\tLENGTH: %d", Length);


								
						Status = WdfMemoryCopyFromBuffer(hMemory,   // Destination
														 Offset*DEFAULT_BYTES_PER_SECTOR,         // Offset into the destination
														 (PVOID)(devExt->DiskImage[ByteOffset.LowPart]), // source
														 DEFAULT_BYTES_PER_SECTOR);
					}
				}
		}
		CheckStatus(Status);
    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}
VOID VHDDEvtIoWrite(
	IN WDFQUEUE  Queue,
	IN WDFREQUEST  Request,
	IN size_t  Length
	)
{
	PDEVICE_EXTENSION      devExt = QueueGetExtension(Queue)->DeviceExtension;
    NTSTATUS               Status = STATUS_INVALID_PARAMETER;
    WDF_REQUEST_PARAMETERS Parameters;
    LARGE_INTEGER          ByteOffset;
	UINT32				   Offset;
    WDFMEMORY              hMemory;

										DbgPrint("VHDDEvtIoWrite routine");

    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Write.DeviceOffset/DEFAULT_BYTES_PER_SECTOR;
										DbgPrint("\tWRITE OFFSET : \t %d", Parameters.Parameters.Write.DeviceOffset/DEFAULT_BYTES_PER_SECTOR);

        Status = WdfRequestRetrieveInputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status) && ByteOffset.LowPart < DISK_IMAGE_SIZE){
										DbgPrint("\t\t\tLENGTH: %d", Length);

			for(Offset = 0; Offset < Length / DEFAULT_BYTES_PER_SECTOR; Offset++, ByteOffset.LowPart++){
						
										DbgPrint("===============WRITE=======================");
										DbgPrint("Length / DEFAULT_BYTES_PER_SECTOR = %d\nByteOffset.LowPart = %d\nOffset = %d", Length / DEFAULT_BYTES_PER_SECTOR, ByteOffset.LowPart, Offset);
				if(devExt->DiskImage[ByteOffset.LowPart]){
					ExFreePool(devExt->DiskImage[ByteOffset.LowPart]); 
					devExt->DiskImage[ByteOffset.LowPart] = 0;
										DbgPrint("\t\tdeallocated : \n\t\t&(devExt->DiskImage[%d])\t0x%x", ByteOffset.LowPart, &(devExt->DiskImage[ByteOffset.LowPart]));
										DbgPrint("\t\tdevExt->DiskImage[%d]\t0x%x", ByteOffset.LowPart,devExt->DiskImage[ByteOffset.LowPart]);
				}
				if(devExt->DiskImage[ByteOffset.LowPart] == NULL){
					devExt->DiskImage[ByteOffset.LowPart] = ExAllocatePool( NonPagedPool, DEFAULT_BYTES_PER_SECTOR);
					RtlZeroMemory(devExt->DiskImage[ByteOffset.LowPart], DEFAULT_BYTES_PER_SECTOR);
										devExt->DiskImage[ByteOffset.LowPart] != NULL ? DbgPrint("\t\t\tsector #%d allocated:",ByteOffset.LowPart ) : DbgPrint("sector #%d didn't allocated",ByteOffset.LowPart);	
										DbgPrint("\t\t\t&(devExt->DiskImage[%d])\t0x%x", ByteOffset.LowPart, &(devExt->DiskImage[ByteOffset.LowPart]));
										DbgPrint("\t\t\t\tdevExt->DiskImage[%d]\t0x%x", ByteOffset.LowPart,devExt->DiskImage[ByteOffset.LowPart]);
				}
				if((PVOID)(devExt->DiskImage[ByteOffset.LowPart]) != NULL){
					Status = WdfMemoryCopyToBuffer(hMemory, // Source
										Offset*DEFAULT_BYTES_PER_SECTOR,              // offset in Source memory where the copy has to start
										(PVOID)(devExt->DiskImage[ByteOffset.LowPart]), // destination
										DEFAULT_BYTES_PER_SECTOR);
				}
			}
        }
		CheckStatus(Status);
    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}
VOID CheckStatus(NTSTATUS Status){
		switch(Status){
			case STATUS_SUCCESS:
				DbgPrint("STATUS_SUCCESS");
				break;
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
			case STATUS_INVALID_DEVICE_REQUEST:
				DbgPrint("STATUS_INVALID_DEVICE_REQUEST");
				break;
			default: DbgPrint("STATUS - %x -%d", Status, Status);
		}
}
NTSTATUS
VHDDFormatDisk(
    IN PDEVICE_EXTENSION devExt
    )
{
    PBOOT_SECTOR bootSector;
    PUCHAR       firstFatSector;
    ULONG        rootDirEntries;
    ULONG        sectorsPerCluster;
    USHORT       fatEntries;     
    USHORT       fatSectorCnt;   
    PDIR_ENTRY   rootDir;        
	UINT		 temp;
    PAGED_CODE();

	devExt->DiskImage[0] = (ExAllocatePool( NonPagedPool, DEFAULT_BYTES_PER_SECTOR));

										DbgPrint("\tdevExt->DiskImage\t0x%x", devExt->DiskImage);
										DbgPrint("\t &(devExt->DiskImage[0])\t0x%x", &(devExt->DiskImage[0]));
										DbgPrint("\tdevExt->DiskImage[0]\t0x%x", devExt->DiskImage[0]);
										devExt->DiskImage[0] != NULL ? DbgPrint("boot sector allocated") : DbgPrint("boot sector didn't allocated");

	for(temp = 1; temp < 10; temp++){
	    devExt->DiskImage[temp] = ExAllocatePool( NonPagedPool, DEFAULT_BYTES_PER_SECTOR);
		RtlZeroMemory(devExt->DiskImage[temp], DEFAULT_BYTES_PER_SECTOR);
		devExt->DiskImage[temp] != NULL ? DbgPrint("sector #%d allocated",temp ) : DbgPrint("sector #%d didn't allocated",temp);	
	}
	for(temp = 10; temp < DISK_IMAGE_SIZE; temp++){
		devExt->DiskImage[temp] = NULL;
	}
	bootSector  = (PBOOT_SECTOR)(devExt->DiskImage[0]);

    devExt->DiskGeometry.BytesPerSector = DEFAULT_BYTES_PER_SECTOR;
    devExt->DiskGeometry.SectorsPerTrack = 32;     // Using VHDD value
    devExt->DiskGeometry.TracksPerCylinder = 2;    // Using VHDD value

    devExt->DiskGeometry.Cylinders.QuadPart = devExt->DiskRegInfo.DiskSize / DEFAULT_BYTES_PER_SECTOR / 32 / 2;

    devExt->DiskGeometry.MediaType = VHDD_MEDIA_TYPE;
										DbgPrint("Cylinders: %ld", devExt->DiskGeometry.Cylinders.QuadPart);
										DbgPrint("TracksPerCylinder: %ld", devExt->DiskGeometry.TracksPerCylinder);
										DbgPrint("SectorsPerTrack: %ld", devExt->DiskGeometry.SectorsPerTrack);
										DbgPrint("BytesPerSector: %ld", devExt->DiskGeometry.BytesPerSector);

    rootDirEntries = devExt->DiskRegInfo.RootDirEntries;
    sectorsPerCluster = devExt->DiskRegInfo.SectorsPerCluster;


    if (rootDirEntries & (DIR_ENTRIES_PER_SECTOR - 1)) {

        rootDirEntries = (rootDirEntries + (DIR_ENTRIES_PER_SECTOR - 1)) & ~ (DIR_ENTRIES_PER_SECTOR - 1);
    }

										DbgPrint("Root dir entries: %ld\nSectors/cluster: %ld\n",rootDirEntries, sectorsPerCluster);

    bootSector->bsJump[0] = 0xeb;
    bootSector->bsJump[1] = 0x3c;
    bootSector->bsJump[2] = 0x90;

    bootSector->bsOemName[0] = 'P';
    bootSector->bsOemName[1] = 'i';
    bootSector->bsOemName[2] = 'l';
    bootSector->bsOemName[3] = 'i';
    bootSector->bsOemName[4] = 't';
    bootSector->bsOemName[5] = 's';
    bootSector->bsOemName[6] = 'i';
    bootSector->bsOemName[7] = 'n';

    bootSector->bsBytesPerSec = (SHORT)devExt->DiskGeometry.BytesPerSector;
    bootSector->bsResSectors  = 1;
    bootSector->bsFATs        = 1;
    bootSector->bsRootDirEnts = (USHORT)rootDirEntries;

    bootSector->bsSectors     = (USHORT)(devExt->DiskRegInfo.DiskSize /
                                         devExt->DiskGeometry.BytesPerSector);
    bootSector->bsMedia       = (UCHAR)devExt->DiskGeometry.MediaType;
    bootSector->bsSecPerClus  = (UCHAR)sectorsPerCluster;

    fatEntries = (bootSector->bsSectors - bootSector->bsResSectors -
				bootSector->bsRootDirEnts / DIR_ENTRIES_PER_SECTOR) /
                bootSector->bsSecPerClus + 2;


        fatSectorCnt = (fatEntries * 2 + DEFAULT_BYTES_PER_SECTOR - 1) / DEFAULT_BYTES_PER_SECTOR;
        fatEntries   = fatEntries + fatSectorCnt;
        fatSectorCnt = (fatEntries * 2 + DEFAULT_BYTES_PER_SECTOR - 1) / DEFAULT_BYTES_PER_SECTOR;
						DbgPrint("FatEntries = %d\nFatSectorCnt = %d", fatEntries, fatSectorCnt);
    bootSector->bsFATsecs       = fatSectorCnt;
    bootSector->bsSecPerTrack   = (USHORT)devExt->DiskGeometry.SectorsPerTrack;
    bootSector->bsHeads         = (USHORT)devExt->DiskGeometry.TracksPerCylinder;
    bootSector->bsBootSignature = 0x29;
    bootSector->bsVolumeID      = 0x12345678;

    bootSector->bsLabel[0]  = 'V';
    bootSector->bsLabel[1]  = 'i';
    bootSector->bsLabel[2]  = 'r';
    bootSector->bsLabel[3]  = 't';
    bootSector->bsLabel[4]  = 'u';
    bootSector->bsLabel[5]  = 'a';
    bootSector->bsLabel[6]  = 'l';
    bootSector->bsLabel[7]  = ' ';
    bootSector->bsLabel[8]  = 'H';
    bootSector->bsLabel[9]  = 'D';
    bootSector->bsLabel[10] = 'D';

    bootSector->bsFileSystemType[0] = 'F';
    bootSector->bsFileSystemType[1] = 'A';
    bootSector->bsFileSystemType[2] = 'T';
    bootSector->bsFileSystemType[3] = '1';
    bootSector->bsFileSystemType[4] = '6';
    bootSector->bsFileSystemType[5] = ' ';
    bootSector->bsFileSystemType[6] = ' ';
    bootSector->bsFileSystemType[7] = ' ';
	

    bootSector->bsSig2[0] = 0x55;
    bootSector->bsSig2[1] = 0xAA;

    firstFatSector    = (PUCHAR)(devExt->DiskImage[1]);
    firstFatSector[0] = (UCHAR)devExt->DiskGeometry.MediaType;
    firstFatSector[1] = 0xFF;
    firstFatSector[2] = 0xFF;


    firstFatSector[3] = 0xFF;

    rootDir = (PDIR_ENTRY)(devExt->DiskImage[1 + fatSectorCnt]);

    rootDir->deName[0] = 'V';
    rootDir->deName[1] = 'I';
    rootDir->deName[2] = 'R';
    rootDir->deName[3] = 'T';
    rootDir->deName[4] = 'U';
    rootDir->deName[5] = 'A';
    rootDir->deName[6] = 'L';
    rootDir->deName[7] = ' ';

    rootDir->deExtension[0] = 'H';
    rootDir->deExtension[1] = 'D';
    rootDir->deExtension[2] = 'D';

    rootDir->deAttributes = DIR_ATTR_VOLUME;
											DbgPrint("\n\n\t\t\t%s\n\n", ((PBOOT_SECTOR)(PVOID)(devExt->DiskImage[0]))->bsOemName);
											DbgPrint("FILE SYSTEM : FAT16");
    return STATUS_SUCCESS;
}