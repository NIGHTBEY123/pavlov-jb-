#include <windows.h>
#include <cstdio>
#include "Features\Features.h"
#include "Core\DX11Hook.h"
#include "GUI\PremiumGUI.h"
#include "Features\ESP.h"
#include "Features\DrawObjectHook.h"
#include "Features\KnifeHook.h"
#include "Features\FileSystemHook.h"
#include "Core\DrawIndexedHook.h"
#include "Core\AutoUpdater.h"
#include <thread>
#include "Features\Aimbot.h"
#include "Features\TriggerBot.h"
#include "Features\Misc.h"
#include "Features\InventoryChanger.h"
#include "Features\SceneChams.h"
#include "Features\HitEffects.h"
#include "Features\GrenadePrediction.h"

void Log(const char* msg) {
    HANDLE hFile = CreateFileA("C:\\\\\\dll_log.txt", FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        CloseHandle(hFile);
    }
}

void DX11Hook::RenderCallback() {
    static bool rShiftHeld = false;
    if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) {
        if (!rShiftHeld) { DX11Hook::menuVisible = !DX11Hook::menuVisible; rShiftHeld = true; }
    } else { rShiftHeld = false; }

    PremiumGUI::menuOpen = DX11Hook::menuVisible;
    // Log removed - per-frame logging causes I/O contention
    __try {
        ESP::Render();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}


    __try {
        HitEffects::Render();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}


    // Grenade trajectory prediction
    __try {
        uintptr_t cb = Memory::GetModuleBase(L"client.dll");
        if (cb) {
            uintptr_t lp = GetLocalPlayerPawn(cb);
            ViewMatrix vvm = Memory::Read<ViewMatrix>(cb + Offsets::dwViewMatrix);
            ImVec2 ds = ImGui::GetIO().DisplaySize;
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            if (dl && lp) GrenadePrediction::Render(dl, lp, cb, vvm, (int)ds.x, (int)ds.y);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}


    __try {
        PremiumGUI::Render();
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
    

    DX11Hook::menuVisible = PremiumGUI::menuOpen;
}

DWORD WINAPI FeatureThread(LPVOID lpParam) {
    Log("[+] FeatureThread basladi...\r\n");
    Sleep(3000); 

    // ---- DYNAMIC PATTERN SCANNER ----
    uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
    if (clientBase) {
        Log("[+] Fetching absolute latest offsets from a2x directly from GitHub...\r\n");
        if (AutoUpdater::UpdateOffsets((HMODULE)lpParam)) {
            char logBuf[512];
            sprintf_s(logBuf, "[+] Offsets Downloaded -> EntList: %llX, LocCtrl: %llX, ViewMat: %llX, GlobVar: %llX\r\n", 
                Offsets::dwEntityList, Offsets::dwLocalPlayerController, Offsets::dwViewMatrix, Offsets::dwGlobalVars);
            Log(logBuf);
        } else {
            Log("[-] Failed to download offsets from a2x! Cheat may crash if outdated.\r\n");
        }
    }

    if (DX11Hook::Hook()) {
        Log("[+] DX11 Hook Basarili!\r\n");
    }

    // Re-enabled after stride fix (0x78 -> 0x70)
    KnifeHook::Initialize();
    RealChams::Init();
    // FileSystemHook::Initialize();

    static int loopCount = 0;
    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {

        __try {
            uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
            if (clientBase) {
                uintptr_t localPawnAddr = GetLocalPlayerPawn(clientBase);
                CEntity localPawn(localPawnAddr);
                
                if (localPawn.IsValid()) {
                    // Bunny hop
                    if (Misc::bhopEnabled) {
                        Misc::BunnyHop(localPawn);
                    }
                    
                    // Trigger bot
                    if (TriggerBot::enabled) {
                        TriggerBot::Run(localPawn);
                    }
                    
                    // Aimbot
                    if (Aimbot::enabled) {
                        Aimbot::Run(localPawn);
                    }
                    
                    // No recoil
                    if (Misc::NoRecoil::enabled) {
                        Misc::NoRecoil::Run(localPawn);
                    }
                    
                    // FOV changer
                    if (Misc::fovEnabled) {
                        Misc::ApplyFOV(localPawn);
                    }
                    
                    // Night mode — call always so disable logic can reset fog
                    Misc::ApplyNightMode(localPawn);
                    
                    // Third person — call always so it can reset observer mode
                    Misc::ApplyThirdPerson(localPawn);
                    
                    // Noclip
                    if (Misc::noclipEnabled) {
                        Misc::NoclipFly(localPawn);
                    }
                    
                    // Skin changer — apply every frame for persistence
                    if (SkinChanger::enabled) {
                        SkinChanger::ApplySkin(localPawnAddr);
                        SkinChanger::needsUpdate = false;
                    }
                    
                    // Anti flash
                    if (Misc::antiFlashEnabled) {
                        Misc::AntiFlash();
                    }
                }
                
                // Radar hack
                if (Misc::radarEnabled) {
                    Misc::RadarHack();
                }
                
                // Anti smoke
                if (Misc::antiSmokeEnabled) {
                    Misc::AntiSmoke();
                }
                
                // Sky color and Night Mode
                Misc::ApplySkyColor(clientBase);
                Misc::ApplyNightMode(localPawn);
                
                // SceneChams / Glow — apply to all players
                if (SceneChams::enabled || ESP::showGlow) {
                    uintptr_t entList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
                    if (entList) {
                        int localTeam = 0;
                        if (localPawn.IsValid()) {
                            localTeam = Memory::Read<int>(localPawnAddr + Offsets::C_BaseEntity::m_iTeamNum);
                        }
                        for (int i = 1; i <= 64; i++) {
                            __try {
                                uintptr_t ctrl = ESP::GetEntityByIndex(entList, i);
                                if (!ctrl) continue;
                                uint32_t pHandle = Memory::Read<uint32_t>(ctrl + Offsets::CCSPlayerController::m_hPlayerPawn);
                                uintptr_t pawn = ESP::ResolvePawnFromHandle(entList, pHandle);
                                if (!pawn || pawn == localPawnAddr) continue;
                                
                                int hp = Memory::Read<int>(pawn + Offsets::C_BaseEntity::m_iHealth);
                                if (hp <= 0 && ESP::glowDeadFilter) continue;
                                if (hp <= 0) continue; // Always skip dead players for glow
                                
                                int team = Memory::Read<int>(pawn + Offsets::C_BaseEntity::m_iTeamNum);
                                bool isEnemy = (team != localTeam);
                                
                                // Apply glow — isEnemy selects visibleColor, !isEnemy selects hiddenColor
                                SceneChams::Apply(pawn, isEnemy, isEnemy);
                            } __except(EXCEPTION_EXECUTE_HANDLER) {}
                        }
                    }
                }
                
                // Spectator list + bomb info
                Misc::UpdateSpectatorList();
                Misc::UpdateBombInfo();
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}

        if (loopCount % 500 == 0) {
            char buf[64];
            sprintf_s(buf, "[FT] Loop %d alive\r\n", loopCount);
            Log(buf);
        }
        loopCount++;
        Sleep(3);
    }
    
    RealChams::Shutdown();
    DX11Hook::Unhook();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        Log("[!!!] DllMain ATTACH - DLL loaded!\r\n");
        DisableThreadLibraryCalls(hinstDLL);
        
        // PIN the DLL in memory so nothing can unload it
        HMODULE hPin = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                          (LPCSTR)DllMain, &hPin);
        
        HANDLE hThread = CreateThread(nullptr, 0, FeatureThread, hinstDLL, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    if (fdwReason == DLL_PROCESS_DETACH) {
        Log("[!] DLL_PROCESS_DETACH called! DLL being unloaded!\r\n");
    }
    return TRUE;
}
 
