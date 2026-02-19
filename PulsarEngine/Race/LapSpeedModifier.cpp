#include <kamek.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/3D/Model/ModelDirector.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/Kart/KartValues.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Lakitu/LakituManager.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/3D/Model/ModelDirector.hpp>
#include <Race/200ccParams.hpp>
#include <PulsarSystem.hpp>
#include <MKWG.hpp>

namespace Pulsar {
namespace Race {
//Mostly a port of MrBean's version with better hooks and arguments documentation
RaceinfoPlayer* LoadCustomLapCount(RaceinfoPlayer* player, u8 id) {
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    const u32 roomType = RKNet::Controller::sInstance->roomType;

    u8 lapCount = 3; // Fallback
    if (KMP::Manager::sInstance && KMP::Manager::sInstance->stgiSection && KMP::Manager::sInstance->stgiSection->holdersArray[0]) {
        lapCount = KMP::Manager::sInstance->stgiSection->holdersArray[0]->raw->lapCount;
    }

    // Online friend rooms — use host’s lap setting
    if (roomType == RKNet::ROOMTYPE_FROOM_HOST || roomType == RKNet::ROOMTYPE_FROOM_NONHOST) {
        if (System::sInstance->IsContext(Pulsar::PULSAR_LAPS1))       lapCount = 1;
        else if (System::sInstance->IsContext(Pulsar::PULSAR_LAPS2))  lapCount = 2;
        else if (System::sInstance->IsContext(Pulsar::PULSAR_LAPS3))  lapCount = 3;
        else if (System::sInstance->IsContext(Pulsar::PULSAR_LAPS4))  lapCount = 4;
        else if (System::sInstance->IsContext(Pulsar::PULSAR_LAPS5))  lapCount = 5;
        else /* Default */                                       lapCount = lapCount;
    }
    // Worldwide, regionals, TTs, ghosts: force track lap count
    else if (
        roomType == RKNet::ROOMTYPE_VS_WW || roomType == RKNet::ROOMTYPE_BT_WW ||
        roomType == RKNet::ROOMTYPE_VS_REGIONAL || roomType == RKNet::ROOMTYPE_BT_REGIONAL ||
        mode == MODE_TIME_TRIAL || mode == MODE_GHOST_RACE) {
        lapCount = lapCount;
    }
    // Offline: use user setting
    else {
        u32 setting = Pulsar::Settings::Mgr::Get().GetUserSettingValue(
            static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG2), 
            Pulsar::SETTINGMKWG_LAPCOUNT);
        switch (setting) {
            case Pulsar::MKWGSETTING_LAPS_1: lapCount = 1; break;
            case Pulsar::MKWGSETTING_LAPS_2: lapCount = 2; break;
            case Pulsar::MKWGSETTING_LAPS_3: lapCount = 3; break;
            case Pulsar::MKWGSETTING_LAPS_4: lapCount = 4; break;
            case Pulsar::MKWGSETTING_LAPS_5: lapCount = 5; break;
            default:                                       break; // already track default
        }
    }

