#pragma once
#include "..\SDK\Entity.h"
#include "..\SDK\Offsets.h"
#include <random>
#include <cmath>

namespace Misc {
    inline bool bhopEnabled = true;
    inline bool radarEnabled = true;
    
    // Noclip / Fly
    inline bool noclipEnabled = false;
    inline int noclipKey = 0x4E; // N
    inline bool noclipKeyWasDown = false;
    inline bool noclipActive = false;
    inline float flySpeed = 500.0f;

    // Spectator list
    inline bool spectatorListEnabled = true;

    // Bomb ESP
    inline bool bombESPEnabled = true;

    // Anti-Smoke ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Remove smoke rendering
    // CSmokeGrenadeProjectile::m_nSmokeEffectTickBegin = 0x1250
    // CSmokeGrenadeProjectile::m_bDidSmokeEffect = 0x1254
    inline bool antiSmokeEnabled = false;

    // Smoke timer data (populated in FeatureThread for ESP render)
    struct SmokeInfo {
        float x, y, z;
        float timeLeft;
        bool active;
    };
    inline SmokeInfo smokes[8] = {};
    inline int smokeCount = 0;

    inline void AntiSmoke() {
        if (!antiSmokeEnabled) { smokeCount = 0; return; }
        __try {
            uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
            if (!clientBase) return;
            if (Offsets::dwPlantedC4 == 0) return; // Prevent raw offset evaluation to clientBase

            uintptr_t plantedC4 = Memory::Read<uintptr_t>(clientBase + Offsets::dwPlantedC4);
            uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
            if (!entityList) return;
            uintptr_t globalVars = Memory::Read<uintptr_t>(clientBase + Offsets::dwGlobalVars);
            float curTime = globalVars ? Memory::Read<float>(globalVars + Offsets::GlobalVar::CurrentTime) : 0;
            float tickInterval = globalVars ? Memory::Read<float>(globalVars + 0x10) : 0.015625f;

            int sCount = 0;
            // Scan for smoke entities (entity indices 64-512)
            for (int i = 64; i < 512 && sCount < 8; i++) {
                uintptr_t entry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((i & 0x7FFF) >> 9));
                if (!entry) continue;
                uintptr_t ent = Memory::Read<uintptr_t>(entry + 0x70 * (i & 0x1FF));
                if (!ent || ent < 0x10000) continue;

                // Check if smoke is active
                int smokeTick = Memory::Read<int>(ent + Offsets::C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin); // m_nSmokeEffectTickBegin
                if (smokeTick > 0) {
                    // This is a smoke! Zero out to remove visual
                    Memory::Write<int>(ent + Offsets::C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin, 0);
                    Memory::Write<bool>(ent + Offsets::C_SmokeGrenadeProjectile::m_bDidSmokeEffect, false); // m_bDidSmokeEffect

                    // Calculate remaining time (smoke = 18s)
                    float smokeStart = smokeTick * tickInterval;
                    float elapsed = curTime - smokeStart;
                    float remaining = 18.0f - elapsed;
                    if (remaining > 0 && remaining < 20.0f) {
                        Vec3 pos = Memory::Read<Vec3>(ent + Offsets::C_BasePlayerPawn::m_vOldOrigin); // m_vOldOrigin
                        smokes[sCount].x = pos.x;
                        smokes[sCount].y = pos.y;
                        smokes[sCount].z = pos.z;
                        smokes[sCount].timeLeft = remaining;
                        smokes[sCount].active = true;
                        sCount++;
                    }
                }
            }
            smokeCount = sCount;
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline bool antiFlashEnabled = false;

    inline void AntiFlash() {
        if (!antiFlashEnabled) return;
        __try {
            uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
            if (!clientBase) return;
            uintptr_t localPawn = GetLocalPlayerPawn(clientBase);
            if (!localPawn) return;
            
            // Set max flash alpha to 0 and duration to 0
            Memory::Write<float>(localPawn + Offsets::C_CSPlayerPawnBase::m_flFlashMaxAlpha, 0.0f);
            Memory::Write<float>(localPawn + Offsets::C_CSPlayerPawnBase::m_flFlashDuration, 0.0f);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ========================================================================
    //  BUNNYHOP ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Improved with perfect timing + rapid cycle
    // ========================================================================
    inline bool jumpHeld = false;
    inline int bhopTick = 0;

    inline void BunnyHop(CEntity& localPawn) {
        if (!bhopEnabled || !localPawn.IsValid() || !localPawn.IsAlive()) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        bool spaceHeld = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        
        if (!spaceHeld) {
            jumpHeld = false;
            bhopTick = 0;
            return;
        }

        int flags = localPawn.GetFlags();
        bool onGround = (flags & 1) != 0;

        if (onGround) {
            // Perfect bhop: +jump immediately on ground contact
            Memory::Write<int>(clientBase + Offsets::dwJump, 65537);
            jumpHeld = true;
            bhopTick = 0;
        } else if (jumpHeld) {
            bhopTick++;
            if (bhopTick >= 1) {
                // Release jump in air so we can jump again on landing
                Memory::Write<int>(clientBase + Offsets::dwJump, 256);
                jumpHeld = false;
                bhopTick = 0;
            }
        }
    }

    // ========================================================================
    //  RADAR HACK ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Set m_bSpotted = true for all enemy pawns
    // ========================================================================
    inline void RadarHack() {
        if (!radarEnabled) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        // Get local team for filtering
        uintptr_t localPawn = GetLocalPlayerPawn(clientBase);
        int localTeam = 0;
        if (localPawn) {
            localTeam = Memory::Read<uint8_t>(localPawn + Offsets::C_BaseEntity::m_iTeamNum);
        }

        for (int i = 1; i <= 64; i++) {
            uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((i & 0x7FFF) >> 9));
            if (!listEntry) continue;
            uintptr_t controller = Memory::Read<uintptr_t>(listEntry + 0x70 * (i & 0x1FF));
            if (!controller || controller < 0x10000) continue;

            uint8_t aliveFlag = Memory::Read<uint8_t>(controller + Offsets::CCSPlayerController::m_bPawnIsAlive);
            if (aliveFlag != 1) continue;

            uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
            if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;
            uint32_t idx = pawnHandle & 0x7FFF;
            if (idx == 0 || idx > 0x4000) continue;

            uintptr_t pawnEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
            if (!pawnEntry || pawnEntry < 0x10000) continue;
            uintptr_t pawn = Memory::Read<uintptr_t>(pawnEntry + 0x70 * (idx & 0x1FF));
            if (!pawn || pawn < 0x10000 || pawn > 0x7FFFFFFFFFFF) continue;

            // Skip teammates
            int team = Memory::Read<uint8_t>(pawn + Offsets::C_BaseEntity::m_iTeamNum);
            if (team == localTeam) continue;

            __try {
                uintptr_t spottedBase = pawn + Offsets::C_CSPlayerPawn::m_entitySpottedState;
                // Try offset 0x8 (from dump) and 0x0 (fallback)
                Memory::Write<bool>(spottedBase + 0x8, true);
                Memory::Write<bool>(spottedBase + 0x0, true);
                // Set SpottedByMask to all (0xFFFFFFFF) so every team sees them
                Memory::Write<uint32_t>(spottedBase + Offsets::EntitySpottedState::m_bSpottedByMask, 0xFFFFFFFF);
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    // ========================================================================
    //  NOCLIP / FLY ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â m_MoveType based (proper noclip, not just velocity)
    // ========================================================================
    inline uint8_t savedMoveType = 2; // MOVETYPE_WALK

    // Engine command execution via engine2.dll
    typedef void* (*CreateInterfaceFn)(const char* name, int* returnCode);
    inline void* engineClientInterface = nullptr;
    inline bool engineInitialized = false;
    
    inline void InitEngine() {
        if (engineInitialized) return;
        HMODULE engine2 = GetModuleHandleW(L"engine2.dll");
        if (!engine2) return;
        auto createInterface = (CreateInterfaceFn)GetProcAddress(engine2, "CreateInterface");
        if (!createInterface) return;
        engineClientInterface = createInterface("Source2EngineToClient001", nullptr);
        if (engineClientInterface) engineInitialized = true;
    }
    
    // ========================================================================
    //  NOCLIP ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Zero console. Writes autoexec.cfg, CS2 runs it on map load.
    //  Inject BEFORE starting match = completely invisible.
    // ========================================================================
    inline bool autoNoclipDone = false;
    inline std::wstring cfgDirPath;
    
    inline std::wstring GetCfgDir() {
        if (!cfgDirPath.empty()) return cfgDirPath;
        wchar_t clientPath[MAX_PATH] = {};
        HMODULE client = GetModuleHandleW(L"client.dll");
        if (!client) return L"";
        GetModuleFileNameW(client, clientPath, MAX_PATH);
        
        // client.dll: .../game/csgo/bin/win64/client.dll ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ cfg: .../game/csgo/cfg/
        std::wstring path(clientPath);
        auto pos = path.rfind(L"\\bin\\");
        if (pos == std::wstring::npos) return L"";
        cfgDirPath = path.substr(0, pos) + L"\\cfg\\";
        return cfgDirPath;
    }
    
    inline bool WriteCfgFile(const wchar_t* filename, const char* content) {
        std::wstring dir = GetCfgDir();
        if (dir.empty()) return false;
        std::wstring fullPath = dir + filename;
        
        HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;
        DWORD written;
        WriteFile(hFile, content, (DWORD)strlen(content), &written, NULL);
        CloseHandle(hFile);
        return true;
    }
    
    inline void AutoNoclip() {
        if (autoNoclipDone) return;
        autoNoclipDone = true;
        
        // Write autoexec.cfg ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â CS2 runs this on every map load
        // If user injects BEFORE starting match ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ completely silent
        const char* autoexecContent = 
            "sv_cheats 1\n"
            "bind n noclip\n";
        
        WriteCfgFile(L"autoexec.cfg", autoexecContent);
        
        // Also write z.cfg for manual "exec z" if already in match
        const char* zContent = 
            "sv_cheats 1\n"
            "noclip\n"
            "bind n noclip\n";
        
        WriteCfgFile(L"z.cfg", zContent);
        
        noclipActive = true;
    }
    
    inline void NoclipFly(CEntity& localPawn) {
        if (!noclipEnabled) return;
        if (!localPawn.IsValid() || !localPawn.IsAlive()) return;
        
        if (!autoNoclipDone) {
            AutoNoclip();
            return;
        }
        
        // N key handled by game bind (bind n noclip)
        // Just track state for panel UI
        bool keyDown = (GetAsyncKeyState(noclipKey) & 0x8000) != 0;
        if (keyDown && !noclipKeyWasDown) {
            noclipActive = !noclipActive;
        }
        noclipKeyWasDown = keyDown;
    }

    // ========================================================================
    //  NO RECOIL (RCS) ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Compensate aim punch
    // ========================================================================
    namespace NoRecoil {
        inline bool enabled = false;
        inline Vec3 prevPunch = {};
        inline void Run(CEntity& localPawn) {
            if (!enabled) return;
            if (!localPawn.IsValid() || !localPawn.IsAlive()) return;

            __try {
                uintptr_t addr = localPawn.Address;
                
                // Check if we're shooting
                int shotsFired = Memory::Read<int>(addr + Offsets::C_CSPlayerPawn::m_iShotsFired);
                if (shotsFired <= 0) { prevPunch = {}; return; }

                // Read aim punch from services
                uintptr_t punchSvc = Memory::Read<uintptr_t>(
                    addr + Offsets::C_CSPlayerPawn::m_pAimPunchServices);
                if (!punchSvc || punchSvc < 0x10000 || punchSvc > 0x7FFFFFFFFFFF) {
                    prevPunch = {};
                    return;
                }
                if (IsBadReadPtr((void*)punchSvc, 0xB0)) {
                    prevPunch = {};
                    return;
                }

                Vec3 punch = Memory::Read<Vec3>(punchSvc + 0x50); // m_predictableBaseAngle
                
                // Sanity checks
                if (punch.x != punch.x || punch.y != punch.y) { prevPunch = {}; return; }
                if (punch.x < -90 || punch.x > 90 || punch.y < -360 || punch.y > 360) { prevPunch = {}; return; }
                if (punch.x == 0 && punch.y == 0) { prevPunch = {}; return; }

                Vec3 delta;
                delta.x = punch.x - prevPunch.x;
                delta.y = punch.y - prevPunch.y;
                delta.z = 0;
                prevPunch = punch;

                if (delta.x == 0 && delta.y == 0) return;

                // Compensate view angles
                uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
                if (!clientBase) return;

                Vec3 viewAngles = Memory::Read<Vec3>(clientBase + Offsets::dwViewAngles);
                viewAngles.x -= delta.x * 2.0f;
                viewAngles.y -= delta.y * 2.0f;

                if (viewAngles.x > 89.0f) viewAngles.x = 89.0f;
                if (viewAngles.x < -89.0f) viewAngles.x = -89.0f;
                while (viewAngles.y > 180.0f) viewAngles.y -= 360.0f;
                while (viewAngles.y < -180.0f) viewAngles.y += 360.0f;

                Memory::Write<Vec3>(clientBase + Offsets::dwViewAngles, viewAngles);

                // Zero visual punch for clean crosshair
                Vec3 zero = {0, 0, 0};
                // Zero m_vecCsViewPunchAngle on CameraServices
                uintptr_t camSvc = Memory::Read<uintptr_t>(addr + Offsets::C_BasePlayerPawn::m_pCameraServices);
                if (camSvc && camSvc > 0x10000) {
                    Memory::Write<Vec3>(camSvc + 0x48, zero); // m_vecCsViewPunchAngle
                }
                // Also zero predictable punch angle directly
                Memory::Write<Vec3>(punchSvc + 0x50, zero);  // m_predictableBaseAngle
                Memory::Write<Vec3>(punchSvc + 0x5C, zero);  // m_predictableBaseAngleVel

            } __except(EXCEPTION_EXECUTE_HANDLER) {
                prevPunch = {};
            }
        }
    }

    // ========================================================================
    //  SPECTATOR LIST ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Find who is spectating local player
    // ========================================================================
    inline constexpr int MAX_SPECTATORS = 10;
    inline char spectatorNames[MAX_SPECTATORS][64] = {};
    inline int spectatorCount = 0;

    inline void UpdateSpectatorList() {
        spectatorCount = 0;
        if (!spectatorListEnabled) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        uintptr_t localPawn = GetLocalPlayerPawn(clientBase);
        if (!localPawn) return;

        uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
        if (!entityList) return;

        for (int i = 1; i <= 64 && spectatorCount < MAX_SPECTATORS; i++) {
            __try {
                uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * ((i & 0x7FFF) >> 9));
                if (!listEntry || listEntry < 0x10000) continue;
                uintptr_t controller = Memory::Read<uintptr_t>(listEntry + 0x78 * (i & 0x1FF));
                if (!controller || controller < 0x10000 || controller > 0x7FFFFFFFFFFF) continue;

                // Get this player's pawn
                uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
                if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;
                uint32_t idx = pawnHandle & 0x7FFF;
                uintptr_t pawnEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
                if (!pawnEntry || pawnEntry < 0x10000) continue;
                uintptr_t pawn = Memory::Read<uintptr_t>(pawnEntry + 0x78 * (idx & 0x1FF));
                if (!pawn || pawn < 0x10000 || pawn > 0x7FFFFFFFFFFF || pawn == localPawn) continue;

                // Check if this player is dead (spectating)
                uint8_t lifeState = Memory::Read<uint8_t>(pawn + Offsets::C_BaseEntity::m_lifeState);
                if (lifeState == 0) continue; // alive, not spectating

                // Get observer services
                uintptr_t obsSvc = Memory::Read<uintptr_t>(pawn + Offsets::C_BasePlayerPawn::m_pObserverServices);
                if (!obsSvc || obsSvc < 0x10000 || obsSvc > 0x7FFFFFFFFFFF) continue;

                // Check who they're observing
                uint32_t obsTarget = Memory::Read<uint32_t>(obsSvc + Offsets::ObserverServices::m_hObserverTarget);
                if (!obsTarget || obsTarget == 0xFFFFFFFF) continue;

                // Resolve the observed pawn
                uint32_t obsIdx = obsTarget & 0x7FFF;
                uintptr_t obsEntry = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (obsIdx >> 9));
                if (!obsEntry) continue;
                uintptr_t obsPawn = Memory::Read<uintptr_t>(obsEntry + 0x78 * (obsIdx & 0x1FF));

                // If they're watching US
                if (obsPawn == localPawn) {
                    // Read name from controller
                    char name[64] = {};
                    for (int c = 0; c < 63; c++) {
                        name[c] = Memory::Read<char>(controller + Offsets::CBasePlayerController::m_iszPlayerName + c);
                        if (name[c] == 0) break;
                    }
                    if (name[0] != 0) {
                        memcpy(spectatorNames[spectatorCount], name, 64);
                        spectatorCount++;
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
    }

    // ========================================================================
    //  BOMB ESP DATA ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Read planted C4 info
    // ========================================================================
    inline bool bombPlanted = false;
    inline float bombTimer = 0.0f;
    inline Vec3 bombPos = {};
    inline char bombSite = '?';

    inline void UpdateBombInfo() {
        bombPlanted = false;
        if (!bombESPEnabled) return;

        uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
        if (!clientBase) return;

        __try {
            uintptr_t plantedC4 = Memory::Read<uintptr_t>(clientBase + Offsets::dwPlantedC4);
            if (!plantedC4) return;

            // CPlantedC4 is a pointer to a list, first entry is the bomb
            uintptr_t bomb = Memory::Read<uintptr_t>(plantedC4);
            if (!bomb || bomb < 0x10000) return;

            // m_bBombTicking (check if bomb is active)
            bool ticking = Memory::Read<bool>(bomb + 0x1160); // m_bBombTicking (v5 dump)
            if (!ticking) return;

            bombPlanted = true;

            // m_flC4Blow ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â absolute game time when bomb explodes
            float blowTime = Memory::Read<float>(bomb + Offsets::C_PlantedC4::m_flC4Blow);

            // Get current game time from GlobalVars
            uintptr_t globalVars = Memory::Read<uintptr_t>(clientBase + Offsets::dwGlobalVars);
            if (globalVars) {
                float curTime = Memory::Read<float>(globalVars + 0x2C); // Hardcoded 0x2C
                bombTimer = blowTime - curTime;
                if (bombTimer < 0) bombTimer = 0;
            }

            // Bomb site (A or B) from m_nBombSite
            int site = Memory::Read<int>(bomb + Offsets::C_PlantedC4::m_nBombSite);
            bombSite = (site == 0) ? 'A' : 'B';

            // Bomb position
            uintptr_t sceneNode = Memory::Read<uintptr_t>(bomb + Offsets::C_BaseEntity::m_pGameSceneNode);
            if (sceneNode) {
                bombPos = Memory::Read<Vec3>(sceneNode + 0xD0);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            bombPlanted = false;
        }
    }

    // ========================================================================
    //  FOV CHANGER ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Override camera FOV via CameraServices
    // ========================================================================
    inline bool fovEnabled = false;
    inline int fovValue = 110;       // 60-140
    inline int viewmodelFov = 68;    // 54-90

    // FOV uses dynamic Offsets::C_BasePlayerPawn::m_pCameraServices
    // and Offsets::CameraServices::m_iFOV

    inline void ApplyFOV(CEntity& local) {
        if (!local.IsValid() || !local.IsAlive()) return;
        __try {
            uintptr_t pawn = local.Address;
            uintptr_t camSvc = Memory::Read<uintptr_t>(pawn + Offsets::C_BasePlayerPawn::m_pCameraServices);
            if (!camSvc || camSvc < 0x10000) return;

            if (fovEnabled) {
                Memory::Write<uint32_t>(camSvc + Offsets::CameraServices::m_iFOV, (uint32_t)fovValue);
                Memory::Write<uint32_t>(camSvc + Offsets::CameraServices::m_iFOVStart, (uint32_t)fovValue);
                Memory::Write<float>(camSvc + 0x29C, 0.0f);                  // m_flFOVRate = instant
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ========================================================================
    //  SKY COLOR - C_EnvSky tint override
    // ========================================================================
    inline bool skyColorEnabled = false;
    inline float skyColor[4] = {0.2f, 0.0f, 0.6f, 1.0f}; // RGBA for color picker
    inline float skyBrightness = 0.2f;
    inline bool skyUpdateNeeded = false;

    inline void ApplySkyColor(uintptr_t clientBase) {
        static int crashCount = 0;
        if (crashCount >= 2) { skyColorEnabled = false; return; }
        
        static uintptr_t cachedEnt = 0;
        static bool searched = false;
        static bool hasSavedOriginal = false;
        struct Color { uint8_t r, g, b, a; };
        static Color originalTint = {255, 255, 255, 255};
        static Color originalTint2 = {255, 255, 255, 255};
        static float originalBrightness = 1.0f;
        static bool wasEnabled = false;
        
        if (!clientBase) return;

        // Restore original sky color when disabled
        if (!skyColorEnabled) {
            if (wasEnabled && cachedEnt && hasSavedOriginal) {
                __try {
                    Memory::Write<Color>(cachedEnt + Offsets::C_EnvSky::m_vTintColor, originalTint);
                    Memory::Write<Color>(cachedEnt + Offsets::C_EnvSky::m_vTintColorLightingOnly, originalTint2);
                    Memory::Write<float>(cachedEnt + Offsets::C_EnvSky::m_flBrightnessScale, originalBrightness);
                } __except(1) {}
                wasEnabled = false;
            }
            return;
        }

        static DWORD lastTime = 0;
        if (GetTickCount() - lastTime < 3000) return;
        lastTime = GetTickCount();

        __try {
            uintptr_t entityList = *(uintptr_t*)(clientBase + Offsets::dwEntityList);
            if (!entityList || entityList < 0x10000) return;

            // Search for env_sky entity
            if (!cachedEnt || !searched) {
                for (int i = 64; i < 512; i++) {
                    __try {
                        uintptr_t le = *(uintptr_t*)(entityList + 0x10 + 8*((i&0x7FFF)>>9));
                        if (!le || le < 0x10000) continue;
                        uintptr_t ent = *(uintptr_t*)(le + 112*(i&0x1FF));
                        if (!ent || ent < 0x10000) continue;
                        uintptr_t pId = *(uintptr_t*)(ent + 0x10);
                        if (!pId || pId < 0x10000) continue;
                        uintptr_t namePtr = *(uintptr_t*)(pId + 0x20);
                        if (!namePtr || namePtr < 0x10000) continue;
                        
                        char n[8] = {};
                        __try {
                            const char* sName = (const char*)namePtr;
                            for (int c = 0; c < 7 && sName[c]; c++) n[c] = sName[c];
                        } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }

                        if (n[0]=='e'&&n[1]=='n'&&n[2]=='v'&&n[3]=='_'&&n[4]=='s'&&n[5]=='k'&&n[6]=='y') {
                            cachedEnt = ent;
                            break;
                        }
                    } __except(EXCEPTION_EXECUTE_HANDLER) {}
                }
                searched = true;
            }

            // Save original color before first write
            if (cachedEnt && !hasSavedOriginal) {
                __try {
                    originalTint = *(Color*)(cachedEnt + Offsets::C_EnvSky::m_vTintColor);
                    originalTint2 = *(Color*)(cachedEnt + Offsets::C_EnvSky::m_vTintColorLightingOnly);
                    originalBrightness = *(float*)(cachedEnt + Offsets::C_EnvSky::m_flBrightnessScale);
                    hasSavedOriginal = true;
                } __except(1) {}
            }

            // Write sky color
            if (cachedEnt) {
                __try {
                    Color col = { (uint8_t)(skyColor[0]*255), (uint8_t)(skyColor[1]*255), (uint8_t)(skyColor[2]*255), 255 };
                    Memory::Write<Color>(cachedEnt + Offsets::C_EnvSky::m_vTintColor, col);
                    Memory::Write<Color>(cachedEnt + Offsets::C_EnvSky::m_vTintColorLightingOnly, col);
                    Memory::Write<float>(cachedEnt + Offsets::C_EnvSky::m_flBrightnessScale, skyBrightness);
                    wasEnabled = true;
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    crashCount++;
                    cachedEnt = 0;
                    searched = false;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) { crashCount++; }
    }

    // ========================================================================
    //  LANGUAGE — TR/ENG toggle
    // ========================================================================
    inline int language = 0;  // 0=TR, 1=EN

    // ========================================================================
    //  NIGHT MODE — Dark fog via CameraServices fog override
    // ========================================================================
    inline bool nightModeEnabled = false;
    inline float worldBrightness = 0.3f;

    inline void ApplyNightMode(CEntity& local) {
        static bool wasNightMode = false;
        if (!local.IsValid() || !local.IsAlive()) return;
        __try {
            uintptr_t pawn = local.Address;
            uintptr_t cameraSvc = Memory::Read<uintptr_t>(pawn + Offsets::C_BasePlayerPawn::m_pCameraServices);
            if (!cameraSvc || cameraSvc < 0x10000) return;

            if (nightModeEnabled) {
                wasNightMode = true;
                
                Memory::Write<bool>(cameraSvc + 436, true); // m_bOverrideFogColor
                
                struct Color32 { uint8_t r, g, b, a; };
                Color32 fogCol = { (uint8_t)(5 * worldBrightness), (uint8_t)(5 * worldBrightness), (uint8_t)(15 * worldBrightness), 255 };
                Memory::Write<Color32>(cameraSvc + 441, fogCol); // m_OverrideFogColor

                Memory::Write<bool>(cameraSvc + 461, true); // m_bOverrideFogStartEnd
                Memory::Write<float>(cameraSvc + 468, 50.0f); // m_fOverrideFogStart (closer)
                Memory::Write<float>(cameraSvc + 488, 800.0f); // m_fOverrideFogEnd (denser)
            } else if (wasNightMode) {
                Memory::Write<bool>(cameraSvc + 436, false);
                Memory::Write<bool>(cameraSvc + 461, false);
                wasNightMode = false;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ========================================================================
    //  THIRD PERSON (TPS) ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Force third-person camera
    //  m_pObserverServices = 0x11F8 (C_BasePlayerPawn)
    //  m_iObserverMode = 0x48 (CPlayer_ObserverServices)
    //  Mode: 0=None, 1=ThirdPerson, 2=FreeCam
    // ========================================================================
    inline bool tpsEnabled = false;
    inline int tpsKey = 0x54; // T key

    inline void ApplyThirdPerson(CEntity& local) {
        if (!local.IsValid() || !local.IsAlive()) return;
        __try {
            uintptr_t pawn = local.Address;
            uintptr_t obsSvc = Memory::Read<uintptr_t>(pawn + Offsets::C_BasePlayerPawn::m_pObserverServices); // m_pObserverServices
            if (!obsSvc || obsSvc < 0x10000) return;

            if (tpsEnabled) {
                // Force third person view
                Memory::Write<uint8_t>(obsSvc + Offsets::ObserverServices::m_hObserverTarget - 4, 1);  // m_iObserverMode
            } else {
                uint8_t curMode = Memory::Read<uint8_t>(obsSvc + Offsets::ObserverServices::m_hObserverTarget - 4);
                if (curMode == 1) {
                    Memory::Write<uint8_t>(obsSvc + Offsets::ObserverServices::m_hObserverTarget - 4, 0);
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ========================================================================
    //  CROSSHAIR OVERLAY ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Custom crosshair rendered via ESP
    // ========================================================================
    inline bool customCrosshairEnabled = false;
    inline int crosshairSize = 5;
    inline int crosshairGap = 2;
    inline float crosshairColor[4] = {0.0f, 1.0f, 0.0f, 1.0f}; // Green

    // ========================================================================
    //  HIT SOUND ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Play sound on hit (beep)
    // ========================================================================
    inline bool hitSoundEnabled = false;

    // ========================================================================
    //  WATERMARK ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â Show cheat name on screen
    // ========================================================================
    inline bool watermarkEnabled = true;
    inline bool showFpsPing = true;  // FPS + Ping display
}

// ========================================================================
//  SKIN CHANGER ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Â  Per-weapon fallback method
// ========================================================================
namespace SkinChanger {
    inline bool enabled = true;

    // Per-weapon skin selection (index into skinList)
    inline int skinAK    = 0;
    inline int skinAWP   = 0;
    inline int skinM4A4  = 0;
    inline int skinM4A1  = 0;
    inline int skinDeagle= 0;
    inline int skinUSP   = 0;
    inline int skinGlock = 0;
    inline int skinKnife = 0;
    inline int knifeModel = 0;
    inline int gloveModel = 0;
    inline int skinGlove = 0;
    inline int selectedSkin = 0; // legacy menu compat

    // Weapon offsets - Linked to dynamic Offsets:: namespace
    inline uintptr_t& OFF_WeaponServices    = Offsets::C_BasePlayerPawn::m_pWeaponServices;
    inline uintptr_t& OFF_hActiveWeapon      = Offsets::CPlayer_WeaponServices::m_hActiveWeapon;
    inline uintptr_t& OFF_AttributeManager   = Offsets::C_EconEntity::m_AttributeManager;
    inline uintptr_t& OFF_Item               = Offsets::C_AttributeContainer::m_Item;
    inline uintptr_t& OFF_ItemIDHigh         = Offsets::C_EconItemView::m_iItemIDHigh;
    inline uintptr_t& OFF_ItemDefIndex       = Offsets::C_EconItemView::m_iItemDefinitionIndex;
    inline uintptr_t& OFF_FallbackPaintKit   = Offsets::C_EconEntity::m_nFallbackPaintKit;
    inline uintptr_t& OFF_FallbackSeed       = Offsets::C_EconEntity::m_nFallbackSeed;
    inline uintptr_t& OFF_FallbackWear       = Offsets::C_EconEntity::m_flFallbackWear;
    inline uintptr_t& OFF_FallbackStatTrak   = Offsets::C_EconEntity::m_nFallbackStatTrak;
    // Note: EIV_Quality / OFF_EntityQuality is rarely updated dynamically, keeping it hardcoded if missing
    inline constexpr uintptr_t OFF_EntityQuality      = 0x1BC;

    struct SkinEntry { const char* name; int paintKit; const char* weapon; };
    inline SkinEntry skinList[] = {
        {"Yok",                 0,      "---"},
        {"AK | Fire Serpent",   180,    "AK"},
        {"AK | Vulcan",         302,    "AK"},
        {"AK | Aquamarine",     456,    "AK"},
        {"AK | Neon Rider",     737,    "AK"},
        {"AK | Bloodsport",     639,    "AK"},
        {"AK | Empress",        675,    "AK"},
        {"AK | Case Hardened",  44,     "AK"},
        {"AK | Redline",        282,    "AK"},
        {"AK | Asiimov",        801,    "AK"},
        {"M4A4 | Howl",         309,    "M4A4"},
        {"M4A4 | Asiimov",      279,    "M4A4"},
        {"M4A4 | Neo-Noir",     695,    "M4A4"},
        {"M4A4 | The Emperor",  756,    "M4A4"},
        {"M4A4 | Spider Lily",  1048,   "M4A4"},
        {"M4A1 | Hyper Beast",  430,    "M4A1"},
        {"M4A1 | Printstream",  909,    "M4A1"},
        {"M4A1 | Player Two",   796,    "M4A1"},
        {"M4A1 | Mecha Ind.",   587,    "M4A1"},
        {"AWP | Dragon Lore",   344,    "AWP"},
        {"AWP | Medusa",        446,    "AWP"},
        {"AWP | Gungnir",       756,    "AWP"},
        {"AWP | Asiimov",       279,    "AWP"},
        {"AWP | Hyper Beast",   475,    "AWP"},
        {"AWP | Lightning",     171,    "AWP"},
        {"AWP | Printstream",   936,    "AWP"},
        {"AWP | Chromatic",     1064,   "AWP"},
        {"Deagle | Blaze",      37,     "DEAG"},
        {"Deagle | Code Red",   735,    "DEAG"},
        {"Deagle | Printstream", 937,   "DEAG"},
        {"Deagle | Ocean Drive",1040,   "DEAG"},
        {"USP | Kill Confirmed", 504,   "USP"},
        {"USP | Neo-Noir",      653,    "USP"},
        {"USP | Printstream",   911,    "USP"},
        {"USP | Orion",         313,    "USP"},
        {"Glock | Fade",        38,     "GLK"},
        {"Glock | Water Elem.", 353,    "GLK"},
        {"Glock | Bullet Queen", 856,   "GLK"},
        {"Glock | Vogue",       1042,   "GLK"},
        // === KNIFE SKINS - COMPLETE LIST ===
        // Basic
        {"Knife | Vanilla",          0,    "KNF"},
        {"Knife | Crimson Web",     12,    "KNF"},
        {"Knife | Fade",            38,    "KNF"},
        {"Knife | Night",           40,    "KNF"},
        {"Knife | Blue Steel",      42,    "KNF"},
        {"Knife | Stained",         43,    "KNF"},
        {"Knife | Case Hardened",   44,    "KNF"},
        {"Knife | Slaughter",       59,    "KNF"},
        {"Knife | Safari Mesh",     72,    "KNF"},
        {"Knife | Boreal Forest",   77,    "KNF"},
        {"Knife | Urban Masked",    98,    "KNF"},
        {"Knife | Scorched",       175,    "KNF"},
        {"Knife | Forest DDPAT",     5,    "KNF"},
        // Chroma
        {"Knife | Tiger Tooth",    409,    "KNF"},
        {"Knife | Damascus Steel", 410,    "KNF"},
        {"Knife | Ultraviolet",    411,    "KNF"},
        {"Knife | Marble Fade",    413,    "KNF"},
        {"Knife | Rust Coat",      414,    "KNF"},
        // Doppler
        {"Knife | Doppler P1",     418,    "KNF"},
        {"Knife | Doppler P2",     419,    "KNF"},
        {"Knife | Doppler P3",     420,    "KNF"},
        {"Knife | Doppler P4",     421,    "KNF"},
        {"Knife | Doppler Ruby",   415,    "KNF"},
        {"Knife | Doppler Sapphire",416,   "KNF"},
        {"Knife | Doppler BlackP", 417,    "KNF"},
        // Gamma Doppler
        {"Knife | Gamma Dopp P1",  568,    "KNF"},
        {"Knife | Gamma Dopp P2",  569,    "KNF"},
        {"Knife | Gamma Dopp P3",  570,    "KNF"},
        {"Knife | Gamma Dopp P4",  571,    "KNF"},
        {"Knife | Gamma Emerald",  568,    "KNF"},
        // Other
        {"Knife | Autotronic",     422,    "KNF"},
        {"Knife | Lore",           445,    "KNF"},
        {"Knife | Black Laminate", 446,    "KNF"},
        {"Knife | Bright Water",   617,    "KNF"},
        {"Knife | Freehand",       725,    "KNF"},
    };
    inline constexpr int SKIN_COUNT = sizeof(skinList) / sizeof(SkinEntry);
    inline int selectedWeaponCat = 0;
    inline const char* weaponCats[] = {"Tumu","AK","M4A4","M4A1","AWP","DEAG","USP","GLK","KNF","GLV"};
    inline constexpr int WEAPON_CAT_COUNT = 10;

    // === KNIFE SKIN FILTERED LIST (only KNF entries from skinList) ===
    // Built at compile-time by scanning skinList for weapon == "KNF"
    struct KnifeSkinMap {
        int indices[64]; // indices into skinList that are KNF
        const char* names[64];
        int count = 0;
        
        constexpr KnifeSkinMap() : indices{}, names{} {
            // Can't use constexpr with strcmp, so we build at startup
        }
    };
    inline int knifeSkinIndices[64] = {};  // maps position -> skinList index
    inline const char* knifeSkinNames[64] = {};
    inline int KNIFE_SKIN_COUNT = 0;
    inline bool knifeSkinBuilt = false;
    
    inline void BuildKnifeSkinList() {
        if (knifeSkinBuilt) return;
        KNIFE_SKIN_COUNT = 0;
        // First entry: "Yok" (no skin)
        knifeSkinIndices[0] = 0;
        knifeSkinNames[0] = "Yok";
        KNIFE_SKIN_COUNT = 1;
        // Scan for KNF entries
        for (int i = 1; i < SKIN_COUNT && KNIFE_SKIN_COUNT < 63; i++) {
            if (strcmp(skinList[i].weapon, "KNF") == 0) {
                knifeSkinIndices[KNIFE_SKIN_COUNT] = i;
                knifeSkinNames[KNIFE_SKIN_COUNT] = skinList[i].name;
                KNIFE_SKIN_COUNT++;
            }
        }
        knifeSkinBuilt = true;
    }
    
    // Get knife paint kit from knife-specific index
    inline int GetKnifePaintKit(int knifeSkinIdx) {
        BuildKnifeSkinList();
        if (knifeSkinIdx <= 0 || knifeSkinIdx >= KNIFE_SKIN_COUNT) return 0;
        int realIdx = knifeSkinIndices[knifeSkinIdx];
        if (realIdx > 0 && realIdx < SKIN_COUNT) return skinList[realIdx].paintKit;
        return 0;
    }

    // Glove skin list
    struct GloveSkinEntry { const char* name; int paintKit; };
    inline GloveSkinEntry gloveSkinList[] = {
        {"Yok",                 0},
        {"Crimson Kimono",      10036},
        {"Emerald Web",         10034},
        {"Foundation",          10037},
        {"Crimson Weave",       10016},
        {"Lunar Weave",         10018},
        {"Vice",                10048},
        {"Pandora's Box",       10037},
        {"Superconductor",      10038},
        {"Hedge Maze",          10040},
        {"Fade",                10063},
        {"Slaughter",           10021},
        {"Mogul",               10024},
        {"Overprint",           10064},
        {"Amphibious",          10043},
        {"Arid",                10044},
        {"Snakebite",           10045},
        {"Polygon",             10047},
        {"Boom!",               10049},
        {"Cool Mint",           10050},
        {"Forest DDPAT",        10053},
        {"Transport",           10054},
    };
    inline constexpr int GLOVE_SKIN_COUNT = sizeof(gloveSkinList) / sizeof(GloveSkinEntry);

    struct KnifeEntry { const char* name; uint16_t defIndex; const char* modelPath; };
    // ALL paths verified from pak01_dir.vpk on 2026-05-26
    inline KnifeEntry knifeList[] = {
        {"Yok",           0,   nullptr},
        {"Bayonet",     500,  "weapons/models/knife/knife_bayonet/weapon_knife_bayonet.vmdl"},
        {"Classic Knife",503, "weapons/models/knife/knife_css/weapon_knife_css.vmdl"},
        {"Flip Knife",  505,  "weapons/models/knife/knife_flip/weapon_knife_flip.vmdl"},
        {"Gut Knife",   506,  "weapons/models/knife/knife_gut/weapon_knife_gut.vmdl"},
        {"Karambit",    507,  "weapons/models/knife/knife_karambit/weapon_knife_karambit.vmdl"},
        {"M9 Bayonet",  508,  "weapons/models/knife/knife_m9/weapon_knife_m9.vmdl"},
        {"Huntsman",    509,  "weapons/models/knife/knife_tactical/weapon_knife_tactical.vmdl"},
        {"Falchion",    512,  "weapons/models/knife/knife_falchion/weapon_knife_falchion.vmdl"},
        {"Bowie",       514,  "weapons/models/knife/knife_bowie/weapon_knife_bowie.vmdl"},
        {"Butterfly",   515,  "weapons/models/knife/knife_butterfly/weapon_knife_butterfly.vmdl"},
        {"Shadow Daggers",516,"weapons/models/knife/knife_push/weapon_knife_push.vmdl"},
        {"Navaja",      520,  "weapons/models/knife/knife_navaja/weapon_knife_navaja.vmdl"},
        {"Stiletto",    522,  "weapons/models/knife/knife_stiletto/weapon_knife_stiletto.vmdl"},
        {"Talon",       523,  "weapons/models/knife/knife_talon/weapon_knife_talon.vmdl"},
        {"Ursus",       519,  "weapons/models/knife/knife_ursus/weapon_knife_ursus.vmdl"},
        {"Skeleton",    525,  "weapons/models/knife/knife_skeleton/weapon_knife_skeleton.vmdl"},
        {"Kukri",       526,  "weapons/models/knife/knife_kukri/weapon_knife_kukri.vmdl"},
        {"Cord",        517,  "weapons/models/knife/knife_cord/weapon_knife_cord.vmdl"},
        {"Canis",       518,  "weapons/models/knife/knife_canis/weapon_knife_canis.vmdl"},
        {"Outdoor",     521,  "weapons/models/knife/knife_outdoor/weapon_knife_outdoor.vmdl"},
    };
    inline constexpr int KNIFE_COUNT = sizeof(knifeList) / sizeof(KnifeEntry);

    struct GloveEntry { const char* name; uint16_t defIndex; };
    inline GloveEntry gloveList[] = {
        {"Yok",0},{"Sport Gloves",5030},{"Driver Gloves",5031},
        {"Hand Wraps",5032},{"Moto Gloves",5033},{"Specialist",5034},
        {"Bloodhound",5027},
    };
    inline constexpr int GLOVE_COUNT = sizeof(gloveList) / sizeof(GloveEntry);

    // ========== PLAYER MODEL CHANGER ==========
    inline int ctModel = 0; // CT model selection
    inline int tModel  = 0; // T model selection
    inline bool playerModelEnabled = false;
    // Player model uses dynamic Offsets::C_BaseEntity::m_iTeamNum

    struct PlayerModelEntry { const char* name; const char* modelPath; };

    inline PlayerModelEntry ctModelList[] = {
        {"Yok",              nullptr},
        {"SAS",              "characters/models/ctm_sas/ctm_sas.vmdl"},
        {"SAS Var.F",        "agents/models/ctm_sas/ctm_sas_variantf.vmdl"},
        {"SAS Var.G",        "agents/models/ctm_sas/ctm_sas_variantg.vmdl"},
        {"FBI",              "agents/models/ctm_fbi/ctm_fbi.vmdl"},
        {"FBI Var.A",        "agents/models/ctm_fbi/ctm_fbi_varianta.vmdl"},
        {"FBI Var.B",        "agents/models/ctm_fbi/ctm_fbi_variantb.vmdl"},
        {"FBI Var.C",        "agents/models/ctm_fbi/ctm_fbi_variantc.vmdl"},
        {"FBI Var.D",        "agents/models/ctm_fbi/ctm_fbi_variantd.vmdl"},
        {"FBI Var.E",        "agents/models/ctm_fbi/ctm_fbi_variante.vmdl"},
        {"FBI Var.F",        "agents/models/ctm_fbi/ctm_fbi_variantf.vmdl"},
        {"FBI Var.G",        "agents/models/ctm_fbi/ctm_fbi_variantg.vmdl"},
        {"FBI Var.H",        "agents/models/ctm_fbi/ctm_fbi_varianth.vmdl"},
        {"Gendarmerie A",    "agents/models/ctm_gendarmerie/ctm_gendarmerie_varianta.vmdl"},
        {"Gendarmerie B",    "agents/models/ctm_gendarmerie/ctm_gendarmerie_variantb.vmdl"},
        {"Gendarmerie C",    "agents/models/ctm_gendarmerie/ctm_gendarmerie_variantc.vmdl"},
        {"Gendarmerie D",    "agents/models/ctm_gendarmerie/ctm_gendarmerie_variantd.vmdl"},
        {"Gendarmerie E",    "agents/models/ctm_gendarmerie/ctm_gendarmerie_variante.vmdl"},
        {"ST6 Var.E",        "agents/models/ctm_st6/ctm_st6_variante.vmdl"},
        {"ST6 Var.G",        "agents/models/ctm_st6/ctm_st6_variantg.vmdl"},
        {"ST6 Var.I",        "agents/models/ctm_st6/ctm_st6_varianti.vmdl"},
        {"ST6 Var.J",        "agents/models/ctm_st6/ctm_st6_variantj.vmdl"},
        {"ST6 Var.K",        "agents/models/ctm_st6/ctm_st6_variantk.vmdl"},
        {"ST6 Var.L",        "agents/models/ctm_st6/ctm_st6_variantl.vmdl"},
        {"ST6 Var.M",        "agents/models/ctm_st6/ctm_st6_variantm.vmdl"},
        {"ST6 Var.N",        "agents/models/ctm_st6/ctm_st6_variantn.vmdl"},
        {"SWAT Var.E",       "agents/models/ctm_swat/ctm_swat_variante.vmdl"},
        {"SWAT Var.F",       "agents/models/ctm_swat/ctm_swat_variantf.vmdl"},
        {"SWAT Var.G",       "agents/models/ctm_swat/ctm_swat_variantg.vmdl"},
        {"SWAT Var.H",       "agents/models/ctm_swat/ctm_swat_varianth.vmdl"},
        {"SWAT Var.I",       "agents/models/ctm_swat/ctm_swat_varianti.vmdl"},
        {"SWAT Var.J",       "agents/models/ctm_swat/ctm_swat_variantj.vmdl"},
        {"SWAT Var.K",       "agents/models/ctm_swat/ctm_swat_variantk.vmdl"},
        {"Diver A",          "agents/models/ctm_diver/ctm_diver_varianta.vmdl"},
        {"Diver B",          "agents/models/ctm_diver/ctm_diver_variantb.vmdl"},
        {"Diver C",          "agents/models/ctm_diver/ctm_diver_variantc.vmdl"},
    };
    inline constexpr int CT_MODEL_COUNT = sizeof(ctModelList) / sizeof(PlayerModelEntry);

    inline PlayerModelEntry tModelList[] = {
        {"Yok",              nullptr},
        {"Phoenix",          "agents/models/tm_phoenix/tm_phoenix.vmdl"},
        {"Phoenix Var.A",    "agents/models/tm_phoenix/tm_phoenix_varianta.vmdl"},
        {"Phoenix Var.B",    "agents/models/tm_phoenix/tm_phoenix_variantb.vmdl"},
        {"Phoenix Var.C",    "agents/models/tm_phoenix/tm_phoenix_variantc.vmdl"},
        {"Phoenix Var.D",    "agents/models/tm_phoenix/tm_phoenix_variantd.vmdl"},
        {"Phoenix Var.F",    "agents/models/tm_phoenix/tm_phoenix_variantf.vmdl"},
        {"Phoenix Var.G",    "agents/models/tm_phoenix/tm_phoenix_variantg.vmdl"},
        {"Phoenix Var.H",    "agents/models/tm_phoenix/tm_phoenix_varianth.vmdl"},
        {"Phoenix Var.I",    "agents/models/tm_phoenix/tm_phoenix_varianti.vmdl"},
        {"Leet A",           "agents/models/tm_leet/tm_leet_varianta.vmdl"},
        {"Leet B",           "agents/models/tm_leet/tm_leet_variantb.vmdl"},
        {"Leet C",           "agents/models/tm_leet/tm_leet_variantc.vmdl"},
        {"Leet D",           "agents/models/tm_leet/tm_leet_variantd.vmdl"},
        {"Leet E",           "agents/models/tm_leet/tm_leet_variante.vmdl"},
        {"Leet F",           "agents/models/tm_leet/tm_leet_variantf.vmdl"},
        {"Leet G",           "agents/models/tm_leet/tm_leet_variantg.vmdl"},
        {"Leet H",           "agents/models/tm_leet/tm_leet_varianth.vmdl"},
        {"Leet I",           "agents/models/tm_leet/tm_leet_varianti.vmdl"},
        {"Leet J",           "agents/models/tm_leet/tm_leet_variantj.vmdl"},
        {"Leet K",           "agents/models/tm_leet/tm_leet_variantk.vmdl"},
        {"Balkan F",         "agents/models/tm_balkan/tm_balkan_variantf.vmdl"},
        {"Balkan G",         "agents/models/tm_balkan/tm_balkan_variantg.vmdl"},
        {"Balkan H",         "agents/models/tm_balkan/tm_balkan_varianth.vmdl"},
        {"Balkan I",         "agents/models/tm_balkan/tm_balkan_varianti.vmdl"},
        {"Balkan J",         "agents/models/tm_balkan/tm_balkan_variantj.vmdl"},
        {"Balkan K",         "agents/models/tm_balkan/tm_balkan_variantk.vmdl"},
        {"Balkan L",         "agents/models/tm_balkan/tm_balkan_variantl.vmdl"},
        {"Pro F",            "agents/models/tm_professional/tm_professional_varf.vmdl"},
        {"Pro F1",           "agents/models/tm_professional/tm_professional_varf1.vmdl"},
        {"Pro F2",           "agents/models/tm_professional/tm_professional_varf2.vmdl"},
        {"Pro F3",           "agents/models/tm_professional/tm_professional_varf3.vmdl"},
        {"Pro F4",           "agents/models/tm_professional/tm_professional_varf4.vmdl"},
        {"Pro F5",           "agents/models/tm_professional/tm_professional_varf5.vmdl"},
        {"Pro G",            "agents/models/tm_professional/tm_professional_varg.vmdl"},
        {"Pro H",            "agents/models/tm_professional/tm_professional_varh.vmdl"},
        {"Pro I",            "agents/models/tm_professional/tm_professional_vari.vmdl"},
        {"Pro J",            "agents/models/tm_professional/tm_professional_varj.vmdl"},
        {"Jungle Raider A",  "agents/models/tm_jungle_raider/tm_jungle_raider_varianta.vmdl"},
        {"Jungle Raider B",  "agents/models/tm_jungle_raider/tm_jungle_raider_variantb.vmdl"},
        {"Jungle Raider C",  "agents/models/tm_jungle_raider/tm_jungle_raider_variantc.vmdl"},
        {"Jungle Raider D",  "agents/models/tm_jungle_raider/tm_jungle_raider_variantd.vmdl"},
        {"Jungle Raider E",  "agents/models/tm_jungle_raider/tm_jungle_raider_variante.vmdl"},
        {"Jungle Raider F",  "agents/models/tm_jungle_raider/tm_jungle_raider_variantf.vmdl"},
    };
    inline constexpr int T_MODEL_COUNT = sizeof(tModelList) / sizeof(PlayerModelEntry);


    inline float skinWear = 0.0001f;
    inline int skinSeed = 0;
    inline bool needsUpdate = false; // set ONLY from menu HandleAction

    // Get weapon category from defIndex
    inline const char* GetWeaponCat(uint16_t d) {
        switch(d) {
            case 7:  return "AK";   case 16: return "M4A4"; case 60: return "M4A1";
            case 9:  return "AWP";  case 1:  return "DEAG"; case 61: return "USP";
            case 4:  return "GLK";  case 42: case 59: return "KNF";
            default: return (d >= 500 && d <= 526) ? "KNF" : nullptr;
        }
    }

    // Get paint kit for this weapon type
    inline int GetPaintKit(uint16_t defIdx) {
        const char* cat = GetWeaponCat(defIdx);
        if (!cat) return 0;

        // Knives use their own filtered list
        if (strcmp(cat, "KNF") == 0) {
            return GetKnifePaintKit(skinKnife);
        }

        int idx = 0;
        if      (strcmp(cat,"AK")==0)   idx = skinAK;
        else if (strcmp(cat,"AWP")==0)  idx = skinAWP;
        else if (strcmp(cat,"M4A4")==0) idx = skinM4A4;
        else if (strcmp(cat,"M4A1")==0) idx = skinM4A1;
        else if (strcmp(cat,"DEAG")==0) idx = skinDeagle;
        else if (strcmp(cat,"USP")==0)  idx = skinUSP;
        else if (strcmp(cat,"GLK")==0)  idx = skinGlock;

        if (idx > 0 && idx < SKIN_COUNT) {
            if (strcmp(skinList[idx].weapon, cat) == 0)
                return skinList[idx].paintKit;
        }

        if (selectedSkin > 0 && selectedSkin < SKIN_COUNT) {
            const char* skinCat = skinList[selectedSkin].weapon;
            if (strcmp(skinCat, cat) == 0)
                return skinList[selectedSkin].paintKit;
        }
        return 0;
    }

    inline void ApplySkin(uintptr_t localPawn) {
        if (!enabled || !localPawn) return;
        __try {
            uintptr_t weaponSvc = Memory::Read<uintptr_t>(localPawn + OFF_WeaponServices);
            if (!weaponSvc || weaponSvc < 0x10000) return;

            uintptr_t clientBase = Memory::GetModuleBase(L"client.dll");
            if (!clientBase) return;
            uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
            if (!entityList) return;

            // Process ALL weapons — SKIP knives (KnifeHook handles them via FSN hook)
            for (int slot = 0; slot < 16; slot++) {
                __try {
                    uint32_t wHandle = Memory::Read<uint32_t>(weaponSvc + 0x48 + slot * 4);
                    if (!wHandle || wHandle == 0xFFFFFFFF) continue;

                    uint32_t wi = wHandle & 0x7FFF;
                    uintptr_t we = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (wi >> 9));
                    if (!we || we < 0x10000) continue;
                    uintptr_t weapon = Memory::Read<uintptr_t>(we + 0x70 * (wi & 0x1FF));
                    if (!weapon || weapon < 0x10000) continue;

                    uintptr_t item = weapon + OFF_AttributeManager + OFF_Item;
                    uint16_t defIdx = Memory::Read<uint16_t>(item + OFF_ItemDefIndex);
                    if (defIdx == 0) continue;

                    // SKIP knives — ProcessKnife (FSN hook thread) handles them
                    // Writing from 2 threads = race condition = CRASH
                    bool isKnife = (defIdx == 42 || defIdx == 59 || (defIdx >= 500 && defIdx <= 526));
                    if (isKnife) continue;

                    int paintKit = GetPaintKit(defIdx);
                    if (paintKit <= 0) continue;

                    // Write fallback values for guns only
                    Memory::Write<int>(weapon + OFF_FallbackPaintKit, paintKit);
                    Memory::Write<float>(weapon + OFF_FallbackWear, skinWear);
                    Memory::Write<int>(weapon + OFF_FallbackSeed, skinSeed);
                    Memory::Write<int>(weapon + OFF_FallbackStatTrak, -1);
                    Memory::Write<uint64_t>(item + 0x1C8, 1);
                    Memory::Write<uint32_t>(item + OFF_ItemIDHigh, (uint32_t)-1);
                    Memory::Write<uint32_t>(item + 0x1D4, 0);
                    Memory::Write<uint32_t>(item + 0x1D8, 0);
                    Memory::Write<int>(item + OFF_EntityQuality, 4);

                } __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
            }

            // ForceFullUpdate is now handled in KnifeHook::HookedFSN (game thread)
            // needsUpdate flag is read there, not here

        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

