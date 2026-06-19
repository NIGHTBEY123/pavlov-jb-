#pragma once
#include "..\SDK\Entity.h"
#include "..\SDK\Offsets.h"
#include <cmath>
#include <Windows.h>

namespace Aimbot {
    inline bool enabled = true;
    inline int mode = 0;  // 0=legit, 1=high
    inline float fov = 5.0f;
    inline float smooth = 5.0f;
    inline int targetBone = 7; // BONE_HEAD=7 (verified highest Z)
    inline int hotkey = VK_MENU; // ALT
    inline bool rcsEnabled = true;
    inline bool ignoreFlash = true;
    inline float humanizeAmount = 0.0f;

    constexpr float PI = 3.14159265358979f;
    constexpr uintptr_t ENTITY_STRIDE = 0x70;

    inline uintptr_t GetEntity(uintptr_t entityList, int index) {
        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((index & 0x7FFF) >> 9));
        if (!bucket) return 0;
        return Memory::Read<uintptr_t>(bucket + ENTITY_STRIDE * (index & 0x1FF));
    }

    inline uintptr_t ResolvePawn(uintptr_t entityList, uint32_t handle) {
        if (!handle || handle == 0xFFFFFFFF) return 0;
        uint32_t idx = handle & 0x7FFF;
        if (idx == 0 || idx > 0x4000) return 0;
        uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
        if (!bucket) return 0;
        return Memory::Read<uintptr_t>(bucket + ENTITY_STRIDE * (idx & 0x1FF));
    }

    // DragonBurn-style: screen coordinate + mouse_event
    inline void Run(CEntity& localPawn) {
        if (!enabled || !localPawn.IsValid() || !localPawn.IsAlive()) return;
        if (!(GetAsyncKeyState(hotkey) & 0x8000)) return;
        if (!ignoreFlash && localPawn.GetFlashDuration() > 0.0f) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        Vec3 localPos = localPawn.GetOrigin();
        if (localPos.IsZero()) return;

        Vec3 eyePos = localPos;
        int flags = localPawn.GetFlags();
        eyePos.z += (flags & 2) ? 46.0f : 64.062561f;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        if (screenW <= 0 || screenH <= 0) return;

        ViewMatrix vm = Memory::Read<ViewMatrix>(clientBase + Offsets::dwViewMatrix);
        int screenCenterX = screenW / 2;
        int screenCenterY = screenH / 2;

        // Read sensitivity
        float sensitivity = 2.5f;
        __try {
            uintptr_t sensPtr = Memory::Read<uintptr_t>(clientBase + Offsets::dwSensitivity);
            if (sensPtr) {
                float s = Memory::Read<float>(sensPtr + Offsets::dwSensitivity_sensitivity);
                if (s > 0.01f && s < 100.0f) sensitivity = s;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        float bestDist = 99999.0f;
        Vec2 bestScreenPos = {0, 0};
        bool foundTarget = false;

        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        // Find local team
        uint8_t localTeam = 0;
        for (int i = 1; i <= 64; i++) {
            uintptr_t c = GetEntity(entityList, i);
            if (!c || c < 0x10000) continue;
            uint32_t ph = Memory::Read<uint32_t>(c + Offsets::CCSPlayerController::m_hPlayerPawn);
            uintptr_t pw = ResolvePawn(entityList, ph);
            if (pw == localPawn.Address) {
                localTeam = Memory::Read<uint8_t>(c + Offsets::C_BaseEntity::m_iTeamNum);
                break;
            }
        }

        for (int i = 1; i <= 64; i++) {
            uintptr_t controller = GetEntity(entityList, i);
            if (!controller || controller < 0x10000) continue;

            uint8_t aliveFlag = Memory::Read<uint8_t>(controller + Offsets::CCSPlayerController::m_bPawnIsAlive);
            if (aliveFlag != 1) continue;

            uint8_t ctrlTeam = Memory::Read<uint8_t>(controller + Offsets::C_BaseEntity::m_iTeamNum);
            if (localTeam != 0 && ctrlTeam == localTeam) continue;

            uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
            uintptr_t pawn = ResolvePawn(entityList, pawnHandle);
            if (!pawn || pawn == localPawn.Address) continue;

            CEntity enemy(pawn);
            if (!enemy.IsAlive()) continue;

            Vec3 bonePos = enemy.GetBonePos(targetBone);
            if (bonePos.IsZero()) continue;

            // Head bone is at neck joint, offset up for actual head center
            if (targetBone == 7) bonePos.z += 3.5f;

            // WorldToScreen
            float w = vm.matrix[3][0] * bonePos.x + vm.matrix[3][1] * bonePos.y + vm.matrix[3][2] * bonePos.z + vm.matrix[3][3];
            if (w < 0.001f) continue;
            float invW = 1.0f / w;
            float sx = (screenW * 0.5f) + (vm.matrix[0][0] * bonePos.x + vm.matrix[0][1] * bonePos.y + vm.matrix[0][2] * bonePos.z + vm.matrix[0][3]) * invW * (screenW * 0.5f);
            float sy = (screenH * 0.5f) - (vm.matrix[1][0] * bonePos.x + vm.matrix[1][1] * bonePos.y + vm.matrix[1][2] * bonePos.z + vm.matrix[1][3]) * invW * (screenH * 0.5f);

            float dx = sx - screenCenterX;
            float dy = sy - screenCenterY;
            float screenDist = sqrtf(dx * dx + dy * dy);

            float fovPixels = (fov / 90.0f) * screenCenterX;
            if (screenDist > fovPixels) continue;

            if (screenDist < bestDist) {
                bestDist = screenDist;
                bestScreenPos = {sx, sy};
                foundTarget = true;
            }
        }

        if (foundTarget) {
            float targetX = bestScreenPos.x - screenCenterX;
            float targetY = bestScreenPos.y - screenCenterY;

            // RCS compensation
            if (rcsEnabled) {
                int shotsFired = localPawn.GetShotsFired();
                if (shotsFired > 1) {
                    Vec3 punch = localPawn.GetAimPunchAngle();
                    if (punch.x == punch.x && punch.y == punch.y) {
                        // Punch angle to screen offset (CS2 uses 2x multiplier)
                        float punchScale = 2.0f;
                        targetY += (punch.x * punchScale / 90.0f) * screenCenterY;
                        targetX += -(punch.y * punchScale / 90.0f) * screenCenterX;
                    }
                }
            }

            // Simple smooth mouse movement (no sensitivity math - mouse_event is raw)
            if (smooth > 0.0f) {
                targetX /= smooth;
                targetY /= smooth;
            }

            // Clamp
            if (fabsf(targetX) > 150.0f) targetX = (targetX > 0) ? 150.0f : -150.0f;
            if (fabsf(targetY) > 150.0f) targetY = (targetY > 0) ? 150.0f : -150.0f;
            if (fabsf(targetX) < 0.3f && fabsf(targetY) < 0.3f) return;

            mouse_event(MOUSEEVENTF_MOVE, (DWORD)(long)targetX, (DWORD)(long)targetY, 0, 0);
        }
    }
}
