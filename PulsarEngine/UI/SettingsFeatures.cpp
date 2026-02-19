/*#include <kamek.hpp>
#include <Settings/SettingsParam.hpp>
#include <PulsarSystem.hpp>
#include <SlotExpansion/UI/ExpCupSelect.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/UI/Section/SectionMgr.hpp>
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
#include <MarioKartWii/UI/Ctrl/CtrlRace/CtrlRaceRankNum.hpp>
#include <MarioKartWii/Race/Raceinfo/Raceinfo.hpp>
#include <Gamemodes/KO/KOMgr.hpp>
#include <Gamemodes/KO/KORaceEndPage.hpp>
#include <Gamemodes/KO/KOWinnerPage.hpp>
#include <MarioKartWii/3D/Camera/CameraMgr.hpp>
#include <MarioKartWii/3D/Camera/RaceCamera.hpp>
#include "core/rvl/gx/GX.hpp"
#include <types.hpp>

namespace Pulsar{
namespace Settings{



void HUDColors() {
    if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_COLORS) == COLORS_MKWG) {
        // MKWG Purple
        u8 red = 0xFF, green = 0x00, blue = 0xFF, alpha = 0xFF;
        U8_RED1 = U8_RED2 = U8_RED3 = U8_RED4 = U8_RED5 = U8_RED6 = red;
        U8_GREEN1 = U8_GREEN2 = U8_GREEN3 = U8_GREEN4 = U8_GREEN5 = U8_GREEN6 = green;
        U8_BLUE1 = U8_BLUE2 = U8_BLUE3 = U8_BLUE4 = U8_BLUE5 = U8_BLUE6 = blue;
        U8_ALPHA1 = U8_ALPHA2 = U8_ALPHA3 = U8_ALPHA4 = U8_ALPHA5 = U8_ALPHA6 = alpha;
    }
    else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_COLORS) == COLORS_DEFAULT) {
        // Default Yellow
        U8_RED1 = 0xFF; U8_GREEN1 = 0xFF; U8_BLUE1 = 0x00; U8_ALPHA1 = 0xFF;
        U8_RED2 = 0xD2; U8_GREEN2 = 0xAA; U8_BLUE2 = 0x00; U8_ALPHA2 = 0xFF;
        U8_RED3 = 0xD2; U8_GREEN3 = 0xAA; U8_BLUE3 = 0x00; U8_ALPHA3 = 0x46;
        U8_RED4 = 0xFF; U8_GREEN4 = 0xFF; U8_BLUE4 = 0x00; U8_ALPHA4 = 0xFF;
        U8_RED5 = 0xD2; U8_GREEN5 = 0xAA; U8_BLUE5 = 0x00; U8_ALPHA5 = 0xFF;
        U8_RED6 = 0xD2; U8_GREEN6 = 0xAA; U8_BLUE6 = 0x00; U8_ALPHA6 = 0x46;
    }
    else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_COLORS) == COLORS_CUSTOM) { 
        // Custom color
        u8 red   = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_RED)   * 16 + 15;
        u8 green = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_GREEN) * 16 + 15;
        u8 blue  = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_BLUE)  * 16 + 15;
        u8 alpha = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG4, SETTINGS_ALPHA) * 16 + 15;
        
        U8_RED1 = U8_RED2 = U8_RED3 = U8_RED4 = U8_RED5 = U8_RED6 = red;
        U8_GREEN1 = U8_GREEN2 = U8_GREEN3 = U8_GREEN4 = U8_GREEN5 = U8_GREEN6 = green;
        U8_BLUE1 = U8_BLUE2 = U8_BLUE3 = U8_BLUE4 = U8_BLUE5 = U8_BLUE6 = blue;
        U8_ALPHA1 = U8_ALPHA2 = U8_ALPHA3 = U8_ALPHA4 = U8_ALPHA5 = U8_ALPHA6 = alpha;
    }
}
PageLoadHook ColorHUD(HUDColors);

// Assuming CtrlRaceRankNum* ctrl is already valid
void CustomLoadPositionTracker(CtrlRaceRankNum* ctrl, u8 hudSlotId, int setting) {
    const char* variant = nullptr;

    if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_CUSTOM) {
            variant = "position_variant_1";
    } else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_MKWG) {
            variant = "position_variant_2";
    } else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_DEFAULT) {
            variant = "position_variant_3";
    }
}
kmCall(0x807f4bb4, CustomLoadPositionTracker);

} //namespace Settings

namespace KO {

// KO-mode pos tracker coloring with knockout flash
u8 PosTrackerColor(CtrlRaceRankNum& posTracker) {
    const u32 playerId = posTracker.GetPlayerId(); 
    nw4r::lyt::Picture* posPane = static_cast<nw4r::lyt::Picture*>(posTracker.layout.GetPaneByName("position"));
    Mgr* mgr = System::sInstance->koMgr;
    ut::Color color;
    color.r = 0xFF;
    color.g = 0xFF;
    color.b = 0xFF;
    color.a = 0xFF;
    if (posPane) {
        if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_DEFAULT) {
            color.r = 0xFF, color.g = 0xFF, color.b = 0xFF, color.a = 0xFF;
        }
        else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_MKWG) {
            color.r = 0x0, color.g = 0x0, color.b = 0xFF, color.a = 0xFF;
        }
        else if (Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, Pulsar::SETTINGS_POSITIONCOLORS) == SETTINGS_POSITIONCOLORS_CUSTOM) {
            color.r = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, SECSETTINGS_RED) * 16 + 15;
            color.g = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, SECSETTINGS_GREEN) * 16 + 15;
            color.b = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, SECSETTINGS_BLUE) * 16 + 15;
            color.a = Settings::Mgr::Get().GetUserSettingValue(Settings::SETTINGSTYPE_MKWG5, SECSETTINGS_ALPHA) * 16 + 15;
        }
        if (System::sInstance->IsContext(PULSAR_MODE_KO)) {
            u8 idx = posTracker.hudSlotId;
            u8 pid = mgr->isSpectating ? RaceCameraMgr::sInstance->focusedPlayerIdx
                                        : Racedata::sInstance->racesScenario.settings.hudPlayerIds[idx];
            if (Raceinfo::sInstance->raceFrames > 0 && mgr->GetWouldBeKnockedOut(pid)) {
                s32 increment = mgr->posTrackerAnmFrames[idx] >= 31 ? 8 : -8;
                color.g = posPane->vertexColours[0].g + increment;
                color.b = color.g;
                mgr->posTrackerAnmFrames[idx] = (mgr->posTrackerAnmFrames[idx] + 1) % 62;
            } else {
                mgr->posTrackerAnmFrames[idx] = 0;
            }
        }
        for (int i = 0; i < 4; i++) posPane->vertexColours[i] = color;
    }
    return playerId;
}
kmCall(0x807f4b64, PosTrackerColor);
} // namespace KO
} // namespace Pulsar
*/ 