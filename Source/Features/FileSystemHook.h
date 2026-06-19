#pragma once
#include <windows.h>
#include <cstdint>
#include "..\Core\Memory.h"
#include "..\include\MinHook.h"

namespace FileSystemHook {

    using FnFullRead = int(__fastcall*)(void* thisptr, void* path, void* buf, int size, int* readBytes, bool b, void* unk);
    inline FnFullRead originalFullRead = nullptr;
    inline bool hooked = false;

    inline int __fastcall HookedFullRead(void* thisptr, void* path, void* buf, int size, int* readBytes, bool b, void* unk) {
        return originalFullRead(thisptr, path, buf, size, readBytes, b, unk);
    }

    inline void Initialize() {
        HMODULE engine2 = GetModuleHandleA("engine2.dll");
        if (!engine2) return;

        uintptr_t addr = Memory::PatternScan(engine2, "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B FA 48 8B D9 48 85 D2 75 0C");
        if (!addr) return;

        if (MH_CreateHook((void*)addr, HookedFullRead, (void**)&originalFullRead) == MH_OK) {
            if (MH_EnableHook((void*)addr) == MH_OK) {
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