    // Set the lap count
    Racedata::sInstance->racesScenario.settings.lapCount = lapCount;
    return new (player) RaceinfoPlayer(id, lapCount);
}

// Safe call — happens just before players are created, after race setup is ready
kmCall(0x805328D4, LoadCustomLapCount);


void DisplayCorrectLap(AnmTexPatHolder* texPat) {
    register u32 maxLap;
    asm(mr maxLap, r29;);
    texPat->UpdateRateAndSetFrame((float)(maxLap - 2));
    return;
}
kmCall(0x80723d70, DisplayCorrectLap);

Kart::Stats* ApplySpeedModifier(KartId kartId, CharacterId characterId) {
    union SpeedModConv {
        float speedMod;
        u32 kmpValue;
    };

    Kart::Stats* stats = Kart::ComputeStats(kartId, characterId); 
    const GameMode gameMode = Racedata::sInstance->menusScenario.settings.gamemode;
    SpeedModConv speedModConv;
    speedModConv.kmpValue = (KMP::Manager::sInstance->stgiSection->holdersArray[0]->raw->speedMod << 16);
    if(speedModConv.speedMod == 0.0f) speedModConv.speedMod = 1.0f;

    float factor = 1.0f;

    
    Item::greenShellSpeed               = 105.0f;                     
    Item::redShellInitialSpeed          = 75.0f;                  
    Item::redShellSpeed                 = 130.0f;                        
    Item::blueShellSpeed                = 260.0f;                       
    Item::blueShellMinimumDiveDistance  = 640000.0f;      
    Item::blueShellHomingSpeed          = 130.0f;                 
    Kart::hardSpeedCap                  = 120.0f;                                  
    Kart::bulletSpeed                   = 145.0f;                          
    Kart::starSpeed                     = 105.0f;                            
    Kart::megaTCSpeed                   = 95.0f;

    

    if (System::sInstance->IsContext(PULSAR_200) && Racedata::sInstance->racesScenario.settings.engineClass == CC_100) {
        if (gameMode != MODE_BATTLE && gameMode != MODE_PUBLIC_BATTLE && gameMode != MODE_PRIVATE_BATTLE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) {
            factor = 1.6667f;
        }
        else if (gameMode == MODE_BATTLE || gameMode == MODE_PUBLIC_BATTLE || gameMode == MODE_PRIVATE_BATTLE) {
            factor = 1.214f;
        }
        Item::greenShellSpeed               = 105.0f * 1.5f * 1.1111f;                     
        Item::redShellInitialSpeed          = 75.0f * 1.5f * 1.1111f;                  
        Item::redShellSpeed                 = 130.0f * 1.5f * 1.1111f;                        
        Item::blueShellSpeed                = 260.0f * 1.5f * 1.1111f;                       
        Item::blueShellMinimumDiveDistance  = 640000.0f * 1.5f * 1.1111f;      
        Item::blueShellHomingSpeed          = 130.0f * 1.4f * 1.1111f;                 
        Kart::hardSpeedCap                  = 120.0f * 1.5f;                                  
        Kart::bulletSpeed                   = 145.0f * 1.5f * 1.1111f;
        Kart::starSpeed                     = 105.0f * 1.5f * 1.1111f;                            
        Kart::megaTCSpeed                   = 95.0f * 1.5f * 1.1111f;                 
    }
    else if (MKWG::System::Is400cc() && Racedata::sInstance->racesScenario.settings.engineClass == CC_50) {
        if (gameMode != MODE_BATTLE && gameMode != MODE_PUBLIC_BATTLE && gameMode != MODE_PRIVATE_BATTLE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) {
            factor = 2.8125f;
        }
        else if (gameMode == MODE_BATTLE || gameMode == MODE_PUBLIC_BATTLE || gameMode == MODE_PRIVATE_BATTLE) {
            factor = 1.214f;
        }
        Item::greenShellSpeed               = 105.0f * 2.25f * 1.25f;                     
        Item::redShellInitialSpeed          = 75.0f * 2.25f * 1.25f;                 
        Item::redShellSpeed                 = 130.0f * 2.25f * 1.25f;                       
        Item::blueShellSpeed                = 260.0f * 2.25f * 1.25f;                      
        Item::blueShellMinimumDiveDistance  = 640000.0f * 2.25f * 1.25f;     
        Item::blueShellHomingSpeed          = 130.0f * 2.25f * 1.25f;               
        Kart::hardSpeedCap                  = 120.0f * 2.25f;                        
        Kart::bulletSpeed                   = 145.0f * 2.25f * 1.25f;
        Kart::starSpeed                     = 105.0f * 2.25f * 1.25f;                           
        Kart::megaTCSpeed                   = 95.0f * 2.25f * 1.25f;                        
    }
    else if (MKWG::System::Is99999cc() && Racedata::sInstance->racesScenario.settings.engineClass == CC_50) {
        if (gameMode != MODE_BATTLE && gameMode != MODE_PUBLIC_BATTLE && gameMode != MODE_PRIVATE_BATTLE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) {
            factor = 16.55f;
        }
        else if (gameMode == MODE_BATTLE || gameMode == MODE_PUBLIC_BATTLE || gameMode == MODE_PRIVATE_BATTLE) {
            factor = 1.214f;
        }
        Item::greenShellSpeed               = 105.0f  * 13.55f * 1.25f;
        Item::redShellInitialSpeed          = 75.0f   * 13.55f * 1.25f;
        Item::redShellSpeed                 = 130.0f  * 13.55f * 1.25f;
        Item::blueShellSpeed                = 260.0f  * 13.55f * 1.25f;
        Item::blueShellMinimumDiveDistance  = 640000.0f * 13.55f * 1.25f;
        Item::blueShellHomingSpeed          = 130.0f  * 13.55f * 1.25f;
        Kart::hardSpeedCap                  = 120.0f  * 13.55f;
        Kart::bulletSpeed                   = 145.0f  * 13.55f * 1.25f;
        Kart::starSpeed                     = 105.0f  * 13.55f * 1.25f;
        Kart::megaTCSpeed                   = 95.0f   * 13.55f * 1.25f;
    }
    else if (MKWG::System::Is100cc() && Racedata::sInstance->racesScenario.settings.engineClass == CC_50) {
        if (gameMode != MODE_BATTLE && gameMode != MODE_PUBLIC_BATTLE && gameMode != MODE_PRIVATE_BATTLE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) {
        factor = 1.125f;
    }
    else if (gameMode == MODE_BATTLE || gameMode == MODE_PUBLIC_BATTLE || gameMode == MODE_PRIVATE_BATTLE) {
        factor = 1.214f;
    }
    Item::greenShellSpeed               = 105.0f  * 0.9f * 1.25;
    Item::redShellInitialSpeed          = 75.0f   * 0.9f * 1.25;
    Item::redShellSpeed                 = 130.0f  * 0.9f * 1.25;
    Item::blueShellSpeed                = 260.0f  * 0.9f * 1.25;
    Item::blueShellMinimumDiveDistance  = 640000.0f * 0.9f * 1.25;
    Item::blueShellHomingSpeed          = 130.0f  * 0.9f * 1.25;
    Kart::hardSpeedCap                  = 120.0f;
    Kart::bulletSpeed                   = 145.0f  * 0.9f * 1.25;
    Kart::starSpeed                     = 105.0f  * 0.9f * 1.25;
    Kart::megaTCSpeed                   = 95.0f   * 0.9f * 1.25;
    }

    else if (MKWG::System::Is50cc() && Racedata::sInstance->racesScenario.settings.engineClass == CC_50) {
        if (gameMode != MODE_BATTLE && gameMode != MODE_PUBLIC_BATTLE && gameMode != MODE_PRIVATE_BATTLE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) {
            factor = 1.0f;
        }
        else if (gameMode == MODE_BATTLE || gameMode == MODE_PUBLIC_BATTLE || gameMode == MODE_PRIVATE_BATTLE) {
            factor = 1.214f;
        }
        Item::greenShellSpeed               = 105.0f;
        Item::redShellInitialSpeed          = 75.0f;
        Item::redShellSpeed                 = 130.0f;
        Item::blueShellSpeed                = 260.0f;
        Item::blueShellMinimumDiveDistance  = 640000.0f;
        Item::blueShellHomingSpeed          = 130.0f;
        Kart::hardSpeedCap                  = 120.0f;
        Kart::bulletSpeed                   = 145.0f;
        Kart::starSpeed                     = 105.0f;
        Kart::megaTCSpeed                   = 95.0f;
    }
    stats->baseSpeed *= factor;
    stats->standard_acceleration_as[0] *= factor;
    stats->standard_acceleration_as[1] *= factor;
    stats->standard_acceleration_as[2] *= factor;
    stats->standard_acceleration_as[3] *= factor;        
    if (MKWG::System::Is99999cc() && Racedata::sInstance->racesScenario.settings.engineClass == CC_50) {
        Kart::minDriftSpeedRatio = 0.55f;
    }
    else Kart::minDriftSpeedRatio = 0.55f * (1.0f / factor); 
    Kart::unknown_70 = 70.0f * factor;
    Kart::regularBoostAccel = 3.0f * factor;
    return stats;
}
kmCall(0x8058f670, ApplySpeedModifier);

kmWrite32(0x805336B8, 0x60000000);
kmWrite32(0x80534350, 0x60000000);
kmWrite32(0x80534BBC, 0x60000000);
kmWrite32(0x80723D10, 0x281D0009);
kmWrite32(0x80723D40, 0x3BA00009);
kmWrite24(0x808AAA0C, 'PUL');
}//namespace Race
}//namespace Pulsar

