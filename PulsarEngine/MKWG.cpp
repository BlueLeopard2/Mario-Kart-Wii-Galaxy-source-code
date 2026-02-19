#include <kamek.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <SlotExpansion/CupsConfig.hpp>
#include <Settings/UI/SettingsPanel.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <PulsarSystem.hpp>
#include <MKWG.hpp>
#include <core/RK/RKSystem.hpp>
#include <core/nw4r/ut/Misc.hpp>
#include <Config.hpp>
#include <core/egg/DVD/DvdRipper.hpp>

// All Code Credits go to the WTP Team unless otherwise mentioned.

namespace MKWG {
    Pulsar::System *System::Create() {
        return new System();
    }
    Pulsar::System::Inherit CreateMKWG(System::Create);

    bool System::Is50cc() {
        const Pulsar::Settings::Mgr& settings = Pulsar::Settings::Mgr::Get();
        return Racedata::sInstance->racesScenario.settings.engineClass == CC_50 && settings.GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG3, Pulsar::SETTINGMKWG_CUSTOMCC) == Pulsar::MKWGSETTING_CUSTOMCC_50;
    }
    bool System::Is100cc() {
        const Pulsar::Settings::Mgr& settings = Pulsar::Settings::Mgr::Get();
        return Racedata::sInstance->racesScenario.settings.engineClass == CC_50 && settings.GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG3, Pulsar::SETTINGMKWG_CUSTOMCC) == Pulsar::MKWGSETTING_CUSTOMCC_100;
    }
    bool System::Is400cc() {
        const Pulsar::Settings::Mgr& settings = Pulsar::Settings::Mgr::Get();
        return Racedata::sInstance->racesScenario.settings.engineClass == CC_50 && settings.GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG3, Pulsar::SETTINGMKWG_CUSTOMCC) == Pulsar::MKWGSETTING_CUSTOMCC_400;
    }
    bool System::Is99999cc() {
        const Pulsar::Settings::Mgr& settings = Pulsar::Settings::Mgr::Get();
        return Racedata::sInstance->racesScenario.settings.engineClass == CC_50 && settings.GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG3, Pulsar::SETTINGMKWG_CUSTOMCC) == Pulsar::MKWGSETTING_CUSTOMCC_99999;
    }
      
System::WeightClass System::GetWeightClass(const CharacterId id) {
    switch (id) {
        case BABY_MARIO:
        case BABY_LUIGI:
        case BABY_PEACH:
        case BABY_DAISY:
        case TOAD:
        case TOADETTE:
        case KOOPA_TROOPA:
        case DRY_BONES:
            return LIGHTWEIGHT;
        case MARIO:
        case LUIGI:
        case PEACH:
        case DAISY:
        case YOSHI:
        case BIRDO:
        case DIDDY_KONG:
        case BOWSER_JR:
            return MEDIUMWEIGHT;
        case WARIO:
        case WALUIGI:
        case DONKEY_KONG:
        case BOWSER:
        case KING_BOO:
        case ROSALINA:
        case FUNKY_KONG:
        case DRY_BOWSER:
            return HEAVYWEIGHT;
        default:
            return MIIS;
    }
}

// Instant Voting Roulette Decide [Ro]
kmWrite32(0x80643BC4, 0x60000000);
kmWrite32(0x80643C2C, 0x60000000);

// Fix Custom Region Joining Via Friend Roster [Kazuki]
kmWrite32(0x8065a01c, 0x60000000);
kmWrite32(0x8065a020, 0x2C0000FF);
kmWrite32(0x8065a068, 0x60000000);
kmWrite32(0x8065a06c, 0x2C0000FF);

// Allow Looking Backwards During Respawn [Ro]
kmWrite32(0x805A228C, 0x60000000);

// Always Show Timer on Vote Screen [Chadderz]
kmWrite32(0x80650254, 0x60000000);

// Allow pausing before the race starts [Sponge]
kmWrite32(0x80856a28, 0x48000050);

// Show Nametags During Countdown By [Ro]
kmWrite32(0x807F13F0, 0x38600001);

//Always Show Timer After Online Race Results [Sponge]
kmWrite32(0x8064DB2C, 0x60000000);

//CPUs Get Startup Boost
kmWrite32(0x8073ef10, 0x60000000);

} // namespace MKWG

