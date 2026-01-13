#include "mcc/settings/Settings.h"
#include "global/Global.h"
#include "nlohmann/json.hpp"

#include <offset_mcc.h>
#include <filesystem>
#include <fstream>
#include <windows.h>

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace MCC::Settings {
    static SplitscreenConfig g_Config;
    static CGameManager::Profile_t g_Profiles[4];

    static fs::path GetConfigPath() {
    char exePath[MAX_PATH] = {0};
    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH))
        return "settings.json";

        fs::path dir = fs::path(exePath).parent_path();
        return dir / "settings.json";
    }    

    const SplitscreenConfig& Splitscreen::Get() {
        return g_Config;
    }

    bool Splitscreen::Load() {
        fs::path path = GetConfigPath();

        if (!fs::exists(path))
            return false;

        std::ifstream file(path);
        if (!file.is_open())
            return false;

        json j;
        file >> j;

        if (!j.contains("splitscreen"))
            return false;

        auto& s = j["splitscreen"];

        g_Config.b_override            = s.value("b_override", false);
        g_Config.player_count          = s.value("player_count", 1);
        g_Config.b_use_player0_profile = s.value("b_use_player0_profile", true);
        g_Config.b_player0_use_km      = s.value("b_player0_use_km", false);
        g_Config.b_override_profile    = s.value("b_override_profile", false);

        return true;
    }

    bool Profile::Load() {
        fs::path path = GetConfigPath();

        if (!fs::exists(path))
            return false;

            std::ifstream file(path);
            if (!file.is_open())
                return false;

            json j;
            file >> j;

            if (!j.contains("profile"))
                return false;

            auto& p = j["profile"];
            for (int i = 0; i < 4; ++i) {
                if (!p.contains(std::to_string(i)))
                    continue;

                auto& entry = p[std::to_string(i)];
                g_Profiles[i].id = entry.value("id", g_Profiles[i].id);
                g_Profiles[i].controller_index = entry.value("controller_index", g_Profiles[i].controller_index);
                auto name = entry.value("name", "");
                if (!name.empty())
                    mbstowcs(g_Profiles[i].name, name.c_str(), 1024);
                if (!entry.contains("profile"))
                    continue;

                auto& prof = entry["profile"];
                auto& dst = g_Profiles[i].profile;

                dst.SubtitleSetting = prof.value("SubtitleSetting", dst.SubtitleSetting);
                dst.SubtitleSizeSetting = prof.value("SubtitleSizeSetting", dst.SubtitleSizeSetting);
                dst.SubtitleBackgroundSetting = prof.value("SubtitleBackgroundSetting", dst.SubtitleBackgroundSetting);
                dst.SubtitleShadowColorSetting = prof.value("SubtitleShadowColorSetting", dst.SubtitleShadowColorSetting);
                dst.DialogueColorSetting = prof.value("DialogueColorSetting", dst.DialogueColorSetting);
                dst.DialoguePaletteSetting = prof.value("DialoguePaletteSetting", dst.DialoguePaletteSetting);
                dst.SpeakerSetting = prof.value("SpeakerSetting", dst.SpeakerSetting);
                dst.SpeakerColorStyleSetting = prof.value("SpeakerColorStyleSetting", dst.SpeakerColorStyleSetting);
                dst.SpeakerColorSetting = prof.value("SpeakerColorSetting", dst.SpeakerColorSetting);
                dst.SpeakerPaletteSetting = prof.value("SpeakerPaletteSetting", dst.SpeakerPaletteSetting);
                dst.SubtitleFontSetting = prof.value("SubtitleFontSetting", dst.SubtitleFontSetting);
                dst.SubtitleBackgroundOpacitySetting = prof.value("SubtitleBackgroundOpacitySetting", dst.SubtitleBackgroundOpacitySetting);
                dst.SubtitleShadowOpacitySetting = prof.value("SubtitleShadowOpacitySetting", dst.SubtitleShadowOpacitySetting);
                dst.FOVSetting = prof.value("FOVSetting", dst.FOVSetting);
                dst.VehicleFOVSetting = prof.value("VehicleFOVSetting", dst.VehicleFOVSetting);
                dst.CrosshairLocation = prof.value("CrosshairLocation", dst.CrosshairLocation);
                dst.LookControlsInverted = prof.value("LookControlsInverted", dst.LookControlsInverted);
                dst.MouseLookControlsInverted = prof.value("MouseLookControlsInverted", dst.MouseLookControlsInverted);
                dst.VibrationDisabled = prof.value("VibrationDisabled", dst.VibrationDisabled);
                dst.ImpulseTriggersDisabled = prof.value("ImpulseTriggersDisabled", dst.ImpulseTriggersDisabled);
                dst.AircraftControlsInverted = prof.value("AircraftControlsInverted", dst.AircraftControlsInverted);
                dst.MouseAircraftControlsInverted = prof.value("MouseAircraftControlsInverted", dst.MouseAircraftControlsInverted);
                dst.AutoCenterEnabled = prof.value("AutoCenterEnabled", dst.AutoCenterEnabled);
                dst.CrouchLockEnabled = prof.value("CrouchLockEnabled", dst.CrouchLockEnabled);
                dst.MKCrouchLockEnabled = prof.value("MKCrouchLockEnabled", dst.MKCrouchLockEnabled);
                dst.ClenchProtectionEnabled = prof.value("ClenchProtectionEnabled", dst.ClenchProtectionEnabled);
                dst.UseFemaleVoice = prof.value("UseFemaleVoice", dst.UseFemaleVoice);
                dst.HoldToZoom = prof.value("HoldToZoom", dst.HoldToZoom);
                dst.PlayerModelPrimaryColorIndex = prof.value("PlayerModelPrimaryColorIndex", dst.PlayerModelPrimaryColorIndex);
                dst.PlayerModelSecondaryColorIndex = prof.value("PlayerModelSecondaryColorIndex", dst.PlayerModelSecondaryColorIndex);
                dst.PlayerModelTertiaryColorIndex = prof.value("PlayerModelTertiaryColorIndex", dst.PlayerModelTertiaryColorIndex);
                dst.UseEliteModel = prof.value("UseEliteModel", dst.UseEliteModel);
                dst.LockMaxAspectRatio = prof.value("LockMaxAspectRatio", dst.LockMaxAspectRatio);
                dst.un = prof.value("un", dst.un);
                dst.UsersSkinsEnabled = prof.value("UsersSkinsEnabled", dst.UsersSkinsEnabled);
                dst.PlayerModelPermutation = prof.value("PlayerModelPermutation", dst.PlayerModelPermutation);
                dst.HelmetIndex = prof.value("HelmetIndex", dst.HelmetIndex);
                dst.LeftShoulderIndex = prof.value("LeftShoulderIndex", dst.LeftShoulderIndex);
                dst.RightShoulderIndex = prof.value("RightShoulderIndex", dst.RightShoulderIndex);
                dst.ChestIndex = prof.value("ChestIndex", dst.ChestIndex);
                dst.WristIndex = prof.value("WristIndex", dst.WristIndex);
                dst.UtilityIndex = prof.value("UtilityIndex", dst.UtilityIndex);
                dst.ArmsIndex = prof.value("ArmsIndex", dst.ArmsIndex);
                dst.LegsIndex = prof.value("LegsIndex", dst.LegsIndex);
                dst.BackpackIndex = prof.value("BackpackIndex", dst.BackpackIndex);
                dst.SpartanBodyIndex = prof.value("SpartanBodyIndex", dst.SpartanBodyIndex);
                dst.SpartanArmorEffectIndex = prof.value("SpartanArmorEffectIndex", dst.SpartanArmorEffectIndex);
                dst.KneesIndex = prof.value("KneesIndex", dst.KneesIndex);
                dst.VisorColorIndex = prof.value("VisorColorIndex", dst.VisorColorIndex);
                dst.EliteHelmetIndex = prof.value("EliteHelmetIndex", dst.EliteHelmetIndex);
                dst.EliteLeftShoulderIndex = prof.value("EliteLeftShoulderIndex", dst.EliteLeftShoulderIndex);
                dst.EliteRightShoulderIndex = prof.value("EliteRightShoulderIndex", dst.EliteRightShoulderIndex);
                dst.EliteChestIndex = prof.value("EliteChestIndex", dst.EliteChestIndex);
                dst.EliteArmsIndex = prof.value("EliteArmsIndex", dst.EliteArmsIndex);
                dst.EliteLegsIndex = prof.value("EliteLegsIndex", dst.EliteLegsIndex);
                dst.EliteArmorIndex = prof.value("EliteArmorIndex", dst.EliteArmorIndex);
                dst.EliteArmorEffectIndex = prof.value("EliteArmorEffectIndex", dst.EliteArmorEffectIndex);
                dst.VoiceIndex = prof.value("VoiceIndex", dst.VoiceIndex);
                dst.PlayerModelPrimaryColor = prof.value("PlayerModelPrimaryColor", dst.PlayerModelPrimaryColor);
                dst.PlayerModelSecondaryColor = prof.value("PlayerModelSecondaryColor", dst.PlayerModelSecondaryColor);
                dst.PlayerModelTertiaryColor = prof.value("PlayerModelTertiaryColor", dst.PlayerModelTertiaryColor);
                dst.SpartanPose = prof.value("SpartanPose", dst.SpartanPose);
                dst.ElitePose = prof.value("ElitePose", dst.ElitePose);
                //TODO: implement Skin_t property 
                const std::string& s = prof["ServiceTag"].get<std::string>();
                wmemset(dst.ServiceTag, 0, 4);
                if (!s.empty()) {
                    mbstowcs(dst.ServiceTag, s.c_str(), 4);
                }
                dst.OnlineMedalFlasher = prof.value("OnlineMedalFlasher", dst.OnlineMedalFlasher);
                dst.VerticalLookSensitivity = prof.value("VerticalLookSensitivity", dst.VerticalLookSensitivity);
                dst.HorizontalLookSensitivity = prof.value("HorizontalLookSensitivity", dst.HorizontalLookSensitivity);
                dst.LookAcceleration = prof.value("LookAcceleration", dst.LookAcceleration);
                dst.LookAxialDeadZone = prof.value("LookAxialDeadZone", dst.LookAxialDeadZone);
                dst.LookRadialDeadZone = prof.value("LookRadialDeadZone", dst.LookRadialDeadZone);
                dst.ZoomLookSensitivityMultiplier = prof.value("ZoomLookSensitivityMultiplier", dst.ZoomLookSensitivityMultiplier);
                dst.VehicleLookSensitivityMultiplier = prof.value("VehicleLookSensitivityMultiplier", dst.VehicleLookSensitivityMultiplier);
                dst.ButtonPreset = prof.value("ButtonPreset", dst.ButtonPreset);
                dst.StickPreset = prof.value("StickPreset", dst.StickPreset);
                dst.LeftyToggle = prof.value("LeftyToggle", dst.LeftyToggle);
                dst.FlyingCameraTurnSensitivity = prof.value("FlyingCameraTurnSensitivity", dst.FlyingCameraTurnSensitivity);
                dst.FlyingCameraPanning = prof.value("FlyingCameraPanning", dst.FlyingCameraPanning);
                dst.FlyingCameraSpeed = prof.value("FlyingCameraSpeed", dst.FlyingCameraSpeed);
                dst.FlyingCameraThrust = prof.value("FlyingCameraThrust", dst.FlyingCameraThrust);
                dst.TheaterTurnSensitivity = prof.value("TheaterTurnSensitivity", dst.TheaterTurnSensitivity);
                dst.TheaterPanning = prof.value("TheaterPanning", dst.TheaterPanning);
                dst.TheaterSpeed = prof.value("TheaterSpeed", dst.TheaterSpeed);
                dst.TheaterThrust = prof.value("TheaterThrust", dst.TheaterThrust);
                dst.MKTheaterTurnSensitivity = prof.value("MKTheaterTurnSensitivity", dst.MKTheaterTurnSensitivity);
                dst.MKTheaterPanning = prof.value("MKTheaterPanning", dst.MKTheaterPanning);
                dst.MKTheaterSpeed = prof.value("MKTheaterSpeed", dst.MKTheaterSpeed);
                dst.MKTheaterThrust = prof.value("MKTheaterThrust", dst.MKTheaterThrust);
                dst.SwapTriggersAndBumpers = prof.value("SwapTriggersAndBumpers", dst.SwapTriggersAndBumpers);
                dst.UseModernAimControl = prof.value("UseModernAimControl", dst.UseModernAimControl);
                dst.UseDoublePressJumpToJetpack = prof.value("UseDoublePressJumpToJetpack", dst.UseDoublePressJumpToJetpack);
                dst.DualWieldInverted = prof.value("DualWieldInverted", dst.DualWieldInverted);
                dst.ControllerDualWieldInverted = prof.value("ControllerDualWieldInverted", dst.ControllerDualWieldInverted);
                dst.ControllerHornetControlJoystick = prof.value("ControllerHornetControlJoystick", dst.ControllerHornetControlJoystick);
                dst.ControllerBansheeTrickButtonsSwapped = prof.value("ControllerBansheeTrickButtonsSwapped", dst.ControllerBansheeTrickButtonsSwapped);
                dst.ColorCorrection = prof.value("ColorCorrection", dst.ColorCorrection);
                dst.EnemyPlayerNameColor = prof.value("EnemyPlayerNameColor", dst.EnemyPlayerNameColor);
                dst.GameEngineTimer = prof.value("GameEngineTimer", dst.GameEngineTimer);
            }

            return true;

    }

    bool Splitscreen::Save() {
        fs::path path = GetConfigPath();

        json j;

        j["splitscreen"] = {
            {"b_override", g_Config.b_override},
            {"player_count", g_Config.player_count},
            {"b_use_player0_profile", g_Config.b_use_player0_profile},
            {"b_player0_use_km", g_Config.b_player0_use_km},
            {"b_override_profile", g_Config.b_override_profile}
        };

        std::ofstream file(path, std::ios::trunc);
        if (!file.is_open())
            return false;

        file << j.dump(4);
        return true;
    }

    bool Profile::Save() {
            fs::path path = GetConfigPath();

            json j;
            if (fs::exists(path)) {
                std::ifstream file(path);
                if (file.is_open()) {
                    file >> j;
                }
            }

            for (int i = 0; i < 4; ++i) {
                std::string name;
                size_t len = wcslen(g_Profiles[i].name);
                name.resize(len);
                wcstombs(name.data(), g_Profiles[i].name, len);

                json prof;
                auto& src = g_Profiles[i].profile;

                prof["SubtitleSetting"] = src.SubtitleSetting;
                prof["SubtitleSizeSetting"] = src.SubtitleSizeSetting;
                prof["SubtitleBackgroundSetting"] = src.SubtitleBackgroundSetting;
                prof["SubtitleShadowColorSetting"] = src.SubtitleShadowColorSetting;
                prof["DialogueColorStyleSetting"] = src.DialogueColorStyleSetting;
                prof["DialogueColorSetting"] = src.DialogueColorSetting;
                prof["DialoguePaletteSetting"] = src.DialoguePaletteSetting;
                prof["SpeakerSetting"] = src.SpeakerSetting;
                prof["SpeakerColorStyleSetting"] = src.SpeakerColorStyleSetting;
                prof["SpeakerColorSetting"] = src.SpeakerColorSetting;
                prof["SpeakerPaletteSetting"] = src.SpeakerPaletteSetting;
                prof["SubtitleFontSetting"] = src.SubtitleFontSetting;
                prof["SubtitleBackgroundOpacitySetting"] = src.SubtitleBackgroundOpacitySetting;
                prof["SubtitleShadowOpacitySetting"] = src.SubtitleShadowOpacitySetting;
                prof["FOVSetting"] = src.FOVSetting;
                prof["VehicleFOVSetting"] = src.VehicleFOVSetting;
                prof["CrosshairLocation"] = src.CrosshairLocation;
                prof["LookControlsInverted"] = src.LookControlsInverted;
                prof["MouseLookControlsInverted"] = src.MouseLookControlsInverted;
                prof["VibrationDisabled"] = src.VibrationDisabled;
                prof["ImpulseTriggersDisabled"] = src.ImpulseTriggersDisabled;
                prof["AircraftControlsInverted"] = src.AircraftControlsInverted;
                prof["MouseAircraftControlsInverted"] = src.MouseAircraftControlsInverted;
                prof["AutoCenterEnabled"] = src.AutoCenterEnabled;
                prof["CrouchLockEnabled"] = src.CrouchLockEnabled;
                prof["MKCrouchLockEnabled"] = src.MKCrouchLockEnabled;
                prof["ClenchProtectionEnabled"] = src.ClenchProtectionEnabled;
                prof["UseFemaleVoice"] = src.UseFemaleVoice;
                prof["HoldToZoom"] = src.HoldToZoom;
                prof["PlayerModelPrimaryColorIndex"] = src.PlayerModelPrimaryColorIndex;
                prof["PlayerModelSecondaryColorIndex"] = src.PlayerModelSecondaryColorIndex;
                prof["PlayerModelTertiaryColorIndex"] = src.PlayerModelTertiaryColorIndex;
                prof["UseEliteModel"] = src.UseEliteModel;
                prof["LockMaxAspectRatio"] = src.LockMaxAspectRatio;
                prof["un"] = src.un;
                prof["UsersSkinsEnabled"] = src.UsersSkinsEnabled;
                prof["PlayerModelPermutation"] = src.PlayerModelPermutation;
                prof["HelmetIndex"] = src.HelmetIndex;
                prof["LeftShoulderIndex"] = src.LeftShoulderIndex;
                prof["RightShoulderIndex"] = src.RightShoulderIndex;
                prof["ChestIndex"] = src.ChestIndex;
                prof["WristIndex"] = src.WristIndex;
                prof["UtilityIndex"] = src.UtilityIndex;
                prof["ArmsIndex"] = src.ArmsIndex;
                prof["LegsIndex"] = src.LegsIndex;
                prof["BackpackIndex"] = src.BackpackIndex;
                prof["SpartanBodyIndex"] = src.SpartanBodyIndex;
                prof["SpartanArmorEffectIndex"] = src.SpartanArmorEffectIndex;
                prof["KneesIndex"] = src.KneesIndex;
                prof["VisorColorIndex"] = src.VisorColorIndex;
                prof["EliteHelmetIndex"] = src.EliteHelmetIndex;
                prof["EliteLeftShoulderIndex"] = src.EliteLeftShoulderIndex;
                prof["EliteRightShoulderIndex"] = src.EliteRightShoulderIndex;
                prof["EliteChestIndex"] = src.EliteChestIndex;
                prof["EliteArmsIndex"] = src.EliteArmsIndex;
                prof["EliteLegsIndex"] = src.EliteLegsIndex;
                prof["EliteArmorIndex"] = src.EliteArmorIndex;
                prof["EliteArmorEffectIndex"] = src.EliteArmorEffectIndex;
                prof["VoiceIndex"] = src.VoiceIndex;
                prof["PlayerModelPrimaryColor"] = src.PlayerModelPrimaryColor;
                prof["PlayerModelSecondaryColor"] = src.PlayerModelSecondaryColor;
                prof["PlayerModelTertiaryColor"] = src.PlayerModelTertiaryColor;
                prof["SpartanPose"] = src.SpartanPose;
                prof["ElitePose"] = src.ElitePose;
                //TODO: implement Skin_t parameter
                char buf[5] = {};
                wcstombs(buf, src.ServiceTag, 4);
                prof["ServiceTag"] = std::string(buf, strnlen(buf, 4));
                prof["OnlineMedalFlasher"] = src.OnlineMedalFlasher;
                prof["VerticalLookSensitivity"] = src.VerticalLookSensitivity;
                prof["HorizontalLookSensitivity"] = src.HorizontalLookSensitivity;
                prof["LookAcceleration"] = src.LookAcceleration;
                prof["LookAxialDeadZone"] = src.LookAxialDeadZone;
                prof["LookRadialDeadZone"] = src.LookRadialDeadZone;
                prof["ZoomLookSensitivityMultiplier"] = src.ZoomLookSensitivityMultiplier;
                prof["VehicleLookSensitivityMultiplier"] = src.VehicleLookSensitivityMultiplier;
                prof["ButtonPreset"] = src.ButtonPreset;
                prof["StickPreset"] = src.StickPreset;
                prof["LeftyToggle"] = src.LeftyToggle;
                prof["FlyingCameraTurnSensitivity"] = src.FlyingCameraTurnSensitivity;
                prof["FlyingCameraPanning"] = src.FlyingCameraPanning;
                prof["FlyingCameraSpeed"] = src.FlyingCameraSpeed;
                prof["FlyingCameraThrust"] = src.FlyingCameraThrust;
                prof["TheaterTurnSensitivity"] = src.TheaterTurnSensitivity;
                prof["TheaterPanning"] = src.TheaterPanning;
                prof["TheaterSpeed"] = src.TheaterSpeed;
                prof["TheaterThrust"] = src.TheaterThrust;
                prof["MKTheaterTurnSensitivity"] = src.MKTheaterTurnSensitivity;
                prof["MKTheaterPanning"] = src.MKTheaterPanning;
                prof["MKTheaterSpeed"] = src.MKTheaterSpeed;
                prof["MKTheaterThrust"] = src.MKTheaterThrust;
                prof["SwapTriggersAndBumpers"] = src.SwapTriggersAndBumpers;
                prof["UseModernAimControl"] = src.UseModernAimControl;
                prof["UseDoublePressJumpToJetpack"] = src.UseDoublePressJumpToJetpack;
                prof["DualWieldInverted"] = src.DualWieldInverted;
                prof["ControllerDualWieldInverted"] = src.ControllerDualWieldInverted;
                prof["ControllerHornetControlJoystick"] = src.ControllerHornetControlJoystick;
                prof["ControllerBansheeTrickButtonsSwapped"] = src.ControllerBansheeTrickButtonsSwapped;
                prof["ColorCorrection"] = src.ColorCorrection;
                prof["EnemyPlayerNameColor"] = src.EnemyPlayerNameColor;
                prof["GameEngineTimer"] = src.GameEngineTimer;

                j["profile"][std::to_string(i)] = {
                    {"id", g_Profiles[i].id},
                    {"controller_index", g_Profiles[i].controller_index},
                    {"name", name},
                    {"profile", prof}
                };
            }

            std::ofstream file(path, std::ios::trunc);
            if (!file.is_open())
                return false;

            file << j.dump(4);
            return true;
        }

    void Splitscreen::ApplyToRuntime() {
        auto p = AlphaRing::Global::MCC::Splitscreen();
        if (!p)
            return;

        p->b_override            = g_Config.b_override;
        p->player_count          = g_Config.player_count;
        p->b_use_player0_profile = g_Config.b_use_player0_profile;
        p->b_player0_use_km      = g_Config.b_player0_use_km;
        p->b_override_profile    = g_Config.b_override_profile;
    }

    void Profile::ApplyToRuntime() {
        for (int i = 0; i < 4; ++i) {
            auto dst = CGameManager::get_profile(i);
            auto& src = g_Profiles[i];

            dst->controller_index = src.controller_index;
            dst->id = src.id;
            wcscpy_s(dst->name, src.name);
            dst->profile = src.profile;
        }
    }

    void Splitscreen::CaptureFromRuntime() {
        auto p = AlphaRing::Global::MCC::Splitscreen();
        if (!p)
            return;

        g_Config.b_override            = p->b_override;
        g_Config.player_count          = p->player_count;
        g_Config.b_use_player0_profile = p->b_use_player0_profile;
        g_Config.b_player0_use_km      = p->b_player0_use_km;
        g_Config.b_override_profile    = p->b_override_profile;
    }

    void Profile::CaptureFromRuntime() {
        for (int i = 0; i < 4; ++i) {
            auto dst = CGameManager::get_profile(i); 
            auto& src = g_Profiles[i];            

            dst->controller_index = src.controller_index;
            dst->id = src.id;
            wcscpy_s(dst->name, _countof(dst->name), src.name);
            dst->profile = src.profile;
        }
    }
}