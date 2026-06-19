#pragma once
#include "..\imgui\imgui.h"
#include <cmath>
#include <cstdlib>
#include <chrono>

// ========================================================================
//  HIT EFFECTS — Lightning + Damage Numbers (ImGui Overlay)
//  No game engine interaction - purely visual, zero crash risk
// ========================================================================
namespace HitEffects {

    inline bool lightningEnabled = true;
    inline bool damageNumbersEnabled = true;
    inline float lightningDuration = 0.35f; // seconds
    inline float damageDuration = 1.2f; // seconds

    // Lightning bolt storage
    struct LightningBolt {
        float x, y;         // Screen position
        float time;          // When it was created
        bool active;
        float segments[16];  // Random horizontal offsets for zigzag
        int numSegments;
        float height;        // Bolt height in pixels
    };

    // Damage number storage
    struct DamageNumber {
        float x, y;          // Screen position
        int damage;
        float time;           // When it was created
        bool active;
        bool isHeadshot;
        float velX, velY;     // Drift velocity
    };

    inline LightningBolt bolts[16] = {};
    inline DamageNumber numbers[32] = {};
    inline int boltIdx = 0;
    inline int numIdx = 0;

    inline float GetTime() {
        static auto start = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(now - start).count();
    }

    // Call when a hit is detected
    inline void OnHit(float screenX, float screenY, int damage, bool headshot) {
        float now = GetTime();

        // Add lightning bolt
        if (lightningEnabled) {
            LightningBolt& b = bolts[boltIdx % 16];
            b.x = screenX;
            b.y = screenY;
            b.time = now;
            b.active = true;
            b.height = 80.0f + (rand() % 60);
            b.numSegments = 8 + (rand() % 6);
            for (int i = 0; i < b.numSegments; i++) {
                b.segments[i] = (float)(rand() % 30 - 15); // -15 to +15 horizontal offset
            }
            boltIdx++;
        }

        // Add damage number
        if (damageNumbersEnabled) {
            DamageNumber& d = numbers[numIdx % 32];
            d.x = screenX + (rand() % 20 - 10);
            d.y = screenY - 10;
            d.damage = damage;
            d.time = now;
            d.active = true;
            d.isHeadshot = headshot;
            d.velX = (float)(rand() % 20 - 10) * 0.5f;
            d.velY = -40.0f - (float)(rand() % 20); // Float upward
            numIdx++;
        }
    }

    // Render all active effects
    inline void Render() {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;
        float now = GetTime();

        // ====== LIGHTNING BOLTS ======
        for (int i = 0; i < 16; i++) {
            LightningBolt& b = bolts[i];
            if (!b.active) continue;
            float elapsed = now - b.time;
            if (elapsed > lightningDuration) { b.active = false; continue; }

            float alpha = 1.0f - (elapsed / lightningDuration);
            alpha = alpha * alpha; // Ease out
            int a = (int)(alpha * 255);
            int aGlow = (int)(alpha * 100);

            // Core bolt color: white-blue
            ImU32 coreCol = IM_COL32(200, 220, 255, a);
            ImU32 glowCol = IM_COL32(100, 150, 255, aGlow);

            float segH = b.height / b.numSegments;
            float prevX = b.x;
            float prevY = b.y - b.height; // Start from top

            // Draw glow first (thicker, translucent)
            for (int s = 0; s < b.numSegments; s++) {
                float nextX = b.x + b.segments[s] * (1.0f + elapsed * 2.0f); // Expand over time
                float nextY = prevY + segH;
                dl->AddLine(ImVec2(prevX, prevY), ImVec2(nextX, nextY), glowCol, 6.0f);
                prevX = nextX;
                prevY = nextY;
            }

            // Draw core (thin, bright)
            prevX = b.x;
            prevY = b.y - b.height;
            for (int s = 0; s < b.numSegments; s++) {
                float nextX = b.x + b.segments[s] * (1.0f + elapsed * 2.0f);
                float nextY = prevY + segH;
                dl->AddLine(ImVec2(prevX, prevY), ImVec2(nextX, nextY), coreCol, 2.0f);
                prevX = nextX;
                prevY = nextY;
            }

            // Small flash at hit point
            if (elapsed < 0.1f) {
                float flashR = 15.0f * (1.0f - elapsed / 0.1f);
                dl->AddCircleFilled(ImVec2(b.x, b.y), flashR, IM_COL32(255,255,255,(int)(alpha*180)));
                dl->AddCircleFilled(ImVec2(b.x, b.y), flashR*1.5f, IM_COL32(100,150,255,(int)(alpha*60)));
            }
        }

        // ====== DAMAGE NUMBERS ======
        for (int i = 0; i < 32; i++) {
            DamageNumber& d = numbers[i];
            if (!d.active) continue;
            float elapsed = now - d.time;
            if (elapsed > damageDuration) { d.active = false; continue; }

            float alpha = 1.0f - (elapsed / damageDuration);
            float scale = 1.0f + elapsed * 0.3f; // Slightly grow
            float posX = d.x + d.velX * elapsed;
            float posY = d.y + d.velY * elapsed + 20.0f * elapsed * elapsed; // Gravity

            char dmgBuf[8];
            sprintf_s(dmgBuf, "%d", d.damage);

            float fontSize = ImGui::GetFontSize();
            float drawSize = fontSize * (d.isHeadshot ? 1.4f : 1.0f) * scale;
            ImVec2 textSize = ImGui::CalcTextSize(dmgBuf);
            float tw = textSize.x * (drawSize / fontSize);

            int a = (int)(alpha * 255);

            // Shadow
            dl->AddText(nullptr, drawSize, ImVec2(posX - tw/2 + 1, posY + 1), IM_COL32(0,0,0,a/2), dmgBuf);

            // Main color: headshot = red+big, body = yellow
            ImU32 col;
            if (d.isHeadshot) {
                col = IM_COL32(255, 50, 50, a); // Red for headshot
            } else {
                col = IM_COL32(255, 200, 50, a); // Yellow for body
            }
            dl->AddText(nullptr, drawSize, ImVec2(posX - tw/2, posY), col, dmgBuf);

            // Headshot marker
            if (d.isHeadshot && elapsed < 0.3f) {
                float markAlpha = 1.0f - elapsed / 0.3f;
                dl->AddText(nullptr, drawSize * 0.6f, ImVec2(posX + tw/2 + 2, posY - 4),
                           IM_COL32(255,100,100,(int)(markAlpha*200)), "HS");
            }
        }
    }
}
