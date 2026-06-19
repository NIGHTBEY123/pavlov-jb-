#pragma once
#include "..\imgui\imgui.h"
#include "..\Features\Aimbot.h"
#include "..\Features\ESP.h"
#include "..\Features\InventoryChanger.h"
#include "..\Features\Misc.h"
#include "..\Features\TriggerBot.h"
#include "..\Features\Chams.h"
#include "..\Features\Features.h"

#include <cmath>
#include <algorithm>
#include <string>
#include <shellapi.h>

namespace PremiumGUI
{
    inline bool  menuOpen      = true;
    inline int   selectedItem  = 0;
    inline float lastKeyTime   = 0;
    inline int   currentTab    = 0;  // 0=combat, 1=visuals, 2=misc

    inline void LoadFonts() {}

    // ==================== ITEM SYSTEM ====================
    // type: 0=toggle, 1=key bind, 2=mode, 3=color, 4=separator
    struct MenuItem {
        const char* name;
        bool* toggle;
        int* keyBind;
        int type;
        int* modePtr;
        float* colorPtr;
        const char** modeNames;
        int modeCount;
    };

    // Mode names
    inline const char* aimModes[] = {"legit", "rage"};
    inline const char* trigModes[] = {"legit", "rage"};

    // Key helpers
    inline const char* KeyName(int vk) {
        switch(vk) {
            case VK_MENU:     return "ALT";
            case VK_SHIFT:    return "SHIFT";
            case VK_CONTROL:  return "CTRL";
            case VK_XBUTTON1: return "MOUSE4";
            case VK_XBUTTON2: return "MOUSE5";
            case VK_RBUTTON:  return "MOUSE2";
            case 0x58:        return "X";
            case 0x43:        return "C";
            case 0x56:        return "V";
            default:          return "ALT";
        }
    }
    inline void CycleKey(int* key) {
        const int keys[] = {VK_MENU, VK_SHIFT, VK_CONTROL, VK_XBUTTON1, VK_XBUTTON2, VK_RBUTTON, 0x58, 0x43, 0x56};
        int idx = 0;
        for (int i = 0; i < 9; i++) { if (keys[i] == *key) { idx = i; break; } }
        *key = keys[(idx + 1) % 9];
    }

    // Color presets
    struct ColorPreset { const char* name; float r, g, b, a; };
    inline const ColorPreset colorPresets[] = {
        {"CYAN",    0.0f,  0.78f, 1.0f,  0.8f},
        {"RED",     1.0f,  0.2f,  0.2f,  0.8f},
        {"GREEN",   0.0f,  1.0f,  0.4f,  0.8f},
        {"YELLOW",  1.0f,  0.9f,  0.2f,  0.8f},
        {"ORANGE",  1.0f,  0.5f,  0.0f,  0.8f},
        {"PURPLE",  0.7f,  0.2f,  1.0f,  0.8f},
        {"WHITE",   1.0f,  1.0f,  1.0f,  0.8f},
        {"PINK",    1.0f,  0.4f,  0.7f,  0.8f},
    };
    inline constexpr int NUM_PRESETS = sizeof(colorPresets) / sizeof(ColorPreset);
    inline int boxColorIdx = 0, skelColorIdx = 0, glowColorIdx = 0, chamsColorIdx = 0, snapColorIdx = 0;

    inline const char* GetColorName(float* col) {
        for (int i = 0; i < NUM_PRESETS; i++) {
            if (fabsf(col[0] - colorPresets[i].r) < 0.05f &&
                fabsf(col[1] - colorPresets[i].g) < 0.05f)
                return colorPresets[i].name;
        }
        return "CYAN";
    }
    inline void CycleColor(float* col, int* idx) {
        *idx = (*idx + 1) % NUM_PRESETS;
        col[0] = colorPresets[*idx].r;
        col[1] = colorPresets[*idx].g;
        col[2] = colorPresets[*idx].b;
        col[3] = colorPresets[*idx].a;
    }

    // ==================== TAB DEFINITIONS ====================
    inline const char* tabNames[] = {"COMBAT", "VISUALS", "MISC"};
    inline constexpr int TAB_COUNT = 3;

