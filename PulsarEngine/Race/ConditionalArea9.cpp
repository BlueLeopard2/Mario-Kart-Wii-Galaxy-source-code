#include <PulsarSystem.hpp>
namespace Pulsar{
    asmFunc ConditionalAREA9_1() {
        ASM(
            mr        r31, r4;
            lwz       r11, 0x68(r31);
            blr;
        )
        }
    kmCall(0x8078709C, ConditionalAREA9_1);

    extern "C" void sInstance__8Racedata(void*);
    extern "C" void sInstance__8Raceinfo(void*);
    asmFunc ConditionalAREA9_2() {
        ASM(
        cmplwi    r25, 0x9;
        bne-      loc_0x78;
        lha       r0, 0x2A(r4);
        cmplwi    r0, 0;
        beq-      loc_0x78;
        lis       r12, sInstance__8Racedata@ha;
        lwz       r12, sInstance__8Racedata@l(r12);
        addi      r29, r11, 0xB84;
        lbzx      r29, r29, r12;
        rlwinm    r29,r29,2,14,29;
        lis       r12, sInstance__8Raceinfo@ha;
        lwz       r12, sInstance__8Raceinfo@l(r12);
        lwz       r12, 0xC(r12);
        lwzx      r12, r12, r29;
        lhz       r12, 0xA(r12);
        lbz       r0, 0x2A(r4);
        lbz       r29, 0x2B(r4);
        cmplw     r0, r29;
        bge-      loc_0x60;
        cmplw     r12, r0;
        blt-      loc_0x70;
        cmplw     r12, r29;
        bgt-      loc_0x70;
        b         loc_0x78;

        loc_0x60:
        cmplw     r12, r29;
        blt-      loc_0x78;
        cmplw     r12, r0;
        bgt-      loc_0x78;

        loc_0x70:
        li        r0, 0xFF;
        b         loc_0x7C;

        loc_0x78:
        lha       r0, 0x28(r4);

        loc_0x7C:
        blr;
        )
        }
    kmCall(0x8078702C, ConditionalAREA9_2);
}