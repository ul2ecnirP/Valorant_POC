#ifndef DRIVER_HPP
#define DRIVER_HPP
#include <Windows.h>
#include <stdio.h>
constexpr DWORD init_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x975, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for setting g_target_process by target process id
constexpr DWORD read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x976, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for reading memory
constexpr DWORD write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x977, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //custom io control code for writing memory
constexpr ULONG getbaseaddr = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x978, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the process addrr
constexpr ULONG getguardedregion = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x979, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the guarded region
constexpr ULONG HelloDriver = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x980, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); //get the guarded region

uintptr_t guardedreg = 0;

class driver_manager {
    HANDLE m_driver_handle = nullptr; //handle to our driver

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

public:
    driver_manager(const char* driver_name, DWORD target_process_id) {
        m_driver_handle = CreateFileA(driver_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        attach_to_process(target_process_id);
    }

    void attach_to_process(DWORD process_id) {
        info_t io_info;

        io_info.target_pid = process_id;

        DeviceIoControl(m_driver_handle, init_code, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
    }

    PVOID GetProcessBase(DWORD process_id){
        info_t io_info;
        io_info.target_pid = process_id;
        DeviceIoControl(m_driver_handle, getbaseaddr, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
        return io_info.ProcessBase;
    }
    void GetGuardedRegion(){
        info_t io_info;
        DeviceIoControl(m_driver_handle, getguardedregion, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
        guardedreg = io_info.Guarded_region;
    }
    ULONG MochiMochiDriver(){
        info_t io_info;
        DeviceIoControl(m_driver_handle, HelloDriver, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
        return io_info.return_HelloDriver;
        
    }
    template<typename T> T RPMv(const UINT64 address) {
        info_t io_info;
        T read_data;
        io_info.target_address = address;
        io_info.buffer_address = (UINT64)&read_data;
        io_info.size = sizeof(T);
        DeviceIoControl(m_driver_handle, read_code, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
    
        try{
            return read_data;
        } catch(...){
            printf("erreur RPMv\n");
            exit(STATUS_FATAL_APP_EXIT);
        }

        
    }

    template<typename T> T RPMg(const uintptr_t address) {
        T buffer;
        buffer = RPMv<T>(address);
        uintptr_t val = guardedreg + (*(uintptr_t*)&buffer & 0xFFFFFF);
		return *(T*)&val;
    }

	inline static bool isguarded(uintptr_t pointer) noexcept
	{
		static constexpr uintptr_t filter = 0xFFFFFFF000000000;
		uintptr_t result = pointer & filter;
		return result == 0x8000000000 || result == 0x10000000000;
	}

    template<typename T> T RPM(const uintptr_t address) {
        T buffer;
        return buffer = RPMv<T>(address);
    }

    uintptr_t RPMPTR(const uintptr_t address) {
        uintptr_t buffer;
        buffer = RPMv< uintptr_t >(address);
        if(isguarded(buffer)){
            return RPMg<uintptr_t>(address);
        }
        return buffer;
    }  
    template<typename T> bool WPM(const UINT64 address, const T buffer) {//Never used, idk if this work
        info_t io_info;

        io_info.target_address = address;
        io_info.buffer_address = (UINT64)&buffer;
        io_info.size = sizeof(T);

        DeviceIoControl(m_driver_handle, write_code, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
        return io_info.return_size == sizeof(T);
    }




};

#endif