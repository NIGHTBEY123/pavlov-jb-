#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <Psapi.h>
#include "../Core/Memory.h"
#include "../SDK/Offsets.h"
#include "../include/MinHook.h"

// ========================================================================
//  REAL CHAMS — scenesystem.dll DrawObject Color Override
//  Uses simple detour hooking (no MinHook dependency)
//  Based on UnknownCheats CS2 internal chams research 2025
// ========================================================================
namespace RealChams {

    inline void HookLog(const char* msg) {
        HANDLE hFile = CreateFileA("C:\\\\\\dll_log.txt", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
            CloseHandle(hFile);
        }
    }
    inline void HookLogFmt(const char* fmt, ...) {
        char buf[512]; va_list a; va_start(a, fmt); vsnprintf(buf, 512, fmt, a); va_end(a); HookLog(buf);
    }

    // ---- Settings ----
    inline bool enabled = true;
    inline float enemyVisColor[4]    = {0.0f, 1.0f, 0.8f, 1.0f}; // Cyan (visible)
    inline float enemyHiddenColor[4] = {1.0f, 0.2f, 0.2f, 1.0f}; // Red (hidden)
    inline bool  hooked = false;

    // ---- Structures ----
    struct Color_t {
        uint8_t r, g, b, a;
    };

    // CMeshData — from CS2 Source 2 reversing (2025)
    struct CMeshData {
        void* m_pMesh;                    // 0x00
        void* ptr1;                       // 0x08
        uint8_t pad0[0x8];               // 0x10
        void* m_pSceneAnimatableObject;   // 0x18
        void* m_pMaterial;                // 0x20
        void* m_pMaterial2;               // 0x28
        uint8_t pad1[0x10];              // 0x30
        void* m_pObjectInfo;              // 0x40
        uint8_t pad2[0x8];               // 0x48
        Color_t m_pColor;                 // 0x50
        uint8_t pad3[0x14];              // 0x54
    };

    // ---- Pattern scanner ----
    inline uintptr_t ScanPattern(uintptr_t base, size_t size, const char* sig) {
        uint8_t bytes[256];
        bool wild[256];
        int len = 0;

        const char* p = sig;
        while (*p) {
            while (*p == ' ') p++;
            if (!*p) break;
            if (*p == '?') {
                bytes[len] = 0;
                wild[len] = true;
                p++;
                if (*p == '?') p++;
            } else {
                bytes[len] = (uint8_t)strtoul(p, nullptr, 16);
                wild[len] = false;
                p += 2;
            }
            len++;
        }

        for (size_t i = 0; i <= size - len; i++) {
            bool ok = true;
            for (int j = 0; j < len; j++) {
                if (!wild[j] && bytes[j] != *(uint8_t*)(base + i + j)) {
                    ok = false;
                    break;
                }
            }
            if (ok) return base + i;
        }
        return 0;
    }

    // ---- MinHook Detour Hook ----
    inline void* oDrawObject = nullptr;
    inline uintptr_t hookTarget = 0;

    // ---- DrawObject function type ----
    using DrawObjectFn = void(__fastcall*)(void* thisptr, void* pAnimObj, void* pCtx,
                                           CMeshData* meshData, int meshCount, void* unk);

    inline bool PlaceHook(uintptr_t target, void* detour) {
        // Assume MH_Initialize has already been called by DX11Hook
        if (MH_CreateHook((void*)target, detour, &oDrawObject) != MH_OK) {
            HookLog("[CHAMS] MH_CreateHook failed!\r\n");
            return false;
        }
        if (MH_EnableHook((void*)target) != MH_OK) {
            HookLog("[CHAMS] MH_EnableHook failed!\r\n");
            return false;
        }
        hookTarget = target;
        return true;
    }

    inline void RemoveHook() {
        if (hookTarget) {
            MH_DisableHook((void*)hookTarget);
            hookTarget = 0;
        }
    }

    inline DrawObjectFn GetOriginal() {
        return reinterpret_cast<DrawObjectFn>(oDrawObject);
    }

