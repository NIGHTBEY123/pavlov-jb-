#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "..\SDK\Offsets.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace DrawIndexedHook {

    inline void Log(const char* fmt, ...) {
        char buf[512]; va_list a; va_start(a, fmt);
        vsnprintf(buf, sizeof(buf), fmt, a); va_end(a);
        HANDLE h = CreateFileA("C:\\\\\\drawlog.txt",
            FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) { DWORD w; WriteFile(h, buf, (DWORD)strlen(buf), &w, NULL); CloseHandle(h); }
    }

    inline bool hooked = false;

    using DrawIndexedFn = void(__stdcall*)(ID3D11DeviceContext*, UINT, UINT, INT);
    using DrawIndexedInstancedFn = void(__stdcall*)(ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT);
    inline DrawIndexedFn oDrawIndexed = nullptr;
    inline DrawIndexedInstancedFn oDrawIndexedInstanced = nullptr;

    // GPU resources
    inline ID3D11Device* pDevice = nullptr;
    inline ID3D11PixelShader* pFlatPS = nullptr;
    inline ID3D11PixelShader* pMetallicPS = nullptr;
    inline ID3D11PixelShader* pGlowPS = nullptr;
    inline ID3D11Buffer* pColorBuffer = nullptr;
    inline ID3D11DepthStencilState* pDepthDisabled = nullptr;
    inline ID3D11DepthStencilState* pDepthEnabled = nullptr;
    inline ID3D11BlendState* pAdditiveBlend = nullptr;
    inline ID3D11BlendState* pAlphaBlend = nullptr;
    inline bool resourcesCreated = false;

    // Settings (exposed to GUI)
    inline bool chamsEnabled = false;
    inline bool chamsThroughWalls = true;
    inline bool chamsTeamShow = false;
    inline bool chamsHideLocal = true;
    inline bool isImGuiRendering = false;
    inline int   chamsMode = 0; // 0=Flat, 1=Glow, 2=Metallic
    inline float chamsAlpha = 1.0f;
    inline float chamsEnemyVisible[4]  = {0.0f, 1.0f, 0.4f, 1.0f};
    inline float chamsEnemyHidden[4]   = {1.0f, 0.2f, 0.2f, 1.0f};
    inline float chamsFriendVisible[4] = {0.2f, 0.4f, 1.0f, 1.0f};
    inline float chamsFriendHidden[4]  = {0.8f, 0.8f, 0.0f, 1.0f};

    // Stride logging (first 300 frames only)
    inline bool strideLogDone = false;
    inline int  frameCount = 0;
    
    inline uintptr_t* pContextVtable = nullptr;

    struct ColorCB { float color[4]; };

    // ===== STRIDE DETECTION =====
    // Widened to catch all possible CS2 player model formats
    inline bool IsPlayerModel(UINT indexCount, UINT stride) {
        // CS2 uses various strides depending on shader/material
        // Accept any stride >= 32 with enough triangles for a player model
        if (stride >= 32 && stride <= 112) {
            return (indexCount >= 3000 && indexCount <= 100000);
        }
        return false;
    }

    inline UINT GetCurrentStride(ID3D11DeviceContext* ctx) {
        ID3D11Buffer* vb = nullptr;
        UINT stride = 0, offset = 0;
        ctx->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
        if (vb) vb->Release();
        return stride;
    }

    inline void SetColor(ID3D11DeviceContext* ctx, float r, float g, float b, float a) {
        if (!pColorBuffer) return;
        D3D11_MAPPED_SUBRESOURCE ms;
        if(SUCCEEDED(ctx->Map(pColorBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&ms))){
            float*c=(float*)ms.pData;c[0]=r;c[1]=g;c[2]=b;c[3]=a;
            ctx->Unmap(pColorBuffer,0);
        }
        ctx->PSSetConstantBuffers(0,1,&pColorBuffer);
    }

    // ===== CREATE GPU RESOURCES =====
    inline bool CreateResources(ID3D11DeviceContext* ctx) {
        if (resourcesCreated) return true;
        ctx->GetDevice(&pDevice);
        if (!pDevice) return false;
        
        // --- Flat PS ---
        const char* flatSrc = "cbuffer CB:register(b0){float4 c;};float4 main():SV_Target{return c;}";
        ID3DBlob* b=nullptr,*e=nullptr;
        if(FAILED(D3DCompile(flatSrc,strlen(flatSrc),0,0,0,"main","ps_5_0",0,0,&b,&e))){if(e)e->Release();return false;}
        pDevice->CreatePixelShader(b->GetBufferPointer(),b->GetBufferSize(),0,&pFlatPS);
        b->Release();

        // --- Glow PS (additive bright) ---
        const char* glowSrc = 
            "cbuffer CB:register(b0){float4 c;};\n"
            "float4 main():SV_Target{ return float4(c.rgb * 2.5, c.a); }";
        {
            ID3DBlob* gb=nullptr,*ge=nullptr;
            if(SUCCEEDED(D3DCompile(glowSrc,strlen(glowSrc),0,0,0,"main","ps_5_0",0,0,&gb,&ge))){
                pDevice->CreatePixelShader(gb->GetBufferPointer(),gb->GetBufferSize(),0,&pGlowPS);
                gb->Release();
            } else { if(ge)ge->Release(); }
        }

        // --- Metallic PS (fake reflection) ---
        const char* metalSrc =
            "cbuffer CB:register(b0){float4 c;};\n"
            "float4 main(float4 pos:SV_Position):SV_Target{\n"
            "  float2 uv = pos.xy / float2(1920.0, 1080.0);\n"
            "  float shimmer = sin(uv.x * 50.0 + uv.y * 30.0) * 0.15 + 0.85;\n"
            "  float highlight = pow(saturate(1.0 - abs(uv.y - 0.5) * 2.0), 3.0) * 0.5;\n"
            "  return float4(c.rgb * shimmer + highlight, c.a);\n"
            "}";
        {
            ID3DBlob* mb=nullptr,*me=nullptr;
            if(SUCCEEDED(D3DCompile(metalSrc,strlen(metalSrc),0,0,0,"main","ps_5_0",0,0,&mb,&me))){
                pDevice->CreatePixelShader(mb->GetBufferPointer(),mb->GetBufferSize(),0,&pMetallicPS);
                mb->Release();
            } else { if(me)me->Release(); }
        }

        // Color CB
        D3D11_BUFFER_DESC cbd={};cbd.ByteWidth=16;cbd.Usage=D3D11_USAGE_DYNAMIC;
        cbd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;cbd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(&cbd,0,&pColorBuffer);

        // Depth disabled
        D3D11_DEPTH_STENCIL_DESC dsd={};
        dsd.DepthEnable=FALSE;dsd.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ZERO;
        pDevice->CreateDepthStencilState(&dsd,&pDepthDisabled);

        // Depth enabled
        D3D11_DEPTH_STENCIL_DESC dse={};
        dse.DepthEnable=TRUE;dse.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;
        dse.DepthFunc=D3D11_COMPARISON_LESS_EQUAL;
        pDevice->CreateDepthStencilState(&dse,&pDepthEnabled);

        // Additive blend
        D3D11_BLEND_DESC addBlend={};
        addBlend.RenderTarget[0].BlendEnable=TRUE;
        addBlend.RenderTarget[0].SrcBlend=D3D11_BLEND_ONE;
        addBlend.RenderTarget[0].DestBlend=D3D11_BLEND_ONE;
        addBlend.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD;
        addBlend.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
        addBlend.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
        addBlend.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
        addBlend.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
        pDevice->CreateBlendState(&addBlend,&pAdditiveBlend);

        // Alpha blend
        D3D11_BLEND_DESC alphaBlend={};
        alphaBlend.RenderTarget[0].BlendEnable=TRUE;
        alphaBlend.RenderTarget[0].SrcBlend=D3D11_BLEND_SRC_ALPHA;
        alphaBlend.RenderTarget[0].DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
        alphaBlend.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD;
        alphaBlend.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
        alphaBlend.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
        alphaBlend.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
        alphaBlend.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
        pDevice->CreateBlendState(&alphaBlend,&pAlphaBlend);

        resourcesCreated = true;
        Log("GPU Chams resources created OK\r\n");
        return true;
    }

    // ===== DrawIndexed HOOK =====
    inline void __stdcall hkDrawIndexed(ID3D11DeviceContext* ctx,
                                         UINT IndexCount, UINT StartIdx, INT BaseVtx) {
        if (isImGuiRendering || !resourcesCreated) {
            oDrawIndexed(ctx, IndexCount, StartIdx, BaseVtx);
            return;
        }

        // --- STRIDE LOGGING (first 500 frames, ALL significant draws) ---
        if (!strideLogDone) {
            frameCount++;
            if (IndexCount >= 1000) {
                UINT stride = GetCurrentStride(ctx);
                Log("[DI] s=%u i=%u\r\n", stride, IndexCount);
            }
            if (frameCount > 500) { strideLogDone = true; Log("--- Log done ---\r\n"); }
        }

        if (!chamsEnabled) {
            oDrawIndexed(ctx, IndexCount, StartIdx, BaseVtx);
            return;
        }

        UINT stride = GetCurrentStride(ctx);
        
        if (IsPlayerModel(IndexCount, stride)) {
            // Save state
            ID3D11PixelShader* origPS = nullptr;
            ID3D11DepthStencilState* origDSS = nullptr;
            UINT origRef = 0;
            ID3D11BlendState* origBS = nullptr;
            float origBF[4]={}; UINT origMask=0;
            
            ctx->PSGetShader(&origPS, nullptr, nullptr);
            ctx->OMGetDepthStencilState(&origDSS, &origRef);
            ctx->OMGetBlendState(&origBS, origBF, &origMask);

            // Select PS
            ID3D11PixelShader* ps = pFlatPS;
            if (chamsMode == 1 && pGlowPS) ps = pGlowPS;
            if (chamsMode == 2 && pMetallicPS) ps = pMetallicPS;

            // PASS 1: Hidden (through wall)
            if (chamsThroughWalls) {
                ctx->OMSetDepthStencilState(pDepthDisabled, 0);
                ctx->PSSetShader(ps, nullptr, 0);
                if (chamsMode == 1) {
                    float bf[4]={1,1,1,1};
                    ctx->OMSetBlendState(pAdditiveBlend, bf, 0xFFFFFFFF);
                }
                SetColor(ctx, chamsEnemyHidden[0], chamsEnemyHidden[1],
                         chamsEnemyHidden[2], chamsEnemyHidden[3] * chamsAlpha);
                oDrawIndexed(ctx, IndexCount, StartIdx, BaseVtx);
            }

            // PASS 2: Visible
            ctx->OMSetDepthStencilState(pDepthEnabled, 0);
            ctx->PSSetShader(ps, nullptr, 0);
            if (chamsMode == 1) {
                float bf[4]={1,1,1,1};
                ctx->OMSetBlendState(pAdditiveBlend, bf, 0xFFFFFFFF);
            } else {
                float bf[4]={1,1,1,1};
                ctx->OMSetBlendState(pAlphaBlend, bf, 0xFFFFFFFF);
            }
            SetColor(ctx, chamsEnemyVisible[0], chamsEnemyVisible[1],
                     chamsEnemyVisible[2], chamsEnemyVisible[3] * chamsAlpha);
            oDrawIndexed(ctx, IndexCount, StartIdx, BaseVtx);

            // Restore
            ctx->PSSetShader(origPS, nullptr, 0);
            ctx->OMSetDepthStencilState(origDSS, origRef);
            ctx->OMSetBlendState(origBS, origBF, origMask);
            if (origPS) origPS->Release();
            if (origDSS) origDSS->Release();
            if (origBS) origBS->Release();
            return;
        }

        oDrawIndexed(ctx, IndexCount, StartIdx, BaseVtx);
    }

    // ===== DrawIndexedInstanced HOOK =====
    inline void __stdcall hkDrawIndexedInstanced(ID3D11DeviceContext* ctx,
            UINT IdxPerInst, UINT InstCount, UINT StartIdx, INT BaseVtx, UINT StartInst) {
        
        if (isImGuiRendering || !resourcesCreated) {
            oDrawIndexedInstanced(ctx, IdxPerInst, InstCount, StartIdx, BaseVtx, StartInst);
            return;
        }

        // --- STRIDE LOGGING (first 500 frames, ALL significant draws) ---
        // This is the MAIN draw call CS2 uses (not DrawIndexed)
        if (!strideLogDone) {
            frameCount++;
            if (IdxPerInst >= 1000) {
                UINT stride = GetCurrentStride(ctx);
                Log("[DII] s=%u i=%u inst=%u\r\n", stride, IdxPerInst, InstCount);
            }
            if (frameCount > 500) { strideLogDone = true; Log("--- Log done ---\r\n"); }
        }

        if (!chamsEnabled) {
            oDrawIndexedInstanced(ctx, IdxPerInst, InstCount, StartIdx, BaseVtx, StartInst);
            return;
        }

        UINT stride = GetCurrentStride(ctx);
        
        if (IsPlayerModel(IdxPerInst, stride)) {
            ID3D11PixelShader* origPS = nullptr;
            ID3D11DepthStencilState* origDSS = nullptr;
            UINT origRef = 0;
            ID3D11BlendState* origBS = nullptr;
            float origBF[4]={}; UINT origMask=0;
            
            ctx->PSGetShader(&origPS, nullptr, nullptr);
            ctx->OMGetDepthStencilState(&origDSS, &origRef);
            ctx->OMGetBlendState(&origBS, origBF, &origMask);

            ID3D11PixelShader* ps = pFlatPS;
            if (chamsMode == 1 && pGlowPS) ps = pGlowPS;
            if (chamsMode == 2 && pMetallicPS) ps = pMetallicPS;

            if (chamsThroughWalls) {
                ctx->OMSetDepthStencilState(pDepthDisabled, 0);
                ctx->PSSetShader(ps, nullptr, 0);
                SetColor(ctx, chamsEnemyHidden[0], chamsEnemyHidden[1],
                         chamsEnemyHidden[2], chamsEnemyHidden[3] * chamsAlpha);
                oDrawIndexedInstanced(ctx, IdxPerInst, InstCount, StartIdx, BaseVtx, StartInst);
            }

            ctx->OMSetDepthStencilState(pDepthEnabled, 0);
            ctx->PSSetShader(ps, nullptr, 0);
            SetColor(ctx, chamsEnemyVisible[0], chamsEnemyVisible[1],
                     chamsEnemyVisible[2], chamsEnemyVisible[3] * chamsAlpha);
            oDrawIndexedInstanced(ctx, IdxPerInst, InstCount, StartIdx, BaseVtx, StartInst);

            ctx->PSSetShader(origPS, nullptr, 0);
            ctx->OMSetDepthStencilState(origDSS, origRef);
            ctx->OMSetBlendState(origBS, origBF, origMask);
            if (origPS) origPS->Release();
            if (origDSS) origDSS->Release();
            if (origBS) origBS->Release();
            return;
        }

        oDrawIndexedInstanced(ctx, IdxPerInst, InstCount, StartIdx, BaseVtx, StartInst);
    }

    // ===== INIT =====
    inline bool Init(ID3D11DeviceContext* context) {
        if (!context) return false;
        DeleteFileA("C:\\\\\\drawlog.txt");
        Log("=== DrawIndexedHook Init v2 ===\r\n");

        pContextVtable = *(uintptr_t**)context;
        if (!pContextVtable) { Log("VTable NULL!\r\n"); return false; }

        if (!CreateResources(context)) { Log("CreateResources FAILED\r\n"); return false; }

        oDrawIndexed = (DrawIndexedFn)pContextVtable[12];
        oDrawIndexedInstanced = (DrawIndexedInstancedFn)pContextVtable[20];
        
        Log("DI[12] = %p, DII[20] = %p\r\n", (void*)oDrawIndexed, (void*)oDrawIndexedInstanced);

        DWORD oldProt;
        if (VirtualProtect(&pContextVtable[12], sizeof(uintptr_t)*9, PAGE_EXECUTE_READWRITE, &oldProt)) {
            pContextVtable[12] = (uintptr_t)&hkDrawIndexed;
            pContextVtable[20] = (uintptr_t)&hkDrawIndexedInstanced;
            VirtualProtect(&pContextVtable[12], sizeof(uintptr_t)*9, oldProt, &oldProt);
            hooked = true;
            Log("VTable HOOKED!\r\n");
        } else {
            Log("VirtualProtect FAILED!\r\n");
            return false;
        }

        return true;
    }

    inline void Cleanup() {
        if(!hooked) return;
        if (pContextVtable && oDrawIndexed && oDrawIndexedInstanced) {
            DWORD oldProt;
            if (VirtualProtect(&pContextVtable[12], sizeof(uintptr_t)*9, PAGE_EXECUTE_READWRITE, &oldProt)) {
                pContextVtable[12] = (uintptr_t)oDrawIndexed;
                pContextVtable[20] = (uintptr_t)oDrawIndexedInstanced;
                VirtualProtect(&pContextVtable[12], sizeof(uintptr_t)*9, oldProt, &oldProt);
            }
        }
        if(pFlatPS) pFlatPS->Release();
        if(pGlowPS) pGlowPS->Release();
        if(pMetallicPS) pMetallicPS->Release();
        if(pColorBuffer) pColorBuffer->Release();
        if(pDepthDisabled) pDepthDisabled->Release();
        if(pDepthEnabled) pDepthEnabled->Release();
        if(pAdditiveBlend) pAdditiveBlend->Release();
        if(pAlphaBlend) pAlphaBlend->Release();
        if(pDevice) pDevice->Release();
        hooked=false;resourcesCreated=false;
    }
}
