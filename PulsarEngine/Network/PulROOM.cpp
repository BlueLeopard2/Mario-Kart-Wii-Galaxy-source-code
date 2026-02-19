#include <kamek.hpp>
#include <MarioKartWii/RKNet/ROOM.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <Settings/UI/SettingsPanel.hpp>
#include <Settings/Settings.hpp>
#include <Network/Network.hpp>
#include <Network/PacketExpansion.hpp>

namespace Pulsar {
namespace Network {

//Implements the ability for a host to send a message, allowing for custom host settings

//If we are in a room, we are guaranteed to be in a situation where Pul packets are being sent
//however, no reason to send the settings outside of START packets and if we are not the host, this is easily changed by just editing the check

static void ConvertROOMPacketToData(const PulROOM& packet) {
    System* system = System::sInstance;
    system->netMgr.hostContext = packet.hostSystemContext;
    system->netMgr.racesPerGP = packet.raceCount;
    system->netMgr.KOContext = packet.KOSystemContext;
}

static void BeforeROOMSend(RKNet::PacketHolder<PulROOM>* packetHolder, PulROOM* src, u32 len) {
    packetHolder->Copy(src, len); //default

    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
    Pulsar::System* system = Pulsar::System::sInstance;
    PulROOM* destPacket = packetHolder->packet;
    if (destPacket->messageType == 1 && sub.localAid == sub.hostAid) {
        packetHolder->packetSize += sizeof(PulROOM) - sizeof(RKNet::ROOMPacket); //this has been changed by copy so it's safe to do this
        const Settings::Mgr& settings = Settings::Mgr::Get();

        const u8 koSetting = settings.GetSettingValue(Settings::SETTINGSTYPE_KO, SETTINGKO_ENABLED) && destPacket->message == 0; //KO only enabled for normal GPs
        //invert mii setting as the first button is enabled, not disabled, so a value of 1 indicates disabled
        const u8 ottOnline = settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ONLINE);
        const u8 ottChangeCombo = settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ALLOWCHANGECOMBO) == OTTSETTING_COMBO_ENABLED;
        const u8 charRestrictLight = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_LIGHT;
        const u8 charRestrictMedium = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_MEDIUM;
        const u8 charRestrictHeavy = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_HEAVY;
        const u8 kartRestrict = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_VEHICLERESTRICT) == MKWGSETTING_VEHICLERESTRICT_KARTS;
        const u8 bikeRestrict = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_VEHICLERESTRICT) == MKWGSETTING_VEHICLERESTRICT_BIKES;
        const u8 itemModeRandom = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_RANDOM;
        const u8 itemModeBlastBlitz = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_BLASTBLITZ;
        const u8 itemModeMushroom = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_MUSHROOM;
        const u8 flyingBloop = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_FLYINGBLOOP) == MKWGSETTING_FLYINGBLOOP_BLOOPER;
        const u8 koFinal = settings.GetSettingValue(Settings::SETTINGSTYPE_KO, SETTINGKO_FINAL) == KOSETTING_FINAL_ALWAYS;
        const u8 vehicleModeRegular = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_REGULAR;
        const u8 vehicleModeAllInside = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_ALLINSIDE;
        const u8 vehicleModeBikeInside = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_BIKEINSIDE;
        const u8 vehicleModeAllOutside = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_ALLOUTSIDE;
        const u8 vehicleModeRiibalanced = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_RIIBALANCED;
        const u8 vehicleModeGimmick = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_FORCEVEHICLEMODE) == MKWGSETTING_FORCE_VEHICLEMODE_GIMMICK;
        const u8 lapsDefault = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_DEFAULT;
        const u8 laps1 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_1;
        const u8 laps2 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_2;
        const u8 laps3 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_3;
        const u8 laps4 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_4;
        const u8 laps5 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_5;
        const u8 NoInvisWalls = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_INVIS_WALLS) == MKWGSETTING_INVISWALLS_DISABLED;
        //const u8 ottUMT = settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ALLOWUMTS) == OTTSETTING_UMTS_ENABLED;

        
        destPacket->hostSystemContext = 
        (static_cast<u64>(ottOnline != OTTSETTING_OFFLINE_DISABLED) << PULSAR_MODE_OTT) //ott
      | (static_cast<u64>(ottOnline == OTTSETTING_ONLINE_FEATHER) << PULSAR_FEATHER) //ott feather
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ALLOWUMTS) ^ true) << PULSAR_UMTS) //ott umts
      | (static_cast<u64>(ottChangeCombo) << PULSAR_CHANGECOMBO)
      | (static_cast<u64>(koSetting) << PULSAR_MODE_KO)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_ALLOW_MIIHEADS) ^ true) << PULSAR_MIIHEADS)
      | (static_cast<u64>(charRestrictLight) << PULSAR_CHARRESTRICTLIGHT)
      | (static_cast<u64>(charRestrictMedium) << PULSAR_CHARRESTRICTMEDIUM)
      | (static_cast<u64>(charRestrictHeavy) << PULSAR_CHARRESTRICTHEAVY)
      | (static_cast<u64>(kartRestrict) << PULSAR_KARTRESTRICT)
      | (static_cast<u64>(bikeRestrict) << PULSAR_BIKERESTRICT)
      | (static_cast<u64>(itemModeRandom) << PULSAR_ITEMMODERANDOM)
      | (static_cast<u64>(itemModeBlastBlitz) << PULSAR_ITEMMODEBLASTBLITZ)
      | (static_cast<u64>(itemModeMushroom) << PULSAR_ITEMMODEMUSHROOM)
      | (static_cast<u64>(koFinal) << PULSAR_KOFINAL)
      | (static_cast<u64>(vehicleModeRegular) << PULSAR_VEHICLEMODEREGULAR)
      | (static_cast<u64>(vehicleModeAllInside) << PULSAR_VEHICLEMODEALLINSIDE)
      | (static_cast<u64>(vehicleModeAllOutside) << PULSAR_VEHICLEMODEALLOUTSIDE)
      | (static_cast<u64>(vehicleModeBikeInside) << PULSAR_VEHICLEMODEBIKEINSIDE)
      | (static_cast<u64>(vehicleModeRiibalanced) << PULSAR_VEHICLEMODERIIBALANCED)
      | (static_cast<u64>(vehicleModeGimmick) << PULSAR_VEHICLEMODEGIMMICK)
      | (static_cast<u64>(lapsDefault) << PULSAR_LAPSDEFAULT)
      | (static_cast<u64>(laps1) << PULSAR_LAPS1)
      | (static_cast<u64>(laps2) << PULSAR_LAPS2)
      | (static_cast<u64>(laps3) << PULSAR_LAPS3)
      | (static_cast<u64>(laps4) << PULSAR_LAPS4)
      | (static_cast<u64>(laps5) << PULSAR_LAPS5)
      | (static_cast<u64>(flyingBloop) << PULSAR_FLYINGBLOOP)
      | (static_cast<u64>(settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_THUNDERCLOUD)) << PULSAR_THUNDERCLOUD)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_CC)) << PULSAR_50)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_CC)) << PULSAR_100)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_CC)) << PULSAR_400)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_CC)) << PULSAR_99999)
      | (static_cast<u64>(settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_RADIO_HOSTWINS)) << PULSAR_HAW);

      destPacket->KOSystemContext = (static_cast<u64>(koSetting) << PULSAR_MODE_KO)
      | (static_cast<u64>(koFinal) << PULSAR_KOFINAL);

        u8 raceCount;
        if (koSetting == KOSETTING_ENABLED) raceCount = 0xFE;
        else switch (settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_SCROLL_GP_RACES)) {
        case(1):
            raceCount = 7;
            break;
        case(2):
            raceCount = 11;
            break;
        case(3):
            raceCount = 23;
            break;
        case(4):
            raceCount = 31;
            break;
        case(5):
            raceCount = 63;
            break;
        case(6):
            raceCount = 1;
            break;
        default:
            raceCount = 3;
        }
        destPacket->raceCount = raceCount;
        ConvertROOMPacketToData(*destPacket);
    }
}
kmCall(0x8065b15c, BeforeROOMSend);

