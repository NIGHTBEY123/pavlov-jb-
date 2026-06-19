#pragma once
#include <windows.h>
#include <cstdint>
#include "..\Core\Memory.h"
#include "..\include\MinHook.h"

namespace KnifeHook {

    using FnFrameSnapshotNotify = void(__fastcall*)(void*, void*);
    inline FnFrameSnapshotNotify originalFSN = nullptr;
    inline bool hooked = false;

    inline void __fastcall HookedFSN(void* thisptr, void* unk) {
        if (originalFSN)
            originalFSN(thisptr, unk);
    }

    inline void Initialize() {
        HMODULE engine2 = GetModuleHandleA("engine2.dll");
        if (!engine2) return;

        uintptr_t fsnAddr = Memory::PatternScan(engine2, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B D9 E8 ?? ?? ?? ?? 48 8B F8 48 85 C0 74 0D");
        if (!fsnAddr) return;

        if (MH_CreateHook((void*)fsnAddr, HookedFSN, (void**)&originalFSN) == MH_OK) {
            if (MH_EnableHook((void*)fsnAddr) == MH_OK) {
                hooked = true;
            }
        }
    }

    inline void Shutdown() {
        if (hooked) {
            MH_DisableHook(MH_ALL_HOOKS);
            hooked = false;
        }
    }
}
