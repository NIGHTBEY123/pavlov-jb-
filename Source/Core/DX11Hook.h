#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "..\imgui\imgui.h"
#include "..\imgui\imgui_impl_dx11.h"
#include "..\imgui\imgui_impl_win32.h"
#include "..\include\MinHook.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DX11Hook {
    inline bool menuVisible = false;
    inline HWND gameWindow = nullptr;
    inline ID3D11Device* d3dDevice = nullptr;
    inline ID3D11DeviceContext* d3dContext = nullptr;
    inline IDXGISwapChain* swapChain = nullptr;
    inline ID3D11RenderTargetView* mainRT = nullptr;
    inline WNDPROC originalWndProc = nullptr;

    using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
    using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    inline PresentFn originalPresent = nullptr;
    inline ResizeBuffersFn originalResizeBuffers = nullptr;

    inline void RenderCallback();

    inline HRESULT __stdcall HookedPresent(IDXGISwapChain* pSwapChain, UINT syncInterval, UINT flags) {
        if (!d3dDevice) {
            swapChain = pSwapChain;
            pSwapChain->GetDevice(__uuidof(d3dDevice), (void**)&d3dDevice);
            d3dDevice->GetImmediateContext(&d3dContext);

            ID3D11Texture2D* backBuffer = nullptr;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (backBuffer) {
                d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &mainRT);
                backBuffer->Release();
            }

            DXGI_SWAP_CHAIN_DESC desc = {};
            pSwapChain->GetDesc(&desc);
            gameWindow = desc.OutputWindow;

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;

            ImGui_ImplWin32_Init(gameWindow);
            ImGui_ImplDX11_Init(d3dDevice, d3dContext);

            originalWndProc = (WNDPROC)SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, (LONG_PTR)[](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
                if (menuVisible && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                    return 1;
                return CallWindowProcA(originalWndProc, hWnd, msg, wParam, lParam);
            });
        }

        if (d3dDevice && d3dContext && mainRT) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RenderCallback();

            ImGui::Render();
            d3dContext->OMSetRenderTargets(1, &mainRT, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        return originalPresent(pSwapChain, syncInterval, flags);
    }

    inline HRESULT __stdcall HookedResizeBuffers(IDXGISwapChain* pSwapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT flags) {
        if (mainRT) { mainRT->Release(); mainRT = nullptr; }
        ImGui_ImplDX11_InvalidateDeviceObjects();

        HRESULT hr = originalResizeBuffers(pSwapChain, bufferCount, width, height, newFormat, flags);

        ID3D11Texture2D* backBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (backBuffer) {
            d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &mainRT);
            backBuffer->Release();
        }
        ImGui_ImplDX11_CreateDeviceObjects();
        return hr;
    }

    inline bool Hook() {
        HMODULE dxgiModule = GetModuleHandleA("dxgi.dll");
        if (!dxgiModule) return false;

        uintptr_t presentAddr = 0;
        uintptr_t resizeAddr = 0;

        IDXGIFactory* pFactory = nullptr;
        CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
        if (!pFactory) return false;

        IDXGIAdapter* pAdapter = nullptr;
        pFactory->EnumAdapters(0, &pAdapter);
        if (!pAdapter) { pFactory->Release(); return false; }

        IDXGIOutput* pOutput = nullptr;
        pAdapter->EnumOutputs(0, &pOutput);
        if (!pOutput) { pAdapter->Release(); pFactory->Release(); return false; }

        UINT numModes = 0;
        pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);

        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.Width = 1;
        scd.BufferDesc.Height = 1;
        scd.BufferCount = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = GetDesktopWindow();
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain* pSwapChain = nullptr;
        ID3D11Device* pDev = nullptr;
        ID3D11DeviceContext* pCtx = nullptr;
        D3D_FEATURE_LEVEL fl;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &pSwapChain, &pDev, &fl, &pCtx);
        if (FAILED(hr)) {
            hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &pSwapChain, &pDev, &fl, &pCtx);
        }
        if (FAILED(hr) || !pSwapChain) {
            pOutput->Release(); pAdapter->Release(); pFactory->Release();
            return false;
        }

        void** vtable = *(void***)pSwapChain;

        if (MH_Initialize() != MH_OK) {
            pSwapChain->Release(); pDev->Release(); pCtx->Release();
            pOutput->Release(); pAdapter->Release(); pFactory->Release();
            return false;
        }

        MH_CreateHook(vtable[8], HookedPresent, (void**)&originalPresent);
        MH_CreateHook(vtable[13], HookedResizeBuffers, (void**)&originalResizeBuffers);
        MH_EnableHook(vtable[8]);
        MH_EnableHook(vtable[13]);

        pSwapChain->Release(); pDev->Release(); pCtx->Release();
        pOutput->Release(); pAdapter->Release(); pFactory->Release();

        return true;
    }

    inline void Unhook() {
        if (originalWndProc && gameWindow) {
            SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
        }
        if (mainRT) { mainRT->Release(); mainRT = nullptr; }
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
}
