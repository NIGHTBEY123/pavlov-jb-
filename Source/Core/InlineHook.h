#pragma once
#include <cstdint>
#include <Windows.h>
#include <cstring>

// Simple x64 inline hook using 14-byte absolute JMP
// Much more reliable than vtable patching which CS2 can restore
namespace InlineHook {

    struct HookData {
        void* target;           // Original function address
        void* detour;           // Our hook function
        uint8_t stolen[14];     // Original bytes we overwrote
        uint8_t* trampoline;    // Trampoline to call original
        bool installed;
    };

    // Allocate trampoline near the target (within 2GB for relative calls)
    inline uint8_t* AllocateTrampoline(void* target, size_t size) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        
        uintptr_t addr = (uintptr_t)target;
        uintptr_t minAddr = addr > 0x70000000 ? addr - 0x70000000 : (uintptr_t)si.lpMinimumApplicationAddress;
        uintptr_t maxAddr = addr + 0x70000000;
        if (maxAddr > (uintptr_t)si.lpMaximumApplicationAddress)
            maxAddr = (uintptr_t)si.lpMaximumApplicationAddress;

        // Try to allocate nearby
        uintptr_t tryAddr = addr;
        for (uintptr_t a = tryAddr; a < maxAddr; a += si.dwAllocationGranularity) {
            void* p = VirtualAlloc((void*)a, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (p) return (uint8_t*)p;
        }
        // Fallback: anywhere
        return (uint8_t*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }

    // Install a 14-byte absolute JMP hook (x64)
    // mov rax, <address>; jmp rax
    inline bool Install(HookData& hd, void* target, void* detour) {
        hd.target = target;
        hd.detour = detour;
        hd.installed = false;

        // Allocate trampoline (stolen bytes + jmp back)
        hd.trampoline = AllocateTrampoline(target, 64);
        if (!hd.trampoline) return false;

        // Save original 14 bytes
        memcpy(hd.stolen, target, 14);

        // Build trampoline: original 14 bytes + jmp back to target+14
        memcpy(hd.trampoline, hd.stolen, 14);
        
        // JMP back: mov rax, target+14; jmp rax
        uintptr_t returnAddr = (uintptr_t)target + 14;
        hd.trampoline[14] = 0x48; // mov rax, imm64
        hd.trampoline[15] = 0xB8;
        memcpy(&hd.trampoline[16], &returnAddr, 8);
        hd.trampoline[24] = 0xFF; // jmp rax
        hd.trampoline[25] = 0xE0;

        // Now patch target: mov rax, detour; jmp rax
        DWORD oldProt;
        if (!VirtualProtect(target, 14, PAGE_EXECUTE_READWRITE, &oldProt))
            return false;

        uint8_t jmpCode[14];
        jmpCode[0] = 0x48; // mov rax, imm64
        jmpCode[1] = 0xB8;
        uintptr_t detourAddr = (uintptr_t)detour;
        memcpy(&jmpCode[2], &detourAddr, 8);
        jmpCode[10] = 0xFF; // jmp rax
        jmpCode[11] = 0xE0;
        jmpCode[12] = 0x90; // nop padding
        jmpCode[13] = 0x90;

        memcpy(target, jmpCode, 14);
        VirtualProtect(target, 14, oldProt, &oldProt);

        hd.installed = true;
        return true;
    }

    // Remove hook - restore original bytes
    inline void Remove(HookData& hd) {
        if (!hd.installed || !hd.target) return;
        
        DWORD oldProt;
        if (VirtualProtect(hd.target, 14, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy(hd.target, hd.stolen, 14);
            VirtualProtect(hd.target, 14, oldProt, &oldProt);
        }
        
        if (hd.trampoline) {
            VirtualFree(hd.trampoline, 0, MEM_RELEASE);
            hd.trampoline = nullptr;
        }
        hd.installed = false;
    }
}