kmWrite32(0x8065add0, 0x60000000);
static void AfterROOMReception(const RKNet::PacketHolder<PulROOM>* packetHolder, const PulROOM& src, u32 len) {
    register RKNet::ROOMPacket* packet;
    asm(mr packet, r28;);
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
    const Settings::Mgr& settings = Settings::Mgr::Get();
    //START msg sent by the host, size check should always be guaranteed in theory
    if (src.messageType == 1 && sub.localAid != sub.hostAid && packetHolder->packetSize == sizeof(PulROOM)) {
        ConvertROOMPacketToData(src);

    bool isCharRestrictLight = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_LIGHT;
    bool isCharRestrictMedium = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_MEDIUM;
    bool isCharRestrictHeavy = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_CHARRESTRICT) == MKWGSETTING_CHARRESTRICT_HEAVY;
    bool isKartRestrictKart = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_VEHICLERESTRICT) == MKWGSETTING_VEHICLERESTRICT_KARTS;
    bool isKartRestrictBike = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG1, SETTINGMKWG_VEHICLERESTRICT) == MKWGSETTING_VEHICLERESTRICT_BIKES;
    bool isItemModeRandom = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_RANDOM;
    bool isItemModeBlastBlitz = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_BLASTBLITZ;
    bool isItemModeMushroom = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_ITEMMODE) == MKWGSETTING_ITEMMODE_MUSHROOM;
    bool isFlyingBloop = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_FLYINGBLOOP) == MKWGSETTING_FLYINGBLOOP_BLOOPER;
    bool isLapsDefault = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_DEFAULT;
    bool isLaps1 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_1;
    bool isLaps2 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_2;
    bool isLaps3 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_3;
    bool isLaps4 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_4;
    bool isLaps5 = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_LAPCOUNT) == MKWGSETTING_LAPS_5;
    bool isNoInvisWalls = settings.GetUserSettingValue(Settings::SETTINGSTYPE_MKWG2, SETTINGMKWG_INVIS_WALLS) == MKWGSETTING_INVISWALLS_DISABLED;
        
    u64 newContext = 0;
    Network::Mgr& netMgr = Pulsar::System::sInstance->netMgr;
        newContext = netMgr.hostContext;
        isCharRestrictLight = newContext & (1ULL << PULSAR_CHARRESTRICTLIGHT);
        isCharRestrictMedium = newContext & (1ULL << PULSAR_CHARRESTRICTMEDIUM);
        isCharRestrictHeavy = newContext & (1ULL << PULSAR_CHARRESTRICTHEAVY);
        isKartRestrictKart = newContext & (1ULL << PULSAR_KARTRESTRICT);
        isKartRestrictBike = newContext & (1ULL << PULSAR_BIKERESTRICT);
        isItemModeRandom = newContext & (1ULL << PULSAR_ITEMMODERANDOM);
        isItemModeMushroom = newContext & (1ULL << PULSAR_ITEMMODEMUSHROOM);
        isItemModeBlastBlitz = newContext & (1ULL << PULSAR_ITEMMODEBLASTBLITZ);
        isFlyingBloop = newContext & (1ULL << PULSAR_FLYINGBLOOP);
        isLapsDefault = newContext & (1ULL << PULSAR_LAPSDEFAULT);
        isLaps1 = newContext & (1ULL << PULSAR_LAPS1);
        isLaps2 = newContext & (1ULL << PULSAR_LAPS2);
        isLaps3 = newContext & (1ULL << PULSAR_LAPS3);
        isLaps4 = newContext & (1ULL << PULSAR_LAPS4);
        isLaps5 = newContext & (1ULL << PULSAR_LAPS5);
        isNoInvisWalls = newContext & (1ULL << PULSAR_NOINVISWALLS);
    netMgr.hostContext = newContext;

    u64 context = (static_cast<u64>(isCharRestrictLight) << PULSAR_CHARRESTRICTLIGHT) | (static_cast<u64>(isCharRestrictMedium) << PULSAR_CHARRESTRICTMEDIUM) | (static_cast<u64>(isCharRestrictHeavy) << PULSAR_CHARRESTRICTHEAVY) | (static_cast<u64>(isKartRestrictKart) << PULSAR_KARTRESTRICT) | (static_cast<u64>(isKartRestrictBike) << PULSAR_BIKERESTRICT) | (static_cast<u64>(isItemModeRandom) << PULSAR_ITEMMODERANDOM) | (static_cast<u64>(isItemModeMushroom) << PULSAR_ITEMMODEMUSHROOM) | (static_cast<u64>(isItemModeBlastBlitz) << PULSAR_ITEMMODEBLASTBLITZ) | (static_cast<u64>(isFlyingBloop) << PULSAR_FLYINGBLOOP) | (static_cast<u64>(isLapsDefault) << PULSAR_LAPSDEFAULT) | (static_cast<u64>(isLaps1) << PULSAR_LAPS1) | (static_cast<u64>(isLaps2) << PULSAR_LAPS2) | (static_cast<u64>(isLaps3) << PULSAR_LAPS3) | (static_cast<u64>(isLaps4) << PULSAR_LAPS4) | (static_cast<u64>(isNoInvisWalls) << PULSAR_NOINVISWALLS);
    Pulsar::System::sInstance->context = context;
        
        //Also exit the settings page to prevent weird graphical artefacts
        Page* topPage = SectionMgr::sInstance->curSection->GetTopLayerPage();
        PageId topId = topPage->pageId;
        if (topId == UI::SettingsPanel::id) {
            UI::SettingsPanel* panel = static_cast<UI::SettingsPanel*>(topPage);
            panel->OnBackPress(0);
        }
    }
    memcpy(packet, &src, sizeof(RKNet::ROOMPacket)); //default
}
kmCall(0x8065add8, AfterROOMReception);

