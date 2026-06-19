#pragma once
#include <windows.h>
#include <cstdint>
#include "..\Core\Memory.h"
#include "..\SDK\Offsets.h"

namespace SceneChams {

    inline bool enabled = false;
    inline int mode = 0;
    inline float glowIntensity = 3.0f;
    inline bool throughWall = true;
    inline bool showTeammates = false;
    inline float visibleColor[4] = {0.0f, 1.0f, 0.8f, 1.0f};
    inline float hiddenColor[4] = {1.0f, 0.2f, 0.2f, 1.0f};

    inline void Apply(uintptr_t pawn, bool isVisible, bool isEnemy) {
        if (!enabled || !pawn) return;

        float* col = isVisible ? visibleColor : hiddenColor;

        uintptr_t glow = Memory::Read<uintptr_t>(pawn + Offsets::CGlowProperty::m_Glow);
        if (!glow) return;

        Memory::Write<float>(glow + 0x8, col[0]);
        Memory::Write<float>(glow + 0xC, col[1]);
        Memory::Write<float>(glow + 0x10, col[2]);
        Memory::Write<float>(glow + 0x14, col[3]);

        int glowStyle = isVisible ? 3 : 1;
        Memory::Write<int>(glow + 0x28, glowStyle);
        Memory::Write<bool>(glow + 0x24, true);
        Memory::Write<bool>(glow + 0x2C, false);
        Memory::Write<bool>(glow + 0x30, isEnemy ? 0 : 1);
    }
}
