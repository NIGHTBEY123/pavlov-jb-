#pragma once
#include "..\SDK\Entity.h"
#include "..\SDK\Offsets.h"
#include "..\imgui\imgui.h"
#include "SceneChams.h"
#include "DrawObjectHook.h"
#include "../GUI/IconsFontAwesome6.h"
#include <vector>
#include <cmath>

extern void Log(const char* msg);

namespace ESP {
    inline bool showBox = true;
    inline bool boxFilled = false;          // Filled box with gradient
    inline int  boxColorIdx = 0;            // Color preset index
    inline int  skelColorIdx = 0;           // Skeleton color preset index
    inline bool showHealth = true;
    inline bool showName = true;
    inline bool showArmor = true;          // ENABLED by default
    inline int  armorDisplayMode = 2;      // 0=bar only, 1=text only, 2=both
    inline bool showDistance = false;
    inline bool showSnaplines = false;
    inline bool showMoney = true;          // Show money
    inline bool showAmmo = true;           // Show ammo count
    inline bool enemyOnly = true;
    inline bool showSkeleton = true;
    inline bool showGlow = false;          // Outline glow (CGlowProperty)
    inline bool showChams = false;         // Solid chams (CGlowProperty type 3)
    inline bool showWeapon = true;         // Show weapon name
    inline bool antiFlash = false;         // Remove flash effect
    inline bool showDroppedWeapons = false; // Show weapons on ground
    inline bool showBombESP = true;        // Show bomb on ground
    inline bool showBombTimer = true;      // C4 countdown timer
    inline bool showSpectators = true;     // Who is watching us
    inline bool showGrenadeRadius = false; // Grenade effect radius
    inline bool showSteamInfo = true;      // Steam profile next to name
    inline bool showWeaponIcon = true;     // Weapon silhouette below box
    inline bool showDefuseKit = true;      // Defuse kit icon
    inline int  boxMode = 0;               // 0=Normal, 1=Corner, 2=Filled, 3=Corner+Filled
    inline bool glowDeadFilter = true;     // Don't glow dead players
    inline bool skinChangerEnabled = false; // Master toggle for skin changer

    // Color customization (RGBA floats for ImGui ColorEdit)
    inline float boxColor[4]     = {0.0f, 0.78f, 1.0f, 0.8f};    // Cyan
    inline float skelColor[4]    = {0.0f, 1.0f, 0.8f, 0.86f};    // Teal
    inline float glowColor[4]    = {1.0f, 0.2f, 0.2f, 1.0f};     // Red
    inline float chamsColor[4]   = {0.0f, 0.5f, 1.0f, 1.0f};     // Blue
    inline int chamsMode = 1;          // Render mode: 0=Normal, 1=Color, 3=Glow, 9=WorldGlow
    inline int chamsGlowType = 0;      // Glow type: 0=solid, 1=rim, 2=edge, 3=outline
    inline float snaplineColor[4]= {1.0f, 1.0f, 0.3f, 0.6f};     // Yellow
    inline bool showFovCircle = false;     // Draw aimbot FOV circle

    // Entity list stride
    constexpr int ENTITY_STRIDE = 0x70;  // 112 decimal — verified by debug overlay (0x78 gives garbage)

    // Weapon name lookup by item definition index
    inline const char* GetWeaponNameByDefIndex(uint16_t idx) {
        switch (idx) {
            case 1: return "Deagle"; case 2: return "Dualies"; case 3: return "Five-SeveN";
            case 4: return "Glock"; case 7: return "AK-47"; case 8: return "AUG";
            case 9: return "AWP"; case 10: return "FAMAS"; case 11: return "G3SG1";
            case 13: return "Galil"; case 14: return "M249"; case 16: return "M4A4";
            case 17: return "MAC-10"; case 19: return "P90"; case 23: return "MP5-SD";
            case 24: return "UMP-45"; case 25: return "XM1014"; case 26: return "PP-Bizon";
            case 27: return "MAG-7"; case 28: return "Negev"; case 29: return "Sawed-Off";
            case 30: return "Tec-9"; case 31: return "Zeus"; case 32: return "P2000";
            case 33: return "MP7"; case 34: return "MP9"; case 35: return "Nova";
            case 36: return "P250"; case 38: return "SCAR-20"; case 39: return "SG 553";
            case 40: return "SSG 08"; case 60: return "M4A1-S"; case 61: return "USP-S";
            case 63: return "CZ75"; case 64: return "R8"; case 42: return "Knife";
            case 43: return "Flashbang"; case 44: return "HE Nade"; case 45: return "Smoke";
            case 46: return "Molotov"; case 47: return "Decoy"; case 48: return "Incendiary";
            case 49: return "C4"; case 57: return "MAG-7"; case 59: return "Knife";
            default: return nullptr;
        }
    }

    // FontAwesome fallback icon lookup by item definition index
    inline const char* GetWeaponIconByDefIndex(uint16_t idx) {
        switch (idx) {
            case 1: case 2: case 3: case 4: case 30: case 32: case 36: case 61: case 63: case 64: 
                return ICON_FA_GUN; // Pistols
            case 7: case 8: case 10: case 13: case 16: case 39: case 60: 
                return ICON_FA_CROSSHAIRS; // Rifles
            case 9: case 11: case 38: case 40:
                return ICON_FA_LOCATION_CROSSHAIRS; // Snipers
            case 17: case 19: case 23: case 24: case 26: case 33: case 34: 
                return ICON_FA_BOLT; // SMGs
            case 14: case 28: case 25: case 27: case 29: case 35: case 57:
                return ICON_FA_SCREWDRIVER_WRENCH; // Heavies / Shotguns
            case 42: case 59: 
                return ICON_FA_WRENCH; // Knives
            case 43: case 44: case 45: case 46: case 47: case 48: 
                return ICON_FA_BOMB; // Grenades
            case 49: return ICON_FA_BOMB; // C4
            case 31: return ICON_FA_BOLT_LIGHTNING; // Zeus
            default: return ICON_FA_STAR;
        }
    }

    // ======== CS2 BONE INDICES - FROM ACTUAL BONE DUMP ========
    // Verified by dumping bone positions in-game
    enum Bone : int {
        BONE_PELVIS       = 0,   // Z=-126.7 (origin level)
        BONE_SPINE_1      = 3,   // Z=-83.1 (stomach)
        BONE_SPINE_2      = 5,   // Z=-72.2 (chest)
        BONE_NECK         = 6,   // Z=-66.4 (neck)
        BONE_HEAD         = 7,   // Z=-62.6 (highest = head)
        // Left arm
        BONE_LEFT_SHOULDER  = 8,   // Z=-69.6 (shoulder height)
        BONE_LEFT_ELBOW     = 9,   // similar height
        BONE_LEFT_HAND      = 11,  // Z=-72.0 (hand)
        // Right arm
        BONE_RIGHT_SHOULDER = 12,  // Z=-69.9 (shoulder height)
        BONE_RIGHT_ELBOW    = 13,  // Z=-70.5
        BONE_RIGHT_HAND     = 15,  // Z=-71.9 (hand)
        // Left leg
        BONE_LEFT_HIP       = 17,  // Z=-91.6 (upper leg)
        BONE_LEFT_KNEE      = 18,  // Z=-105.5 (knee)
        BONE_LEFT_FOOT      = 19,  // Z=-122.1 (foot)
        // Right leg
        BONE_RIGHT_HIP      = 20,  // Z=-91.9 (upper leg)
        BONE_RIGHT_KNEE     = 21,  // Z=-107.8 (knee)
        BONE_RIGHT_FOOT     = 22,  // Z=-122.3 (foot)
    };

    // Bone connection pairs for skeleton drawing
    struct BoneConnection { int from; int to; };
    inline const BoneConnection SKELETON_CONNECTIONS[] = {
        // Spine
        { BONE_HEAD, BONE_NECK },
        { BONE_NECK, BONE_SPINE_2 },
        { BONE_SPINE_2, BONE_SPINE_1 },
        // Left arm
        { BONE_SPINE_2, BONE_LEFT_SHOULDER },
        { BONE_LEFT_SHOULDER, BONE_LEFT_ELBOW },
        { BONE_LEFT_ELBOW, BONE_LEFT_HAND },
        // Right arm
        { BONE_SPINE_2, BONE_RIGHT_SHOULDER },
        { BONE_RIGHT_SHOULDER, BONE_RIGHT_ELBOW },
        { BONE_RIGHT_ELBOW, BONE_RIGHT_HAND },
        // Left leg
        { BONE_SPINE_1, BONE_LEFT_HIP },
        { BONE_LEFT_HIP, BONE_LEFT_KNEE },
        { BONE_LEFT_KNEE, BONE_LEFT_FOOT },
        // Right leg
        { BONE_SPINE_1, BONE_RIGHT_HIP },
        { BONE_RIGHT_HIP, BONE_RIGHT_KNEE },
        { BONE_RIGHT_KNEE, BONE_RIGHT_FOOT },
    };
    constexpr int NUM_CONNECTIONS = sizeof(SKELETON_CONNECTIONS) / sizeof(BoneConnection);

