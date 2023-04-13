#include <ntifs.h>
#include "struct.h"
#pragma warning (disable : 4702 4996)


/*
POUR COMPRENDRE EN DETAIL LE FONCTIONNEMENT DE L'IRP

https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_irp

POUR COMPRENDRE LE FONCTIONNEMENT DE L'ioctl

https://learn.microsoft.com/en-us/windows/win32/devio/device-input-and-output-control-ioctl-



*/
extern "C" { // undocumented windows functions
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);
	__declspec(dllimport) NTSTATUS __stdcall ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS, void*, unsigned long, unsigned long*);
}

constexpr DWORD init_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x975, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for setting g_target_process by target process id
constexpr DWORD read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x976, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for reading memory
constexpr DWORD write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x977, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for writing memory
constexpr ULONG getbaseaddr = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x978, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the process addrr
constexpr ULONG getguardedregion = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x979, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the guarded region
constexpr ULONG HelloDriver = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x980, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the guarded region

PVOID BaseDLL = (PVOID)- 1;

struct info_t { //message inter user/driver
	UINT64 target_pid = 0; //process id of process we want to read from / write to
	UINT64 target_address = 0; //address in the target proces we want to read from / write to
	UINT64 buffer_address = 0; //address in our usermode process to copy to (read mode) / read from (write mode)
	UINT64 size = 0; //size of memory to copy between our usermode process and target process
	UINT64 return_size = 0; //number of bytes successfully read / written
	PVOID ProcessBase = 0; //ProcessBase
	PVOID ImageBase = 0; //ImageBase
	uintptr_t Guarded_region = 0; //ptr to guarded region: all offsets that we want
	ULONG return_HelloDriver = 0; //to check if the communication with the driver is possible
};


auto find_guarded_region() -> UINT_PTR
{
	PSYSTEM_BIGPOOL_INFORMATION pool_information = 0;

	ULONG information_length = 0;
	NTSTATUS status = ZwQuerySystemInformation(system_bigpool_information, &information_length, 0, &information_length);

	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		if (pool_information)
			ExFreePool(pool_information);

		pool_information = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPool, information_length);
		status = ZwQuerySystemInformation(system_bigpool_information, pool_information, information_length, &information_length);
	}
	UINT_PTR saved_virtual_address = 0;

	if (pool_information)
	{
		for (ULONG i = 0; i < pool_information->Count; i++)
		{
			SYSTEM_BIGPOOL_ENTRY* allocation_entry = &pool_information->AllocatedInfo[i];

			UINT_PTR virtual_address = (UINT_PTR)allocation_entry->VirtualAddress & ~1ull;

			if (allocation_entry->NonPaged && allocation_entry->SizeInBytes == 0x200000)
			{
				if (saved_virtual_address == 0 && allocation_entry->TagUlong == 'TnoC') {
					saved_virtual_address = virtual_address;
				}
			}
		}

		ExFreePool(pool_information);
	}
	return saved_virtual_address;
}

NTSTATUS ctl_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);
	
	static PEPROCESS s_target_process;

	irp->IoStatus.Information = sizeof(info_t);
	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto buffer = (info_t*)irp->AssociatedIrp.SystemBuffer;

	if (stack) { //check les erreurs
		if (buffer && sizeof(*buffer) >= sizeof(info_t)) {
			const auto ctl_code = stack->Parameters.DeviceIoControl.IoControlCode;

			if (ctl_code == init_code) 
				PsLookupProcessByProcessId(buffer->target_pid, &s_target_process);

			else if (ctl_code == read_code) 
				MmCopyVirtualMemory(s_target_process, buffer->target_address, PsGetCurrentProcess(), buffer->buffer_address, buffer->size, KernelMode, &buffer->return_size);

			else if (ctl_code == write_code) 
				MmCopyVirtualMemory(PsGetCurrentProcess(), buffer->buffer_address, s_target_process, buffer->target_address, buffer->size, KernelMode, &buffer->return_size);
			else if (ctl_code == getbaseaddr) {
				PsLookupProcessByProcessId((HANDLE)buffer->target_pid, &s_target_process);
				KeAttachProcess((PKPROCESS)s_target_process);
				buffer->ProcessBase = PsGetProcessSectionBaseAddress(s_target_process);
				KeDetachProcess();
			}
			else if (ctl_code == HelloDriver) {
				
				buffer->return_HelloDriver = 0x43BA7F6E;
			}
			else if (ctl_code == getguardedregion) {
				buffer->Guarded_region = find_guarded_region();
			}
		}
	}
	//buffer doit contenir ici le resultat de la requete
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS unsupported_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);

	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS create_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS close_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS real_main(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	UNREFERENCED_PARAMETER(registery_path);

	UNICODE_STRING dev_name, sym_link;
	PDEVICE_OBJECT dev_obj;

	RtlInitUnicodeString(&dev_name, L"\\Device\\1337Driver");
	auto status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);
	if (status != STATUS_SUCCESS) return status;

	RtlInitUnicodeString(&sym_link, L"\\DosDevices\\1337Driver");
	status = IoCreateSymbolicLink(&sym_link, &dev_name);
	if (status != STATUS_SUCCESS) return status;

	SetFlag(dev_obj->Flags, DO_BUFFERED_IO); 

	for (int t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++) 
		driver_obj->MajorFunction[t] = unsupported_io;


	driver_obj->MajorFunction[IRP_MJ_CREATE] = create_io; 
	driver_obj->MajorFunction[IRP_MJ_CLOSE] = close_io; 
	driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ctl_io; 
	driver_obj->DriverUnload = NULL; 

	ClearFlag(dev_obj->Flags, DO_DEVICE_INITIALIZING);
	return status;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	UNREFERENCED_PARAMETER(driver_obj);
	UNREFERENCED_PARAMETER(registery_path);
	DbgPrintEx(0, 0, "Hey from kernel !\n");
	UNICODE_STRING  drv_name;
	RtlInitUnicodeString(&drv_name, L"\\Driver\\1337Driver");
	IoCreateDriver(&drv_name, &real_main);
	return STATUS_SUCCESS;
}