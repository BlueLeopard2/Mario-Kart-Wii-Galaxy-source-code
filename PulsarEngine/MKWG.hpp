#include <kamek.hpp>
#include <PulsarSystem.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/GlobalFunctions.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>

//All Code Credits go to the WTP Team unless otherwise mentioned.

extern u32 FPSPatchHook;
extern u32 Blurry;
extern u32 RemoveBloom;
extern u32 RemoveBackgroundBlur;
extern u32 UltraUncutHook;
extern u32 ItemBoxHook;
extern u8 REGION;
extern u32 TTS_CHECK;
extern u8 U8_WWS_CHECK;
extern u32 U32_RBBG_HOOK_PT1;
extern u32 U32_RBBG_HOOK_PT2;
extern u16 U32_ACCURATE_ITEMS;
extern u8 U8_30FPS;
extern u16 U32_COLORS;
extern u8 U8_MENU;
extern u8 U8_BRSTMS;
extern u16 U16_MISSION;
extern u16 U16_CHARACTER;
extern u16 U16_CONTROLLER;
extern u32 U32_RUBBERBANDING;
extern u16 U16_FIRSTPERSON;
extern u16 U16_FINISH_TIMES;
extern u16 U16_MINIMAPS;
extern u16 U16_CPUS;
extern u16 U16_PERFORMANCE;
extern u16 U16_PREDICTION;
extern u16 U16_NEWITEMS;
extern u16 U16_RACECOUNTMOD;
extern u16 U16_FCOFFLINE;
extern u16 U16_FCONLINE;
extern u16 U16_VOLUME;
extern u16 U16_GAMEPLAYA;
extern u16 U16_GAMEPLAYC;
extern u16 U16_GLIDERSPEED;
extern u16 U16_GLIDERSTEERING;
extern u16 U16_GLIDERMUSHROOM;
extern u16 U16_GLIDERSTAR;
extern u16 U16_GLIDERMEGA;
extern u16 U16_GLIDERSHOCKED;
extern u16 U16_RAILRIDE;
extern u16 U16_SLOWDETACH;
extern u16 U16_FASTDETACH;
extern u16 U16_TCACTIVATION;
extern u8 U8_RED1;
extern u8 U8_GREEN1;
extern u8 U8_BLUE1;
extern u8 U8_ALPHA1;
extern u8 U8_RED2;
extern u8 U8_GREEN2;
extern u8 U8_BLUE2;
extern u8 U8_ALPHA2;
extern u8 U8_RED3;
extern u8 U8_GREEN3;
extern u8 U8_BLUE3;
extern u8 U8_ALPHA3;
extern u8 U8_RED4;
extern u8 U8_GREEN4;
extern u8 U8_BLUE4;
extern u8 U8_ALPHA4;
extern u8 U8_RED5;
extern u8 U8_GREEN5;
extern u8 U8_BLUE5;
extern u8 U8_ALPHA5;
extern u8 U8_RED6;
extern u8 U8_GREEN6;
extern u8 U8_BLUE6;
extern u8 U8_ALPHA6;


namespace MKWG {

extern bool prevMushroom;
extern bool prevSlipstream;
extern bool prevStar;
extern bool prevMega;
extern bool prevStopped;
extern bool prevShocked;
extern bool prevGlider;
extern bool mushroom;
extern bool star;
extern bool mega;
extern bool stopped;
extern bool shocked;

class System : public Pulsar::System {
public:
    static bool Is50cc();
    static bool Is100cc();
    static bool Is400cc();
    static bool Is99999cc();
    
    enum WeightClass{
        LIGHTWEIGHT,
        MEDIUMWEIGHT,
        HEAVYWEIGHT,
        MIIS,
        ALLWEIGHT
    };

    enum CharButtonId{
        BUTTON_BABY_MARIO,
        BUTTON_BABY_LUIGI,
        BUTTON_TOAD,
        BUTTON_TOADETTE,
        BUTTON_BABY_PEACH,
        BUTTON_KOOPA_TROOPA,
        BUTTON_BABY_DAISY,
        BUTTON_DRY_BONES,
        BUTTON_MARIO,
        BUTTON_LUIGI,
        BUTTON_YOSHI,
        BUTTON_BIRDO,
        BUTTON_PEACH,
        BUTTON_DIDDY_KONG,
        BUTTON_DAISY,
        BUTTON_BOWSER_JR,
        BUTTON_WARIO,
        BUTTON_WALUIGI,
        BUTTON_KING_BOO,
        BUTTON_ROSALINA,
        BUTTON_DONKEY_KONG,
        BUTTON_FUNKY_KONG,
        BUTTON_BOWSER,
        BUTTON_DRY_BOWSER,
        BUTTON_MII_A,
        BUTTON_MII_B
    };

    WeightClass weight;
    static Pulsar::System *Create(); //My Create function, needs to return Pulsar
    static WeightClass GetWeightClass(CharacterId);
    };
}





