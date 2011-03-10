// TestDriver.c
 
#include <ntddk.h>
 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
VOID UnloadRoutine(IN PDRIVER_OBJECT DriverObject);
 
#pragma code_seg("INIT") 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
  DriverObject->DriverUnload = UnloadRoutine;
 
  DbgPrint("DriverEntry routine");
 
  return STATUS_SUCCESS;
}
#pragma code_seg()

#pragma code_seg("PAGE")
VOID UnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
  DbgPrint("UnloadRoutine routine");
}
#pragma code_seg()