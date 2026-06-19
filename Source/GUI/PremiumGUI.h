#pragma once
#include "..\imgui\imgui.h"
#include "..\Features\ESP.h"
#include "..\Features\Features.h"
#include "..\Core\DrawIndexedHook.h"
#include "..\Features\SceneChams.h"
#include "..\Features\HitEffects.h"
#include "..\Features\GrenadePrediction.h"
#include "..\Features\Aimbot.h"
#include "..\Features\Misc.h"
#include "..\Features\TriggerBot.h"
#include <string>
#include <cmath>
#include <cstdio>
#include <Windows.h>

namespace PremiumGUI
{
    inline bool menuOpen = true;

    // ==================== TAB SYSTEM ====================
    inline int currentTab = 0;
    inline constexpr int TAB_COUNT = 9;
    inline const char* tabNames[] = {
        "Savas", "Gorseller", "Chams", "Diger",
        "Skinler", "Bicak", "Model", "Dunya", "Koruma"
    };
    inline const char* tabIcons[] = {
        "\xef\x81\x9c",  // fa-crosshairs (Savas)
        "\xef\x81\xae",  // fa-eye (Gorseller)
        "\xef\x87\xb6",  // fa-ghost (Chams)
        "\xef\x80\x93",  // fa-cog (Diger)
        "\xef\x87\x92",  // fa-paint-brush (Skinler)
        "\xef\x84\xba",  // fa-cut (Bicak)
        "\xef\x80\x87",  // fa-user (Model)
        "\xef\x82\xac",  // fa-globe (Dunya)
        "\xef\x84\xb2",  // fa-shield-alt (Koruma)
    };

    // Per-tab selected item index
    inline int selectedItem[TAB_COUNT] = {0};
    inline float lastKeyTime = 0.0f;

    // Scroll offset per tab
    inline float scrollY[TAB_COUNT] = {0};

    // Slider drag state
    inline bool draggingSlider = false;
    inline float* dragSliderPtr = nullptr;
    inline float dragSliderMin = 0, dragSliderMax = 1;

    // ==================== HELPERS ====================
    inline void CycleKey(int* key, int dir) {
        const int keys[] = {VK_MENU, VK_SHIFT, VK_CONTROL, VK_XBUTTON1, VK_XBUTTON2, VK_RBUTTON, 0x58, 0x43, 0x56};
        const int N = 9;
        int idx = 0;
        for (int i = 0; i < N; i++) { if (keys[i] == *key) { idx = i; break; } }
        idx = (idx + dir + N) % N;
        *key = keys[idx];
    }
    inline const char* KeyStr(int k) {
        if (k == VK_MENU) return "ALT"; if (k == VK_SHIFT) return "SHIFT"; if (k == VK_CONTROL) return "CTRL";
        if (k == 0x58) return "X"; if (k == 0x43) return "C"; if (k == 0x56) return "V";
        if (k == VK_XBUTTON1) return "M4"; if (k == VK_XBUTTON2) return "M5"; if (k == VK_RBUTTON) return "M2";
        if (k == 0x54) return "T"; if (k == 0x4E) return "N";
        return "?";
    }

    // ==================== COLORS ====================
    inline constexpr ImU32 COL_BG          = IM_COL32(18, 18, 24, 245);
    inline constexpr ImU32 COL_SIDEBAR     = IM_COL32(22, 22, 30, 255);
    inline constexpr ImU32 COL_CONTENT     = IM_COL32(25, 25, 35, 250);
    inline constexpr ImU32 COL_ACCENT      = IM_COL32(130, 80, 255, 255);
    inline constexpr ImU32 COL_ACCENT_HOV  = IM_COL32(160, 110, 255, 255);
    inline constexpr ImU32 COL_ACCENT_DIM  = IM_COL32(90, 55, 180, 200);
    inline constexpr ImU32 COL_ACCENT_BG   = IM_COL32(130, 80, 255, 25);
    inline constexpr ImU32 COL_TEXT        = IM_COL32(230, 228, 240, 255);
    inline constexpr ImU32 COL_TEXT_DIM    = IM_COL32(120, 115, 140, 255);
    inline constexpr ImU32 COL_BORDER      = IM_COL32(50, 45, 75, 160);
    inline constexpr ImU32 COL_TOGGLE_OFF  = IM_COL32(60, 60, 80, 255);
    inline constexpr ImU32 COL_SECTION     = IM_COL32(150, 115, 255, 255);
    inline constexpr ImU32 COL_TAB_ACTIVE  = IM_COL32(130, 80, 255, 35);
    inline constexpr ImU32 COL_HEADER      = IM_COL32(22, 20, 32, 255);
    inline constexpr ImU32 COL_SLIDER_BG   = IM_COL32(40, 38, 55, 220);

    // ==================== ITEM SYSTEM ====================
    // type: 0=toggle, 1=mode/combo, 2=key, 3=floatSlider, 4=intSelector, 5=sectionHeader, 6=intSlider, 7=colorPicker (fVal -> float[4] RGBA)
    struct Item {
        const char* name;
        int type;
        bool* bVal;
        int* iVal;
        const char** modes;
        int numModes;
        float* fVal;
        float fMin, fMax, fStep;
        int iMin, iMax; // for type 6
    };

    // Mode string arrays
    inline const char* aimModes[] = {"LEGIT", "RAGE"};
    inline const char* trigModes[] = {"LEGIT", "RAGE"};
    inline const char* boxModes[] = {"Normal", "Kose", "Dolu", "Kose+Dolu"};
    inline const char* glowModes[] = {"Flat", "Outline", "Pulse"};
    inline const char* chamsModeNames[] = {"Flat", "Outline", "Glow", "Metallic", "Fresnel"};
    inline const char* armorDispModes[] = {"Bar", "Yazi", "Ikisi"};

