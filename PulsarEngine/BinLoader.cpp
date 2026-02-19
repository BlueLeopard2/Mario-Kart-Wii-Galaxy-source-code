#include <kamek.hpp>
#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MKWG.hpp>

namespace MKWG {

void *GetCustomKartParam(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length) {
    bool insideAll = false;
    bool outsideAll = false;
    bool insideBike = false;
    bool riibalanced = false;
    bool gimmick = false;

    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    const GameMode gameMode = Racedata::sInstance->menusScenario.settings.gamemode;

    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST) {
        insideAll = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODEALLINSIDE) ? Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_ALLINSIDE : Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
        outsideAll = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODEALLOUTSIDE) ? Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_ALLOUTSIDE : Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
        insideBike = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODEBIKEINSIDE) ? Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_BIKEINSIDE : Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
        riibalanced = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODERIIBALANCED) ? Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_RIIBALANCED : Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
        gimmick = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODEGIMMICK) ? Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_GIMMICK : Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
    }

    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_REGIONAL || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_REGIONAL || mode == MODE_TIME_TRIAL || mode == MODE_GHOST_RACE || Pulsar::MKWGSETTING_FORCE_VEHICLEMODE_REGULAR) {
        name = "kartParamMKWG.bin";
    }
    else if (insideAll) {
        name = "kartParamIn.bin";
    }
    else if (outsideAll) {
        name = "kartParamOut.bin";
    }
    else if (insideBike) {
        name = "kartParamBikeInside.bin";
    }
    else if (riibalanced) {
        name = "kartParamRiibalanced.bin";
    }
    else if (gimmick) {
        name = "kartParamGimmick.bin";
    }
    else if (static_cast<Pulsar::MKWGSettingVehicleMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_VEHICLEMODE)) == Pulsar::MKWGSETTING_VEHICLEMODE_ALLINSIDE) {
        name = "kartParamIn.bin";
    }
    else if (static_cast<Pulsar::MKWGSettingVehicleMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_VEHICLEMODE)) == Pulsar::MKWGSETTING_VEHICLEMODE_ALLOUTSIDE) {
        name = "kartParamOut.bin";
    }
    else if (static_cast<Pulsar::MKWGSettingVehicleMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_VEHICLEMODE)) == Pulsar::MKWGSETTING_VEHICLEMODE_BIKEINSIDE) {
        name = "kartParamBikeInside.bin";
    }
    else if (static_cast<Pulsar::MKWGSettingVehicleMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_VEHICLEMODE)) == Pulsar::MKWGSETTING_VEHICLEMODE_RIIBALANCED) {
        name = "kartParamRiibalanced.bin";
    }
    else if (static_cast<Pulsar::MKWGSettingVehicleMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_VEHICLEMODE)) == Pulsar::MKWGSETTING_VEHICLEMODE_GIMMICK) {
        name = "kartParamGimmick.bin";
    }

    return archive->GetFile(type, name, length);
}
kmCall(0x80591a30, GetCustomKartParam);

void *GetCustomKartAIParam(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length) {
    const GameMode gameMode = Racedata::sInstance->racesScenario.settings.gamemode;
    if (static_cast<Pulsar::MKWGSettingHardCPUS>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG3), Pulsar::SETTINGMKWG_HARDCPUS)) == Pulsar::MKWGSETTING_HARDCPUS_ENABLED) {
        name = "KartAISpdParamHard.bin";
    }

    return archive->GetFile(type, name, length);
}
kmCall(0x8073ae9c, GetCustomKartAIParam);

void *GetCustomItemSlot(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length){
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    bool itemModeRegular = false;
    bool itemModeMushroom = false; 
    bool itemModeRandom = false;
    bool itemModeBlastBlitz = false; 

    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST) {
        itemModeRegular = System::sInstance->IsContext(Pulsar::PULSAR_VEHICLEMODEALLINSIDE) ? Pulsar::MKWGSETTING_ITEMMODE_REGULAR : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        itemModeMushroom = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODEMUSHROOM) ? Pulsar::MKWGSETTING_ITEMMODE_MUSHROOM : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODERANDOM) ? Pulsar::MKWGSETTING_ITEMMODE_RANDOM : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        itemModeBlastBlitz = System::sInstance->IsContext(Pulsar::PULSAR_ITEMMODEBLASTBLITZ) ? Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ : Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
    } else {
        itemModeRegular = static_cast<Pulsar::MKWGSettingItemMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG2), Pulsar::SETTINGMKWG_ITEMMODE)) == Pulsar::MKWGSETTING_ITEMMODE_REGULAR;
        itemModeMushroom =  static_cast<Pulsar::MKWGSettingItemMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG2), Pulsar::SETTINGMKWG_ITEMMODE)) == Pulsar::MKWGSETTING_ITEMMODE_MUSHROOM;
        itemModeRandom = static_cast<Pulsar::MKWGSettingItemMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG2), Pulsar::SETTINGMKWG_ITEMMODE)) == Pulsar::MKWGSETTING_ITEMMODE_RANDOM;
        itemModeBlastBlitz =  static_cast<Pulsar::MKWGSettingItemMode>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_MKWG2), Pulsar::SETTINGMKWG_ITEMMODE)) == Pulsar::MKWGSETTING_ITEMMODE_BLASTBLITZ;
    }

    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_REGIONAL || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_REGIONAL) {
        itemModeRegular = true;
        itemModeRandom = itemModeMushroom = itemModeBlastBlitz = false;
    }

    if (itemModeRegular) {
        name="ItemSlotMKWG.bin";
    }
    else if (itemModeMushroom) {
        name="ItemSlotMushroom.bin";
    }
    else if (itemModeRandom) {
        name="ItemSlotRandom.bin";
    }
    else if (itemModeBlastBlitz) {
        name="ItemSlotBlastBlitz.bin";
    }
    return archive->GetFile(type, name, length);
} 
kmCall(0x807bb128, GetCustomItemSlot);
kmCall(0x807bb030, GetCustomItemSlot);
kmCall(0x807bb200, GetCustomItemSlot);
kmCall(0x807bb53c, GetCustomItemSlot);
kmCall(0x807bbb58, GetCustomItemSlot);
}

