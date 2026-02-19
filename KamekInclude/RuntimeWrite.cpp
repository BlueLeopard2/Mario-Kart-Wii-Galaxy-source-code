#include <runtimeWrite.hpp>
#include <core/rvl/OS/OSCache.hpp>

namespace KamekRuntimeWrite {
static inline void FlushAddress(void* addr) {
    OS::DCFlushRange(addr, 4);
    register u32 a = (u32)addr;
    asm("dcbst 0,%0" : : "r"(a));
    asm("sync");
    asm("icbi 0,%0" : : "r"(a));
    asm("isync");
}
void Write32(u32 address, u32 value) {
    *(volatile u32*)address = value;
    FlushAddress((void*)address);
}
void Write16(u32 address, u16 value) {
    *(volatile u16*)address = value;
    FlushAddress((void*)(address & ~3));
}
void Write8(u32 address, u8 value) {
    *(volatile u8*)address = value;
    FlushAddress((void*)(address & ~3));
}
bool CondWrite32(u32 address, u32 expectedOriginal, u32 value) {
    if (*(volatile u32*)address != expectedOriginal) return false;
    Write32(address, value);
    return true;
}
bool CondWrite16(u32 address, u16 expectedOriginal, u16 value) {
    if (*(volatile u16*)address != expectedOriginal) return false;
    Write16(address, value);
    return true;
}
bool CondWrite8(u32 address, u8 expectedOriginal, u8 value) {
    if (*(volatile u8*)address != expectedOriginal) return false;
    Write8(address, value);
    return true;
}
bool Branch(u32 fromAddress, u32 toAddress, bool link) {
    s32 delta = (s32)toAddress - (s32)fromAddress;
    if ((delta & 0x3) != 0) return false;
    if (delta < -0x02000000 || delta > 0x01FFFFFC) return false;
    u32 inst = 0x48000000 | ((u32)delta & 0x03FFFFFC);
    if (link) inst |= 1;
    Write32(fromAddress, inst);
    return true;
}
}  // namespace KamekRuntimeWrite
