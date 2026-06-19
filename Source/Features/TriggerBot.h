#pragma once
#include "..\SDK\Entity.h"
#include "..\SDK\Offsets.h"
#include <random>
#include <chrono>

namespace TriggerBot {
    inline bool enabled = true;
    inline int mode = 0;  // 0=legit, 1=high
    inline int hotkey = 0x58; // X key
    inline int delayMin = 55;
    inline int delayMax = 145;

    // Non-blocking delay using timestamps instead of Sleep()
    inline bool waitingToShoot = false;
    inline std::chrono::steady_clock::time_point shootTime;
    inline bool attackActive = false;
    inline std::chrono::steady_clock::time_point attackEnd;

    inline void Run(CEntity& localPawn) {
        if (!enabled || !localPawn.IsValid() || !localPawn.IsAlive()) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        auto now = std::chrono::steady_clock::now();

        // Handle active attack release (non-blocking)
        if (attackActive) {
            if (now >= attackEnd) {
                Memory::Write<int>(clientBase + Offsets::dwAttack, 256);
                attackActive = false;
            }
            return;
        }

        if (!(GetAsyncKeyState(hotkey) & 0x8000)) {
            waitingToShoot = false;
            return;
        }

        int entityIndex = Memory::Read<int>(localPawn.Address + Offsets::C_CSPlayerPawn::m_iIDEntIndex);
        if (entityIndex <= 0) { waitingToShoot = false; return; }

        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        // FIXED: Use consistent entity list traversal pattern
        uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((entityIndex & 0x7FFF) >> 9));
        if (!listEntry) return;

        uintptr_t pawn = Memory::Read<uintptr_t>(listEntry + 0x70 * (entityIndex & 0x1FF));
        if (!pawn) return;

        CEntity target(pawn);
        if (!target.IsAlive() || target.GetTeam() == localPawn.GetTeam()) return;

        // Start delay timer (non-blocking)
        if (!waitingToShoot) {
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(delayMin, delayMax);
            shootTime = now + std::chrono::milliseconds(dist(rng));
            waitingToShoot = true;
            return;
        }

        // Check if delay passed
        if (now >= shootTime) {
            Memory::Write<int>(clientBase + Offsets::dwAttack, 65537);
            attackActive = true;
            attackEnd = now + std::chrono::milliseconds(15);
            waitingToShoot = false;
        }
    }
}
