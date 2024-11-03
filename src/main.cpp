#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// DirectX variables
ID3D11Device* dev = nullptr;
ID3D11DeviceContext* devcon = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* backBufferRTV = nullptr;
ID3D11InputLayout* inputLayout = nullptr;
ID3D11VertexShader* vertexShader = nullptr;
ID3D11PixelShader* pixelShader = nullptr;
ID3D11Buffer* constantBuffer = nullptr;
ID3D11SamplerState* samplerState = nullptr;
ID3D11ShaderResourceView* textureView = nullptr;

// Screen resolution
const int screenWidth = 800;
const int screenHeight = 600;

// Color palette array
DirectX::XMFLOAT4 colorPalette[4] = {
    { 0.0f, 0.0f, 1.0f, 1.0f },   // Blue
    { 0.0f, 0.0f, 0.0f, 1.0f },   // Black
    { 1.0f, 0.5f, 0.0f, 1.0f },   // Orange
    { 0.8f, 0.2f, 1.0f, 1.0f }    // Purple
};

// Function declarations
void InitD3D(HWND hWnd);
void RenderFrame();
void CleanupD3D();
void InitPipeline();
void InitGraphics();
void UpdateColorPaletteUI();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "PixelBGGenerator", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, "Automated Pixel Background Generator", WS_OVERLAPPEDWINDOW, 100, 100, screenWidth, screenHeight, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Initialize Direct3D
    InitD3D(hwnd);

    // Main loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            RenderFrame();
        }
    }

    // Clean up DirectX
    CleanupD3D();

    return (int)msg.wParam;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Initialize DirectX
void InitD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = screenWidth;
    scd.BufferDesc.Height = screenHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &scd, &swapChain, &dev, NULL, &devcon);

    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backBufferRTV);
    pBackBuffer->Release();
    devcon->OMSetRenderTargets(1, &backBufferRTV, NULL);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (FLOAT)screenWidth;
    viewport.Height = (FLOAT)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    devcon->RSSetViewports(1, &viewport);

    InitPipeline();
    InitGraphics();
}

// Initialize shaders and pipeline
void InitPipeline() {
    ID3DBlob* VS, * PS;

    D3DReadFileToBlob(L"VertexShader.cso", &VS);
    D3DReadFileToBlob(L"PixelShader.cso", &PS);

    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &vertexShader);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pixelShader);

    devcon->VSSetShader(vertexShader, 0, 0);
    devcon->PSSetShader(pixelShader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    dev->CreateInputLayout(layout, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &inputLayout);
    devcon->IASetInputLayout(inputLayout);

    VS->Release();
    PS->Release();
}

// Load and initialize the graphics (reference image, buffers)
void InitGraphics() {
    // Load reference image texture
    D3DX11CreateShaderResourceViewFromFile(dev, "textures/reference_image.png", NULL, NULL, &textureView, NULL);
    devcon->PSSetShaderResources(0, 1, &textureView);
}

// Update color palette using ImGui
void UpdateColorPaletteUI() {
    ImGui::Begin("Color Palette");
    for (int i = 0; i < 4; i++) {
        ImGui::ColorEdit4("Color", (float*)&colorPalette[i]);
    }
    ImGui::End();
}

// Render a frame
void RenderFrame() {
    float bgColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    devcon->ClearRenderTargetView(backBufferRTV, bgColor);

    // Update color palette
    devcon->UpdateSubresource(constantBuffer, 0, NULL, &colorPalette, 0, 0);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UpdateColorPaletteUI();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    swapChain->Present(0, 0);
}

// Clean up DirectX
void CleanupD3D() {
    swapChain->Release();
    backBufferRTV->Release();
    dev->Release();
    devcon->Release();
    inputLayout->Release();
    vertexShader->Release();
    pixelShader->Release();
    textureView->Release();
}
