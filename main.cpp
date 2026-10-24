/* Authors:
* Course:
* Date:
* 
* Project Name: CubeBeatz
*/

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <sal.h>

#pragma comment(lib, "d3d11.lib")  
#pragma comment(lib, "dxgi.lib")    

const wchar_t CLASS_NAME[] = L"CubeBeatzWindowClass";

ID3D11Device* device = nullptr;
ID3D11DeviceContext* deviceContext = nullptr;
IDXGISwapChain* swapChain = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

bool InitializeDirect3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd; 
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
    
    if (FAILED(result))
        return false;

    ID3D11Texture2D* backBuffer = nullptr; 

    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

    if (FAILED(result)) return false; 

    result = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

    backBuffer->Release();

    if (FAILED(result)) return false; 

    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return true; 
}

//A Dark background color: 
void RenderFrame() {
    float backgroundColor[] = {
        0.04F, 0.04f, 0.08f, 1.0f
    };

    deviceContext->ClearRenderTargetView(renderTargetView, backgroundColor);

    swapChain->Present(1, 0);
}

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
        case WM_DESTROY:{
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if(!RegisterClass(&wc)) {
        return 1;
	}

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"CubeBeatz",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        1280, 720, 
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr) {
        return 0;
    }

    if (!InitializeDirect3D(hwnd)) {
        MessageBox(hwnd, L"Failed to initialize Direct3D", L"CubeBeatz Error", MB_OK | MB_ICONERROR);
		
		if (renderTargetView) renderTargetView->Release();
        if (swapChain) swapChain->Release();
        if (deviceContext) deviceContext->Release();
        if (device) device->Release();

		DestroyWindow(hwnd);
		return 1;

    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
			RenderFrame();
        }
    }

	if (renderTargetView) renderTargetView->Release();
	if (swapChain) swapChain->Release();
	if (deviceContext) deviceContext->Release();
	if (device) device->Release();

    return 0;
}