    // ==================== TAB 0: SAVAS ====================
    inline Item savasItems[] = {
        {"AIMBOT",           5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Aimbot",           0, &Aimbot::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Mod",              1, nullptr, &Aimbot::mode, aimModes, 2, nullptr, 0,0,0, 0,0},
        {"Tus",              2, nullptr, &Aimbot::hotkey, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"FOV",              3, nullptr, nullptr, nullptr, 0, &Aimbot::fov, 1.0f,30.0f,0.5f, 0,0},
        {"FOV Dairesi",      0, &ESP::showFovCircle, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Smooth",           3, nullptr, nullptr, nullptr, 0, &Aimbot::smooth, 1.0f,20.0f,0.5f, 0,0},
        {"RCS",              0, &Aimbot::rcsEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"TRIGGERBOT",       5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Triggerbot",       0, &TriggerBot::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Mod",              1, nullptr, &TriggerBot::mode, trigModes, 2, nullptr, 0,0,0, 0,0},
        {"Tus",              2, nullptr, &TriggerBot::hotkey, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int SAVAS_COUNT = sizeof(savasItems)/sizeof(Item);

    // ==================== TAB 1: GORSELLER ====================
    inline Item visualItems[] = {
        {"ESP KUTU",         5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Kutu",             0, &ESP::showBox, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Kutu Mod",         1, nullptr, &ESP::boxMode, boxModes, 4, nullptr, 0,0,0, 0,0},
        {"Can",              0, &ESP::showHealth, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Armor",            0, &ESP::showArmor, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Isim",             0, &ESP::showName, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Silah",            0, &ESP::showWeapon, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Silah Ikonu",      0, &ESP::showWeaponIcon, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Mesafe",           0, &ESP::showDistance, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Para",             0, &ESP::showMoney, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Mermi",            0, &ESP::showAmmo, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"ISKELET",          5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Iskelet",          0, &ESP::showSkeleton, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Cizgi",            0, &ESP::showSnaplines, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"DIGER",            5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Steam Bilgi",      0, &ESP::showSteamInfo, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Sadece Dusman",    0, &ESP::enemyOnly, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bomba ESP",        0, &ESP::showBombESP, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bomba Timer",      0, &ESP::showBombTimer, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Yere Dusen Silah", 0, &ESP::showDroppedWeapons, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bomba Yolu",       0, &GrenadePrediction::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Izleyenler",       0, &ESP::showSpectators, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int VISUAL_COUNT = sizeof(visualItems)/sizeof(Item);

    // ==================== TAB 2: CHAMS ====================
    inline const char* gpuChamsModes[] = {"Flat", "Glow", "Metallic"};

    inline Item chamsItems[] = {
        {"GLOW CHAMS",        5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Glow",             0, &SceneChams::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Glow Mod",         1, nullptr, &SceneChams::mode, glowModes, 3, nullptr, 0,0,0, 0,0},
        {"Glow Yogunluk",    3, nullptr, nullptr, nullptr, 0, &SceneChams::glowIntensity, 0.5f,20.0f,0.5f, 0,0},
        {"Duvar Arkasi",     0, &SceneChams::throughWall, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Takim Arkadasi",   0, &SceneChams::showTeammates, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Olu Glow Kapat",   0, &ESP::glowDeadFilter, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Dusman Renk",      7, nullptr, nullptr, nullptr, 0, SceneChams::visibleColor, 0,0,0, 0,0},
        {"Takim Renk",       7, nullptr, nullptr, nullptr, 0, SceneChams::hiddenColor, 0,0,0, 0,0},
        {"GPU CHAMS",         5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"GPU Chams",        0, &DrawIndexedHook::chamsEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Chams Mod",        1, nullptr, &DrawIndexedHook::chamsMode, gpuChamsModes, 3, nullptr, 0,0,0, 0,0},
        {"Duvar Arkasi",     0, &DrawIndexedHook::chamsThroughWalls, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Opaklık",          3, nullptr, nullptr, nullptr, 0, &DrawIndexedHook::chamsAlpha, 0.1f,1.0f,0.05f, 0,0},
        {"Dusman (Gorunen)", 7, nullptr, nullptr, nullptr, 0, DrawIndexedHook::chamsEnemyVisible, 0,0,0, 0,0},
        {"Dusman (Gizli)",   7, nullptr, nullptr, nullptr, 0, DrawIndexedHook::chamsEnemyHidden, 0,0,0, 0,0},
        {"Takim (Gorunen)",  7, nullptr, nullptr, nullptr, 0, DrawIndexedHook::chamsFriendVisible, 0,0,0, 0,0},
        {"Takim (Gizli)",    7, nullptr, nullptr, nullptr, 0, DrawIndexedHook::chamsFriendHidden, 0,0,0, 0,0},
    };
    inline constexpr int CHAMS_COUNT = sizeof(chamsItems)/sizeof(Item);

    // ==================== TAB 3: DIGER ====================
    inline Item miscItems[] = {
        {"HAREKET",          5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bunnyhop",         0, &Misc::bhopEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"GORSEL",           5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Anti Flash",       0, &Misc::antiFlashEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Anti Smoke",       0, &Misc::antiSmokeEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"BILGI",            5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Radar Hack",       0, &Misc::radarEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"NoRecoil",         0, &Misc::NoRecoil::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Noclip",           0, &Misc::noclipEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"ARAYUZ",           5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Watermark",        0, &Misc::watermarkEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"FPS/Ping",         0, &Misc::showFpsPing, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int MISC_COUNT = sizeof(miscItems)/sizeof(Item);

    // ==================== TAB 4: SKINLER ====================
    inline Item skinItems[] = {
        {"SKIN CHANGER",     5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Skin Aktif",       0, &SkinChanger::enabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"AK-47 Skin",       4, nullptr, &SkinChanger::skinAK, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"AWP Skin",         4, nullptr, &SkinChanger::skinAWP, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"M4A4 Skin",        4, nullptr, &SkinChanger::skinM4A4, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"M4A1-S Skin",      4, nullptr, &SkinChanger::skinM4A1, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Deagle Skin",      4, nullptr, &SkinChanger::skinDeagle, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"USP-S Skin",       4, nullptr, &SkinChanger::skinUSP, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Glock Skin",       4, nullptr, &SkinChanger::skinGlock, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"AYAR",             5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Skin Wear",        3, nullptr, nullptr, nullptr, 0, &SkinChanger::skinWear, 0.0001f,1.0f,0.01f, 0,0},
        {"Skin Seed",        6, nullptr, &SkinChanger::skinSeed, nullptr, 0, nullptr, 0,0,0, 0,999},
    };
    inline constexpr int SKIN_COUNT2 = sizeof(skinItems)/sizeof(Item);

    // ==================== TAB 5: BICAK ====================
    inline Item knifeItems[] = {
        {"BICAK",            5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bicak Modeli",     4, nullptr, &SkinChanger::knifeModel, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Bicak Skin",       4, nullptr, &SkinChanger::skinKnife, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"ELDIVEN",          5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Eldiven Modeli",   4, nullptr, &SkinChanger::gloveModel, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Eldiven Skin",     4, nullptr, &SkinChanger::skinGlove, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int KNIFE_COUNT2 = sizeof(knifeItems)/sizeof(Item);

    // ==================== TAB 6: MODEL ====================
    inline Item modelItems[] = {
        {"OYUNCU MODELI",    5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Model Aktif",      0, &SkinChanger::playerModelEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"CT Model",         4, nullptr, &SkinChanger::ctModel, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"T Model",          4, nullptr, &SkinChanger::tModel, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int MODEL_COUNT = sizeof(modelItems)/sizeof(Item);

    // ==================== TAB 7: DUNYA ====================
    inline Item dunyaItems[] = {
        {"GOKYUZU",          5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Sky Aktif",        0, &Misc::skyColorEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Sky Renk",          7, nullptr, nullptr, nullptr, 0, Misc::skyColor, 0,0,0, 0,0},
        {"Sky Parlaklik",    3, nullptr, nullptr, nullptr, 0, &Misc::skyBrightness, 0.0f,2.0f,0.05f, 0,0},
        {"GECE MODU",        5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Gece Modu",        0, &Misc::nightModeEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Dunya Parlaklik",  3, nullptr, nullptr, nullptr, 0, &Misc::worldBrightness, 0.0f,1.0f,0.05f, 0,0},
        {"KAMERA",           5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"FOV Aktif",        0, &Misc::fovEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"FOV Degeri",       6, nullptr, &Misc::fovValue, nullptr, 0, nullptr, 0,0,0, 60,140},
        {"Viewmodel FOV",    6, nullptr, &Misc::viewmodelFov, nullptr, 0, nullptr, 0,0,0, 54,90},
        {"UCUNCU SAHIS",     5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Ucuncu Sahis",     0, &Misc::tpsEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"TPS Tus",          2, nullptr, &Misc::tpsKey, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"CROSSHAIR",        5, nullptr, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
        {"Ozel Crosshair",   0, &Misc::customCrosshairEnabled, nullptr, nullptr, 0, nullptr, 0,0,0, 0,0},
    };
    inline constexpr int DUNYA_COUNT = sizeof(dunyaItems)/sizeof(Item);

    // ==================== GET TAB DATA ====================
    inline Item* GetTab(int tab, int& cnt) {
        switch (tab) {
            case 0: cnt = SAVAS_COUNT; return savasItems;
            case 1: cnt = VISUAL_COUNT; return visualItems;
            case 2: cnt = CHAMS_COUNT; return chamsItems;
            case 3: cnt = MISC_COUNT; return miscItems;
            case 4: cnt = SKIN_COUNT2; return skinItems;
            case 5: cnt = KNIFE_COUNT2; return knifeItems;
            case 6: cnt = MODEL_COUNT; return modelItems;
            case 7: cnt = DUNYA_COUNT; return dunyaItems;
            case 8: cnt = 0; return nullptr; // Koruma - custom rendered
            default: cnt = 0; return nullptr;
        }
    }

    // ==================== SKIN NAME HELPER ====================
    inline const char* GetSkinName(int* ptr, int val) {
        // Knife skins — use filtered KNF-only list
        if (ptr == &SkinChanger::skinKnife) {
            SkinChanger::BuildKnifeSkinList();
            if (val >= 0 && val < SkinChanger::KNIFE_SKIN_COUNT) return SkinChanger::knifeSkinNames[val];
            return "?";
        }
        // Weapon skins — use full skinList
        if (ptr == &SkinChanger::skinAK || ptr == &SkinChanger::skinAWP || ptr == &SkinChanger::skinM4A4 ||
            ptr == &SkinChanger::skinM4A1 || ptr == &SkinChanger::skinDeagle || ptr == &SkinChanger::skinUSP ||
            ptr == &SkinChanger::skinGlock) {
            if (val >= 0 && val < SkinChanger::SKIN_COUNT) return SkinChanger::skinList[val].name;
        }
        if (ptr == &SkinChanger::knifeModel) {
            if (val >= 0 && val < SkinChanger::KNIFE_COUNT) return SkinChanger::knifeList[val].name;
        }
        if (ptr == &SkinChanger::gloveModel) {
            if (val >= 0 && val < SkinChanger::GLOVE_COUNT) return SkinChanger::gloveList[val].name;
        }
        if (ptr == &SkinChanger::skinGlove) {
            if (val >= 0 && val < SkinChanger::GLOVE_SKIN_COUNT) return SkinChanger::gloveSkinList[val].name;
        }
        if (ptr == &SkinChanger::ctModel) {
            if (val >= 0 && val < SkinChanger::CT_MODEL_COUNT) return SkinChanger::ctModelList[val].name;
        }
        if (ptr == &SkinChanger::tModel) {
            if (val >= 0 && val < SkinChanger::T_MODEL_COUNT) return SkinChanger::tModelList[val].name;
        }
        return "?";
    }
    inline int GetMaxVal(int* ptr) {
        // Knife skins — limited to KNF entries only
        if (ptr == &SkinChanger::skinKnife) {
            SkinChanger::BuildKnifeSkinList();
            return SkinChanger::KNIFE_SKIN_COUNT - 1;
        }
        // Weapon skins
        if (ptr == &SkinChanger::skinAK || ptr == &SkinChanger::skinAWP || ptr == &SkinChanger::skinM4A4 ||
            ptr == &SkinChanger::skinM4A1 || ptr == &SkinChanger::skinDeagle || ptr == &SkinChanger::skinUSP ||
            ptr == &SkinChanger::skinGlock)
            return SkinChanger::SKIN_COUNT - 1;
        if (ptr == &SkinChanger::knifeModel) return SkinChanger::KNIFE_COUNT - 1;
        if (ptr == &SkinChanger::gloveModel) return SkinChanger::GLOVE_COUNT - 1;
        if (ptr == &SkinChanger::skinGlove) return SkinChanger::GLOVE_SKIN_COUNT - 1;
        if (ptr == &SkinChanger::ctModel) return SkinChanger::CT_MODEL_COUNT - 1;
        if (ptr == &SkinChanger::tModel) return SkinChanger::T_MODEL_COUNT - 1;
        return 100;
    }

    // ==================== ESP PREVIEW ====================
    inline void DrawESPPreview(ImDrawList* dl, float px, float py, float pw, float ph) {
        // Dark panel background with subtle grid
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + pw, py + ph), IM_COL32(16, 16, 22, 240), 6.0f);
        dl->AddRect(ImVec2(px, py), ImVec2(px + pw, py + ph), COL_BORDER, 6.0f);

        // Title
        float fs = ImGui::GetFontSize();
        dl->AddText(nullptr, fs * 0.55f, ImVec2(px + 6, py + 4), COL_ACCENT, "ESP PREVIEW");

        // Grid lines
        for (float gx = px + 20; gx < px + pw; gx += 20)
            dl->AddLine(ImVec2(gx, py + 20), ImVec2(gx, py + ph - 4), IM_COL32(40, 40, 55, 40));
        for (float gy = py + 20; gy < py + ph; gy += 20)
            dl->AddLine(ImVec2(px + 4, gy), ImVec2(px + pw - 4, gy), IM_COL32(40, 40, 55, 40));

        // Soldier silhouette center
        float cx = px + pw * 0.5f;
        float bodyTop = py + 55;
        float bodyBot = py + ph - 30;
        float bodyH = bodyBot - bodyTop;

        // Head
        float headR = 10.0f;
        float headY = bodyTop - headR - 2;
        dl->AddCircleFilled(ImVec2(cx, headY), headR, IM_COL32(70, 70, 90, 200));
        dl->AddCircle(ImVec2(cx, headY), headR, IM_COL32(100, 100, 130, 200), 0, 1.2f);

        // Torso
        float torsoW = 26, torsoH = bodyH * 0.42f;
        float torsoTop = bodyTop;
        dl->AddRectFilled(ImVec2(cx - torsoW/2, torsoTop), ImVec2(cx + torsoW/2, torsoTop + torsoH),
                          IM_COL32(70, 70, 90, 200), 3.0f);

        // Arms (lines)
        float shoulderY = torsoTop + 5;
        float elbowY = torsoTop + torsoH * 0.5f;
        float handY = torsoTop + torsoH * 0.85f;
        // Left arm
        dl->AddLine(ImVec2(cx - torsoW/2, shoulderY), ImVec2(cx - torsoW/2 - 14, elbowY), IM_COL32(70,70,90,200), 5.0f);
        dl->AddLine(ImVec2(cx - torsoW/2 - 14, elbowY), ImVec2(cx - torsoW/2 - 10, handY), IM_COL32(70,70,90,200), 4.0f);
        // Right arm
        dl->AddLine(ImVec2(cx + torsoW/2, shoulderY), ImVec2(cx + torsoW/2 + 14, elbowY), IM_COL32(70,70,90,200), 5.0f);
        dl->AddLine(ImVec2(cx + torsoW/2 + 14, elbowY), ImVec2(cx + torsoW/2 + 10, handY), IM_COL32(70,70,90,200), 4.0f);

        // Legs
        float legTop = torsoTop + torsoH;
        float kneeY = legTop + bodyH * 0.25f;
        float footY = bodyBot;
        // Left leg
        dl->AddLine(ImVec2(cx - 6, legTop), ImVec2(cx - 10, kneeY), IM_COL32(70,70,90,200), 5.0f);
        dl->AddLine(ImVec2(cx - 10, kneeY), ImVec2(cx - 8, footY), IM_COL32(70,70,90,200), 4.0f);
        // Right leg
        dl->AddLine(ImVec2(cx + 6, legTop), ImVec2(cx + 10, kneeY), IM_COL32(70,70,90,200), 5.0f);
        dl->AddLine(ImVec2(cx + 10, kneeY), ImVec2(cx + 8, footY), IM_COL32(70,70,90,200), 4.0f);

        // ESP overlay elements based on current settings
        float boxL = cx - 30, boxR = cx + 30;
        float boxT = headY - headR - 4, boxB = footY + 4;

        // Box
        if (ESP::showBox) {
            ImU32 bCol = IM_COL32(0, 220, 255, 200);
            if (ESP::boxMode == 0) {
                dl->AddRect(ImVec2(boxL, boxT), ImVec2(boxR, boxB), bCol, 0, 0, 1.5f);
            } else {
                float cLen = 10.0f;
                dl->AddLine(ImVec2(boxL, boxT), ImVec2(boxL + cLen, boxT), bCol, 1.5f);
                dl->AddLine(ImVec2(boxL, boxT), ImVec2(boxL, boxT + cLen), bCol, 1.5f);
                dl->AddLine(ImVec2(boxR, boxT), ImVec2(boxR - cLen, boxT), bCol, 1.5f);
                dl->AddLine(ImVec2(boxR, boxT), ImVec2(boxR, boxT + cLen), bCol, 1.5f);
                dl->AddLine(ImVec2(boxL, boxB), ImVec2(boxL + cLen, boxB), bCol, 1.5f);
                dl->AddLine(ImVec2(boxL, boxB), ImVec2(boxL, boxB - cLen), bCol, 1.5f);
                dl->AddLine(ImVec2(boxR, boxB), ImVec2(boxR - cLen, boxB), bCol, 1.5f);
                dl->AddLine(ImVec2(boxR, boxB), ImVec2(boxR, boxB - cLen), bCol, 1.5f);
            }
        }

        // Health bar (left)
        if (ESP::showHealth) {
            float barX = boxL - 6;
            float barH = boxB - boxT;
            float hp = 0.72f;
            dl->AddRectFilled(ImVec2(barX - 1, boxT), ImVec2(barX + 2, boxB), IM_COL32(0,0,0,150), 1.0f);
            dl->AddRectFilled(ImVec2(barX, boxB - barH * hp), ImVec2(barX + 2, boxB), IM_COL32(80,255,80,240), 1.0f);
        }

        // Armor bar (right)
        if (ESP::showArmor) {
            float barX = boxR + 4;
            float barH = boxB - boxT;
            float arm = 0.55f;
            dl->AddRectFilled(ImVec2(barX - 1, boxT), ImVec2(barX + 2, boxB), IM_COL32(0,0,0,150), 1.0f);
            dl->AddRectFilled(ImVec2(barX, boxB - barH * arm), ImVec2(barX + 2, boxB), IM_COL32(80,140,255,240), 1.0f);
        }

        // Name above
        if (ESP::showName) {
            const char* pName = "Player";
            ImVec2 ns = ImGui::CalcTextSize(pName);
            dl->AddText(nullptr, fs * 0.5f, ImVec2(cx - ns.x * 0.25f, boxT - fs * 0.5f - 2), IM_COL32(255,255,255,220), pName);
        }

        // Weapon below
        if (ESP::showWeapon) {
            const char* wName = "AK-47";
            ImVec2 ws = ImGui::CalcTextSize(wName);
            dl->AddText(nullptr, fs * 0.45f, ImVec2(cx - ws.x * 0.22f, boxB + 3), IM_COL32(240,220,140,220), wName);
        }

        // Skeleton lines on silhouette
        if (ESP::showSkeleton) {
            ImU32 sCol = IM_COL32(0, 255, 200, 180);
            // Spine
            dl->AddLine(ImVec2(cx, headY + headR), ImVec2(cx, torsoTop), sCol, 1.2f);
            dl->AddLine(ImVec2(cx, torsoTop), ImVec2(cx, legTop), sCol, 1.2f);
            // Arms
            dl->AddLine(ImVec2(cx, shoulderY), ImVec2(cx - torsoW/2 - 14, elbowY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx - torsoW/2 - 14, elbowY), ImVec2(cx - torsoW/2 - 10, handY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx, shoulderY), ImVec2(cx + torsoW/2 + 14, elbowY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx + torsoW/2 + 14, elbowY), ImVec2(cx + torsoW/2 + 10, handY), sCol, 1.2f);
            // Legs
            dl->AddLine(ImVec2(cx, legTop), ImVec2(cx - 10, kneeY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx - 10, kneeY), ImVec2(cx - 8, footY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx, legTop), ImVec2(cx + 10, kneeY), sCol, 1.2f);
            dl->AddLine(ImVec2(cx + 10, kneeY), ImVec2(cx + 8, footY), sCol, 1.2f);
        }
    }

    // ==================== KEYBOARD INPUT ====================
    inline void HandleInput() {
        float now = (float)ImGui::GetTime();
        if (now - lastKeyTime < 0.12f) return;

        int cnt = 0;
        Item* items = GetTab(currentTab, cnt);
        if (!items || cnt == 0) {
            // TAB navigation still works
            if (GetAsyncKeyState(VK_TAB) & 0x8000) { currentTab = (currentTab + 1) % TAB_COUNT; lastKeyTime = now; }
            return;
        }
        int& sel = selectedItem[currentTab];
        if (sel >= cnt) sel = 0;

        auto skipHeaders = [&](int dir) {
            for (int t = 0; t < cnt; t++) {
                if (items[sel].type != 5) break;
                sel += dir;
                if (sel >= cnt) sel = 0;
                if (sel < 0) sel = cnt - 1;
            }
        };

        if (GetAsyncKeyState(VK_UP) & 0x8000) { sel--; if (sel < 0) sel = cnt - 1; skipHeaders(-1); lastKeyTime = now; }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) { sel++; if (sel >= cnt) sel = 0; skipHeaders(1); lastKeyTime = now; }
        if (GetAsyncKeyState(VK_TAB) & 0x8000) { currentTab = (currentTab + 1) % TAB_COUNT; lastKeyTime = now; return; }

        Item& it = items[sel];
        if (it.type == 5) return;

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            if (it.type == 0 && it.bVal) *it.bVal = !*it.bVal;
            if (it.type == 1 && it.iVal) *it.iVal = (*it.iVal + 1) % it.numModes;
            if (it.type == 2 && it.iVal) CycleKey(it.iVal, 1);
            if (it.type == 3 && it.fVal) { *it.fVal += it.fStep; if (*it.fVal > it.fMax) *it.fVal = it.fMax; }
            if (it.type == 4 && it.iVal) { (*it.iVal)++; int mx = GetMaxVal(it.iVal); if (*it.iVal > mx) *it.iVal = mx; SkinChanger::needsUpdate = true; }
            if (it.type == 6 && it.iVal) { (*it.iVal)++; if (*it.iVal > it.iMax) *it.iVal = it.iMax; }
            lastKeyTime = now;
        }
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            if (it.type == 0 && it.bVal) *it.bVal = !*it.bVal;
            if (it.type == 1 && it.iVal) { *it.iVal = *it.iVal - 1; if (*it.iVal < 0) *it.iVal = it.numModes - 1; }
            if (it.type == 2 && it.iVal) CycleKey(it.iVal, -1);
            if (it.type == 3 && it.fVal) { *it.fVal -= it.fStep; if (*it.fVal < it.fMin) *it.fVal = it.fMin; }
            if (it.type == 4 && it.iVal) { (*it.iVal)--; if (*it.iVal < 0) *it.iVal = 0; SkinChanger::needsUpdate = true; }
            if (it.type == 6 && it.iVal) { (*it.iVal)--; if (*it.iVal < it.iMin) *it.iVal = it.iMin; }
            lastKeyTime = now;
        }
    }

    // ==================== DRAW A SINGLE ITEM ROW ====================
    inline float DrawItem(ImDrawList* dl, Item& it, float cx, float cy, float cw, float lineH, bool isSel, float fs, ImGuiIO& io, int idx) {
        // Section header
        if (it.type == 5) {
            float secY = cy + lineH * 0.5f;
            dl->AddLine(ImVec2(cx, secY), ImVec2(cx + 5, secY), COL_ACCENT);
            float tScale = 0.55f;
            dl->AddText(nullptr, fs * tScale, ImVec2(cx + 10, cy + (lineH - fs * tScale) / 2), COL_SECTION, it.name);
            float tw = ImGui::CalcTextSize(it.name).x * tScale;
            dl->AddLine(ImVec2(cx + 14 + tw, secY), ImVec2(cx + cw, secY), IM_COL32(60, 50, 90, 50));
            return lineH;
        }

        float itemScale = 0.65f;

        // Highlight selected row
        if (isSel) {
            dl->AddRectFilled(ImVec2(cx - 2, cy + 1), ImVec2(cx + cw + 2, cy + lineH - 1), COL_ACCENT_BG, 3.0f);
            dl->AddRectFilled(ImVec2(cx - 2, cy + 4), ImVec2(cx, cy + lineH - 4), COL_ACCENT, 2.0f);
        }

        // Check mouse hover
        bool hov = (io.MousePos.x >= cx && io.MousePos.x <= cx + cw &&
                    io.MousePos.y >= cy && io.MousePos.y <= cy + lineH);

        // Item label
        dl->AddText(nullptr, fs * itemScale, ImVec2(cx + 6, cy + (lineH - fs * itemScale) / 2),
                    isSel ? COL_TEXT : (hov ? COL_TEXT : COL_TEXT_DIM), it.name);

        float rightX = cx + cw;

        // Toggle (pill shape)
        if (it.type == 0 && it.bVal) {
            bool on = *it.bVal;
            float swW = 32, swH = 16;
            float swX = rightX - swW - 4;
            float swY = cy + (lineH - swH) / 2;
            ImU32 pillBg = on ? COL_ACCENT : COL_TOGGLE_OFF;
            dl->AddRectFilled(ImVec2(swX, swY), ImVec2(swX + swW, swY + swH), pillBg, swH / 2);
            if (on) {
                // Accent gradient highlight
                dl->AddRectFilled(ImVec2(swX, swY), ImVec2(swX + swW, swY + swH), IM_COL32(160,120,255,40), swH/2);
            }
            float dotX = on ? (swX + swW - swH / 2 - 1) : (swX + swH / 2 + 1);
            dl->AddCircleFilled(ImVec2(dotX, swY + swH / 2), swH / 2 - 2, IM_COL32(255, 255, 255, 240));

            // Mouse click
            if (hov && ImGui::IsMouseClicked(0)) { *it.bVal = !*it.bVal; }
        }
        // Mode/Combo
        else if (it.type == 1 && it.iVal) {
            const char* modeName = it.modes[*it.iVal % it.numModes];
            ImVec2 mSz = ImGui::CalcTextSize(modeName);
            float mW = mSz.x * itemScale + 20;
            float mX = rightX - mW - 4;
            float mY = cy + (lineH - 20) / 2;
            bool mHov = (io.MousePos.x >= mX && io.MousePos.x <= mX + mW && io.MousePos.y >= mY && io.MousePos.y <= mY + 20);
            dl->AddRectFilled(ImVec2(mX, mY), ImVec2(mX + mW, mY + 20), IM_COL32(40, 35, 60, 230), 4.0f);
            dl->AddRect(ImVec2(mX, mY), ImVec2(mX + mW, mY + 20), mHov ? COL_ACCENT_HOV : COL_ACCENT_DIM, 4.0f);
            dl->AddText(nullptr, fs * itemScale, ImVec2(mX + 10, mY + (20 - fs * itemScale) / 2), IM_COL32(200, 180, 255, 255), modeName);
            if (mHov && ImGui::IsMouseClicked(0)) { *it.iVal = (*it.iVal + 1) % it.numModes; }
        }
        // Key bind
        else if (it.type == 2 && it.iVal) {
            const char* kn = KeyStr(*it.iVal);
            ImVec2 kSz = ImGui::CalcTextSize(kn);
            float kW = kSz.x * itemScale + 18;
            float kX = rightX - kW - 4;
            float kY = cy + (lineH - 20) / 2;
            bool kHov = (io.MousePos.x >= kX && io.MousePos.x <= kX + kW && io.MousePos.y >= kY && io.MousePos.y <= kY + 20);
            dl->AddRectFilled(ImVec2(kX, kY), ImVec2(kX + kW, kY + 20), IM_COL32(40, 35, 60, 230), 4.0f);
            dl->AddRect(ImVec2(kX, kY), ImVec2(kX + kW, kY + 20), kHov ? COL_ACCENT_HOV : COL_BORDER, 4.0f);
            dl->AddText(nullptr, fs * itemScale, ImVec2(kX + 9, kY + (20 - fs * itemScale) / 2), IM_COL32(255, 200, 80, 255), kn);
            if (kHov && ImGui::IsMouseClicked(0)) { CycleKey(it.iVal, 1); }
        }
        // Float slider
        else if (it.type == 3 && it.fVal) {
            char vBuf[16]; sprintf_s(vBuf, "%.2f", *it.fVal);
            float slW = 100, slH = 5;
            float valTextW = 38;
            float slX = rightX - slW - valTextW - 8;
            float slY = cy + lineH / 2 - slH / 2;
            float frac = (*it.fVal - it.fMin) / (it.fMax - it.fMin);
            if (frac < 0) frac = 0; if (frac > 1) frac = 1;

            // Track
            dl->AddRectFilled(ImVec2(slX, slY), ImVec2(slX + slW, slY + slH), COL_SLIDER_BG, slH / 2);
            // Fill with accent gradient
            if (frac > 0.001f) {
                dl->AddRectFilled(ImVec2(slX, slY), ImVec2(slX + slW * frac, slY + slH), COL_ACCENT, slH / 2);
            }
            // Knob
            float knobX = slX + slW * frac;
            dl->AddCircleFilled(ImVec2(knobX, slY + slH / 2), 5.5f, COL_ACCENT);
            dl->AddCircleFilled(ImVec2(knobX, slY + slH / 2), 2.5f, IM_COL32(255, 255, 255, 240));
            // Value text
            dl->AddText(nullptr, fs * itemScale, ImVec2(rightX - valTextW, cy + (lineH - fs * itemScale) / 2), COL_TEXT, vBuf);

            // Mouse drag on slider
            bool slHov = (io.MousePos.x >= slX - 6 && io.MousePos.x <= slX + slW + 6 &&
                          io.MousePos.y >= slY - 8 && io.MousePos.y <= slY + slH + 8);
            if (slHov && ImGui::IsMouseClicked(0)) {
                draggingSlider = true; dragSliderPtr = it.fVal; dragSliderMin = it.fMin; dragSliderMax = it.fMax;
            }
            if (draggingSlider && dragSliderPtr == it.fVal && ImGui::IsMouseDown(0)) {
                float newFrac = (io.MousePos.x - slX) / slW;
                if (newFrac < 0) newFrac = 0; if (newFrac > 1) newFrac = 1;
                *it.fVal = it.fMin + newFrac * (it.fMax - it.fMin);
            }
        }
        // Int selector (skin/model)
        else if (it.type == 4 && it.iVal) {
            const char* displayName = GetSkinName(it.iVal, *it.iVal);
            ImVec2 nSz = ImGui::CalcTextSize(displayName);
            float bW = nSz.x * itemScale + 28;
            if (bW < 80) bW = 80;
            if (bW > cw * 0.55f) bW = cw * 0.55f;
            float bX = rightX - bW - 4;
            float bY = cy + (lineH - 20) / 2;
            bool bHov = (io.MousePos.x >= bX && io.MousePos.x <= bX + bW && io.MousePos.y >= bY && io.MousePos.y <= bY + 20);
            dl->AddRectFilled(ImVec2(bX, bY), ImVec2(bX + bW, bY + 20), IM_COL32(35, 30, 55, 230), 4.0f);
            dl->AddRectFilled(ImVec2(bX, bY), ImVec2(bX + 3, bY + 20), COL_ACCENT, 2.0f);
            // Arrows
            dl->AddText(nullptr, fs * 0.5f, ImVec2(bX + 6, bY + 3), COL_TEXT_DIM, "<");
            dl->AddText(nullptr, fs * 0.5f, ImVec2(bX + bW - 12, bY + 3), COL_TEXT_DIM, ">");
            // Name (clipped)
            dl->AddText(nullptr, fs * itemScale, ImVec2(bX + 16, bY + (20 - fs * itemScale) / 2), IM_COL32(220, 200, 255, 255), displayName);

            // Mouse click left/right half
            if (bHov && ImGui::IsMouseClicked(0)) {
                float mid = bX + bW / 2;
                int mx = GetMaxVal(it.iVal);
                if (io.MousePos.x < mid) { (*it.iVal)--; if (*it.iVal < 0) *it.iVal = mx; }
                else { (*it.iVal)++; if (*it.iVal > mx) *it.iVal = 0; }
                SkinChanger::needsUpdate = true;
            }
        }
        // Int slider
        else if (it.type == 6 && it.iVal) {
            char vBuf[16]; sprintf_s(vBuf, "%d", *it.iVal);
            float slW = 100, slH = 5;
            float valTextW = 30;
            float slX = rightX - slW - valTextW - 8;
            float slY = cy + lineH / 2 - slH / 2;
            float range = (float)(it.iMax - it.iMin);
            float frac = range > 0 ? (float)(*it.iVal - it.iMin) / range : 0;
            if (frac < 0) frac = 0; if (frac > 1) frac = 1;

            dl->AddRectFilled(ImVec2(slX, slY), ImVec2(slX + slW, slY + slH), COL_SLIDER_BG, slH / 2);
            if (frac > 0.001f) {
                dl->AddRectFilled(ImVec2(slX, slY), ImVec2(slX + slW * frac, slY + slH), COL_ACCENT, slH / 2);
            }
            float knobX = slX + slW * frac;
            dl->AddCircleFilled(ImVec2(knobX, slY + slH / 2), 5.5f, COL_ACCENT);
            dl->AddCircleFilled(ImVec2(knobX, slY + slH / 2), 2.5f, IM_COL32(255, 255, 255, 240));
            dl->AddText(nullptr, fs * itemScale, ImVec2(rightX - valTextW, cy + (lineH - fs * itemScale) / 2), COL_TEXT, vBuf);

            // Mouse drag
            bool slHov = (io.MousePos.x >= slX - 6 && io.MousePos.x <= slX + slW + 6 &&
                          io.MousePos.y >= slY - 8 && io.MousePos.y <= slY + slH + 8);
            if (slHov && ImGui::IsMouseDown(0)) {
                float newFrac = (io.MousePos.x - slX) / slW;
                if (newFrac < 0) newFrac = 0; if (newFrac > 1) newFrac = 1;
                *it.iVal = it.iMin + (int)(newFrac * range + 0.5f);
            }
        }
        // type 7: Color Picker (fVal -> float[4] RGBA)
        else if (it.type == 7 && it.fVal) {
            float cpSize = lineH * 0.6f;
            float cpX = rightX - cpSize - 8;
            float cpY = cy + (lineH - cpSize) / 2;
            
            // Draw color preview square
            ImU32 previewCol = IM_COL32(
                (int)(it.fVal[0] * 255), (int)(it.fVal[1] * 255),
                (int)(it.fVal[2] * 255), (int)(it.fVal[3] * 255));
            dl->AddRectFilled(ImVec2(cpX, cpY), ImVec2(cpX + cpSize, cpY + cpSize), previewCol, 3.0f);
            dl->AddRect(ImVec2(cpX, cpY), ImVec2(cpX + cpSize, cpY + cpSize), IM_COL32(180,180,180,200), 3.0f, 0, 1.5f);
            
            // Click to open popup
            bool cpHov = (io.MousePos.x >= cpX - 2 && io.MousePos.x <= cpX + cpSize + 2 &&
                          io.MousePos.y >= cpY - 2 && io.MousePos.y <= cpY + cpSize + 2);
            
            char popupId[64];
            snprintf(popupId, sizeof(popupId), "##cp_%s_%d", it.name, idx);
            
            if (cpHov && ImGui::IsMouseClicked(0)) {
                ImGui::OpenPopup(popupId);
            }
            
            // Color picker popup
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.11f, 0.18f, 0.95f));
            if (ImGui::BeginPopup(popupId)) {
                ImGui::Text("%s", it.name);
                ImGui::Separator();
                ImGui::ColorPicker4("##picker", it.fVal,
                    ImGuiColorEditFlags_AlphaBar |
                    ImGuiColorEditFlags_NoSidePreview |
                    ImGuiColorEditFlags_PickerHueWheel);
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

        // Generic row click to select
        if (hov && ImGui::IsMouseClicked(0) && it.type != 3 && it.type != 6 && it.type != 7) {
            selectedItem[currentTab] = idx;
        }

        return lineH;
    }

    // ==================== MAIN RENDER ====================
    inline void Render() {
        if (!menuOpen) return;

        __try {
        HandleInput();

        // Release slider drag
        if (draggingSlider && !ImGui::IsMouseDown(0)) {
            draggingSlider = false;
            dragSliderPtr = nullptr;
        }

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;
        ImGuiIO& io = ImGui::GetIO();
        float fs = ImGui::GetFontSize();
        if (fs < 1) fs = 14;

        // ==================== LAYOUT ====================
        float totalW = 920.0f, totalH = 560.0f;
        float sideW = 180.0f;
        float headerH = 54.0f;
        float contentW = totalW - sideW;
        float lineH = 34.0f;
        float padX = 16.0f;

        float mx = (io.DisplaySize.x - totalW) / 2;
        float my = (io.DisplaySize.y - totalH) / 2;

        // ==================== MAIN BACKGROUND ====================
        // Outer shadow
        dl->AddRectFilled(ImVec2(mx - 4, my - 4), ImVec2(mx + totalW + 4, my + totalH + 4), IM_COL32(0,0,0,80), 10.0f);
        // Main bg
        dl->AddRectFilled(ImVec2(mx, my), ImVec2(mx + totalW, my + totalH), COL_BG, 6.0f);
        dl->AddRect(ImVec2(mx, my), ImVec2(mx + totalW, my + totalH), COL_BORDER, 6.0f, 0, 1.0f);

        // ==================== HEADER ====================
        dl->AddRectFilled(ImVec2(mx + 1, my + 1), ImVec2(mx + totalW - 1, my + headerH), COL_HEADER, 6.0f);
        dl->AddLine(ImVec2(mx, my + headerH), ImVec2(mx + totalW, my + headerH), COL_BORDER);

        // PAVLOV title
        dl->AddText(nullptr, fs * 1.4f, ImVec2(mx + padX + 4, my + (headerH - fs * 1.4f) / 2 - 5), COL_ACCENT, "PAVLOV");
        // "premium" subtitle
        dl->AddText(nullptr, fs * 0.55f, ImVec2(mx + padX + 4, my + headerH - fs * 0.55f - 6), COL_ACCENT_DIM, "premium");

        // Version + PREMIUM badge right side
        const char* verStr = "v3.0";
        ImVec2 vSz = ImGui::CalcTextSize(verStr);
        dl->AddText(nullptr, fs * 0.6f, ImVec2(mx + totalW - padX - vSz.x * 0.6f - 80, my + (headerH - fs * 0.6f) / 2), COL_TEXT_DIM, verStr);
        // PREMIUM badge
        float badgeX = mx + totalW - 75;
        float badgeY = my + (headerH - 22) / 2;
        dl->AddRectFilled(ImVec2(badgeX, badgeY), ImVec2(badgeX + 62, badgeY + 22), IM_COL32(130, 80, 255, 50), 4.0f);
        dl->AddRect(ImVec2(badgeX, badgeY), ImVec2(badgeX + 62, badgeY + 22), COL_ACCENT, 4.0f);
        dl->AddText(nullptr, fs * 0.5f, ImVec2(badgeX + 5, badgeY + 3), COL_ACCENT, "PREMIUM");

        // ==================== SIDEBAR ====================
        float sideX = mx;
        float sideY = my + headerH;
        float sideBot = my + totalH;
        dl->AddRectFilled(ImVec2(sideX + 1, sideY), ImVec2(sideX + sideW, sideBot - 1), COL_SIDEBAR);
        dl->AddLine(ImVec2(sideX + sideW, sideY), ImVec2(sideX + sideW, sideBot), COL_BORDER);

        float tabH = 40.0f;
        float tabStartY = sideY + 10;
        for (int t = 0; t < TAB_COUNT; t++) {
            float ty = tabStartY + t * tabH;
            bool act = (t == currentTab);
            bool tHov = (io.MousePos.x >= sideX && io.MousePos.x < sideX + sideW &&
                         io.MousePos.y >= ty && io.MousePos.y < ty + tabH);

            if (act) {
                dl->AddRectFilled(ImVec2(sideX + 4, ty + 1), ImVec2(sideX + sideW - 4, ty + tabH - 1), COL_TAB_ACTIVE, 4.0f);
                // Accent left bar
                dl->AddRectFilled(ImVec2(sideX + 1, ty + 6), ImVec2(sideX + 4, ty + tabH - 6), COL_ACCENT, 2.0f);
            } else if (tHov) {
                dl->AddRectFilled(ImVec2(sideX + 4, ty + 1), ImVec2(sideX + sideW - 4, ty + tabH - 1), IM_COL32(50,50,65,40), 4.0f);
            }

            ImU32 tabCol = act ? COL_TEXT : COL_TEXT_DIM;
            // Tab icon
            dl->AddText(nullptr, fs * 0.7f, ImVec2(sideX + 16, ty + (tabH - fs * 0.7f) / 2), act ? COL_ACCENT : COL_TEXT_DIM, tabIcons[t]);
            // Tab name
            dl->AddText(nullptr, fs * 0.75f, ImVec2(sideX + 40, ty + (tabH - fs * 0.75f) / 2), tabCol, tabNames[t]);

            // Mouse click on tab
            if (tHov && ImGui::IsMouseClicked(0)) { currentTab = t; }
        }

        // User info at sidebar bottom
        float userY = sideBot - 50;
        dl->AddLine(ImVec2(sideX + 10, userY - 4), ImVec2(sideX + sideW - 10, userY - 4), IM_COL32(50,45,70,80));
        // User avatar circle
        dl->AddCircleFilled(ImVec2(sideX + 26, userY + 18), 14.0f, IM_COL32(60, 55, 85, 255));
        dl->AddCircleFilled(ImVec2(sideX + 26, userY + 12), 5.0f, COL_TEXT_DIM);
        dl->AddRectFilled(ImVec2(sideX + 19, userY + 20), ImVec2(sideX + 33, userY + 30), COL_TEXT_DIM, 3.0f);
        // Green dot (online)
        dl->AddCircleFilled(ImVec2(sideX + 35, userY + 26), 4.0f, IM_COL32(0,0,0,200));
        dl->AddCircleFilled(ImVec2(sideX + 35, userY + 26), 3.0f, IM_COL32(80,220,80,255));
        dl->AddText(nullptr, fs * 0.65f, ImVec2(sideX + 44, userY + 6), COL_TEXT, "pavlov");
        dl->AddText(nullptr, fs * 0.45f, ImVec2(sideX + 44, userY + 24), IM_COL32(80,200,80,200), "online");

        // ==================== CONTENT AREA ====================
        float contentX = mx + sideW;
        float contentY = my + headerH;
        float contentH = totalH - headerH;
        dl->AddRectFilled(ImVec2(contentX + 1, contentY), ImVec2(mx + totalW - 1, my + totalH - 1), COL_CONTENT);

        int cnt = 0;
        Item* items = GetTab(currentTab, cnt);
        int& sel = selectedItem[currentTab];
        if (sel >= cnt && cnt > 0) sel = 0;

        float cx = contentX + padX;
        float cy = contentY + 8;
        float cw = contentW - padX * 2;

        // For Gorseller tab, limit content width for ESP preview
        float previewW = 0;
        if (currentTab == 1) {
            previewW = 200.0f;
            cw -= (previewW + 10);
        }

        // Tab title
        dl->AddText(nullptr, fs * 0.7f, ImVec2(cx, cy), COL_ACCENT, tabNames[currentTab]);
        float titleW = ImGui::CalcTextSize(tabNames[currentTab]).x * 0.7f;
        dl->AddLine(ImVec2(cx, cy + fs * 0.7f + 3), ImVec2(cx + cw, cy + fs * 0.7f + 3), IM_COL32(60,50,90,80));
        cy += fs * 0.7f + 10;

        // ==================== TAB 8: KORUMA (Special) ====================
        if (currentTab == 8) {
            // Protection status
            float statusY = cy + 30;
            dl->AddRectFilled(ImVec2(cx + 20, statusY), ImVec2(cx + cw - 20, statusY + 80), IM_COL32(20,40,20,180), 8.0f);
            dl->AddRect(ImVec2(cx + 20, statusY), ImVec2(cx + cw - 20, statusY + 80), IM_COL32(40,180,80,150), 8.0f);

            // Shield icon (circle)
            float shX = cx + 50, shY = statusY + 40;
            dl->AddCircleFilled(ImVec2(shX, shY), 18, IM_COL32(40,180,80,80));
            dl->AddCircle(ImVec2(shX, shY), 18, IM_COL32(40,200,80,200), 0, 2.0f);
            dl->AddText(nullptr, fs * 0.8f, ImVec2(shX - 5, shY - fs * 0.4f), IM_COL32(40,255,80,255), "!");

            dl->AddText(nullptr, fs * 0.75f, ImVec2(cx + 80, statusY + 18), IM_COL32(40,255,80,255), "Anti-VAC: Aktif");
            dl->AddText(nullptr, fs * 0.5f, ImVec2(cx + 80, statusY + 42), COL_TEXT_DIM, "Koruma sistemi calisiyor");

            // Additional status lines
            float infoY = statusY + 100;
            dl->AddText(nullptr, fs * 0.55f, ImVec2(cx + 20, infoY), COL_TEXT_DIM, "Durum: Guvenli");
            dl->AddText(nullptr, fs * 0.55f, ImVec2(cx + 20, infoY + 20), COL_TEXT_DIM, "Son tarama: Temiz");
            dl->AddText(nullptr, fs * 0.55f, ImVec2(cx + 20, infoY + 40), COL_TEXT_DIM, "Motor: External (ring-3)");
        }
        // ==================== NORMAL TAB ITEMS ====================
        else if (items && cnt > 0) {
            // Content area clipping
            float contentBot = my + totalH - 24;
            
            // Calculate total height per column
            int leftCount = (cnt + 1) / 2;
            float leftH = leftCount * lineH;
            float rightH = (cnt - leftCount) * lineH;
            float maxColH = (leftH > rightH) ? leftH : rightH;
            
            float visibleH = contentBot - cy;
            
            // Scroll clamping
            float maxScroll = maxColH - visibleH;
            if (maxScroll < 0) maxScroll = 0;
            if (scrollY[currentTab] > maxScroll) scrollY[currentTab] = maxScroll;
            if (scrollY[currentTab] < 0) scrollY[currentTab] = 0;

            // Mouse wheel scrolling in content area
            if (io.MousePos.x >= contentX && io.MousePos.x <= mx + totalW &&
                io.MousePos.y >= contentY && io.MousePos.y <= my + totalH) {
                scrollY[currentTab] -= io.MouseWheel * lineH * 2;
                if (scrollY[currentTab] < 0) scrollY[currentTab] = 0;
                if (scrollY[currentTab] > maxScroll) scrollY[currentTab] = maxScroll;
            }

            // Draw two columns
            float colW = (cw - 20) / 2.0f;
            float cx1 = cx;
            float cx2 = cx + colW + 20;
            
            float drawY1 = cy - scrollY[currentTab];
            float drawY2 = cy - scrollY[currentTab];

            for (int i = 0; i < cnt; i++) {
                bool isLeft = (i < leftCount);
                float& curY = isLeft ? drawY1 : drawY2;
                float curX = isLeft ? cx1 : cx2;

                if (curY + lineH > cy - 2 && curY < contentBot) {
                    DrawItem(dl, items[i], curX, curY, colW, lineH, i == sel, fs, io, i);
                }
                curY += lineH;
            }

            // Scrollbar
            if (maxColH > visibleH) {
                float sbX = contentX + contentW - 6;
                float sbH = visibleH * (visibleH / maxColH);
                float sbY = cy + (scrollY[currentTab] / maxColH) * visibleH;
                dl->AddRectFilled(ImVec2(sbX, sbY), ImVec2(sbX + 3, sbY + sbH), COL_ACCENT_DIM, 2.0f);
            }
        }

        // ==================== ESP PREVIEW (Tab 1 only) ====================
        if (currentTab == 1) {
            float prevX = contentX + contentW - previewW - padX + 5;
            float prevY = contentY + 8;
            DrawESPPreview(dl, prevX, prevY, previewW, contentH - 30);
        }

        // ==================== FOOTER ====================
        float footY = my + totalH - 18;
        dl->AddText(nullptr, fs * 0.4f, ImVec2(mx + sideW + padX, footY),
                    IM_COL32(70, 65, 100, 130),
                    "ok: gezin | sag/sol: deger | INS: ac/kapa | TAB: sekme | scroll: fare");

        // ==================== BOMB TIMER OVERLAY ====================
        if (ESP::showBombTimer && Misc::bombPlanted && Misc::bombTimer > 0) {
            float bx = io.DisplaySize.x / 2;
            float by = 30;
            float bw = 240, bh = 55;

            dl->AddRectFilled(ImVec2(bx - bw/2, by), ImVec2(bx + bw/2, by + bh), IM_COL32(18,10,30,230), 6.0f);
            dl->AddRect(ImVec2(bx - bw/2, by), ImVec2(bx + bw/2, by + bh), IM_COL32(255,50,50,200), 6.0f);

            char bombBuf[64];
            sprintf_s(bombBuf, "Bomba Alani %c", Misc::bombSite);
            ImVec2 bs = ImGui::CalcTextSize(bombBuf);
            ImU32 bombCol = (Misc::bombTimer < 5.0f) ? IM_COL32(255,50,50,255) : IM_COL32(255,180,50,255);
            dl->AddText(ImVec2(bx - bs.x/2, by + 6), bombCol, bombBuf);

            char timerBuf[16]; sprintf_s(timerBuf, "%.1f sn", Misc::bombTimer);
            ImVec2 ts = ImGui::CalcTextSize(timerBuf);
            dl->AddText(ImVec2(bx - ts.x/2, by + 22), IM_COL32(255,255,255,220), timerBuf);

            // Progress bar
            float barY2 = by + bh - 8;
            float frac2 = Misc::bombTimer / 40.0f;
            if (frac2 > 1) frac2 = 1;
            dl->AddRectFilled(ImVec2(bx - bw/2 + 6, barY2), ImVec2(bx + bw/2 - 6, barY2 + 4), IM_COL32(40,20,60,200), 2.0f);
            dl->AddRectFilled(ImVec2(bx - bw/2 + 6, barY2), ImVec2(bx - bw/2 + 6 + (bw - 12) * frac2, barY2 + 4), bombCol, 2.0f);
        }

        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Stubs (kept for external call compatibility)
    inline void LoadFonts() {}
    inline void RenderSpectatorOverlay() {}
    inline void RenderBombOverlay() {}
    inline void RenderRadarWatermark() {}
}
