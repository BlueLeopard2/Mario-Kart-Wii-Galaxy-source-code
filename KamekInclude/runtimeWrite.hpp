#ifndef _KAMEK_RUNTIME_WRITE_
#define _KAMEK_RUNTIME_WRITE_
#include <types.hpp>

namespace KamekRuntimeWrite {
void Write32(u32 address, u32 value);
void Write16(u32 address, u16 value);
void Write8(u32 address, u8 value);
bool CondWrite32(u32 address, u32 expectedOriginal, u32 value);
bool CondWrite16(u32 address, u16 expectedOriginal, u16 value);
bool CondWrite8(u32 address, u8 expectedOriginal, u8 value);
bool Branch(u32 fromAddress, u32 toAddress, bool link);
}  // namespace KamekRuntimeWrite

#define kmRuntimeUse(addrHex) extern "C" char __kAutoMap_##addrHex
#define kmRuntimeAddr(addrHex) ((u32) & __kAutoMap_##addrHex)

#define kmRuntimeWrite32A(addrHex, value) KamekRuntimeWrite::Write32(kmRuntimeAddr(addrHex), (value))
#define kmRuntimeWrite16A(addrHex, value) KamekRuntimeWrite::Write16(kmRuntimeAddr(addrHex), (value))
#define kmRuntimeWrite8A(addrHex, value) KamekRuntimeWrite::Write8(kmRuntimeAddr(addrHex), (value))

#define kmRuntimeCondWrite32A(addrHex, orig, value) KamekRuntimeWrite::CondWrite32(kmRuntimeAddr(addrHex), (orig), (value))
#define kmRuntimeCondWrite16A(addrHex, orig, value) KamekRuntimeWrite::CondWrite16(kmRuntimeAddr(addrHex), (orig), (value))
#define kmRuntimeCondWrite8A(addrHex, orig, value) KamekRuntimeWrite::CondWrite8(kmRuntimeAddr(addrHex), (orig), (value))

#define kmRuntimeBranchA(addrHex, dest) KamekRuntimeWrite::Branch(kmRuntimeAddr(addrHex), (u32)(dest), false)
#define kmRuntimeCallA(addrHex, dest) KamekRuntimeWrite::Branch(kmRuntimeAddr(addrHex), (u32)(dest), true)

#endif
