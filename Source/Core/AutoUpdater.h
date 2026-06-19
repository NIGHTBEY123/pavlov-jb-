#pragma once
#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#include <string>
#include "..\SDK\Offsets.h"

namespace AutoUpdater {

    inline std::string FetchURL(HINTERNET hConnect, const wchar_t* path) {
        std::string result;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
        if (!hRequest) return result;

        if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            if (WinHttpReceiveResponse(hRequest, NULL)) {
                char buf[4096];
                DWORD read = 0;
                while (WinHttpReadData(hRequest, buf, sizeof(buf), &read) && read > 0) {
                    result.append(buf, read);
                    read = 0;
                }
            }
        }
        WinHttpCloseHandle(hRequest);
        return result;
    }

    inline uintptr_t FindJSONVal(const std::string& data, const char* key) {
        std::string search = "\"";
        search += key;
        search += "\":";
        size_t pos = data.find(search);
        if (pos == std::string::npos) return 0;
        pos = data.find(':', pos);
        if (pos == std::string::npos) return 0;
        pos++;
        while (pos < data.size() && (data[pos] == ' ' || data[pos] == '\t')) pos++;
        if (pos >= data.size()) return 0;
        if (data[pos] == '"') pos++;
        std::string valStr;
        while (pos < data.size() && data[pos] != ',' && data[pos] != '}' && data[pos] != '"' && data[pos] != '\n' && data[pos] != '\r') {
            valStr += data[pos++];
        }
        if (valStr.find("0x") == 0 || valStr.find("0X") == 0)
            return strtoull(valStr.c_str(), nullptr, 16);
        return strtoull(valStr.c_str(), nullptr, 10);
    }

    inline void ParseOffsets(const std::string& offsetsData, const std::string& buttonsData, bool& ok) {
        if (offsetsData.empty()) return;

        Offsets::dwEntityList = FindJSONVal(offsetsData, "dwEntityList");
        Offsets::dwLocalPlayerController = FindJSONVal(offsetsData, "dwLocalPlayerController");
        Offsets::dwLocalPlayerPawn = FindJSONVal(offsetsData, "dwLocalPlayerPawn");
        Offsets::dwViewMatrix = FindJSONVal(offsetsData, "dwViewMatrix");
        Offsets::dwGlobalVars = FindJSONVal(offsetsData, "dwGlobalVars");
        Offsets::dwPlantedC4 = FindJSONVal(offsetsData, "dwPlantedC4");
        Offsets::dwSensitivity = FindJSONVal(offsetsData, "dwSensitivity");
        Offsets::dwSensitivity_sensitivity = FindJSONVal(offsetsData, "dwSensitivity_sensitivity");
        Offsets::dwViewAngles = FindJSONVal(offsetsData, "dwViewAngles");
        Offsets::dwNetworkGameClient = FindJSONVal(offsetsData, "dwNetworkGameClient");
        Offsets::dwNetworkGameClient_signOnState = FindJSONVal(offsetsData, "dwNetworkGameClient_signOnState");
        Offsets::dwNetworkGameClient_deltaTick = FindJSONVal(offsetsData, "dwNetworkGameClient_deltaTick");
        Offsets::dwBuildNumber = FindJSONVal(offsetsData, "dwBuildNumber");

        if (!buttonsData.empty()) {
            Offsets::dwAttack = FindJSONVal(buttonsData, "attack");
            Offsets::dwJump = FindJSONVal(buttonsData, "jump");
        }

        ok = (Offsets::dwEntityList != 0 && Offsets::dwLocalPlayerController != 0);
    }

    inline bool UpdateOffsets(HMODULE hModule) {
        std::string offsetsData, buttonsData;
        bool ok = false;

        HANDLE hFile = CreateFileA("C:\\dll_log.txt", FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        auto LogMsg = [&](const char* msg) {
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD written;
                WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
            }
        };

        LogMsg("[AU] Downloading offsets from a2x/cs2-dumper...\r\n");

        HINTERNET hSession = WinHttpOpen(L"CS2 Internal/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
        if (!hSession) { if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile); return false; }

        HINTERNET hConnect = WinHttpConnect(hSession, L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile); return false; }

        offsetsData = FetchURL(hConnect, L"/a2x/cs2-dumper/main/output/offsets.json");
        buttonsData = FetchURL(hConnect, L"/a2x/cs2-dumper/main/output/buttons.json");

        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        ParseOffsets(offsetsData, buttonsData, ok);

        if (ok) {
            char logBuf[512];
            sprintf_s(logBuf, "[AU] EntList: %llX, LocCtrl: %llX, ViewMat: %llX | OK\r\n",
                Offsets::dwEntityList, Offsets::dwLocalPlayerController, Offsets::dwViewMatrix);
            LogMsg(logBuf);
        } else {
            LogMsg("[AU] EntList: FAIL\r\n");
        }

        if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
        return ok;
    }
}