/*
//ROOMPacket bits arrangement: 0-4 GPraces
//u8 racesPerGP = 0;



//Adds the settings to the free bits of the packet, only called for the host, msgType1 has 14 free bits as the game only has 4 gamemodes
void SetAllToSendPackets(RKNet::ROOMHandler& roomHandler, u32 packetArg) {
    RKNet::ROOMPacketReg packetReg ={ packetArg };
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const u8 localAid = controller->subs[controller->currentSub].localAid;
    Pulsar::System* system = Pulsar::System::sInstance;
    if((packetReg.packet.messageType) == 1 && localAid == controller->subs[controller->currentSub].hostAid) {
        const u8 hostParam = Settings::Mgr::GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_RADIO_HOSTWINS);
        packetReg.packet.message |= hostParam << 2; //uses bit 2 of message

        const u8 gpParam = Settings::Mgr::GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_SCROLL_GP_RACES);
        const u8 disableMiiHeads = Settings::Mgr::GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_ALLOW_MIIHEADS);
        packetReg.packet.message |= gpParam << 3; //uses bits 3-5
        packetReg.packet.message |= disableMiiHeads << 6; //uses bit 6
        packetReg.packet.message |= Settings::Mgr::GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ONLINE) << 7; //7 for OTT
        packetReg.packet.message |= Settings::Mgr::GetSettingValue(Settings::SETTINGSTYPE_KO, SETTINGKO_ENABLED) << 8; //8 for KO

        ConvertROOMPacketToData(packetReg.packet.message >> 2); //5 right now (2-8) + 1 reserved (9)
        packetReg.packet.message |= (System::sInstance->SetPackROOMMsg() << 0xA & 0b1111110000000000); //6 bits for packs (10-15)
    }
    for(int i = 0; i < 12; ++i) if(i != localAid) roomHandler.toSendPackets[i] = packetReg.packet;
}
kmBranch(0x8065ae70, SetAllToSendPackets);
//kmCall(0x805dce34, SetAllToSendPackets);
//kmCall(0x805dcd2c, SetAllToSendPackets);
//kmCall(0x805d9fe8, SetAllToSendPackets);

//Non-hosts extract the setting, store it and then return the packet without these bits
RKNet::ROOMPacket GetParamFromPacket(u32 packetArg, u8 aidOfSender) {
    RKNet::ROOMPacketReg packetReg ={ packetArg };
    if(packetReg.packet.messageType == 1) {
        const RKNet::Controller* controller = RKNet::Controller::sInstance;
        //Seeky's code to prevent guests from start the GP
        if(controller->subs[controller->currentSub].hostAid != aidOfSender) packetReg.packet.messageType = 0;
        else {
            ConvertROOMPacketToData((packetReg.packet.message & 0b0000001111111100) >> 2);
            System::sInstance->ParsePackROOMMsg(packetReg.packet.message >> 0xA);
        }
        packetReg.packet.message &= 0x3;
        Page* topPage = SectionMgr::sInstance->curSection->GetTopLayerPage();
        PageId topId = topPage->pageId;
        if(topId == UI::SettingsPanel::id) {
            UI::SettingsPanel* panel = static_cast<UI::SettingsPanel*>(topPage);
            panel->OnBackPress(0);
        }
    }
    return packetReg.packet;
}
kmBranch(0x8065af70, GetParamFromPacket);
*/

//Implements that setting
kmCall(0x806460B8, System::GetRaceCount);
kmCall(0x8064f51c, System::GetRaceCount);
}//namespace Network
}//namespace Pulsar
