#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWIi/Item/ItemPlayer.hpp>
#include <MarioKartWii/Item/Obj/Kumo.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <PulsarSystem.hpp>
#include <Settings/SettingsParam.hpp>
#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MKWG.hpp>
#include <core/rvl/OS/OSCache.hpp>
#include <runtimeWrite.hpp>

namespace Pulsar {
namespace Race {

void MegaTC(Kart::Movement& movement, int frames, int unk0, int unk1) {
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW) {
        movement.ApplyLightningEffect(frames, unk0, unk1);
        return;
    }
    else if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_REGIONAL || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_REGIONAL) {
        movement.ActivateMega();
        return;
    }
    else if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || mode ==  MODE_VS_RACE || mode == MODE_BATTLE) {
        if (Pulsar::Settings::Mgr::Get().GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG2, Pulsar::SETTINGMKWG_THUNDERCLOUD) == Pulsar::MKWGSETTING_THUNDERCLOUD_MEGA) {
            movement.ActivateMega();
            return;
        }
    } else {
        if (Pulsar::Settings::Mgr::Get().GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG2, Pulsar::SETTINGMKWG_THUNDERCLOUD) == Pulsar::MKWGSETTING_THUNDERCLOUD_MEGA) {
            movement.ActivateMega();
            return;
        }
    }
    // Default behavior (SHRINK)
    movement.ApplyLightningEffect(frames, unk0, unk1);
}
kmCall(0x80580630, MegaTC);

void LoadCorrectTCBRRES(Item::ObjKumo& objKumo, const char* mdlName, const char* shadowSrc, u8 whichShadowListToUse,
    Item::Obj::AnmParam* anmParam) {
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || 
        RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || 
        mode == MODE_VS_RACE || mode == MODE_BATTLE) {
        // MEGA mode uses megaTC.brres
        if ((RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_WW && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_BT_WW) && (Pulsar::Settings::Mgr::Get().GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG2, Pulsar::SETTINGMKWG_THUNDERCLOUD) == Pulsar::MKWGSETTING_THUNDERCLOUD_MEGA || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_REGIONAL || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_REGIONAL)) {
            objKumo.LoadGraphics("megaTC.brres", mdlName, shadowSrc, 1, anmParam, static_cast<nw4r::g3d::ScnMdl::BufferOption>(0), nullptr, 0);
            return;
        }
    }
    objKumo.LoadGraphicsImplicitBRRES(mdlName, shadowSrc, 1, anmParam, 
        static_cast<nw4r::g3d::ScnMdl::BufferOption>(0), nullptr);
}
kmCall(0x807af568, LoadCorrectTCBRRES);

//TCOnActivation, BlueLeopard, Erythtini
kmRuntimeUse(0x80798004);
static void UpdateManualTCActivation() { 
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_REGIONAL || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_REGIONAL) 
        kmRuntimeWrite16A(0x80798004, 0x4082);
    else if (Pulsar::Settings::Mgr::Get().GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG2, Pulsar::SETTINGMKWG_THUNDERCLOUD) == Pulsar::MKWGSETTING_THUNDERCLOUD_ACTIVATION && (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || mode == MODE_VS_RACE || mode == MODE_BATTLE)) 
        kmRuntimeWrite16A(0x80798004, 0x4800);
    else if (Pulsar::Settings::Mgr::Get().GetUserSettingValue(Pulsar::Settings::SETTINGSTYPE_MKWG2, Pulsar::SETTINGMKWG_THUNDERCLOUD) == Pulsar::MKWGSETTING_THUNDERCLOUD_ACTIVATION) 
        kmRuntimeWrite16A(0x80798004, 0x4800);
    else kmRuntimeWrite16A(0x80798004, 0x4082);
}
static RaceLoadHook UpdateTCModeHook(UpdateManualTCActivation);

} // Race
} // Pulsar