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

            if (!j.contains("profile_t"))
                return false;

            auto& p = j["profile_t"];
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
                auto& dst2 = g_Profiles[i].mapping;

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
                auto& skinArray = prof["Skins"];
                for (size_t o = 0; o < skinArray.size() && o < 32; ++o) {
                    auto& s = skinArray[o];
                    dst.Skins[o].object = s.value("object", dst.Skins[o].object);
                    dst.Skins[o].skin   = s.value("skin", dst.Skins[o].skin);
                }
                const std::string s = prof.value("ServiceTag", "");
                wmemset(dst.ServiceTag, 0, 4);
                if (!s.empty()) {
                    mbstowcs_s(nullptr, dst.ServiceTag, 4, s.c_str(), _TRUNCATE);
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
                auto& loadoutArray = prof["LoadoutSlots"];
                for (size_t o = 0; o < loadoutArray.size() && o < 5; ++o) {
                    auto& slot = loadoutArray[o];
                    dst.LoadoutSlots[o].TacticalPackageIndex    = slot.value("TacticalPackageIndex", dst.LoadoutSlots[o].TacticalPackageIndex);
                    dst.LoadoutSlots[o].SupportUpgradeIndex     = slot.value("SupportUpgradeIndex", dst.LoadoutSlots[o].SupportUpgradeIndex);
                    dst.LoadoutSlots[o].PrimaryWeaponIndex      = slot.value("PrimaryWeaponIndex", dst.LoadoutSlots[o].PrimaryWeaponIndex);
                    dst.LoadoutSlots[o].SecondaryWeaponIndex    = slot.value("SecondaryWeaponIndex", dst.LoadoutSlots[o].SecondaryWeaponIndex);
                    dst.LoadoutSlots[o].PrimaryWeaponVariantIndex   = slot.value("PrimaryWeaponVariantIndex", dst.LoadoutSlots[o].PrimaryWeaponVariantIndex);
                    dst.LoadoutSlots[o].SecondaryWeaponVariantIndex = slot.value("SecondaryWeaponVariantIndex", dst.LoadoutSlots[o].SecondaryWeaponVariantIndex);
                    dst.LoadoutSlots[o].EquipmentIndex          = slot.value("EquipmentIndex", dst.LoadoutSlots[o].EquipmentIndex);
                    dst.LoadoutSlots[o].GrenadeIndex            = slot.value("GrenadeIndex", dst.LoadoutSlots[o].GrenadeIndex);
                    std::string name = slot.value("Name", "");
                    wmemset(dst.LoadoutSlots[o].Name, 0, 14);
                    mbstowcs(dst.LoadoutSlots[o].Name, name.c_str(), 14);
                }
                std::string gs = prof.value("GameSpecific", std::string{});
                std::memset(dst.GameSpecific, 0, sizeof(dst.GameSpecific));
                std::strncpy(dst.GameSpecific, gs.c_str(), sizeof(dst.GameSpecific) - 1);
                dst.MouseSensitivity = prof.value("MouseSensitivity", dst.MouseSensitivity);
                dst.MouseSmoothing = prof.value("MouseSmoothing", dst.MouseSmoothing);
                dst.MouseAcceleration = prof.value("MouseAcceleration", dst.MouseAcceleration);
                dst.PixelPerfectHudScale = prof.value("PixelPerfectHudScale", dst.PixelPerfectHudScale);
                dst.MouseAccelerationMinRate = prof.value("MouseAccelerationMinRate", dst.MouseAccelerationMinRate);
                dst.MouseAccelerationMaxAccel = prof.value("MouseAccelerationMaxAccel", dst.MouseAccelerationMaxAccel);
                dst.MouseAccelerationScale = prof.value("MouseAccelerationScale", dst.MouseAccelerationScale);
                dst.MouseAccelerationExp = prof.value("MouseAccelerationExp", dst.MouseAccelerationExp);
                dst.KeyboardMouseButtonPreset = prof.value("KeyboardMouseButtonPreset", dst.KeyboardMouseButtonPreset);
                auto& mappings = prof["CustomKeyboardMouseMappingV2"];
                for (size_t o = 0; o < mappings.size() && o < 66; ++o) {
                    auto& srcMap = mappings[o];
                    auto& dstMap = dst.CustomKeyboardMouseMappingV2[o];
                    dstMap.AbstractButton = srcMap.value("AbstractButton", dstMap.AbstractButton);
                    auto& keys = srcMap["VirtualKeyCodes"];
                    for (size_t k = 0; k < keys.size() && k < 5; ++k) {
                        dstMap.VirtualKeyCodes[k] = keys[k].get<int>();
                    }
                }
                dst.MasterVolume = prof.value("MasterVolume", dst.MasterVolume);
                dst.MusicVolume = prof.value("MusicVolume", dst.MusicVolume);
                dst.SfxVolume = prof.value("SfxVolume", dst.SfxVolume);
                std::string buf = prof.value("buffer4", std::string{});
                std::memset(dst.buffer4, 0, sizeof(dst.buffer4));
                std::strncpy(dst.buffer4, buf.c_str(), sizeof(dst.buffer4) - 1);
                dst.Brightness = prof.value("Brightness", dst.Brightness);
                auto& offsets = prof["WeaponDisplayOffset"];
                for (size_t o = 0; o < offsets.size() && o < 5; ++o) {
                    auto& off = offsets[o];
                    dst.WeaponDisplayOffset[o].x = off.value("x", dst.WeaponDisplayOffset[o].x);
                    dst.WeaponDisplayOffset[o].y = off.value("y", dst.WeaponDisplayOffset[o].y);
                    dst.WeaponDisplayOffset[o].z = off.value("z", dst.WeaponDisplayOffset[o].z);
                }
                dst.ColorBlindMode = prof.value("ColorBlindMode", dst.ColorBlindMode);
                dst.ColorBlindStrength = prof.value("ColorBlindStrength", dst.ColorBlindStrength);
                dst.ColorBlindBrightness = prof.value("ColorBlindBrightness", dst.ColorBlindBrightness);
                dst.ColorBlindContrast = prof.value("ColorBlindContrast", dst.ColorBlindContrast);
                dst.RemasteredHUDSetting = prof.value("RemasteredHUDSetting", dst.RemasteredHUDSetting);
                dst.HUDScale = prof.value("HUDScale", dst.HUDScale);
                auto& actionsJson = entry["mapping"]["actions"];
                for (size_t a = 0; a < actionsJson.size() && a < 66; ++a) {
                    dst2.actions[a] = static_cast<CGamepadMapping::eButton>(actionsJson[a].get<int>());
                }
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
                json mapp;
                auto& src = g_Profiles[i].profile;
                auto& src2 = g_Profiles[i].mapping;

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
                json skinArray = json::array();
                for (int o = 0; o < 32; ++o) {
                    json skin;
                    skin["object"] = src.Skins[o].object;
                    skin["skin"]   = src.Skins[o].skin;
                    skinArray.push_back(skin);
                }
                prof["Skins"] = skinArray;
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
                json loadoutArray = json::array();
                for (int o = 0; o < 5; ++o) {
                    json slot;
                    slot["TacticalPackageIndex"]    = src.LoadoutSlots[o].TacticalPackageIndex;
                    slot["SupportUpgradeIndex"]     = src.LoadoutSlots[o].SupportUpgradeIndex;
                    slot["PrimaryWeaponIndex"]      = src.LoadoutSlots[o].PrimaryWeaponIndex;
                    slot["SecondaryWeaponIndex"]    = src.LoadoutSlots[o].SecondaryWeaponIndex;
                    slot["PrimaryWeaponVariantIndex"]   = src.LoadoutSlots[o].PrimaryWeaponVariantIndex;
                    slot["SecondaryWeaponVariantIndex"] = src.LoadoutSlots[o].SecondaryWeaponVariantIndex;
                    slot["EquipmentIndex"]          = src.LoadoutSlots[o].EquipmentIndex;
                    slot["GrenadeIndex"]            = src.LoadoutSlots[o].GrenadeIndex;

                    char buf[15] = {};
                    wcstombs(buf, src.LoadoutSlots[o].Name, 14);
                    slot["Name"] = std::string(buf);

                    loadoutArray.push_back(slot);
                }
                prof["LoadoutSlots"] = loadoutArray;
                prof["GameSpecific"] = std::string(src.GameSpecific, strnlen(src.GameSpecific, sizeof(src.GameSpecific)));
                prof["MouseSensitivity"] = src.MouseSensitivity;
                prof["MouseSmoothing"] = src.MouseSmoothing;
                prof["MouseAcceleration"] = src.MouseAcceleration;
                prof["PixelPerfectHudScale"] = src.PixelPerfectHudScale;
                prof["MouseAccelerationMinRate"] = src.MouseAccelerationMinRate;
                prof["MouseAccelerationMaxAccel"] = src.MouseAccelerationMaxAccel;
                prof["MouseAccelerationScale"] = src.MouseAccelerationScale;
                prof["MouseAccelerationExp"] = src.MouseAccelerationExp;
                prof["KeyboardMouseButtonPreset"] = src.KeyboardMouseButtonPreset;
                json customKeyboardMouseMappingArray = json::array();
                for (int o = 0; o < 66; ++o) {
                    json cKMMObj;
                    cKMMObj["AbstractButton"] = src.CustomKeyboardMouseMappingV2[o].AbstractButton;
                    json virtualKeyCodesArray = json::array();
                    for (int k = 0; k < 5; ++k) {
                        virtualKeyCodesArray.push_back(src.CustomKeyboardMouseMappingV2[o].VirtualKeyCodes[k]);
                    }
                    cKMMObj["VirtualKeyCodes"] = virtualKeyCodesArray;
                    customKeyboardMouseMappingArray.push_back(cKMMObj);
                }
                prof["CustomKeyboardMouseMappingV2"] = customKeyboardMouseMappingArray;
                prof["MasterVolume"] = src.MasterVolume;
                prof["MusicVolume"] = src.MusicVolume;
                prof["SfxVolume"] = src.SfxVolume;
                prof["buffer4"] = std::string(src.buffer4, strnlen(src.buffer4, sizeof(src.buffer4)));
                prof["Brightness"] = src.Brightness;
                json weaponDisplayOffsetArray = json::array();
                for (int o = 0; o < 5; ++o) {
                    json offset;
                    offset["x"] = src.WeaponDisplayOffset[o].x;
                    offset["y"] = src.WeaponDisplayOffset[o].y;
                    offset["z"] = src.WeaponDisplayOffset[o].z;
                    weaponDisplayOffsetArray.push_back(offset);
                }
                prof["WeaponDisplayOffset"] = weaponDisplayOffsetArray;
                prof["ColorBlindMode"] = src.ColorBlindMode;
                prof["ColorBlindStrength"] = src.ColorBlindStrength;
                prof["ColorBlindBrightness"] = src.ColorBlindBrightness;
                prof["ColorBlindContrast"] = src.ColorBlindContrast;
                prof["RemasteredHUDSetting"] = src.RemasteredHUDSetting;
                prof["HUDScale"] = src.HUDScale;
                json mappingJson = json::array();
                for (int a = 0; a < 66; ++a) {
                    mappingJson.push_back(static_cast<int>(src2.actions[a]));
                }
                mapp["actions"] = mappingJson;

                j["profile_t"][std::to_string(i)] = {
                    {"id", g_Profiles[i].id},
                    {"controller_index", g_Profiles[i].controller_index},
                    {"name", name},
                    {"profile", prof},
                    {"mapping", mapp}
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
            wcscpy_s(dst->name, _countof(dst->name), src.name);
            dst->profile = src.profile;
            dst->mapping = src.mapping;
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
            auto src = CGameManager::get_profile(i);
            auto& dst = g_Profiles[i];

            dst.controller_index = src->controller_index;
            dst.id = src->id;
            wcscpy_s(dst.name, src->name);
            memcpy(&dst.profile, &src->profile, sizeof(dst.profile));
            memcpy(&dst.mapping, &src->mapping, sizeof(dst.mapping));
        }
    }

    void Profile::Initialize(CGameManager* mng)
    {
        pGameManager = mng;

        for (int i = 0; i < 4; ++i)
        {
            //sets up id, controller_index, name, profile and mapping
            auto& dst = g_Profiles[i];
            auto runtime = CGameManager::get_profile(i);

            dst.controller_index = runtime->controller_index;
            dst.id = runtime->id;
            wcscpy_s(dst.name, _countof(dst.name), runtime-> name);
            __int64 xuid = CGameManager::get_xuid(i);
        }
    }

}