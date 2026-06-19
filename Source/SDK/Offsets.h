#pragma once
#include <cstdint>

namespace Offsets {

    // =====================================================
    // GLOBAL OFFSETS (from offsets.json)
    // =====================================================
    inline uintptr_t dwEntityList              = 0x0;
    inline uintptr_t dwLocalPlayerController   = 0x0;
    inline uintptr_t dwLocalPlayerPawn         = 0x0;
    inline uintptr_t dwViewMatrix              = 0x0;
    inline uintptr_t dwGlobalVars              = 0x0;
    inline uintptr_t dwPlantedC4               = 0x0;
    inline uintptr_t dwSensitivity             = 0x0;
    inline uintptr_t dwSensitivity_sensitivity = 0x0;
    inline uintptr_t dwViewAngles              = 0x0;
    inline uintptr_t dwAttack                  = 0x0;
    inline uintptr_t dwJump                    = 0x0;

    // Engine2.dll offsets (from offsets.json engine2.dll section)
    inline uintptr_t dwNetworkGameClient              = 0x0;
    inline uintptr_t dwNetworkGameClient_signOnState  = 0x0;
    inline uintptr_t dwNetworkGameClient_deltaTick    = 0x0;
    inline uintptr_t dwBuildNumber                    = 0x0;

    // =====================================================
    // NETVARS (defaults from latest a2x dump, overridden at runtime)
    // =====================================================

    namespace C_BaseEntity {
        inline uintptr_t m_iHealth          = 844;
        inline uintptr_t m_iMaxHealth       = 840;
        inline uintptr_t m_iTeamNum         = 1003;
        inline uintptr_t m_pGameSceneNode   = 816;
        inline uintptr_t m_fFlags           = 1016;   // WAS 0x3F8=1016, flat search was returning 99!
        inline uintptr_t m_vecAbsVelocity   = 1020;
        inline uintptr_t m_lifeState        = 852;
        inline uintptr_t m_MoveType         = 1317;
        inline uintptr_t m_nSubclassID      = 896;
    }

    namespace CBasePlayerController {
        inline uintptr_t m_hPawn          = 1724;
        inline uintptr_t m_iszPlayerName  = 1780;
    }

    namespace CCSPlayerController {
        inline uintptr_t m_hPlayerPawn       = 2316;
        inline uintptr_t m_bPawnIsAlive      = 2324;
        inline uintptr_t m_iPawnHealth       = 2328;
        inline uintptr_t m_iPawnArmor        = 2332;
        inline uintptr_t m_bPawnHasDefuser   = 2336;
        inline uintptr_t m_bPawnHasHelmet    = 2337;
        inline uintptr_t m_iMusicKitID       = 2376;
        inline uintptr_t m_pInventoryServices = 2080;
        inline uintptr_t m_iScore            = 2356;
    }

    namespace C_CSPlayerPawn {
        inline uintptr_t m_entitySpottedState  = 7224;
        inline uintptr_t m_angEyeAngles       = 13088;
        inline uintptr_t m_iIDEntIndex        = 13308;
        inline uintptr_t m_bIsWalking         = 7216;
        inline uintptr_t m_flVelocityModifier = 7276;
        inline uintptr_t m_pBulletServices    = 5224;
        inline uintptr_t m_pAimPunchServices  = 5264;
        inline uintptr_t m_iShotsFired        = 7268;
        inline uintptr_t m_ArmorValue         = 7292;
        inline uintptr_t m_bIsScoped          = 7248;
        inline uintptr_t m_bIsDefusing        = 7250;
    }

    namespace EntitySpottedState {
        inline uintptr_t m_bSpotted       = 0x8;
        inline uintptr_t m_bSpottedByMask = 0xC;
    }

    namespace AimPunchServices {
        inline uintptr_t m_predictableBaseAngle = 0x50;
    }

    namespace CSkeletonInstance {
        inline uintptr_t m_modelState = 336; // 0x150
    }
    inline uintptr_t BoneArrayOffset = 0x80;

    // Econ / Inventory
    namespace C_EconEntity {
        inline uintptr_t m_AttributeManager = 4480;
        inline uintptr_t m_nFallbackPaintKit = 5720;
        inline uintptr_t m_nFallbackSeed     = 5724;
        inline uintptr_t m_flFallbackWear    = 5728;
        inline uintptr_t m_nFallbackStatTrak = 5732;
    }
    
    namespace C_AttributeContainer {
        inline uintptr_t m_Item = 80;
    }

    namespace C_EconItemView {
        inline uintptr_t m_iItemDefinitionIndex = 442;
        inline uintptr_t m_iItemID              = 456;
        inline uintptr_t m_iItemIDHigh          = 464;
        inline uintptr_t m_iItemIDLow           = 468;
        inline uintptr_t m_iAccountID           = 472;
    }

    namespace CPlayer_WeaponServices {
        inline uintptr_t m_hMyWeapons    = 72;
        inline uintptr_t m_hActiveWeapon = 96;
    }

    namespace C_BasePlayerPawn {
        inline uintptr_t m_pWeaponServices    = 4576;
        inline uintptr_t m_pObserverServices  = 4600;
        inline uintptr_t m_pCameraServices    = 4632;
        inline uintptr_t m_vOldOrigin         = 5008;
        inline uintptr_t m_hMyWearables       = 4552;
    }
    
    namespace C_CSPlayerPawnBase {
        inline uintptr_t m_flFlashDuration     = 5120;
        inline uintptr_t m_flFlashMaxAlpha     = 5116;
    }
    
    namespace CameraServices {
        inline uintptr_t m_iFOV      = 0x290;
        inline uintptr_t m_iFOVStart = 0x294;
    }

    namespace ObserverServices {
        inline uintptr_t m_hObserverTarget     = 76;
    }
    
    namespace InventoryServices {
        inline uintptr_t m_unMusicID = 88;
    }

    namespace C_PlantedC4 {
        inline uintptr_t m_nBombSite         = 0x1164;
        inline uintptr_t m_flC4Blow          = 0x1190;
        inline uintptr_t m_bBeingDefused     = 0x119C;
        inline uintptr_t m_flDefuseCountDown = 0x11B0;
        inline uintptr_t m_bC4Activated      = 0x11A8;
    }

    // Glow / SceneChams
    namespace CGlowProperty {
        inline uintptr_t m_Glow = 3544;  // from C_BaseModelEntity
    }

    namespace C_BaseModelEntity {
        inline uintptr_t m_clrRender = 3224; // 0xC98
    }

    namespace C_SmokeGrenadeProjectile {
        inline uintptr_t m_bDidSmokeEffect = 4692; // 0x1254
        inline uintptr_t m_nSmokeEffectTickBegin = 4696; // 0x1258
    }

    // GlobalVars
    namespace GlobalVar {
        inline uintptr_t CurrentTime = 0x2C;
        inline uintptr_t TickCount = 0x40;
        inline uintptr_t IntervalPerTick = 0x14;
    }

    namespace C_EnvSky {
        inline uintptr_t m_vTintColor = 4025; 
        inline uintptr_t m_vTintColorLightingOnly = 4029;
        inline uintptr_t m_flBrightnessScale = 4036; 
    }
}
