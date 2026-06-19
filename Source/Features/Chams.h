#pragma once
#include <windows.h>
#include <cstdint>
#include "..\Core\Memory.h"
#include "..\SDK\Offsets.h"

namespace Chams {
    inline bool enabled = false;
    inline float color[4] = {0.0f, 0.5f, 1.0f, 1.0f};

    inline void Apply(uintptr_t pawn, bool isVisible) {
        if (!enabled || !pawn) return;
        uintptr_t glow = Memory::Read<uintptr_t>(pawn + Offsets::CGlowProperty::m_Glow);
        if (!glow) return;
        Memory::Write<float>(glow + 0x8, color[0]);
        Memory::Write<float>(glow + 0xC, color[1]);
        Memory::Write<float>(glow + 0x10, color[2]);
        Memory::Write<float>(glow + 0x14, color[3]);
        Memory::Write<bool>(glow + 0x24, true);
    }
}
