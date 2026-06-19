#pragma once
#include <windows.h>
#include <cstdint>
#include <cmath>
#include "..\imgui\imgui.h"
#include "..\Core\Memory.h"
#include "..\SDK\Offsets.h"

namespace GrenadePrediction {

    inline bool enabled = true;

    constexpr float GRAVITY = 800.0f;
    constexpr float TIME_STEP = 0.016f;
    constexpr int MAX_STEPS = 128;

    struct projectile_t {
        Vec3 pos;
        Vec3 vel;
        float liveTime;
        int type;
    };

    inline void Render(ImDrawList* dl, uintptr_t localPawn, uintptr_t clientBase, const ViewMatrix& vm, int screenW, int screenH) {
        if (!enabled || !dl || !localPawn) return;

        uintptr_t weaponSvc = Memory::Read<uintptr_t>(localPawn + Offsets::C_BasePlayerPawn::m_pWeaponServices);
        if (!weaponSvc) return;

        uint32_t hWeapon = Memory::Read<uint32_t>(weaponSvc + Offsets::CPlayer_WeaponServices::m_hActiveWeapon);
        if (!hWeapon || hWeapon == 0xFFFFFFFF) return;

        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        uint32_t idx = hWeapon & 0x7FFF;
        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
        if (!bucket) return;
        uintptr_t weapon = Memory::Read<uintptr_t>(bucket + 0x70 * (idx & 0x1FF));
        if (!weapon) return;

        uintptr_t item = weapon + Offsets::C_EconEntity::m_AttributeManager + Offsets::C_AttributeContainer::m_Item;
        uint16_t defIdx = Memory::Read<uint16_t>(item + Offsets::C_EconItemView::m_iItemDefinitionIndex);

        bool isGrenade = false;
        float speed = 0.0f;
        int type = 0;

        switch (defIdx) {
            case 43: isGrenade = true; speed = 560.0f; type = 0; break; // Flash
            case 44: isGrenade = true; speed = 560.0f; type = 1; break; // HE
            case 45: isGrenade = true; speed = 560.0f; type = 2; break; // Smoke
            case 46: isGrenade = true; speed = 560.0f; type = 3; break; // Molotov
            case 47: isGrenade = true; speed = 560.0f; type = 3; break; // Incendiary
            case 48: isGrenade = true; speed = 560.0f; type = 4; break; // Decoy
            default: break;
        }

        if (!isGrenade) return;

        Vec3 origin = Memory::Read<Vec3>(localPawn + Offsets::C_BasePlayerPawn::m_vOldOrigin);
        Vec3 eyeAng = Memory::Read<Vec3>(localPawn + Offsets::C_CSPlayerPawn::m_angEyeAngles);

        float pitch = eyeAng.x * (3.14159265f / 180.0f);
        float yaw = eyeAng.y * (3.14159265f / 180.0f);

        Vec3 dir;
        dir.x = cosf(yaw) * cosf(pitch);
        dir.y = sinf(yaw) * cosf(pitch);
        dir.z = -sinf(pitch);

        Vec3 startPos = origin;
        startPos.z += 60.0f;
        Vec3 velocity = {dir.x * speed, dir.y * speed, dir.z * speed};

        Vec3 prevPos = startPos;
        projectile_t proj = {startPos, velocity, 0.0f, type};

        for (int i = 0; i < MAX_STEPS; i++) {
            proj.vel.z -= GRAVITY * TIME_STEP;
            proj.pos.x += proj.vel.x * TIME_STEP;
            proj.pos.y += proj.vel.y * TIME_STEP;
            proj.pos.z += proj.vel.z * TIME_STEP;
            proj.liveTime += TIME_STEP;

            Vec2 screenPos, screenPrev;
            if (WorldToScreen(proj.pos, screenPos, vm, screenW, screenH) &&
                WorldToScreen(prevPos, screenPrev, vm, screenW, screenH)) {
                float alpha = 1.0f - (float)i / MAX_STEPS;
                dl->AddLine(ImVec2(screenPrev.x, screenPrev.y), ImVec2(screenPos.x, screenPos.y),
                           IM_COL32(255, 200, 50, (int)(alpha * 200)), 1.5f);
                dl->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 2.5f,
                                   IM_COL32(255, 200, 50, (int)(alpha * 255)), 8);
            }
            prevPos = proj.pos;
        }
    }

    inline bool WorldToScreen(const Vec3& world, Vec2& screen, const ViewMatrix& vm, int screenW, int screenH) {
        float w = vm.matrix[3][0] * world.x + vm.matrix[3][1] * world.y + vm.matrix[3][2] * world.z + vm.matrix[3][3];
        if (w < 0.001f) return false;
        float invW = 1.0f / w;
        screen.x = (screenW * 0.5f) + (vm.matrix[0][0] * world.x + vm.matrix[0][1] * world.y + vm.matrix[0][2] * world.z + vm.matrix[0][3]) * invW * (screenW * 0.5f);
        screen.y = (screenH * 0.5f) - (vm.matrix[1][0] * world.x + vm.matrix[1][1] * world.y + vm.matrix[1][2] * world.z + vm.matrix[1][3]) * invW * (screenH * 0.5f);
        return true;
    }
}
