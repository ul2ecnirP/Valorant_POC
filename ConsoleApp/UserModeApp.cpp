
#include "kernel/driver.hpp"
#include "uworld/uworld.h"
#include "readfunc.hpp"
#include "options.hpp" 
#include "playerloop.hpp"
#include "global.hpp"
#include "ESP/ESP.hpp"//REDACTED
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <thread>



int main() {
	int buffer[1] = {0};
	if(pid == -1){
		printf("Wrong pid.");
		exit(EXIT_FAILURE);
		return -1;
	}
	if(driver->MochiMochiDriver() !=0x43BA7F6E){
		printf("Kernel Error: redemarer l'ordinateur peut aider au probleme");
		exit(0);
	}
	printf("PID > %d\n", pid); 
	PVOID baseaddrr;
	uintptr_t world_ptr;
	uintptr_t uworld;
	driver->GetGuardedRegion();
	baseaddrr = driver->GetProcessBase(pid);
	world_ptr = ReadWorldPtr((uintptr_t)baseaddrr);
	uworld = getuworld(guardedreg, world_ptr);
	uworld_global = uworld;
	printf("UWORLD > %X | %X\n", uworld, uworld_global);
	std::thread(playerloop).detach();
	DoEsp();
	

}

//read = RPMPTR
//readv = RPM
