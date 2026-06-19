#pragma once
#include <windows.h>
#include <cstdint>
#include "..\Core\Memory.h"
#include "..\SDK\Offsets.h"

namespace InventoryChanger {

    inline bool musicKitEnabled = false;
    inline int selectedMusicKit = 0;

    struct MusicKitEntry { const char* name; int id; };
    inline MusicKitEntry musicKits[] = {
        {"None", 0}, {"Desert Fire", 1}, {"Momentum", 2}, {"Total Domination", 3},
        {"Neighborhood", 4}, {"Rising Sun", 5}, {"Fight Song", 6}, {"Rock", 7},
        {"The Talos", 8}, {"Triumph", 9}, {"Wrath", 10}, {"For No Mankind", 11},
        {"Hotline Miami", 12}, {"Death's Head", 13}, {"Isorhythm", 14},
        {"Liftoff", 15}, {"From Dust", 16}, {"Lunar", 17}, {"Azure", 18},
    };
    inline constexpr int MUSIC_KIT_COUNT = sizeof(musicKits) / sizeof(MusicKitEntry);

    inline void ApplyMusicKit(uintptr_t localController) {
        if (!musicKitEnabled || !localController) return;
        if (selectedMusicKit <= 0 || selectedMusicKit >= MUSIC_KIT_COUNT) return;

        uintptr_t invSvc = Memory::Read<uintptr_t>(localController + Offsets::CCSPlayerController::m_pInventoryServices);
        if (!invSvc) return;

        Memory::Write<int>(invSvc + Offsets::InventoryServices::m_unMusicID, musicKits[selectedMusicKit].id);
    }
}
