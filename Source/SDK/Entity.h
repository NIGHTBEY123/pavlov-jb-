#pragma once
#include "Offsets.h"
#include "..\Core\Memory.h"
#include <cmath>

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;

    bool IsZero() const { return x == 0.0f && y == 0.0f && z == 0.0f; }
    bool IsValid() const { return !isnan(x) && !isnan(y) && !isnan(z); }
    float DistanceTo(const Vec3& o) const {
        float dx = x - o.x, dy = y - o.y, dz = z - o.z;
        return sqrtf(dx*dx + dy*dy + dz*dz);
    }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
};

struct ViewMatrix {
    float matrix[4][4];
};

constexpr int BONE_HEAD       = 7;
constexpr int BONE_NECK       = 6;
constexpr int BONE_CHEST      = 5;
constexpr int BONE_PELVIS     = 0;
constexpr int BONE_LEFT_HIP   = 23;
constexpr int BONE_LEFT_KNEE  = 24;
constexpr int BONE_LEFT_FOOT  = 25;
constexpr int BONE_RIGHT_HIP  = 27;
constexpr int BONE_RIGHT_KNEE = 28;
constexpr int BONE_RIGHT_FOOT = 29;
constexpr int BONE_LEFT_SHOULDER = 8;
constexpr int BONE_LEFT_ELBOW    = 9;
constexpr int BONE_LEFT_HAND     = 10;
constexpr int BONE_RIGHT_SHOULDER = 13;
constexpr int BONE_RIGHT_ELBOW    = 14;
constexpr int BONE_RIGHT_HAND     = 15;

class CEntity {
public:
    uintptr_t Address = 0;

    CEntity() {}
    CEntity(uintptr_t addr) : Address(addr) {}
    CEntity(const CEntity& o) : Address(o.Address) {}

    bool IsValid() const { return Address && Address >= 0x10000; }
    explicit operator bool() const { return IsValid(); }

    bool IsAlive() const {
        int hp = GetHealth();
        int lifeState = Memory::Read<int>(Address + Offsets::C_BaseEntity::m_lifeState);
        return hp > 0 && lifeState == 0;
    }

    int GetHealth() const { return Memory::Read<int>(Address + Offsets::C_BaseEntity::m_iHealth); }
    int GetTeam()   const { return Memory::Read<int>(Address + Offsets::C_BaseEntity::m_iTeamNum); }
    int GetFlags()  const { return Memory::Read<int>(Address + Offsets::C_BaseEntity::m_fFlags); }

    Vec3 GetOrigin() const {
        uintptr_t sceneNode = Memory::Read<uintptr_t>(Address + Offsets::C_BaseEntity::m_pGameSceneNode);
        if (!sceneNode) return {};
        return Memory::Read<Vec3>(sceneNode + 0x80);
    }

    Vec3 GetBonePos(int boneIndex) const {
        if (!Address) return {};
        uintptr_t sceneNode = Memory::Read<uintptr_t>(Address + Offsets::C_BaseEntity::m_pGameSceneNode);
        if (!sceneNode) return {};
        uintptr_t skeleton = Memory::Read<uintptr_t>(sceneNode + Offsets::CSkeletonInstance::m_modelState);
        if (!skeleton) return {};
        uintptr_t boneArray = Memory::Read<uintptr_t>(skeleton + Offsets::BoneArrayOffset);
        if (!boneArray) return {};
        return Memory::Read<Vec3>(boneArray + boneIndex * 0x20);
    }

    Vec3 GetViewAngles() const {
        return Memory::Read<Vec3>(Address + Offsets::C_CSPlayerPawn::m_angEyeAngles);
    }

    float GetFlashDuration() const {
        return Memory::Read<float>(Address + Offsets::C_CSPlayerPawnBase::m_flFlashDuration);
    }

    int GetShotsFired() const {
        return Memory::Read<int>(Address + Offsets::C_CSPlayerPawn::m_iShotsFired);
    }

    Vec3 GetAimPunchAngle() const {
        uintptr_t svc = Memory::Read<uintptr_t>(Address + Offsets::C_CSPlayerPawn::m_pAimPunchServices);
        if (!svc) return {};
        return Memory::Read<Vec3>(svc + Offsets::AimPunchServices::m_predictableBaseAngle);
    }

    uintptr_t GetWeaponServices() const {
        return Memory::Read<uintptr_t>(Address + Offsets::C_BasePlayerPawn::m_pWeaponServices);
    }

    void GetClassName(char* out, int maxLen) const {
        if (!Address || maxLen <= 0) { if (maxLen > 0) out[0] = 0; return; }
        out[0] = 0;
    }
};

inline uintptr_t GetLocalPlayerPawn(uintptr_t clientBase) {
    if (!clientBase) return 0;
    uintptr_t controller = Memory::Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerController);
    if (!controller) return 0;
    uint32_t pawnHandle = Memory::Read<uint32_t>(controller + Offsets::CCSPlayerController::m_hPlayerPawn);
    if (!pawnHandle || pawnHandle == 0xFFFFFFFF) return 0;

    uintptr_t entityList = Memory::Read<uintptr_t>(clientBase + Offsets::dwEntityList);
    if (!entityList) return 0;

    uint32_t idx = pawnHandle & 0x7FFF;
    uintptr_t bucket = Memory::Read<uintptr_t>(entityList + 0x10 + 0x8 * (idx >> 9));
    if (!bucket) return 0;
    return Memory::Read<uintptr_t>(bucket + 0x70 * (idx & 0x1FF));
}