    // ---- Check if scene object belongs to a player entity and return pawn addr ----
    inline uintptr_t GetPlayerPawnFromSceneObj(void* pSceneAnimObj) {
        if (!pSceneAnimObj || (uintptr_t)pSceneAnimObj < 0x10000) return 0;
        __try {
            // Try known hOwner offsets in CSceneAnimatableObject
            static const int ownerOffsets[] = {0x50, 0x48, 0x58, 0x40, 0x38};
            
            uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
            if (!clientBase) return 0;

            uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
            if (!entityList) return 0;

            for (int off : ownerOffsets) {
                if (IsBadReadPtr((void*)((uintptr_t)pSceneAnimObj + off), 4)) continue;
                
                uint32_t hOwner = *(uint32_t*)((uintptr_t)pSceneAnimObj + off);
                if (hOwner == 0 || hOwner == 0xFFFFFFFF || hOwner > 0x100000) continue;

                int idx = hOwner & 0x7FFF;
                if (idx <= 0 || idx > 64) continue; // Players are typically idx 1-64

                uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x8 * ((idx >> 9) + 1));
                if (!listEntry) continue;

                uintptr_t controller = Memory::Read<uintptr_t>(listEntry + 0x78 * (idx & 0x1FF));
                if (!controller || controller < 0x10000) continue;

                // Read entity identity
                uintptr_t identity = Memory::Read<uintptr_t>(controller + 0x10);
                if (!identity || identity < 0x10000) continue;

                uintptr_t namePtr = Memory::Read<uintptr_t>(identity + 0x20);
                if (!namePtr || namePtr < 0x10000) continue;

                char cn[20] = {};
                if (IsBadReadPtr((void*)namePtr, 16)) continue;
                memcpy(cn, (void*)namePtr, 16);
                cn[16] = 0;

                if (strstr(cn, "player") || strstr(cn, "cs_player")) {
                    // Controller found. Resolve pawn from it.
                    uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
                    if (!pawnHandle || pawnHandle == 0xFFFFFFFF) return 0;
                    uint32_t pIdx = pawnHandle & 0x7FFF;
                    uintptr_t pBucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (pIdx >> 9));
                    if (!pBucket) return 0;
                    uintptr_t pawn = Memory::Read<uintptr_t>(pBucket + 0x70 * (pIdx & 0x1FF));
                    return pawn;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return 0;
    }

    // ---- Hook callback ----
    inline void __fastcall hkDrawObject(void* thisptr, void* pAnimObj, void* pCtx,
                                         CMeshData* meshData, int meshCount, void* unk) {
        if (enabled && meshData && meshCount > 0) {
            __try {
                uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
                uintptr_t localPawnAddr = clientBase ? GetLocalPlayerPawn(clientBase) : 0;
                uint8_t localTeam = localPawnAddr ? Memory::Read<uint8_t>(localPawnAddr + Offsets::C_BaseEntity::m_iTeamNum) : 0;

                for (int i = 0; i < meshCount && i < 32; i++) {
                    CMeshData& mesh = meshData[i];
                    if (mesh.m_pSceneAnimatableObject) {
                        uintptr_t pawn = GetPlayerPawnFromSceneObj(mesh.m_pSceneAnimatableObject);
                        if (pawn && pawn != localPawnAddr) {
                            uint8_t team = Memory::Read<uint8_t>(pawn + Offsets::C_BaseEntity::m_iTeamNum);
                            if (team != localTeam) {
                                // Enemy
                                mesh.m_pColor.r = (uint8_t)(enemyVisColor[0] * 255);
                                mesh.m_pColor.g = (uint8_t)(enemyVisColor[1] * 255);
                                mesh.m_pColor.b = (uint8_t)(enemyVisColor[2] * 255);
                                mesh.m_pColor.a = 255;
                            } else {
                                // Teammate - restore original or white
                                mesh.m_pColor.r = 255;
                                mesh.m_pColor.g = 255;
                                mesh.m_pColor.b = 255;
                                mesh.m_pColor.a = 255;
                            }
                        } else if (pawn == localPawnAddr) {
                            // Local player - restore original or white
                            mesh.m_pColor.r = 255;
                            mesh.m_pColor.g = 255;
                            mesh.m_pColor.b = 255;
                            mesh.m_pColor.a = 255;
                        }
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
        // Call original via trampoline
        GetOriginal()(thisptr, pAnimObj, pCtx, meshData, meshCount, unk);
    }

    // ---- DrawObject pattern signatures for scenesystem.dll ----
    inline bool Init() {
        HookLog("[CHAMS] Starting DrawObject hook...\r\n");

        HMODULE hScene = GetModuleHandleA("scenesystem.dll");
        if (!hScene) {
            HookLog("[CHAMS] scenesystem.dll not loaded!\r\n");
            return false;
        }

        MODULEINFO modInfo = {};
        GetModuleInformation(GetCurrentProcess(), hScene, &modInfo, sizeof(modInfo));
        uintptr_t base = (uintptr_t)hScene;
        size_t size = modInfo.SizeOfImage;
        HookLogFmt("[CHAMS] scenesystem base=0x%llX size=%d\r\n", base, (int)size);

        // Try multiple known patterns for DrawObject
        const char* sigs[] = {
            // Pattern 1: Common DrawObject prologue
            "48 8B C4 48 89 50 10 48 89 48 08 55 41 56",
            // Pattern 2: Alternative prologue  
            "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56",
            // Pattern 3: Another variant
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC",
            // Pattern 4: Shorter match
            "48 8B C4 48 89 50 10 55 56 57 41 54 41 55",
            // Pattern 5: Generic function start
            "40 53 56 57 48 83 EC 30 48 8B DA 48 8B F1 E8",
        };

        uintptr_t addr = 0;
        for (int i = 0; i < _countof(sigs); i++) {
            addr = ScanPattern(base, size, sigs[i]);
            if (addr) {
                HookLogFmt("[CHAMS] Pattern %d hit @ 0x%llX (RVA: 0x%X)\r\n", i, addr, (int)(addr - base));
                break;
            }
        }

        if (!addr) {
            HookLog("[CHAMS] No DrawObject pattern found! Trying export scan...\r\n");
            
            // Alternative: scan for DrawObject by looking for specific instruction sequences
            // within scenesystem that reference CMeshData-like structs
            // Look for functions that read at offset 0x18 and 0x50 (pSceneAnimObj and Color)
            const char* altSigs[] = {
                "4C 8B 43 18 48 8B CB",  // mov r8, [rbx+18h] = pSceneAnimObj
                "8B 43 50 89",           // mov eax, [rbx+50h] = m_pColor
            };
            for (int i = 0; i < 2; i++) {
                uintptr_t ref = ScanPattern(base, size, altSigs[i]);
                if (ref) {
                    // Walk backwards to find function start (look for common x64 prologues)
                    for (int back = 0; back < 256; back++) {
                        uint8_t* p = (uint8_t*)(ref - back);
                        // Check for "48 8B C4" or "48 89 5C" or "40 53/55/56/57"
                        if ((p[0] == 0x48 && p[1] == 0x8B && p[2] == 0xC4) ||
                            (p[0] == 0x48 && p[1] == 0x89 && p[2] == 0x5C) ||
                            (p[0] == 0x40 && (p[1] >= 0x53 && p[1] <= 0x57))) {
                            addr = (uintptr_t)p;
                            HookLogFmt("[CHAMS] Alt pattern found @ 0x%llX (back %d from ref)\r\n", addr, back);
                            break;
                        }
                    }
                    if (addr) break;
                }
            }
        }

        if (!addr) {
            HookLog("[CHAMS] FAILED: Cannot find DrawObject function.\r\n");
            HookLog("[CHAMS] Falling back to glow-only chams.\r\n");
            return false;
        }

        // Validate function start
        if (IsBadReadPtr((void*)addr, 32)) {
            HookLog("[CHAMS] Address not readable!\r\n");
            return false;
        }

        // Place the detour hook
        if (!PlaceHook(addr, (void*)&hkDrawObject)) {
            HookLog("[CHAMS] PlaceHook failed!\r\n");
            return false;
        }

        hooked = true;
        HookLog("[CHAMS] DrawObject hook ACTIVE! Real chams enabled.\r\n");
        return true;
    }

    inline void Shutdown() {
        if (hooked) {
            RemoveHook();
            hooked = false;
            HookLog("[CHAMS] Hook removed.\r\n");
        }
    }
}