    // Combat tab items
    inline MenuItem combatItems[] = {
        {"Aimbot",          &Aimbot::enabled,       nullptr,             0, nullptr,           nullptr, nullptr, 0},
        {"Aim Mode",        nullptr,                nullptr,             2, &Aimbot::mode,     nullptr, aimModes, 2},
        {"Aim Key",         nullptr,                &Aimbot::hotkey,     1, nullptr,           nullptr, nullptr, 0},
        {"TriggerBot",      &TriggerBot::enabled,   nullptr,             0, nullptr,           nullptr, nullptr, 0},
        {"Trig Mode",       nullptr,                nullptr,             2, &TriggerBot::mode, nullptr, trigModes, 2},
        {"Trig Key",        nullptr,                &TriggerBot::hotkey, 1, nullptr,           nullptr, nullptr, 0},
        {"Anti Flash",      &ESP::antiFlash,        nullptr,             0, nullptr,           nullptr, nullptr, 0},
    };
    inline constexpr int COMBAT_COUNT = sizeof(combatItems) / sizeof(MenuItem);

    // Visuals tab items
    inline MenuItem visualItems[] = {
        {"Box ESP",         &ESP::showBox,          nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"  Box Color",     nullptr,                nullptr,             3, nullptr, ESP::boxColor, nullptr, 0},
        {"Health Bar",      &ESP::showHealth,       nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Armor Bar",       &ESP::showArmor,        nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Name ESP",        &ESP::showName,         nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Weapon ESP",      &ESP::showWeapon,       nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Skeleton",        &ESP::showSkeleton,     nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"  Skel Color",    nullptr,                nullptr,             3, nullptr, ESP::skelColor, nullptr, 0},
        {"Snaplines",       &ESP::showSnaplines,    nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"  Snap Color",    nullptr,                nullptr,             3, nullptr, ESP::snaplineColor, nullptr, 0},
        {"Distance",        &ESP::showDistance,     nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Glow",            &ESP::showGlow,         nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"  Glow Color",    nullptr,                nullptr,             3, nullptr, ESP::glowColor, nullptr, 0},
        {"Chams",           &ESP::showChams,        nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"  Chams Color",   nullptr,                nullptr,             3, nullptr, ESP::chamsColor, nullptr, 0},
    };
    inline constexpr int VISUAL_COUNT = sizeof(visualItems) / sizeof(MenuItem);

    // Misc tab items
    inline MenuItem miscItems[] = {
        {"Radar Hack",      &Misc::radarEnabled,    nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"BunnyHop",        &Misc::bhopEnabled,     nullptr,             0, nullptr, nullptr, nullptr, 0},
        {"Fly Hack",        &Misc::noclipEnabled,   nullptr,             0, nullptr, nullptr, nullptr, 0},
    };
    inline constexpr int MISC_COUNT = sizeof(miscItems) / sizeof(MenuItem);

    inline MenuItem* GetCurrentItems(int& count) {
        switch(currentTab) {
            case 0: count = COMBAT_COUNT; return combatItems;
            case 1: count = VISUAL_COUNT; return visualItems;
            case 2: count = MISC_COUNT;   return miscItems;
            default: count = 0; return nullptr;
        }
    }

    inline void RenderSpectatorOverlay() {}
    inline void RenderBombOverlay() {}
    inline void RenderRadarWatermark() {}

    inline void HandleAction(MenuItem& item) {
        if (item.type == 0 && item.toggle)
            *item.toggle = !*item.toggle;
        else if (item.type == 1 && item.keyBind)
            CycleKey(item.keyBind);
        else if (item.type == 2 && item.modePtr)
            *item.modePtr = (*item.modePtr + 1) % item.modeCount;
        else if (item.type == 3 && item.colorPtr) {
            if (item.colorPtr == ESP::boxColor)       CycleColor(item.colorPtr, &boxColorIdx);
            else if (item.colorPtr == ESP::skelColor)  CycleColor(item.colorPtr, &skelColorIdx);
            else if (item.colorPtr == ESP::glowColor)  CycleColor(item.colorPtr, &glowColorIdx);
            else if (item.colorPtr == ESP::chamsColor) CycleColor(item.colorPtr, &chamsColorIdx);
            else if (item.colorPtr == ESP::snaplineColor) CycleColor(item.colorPtr, &snapColorIdx);
        }
    }

    inline void Render()
    {
        if (!menuOpen) return;

        float now = (float)ImGui::GetTime();
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();

        int itemCount = 0;
        MenuItem* items = GetCurrentItems(itemCount);
        if (!items) return;

        // Clamp selected item
        if (selectedItem >= itemCount) selectedItem = 0;
        if (selectedItem < 0) selectedItem = itemCount - 1;

        // Input handling
        if (now - lastKeyTime > 0.13f) {
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                selectedItem--;
                if (selectedItem < 0) selectedItem = itemCount - 1;
                lastKeyTime = now;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                selectedItem++;
                if (selectedItem >= itemCount) selectedItem = 0;
                lastKeyTime = now;
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
                currentTab--;
                if (currentTab < 0) currentTab = TAB_COUNT - 1;
                selectedItem = 0;
                lastKeyTime = now;
            }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
                // If on a mode/color/key item, handle action; otherwise switch tab
                auto& item = items[selectedItem];
                if (item.type >= 1) {
                    HandleAction(item);
                } else {
                    currentTab++;
                    if (currentTab >= TAB_COUNT) currentTab = 0;
                    selectedItem = 0;
                }
                lastKeyTime = now;
            }
            if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                HandleAction(items[selectedItem]);
                lastKeyTime = now;
            }
        }

        // ==================== LAYOUT ====================
        float lineH    = 26.0f;
        float menuW    = 340.0f;
        float padX     = 14.0f;
        float tabH     = 30.0f;
        float headerH  = 36.0f;
        float menuH    = headerH + tabH + itemCount * lineH + 12.0f;
        float menuX    = 14.0f;
        float menuY    = (io.DisplaySize.y - menuH) * 0.5f;

        // ==================== BACKGROUND ====================
        // Main panel
        dl->AddRectFilled(ImVec2(menuX, menuY), ImVec2(menuX + menuW, menuY + menuH),
                         IM_COL32(12, 12, 16, 245));
        // Border
        dl->AddRect(ImVec2(menuX, menuY), ImVec2(menuX + menuW, menuY + menuH),
                   IM_COL32(60, 60, 80, 200), 0, 0, 1.0f);

        // ==================== HEADER ====================
        // Accent line on top
        dl->AddRectFilled(ImVec2(menuX, menuY), ImVec2(menuX + menuW, menuY + 2),
                         IM_COL32(100, 50, 255, 255));

        // Title
        const char* title = "NEXUS";
        ImVec2 titleSz = ImGui::CalcTextSize(title);
        dl->AddText(ImVec2(menuX + (menuW - titleSz.x) * 0.5f, menuY + 8),
                   IM_COL32(100, 50, 255, 255), title);

        // Subtitle
        const char* sub = "internal | v2.4";
        ImVec2 subSz = ImGui::CalcTextSize(sub);
        float subScale = 0.7f;
        dl->AddText(nullptr, ImGui::GetFontSize() * subScale,
                   ImVec2(menuX + (menuW - subSz.x * subScale) * 0.5f, menuY + 22),
                   IM_COL32(100, 100, 120, 200), sub);

        // ==================== TABS ====================
        float tabY = menuY + headerH;
        float tabW = menuW / TAB_COUNT;

        for (int t = 0; t < TAB_COUNT; t++) {
            float tx = menuX + t * tabW;
            bool active = (t == currentTab);

            // Tab background
            if (active) {
                dl->AddRectFilled(ImVec2(tx, tabY), ImVec2(tx + tabW, tabY + tabH),
                                 IM_COL32(30, 25, 50, 255));
                // Active indicator
                dl->AddRectFilled(ImVec2(tx, tabY + tabH - 2), ImVec2(tx + tabW, tabY + tabH),
                                 IM_COL32(100, 50, 255, 255));
            }

            // Tab text
            ImVec2 tSz = ImGui::CalcTextSize(tabNames[t]);
            float tScale = 0.78f;
            ImU32 tCol = active ? IM_COL32(255, 255, 255, 240) : IM_COL32(100, 100, 120, 180);
            dl->AddText(nullptr, ImGui::GetFontSize() * tScale,
                       ImVec2(tx + (tabW - tSz.x * tScale) * 0.5f, tabY + (tabH - ImGui::GetFontSize() * tScale) * 0.5f),
                       tCol, tabNames[t]);

            // Tab click
            if (io.MousePos.x >= tx && io.MousePos.x < tx + tabW &&
                io.MousePos.y >= tabY && io.MousePos.y < tabY + tabH) {
                if (ImGui::IsMouseClicked(0)) {
                    currentTab = t;
                    selectedItem = 0;
                }
            }
        }

        // Separator
        dl->AddLine(ImVec2(menuX, tabY + tabH), ImVec2(menuX + menuW, tabY + tabH),
                   IM_COL32(50, 50, 70, 200));

        // ==================== ITEMS ====================
        float curY = tabY + tabH + 6.0f;

        for (int i = 0; i < itemCount; i++) {
            auto& item = items[i];
            bool sel = (i == selectedItem);

            bool hovered = (io.MousePos.x >= menuX && io.MousePos.x <= menuX + menuW &&
                           io.MousePos.y >= curY && io.MousePos.y <= curY + lineH);
            bool highlight = sel || hovered;

            // Selection highlight
            if (highlight) {
                dl->AddRectFilled(ImVec2(menuX + 2, curY), ImVec2(menuX + menuW - 2, curY + lineH),
                                 IM_COL32(100, 50, 255, 30));
                // Left accent bar
                dl->AddRectFilled(ImVec2(menuX + 2, curY + 2), ImVec2(menuX + 4, curY + lineH - 2),
                                 IM_COL32(100, 50, 255, 200));
            }

            // Item name
            float textY = curY + (lineH - ImGui::GetFontSize() * 0.75f) * 0.5f;
            bool isSubItem = (item.name[0] == ' ' && item.name[1] == ' ');
            float nameX = menuX + padX + (isSubItem ? 10.0f : 0.0f);

            ImU32 nameCol;
            if (highlight)
                nameCol = IM_COL32(255, 255, 255, 240);
            else if (isSubItem)
                nameCol = IM_COL32(110, 110, 130, 180);
            else
                nameCol = IM_COL32(170, 170, 185, 200);

            dl->AddText(nullptr, ImGui::GetFontSize() * 0.75f,
                       ImVec2(nameX, textY), nameCol, item.name);

            // Status value
            const char* st = "";
            ImU32 stCol = IM_COL32(255,255,255,255);

            if (item.type == 0) {
                bool on = item.toggle ? *item.toggle : false;
                st = on ? "ON" : "OFF";
                stCol = on ? IM_COL32(80, 255, 80, 255) : IM_COL32(255, 60, 60, 180);
            } else if (item.type == 1 && item.keyBind) {
                st = KeyName(*item.keyBind);
                stCol = IM_COL32(255, 200, 50, 230);
            } else if (item.type == 2 && item.modePtr && item.modeNames) {
                st = item.modeNames[*item.modePtr];
                stCol = (*item.modePtr == 0) ? IM_COL32(100, 200, 255, 230) : IM_COL32(255, 80, 80, 230);
            } else if (item.type == 3 && item.colorPtr) {
                st = GetColorName(item.colorPtr);
                stCol = IM_COL32((int)(item.colorPtr[0]*255), (int)(item.colorPtr[1]*255),
                                (int)(item.colorPtr[2]*255), 255);
            }

            // Right-aligned status
            ImVec2 stSz = ImGui::CalcTextSize(st);
            float stX = menuX + menuW - padX - stSz.x * 0.75f;

            // Color preview square for color items
            if (item.type == 3 && item.colorPtr) {
                float sqSz = 8.0f;
                float sqX = stX - sqSz - 5;
                float sqY = textY + 3;
                ImU32 preview = IM_COL32((int)(item.colorPtr[0]*255), (int)(item.colorPtr[1]*255),
                                        (int)(item.colorPtr[2]*255), 255);
                dl->AddRectFilled(ImVec2(sqX, sqY), ImVec2(sqX + sqSz, sqY + sqSz), preview);
                dl->AddRect(ImVec2(sqX-1, sqY-1), ImVec2(sqX + sqSz+1, sqY + sqSz+1),
                           IM_COL32(255,255,255,40));
            }

            dl->AddText(nullptr, ImGui::GetFontSize() * 0.75f,
                       ImVec2(stX, textY), stCol, st);

            // Mouse click
            if (hovered && ImGui::IsMouseClicked(0)) {
                selectedItem = i;
                HandleAction(item);
            }

            curY += lineH;
        }

        // ==================== FOOTER ====================
        float footY = curY + 2;
        dl->AddLine(ImVec2(menuX + padX, footY), ImVec2(menuX + menuW - padX, footY),
                   IM_COL32(50, 50, 70, 150));

        const char* hint = "[INS] toggle  [arrows] navigate  [enter] select";
        float hScale = 0.6f;
        ImVec2 hSz = ImGui::CalcTextSize(hint);
        dl->AddText(nullptr, ImGui::GetFontSize() * hScale,
                   ImVec2(menuX + (menuW - hSz.x * hScale) * 0.5f, footY + 3),
                   IM_COL32(80, 80, 100, 150), hint);
    }

} // namespace PremiumGUI
