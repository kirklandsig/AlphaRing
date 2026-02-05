#include "mcc/settings/Settings.h"
#include "global/Global.h"
#include "nlohmann/json.hpp"
#include "log/Log.h"

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
                dst.DialogueColorStyleSetting = prof.value("DialogueColorStyleSetting", dst.DialogueColorStyleSetting);
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
                if (prof.contains("Skins") && prof["Skins"].is_array()) {
                    auto& skinArray = prof["Skins"];
                    for (size_t o = 0; o < skinArray.size() && o < 32; ++o) {
                        auto& s = skinArray[o];
                        dst.Skins[o].object = s.value("object", dst.Skins[o].object);
                        dst.Skins[o].skin   = s.value("skin", dst.Skins[o].skin);
                    }
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
                if (prof.contains("LoadoutSlots") && prof["LoadoutSlots"].is_array()) {
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
                if (prof.contains("CustomKeyboardMouseMappingV2") && prof["CustomKeyboardMouseMappingV2"].is_array()) {
                    auto& mappings = prof["CustomKeyboardMouseMappingV2"];
                    for (size_t o = 0; o < mappings.size() && o < 66; ++o) {
                        auto& srcMap = mappings[o];
                        auto& dstMap = dst.CustomKeyboardMouseMappingV2[o];
                        dstMap.AbstractButton = srcMap.value("AbstractButton", dstMap.AbstractButton);
                        if (srcMap.contains("VirtualKeyCodes") && srcMap["VirtualKeyCodes"].is_array()) {
                            auto& keys = srcMap["VirtualKeyCodes"];
                            for (size_t k = 0; k < keys.size() && k < 5; ++k) {
                                dstMap.VirtualKeyCodes[k] = keys[k].get<int>();
                            }
                        }
                    }
                }
                dst.MasterVolume = prof.value("MasterVolume", dst.MasterVolume);
                dst.MusicVolume = prof.value("MusicVolume", dst.MusicVolume);
                dst.SfxVolume = prof.value("SfxVolume", dst.SfxVolume);
                std::string buf = prof.value("buffer4", std::string{});
                std::memset(dst.buffer4, 0, sizeof(dst.buffer4));
                std::strncpy(dst.buffer4, buf.c_str(), sizeof(dst.buffer4) - 1);
                dst.Brightness = prof.value("Brightness", dst.Brightness);
                if (prof.contains("WeaponDisplayOffset") && prof["WeaponDisplayOffset"].is_array()) {
                    auto& offsets = prof["WeaponDisplayOffset"];
                    for (size_t o = 0; o < offsets.size() && o < 5; ++o) {
                        auto& off = offsets[o];
                        dst.WeaponDisplayOffset[o].x = off.value("x", dst.WeaponDisplayOffset[o].x);
                        dst.WeaponDisplayOffset[o].y = off.value("y", dst.WeaponDisplayOffset[o].y);
                        dst.WeaponDisplayOffset[o].z = off.value("z", dst.WeaponDisplayOffset[o].z);
                    }
                }
                dst.ColorBlindMode = prof.value("ColorBlindMode", dst.ColorBlindMode);
                dst.ColorBlindStrength = prof.value("ColorBlindStrength", dst.ColorBlindStrength);
                dst.ColorBlindBrightness = prof.value("ColorBlindBrightness", dst.ColorBlindBrightness);
                dst.ColorBlindContrast = prof.value("ColorBlindContrast", dst.ColorBlindContrast);
                dst.RemasteredHUDSetting = prof.value("RemasteredHUDSetting", dst.RemasteredHUDSetting);
                dst.HUDScale = prof.value("HUDScale", dst.HUDScale);
                if (entry.contains("mapping") && entry["mapping"].contains("actions") && entry["mapping"]["actions"].is_array()) {
                    auto& actionsJson = entry["mapping"]["actions"];
                    for (size_t a = 0; a < actionsJson.size() && a < 66; ++a) {
                        dst2.actions[a] = static_cast<CGamepadMapping::eButton>(actionsJson[a].get<int>());
                    }
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
            LOG_INFO("Profile::Save starting...");
            fs::path path = GetConfigPath();
            LOG_DEBUG("Profile::Save: Path = {}", path.string());

            json j;
            if (fs::exists(path)) {
                std::ifstream file(path);
                if (file.is_open()) {
                    try {
                        file >> j;
                        LOG_DEBUG("Profile::Save: Loaded existing settings file");
                    } catch (const std::exception& e) {
                        LOG_WARNING("Profile::Save: Failed to parse existing file: {}", e.what());
                        j = json::object();
                    }
                }
            }

            for (int i = 0; i < 4; ++i) {
                LOG_DEBUG("Profile::Save: Processing player {}", i);
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
                // ServiceTag - ensure null termination before conversion
                wchar_t tagCopy[5] = {};
                memcpy(tagCopy, src.ServiceTag, sizeof(src.ServiceTag));
                tagCopy[4] = L'\0';
                char buf[5] = {};
                wcstombs(buf, tagCopy, 4);
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

                    // Ensure null termination before conversion
                    wchar_t nameCopy[15] = {};
                    memcpy(nameCopy, src.LoadoutSlots[o].Name, sizeof(src.LoadoutSlots[o].Name));
                    nameCopy[14] = L'\0';
                    char nameBuf[15] = {};
                    wcstombs(nameBuf, nameCopy, 14);
                    slot["Name"] = std::string(nameBuf);

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
            if (!file.is_open()) {
                LOG_ERROR("Profile::Save: Failed to open file for writing");
                return false;
            }

            file << j.dump(4);
            LOG_INFO("Profile::Save: Successfully saved all profiles");
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
        LOG_INFO("Profile::CaptureFromRuntime starting...");
        for (int i = 0; i < 4; ++i) {
            LOG_DEBUG("CaptureFromRuntime: Processing player {}", i);
            auto src = CGameManager::get_profile(i);
            if (!src) {
                LOG_ERROR("CaptureFromRuntime: get_profile({}) returned null!", i);
                continue;
            }
            auto& dst = g_Profiles[i];

            dst.controller_index = src->controller_index;
            dst.id = src->id;
            wcscpy_s(dst.name, src->name);
            dst.profile.SubtitleSetting = src->profile.SubtitleSetting;
            dst.profile.SubtitleSizeSetting = src->profile.SubtitleSizeSetting;
            dst.profile.SubtitleBackgroundSetting = src->profile.SubtitleBackgroundSetting;
            dst.profile.SubtitleShadowColorSetting = src->profile.SubtitleShadowColorSetting;
            dst.profile.DialogueColorStyleSetting = src->profile.DialogueColorStyleSetting;
            dst.profile.DialogueColorSetting = src->profile.DialogueColorSetting;
            dst.profile.DialoguePaletteSetting = src->profile.DialoguePaletteSetting;
            dst.profile.SpeakerSetting = src->profile.SpeakerSetting;
            dst.profile.SpeakerColorStyleSetting = src->profile.SpeakerColorStyleSetting;
            dst.profile.SpeakerColorSetting = src->profile.SpeakerColorSetting;
            dst.profile.SpeakerPaletteSetting = src->profile.SpeakerPaletteSetting;
            dst.profile.SubtitleFontSetting = src->profile.SubtitleFontSetting;
            dst.profile.SubtitleBackgroundOpacitySetting = src->profile.SubtitleBackgroundOpacitySetting;
            dst.profile.SubtitleShadowOpacitySetting = src->profile.SubtitleShadowOpacitySetting;
            dst.profile.FOVSetting = src->profile.FOVSetting;
            dst.profile.VehicleFOVSetting = src->profile.VehicleFOVSetting;
            dst.profile.CrosshairLocation = src->profile.CrosshairLocation;
            dst.profile.LookControlsInverted = src->profile.LookControlsInverted;
            dst.profile.MouseLookControlsInverted = src->profile.MouseLookControlsInverted;
            dst.profile.VibrationDisabled = src->profile.VibrationDisabled;
            dst.profile.ImpulseTriggersDisabled = src->profile.ImpulseTriggersDisabled;
            dst.profile.AircraftControlsInverted = src->profile.AircraftControlsInverted;
            dst.profile.MouseAircraftControlsInverted = src->profile.MouseAircraftControlsInverted;
            dst.profile.AutoCenterEnabled = src->profile.AutoCenterEnabled;
            dst.profile.CrouchLockEnabled = src->profile.CrouchLockEnabled;
            dst.profile.MKCrouchLockEnabled = src->profile.MKCrouchLockEnabled;
            dst.profile.ClenchProtectionEnabled = src->profile.ClenchProtectionEnabled;
            dst.profile.UseFemaleVoice = src->profile.UseFemaleVoice;
            dst.profile.HoldToZoom = src->profile.HoldToZoom;
            dst.profile.PlayerModelPrimaryColorIndex = src->profile.PlayerModelPrimaryColorIndex;
            dst.profile.PlayerModelSecondaryColorIndex = src->profile.PlayerModelSecondaryColorIndex;
            dst.profile.PlayerModelTertiaryColorIndex = src->profile.PlayerModelTertiaryColorIndex;
            dst.profile.UseEliteModel = src->profile.UseEliteModel;
            dst.profile.LockMaxAspectRatio = src->profile.LockMaxAspectRatio;
            dst.profile.un = src->profile.un;
            dst.profile.UsersSkinsEnabled = src->profile.UsersSkinsEnabled;
            dst.profile.PlayerModelPermutation = src->profile.PlayerModelPermutation;
            dst.profile.HelmetIndex = src->profile.HelmetIndex;
            dst.profile.LeftShoulderIndex = src->profile.LeftShoulderIndex;
            dst.profile.RightShoulderIndex = src->profile.RightShoulderIndex;
            dst.profile.ChestIndex = src->profile.ChestIndex;
            dst.profile.WristIndex = src->profile.WristIndex;
            dst.profile.UtilityIndex = src->profile.UtilityIndex;
            dst.profile.ArmsIndex = src->profile.ArmsIndex;
            dst.profile.LegsIndex = src->profile.LegsIndex;
            dst.profile.BackpackIndex = src->profile.BackpackIndex;
            dst.profile.SpartanBodyIndex = src->profile.SpartanBodyIndex;
            dst.profile.SpartanArmorEffectIndex = src->profile.SpartanArmorEffectIndex;
            dst.profile.KneesIndex = src->profile.KneesIndex;
            dst.profile.VisorColorIndex = src->profile.VisorColorIndex;
            dst.profile.EliteHelmetIndex = src->profile.EliteHelmetIndex;
            dst.profile.EliteLeftShoulderIndex = src->profile.EliteLeftShoulderIndex;
            dst.profile.EliteRightShoulderIndex = src->profile.EliteRightShoulderIndex;
            dst.profile.EliteChestIndex = src->profile.EliteChestIndex;
            dst.profile.EliteArmsIndex = src->profile.EliteArmsIndex;
            dst.profile.EliteLegsIndex = src->profile.EliteLegsIndex;
            dst.profile.EliteArmorIndex = src->profile.EliteArmorIndex;
            dst.profile.EliteArmorEffectIndex = src->profile.EliteArmorEffectIndex;
            dst.profile.VoiceIndex = src->profile.VoiceIndex;
            dst.profile.PlayerModelPrimaryColor = src->profile.PlayerModelPrimaryColor;
            dst.profile.PlayerModelSecondaryColor = src->profile.PlayerModelSecondaryColor;
            dst.profile.PlayerModelTertiaryColor = src->profile.PlayerModelTertiaryColor;
            dst.profile.SpartanPose = src->profile.SpartanPose;
            dst.profile.ElitePose = src->profile.ElitePose;
            memcpy(dst.profile.Skins, src->profile.Skins, sizeof(dst.profile.Skins));
            memcpy(dst.profile.ServiceTag, src->profile.ServiceTag, sizeof(dst.profile.ServiceTag));
            dst.profile.OnlineMedalFlasher = src->profile.OnlineMedalFlasher;
            dst.profile.VerticalLookSensitivity = src->profile.VerticalLookSensitivity;
            dst.profile.HorizontalLookSensitivity = src->profile.HorizontalLookSensitivity;
            dst.profile.LookAcceleration = src->profile.LookAcceleration;
            dst.profile.LookAxialDeadZone = src->profile.LookAxialDeadZone;
            dst.profile.LookRadialDeadZone = src->profile.LookRadialDeadZone;
            dst.profile.ZoomLookSensitivityMultiplier = src->profile.ZoomLookSensitivityMultiplier;
            dst.profile.VehicleLookSensitivityMultiplier = src->profile.VehicleLookSensitivityMultiplier;
            dst.profile.ButtonPreset = src->profile.ButtonPreset;
            dst.profile.StickPreset = src->profile.StickPreset;
            dst.profile.LeftyToggle = src->profile.LeftyToggle;
            dst.profile.FlyingCameraTurnSensitivity = src->profile.FlyingCameraTurnSensitivity;
            dst.profile.FlyingCameraPanning = src->profile.FlyingCameraPanning;
            dst.profile.FlyingCameraSpeed = src->profile.FlyingCameraSpeed;
            dst.profile.FlyingCameraThrust = src->profile.FlyingCameraThrust;
            dst.profile.TheaterTurnSensitivity = src->profile.TheaterTurnSensitivity;
            dst.profile.TheaterPanning = src->profile.TheaterPanning;
            dst.profile.TheaterSpeed = src->profile.TheaterSpeed;
            dst.profile.TheaterThrust = src->profile.TheaterThrust;
            dst.profile.MKTheaterTurnSensitivity = src->profile.MKTheaterTurnSensitivity;
            dst.profile.MKTheaterPanning = src->profile.MKTheaterPanning;
            dst.profile.MKTheaterSpeed = src->profile.MKTheaterSpeed;
            dst.profile.MKTheaterThrust = src->profile.MKTheaterThrust;
            dst.profile.SwapTriggersAndBumpers = src->profile.SwapTriggersAndBumpers;
            dst.profile.UseModernAimControl = src->profile.UseModernAimControl;
            dst.profile.UseDoublePressJumpToJetpack = src->profile.UseDoublePressJumpToJetpack;
            dst.profile.DualWieldInverted = src->profile.DualWieldInverted;
            dst.profile.ControllerDualWieldInverted = src->profile.ControllerDualWieldInverted;
            dst.profile.ControllerHornetControlJoystick = src->profile.ControllerHornetControlJoystick;
            dst.profile.ControllerBansheeTrickButtonsSwapped = src->profile.ControllerBansheeTrickButtonsSwapped;
            dst.profile.ColorCorrection = src->profile.ColorCorrection;
            dst.profile.EnemyPlayerNameColor = src->profile.EnemyPlayerNameColor;
            dst.profile.GameEngineTimer = src->profile.GameEngineTimer;
            memcpy(dst.profile.LoadoutSlots, src->profile.LoadoutSlots, sizeof(dst.profile.LoadoutSlots));
            memcpy(dst.profile.GameSpecific, src->profile.GameSpecific, sizeof(dst.profile.GameSpecific));
            dst.profile.MouseSensitivity = src->profile.MouseSensitivity;
            dst.profile.MouseSmoothing = src->profile.MouseSmoothing;
            dst.profile.MouseAcceleration = src->profile.MouseAcceleration;
            dst.profile.PixelPerfectHudScale = src->profile.PixelPerfectHudScale;
            dst.profile.MouseAccelerationMinRate = src->profile.MouseAccelerationMinRate;
            dst.profile.MouseAccelerationMaxAccel = src->profile.MouseAccelerationMaxAccel;
            dst.profile.MouseAccelerationScale = src->profile.MouseAccelerationScale;
            dst.profile.MouseAccelerationExp = src->profile.MouseAccelerationExp;
            dst.profile.KeyboardMouseButtonPreset = src->profile.KeyboardMouseButtonPreset;
            memcpy(dst.profile.CustomKeyboardMouseMappingV2, src->profile.CustomKeyboardMouseMappingV2, sizeof(dst.profile.CustomKeyboardMouseMappingV2));
            dst.profile.MasterVolume = src->profile.MasterVolume;
            dst.profile.MusicVolume = src->profile.MusicVolume;
            dst.profile.SfxVolume = src->profile.SfxVolume;
            memcpy(dst.profile.buffer4, src->profile.buffer4, sizeof(dst.profile.buffer4));
            dst.profile.Brightness = src->profile.Brightness;
            memcpy(dst.profile.WeaponDisplayOffset, src->profile.WeaponDisplayOffset, sizeof(dst.profile.WeaponDisplayOffset));
            dst.profile.ColorBlindMode = src->profile.ColorBlindMode;
            dst.profile.ColorBlindStrength = src->profile.ColorBlindStrength;
            dst.profile.ColorBlindBrightness = src->profile.ColorBlindBrightness;
            dst.profile.ColorBlindContrast = src->profile.ColorBlindContrast;
            dst.profile.RemasteredHUDSetting = src->profile.RemasteredHUDSetting;
            dst.profile.HUDScale = src->profile.HUDScale;
            memcpy(&dst.mapping, &src->mapping, sizeof(dst.mapping));
            LOG_DEBUG("CaptureFromRuntime: Player {} done", i);
        }
        LOG_INFO("Profile::CaptureFromRuntime complete");
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

    // Custom Mapping Profiles
    static fs::path GetCustomMappingsPath() {
        char exePath[MAX_PATH] = {0};
        if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH))
            return "custom_mappings.json";

        fs::path dir = fs::path(exePath).parent_path();
        return dir / "custom_mappings.json";
    }

    std::vector<std::string> CustomMapping::GetProfileNames() {
        std::vector<std::string> names;
        fs::path path = GetCustomMappingsPath();

        if (!fs::exists(path))
            return names;

        std::ifstream file(path);
        if (!file.is_open())
            return names;

        try {
            json j;
            file >> j;

            if (j.contains("profiles") && j["profiles"].is_object()) {
                for (auto& [key, value] : j["profiles"].items()) {
                    names.push_back(key);
                }
            }
        } catch (...) {
            // Invalid JSON, return empty
        }

        return names;
    }

    bool CustomMapping::SaveProfile(const std::string& name, const CGamepadMapping& mapping) {
        fs::path path = GetCustomMappingsPath();

        json j;
        // Load existing if present
        if (fs::exists(path)) {
            std::ifstream file(path);
            if (file.is_open()) {
                try {
                    file >> j;
                } catch (...) {
                    j = json::object();
                }
            }
        }

        // Ensure profiles object exists
        if (!j.contains("profiles") || !j["profiles"].is_object()) {
            j["profiles"] = json::object();
        }

        // Save the mapping actions array
        json actionsArray = json::array();
        for (int i = 0; i < 66; ++i) {
            actionsArray.push_back(static_cast<int>(mapping.actions[i]));
        }

        j["profiles"][name] = {
            {"actions", actionsArray}
        };

        std::ofstream file(path, std::ios::trunc);
        if (!file.is_open())
            return false;

        file << j.dump(4);
        return true;
    }

    bool CustomMapping::LoadProfile(const std::string& name, CGamepadMapping& mapping) {
        fs::path path = GetCustomMappingsPath();

        if (!fs::exists(path))
            return false;

        std::ifstream file(path);
        if (!file.is_open())
            return false;

        try {
            json j;
            file >> j;

            if (!j.contains("profiles") || !j["profiles"].contains(name))
                return false;

            auto& profile = j["profiles"][name];
            if (!profile.contains("actions"))
                return false;

            auto& actionsArray = profile["actions"];
            for (size_t i = 0; i < actionsArray.size() && i < 66; ++i) {
                mapping.actions[i] = static_cast<CGamepadMapping::eButton>(actionsArray[i].get<int>());
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    bool CustomMapping::DeleteProfile(const std::string& name) {
        fs::path path = GetCustomMappingsPath();

        if (!fs::exists(path))
            return false;

        std::ifstream infile(path);
        if (!infile.is_open())
            return false;

        json j;
        try {
            infile >> j;
        } catch (...) {
            return false;
        }
        infile.close();

        if (!j.contains("profiles") || !j["profiles"].contains(name))
            return false;

        j["profiles"].erase(name);

        std::ofstream outfile(path, std::ios::trunc);
        if (!outfile.is_open())
            return false;

        outfile << j.dump(4);
        return true;
    }

    // Custom Profile Presets (armor, colors, sensitivities, etc.)
    static fs::path GetCustomProfilesPath() {
        char exePath[MAX_PATH] = {0};
        if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH))
            return "custom_profiles.json";

        fs::path dir = fs::path(exePath).parent_path();
        return dir / "custom_profiles.json";
    }

    std::vector<std::string> CustomProfile::GetProfileNames() {
        std::vector<std::string> names;
        fs::path path = GetCustomProfilesPath();

        if (!fs::exists(path))
            return names;

        std::ifstream file(path);
        if (!file.is_open())
            return names;

        try {
            json j;
            file >> j;

            if (j.contains("profiles") && j["profiles"].is_object()) {
                for (auto& [key, value] : j["profiles"].items()) {
                    names.push_back(key);
                }
            }
        } catch (...) {
            // Invalid JSON, return empty
        }

        return names;
    }

    bool CustomProfile::SaveProfile(const std::string& name, const CUserProfile& src) {
        LOG_INFO("SaveProfile: Starting save for preset '{}'", name);

        try {
            fs::path path = GetCustomProfilesPath();
            LOG_DEBUG("SaveProfile: Path = {}", path.string());

            json j;
            // Load existing if present
            if (fs::exists(path)) {
                std::ifstream file(path);
                if (file.is_open()) {
                    try {
                        file >> j;
                        LOG_DEBUG("SaveProfile: Loaded existing profiles file");
                    } catch (const std::exception& e) {
                        LOG_WARNING("SaveProfile: Failed to parse existing file: {}", e.what());
                        j = json::object();
                    }
                }
            }

            // Ensure profiles object exists
            if (!j.contains("profiles") || !j["profiles"].is_object()) {
                j["profiles"] = json::object();
            }

            LOG_DEBUG("SaveProfile: Building profile JSON...");

            // Build profile JSON
            json prof;

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

        LOG_DEBUG("SaveProfile: Saving basic fields done, starting Skins array...");

        // Skins array
        json skinArray = json::array();
        for (int i = 0; i < 32; ++i) {
            json skin;
            skin["object"] = src.Skins[i].object;
            skin["skin"] = src.Skins[i].skin;
            skinArray.push_back(skin);
        }
        prof["Skins"] = skinArray;

        LOG_DEBUG("SaveProfile: Skins done, converting ServiceTag...");

        // ServiceTag - ensure null termination before conversion (src.ServiceTag is wchar_t[4], may not be null-terminated)
        wchar_t tagCopy[5] = {};
        memcpy(tagCopy, src.ServiceTag, sizeof(src.ServiceTag));
        tagCopy[4] = L'\0';
        char buf[5] = {};
        wcstombs(buf, tagCopy, 4);
        prof["ServiceTag"] = std::string(buf, strnlen(buf, 4));
        LOG_DEBUG("SaveProfile: ServiceTag = '{}'", buf);

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

        LOG_DEBUG("SaveProfile: Starting LoadoutSlots array...");

        // LoadoutSlots array
        json loadoutArray = json::array();
        for (int i = 0; i < 5; ++i) {
            LOG_DEBUG("SaveProfile: LoadoutSlot[{}]...", i);
            json slot;
            slot["TacticalPackageIndex"] = src.LoadoutSlots[i].TacticalPackageIndex;
            slot["SupportUpgradeIndex"] = src.LoadoutSlots[i].SupportUpgradeIndex;
            slot["PrimaryWeaponIndex"] = src.LoadoutSlots[i].PrimaryWeaponIndex;
            slot["SecondaryWeaponIndex"] = src.LoadoutSlots[i].SecondaryWeaponIndex;
            slot["PrimaryWeaponVariantIndex"] = src.LoadoutSlots[i].PrimaryWeaponVariantIndex;
            slot["SecondaryWeaponVariantIndex"] = src.LoadoutSlots[i].SecondaryWeaponVariantIndex;
            slot["EquipmentIndex"] = src.LoadoutSlots[i].EquipmentIndex;
            slot["GrenadeIndex"] = src.LoadoutSlots[i].GrenadeIndex;
            // Ensure null termination before conversion (Name is wchar_t[14], may not be null-terminated)
            wchar_t nameCopy[15] = {};
            memcpy(nameCopy, src.LoadoutSlots[i].Name, sizeof(src.LoadoutSlots[i].Name));
            nameCopy[14] = L'\0';
            char nameBuf[15] = {};
            wcstombs(nameBuf, nameCopy, 14);
            slot["Name"] = std::string(nameBuf);
            loadoutArray.push_back(slot);
        }
        prof["LoadoutSlots"] = loadoutArray;
        LOG_DEBUG("SaveProfile: LoadoutSlots done");

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

        // CustomKeyboardMouseMappingV2 array
        json kbmArray = json::array();
        for (int i = 0; i < 66; ++i) {
            json kbm;
            kbm["AbstractButton"] = src.CustomKeyboardMouseMappingV2[i].AbstractButton;
            json vkArray = json::array();
            for (int k = 0; k < 5; ++k) {
                vkArray.push_back(src.CustomKeyboardMouseMappingV2[i].VirtualKeyCodes[k]);
            }
            kbm["VirtualKeyCodes"] = vkArray;
            kbmArray.push_back(kbm);
        }
        prof["CustomKeyboardMouseMappingV2"] = kbmArray;

        prof["MasterVolume"] = src.MasterVolume;
        prof["MusicVolume"] = src.MusicVolume;
        prof["SfxVolume"] = src.SfxVolume;
        prof["buffer4"] = std::string(src.buffer4, strnlen(src.buffer4, sizeof(src.buffer4)));
        prof["Brightness"] = src.Brightness;

        // WeaponDisplayOffset array
        json wdoArray = json::array();
        for (int i = 0; i < 5; ++i) {
            json offset;
            offset["x"] = src.WeaponDisplayOffset[i].x;
            offset["y"] = src.WeaponDisplayOffset[i].y;
            offset["z"] = src.WeaponDisplayOffset[i].z;
            wdoArray.push_back(offset);
        }
        prof["WeaponDisplayOffset"] = wdoArray;

        prof["ColorBlindMode"] = src.ColorBlindMode;
        prof["ColorBlindStrength"] = src.ColorBlindStrength;
        prof["ColorBlindBrightness"] = src.ColorBlindBrightness;
        prof["ColorBlindContrast"] = src.ColorBlindContrast;
        prof["RemasteredHUDSetting"] = src.RemasteredHUDSetting;
        prof["HUDScale"] = src.HUDScale;

        LOG_DEBUG("SaveProfile: All fields done, writing to file...");

        j["profiles"][name] = prof;

        std::ofstream file(path, std::ios::trunc);
        if (!file.is_open()) {
            LOG_ERROR("SaveProfile: Failed to open file for writing: {}", path.string());
            return false;
        }

        file << j.dump(4);
        LOG_INFO("SaveProfile: Successfully saved preset '{}'", name);
        return true;

        } catch (const std::exception& e) {
            LOG_ERROR("SaveProfile: Exception caught: {}", e.what());
            return false;
        } catch (...) {
            LOG_ERROR("SaveProfile: Unknown exception caught");
            return false;
        }
    }

    bool CustomProfile::LoadProfile(const std::string& name, CUserProfile& dst) {
        fs::path path = GetCustomProfilesPath();

        if (!fs::exists(path))
            return false;

        std::ifstream file(path);
        if (!file.is_open())
            return false;

        try {
            json j;
            file >> j;

            if (!j.contains("profiles") || !j["profiles"].contains(name))
                return false;

            auto& prof = j["profiles"][name];

            dst.SubtitleSetting = prof.value("SubtitleSetting", dst.SubtitleSetting);
            dst.SubtitleSizeSetting = prof.value("SubtitleSizeSetting", dst.SubtitleSizeSetting);
            dst.SubtitleBackgroundSetting = prof.value("SubtitleBackgroundSetting", dst.SubtitleBackgroundSetting);
            dst.SubtitleShadowColorSetting = prof.value("SubtitleShadowColorSetting", dst.SubtitleShadowColorSetting);
            dst.DialogueColorStyleSetting = prof.value("DialogueColorStyleSetting", dst.DialogueColorStyleSetting);
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

            // Skins array
            if (prof.contains("Skins") && prof["Skins"].is_array()) {
                auto& skinArray = prof["Skins"];
                for (size_t i = 0; i < skinArray.size() && i < 32; ++i) {
                    auto& s = skinArray[i];
                    dst.Skins[i].object = s.value("object", dst.Skins[i].object);
                    dst.Skins[i].skin = s.value("skin", dst.Skins[i].skin);
                }
            }

            // ServiceTag
            const std::string st = prof.value("ServiceTag", "");
            wmemset(dst.ServiceTag, 0, 4);
            if (!st.empty()) {
                mbstowcs_s(nullptr, dst.ServiceTag, 4, st.c_str(), _TRUNCATE);
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

            // LoadoutSlots array
            if (prof.contains("LoadoutSlots") && prof["LoadoutSlots"].is_array()) {
                auto& loadoutArray = prof["LoadoutSlots"];
                for (size_t i = 0; i < loadoutArray.size() && i < 5; ++i) {
                    auto& slot = loadoutArray[i];
                    dst.LoadoutSlots[i].TacticalPackageIndex = slot.value("TacticalPackageIndex", dst.LoadoutSlots[i].TacticalPackageIndex);
                    dst.LoadoutSlots[i].SupportUpgradeIndex = slot.value("SupportUpgradeIndex", dst.LoadoutSlots[i].SupportUpgradeIndex);
                    dst.LoadoutSlots[i].PrimaryWeaponIndex = slot.value("PrimaryWeaponIndex", dst.LoadoutSlots[i].PrimaryWeaponIndex);
                    dst.LoadoutSlots[i].SecondaryWeaponIndex = slot.value("SecondaryWeaponIndex", dst.LoadoutSlots[i].SecondaryWeaponIndex);
                    dst.LoadoutSlots[i].PrimaryWeaponVariantIndex = slot.value("PrimaryWeaponVariantIndex", dst.LoadoutSlots[i].PrimaryWeaponVariantIndex);
                    dst.LoadoutSlots[i].SecondaryWeaponVariantIndex = slot.value("SecondaryWeaponVariantIndex", dst.LoadoutSlots[i].SecondaryWeaponVariantIndex);
                    dst.LoadoutSlots[i].EquipmentIndex = slot.value("EquipmentIndex", dst.LoadoutSlots[i].EquipmentIndex);
                    dst.LoadoutSlots[i].GrenadeIndex = slot.value("GrenadeIndex", dst.LoadoutSlots[i].GrenadeIndex);
                    std::string slotName = slot.value("Name", "");
                    wmemset(dst.LoadoutSlots[i].Name, 0, 14);
                    mbstowcs(dst.LoadoutSlots[i].Name, slotName.c_str(), 14);
                }
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

            // CustomKeyboardMouseMappingV2 array
            if (prof.contains("CustomKeyboardMouseMappingV2") && prof["CustomKeyboardMouseMappingV2"].is_array()) {
                auto& kbmArray = prof["CustomKeyboardMouseMappingV2"];
                for (size_t i = 0; i < kbmArray.size() && i < 66; ++i) {
                    auto& kbm = kbmArray[i];
                    dst.CustomKeyboardMouseMappingV2[i].AbstractButton = kbm.value("AbstractButton", dst.CustomKeyboardMouseMappingV2[i].AbstractButton);
                    if (kbm.contains("VirtualKeyCodes") && kbm["VirtualKeyCodes"].is_array()) {
                        auto& vkArray = kbm["VirtualKeyCodes"];
                        for (size_t k = 0; k < vkArray.size() && k < 5; ++k) {
                            dst.CustomKeyboardMouseMappingV2[i].VirtualKeyCodes[k] = vkArray[k].get<int>();
                        }
                    }
                }
            }

            dst.MasterVolume = prof.value("MasterVolume", dst.MasterVolume);
            dst.MusicVolume = prof.value("MusicVolume", dst.MusicVolume);
            dst.SfxVolume = prof.value("SfxVolume", dst.SfxVolume);

            std::string b4 = prof.value("buffer4", std::string{});
            std::memset(dst.buffer4, 0, sizeof(dst.buffer4));
            std::strncpy(dst.buffer4, b4.c_str(), sizeof(dst.buffer4) - 1);

            dst.Brightness = prof.value("Brightness", dst.Brightness);

            // WeaponDisplayOffset array
            if (prof.contains("WeaponDisplayOffset") && prof["WeaponDisplayOffset"].is_array()) {
                auto& wdoArray = prof["WeaponDisplayOffset"];
                for (size_t i = 0; i < wdoArray.size() && i < 5; ++i) {
                    auto& off = wdoArray[i];
                    dst.WeaponDisplayOffset[i].x = off.value("x", dst.WeaponDisplayOffset[i].x);
                    dst.WeaponDisplayOffset[i].y = off.value("y", dst.WeaponDisplayOffset[i].y);
                    dst.WeaponDisplayOffset[i].z = off.value("z", dst.WeaponDisplayOffset[i].z);
                }
            }

            dst.ColorBlindMode = prof.value("ColorBlindMode", dst.ColorBlindMode);
            dst.ColorBlindStrength = prof.value("ColorBlindStrength", dst.ColorBlindStrength);
            dst.ColorBlindBrightness = prof.value("ColorBlindBrightness", dst.ColorBlindBrightness);
            dst.ColorBlindContrast = prof.value("ColorBlindContrast", dst.ColorBlindContrast);
            dst.RemasteredHUDSetting = prof.value("RemasteredHUDSetting", dst.RemasteredHUDSetting);
            dst.HUDScale = prof.value("HUDScale", dst.HUDScale);

            return true;
        } catch (...) {
            return false;
        }
    }

    bool CustomProfile::DeleteProfile(const std::string& name) {
        fs::path path = GetCustomProfilesPath();

        if (!fs::exists(path))
            return false;

        std::ifstream infile(path);
        if (!infile.is_open())
            return false;

        json j;
        try {
            infile >> j;
        } catch (...) {
            return false;
        }
        infile.close();

        if (!j.contains("profiles") || !j["profiles"].contains(name))
            return false;

        j["profiles"].erase(name);

        std::ofstream outfile(path, std::ios::trunc);
        if (!outfile.is_open())
            return false;

        outfile << j.dump(4);
        return true;
    }

}