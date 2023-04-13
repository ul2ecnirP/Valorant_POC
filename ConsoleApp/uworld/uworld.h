#pragma once

#include <stdint.h>
#include <math.h>
#include <cmath>
#include <intrin.h>
#pragma intrinsic(_rotl8, _rotl16)

#include "../preprocess.h"
#include "../offsets.h"
#include "uworld_decryption.h"//REDACTED

inline uint64_t rotl8 ( uint64_t x, int8_t r ) 
{
  return (x << r) | (x >> (8 - r));
}

inline uint64_t rotr8(uint64_t x, int8_t r)
{
  return (x >> r) | (x << (8 - r));
}

ULONG ReadWorldPtr(uintptr_t valBase)
{
    printf("valBase %p \n", valBase);
    uint64_t key = driver->RPM<uint64_t>(valBase + offsets::uworld_key);
    printf("key %p \n", key);

#pragma pack(push, 1)
    struct State
    {
        uint64_t Keys[7];
    };
#pragma pack(pop)
    const auto state = driver->RPM<State>(valBase + offsets::uworld_state);
    printf("state %p \n", state.Keys);
    ULONG f = decrypt_uworld(key, (uintptr_t*)&state);//REDACTED
    return f;
}

auto getuworld( uintptr_t pointer, ULONG uworldptr) -> uintptr_t
{
  uintptr_t uworld_addr = driver->RPM<uintptr_t>( pointer + uworldptr );

  unsigned long long uworld_offset;

  if ( uworld_addr > 0x10000000000 )
  {
    uworld_offset = uworld_addr - 0x10000000000;
  }
  else {
    uworld_offset = uworld_addr - 0x8000000000;
  }

  return pointer + uworld_offset;
}
/*
uintptr_t decryptWorld(uintptr_t base_address) {
	const auto key = read<uintptr_t>(base_address + offsets::uworld_key);
	const auto state = read<State>(base_address + offsets::uworld_state);
	const auto uworld_ptr = decrypt_uworld(key, (uintptr_t*)&state);
	return read<uintptr_t>(uworld_ptr);
}
*/