    inline bool WorldToScreen(const Vec3& world, Vec2& screen, const ViewMatrix& vm, int screenW, int screenH) {
        if (!world.IsValid()) return false;
        
        float w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
        // Must check std::isnan in case w itself evaluates to NaN
        if (std::isnan(w) || w < 0.001f) return false;
        
        float invW = 1.0f / w;
        screen.x = (screenW * 0.5f) + (vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3]) * invW * (screenW * 0.5f);
        screen.y = (screenH * 0.5f) - (vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3]) * invW * (screenH * 0.5f);
        
        if (std::isnan(screen.x) || std::isnan(screen.y)) return false;
        
        return true;
    }

    inline void DrawPlayer(ImDrawList* drawList, CEntity& player,
                           const ViewMatrix& vm, int screenW, int screenH, const Vec3& localPos, int hp,
                           int armor = 0, const char* weaponName = nullptr, uint16_t weaponDefIdx = 0, int money = 0, int ammo = -1) {
        Vec3 origin = player.GetOrigin();
        if (origin.IsZero()) return;

        Vec3 headPos = player.GetBonePos(BONE_HEAD);
        Vec3 headTop = headPos.IsZero() ? Vec3{origin.x, origin.y, origin.z + 72.0f}
                                         : Vec3{headPos.x, headPos.y, headPos.z + 10.0f};

        Vec2 screenFeet, screenHead;
        if (!WorldToScreen(origin, screenFeet, vm, screenW, screenH)) return;
        if (!WorldToScreen(headTop, screenHead, vm, screenW, screenH)) return;

        float boxH = screenFeet.y - screenHead.y;
        if (boxH < 5.0f) return;
        float boxW = boxH * 0.35f;

        ImU32 boxCol = IM_COL32((int)(boxColor[0]*255), (int)(boxColor[1]*255), (int)(boxColor[2]*255), (int)(boxColor[3]*255));

        ImU32 outlineColor = IM_COL32(0, 0, 0, 200);
        float boxLeft = screenHead.x - boxW / 2;
        float boxTop = screenHead.y;
        float boxRight = boxLeft + boxW;
        float boxBot = boxTop + boxH;

        // Box — with modes
        if (showBox) {
            ImU32 outCol = outlineColor;
            float cornerLen = boxW * 0.25f;
            if (cornerLen < 6) cornerLen = 6;
            
            if (boxMode == 0) {
                // Mode 0: Normal full box
                drawList->AddRect(ImVec2(boxLeft - 1, boxTop - 1), ImVec2(boxRight + 1, boxBot + 1), outCol, 0, 0, 2.5f);
                drawList->AddRect(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBot), boxCol, 0, 0, 1.2f);
            } else if (boxMode == 1 || boxMode == 3) {
                // Mode 1/3: Corner lines (4 corners)
                float t = 1.5f;
                // Top-left
                drawList->AddLine(ImVec2(boxLeft, boxTop), ImVec2(boxLeft + cornerLen, boxTop), boxCol, t);
                drawList->AddLine(ImVec2(boxLeft, boxTop), ImVec2(boxLeft, boxTop + cornerLen), boxCol, t);
                // Top-right
                drawList->AddLine(ImVec2(boxRight, boxTop), ImVec2(boxRight - cornerLen, boxTop), boxCol, t);
                drawList->AddLine(ImVec2(boxRight, boxTop), ImVec2(boxRight, boxTop + cornerLen), boxCol, t);
                // Bottom-left
                drawList->AddLine(ImVec2(boxLeft, boxBot), ImVec2(boxLeft + cornerLen, boxBot), boxCol, t);
                drawList->AddLine(ImVec2(boxLeft, boxBot), ImVec2(boxLeft, boxBot - cornerLen), boxCol, t);
                // Bottom-right
                drawList->AddLine(ImVec2(boxRight, boxBot), ImVec2(boxRight - cornerLen, boxBot), boxCol, t);
                drawList->AddLine(ImVec2(boxRight, boxBot), ImVec2(boxRight, boxBot - cornerLen), boxCol, t);
            }
            if (boxMode == 2 || boxMode == 3) {
                // Mode 2/3: Filled background (semi-transparent)
                drawList->AddRectFilled(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBot), IM_COL32(boxColor[0]*255, boxColor[1]*255, boxColor[2]*255, 35));
            }
        }

        float fontSize = ImGui::GetFontSize() * 0.55f;
        float fScale = fontSize / ImGui::GetFontSize();
        float barW = 2.0f; // Thinner bars for premium look

        // ===== HP BAR (left side) + text =====
        if (showHealth && hp > 0) {
            float bX = boxLeft - 7;
            float healthFrac = hp / 100.0f;
            if (healthFrac > 1.0f) healthFrac = 1.0f;
            float barH = boxH * healthFrac;
            uint8_t r = (uint8_t)(255 * (1.0f - healthFrac));
            uint8_t g = (uint8_t)(255 * healthFrac);
            ImU32 hpCol = IM_COL32(r, g, 0, 255);
            drawList->AddRectFilled(ImVec2(bX-1, boxTop-1), ImVec2(bX+barW+1, boxBot+1), IM_COL32(0,0,0,150), 1.0f);
            drawList->AddRectFilled(ImVec2(bX, boxBot-barH), ImVec2(bX+barW, boxBot), hpCol, 1.0f);
            if (hp < 100) {
                char hpBuf[8]; sprintf_s(hpBuf, "%d", hp);
                drawList->AddText(nullptr, fontSize*0.9f, ImVec2(bX - 16, boxBot - barH - 1), hpCol, hpBuf);
            }
        }

        // ===== ARMOR BAR (right side) =====
        if (showArmor && armor > 0) {
            float aX = boxRight + 4;
            float armorFrac = armor / 100.0f;
            if (armorFrac > 1.0f) armorFrac = 1.0f;
            float aBarH = boxH * armorFrac;
            ImU32 armorCol = IM_COL32(80, 140, 255, 255);
            drawList->AddRectFilled(ImVec2(aX-1, boxTop-1), ImVec2(aX+barW+1, boxBot+1), IM_COL32(0,0,0,150), 1.0f);
            drawList->AddRectFilled(ImVec2(aX, boxBot-aBarH), ImVec2(aX+barW, boxBot), armorCol, 1.0f);
            if (armor < 100) {
                char arBuf[8]; sprintf_s(arBuf, "%d", armor);
                drawList->AddText(nullptr, fontSize*0.9f, ImVec2(aX + 4, boxBot - aBarH - 1), armorCol, arBuf);
            }
        }

        // ===== NAME is drawn in the Render() loop above, not here =====
        // centerX for bottom info
        float centerX = screenFeet.x;

        // Skeleton
        if (showSkeleton) {
            ImU32 skelCol = IM_COL32((int)(skelColor[0]*255), (int)(skelColor[1]*255), (int)(skelColor[2]*255), (int)(skelColor[3]*255));

            for (int c = 0; c < NUM_CONNECTIONS; c++) {
                Vec3 boneFrom = player.GetBonePos(SKELETON_CONNECTIONS[c].from);
                Vec3 boneTo = player.GetBonePos(SKELETON_CONNECTIONS[c].to);
                if (boneFrom.IsZero() || boneTo.IsZero()) continue;

                Vec2 screenFrom, screenTo;
                if (WorldToScreen(boneFrom, screenFrom, vm, screenW, screenH) &&
                    WorldToScreen(boneTo, screenTo, vm, screenW, screenH)) {
                    drawList->AddLine(ImVec2(screenFrom.x, screenFrom.y), ImVec2(screenTo.x, screenTo.y), IM_COL32(0,0,0,180), 2.5f);
                    drawList->AddLine(ImVec2(screenFrom.x, screenFrom.y), ImVec2(screenTo.x, screenTo.y), skelCol, 1.2f);
                }
            }
        }

        // Snaplines
        if (showSnaplines) {
            ImU32 snapCol = IM_COL32((int)(snaplineColor[0]*255), (int)(snaplineColor[1]*255), (int)(snaplineColor[2]*255), (int)(snaplineColor[3]*255));
            drawList->AddLine(ImVec2((float)screenW / 2, (float)screenH),
                              ImVec2(screenFeet.x, screenFeet.y),
                              snapCol, 1.2f);
        }

        // ===== BOTTOM INFO (below box) =====
        float textY2 = boxBot + 3;
        float centerX2 = screenFeet.x;
        float infoFontSz = fontSize * 1.1f;
        float infoScale = infoFontSz / ImGui::GetFontSize();

        // Weapon icon (styled box with shadow background)
        if (showWeapon && weaponName && weaponName[0]) {
            const char* wIcon = GetWeaponIconByDefIndex(weaponDefIdx);
            char combinedWepBuf[128]; // Increased buffer size from 64 to 128
            
            __try {
                if (showWeaponIcon && wIcon) {
                    snprintf(combinedWepBuf, sizeof(combinedWepBuf), "%s %s", wIcon, weaponName);
                } else {
                    snprintf(combinedWepBuf, sizeof(combinedWepBuf), "%s", weaponName);
                }
                
                ImVec2 wis = ImGui::CalcTextSize(combinedWepBuf);
                float wiW = wis.x * infoScale;
                float wix = centerX2 - wiW/2;
                // Shadow bg
                drawList->AddRectFilled(ImVec2(wix-4, textY2-1), ImVec2(wix+wiW+4, textY2+infoFontSz+2), IM_COL32(0,0,0,140), 3.0f);
                // Shadow text
                drawList->AddText(nullptr, infoFontSz, ImVec2(wix+1, textY2+1), IM_COL32(0,0,0,200), combinedWepBuf);
                drawList->AddText(nullptr, infoFontSz, ImVec2(wix, textY2), IM_COL32(240,220,140,255), combinedWepBuf);
                textY2 += infoFontSz + 4;
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }

        // Money (green if high, red if low)
        if (showMoney && money > 0) {
            char moneyBuf[16]; sprintf_s(moneyBuf, "$%d", money);
            ImVec2 ms = ImGui::CalcTextSize(moneyBuf);
            float mw = ms.x * infoScale;
            // Color: green > 3000, yellow 1000-3000, red < 1000
            ImU32 moneyCol;
            if (money >= 3000) moneyCol = IM_COL32(50, 255, 100, 240);
            else if (money >= 1000) moneyCol = IM_COL32(255, 220, 50, 240);
            else moneyCol = IM_COL32(255, 80, 60, 240);
            drawList->AddText(nullptr, infoFontSz, ImVec2(centerX2 - mw/2 + 1, textY2 + 1), IM_COL32(0,0,0,180), moneyBuf);
            drawList->AddText(nullptr, infoFontSz, ImVec2(centerX2 - mw/2, textY2), moneyCol, moneyBuf);
            textY2 += infoFontSz + 2;
        }

        // Ammo
        if (showAmmo && ammo >= 0) {
            char ammoBuf[16]; sprintf_s(ammoBuf, "%d", ammo);
            ImVec2 as = ImGui::CalcTextSize(ammoBuf);
            float aw = as.x * infoScale;
            drawList->AddText(nullptr, infoFontSz, ImVec2(centerX2 - aw/2 + 1, textY2 + 1), IM_COL32(0,0,0,180), ammoBuf);
            drawList->AddText(nullptr, infoFontSz, ImVec2(centerX2 - aw/2, textY2), IM_COL32(200,200,220,220), ammoBuf);
            textY2 += infoFontSz + 2;
        }

        // Distance
        if (showDistance) {
            float dist = localPos.DistanceTo(origin) * 0.0254f;
            char distBuf[16]; sprintf_s(distBuf, "%.0fm", dist);
            ImVec2 ts = ImGui::CalcTextSize(distBuf);
            float tw = ts.x * infoScale;
            drawList->AddText(nullptr, infoFontSz, ImVec2(centerX2 - tw/2, textY2), IM_COL32(180,180,180,180), distBuf);
        }
    }

    // Entity list helpers
    inline uintptr_t GetEntityByIndex(uintptr_t entityList, int index) {
        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((index & 0x7FFF) >> 9));
        if (!bucket) return 0;
        return Memory::Read<uintptr_t>(bucket + ENTITY_STRIDE * (index & 0x1FF));
    }

    inline uintptr_t ResolvePawnFromHandle(uintptr_t entityList, uint32_t pawnHandle) {
        if (!pawnHandle || pawnHandle == 0xFFFFFFFF) return 0;
        uint32_t idx = pawnHandle & 0x7FFF;
        if (idx == 0 || idx > 0x4000) return 0;
        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
        if (!bucket) return 0;
        return Memory::Read<uintptr_t>(bucket + ENTITY_STRIDE * (idx & 0x1FF));
    }

    // ======== UNIFIED GLOW + CHAMS SYSTEM ========
    // RESEARCH RESULT: In CS2 Source 2, m_clrRender and m_ClientOverrideTint
    // DO NOT change model color. Only CGlowProperty works for visual effects.
    // Chams = Full-body glow (type 0) with high intensity colors
    // Glow  = Outline glow (type 3)
    //
    // CGlowProperty offsets (verified from dump):
    //   m_fGlowColor(Vector3) = 0x08
    //   m_iGlowType(int)      = 0x30  (0=FullBody, 1=Rim, 2=Edge, 3=Outline)
    //   m_iGlowTeam(int)      = 0x34  (-1 = all teams)
    //   m_nGlowRange(int)     = 0x38
    //   m_nGlowRangeMin(int)  = 0x3C
    //   m_glowColorOverride   = 0x40  (Color RGBA)
    //   m_bFlashing(bool)     = 0x44
    //   m_flGlowTime(float)   = 0x48  (CRITICAL: -1 = infinite)
    //   m_flGlowStartTime     = 0x4C  (CRITICAL: 0 = start now)
    //   m_bEligibleForScreenHighlight = 0x50
    //   m_bGlowing(bool)      = 0x51
    // Glow offset is now dynamic: Offsets::CGlowProperty::m_Glow

    // Glow type names for menu
    inline const char* glowTypeNames[] = {"Dolu", "Cizgi", "Kenar"};

    // ======== ESP OVERLAY CHAMS ========
    // CS2 ignores client-side CGlowProperty writes (confirmed by debug logging).
    // Instead, we draw filled quads over player skeleton using ImGui.
    // Each bone segment becomes a filled rectangle matching body part widths.

    // Width multipliers per body part (relative to base width)
    struct BoneWidth { int from; int to; float width; };
    inline const BoneWidth CHAMS_BONES[] = {
        // Spine (wide torso)
        { BONE_HEAD, BONE_NECK, 0.5f },
        { BONE_NECK, BONE_SPINE_2, 1.6f },      // upper chest — wide
        { BONE_SPINE_2, BONE_SPINE_1, 1.8f },    // lower chest — widest
        // Left arm
        { BONE_SPINE_2, BONE_LEFT_SHOULDER, 1.2f },
        { BONE_LEFT_SHOULDER, BONE_LEFT_ELBOW, 0.7f },
        { BONE_LEFT_ELBOW, BONE_LEFT_HAND, 0.5f },
        // Right arm
        { BONE_SPINE_2, BONE_RIGHT_SHOULDER, 1.2f },
        { BONE_RIGHT_SHOULDER, BONE_RIGHT_ELBOW, 0.7f },
        { BONE_RIGHT_ELBOW, BONE_RIGHT_HAND, 0.5f },
        // Left leg
        { BONE_SPINE_1, BONE_LEFT_HIP, 1.4f },   // pelvis — wide
        { BONE_LEFT_HIP, BONE_LEFT_KNEE, 0.9f },
        { BONE_LEFT_KNEE, BONE_LEFT_FOOT, 0.6f },
        // Right leg
        { BONE_SPINE_1, BONE_RIGHT_HIP, 1.4f },  // pelvis — wide
        { BONE_RIGHT_HIP, BONE_RIGHT_KNEE, 0.9f },
        { BONE_RIGHT_KNEE, BONE_RIGHT_FOOT, 0.6f },
    };
    inline constexpr int CHAMS_BONE_COUNT = sizeof(CHAMS_BONES) / sizeof(BoneWidth);

    // Helper: draw a filled rotated rectangle between two screen points
    inline void DrawFilledBoneQuad(ImDrawList* dl, ImVec2 a, ImVec2 b, float halfWidth, ImU32 col) {
        // Direction vector
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 1.0f) return;
        // Perpendicular normal
        float nx = -dy / len * halfWidth;
        float ny =  dx / len * halfWidth;
        // Four corners of the quad
        ImVec2 p1(a.x + nx, a.y + ny);
        ImVec2 p2(a.x - nx, a.y - ny);
        ImVec2 p3(b.x - nx, b.y - ny);
        ImVec2 p4(b.x + nx, b.y + ny);
        dl->AddQuadFilled(p1, p2, p3, p4, col);
    }

    inline void DrawOverlayChams(ImDrawList* dl, CEntity& enemy, ViewMatrix& vm, int sw, int sh) {
        if (!showChams || !dl) return;
        __try {
            ImU32 fillCol = IM_COL32(
                (int)(chamsColor[0] * 255),
                (int)(chamsColor[1] * 255),
                (int)(chamsColor[2] * 255),
                (int)(chamsColor[3] * 200));

            // Slightly brighter edge color
            ImU32 edgeCol = IM_COL32(
                (int)(fminf(chamsColor[0] * 1.3f, 1.0f) * 255),
                (int)(fminf(chamsColor[1] * 1.3f, 1.0f) * 255),
                (int)(fminf(chamsColor[2] * 1.3f, 1.0f) * 255),
                255);

            // Darker inner color for depth effect
            ImU32 darkCol = IM_COL32(
                (int)(chamsColor[0] * 0.5f * 255),
                (int)(chamsColor[1] * 0.5f * 255),
                (int)(chamsColor[2] * 0.5f * 255),
                (int)(chamsColor[3] * 180));

            // Base width scales with distance (closer = wider)
            Vec3 origin = enemy.GetOrigin();
            Vec2 feetScreen, headScreen;
            Vec3 headW = enemy.GetBonePos(BONE_HEAD);
            if (headW.IsZero()) headW = { origin.x, origin.y, origin.z + 72.0f };
            
            float baseWidth = 8.0f; // default
            if (WorldToScreen(origin, feetScreen, vm, sw, sh) &&
                WorldToScreen(headW, headScreen, vm, sw, sh)) {
                float playerHeight = fabsf(feetScreen.y - headScreen.y);
                baseWidth = playerHeight * 0.12f; // ~12% of visual height
                if (baseWidth < 3.0f) baseWidth = 3.0f;
                if (baseWidth > 25.0f) baseWidth = 25.0f;
            }

            // 3-PASS SMOOTH CAPSULE SILHOUETTE CHAMS
            
            // Pass 1: Outer Edge Outline (Beautiful continuous border)
            for (int c = 0; c < CHAMS_BONE_COUNT; c++) {
                Vec3 boneFrom = enemy.GetBonePos(CHAMS_BONES[c].from);
                Vec3 boneTo = enemy.GetBonePos(CHAMS_BONES[c].to);
                if (boneFrom.IsZero() || boneTo.IsZero()) continue;
                if (boneFrom.DistanceTo(boneTo) > 200.0f) continue;

                Vec2 screenFrom, screenTo;
                if (WorldToScreen(boneFrom, screenFrom, vm, sw, sh) &&
                    WorldToScreen(boneTo, screenTo, vm, sw, sh)) {
                    float hw = baseWidth * CHAMS_BONES[c].width;
                    float outlineWidth = hw + 1.8f; // clean border thickness
                    dl->AddLine(ImVec2(screenFrom.x, screenFrom.y), ImVec2(screenTo.x, screenTo.y), edgeCol, outlineWidth * 2.0f);
                    dl->AddCircleFilled(ImVec2(screenFrom.x, screenFrom.y), outlineWidth, edgeCol, 12);
                    dl->AddCircleFilled(ImVec2(screenTo.x, screenTo.y), outlineWidth, edgeCol, 12);
                }
            }
            // Head Outline
            Vec2 headS;
            if (!headW.IsZero() && WorldToScreen(headW, headS, vm, sw, sh)) {
                float headR = baseWidth * 1.0f;
                dl->AddCircleFilled(ImVec2(headS.x, headS.y), headR + 1.8f, edgeCol, 16);
            }

            // Pass 2: Main Inner Fill (Smooth continuous solid body)
            for (int c = 0; c < CHAMS_BONE_COUNT; c++) {
                Vec3 boneFrom = enemy.GetBonePos(CHAMS_BONES[c].from);
                Vec3 boneTo = enemy.GetBonePos(CHAMS_BONES[c].to);
                if (boneFrom.IsZero() || boneTo.IsZero()) continue;
                if (boneFrom.DistanceTo(boneTo) > 200.0f) continue;

                Vec2 screenFrom, screenTo;
                if (WorldToScreen(boneFrom, screenFrom, vm, sw, sh) &&
                    WorldToScreen(boneTo, screenTo, vm, sw, sh)) {
                    float hw = baseWidth * CHAMS_BONES[c].width;
                    dl->AddLine(ImVec2(screenFrom.x, screenFrom.y), ImVec2(screenTo.x, screenTo.y), fillCol, hw * 2.0f);
                    dl->AddCircleFilled(ImVec2(screenFrom.x, screenFrom.y), hw, fillCol, 12);
                    dl->AddCircleFilled(ImVec2(screenTo.x, screenTo.y), hw, fillCol, 12);
                }
            }
            // Head Fill
            if (!headW.IsZero() && WorldToScreen(headW, headS, vm, sw, sh)) {
                float headR = baseWidth * 1.0f;
                dl->AddCircleFilled(ImVec2(headS.x, headS.y), headR, fillCol, 16);
            }

            // Pass 3: Dark Inner Core (Premium 3D Depth effect)
            for (int c = 0; c < CHAMS_BONE_COUNT; c++) {
                Vec3 boneFrom = enemy.GetBonePos(CHAMS_BONES[c].from);
                Vec3 boneTo = enemy.GetBonePos(CHAMS_BONES[c].to);
                if (boneFrom.IsZero() || boneTo.IsZero()) continue;
                if (boneFrom.DistanceTo(boneTo) > 200.0f) continue;

                Vec2 screenFrom, screenTo;
                if (WorldToScreen(boneFrom, screenFrom, vm, sw, sh) &&
                    WorldToScreen(boneTo, screenTo, vm, sw, sh)) {
                    float hw = baseWidth * CHAMS_BONES[c].width;
                    float innerWidth = hw - 1.2f;
                    if (innerWidth > 0.5f) {
                        dl->AddLine(ImVec2(screenFrom.x, screenFrom.y), ImVec2(screenTo.x, screenTo.y), darkCol, innerWidth * 2.0f);
                        dl->AddCircleFilled(ImVec2(screenFrom.x, screenFrom.y), innerWidth, darkCol, 12);
                        dl->AddCircleFilled(ImVec2(screenTo.x, screenTo.y), innerWidth, darkCol, 12);
                    }
                }
            }
            // Head Inner Core
            if (!headW.IsZero() && WorldToScreen(headW, headS, vm, sw, sh)) {
                float headR = baseWidth * 1.0f;
                float innerR = headR - 1.2f;
                if (innerR > 0.5f) {
                    dl->AddCircleFilled(ImVec2(headS.x, headS.y), innerR, darkCol, 16);
                }
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void ApplyPlayerEffects(uintptr_t pawnAddr, bool isVisible) {
        if (!pawnAddr || !showChams) return;
        __try {
            uint8_t r = (uint8_t)(chamsColor[0] * 255);
            uint8_t g = (uint8_t)(chamsColor[1] * 255);
            uint8_t b = (uint8_t)(chamsColor[2] * 255);
            uint8_t a = 255;

            // === METHOD 1: m_ClientOverrideTint (C_BaseModelEntity) ===
            // This directly tints the 3D model with a solid color
            Memory::Write<bool>(pawnAddr + 0xF5C, true);    // m_bUseClientOverrideTint = true
            Memory::Write<uint8_t>(pawnAddr + 0xF58, r);     // m_ClientOverrideTint.r
            Memory::Write<uint8_t>(pawnAddr + 0xF59, g);     // m_ClientOverrideTint.g
            Memory::Write<uint8_t>(pawnAddr + 0xF5A, b);     // m_ClientOverrideTint.b
            Memory::Write<uint8_t>(pawnAddr + 0xF5B, a);     // m_ClientOverrideTint.a

            // === METHOD 2: m_clrRender (C_BaseModelEntity) ===
            // Standard Source 2 render color - colors the model
            Memory::Write<uint8_t>(pawnAddr + 0xC98, r);     // m_clrRender.r
            Memory::Write<uint8_t>(pawnAddr + 0xC99, g);     // m_clrRender.g
            Memory::Write<uint8_t>(pawnAddr + 0xC9A, b);     // m_clrRender.b
            Memory::Write<uint8_t>(pawnAddr + 0xC9B, a);     // m_clrRender.a

            // === METHOD 3: m_nRenderMode override ===
            // 0=Normal, 1=TransColor, 3=Glow, 9=WorldGlow
            // TransColor(1) makes model use m_clrRender as solid tint
            if (chamsMode == 1) {
                Memory::Write<uint8_t>(pawnAddr + 0xC78, 1);  // kRenderTransColor
            } else if (chamsMode == 3) {
                Memory::Write<uint8_t>(pawnAddr + 0xC78, 3);  // kRenderGlow
            } else if (chamsMode == 9) {
                Memory::Write<uint8_t>(pawnAddr + 0xC78, 9);  // kRenderWorldGlow
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void RemovePlayerEffects(uintptr_t pawnAddr) {
        if (!pawnAddr) return;
        __try {
            // Reset to defaults
            Memory::Write<bool>(pawnAddr + 0xF5C, false);   // m_bUseClientOverrideTint = false
            Memory::Write<uint8_t>(pawnAddr + 0xC78, 0);     // m_nRenderMode = Normal
            Memory::Write<uint8_t>(pawnAddr + 0xC98, 255);   // m_clrRender = white
            Memory::Write<uint8_t>(pawnAddr + 0xC99, 255);
            Memory::Write<uint8_t>(pawnAddr + 0xC9A, 255);
            Memory::Write<uint8_t>(pawnAddr + 0xC9B, 255);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void RemoveGlow(uintptr_t pawnAddr) {
        RemovePlayerEffects(pawnAddr);
    }

    inline void ApplyAntiFlash(uintptr_t localPawn) {
        if (!antiFlash || !localPawn) return;
        __try {
            Memory::Write<float>(localPawn + Offsets::C_CSPlayerPawnBase::m_flFlashDuration, 0.0f);
            Memory::Write<float>(localPawn + Offsets::C_CSPlayerPawnBase::m_flFlashMaxAlpha, 0.0f);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void Render() {
        // Startup guard - skip first 10 frames to let game fully initialize
        static int renderFrameCount = 0;
        if (renderFrameCount < 10) { renderFrameCount++; return; }

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        uintptr_t localPawnAddr = GetLocalPlayerPawn(clientBase);
        if (!localPawnAddr) return;

        Vec3 localPos = {};
        CEntity localPawn(localPawnAddr);
        
        if (localPawn.IsValid()) {
            localPos = localPawn.GetOrigin();
        }
        
        ViewMatrix vm = Memory::Read<ViewMatrix>(clientBase + Offsets::dwViewMatrix);
        
        if (vm.matrix[3][3] == 0.0f && vm.matrix[0][0] == 0.0f) return;

        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        int screenW = (int)displaySize.x;
        int screenH = (int)displaySize.y;

        if (screenW <= 0 || screenH <= 0) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;



        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        // Find local team using pawn directly (more reliable)
        uint8_t localTeam = 0;
        __try {
            localTeam = Memory::Read<uint8_t>(localPawnAddr + Offsets::C_BaseEntity::m_iTeamNum);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}



        int espDrawn = 0, espSkipTeam = 0, espSkipResolve = 0, espSkipHP = 0;

        for (int i = 1; i <= 64; i++) {
            __try {

                
                uintptr_t controller = GetEntityByIndex(entityList, i);
                if (!controller || controller < 0x10000) {
                    continue;
                }
                


                uint8_t alive = Memory::Read<uint8_t>(controller + Offsets::CCSPlayerController::m_bPawnIsAlive);
                if (alive != 1) {
                    continue;
                }
                

                
                uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
                uintptr_t pawn = ResolvePawnFromHandle(entityList, pawnHandle);
                if (!pawn || pawn == localPawnAddr) { 
                    espSkipResolve++; 
                    continue; 
                }
                


                CEntity enemy(pawn);
                int pawnHP = enemy.GetHealth();
                if (pawnHP <= 0) { 
                    espSkipHP++; 
                    continue; 
                }
                


                uint8_t ctrlTeam = Memory::Read<uint8_t>(controller + Offsets::C_BaseEntity::m_iTeamNum);
                if (enemyOnly && localTeam != 0 && ctrlTeam == localTeam) { espSkipTeam++; continue; }


                
                if (showChams) {
                    RealChams::enabled = true;
                    RealChams::enemyVisColor[0] = chamsColor[0];
                    RealChams::enemyVisColor[1] = chamsColor[1];
                    RealChams::enemyVisColor[2] = chamsColor[2];
                    RealChams::enemyVisColor[3] = chamsColor[3];
                } else {
                    RealChams::enabled = false;
                }

                if (showGlow) {
                    uintptr_t spottedS = pawn + Offsets::C_CSPlayerPawn::m_entitySpottedState;
                    Memory::Write<bool>(spottedS + 0x8, true);
                }

                if (antiFlash && localPawnAddr) { ApplyAntiFlash(localPawnAddr); }

                if (showName) {
                    char nameStr[128] = {};
                    bool isBot = false;
                    
                    Memory::ReadString(controller + Offsets::CBasePlayerController::m_iszPlayerName, nameStr, 128);
                    
                    if (nameStr[0] == '\0') {
                        nameStr[0] = '?'; nameStr[1] = 0;
                        isBot = true;
                    } else if (nameStr[0] == 'B' && nameStr[1] == 'o' && nameStr[2] == 't') {
                        isBot = true;
                    }

                    // Use origin-based head position to avoid bone crash
                    Vec3 orig = enemy.GetOrigin();
                    Vec3 headTop = { orig.x, orig.y, orig.z + 82.0f };
                    Vec2 nameScreen;
                    if (WorldToScreen(headTop, nameScreen, vm, screenW, screenH)) {
                        float nameScale = 0.75f;
                        float nameFontSize = ImGui::GetFontSize() * nameScale;

                        bool isVisible = false;
                        __try {
                            int crosshairIdx = Memory::Read<int>(localPawnAddr + Offsets::C_CSPlayerPawn::m_iIDEntIndex);
                            if (crosshairIdx > 0 && crosshairIdx < 64) {
                                uintptr_t crossPawn = ResolvePawnFromHandle(entityList, Memory::Read<uint32_t>(
                                    GetEntityByIndex(entityList, crosshairIdx) + Offsets::CCSPlayerController::m_hPlayerPawn));
                                if (crossPawn == pawn) isVisible = true;
                            }
                        } __except(EXCEPTION_EXECUTE_HANDLER) {}

                        ImU32 nameCol = isVisible ? IM_COL32(255,50,50,255) : IM_COL32(255,255,255,220);
                        float steamIconSize = nameFontSize * 0.9f;
                        
                        if (showSteamInfo) {
                            float iconX = nameScreen.x - ImGui::CalcTextSize(nameStr).x * nameScale / 2 - steamIconSize - 3;
                            float iconY = nameScreen.y - nameFontSize - 2;
                            
                            drawList->AddRectFilled(ImVec2(iconX, iconY), 
                                                  ImVec2(iconX + steamIconSize, iconY + steamIconSize),
                                                  IM_COL32(30,30,50,200), 2.0f);
                            drawList->AddRect(ImVec2(iconX, iconY), 
                                             ImVec2(iconX + steamIconSize, iconY + steamIconSize),
                                             IM_COL32(100,80,160,200), 2.0f);
                            
                            if (isBot) {
                                drawList->AddText(nullptr, steamIconSize * 0.8f, 
                                                ImVec2(iconX + steamIconSize*0.25f, iconY + steamIconSize*0.05f), 
                                                IM_COL32(180,180,180,200), "?");
                            } else {
                                float cx = iconX + steamIconSize/2;
                                float cy = iconY + steamIconSize/2;
                                float r = steamIconSize * 0.15f;
                                drawList->AddCircleFilled(ImVec2(cx, cy - r*1.5f), r, IM_COL32(150,150,200,220));
                                drawList->AddRectFilled(ImVec2(cx-r*1.3f, cy), ImVec2(cx+r*1.3f, cy+r*2.5f), 
                                                      IM_COL32(150,150,200,220), r*0.5f);
                            }
                        }

                        ImVec2 ts = ImGui::CalcTextSize(nameStr);
                        drawList->AddText(nullptr, nameFontSize,
                            ImVec2(nameScreen.x - ts.x * nameScale / 2 + 1, nameScreen.y - nameFontSize - 2 + 1),
                            IM_COL32(0,0,0,160), nameStr);
                        drawList->AddText(nullptr, nameFontSize,
                            ImVec2(nameScreen.x - ts.x * nameScale / 2, nameScreen.y - nameFontSize - 2),
                            nameCol, nameStr);

                        if (showSteamInfo && !isBot) {
                            __try {
                                uint64_t steamID = Memory::Read<uint64_t>(controller + 0x6C0);
                                if (steamID > 76500000000000000ULL && steamID < 76570000000000000ULL) {
                                    char sidBuf[24];
                                    sprintf_s(sidBuf, "%llu", steamID);
                                    float sidScale = nameFontSize * 0.6f;
                                    ImVec2 sidSz = ImGui::CalcTextSize(sidBuf);
                                    float sidW = sidSz.x * (sidScale / ImGui::GetFontSize());
                                    drawList->AddText(nullptr, sidScale,
                                        ImVec2(nameScreen.x - sidW/2, nameScreen.y - nameFontSize - 2 + nameFontSize + 1),
                                        IM_COL32(140,130,170,150), sidBuf);
                                }
                            } __except(EXCEPTION_EXECUTE_HANDLER) {}
                        }
                    }
                }
                


                int armor = Memory::Read<int>(pawn + Offsets::C_CSPlayerPawn::m_ArmorValue);
                if (armor < 0) armor = 0;
                if (armor > 100) armor = 100;

                const char* weaponName = nullptr;
                uint16_t currentWeaponDefIdx = 0;
                __try {
                    uintptr_t ws = enemy.GetWeaponServices();
                    if (ws) {
                        uint32_t wHandle = Memory::Read<uint32_t>(ws + Offsets::CPlayer_WeaponServices::m_hActiveWeapon);
                        if (wHandle && wHandle != 0xFFFFFFFF) {
                            uint32_t wIdx = wHandle & 0x7FFF;
                            uintptr_t wBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (wIdx >> 9));
                            if (wBucket) {
                                uintptr_t weapon = Memory::Read<uintptr_t>(wBucket + ENTITY_STRIDE * (wIdx & 0x1FF));
                                if (weapon) {
                                    currentWeaponDefIdx = Memory::Read<uint16_t>(weapon + Offsets::C_EconEntity::m_AttributeManager
                                        + Offsets::C_AttributeContainer::m_Item + Offsets::C_EconItemView::m_iItemDefinitionIndex);
                                    weaponName = GetWeaponNameByDefIndex(currentWeaponDefIdx);
                                }
                            }
                        }
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {}

                int money = 0;
                __try {
                    uintptr_t moneyServices = Memory::Read<uintptr_t>(controller + 0x808);
                    if (moneyServices) {
                        money = Memory::Read<int>(moneyServices + 0x40);
                        if (money < 0 || money > 65000) money = 0;
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {}

                int ammo = -1;
                __try {
                    uintptr_t ws2 = enemy.GetWeaponServices();
                    if (ws2) {
                        uint32_t wH2 = Memory::Read<uint32_t>(ws2 + Offsets::CPlayer_WeaponServices::m_hActiveWeapon);
                        if (wH2 && wH2 != 0xFFFFFFFF) {
                            uint32_t wI2 = wH2 & 0x7FFF;
                            uintptr_t wB2 = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (wI2 >> 9));
                            if (wB2) {
                                uintptr_t w2 = Memory::Read<uintptr_t>(wB2 + ENTITY_STRIDE * (wI2 & 0x1FF));
                                if (w2) {
                                    ammo = Memory::Read<int>(w2 + 0x16D8);
                                    if (ammo < 0 || ammo > 999) ammo = -1;
                                }
                            }
                        }
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {}

                if (showChams) {
                    DrawOverlayChams(drawList, enemy, vm, screenW, screenH);
                }

                DrawPlayer(drawList, enemy, vm, screenW, screenH, localPos, pawnHP > 100 ? 100 : pawnHP, armor, weaponName, currentWeaponDefIdx, money, ammo);
                espDrawn++;
                
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                // Silently skip entity on exception
            }
        }

        static bool espLogOnce = false;
        if (!espLogOnce && espDrawn > 0) {
            char logBuf[256];
            sprintf_s(logBuf, "[ESP] drawn=%d skipHP=%d skipResolve=%d skipTeam=%d localTeam=%d\r\n",
                    espDrawn, espSkipHP, espSkipResolve, espSkipTeam, localTeam);
            HANDLE hf = CreateFileA("C:\\\\\\dll_log.txt", FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     NULL, OPEN_ALWAYS, 0, NULL);
            if (hf != INVALID_HANDLE_VALUE) {
                DWORD w; WriteFile(hf, logBuf, (DWORD)strlen(logBuf), &w, NULL); CloseHandle(hf);
            }
            espLogOnce = true;
        }

        // ESP debug overlay (top-left corner)
        {
            char dbg[512];
            // Show entity list diagnostics
            uintptr_t bucket0 = Memory::Read<uintptr_t>(entityList + 0x10);
            uintptr_t ent1_78 = 0, ent1_70 = 0;
            if (bucket0) {
                ent1_78 = Memory::Read<uintptr_t>(bucket0 + 0x78 * 1);
                ent1_70 = Memory::Read<uintptr_t>(bucket0 + 0x70 * 1);
            }
            sprintf_s(dbg, "ESP: d=%d skip(hp=%d r=%d t=%d) lt=%d | entList=%llX bkt0=%llX e1@78=%llX e1@70=%llX | box=%d hp=%d",
                espDrawn, espSkipHP, espSkipResolve, espSkipTeam, localTeam,
                (uint64_t)entityList, (uint64_t)bucket0, (uint64_t)ent1_78, (uint64_t)ent1_70,
                (int)showBox, (int)showHealth);
            drawList->AddText(nullptr, 13.0f, ImVec2(10, 40), IM_COL32(255,200,0,220), dbg);
        }

        // ======== DROPPED WEAPON ESP (simplified, crash-safe) ========
        __try {
            if (showDroppedWeapons && entityList) {
                for (int wi = 64; wi < 512; wi++) {
                    __try {
                        uintptr_t wBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((wi & 0x7FFF) >> 9));
                        if (!wBucket) continue;
                        uintptr_t wEnt = Memory::Read<uintptr_t>(wBucket + ENTITY_STRIDE * (wi & 0x1FF));
                        if (!wEnt) continue;

                        uint32_t owner = Memory::Read<uint32_t>(wEnt + 0x520);
                        if (owner != 0xFFFFFFFF) continue;

                        uint16_t defIdx = Memory::Read<uint16_t>(wEnt + Offsets::C_EconEntity::m_AttributeManager + Offsets::C_AttributeContainer::m_Item + Offsets::C_EconItemView::m_iItemDefinitionIndex);
                        const char* wName = GetWeaponNameByDefIndex(defIdx);
                        if (!wName || defIdx == 0) continue;

                        uintptr_t sceneNode = Memory::Read<uintptr_t>(wEnt + Offsets::C_BaseEntity::m_pGameSceneNode);
                        if (!sceneNode) continue;
                        Vec3 wPos = Memory::Read<Vec3>(sceneNode + 0xC8);
                        if (wPos.x == 0 && wPos.y == 0) continue;

                        float dist = localPos.DistanceTo(wPos) * 0.0254f;
                        if (dist > 30.0f) continue;

                        Vec2 wScreen;
                        if (WorldToScreen(wPos, wScreen, vm, screenW, screenH)) {
                            float wfs = ImGui::GetFontSize() * 0.45f;
                            ImVec2 ts = ImGui::CalcTextSize(wName);
                            float sc = wfs / ImGui::GetFontSize();
                            float tw = ts.x * sc;
                            float th = wfs;
                            float pad = 3.0f;
                            float bw = tw + pad * 2;
                            float bh = th + pad * 2;
                            float l = wScreen.x - bw / 2;
                            float t = wScreen.y - bh / 2;
                            float cL = 5.0f;  // corner length
                            ImU32 cCol = IM_COL32(255, 200, 50, 200);
                            
                            // Dark background
                            drawList->AddRectFilled(ImVec2(l, t), ImVec2(l+bw, t+bh),
                                IM_COL32(0,0,0,100), 1.0f);
                            
                            // 4 corner brackets
                            drawList->AddLine(ImVec2(l,t), ImVec2(l+cL,t), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l,t), ImVec2(l,t+cL), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l+bw,t), ImVec2(l+bw-cL,t), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l+bw,t), ImVec2(l+bw,t+cL), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l,t+bh), ImVec2(l+cL,t+bh), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l,t+bh), ImVec2(l,t+bh-cL), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l+bw,t+bh), ImVec2(l+bw-cL,t+bh), cCol, 1.2f);
                            drawList->AddLine(ImVec2(l+bw,t+bh), ImVec2(l+bw,t+bh-cL), cCol, 1.2f);
                            
                            // Weapon name centered
                            drawList->AddText(nullptr, wfs,
                                ImVec2(wScreen.x - tw/2, wScreen.y - th/2),
                                IM_COL32(255, 220, 80, 240), wName);
                            
                            // Distance below
                            char distBuf[16];
                            sprintf_s(distBuf, "%.0fm", dist);
                            ImVec2 ds = ImGui::CalcTextSize(distBuf);
                            float dtw = ds.x * sc * 0.9f;
                            drawList->AddText(nullptr, wfs * 0.9f,
                                ImVec2(wScreen.x - dtw/2, t + bh + 1),
                                IM_COL32(180, 180, 180, 160), distBuf);
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== CUSTOM CROSSHAIR ========
        __try {
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== RAINBOW #ytpavlov (bottom-left, ALWAYS ON) ========
        __try {
            ImDrawList* rdl = ImGui::GetBackgroundDrawList();
            if (rdl) {
                const char* rainbowText = "#ytpavlov";
                float rFontSz = ImGui::GetFontSize() * 0.75f;
                float rx = 12.0f;
                float ry = (float)screenH - rFontSz - 12.0f;
                float rTime = GetTickCount64() * 0.001f;

                // Draw each character with offset hue for rainbow wave
                float charX = rx;
                for (int ci = 0; rainbowText[ci]; ci++) {
                    float hue = fmodf(rTime * 0.8f + ci * 0.12f, 1.0f);
                    // HSV to RGB (simplified)
                    float h6 = hue * 6.0f;
                    int hi = (int)h6 % 6;
                    float f = h6 - (int)h6;
                    float q = 1.0f - f, t2 = f;
                    float cr=1,cg=1,cb=1;
                    switch(hi){
                        case 0: cr=1; cg=t2; cb=0; break;
                        case 1: cr=q; cg=1; cb=0; break;
                        case 2: cr=0; cg=1; cb=t2; break;
                        case 3: cr=0; cg=q; cb=1; break;
                        case 4: cr=t2; cg=0; cb=1; break;
                        case 5: cr=1; cg=0; cb=q; break;
                    }
                    char ch[2] = {rainbowText[ci], 0};
                    // Shadow
                    rdl->AddText(nullptr, rFontSz, ImVec2(charX+1, ry+1), IM_COL32(0,0,0,180), ch);
                    rdl->AddText(nullptr, rFontSz, ImVec2(charX, ry), IM_COL32((int)(cr*255),(int)(cg*255),(int)(cb*255),255), ch);
                    charX += ImGui::CalcTextSize(ch).x * (rFontSz / ImGui::GetFontSize());
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== WATERMARK + FPS (Top-Right Animated Premium Bar) ========
        __try {
            ImDrawList* wdl = ImGui::GetBackgroundDrawList();
            if (wdl) {
                static float fps = 0;
                static float fpsTimer = 0;
                static int frameCount2 = 0;
                frameCount2++;
                float curFrameTime = ImGui::GetIO().DeltaTime;
                fpsTimer += curFrameTime;
                if (fpsTimer >= 0.5f) { fps = frameCount2 / fpsTimer; frameCount2 = 0; fpsTimer = 0; }

                SYSTEMTIME st; GetLocalTime(&st);
                char clockBuf[8]; sprintf_s(clockBuf, "%02d:%02d", st.wHour, st.wMinute);

                float wmFontSz = ImGui::GetFontSize() * 0.6f; // Bigger font
                float wmScale = wmFontSz / ImGui::GetFontSize();
                char wmBuf[96]; sprintf_s(wmBuf, "ytpavlov.com  |  %d fps  |  %s  |  v1", (int)fps, clockBuf);
                ImVec2 wmSz = ImGui::CalcTextSize(wmBuf);
                float wmW = wmSz.x * wmScale;
                float wmH = wmFontSz + 14; // Taller bar
                float pad = 10.0f;
                float barW = wmW + pad * 2;
                float wmX = (float)screenW - barW - 8;
                float wmY = 4;
                float t = GetTickCount64() * 0.001f;

                // Animated gradient background — PURE PURPLE tones only
                float gp = t * 0.3f;
                int r1=(int)(sinf(gp)*15+50), g1=(int)(sinf(gp+2)*8+10), b1=(int)(sinf(gp+4)*20+80);
                int r2=(int)(sinf(gp+1.5f)*15+45), g2=(int)(sinf(gp+3.5f)*8+10), b2=(int)(sinf(gp+5.5f)*20+85);
                // Clamp to purple range
                if(g1>25)g1=25; if(g2>25)g2=25; if(r1>70)r1=70; if(r2>70)r2=70;
                wdl->AddRectFilledMultiColor(ImVec2(wmX,wmY), ImVec2(wmX+barW,wmY+wmH),
                    IM_COL32(r1,g1,b1,220), IM_COL32(r2,g2,b2,220), IM_COL32(r1+10,g1,b1+15,210), IM_COL32(r2-5,g2,b2+10,210));

                // Glass overlay
                wdl->AddRectFilledMultiColor(ImVec2(wmX,wmY), ImVec2(wmX+barW,wmY+wmH*0.35f),
                    IM_COL32(255,255,255,15), IM_COL32(255,255,255,15), IM_COL32(255,255,255,0), IM_COL32(255,255,255,0));

                // Rounded border pulse — PURPLE only
                float bp = sinf(t*2)*0.3f+0.7f;
                wdl->AddRect(ImVec2(wmX,wmY), ImVec2(wmX+barW,wmY+wmH), IM_COL32(120,60,200,(int)(bp*180)), 6.0f, 0, 1.2f);

                // Snow particles inside bar
                struct WmSnow { float x,y,spd,sz; };
                static WmSnow snow[16]={};
                static bool snowReady=false;
                if(!snowReady){ for(int s=0;s<16;s++){ snow[s].x=(rand()%1000)/1000.f; snow[s].y=(rand()%1000)/1000.f; snow[s].spd=0.12f+(rand()%100)*0.003f; snow[s].sz=0.6f+(rand()%100)*0.012f; } snowReady=true; }
                for(int s=0;s<16;s++){
                    snow[s].y+=snow[s].spd*curFrameTime;
                    snow[s].x+=sinf(t*1.5f+s*0.7f)*0.001f;
                    if(snow[s].y>1.0f){ snow[s].y=-0.1f; snow[s].x=(rand()%1000)/1000.f; }
                    float sx2=wmX+snow[s].x*barW, sy2=wmY+snow[s].y*wmH;
                    if(sy2>=wmY && sy2<=wmY+wmH){
                        float a2=1.0f-fabsf(snow[s].y-0.5f)*1.5f; if(a2<0)a2=0;
                        wdl->AddCircleFilled(ImVec2(sx2,sy2), snow[s].sz, IM_COL32(200,180,255,(int)(a2*60)), 5);
                    }
                }

                // Wave shimmer at bottom — purple tint
                for(float wx=wmX;wx<wmX+barW;wx+=3.0f){
                    float wy=wmY+wmH-1.5f+sinf(wx*0.08f+t*3)*1.0f;
                    float wa=(sinf(wx*0.05f+t*2)+1)*0.5f;
                    wdl->AddCircleFilled(ImVec2(wx,wy), 0.8f+wa*0.4f, IM_COL32(140,80,220,(int)(wa*60+20)), 4);
                }

                // Text shadow + content
                float textX=wmX+pad, textY=wmY+6;
                wdl->AddText(nullptr, wmFontSz, ImVec2(textX+1,textY+1), IM_COL32(0,0,0,140), wmBuf);

                // "ytpavlov.com" — purple/violet animated color
                const char* namePart="ytpavlov.com";
                float nh=fmodf(t*0.3f,1.0f);
                int nr2=(int)((sinf(nh*6.28f)*0.2f+0.6f)*180+50);
                int ng2=(int)((sinf(nh*6.28f+2.1f)*0.1f+0.15f)*100+20);
                int nb2=(int)((sinf(nh*6.28f+4.2f)*0.2f+0.8f)*255);
                // Keep purple: clamp green low
                if(ng2>80)ng2=80;
                wdl->AddText(nullptr, wmFontSz, ImVec2(textX,textY), IM_COL32(nr2,ng2,nb2,255), namePart);
                float nameW2=ImGui::CalcTextSize(namePart).x*wmScale;

                char restBuf[64]; sprintf_s(restBuf, "  |  %d fps  |  %s  |  v1", (int)fps, clockBuf);
                wdl->AddText(nullptr, wmFontSz, ImVec2(textX+nameW2,textY), IM_COL32(220,210,240,240), restBuf);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== FOV CIRCLE ========
        __try {
            if (showFovCircle && Aimbot::enabled) {
                ImDrawList* fdl = ImGui::GetBackgroundDrawList();
                if (fdl) {
                    float fovPixels = (Aimbot::fov / 90.0f) * (screenW / 2.0f);
                    fdl->AddCircle(ImVec2(screenW/2.0f, screenH/2.0f), fovPixels,
                        IM_COL32(130, 80, 255, 120), 64, 1.0f);
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== BOMB TIMER ========
        __try {
            if (showBombTimer && clientBase) {
                // dwPlantedC4 is a pointer to the C_PlantedC4 entity directly
                uintptr_t plantedC4List = Memory::Read<uintptr_t>(clientBase + Offsets::dwPlantedC4);
                if (plantedC4List) {
                    // Read first entry (entity pointer)
                    uintptr_t c4Ent = Memory::Read<uintptr_t>(plantedC4List);
                    if (c4Ent && c4Ent > 0x10000) {
                        uintptr_t globalVars = Memory::Read<uintptr_t>(clientBase + Offsets::dwGlobalVars);
                        float curTime = 0;
                        if (globalVars) curTime = Memory::Read<float>(globalVars + Offsets::GlobalVar::CurrentTime);

                        float c4Blow = Memory::Read<float>(c4Ent + Offsets::C_PlantedC4::m_flC4Blow);
                        bool beingDefused = Memory::Read<bool>(c4Ent + Offsets::C_PlantedC4::m_bBeingDefused);
                        float defuseEnd = Memory::Read<float>(c4Ent + Offsets::C_PlantedC4::m_flDefuseCountDown);
                        int bombSite = Memory::Read<int>(c4Ent + Offsets::C_PlantedC4::m_nBombSite);
                        bool defused = Memory::Read<bool>(c4Ent + 0x11B4); // m_bBombDefused

                        float timeLeft = c4Blow - curTime;
                        char siteName = (bombSite == 0) ? 'A' : 'B';

                        ImDrawList* bdl = ImGui::GetBackgroundDrawList();
                        // Only show if bomb is still relevant
                        // Sanity: CS2 bomb is max 40s, if timeLeft > 45 it's stale/invalid data
                        if (bdl && c4Blow > 0 && curTime > 0 && timeLeft > -3.0f && timeLeft < 45.0f && !defused) {
                            // ===== PREMIUM BOMB INFO PANEL =====
                            float bw = 260, bh = 70;
                            float bx = (screenW - bw) / 2;
                            float by = screenH * 0.18f;

                            // Dark rounded background with red border glow
                            ImU32 bgCol = IM_COL32(15, 10, 10, 220);
                            ImU32 borderCol = (timeLeft < 5.0f) ? IM_COL32(255, 50, 50, 200) : IM_COL32(200, 80, 40, 180);
                            bdl->AddRectFilled(ImVec2(bx, by), ImVec2(bx+bw, by+bh), bgCol, 10.0f);
                            bdl->AddRect(ImVec2(bx, by), ImVec2(bx+bw, by+bh), borderCol, 10.0f, 0, 1.5f);

                            // === CIRCULAR TIMER (left side) ===
                            float circR = 22.0f;
                            float circX = bx + 40;
                            float circY = by + bh/2;
                            float barFrac = timeLeft / 40.0f;
                            if (barFrac > 1.0f) barFrac = 1.0f;
                            if (barFrac < 0.0f) barFrac = 0.0f;

                            // Background circle
                            bdl->AddCircle(ImVec2(circX, circY), circR, IM_COL32(60, 30, 30, 150), 32, 2.5f);
                            // Arc fill (countdown)
                            float startAngle = -3.14159f / 2.0f; // top
                            float endAngle = startAngle + barFrac * 3.14159f * 2.0f;
                            ImU32 arcCol = (timeLeft < 5.0f) ? IM_COL32(255, 50, 50, 255) : IM_COL32(255, 140, 40, 255);
                            // Draw arc segments
                            int segments = (int)(barFrac * 32);
                            if (segments < 2) segments = 2;
                            for (int s = 0; s < segments; s++) {
                                float a1 = startAngle + (endAngle - startAngle) * s / segments;
                                float a2 = startAngle + (endAngle - startAngle) * (s + 1) / segments;
                                bdl->AddLine(
                                    ImVec2(circX + cosf(a1)*circR, circY + sinf(a1)*circR),
                                    ImVec2(circX + cosf(a2)*circR, circY + sinf(a2)*circR),
                                    arcCol, 3.0f);
                            }

                            // Timer text in center of circle
                            char secBuf[8];
                            sprintf_s(secBuf, "%ds", (int)timeLeft);
                            ImVec2 secSz = ImGui::CalcTextSize(secBuf);
                            float secFontSz = ImGui::GetFontSize() * 0.6f;
                            float secScale = secFontSz / ImGui::GetFontSize();
                            bdl->AddText(nullptr, secFontSz, ImVec2(circX - secSz.x*secScale/2, circY - secFontSz/2), IM_COL32(255,255,255,255), secBuf);

                            // === TEXT INFO (right side) ===
                            float txtX = bx + 75;
                            float txtY1 = by + 12;
                            float mainFontSz = ImGui::GetFontSize() * 0.7f;
                            float subFontSz = ImGui::GetFontSize() * 0.55f;

                            // "Bomba [A/B] noktasinda!"
                            char bombTxt[48];
                            sprintf_s(bombTxt, "Bomba  %c  noktasinda!", siteName);
                            bdl->AddText(nullptr, mainFontSz, ImVec2(txtX+1, txtY1+1), IM_COL32(0,0,0,200), bombTxt);
                            bdl->AddText(nullptr, mainFontSz, ImVec2(txtX, txtY1), IM_COL32(255,255,255,245), bombTxt);

                            // Status line
                            float txtY2 = txtY1 + mainFontSz + 6;
                            const char* statusLabel = "Durum: ";
                            const char* statusVal;
                            ImU32 statusCol;
                            if (beingDefused) {
                                statusVal = "IMHA EDILIYOR";
                                statusCol = IM_COL32(50, 200, 255, 255);
                            } else if (timeLeft < 5.0f) {
                                statusVal = "TEHLIKELI";
                                statusCol = IM_COL32(255, 50, 50, 255);
                            } else {
                                statusVal = "AKTIF";
                                statusCol = IM_COL32(255, 180, 40, 255);
                            }
                            bdl->AddText(nullptr, subFontSz, ImVec2(txtX, txtY2), IM_COL32(180,180,180,200), statusLabel);
                            float labelW = ImGui::CalcTextSize(statusLabel).x * (subFontSz / ImGui::GetFontSize());
                            bdl->AddText(nullptr, subFontSz, ImVec2(txtX + labelW, txtY2), statusCol, statusVal);
                        }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== C4 PLANTED LOCATION ESP (World Marker) ========
        __try {
            if (showBombESP && clientBase) {
                uintptr_t plantedC4List = Memory::Read<uintptr_t>(clientBase + Offsets::dwPlantedC4);
                if (plantedC4List) {
                    uintptr_t c4Ent = Memory::Read<uintptr_t>(plantedC4List);
                    if (c4Ent && c4Ent > 0x10000) {
                        // Get C4 world position via SceneNode
                        uintptr_t c4SceneNode = Memory::Read<uintptr_t>(c4Ent + Offsets::C_BaseEntity::m_pGameSceneNode);
                        Vec3 c4Pos = {};
                        if (c4SceneNode) {
                            c4Pos = Memory::Read<Vec3>(c4SceneNode + 0xC8); // m_vecAbsOrigin
                        }
                        
                        if (c4Pos.x != 0 || c4Pos.y != 0) {
                            bool defused = Memory::Read<bool>(c4Ent + 0x11B4);
                            int bombSite = Memory::Read<int>(c4Ent + Offsets::C_PlantedC4::m_nBombSite);
                            float dist = localPos.DistanceTo(c4Pos) * 0.0254f;
                            char siteChar = (bombSite == 0) ? 'A' : 'B';
                            
                            if (!defused) {
                                Vec2 c4Screen;
                                ImDrawList* bdl = ImGui::GetBackgroundDrawList();
                                if (bdl) {
                                    if (WorldToScreen(c4Pos, c4Screen, vm, screenW, screenH)) {
                                        // ON-SCREEN: Draw C4 marker at bomb position
                                        float pulse = (sinf(GetTickCount64() * 0.005f) + 1.0f) * 0.5f; // 0-1 pulse
                                        float radius = 8.0f + pulse * 4.0f;
                                        ImU32 bombCol = IM_COL32(255, 50, 50, (int)(180 + pulse * 75));
                                        ImU32 glowCol = IM_COL32(255, 80, 30, (int)(60 + pulse * 40));
                                        
                                        // Glow
                                        bdl->AddCircleFilled(ImVec2(c4Screen.x, c4Screen.y), radius + 6, glowCol, 16);
                                        // Inner circle
                                        bdl->AddCircleFilled(ImVec2(c4Screen.x, c4Screen.y), radius, bombCol, 16);
                                        // C4 icon (small bomb text)
                                        bdl->AddText(nullptr, 10.0f, ImVec2(c4Screen.x - 5, c4Screen.y - 5), IM_COL32(255,255,255,255), "C4");
                                        
                                        // Site + Distance label
                                        char c4Label[32];
                                        sprintf_s(c4Label, "BOMBA [%c] %.0fm", siteChar, dist);
                                        float labelFontSz = ImGui::GetFontSize() * 0.5f;
                                        ImVec2 labelSz = ImGui::CalcTextSize(c4Label);
                                        float lw = labelSz.x * (labelFontSz / ImGui::GetFontSize());
                                        
                                        // Label background
                                        bdl->AddRectFilled(
                                            ImVec2(c4Screen.x - lw/2 - 3, c4Screen.y + radius + 4),
                                            ImVec2(c4Screen.x + lw/2 + 3, c4Screen.y + radius + 4 + labelFontSz + 4),
                                            IM_COL32(0,0,0,180), 3.0f);
                                        bdl->AddText(nullptr, labelFontSz,
                                            ImVec2(c4Screen.x - lw/2, c4Screen.y + radius + 6),
                                            IM_COL32(255, 100, 50, 255), c4Label);
                                    } else {
                                        // OFF-SCREEN: Draw edge arrow pointing to C4
                                        float cx = (float)screenW / 2;
                                        float cy = (float)screenH / 2;
                                        
                                        // Calculate direction from screen center to C4 world pos
                                        // Use a simplified 2D projection
                                        float dx = c4Pos.x - localPos.x;
                                        float dy = c4Pos.y - localPos.y;
                                        float angle = atan2f(dy, dx);
                                        
                                        // Edge position (with margin)
                                        float margin = 40.0f;
                                        float edgeX = cx + cosf(angle) * (screenW/2 - margin);
                                        float edgeY = cy - sinf(angle) * (screenH/2 - margin);
                                        
                                        // Clamp to screen edges
                                        if (edgeX < margin) edgeX = margin;
                                        if (edgeX > screenW - margin) edgeX = (float)(screenW - margin);
                                        if (edgeY < margin) edgeY = margin;
                                        if (edgeY > screenH - margin) edgeY = (float)(screenH - margin);
                                        
                                        float pulse2 = (sinf(GetTickCount64() * 0.005f) + 1.0f) * 0.5f;
                                        ImU32 arrowCol = IM_COL32(255, 60, 30, (int)(200 + pulse2 * 55));
                                        
                                        // Arrow triangle pointing outward
                                        float arrowSize = 12.0f;
                                        float aAngle = atan2f(edgeY - cy, edgeX - cx);
                                        ImVec2 tip(edgeX + cosf(aAngle) * arrowSize, edgeY + sinf(aAngle) * arrowSize);
                                        ImVec2 l(edgeX + cosf(aAngle + 2.5f) * arrowSize, edgeY + sinf(aAngle + 2.5f) * arrowSize);
                                        ImVec2 r(edgeX + cosf(aAngle - 2.5f) * arrowSize, edgeY + sinf(aAngle - 2.5f) * arrowSize);
                                        bdl->AddTriangleFilled(tip, l, r, arrowCol);
                                        
                                        // Distance text
                                        char distBuf[16];
                                        sprintf_s(distBuf, "%.0fm", dist);
                                        float dfSz = ImGui::GetFontSize() * 0.45f;
                                        ImVec2 dSz = ImGui::CalcTextSize(distBuf);
                                        float dw = dSz.x * (dfSz / ImGui::GetFontSize());
                                        bdl->AddText(nullptr, dfSz,
                                            ImVec2(edgeX - dw/2, edgeY - arrowSize - dfSz - 2),
                                            IM_COL32(255, 200, 50, 220), distBuf);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== SMOKE / MOLOTOV / HE AREA TIMER ========
        __try {
            if (entityList && clientBase) {
                // Scan for active grenades/projectiles in entity list
                for (int gIdx = 1; gIdx < 64; gIdx++) {
                    __try {
                        uintptr_t gBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (gIdx >> 9));
                        if (!gBucket) continue;
                        uintptr_t gEnt = Memory::Read<uintptr_t>(gBucket + 0x78 * (gIdx & 0x1FF));
                        if (!gEnt || gEnt < 0x10000) continue;

                        // Read entity class name hash via schema  
                        // We check m_iHealth == 1 as a proxy for "active projectile" entity
                        // and use the designer name to detect type
                        uintptr_t gSceneNode = Memory::Read<uintptr_t>(gEnt + Offsets::C_BaseEntity::m_pGameSceneNode);
                        if (!gSceneNode) continue;
                        Vec3 gPos = Memory::Read<Vec3>(gSceneNode + 0xC8);
                        if (gPos.IsZero()) continue;

                        // Check if it's a smoke or inferno via m_nSubclassID or classname
                        // Inferno entities have m_fireCount at specific offset
                        int fireCount = Memory::Read<int>(gEnt + 0x12D0); // C_Inferno::m_fireCount
                        
                        if (fireCount > 0 && fireCount < 200) {
                            // This is a MOLOTOV/INCENDIARY fire zone
                            Vec2 fireScreen;
                            if (WorldToScreen(gPos, fireScreen, vm, screenW, screenH)) {
                                ImDrawList* fdl = ImGui::GetBackgroundDrawList();
                                if (fdl) {
                                    // Fire area indicator
                                    float pulse = (sinf(GetTickCount64() * 0.008f) + 1.0f) * 0.5f;
                                    float baseRadius = 30.0f + fireCount * 0.8f;
                                    float radius = baseRadius + pulse * 5.0f;
                                    
                                    // Fire zone circle (orange-red gradient)
                                    fdl->AddCircle(ImVec2(fireScreen.x, fireScreen.y), radius,
                                        IM_COL32(255, 100, 0, (int)(120 + pulse*80)), 24, 2.0f);
                                    fdl->AddCircleFilled(ImVec2(fireScreen.x, fireScreen.y), radius * 0.3f,
                                        IM_COL32(255, 60, 0, (int)(40 + pulse*30)), 12);

                                    // Read fire duration (approx 7 seconds total)
                                    uintptr_t gGlobalVars = Memory::Read<uintptr_t>(clientBase + Offsets::dwGlobalVars);
                                    float gCurTime = 0;
                                    if (gGlobalVars) gCurTime = Memory::Read<float>(gGlobalVars + Offsets::GlobalVar::CurrentTime);
                                    float fireStartTime = Memory::Read<float>(gEnt + 0x1298); // approximate
                                    float fireTimeLeft = 7.0f;
                                    if (fireStartTime > 0 && gCurTime > 0) {
                                        fireTimeLeft = 7.0f - (gCurTime - fireStartTime);
                                        if (fireTimeLeft < 0) fireTimeLeft = 0;
                                    }

                                    // Timer text
                                    char fireBuf[32];
                                    sprintf_s(fireBuf, "MOLOTOV %.1fs", fireTimeLeft);
                                    float ffSz = ImGui::GetFontSize() * 0.5f;
                                    ImVec2 fts = ImGui::CalcTextSize(fireBuf);
                                    float ftw = fts.x * (ffSz / ImGui::GetFontSize());
                                    fdl->AddRectFilled(
                                        ImVec2(fireScreen.x - ftw/2 - 3, fireScreen.y - radius - ffSz - 6),
                                        ImVec2(fireScreen.x + ftw/2 + 3, fireScreen.y - radius - 2),
                                        IM_COL32(0,0,0,160), 3.0f);
                                    fdl->AddText(nullptr, ffSz,
                                        ImVec2(fireScreen.x - ftw/2, fireScreen.y - radius - ffSz - 4),
                                        IM_COL32(255, 150, 50, 255), fireBuf);
                                }
                            }
                        }

                        // Check for smoke grenade (m_bSmokeEffectSpawned)
                        bool smokeActive = Memory::Read<bool>(gEnt + 0x1284); // m_bSmokeEffectSpawned
                        int smokeTickBegin = Memory::Read<int>(gEnt + 0x1288); // m_nSmokeEffectTickBegin
                        
                        if (smokeActive && smokeTickBegin > 0) {
                            Vec2 smokeScreen;
                            if (WorldToScreen(gPos, smokeScreen, vm, screenW, screenH)) {
                                ImDrawList* sdl2 = ImGui::GetBackgroundDrawList();
                                if (sdl2) {
                                    float pulse2 = (sinf(GetTickCount64() * 0.004f) + 1.0f) * 0.5f;
                                    float smokeRadius = 50.0f + pulse2 * 8.0f;

                                    // Smoke area circle (gray/blue)
                                    sdl2->AddCircle(ImVec2(smokeScreen.x, smokeScreen.y), smokeRadius,
                                        IM_COL32(150, 180, 220, (int)(80 + pulse2*60)), 24, 2.0f);
                                    sdl2->AddCircleFilled(ImVec2(smokeScreen.x, smokeScreen.y), smokeRadius * 0.4f,
                                        IM_COL32(150, 180, 220, (int)(20 + pulse2*20)), 16);

                                    // Smoke timer (18 seconds total)
                                    uintptr_t sGlobalVars = Memory::Read<uintptr_t>(clientBase + Offsets::dwGlobalVars);
                                    float sCurTime = 0;
                                    if (sGlobalVars) sCurTime = Memory::Read<float>(sGlobalVars + Offsets::GlobalVar::CurrentTime);
                                    float smokeTimeElapsed = 0;
                                    if (sGlobalVars) {
                                        float tickInterval = Memory::Read<float>(sGlobalVars + Offsets::GlobalVar::IntervalPerTick);
                                        if (tickInterval > 0) smokeTimeElapsed = (sCurTime / tickInterval - smokeTickBegin) * tickInterval;
                                    }
                                    float smokeTimeLeft = 18.0f - smokeTimeElapsed;
                                    if (smokeTimeLeft < 0) smokeTimeLeft = 0;

                                    char smokeBuf[32];
                                    sprintf_s(smokeBuf, "SMOKE %.1fs", smokeTimeLeft);
                                    float sfSz = ImGui::GetFontSize() * 0.5f;
                                    ImVec2 sts = ImGui::CalcTextSize(smokeBuf);
                                    float stw = sts.x * (sfSz / ImGui::GetFontSize());
                                    sdl2->AddRectFilled(
                                        ImVec2(smokeScreen.x - stw/2 - 3, smokeScreen.y - smokeRadius - sfSz - 6),
                                        ImVec2(smokeScreen.x + stw/2 + 3, smokeScreen.y - smokeRadius - 2),
                                        IM_COL32(0,0,0,160), 3.0f);
                                    sdl2->AddText(nullptr, sfSz,
                                        ImVec2(smokeScreen.x - stw/2, smokeScreen.y - smokeRadius - sfSz - 4),
                                        IM_COL32(150, 200, 255, 255), smokeBuf);
                                }
                            }
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        // ======== GRENADE & MOLOTOV ESP ========
        __try {
            ImDrawList* gdl = ImGui::GetBackgroundDrawList();
            if (gdl && entityList) { // Add your panel toggle variable here if you have one, e.g. showGrenadeESP
                for (int i = 65; i < 1024; i++) {
                    __try {
                        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((i & 0x7FFF) >> 9));
                        if (!bucket) continue;
                        uintptr_t entity = Memory::Read<uintptr_t>(bucket + ENTITY_STRIDE * (i & 0x1FF));
                        if (!entity) continue;
                        
                        CEntity ent(entity);
                        char className[64] = {0};
                        ent.GetClassName(className, sizeof(className));
                        if (className[0] == '\0') continue;

                        bool isSmoke = (strcmp(className, "smokegrenade_projectile") == 0);
                        bool isInferno = (strcmp(className, "inferno") == 0); // Molotov fire

                        if (isSmoke || isInferno) {
                            Vec3 origin = ent.GetOrigin();
                            if (origin.IsZero()) continue; // No position

                            Vec2 screenPos;
                            if (WorldToScreen(origin, screenPos, vm, screenW, screenH)) {
                                float radius = isSmoke ? 144.0f : 120.0f; // Approx visual radius
                                float screenRadius = radius / max(1.0f, localPawn.GetOrigin().DistanceTo(origin)) * 500.0f;
                                if (screenRadius > 200.0f) screenRadius = 200.0f;
                                if (screenRadius < 10.0f) screenRadius = 10.0f;

                                ImU32 colBg = isSmoke ? IM_COL32(150, 150, 150, 40) : IM_COL32(255, 100, 0, 40);
                                ImU32 colBorder = isSmoke ? IM_COL32(200, 200, 200, 180) : IM_COL32(255, 60, 0, 180);
                                const char* label = isSmoke ? "SMOKE" : "MOLOTOV";

                                // Draw circle on ground
                                gdl->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), screenRadius, colBg, 32);
                                gdl->AddCircle(ImVec2(screenPos.x, screenPos.y), screenRadius, colBorder, 32, 2.0f);

                                // Draw icon/label
                                ImVec2 labelSz = ImGui::CalcTextSize(label);
                                gdl->AddRectFilled(
                                    ImVec2(screenPos.x - labelSz.x/2 - 6, screenPos.y - labelSz.y/2 - 4),
                                    ImVec2(screenPos.x + labelSz.x/2 + 6, screenPos.y + labelSz.y/2 + 4),
                                    IM_COL32(0,0,0,180), 6.0f
                                );
                                gdl->AddText(ImVec2(screenPos.x - labelSz.x/2, screenPos.y - labelSz.y/2), colBorder, label);
                            }
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        // ======== SPECTATOR LIST ========
        __try {
            if (showSpectators && entityList && localPawnAddr) {
                ImDrawList* sdl = ImGui::GetBackgroundDrawList();
                if (sdl) {
                    float sx = screenW - 220; // Move left a bit
                    float sy = 120; // Move down a bit
                    int specCount = 0;
                    char specNames[8][64] = {};

                    for (int si = 1; si <= 64; si++) {
                        __try {
                            uintptr_t sBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((si & 0x7FFF) >> 9));
                            if (!sBucket) continue;
                            uintptr_t sCtrl = Memory::Read<uintptr_t>(sBucket + ENTITY_STRIDE * (si & 0x1FF));
                            if (!sCtrl) continue;

                            // Get this controller's pawn handle
                            uint32_t sPawnHandle = Memory::Read<uint32_t>(sCtrl + Offsets::CCSPlayerController::m_hPlayerPawn);
                            if (!sPawnHandle || sPawnHandle == 0xFFFFFFFF) continue;

                            // Resolve pawn
                            uint32_t sPawnIdx = sPawnHandle & 0x7FFF;
                            uintptr_t sPawnBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (sPawnIdx >> 9));
                            if (!sPawnBucket) continue;
                            uintptr_t sPawn = Memory::Read<uintptr_t>(sPawnBucket + ENTITY_STRIDE * (sPawnIdx & 0x1FF));
                            if (!sPawn || sPawn == localPawnAddr) continue;

                            // Check if this pawn is observing US
                            uintptr_t obsSvc = Memory::Read<uintptr_t>(sPawn + Offsets::C_BasePlayerPawn::m_pObserverServices);
                            if (!obsSvc) continue;
                            uint32_t obsTarget = Memory::Read<uint32_t>(obsSvc + Offsets::ObserverServices::m_hObserverTarget);
                            if (!obsTarget || obsTarget == 0xFFFFFFFF) continue;

                            // Check if observer target matches our pawn handle
                            uintptr_t localCtrl = Memory::Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerController);
                            if (!localCtrl) continue;
                            uint32_t localPawnHandle = Memory::Read<uint32_t>(localCtrl + Offsets::CCSPlayerController::m_hPlayerPawn);

                            if (obsTarget == localPawnHandle && specCount < 8) {
                                if (specCount == 0) Log("[ESP] Player 1 text rendering\r\n");
                                char name[64] = {};
                                for (int c = 0; c < 63; c++) {
                                    name[c] = Memory::Read<char>(sCtrl + Offsets::CBasePlayerController::m_iszPlayerName + c);
                                    if (name[c] == 0) break;
                                }
                                if (name[0]) {
                                    strcpy_s(specNames[specCount], name);
                                    specCount++;
                                }
                            }
                        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
                    }

                    if (specCount > 0) {
                        float boxW = 200;
                        float hdrH = 34;
                        float itemH = 24;
                        float boxH = hdrH + specCount * itemH + 8;
                        float t = GetTickCount64() * 0.001f;
                        float pulse = sinf(t * 3) * 0.5f + 0.5f;

                        // Premium Panel Background
                        sdl->AddRectFilled(ImVec2(sx, sy), ImVec2(sx + boxW, sy + boxH), IM_COL32(15, 15, 20, 220), 8.0f);
                        
                        // Header Gradient
                        sdl->AddRectFilledMultiColor(ImVec2(sx, sy), ImVec2(sx + boxW, sy + hdrH),
                            IM_COL32(130, 80, 255, 150), IM_COL32(180, 60, 255, 80), IM_COL32(180, 60, 255, 0), IM_COL32(130, 80, 255, 0));
                        
                        // Border Pulse
                        sdl->AddRect(ImVec2(sx, sy), ImVec2(sx + boxW, sy + boxH), IM_COL32(130, 80, 255, (int)(100 + pulse*80)), 8.0f, 0, 1.5f);
                        
                        // Title
                        char titleBuf[32];
                        sprintf_s(titleBuf, "IZLEYENLER (%d)", specCount);
                        sdl->AddText(nullptr, ImGui::GetFontSize() * 1.1f, ImVec2(sx + 35, sy + 8), IM_COL32(255,255,255,255), titleBuf);
                        
                        // Eye Icon (simulated with basic geometry)
                        float ex = sx + 18, ey = sy + 16;
                        sdl->AddCircleFilled(ImVec2(ex, ey), 8, IM_COL32(130, 80, 255, 255));
                        sdl->AddCircleFilled(ImVec2(ex, ey), 4, IM_COL32(0, 0, 0, 255));
                        sdl->AddCircleFilled(ImVec2(ex + 1, ey - 1), 1.5f, IM_COL32(255, 255, 255, 200));

                        // Divider
                        sdl->AddLine(ImVec2(sx + 10, sy + hdrH), ImVec2(sx + boxW - 10, sy + hdrH), IM_COL32(130, 80, 255, 100), 1.0f);
                        
                        // Spectator List
                        for (int s = 0; s < specCount; s++) {
                            float itemY = sy + hdrH + 6 + s * itemH;
                            // Avatar placeholder
                            sdl->AddRectFilled(ImVec2(sx + 12, itemY), ImVec2(sx + 12 + 16, itemY + 16), IM_COL32(60, 60, 70, 255), 4.0f);
                            // Name
                            sdl->AddText(ImVec2(sx + 36, itemY + 1), IM_COL32(220, 220, 220, 255), specNames[s]);
                        }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

    }
}

 
 
