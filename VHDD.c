#include "VHDD.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, VHDDEvtDriverDeviceAdd)
#pragma alloc_text(PAGE, VHDDEvtCleanupCallback)
#pragma alloc_text(PAGE, VHDDQueryDiskRegParameters)
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
	WDFDEVICE				Device;
	WDF_OBJECT_ATTRIBUTES	DeviceAttributes;
	NTSTATUS				status;
	PDEVICE_EXTENSION		pDeviceExtension;
	WDF_OBJECT_ATTRIBUTES	QueueAttributes;
	WDF_IO_QUEUE_CONFIG		IOQueueConfig;
	WDFQUEUE				Queue;
	PQUEUE_EXTENSION		pQueueExtension;
	GUID					diskGuid;
	DECLARE_CONST_UNICODE_STRING(DevName, NT_DEVICE_NAME);
	DECLARE_CONST_UNICODE_STRING(SDLLstr, L"SDDL_DEVOBJ_SYS_ALL"); //_ADM_RWX_WORLD_RW_RES_R
	WDFMEMORY mem = NULL;	
	PAGED_CODE();
    UNREFERENCED_PARAMETER(Driver);

	DbgPrint("VHDDEvtDriverDeviceAdd routine");
	//4d36e967-e325-11ce-bfc1-08002be10318
	diskGuid.Data1 = 1295444327; //in hex - 4d36e967
	diskGuid.Data2 = 58149; //in hex - e325
	diskGuid.Data3 = 4558; //in hex - 11ce
	diskGuid.Data4[0] = 191; //in hex - bfc1
	diskGuid.Data4[1] = 193;
	//191   193
	diskGuid.Data4[2] = 8;
	diskGuid.Data4[3] = 0;
	diskGuid.Data4[4] = 43;
	diskGuid.Data4[5] = 225;
	diskGuid.Data4[6] = 3;
	diskGuid.Data4[7] = 24;
	DbgPrint("%x-%x-%x-%x%x-%2x%2x%2x%2x%2x%2x",diskGuid.Data1, diskGuid.Data2, diskGuid.Data3,
					diskGuid.Data4[0],diskGuid.Data4[1], diskGuid.Data4[2], diskGuid.Data4[3], diskGuid.Data4[4],
					diskGuid.Data4[5],diskGuid.Data4[6], diskGuid.Data4[7]);
	DbgPrint("4d36e967-e325-11ce-bfc1-08002be10318");
	//DeviceInit = WdfControlDeviceInitAllocate(Driver, &SDLLstr);
	if(!NT_SUCCESS(status = WdfDeviceInitAssignName(DeviceInit, &DevName)))
	{	
		DbgPrint("return after WdfDeviceInitAssignName");
		return status;
	}
	WdfDeviceInitSetDeviceClass(DeviceInit, &diskGuid);
	WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_DISK);
	WdfDeviceInitSetExclusive(DeviceInit, FALSE);
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&DeviceAttributes, DEVICE_EXTENSION);
	DeviceAttributes.EvtCleanupCallback = VHDDEvtCleanupCallback;

	if(!NT_SUCCESS(status = WdfDeviceCreate(&DeviceInit, &DeviceAttributes, &Device)))
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

    pDeviceExtension->DiskRegInfo.DriveLetter.Buffer =
        (PWSTR) &pDeviceExtension->DriveLetterBuffer;
    pDeviceExtension->DiskRegInfo.DriveLetter.MaximumLength =
        sizeof(pDeviceExtension->DriveLetterBuffer);
    VHDDQueryDiskRegParameters(WdfDriverGetRegistryPath(WdfDeviceGetDriver(Device)),&pDeviceExtension->DiskRegInfo);

    pDeviceExtension->DiskImage = ExAllocatePoolWithTag(
        NonPagedPool,
        pDeviceExtension->DiskRegInfo.DiskSize,
        VHDD_TAG
        );
	
    if (pDeviceExtension->DiskImage) {
		
        UNICODE_STRING deviceName;
        UNICODE_STRING win32Name;

		RtlZeroMemory(pDeviceExtension->DiskImage, pDeviceExtension->DiskRegInfo.DiskSize);

	pDeviceExtension->DiskGeometry.BytesPerSector = 512;
    pDeviceExtension->DiskGeometry.SectorsPerTrack = 32;
    pDeviceExtension->DiskGeometry.TracksPerCylinder = 2;   
    pDeviceExtension->DiskGeometry.Cylinders.QuadPart = pDeviceExtension->DiskRegInfo.DiskSize / 512 / 32 / 2;
	pDeviceExtension->DiskGeometry.MediaType = VHDD_MEDIA_TYPE;

	pDeviceExtension->Partition.StartingOffset.QuadPart = 0;
	pDeviceExtension->Partition.PartitionLength.QuadPart = DEFAULT_DISK_SIZE;
	pDeviceExtension->Partition.HiddenSectors = 0;
	pDeviceExtension->Partition.PartitionNumber = 0;
	pDeviceExtension->Partition.PartitionType = PARTITION_ENTRY_UNUSED;
	pDeviceExtension->Partition.BootIndicator = FALSE;
	pDeviceExtension->Partition.RecognizedPartition = FALSE;
	pDeviceExtension->Partition.RewritePartition = FALSE;

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

	WdfControlFinishInitializing(Device);

	return status;
}
VOID
VHDDQueryDiskRegParameters(
    IN PWSTR RegistryPath,
    IN PDISK_INFO DiskRegInfo
    )

