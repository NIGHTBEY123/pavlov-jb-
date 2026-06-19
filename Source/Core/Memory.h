#pragma once
#include <windows.h>
#include <vector>
#include <cstring>

namespace Memory {
    // Safe read - returns default value on access violation
    template <typename T>
    __declspec(noinline) T Read(uintptr_t address) {
        if (address < 0x1000 || address >= 0x7FFFFFFFFFFF) return T{};
        T result{};
        __try {
            if (IsBadReadPtr((const void*)address, sizeof(T))) return T{};
            memcpy(&result, (const void*)address, sizeof(T));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            memset(&result, 0, sizeof(T));
        }
        return result;
    }

    __declspec(noinline) inline void ReadString(uintptr_t address, char* buffer, size_t maxLength) {
        if (!buffer || maxLength == 0) return;
        buffer[0] = '\0';
        if (address < 0x1000 || address >= 0x7FFFFFFFFFFF) return;
        __try {
            if (IsBadReadPtr((const void*)address, maxLength - 1)) return;
            memcpy(buffer, (const void*)address, maxLength - 1);
            buffer[maxLength - 1] = '\0';
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            buffer[0] = '\0';
        }
    }

    template <typename T>
    __declspec(noinline) void Write(uintptr_t address, T value) {
        if (address < 0x1000 || address >= 0x7FFFFFFFFFFF) return;
        __try {
            // First try direct write
            memcpy((void*)address, &value, sizeof(T));
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // If it fails, try with VirtualProtect
            __try {
                DWORD oldProtect;
                if (VirtualProtect((void*)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    memcpy((void*)address, &value, sizeof(T));
                    VirtualProtect((void*)address, sizeof(T), oldProtect, &oldProtect);
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    inline uintptr_t GetModuleBase(const wchar_t* moduleName) {
        return (uintptr_t)GetModuleHandleW(moduleName);
    }

    inline uintptr_t PatternScan(HMODULE mod, const char* pattern) {
        if (!mod) return 0;
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)mod;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return 0;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)mod + dosHeader->e_lfanew);
        
        uint8_t* base = (uint8_t*)mod;
        size_t size = ntHeaders->OptionalHeader.SizeOfImage;
        
        uint8_t bytes[256] = {}; char mask[256] = {}; int len = 0;
        const char* p = pattern;
        while (*p) {
            while (*p == ' ') p++;
            if (!*p) break;
            if (*p == '?') { bytes[len] = 0; mask[len] = '?'; p++; if (*p == '?') p++; }
            else { bytes[len] = (uint8_t)strtoul(p, nullptr, 16); mask[len] = 'x'; p += 2; }
            len++;
        }
        for (size_t i = 0; i < size - len; i++) {
            bool found = true;
            for (int j = 0; j < len; j++) {
                if (mask[j] == 'x' && base[i + j] != bytes[j]) { found = false; break; }
            }
            if (found) return (uintptr_t)(base + i);
        }
        return 0;
    }

    inline uintptr_t ResolveRelativeAddress(uintptr_t instruction, uint32_t offsetOffset, uint32_t instructionSize) {
        if (!instruction) return 0;
        int32_t offset = Read<int32_t>(instruction + offsetOffset);
        if (!offset) return 0;
        return instruction + instructionSize + offset;
    }
}