{

    RTL_QUERY_REGISTRY_TABLE rtlQueryRegTable[5 + 1];  // Need 1 for NULL
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

    DbgPrint("DiskSize          = 0x%lx\n", DiskRegInfo->DiskSize);
    DbgPrint("RootDirEntries    = 0x%lx\n", DiskRegInfo->RootDirEntries);
    DbgPrint("SectorsPerCluster = 0x%lx\n", DiskRegInfo->SectorsPerCluster);
    DbgPrint("DriveLetter       = %wZ\n",   &(DiskRegInfo->DriveLetter));

    return;
}
VOID VHDDEvtCleanupCallback(
	IN WDFOBJECT Device
	)
{
    PDEVICE_EXTENSION pDeviceExtension = DeviceGetExtension(Device);

    PAGED_CODE();

	DbgPrint("VHDDEvtCleanUpCallback routine");

    if(pDeviceExtension->DiskImage) {
        ExFreePool(pDeviceExtension->DiskImage);
    }
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
	
	switch (IoControlCode) {
		//this is only for debugging
		//--------------------//--------------------//--------------------//--------------------
	case IOCTL_STORAGE_BREAK_RESERVATION:
		DbgPrint("IOCTL_STORAGE_BREAK_RESERVATION");
		break;
	case IOCTL_STORAGE_CHECK_VERIFY:
		DbgPrint("IOCTL_STORAGE_CHECK_VERIFY");
		break;
	case IOCTL_STORAGE_CHECK_VERIFY2:
		DbgPrint("IOCTL_STORAGE_CHECK_VERIFY2");
		break;
	case IOCTL_STORAGE_EJECT_MEDIA:
		DbgPrint("IOCTL_STORAGE_EJECT_MEDIA");
		break;
	case IOCTL_STORAGE_EJECTION_CONTROL:
		DbgPrint("IOCTL_STORAGE_EJECTION_CONTROL");
		break;
	case IOCTL_STORAGE_FIND_NEW_DEVICES:
		DbgPrint("IOCTL_STORAGE_FIND_NEW_DEVICES");
		break;
	case IOCTL_STORAGE_GET_DEVICE_NUMBER:
		DbgPrint("IOCTL_STORAGE_GET_DEVICE_NUMBER");
		break;
	case IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER:
		DbgPrint("IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER");
		break;
	case IOCTL_STORAGE_GET_MEDIA_TYPES:
		DbgPrint("IOCTL_STORAGE_GET_MEDIA_TYPES");
		break;
	case IOCTL_STORAGE_GET_MEDIA_TYPES_EX:
		DbgPrint("IOCTL_STORAGE_GET_MEDIA_TYPES_EX");
		break;
	case IOCTL_STORAGE_LOAD_MEDIA:
		DbgPrint("IOCTL_STORAGE_LOAD_MEDIA");
		break;
	case IOCTL_STORAGE_LOAD_MEDIA2:
		DbgPrint("IOCTL_STORAGE_LOAD_MEDIA2");
		break;
	case IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES:
		DbgPrint("IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES");
		break;
	case IOCTL_STORAGE_MCN_CONTROL:
		DbgPrint("IOCTL_STORAGE_MCN_CONTROL");
		break;
	case IOCTL_STORAGE_MEDIA_REMOVAL:
		DbgPrint("IOCTL_STORAGE_MEDIA_REMOVAL");
		break;
	case IOCTL_STORAGE_PERSISTENT_RESERVE_IN:
		DbgPrint("IOCTL_STORAGE_PERSISTENT_RESERVE_IN");
		break;
	case IOCTL_STORAGE_PERSISTENT_RESERVE_OUT:
		DbgPrint("IOCTL_STORAGE_PERSISTENT_RESERVE_OUT");
		break;
	case IOCTL_STORAGE_PREDICT_FAILURE:
		DbgPrint("IOCTL_STORAGE_PREDICT_FAILURE");
		break;
	case IOCTL_STORAGE_RELEASE:
		DbgPrint("IOCTL_STORAGE_RELEASE");
		break;
	case IOCTL_STORAGE_RESERVE:
		DbgPrint("IOCTL_STORAGE_RESERVE");
		break;
	case IOCTL_STORAGE_RESET_BUS:
		DbgPrint("IOCTL_STORAGE_RESET_BUS");
		break;
	case IOCTL_STORAGE_RESET_DEVICE:
		DbgPrint("IOCTL_STORAGE_RESET_DEVICE");
		break;
	case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
		DbgPrint("IOCTL_MOUNTDEV_QUERY_DEVICE_NAME"); 
		break; 
		//--------------------//--------------------//--------------------//--------------------
	case IOCTL_STORAGE_SET_HOTPLUG_INFO:
		DbgPrint("IOCTL_STORAGE_SET_HOTPLUG_INFO");
		break;
	case IOCTL_STORAGE_GET_HOTPLUG_INFO:
		DbgPrint("IOCTL_STORAGE_GET_HOTPLUG_INFO");
		break;
	case IOCTL_STORAGE_QUERY_PROPERTY:
		DbgPrint("IOCTL_STORAGE_QUERY_PROPERTY");
		break;
		//сообщить о типе, размере и природе раздела диска.
	case IOCTL_DISK_GET_PARTITION_INFO:{
			
			PPARTITION_INFORMATION outputBuffer;
			DbgPrint("IOCTL_DISK_GET_PARTITION_INFO");
			Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(PARTITION_INFORMATION), &outputBuffer, &bufSize);
			if(NT_SUCCESS(Status) ) {
				
				RtlCopyMemory(outputBuffer, &(devExt->Partition), sizeof(PARTITION_INFORMATION));
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
		Status = STATUS_INVALID_DEVICE_REQUEST; 
		break; 
		//получить размер диска
	case IOCTL_DISK_GET_LENGTH_INFO:	{
			
			PGET_LENGTH_INFORMATION outputBuffer;
			GET_LENGTH_INFORMATION  size;
			size.Length.QuadPart = DEFAULT_DISK_SIZE;
			DbgPrint("IOCTL_DISK_GET_LENGTH_INFO");
			Status = WdfRequestRetrieveOutputBuffer(Request, sizeof(GET_LENGTH_INFORMATION), &outputBuffer, &bufSize);
			if(NT_SUCCESS(Status) ) {
				
				RtlCopyMemory(outputBuffer, &size, sizeof(GET_LENGTH_INFORMATION));
				Status = STATUS_SUCCESS;
			
			}
		}
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
    WDFMEMORY              hMemory;

    __analysis_assume(Length > 0);

	DbgPrint("VHDDEvtIoRead routine");

    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Read.DeviceOffset;

        Status = WdfRequestRetrieveOutputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status)){

            Status = WdfMemoryCopyFromBuffer(hMemory,   // Destination
                                             0,         // Offset into the destination
                                             devExt->DiskImage + ByteOffset.LowPart, // source
                                             Length);
        }
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
    WDFMEMORY              hMemory;

    __analysis_assume(Length > 0);

	DbgPrint("VHDDEvtIoWrite routine");

    WDF_REQUEST_PARAMETERS_INIT(&Parameters);
    WdfRequestGetParameters(Request, &Parameters);

    ByteOffset.QuadPart = Parameters.Parameters.Write.DeviceOffset;

        Status = WdfRequestRetrieveInputMemory(Request, &hMemory);
        if(NT_SUCCESS(Status)){

            Status = WdfMemoryCopyToBuffer(hMemory, // Source
                                    0,              // offset in Source memory where the copy has to start
                                    devExt->DiskImage + ByteOffset.LowPart, // destination
                                    Length);
        }
    WdfRequestCompleteWithInformation(Request, Status, (ULONG_PTR)Length);
}
/*VOID VHDDDriverUnload (
    IN WDFDRIVER  Driver
    )
{
	DbgPrint("VHDDDriverUnload routine");
}
*/