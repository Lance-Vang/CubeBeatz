/* Authors:
* Course:
* Date:
* 
* Project Name: CubeBeatz
*/

#include <windows.h> //Windows function and Windows management
#include <d3d11.h> // Direct3D 11 graphics functionality
#include <dxgi.h> // graphics display and swap-chain functions
#include <sal.h>

// Links the Firect3D and DXGI Libraries 
#pragma comment(lib, "d3d11.lib")  
#pragma comment(lib, "dxgi.lib")    

// Identifies the application window
const wchar_t CLASS_NAME[] = L"CubeBeatzWindowClass"; 

ID3D11Device* device = nullptr; //Direct3D device used to create graphic resources
ID3D11DeviceContext* deviceContext = nullptr; // sends rendering commands to Direct3D
IDXGISwapChain* swapChain = nullptr; // Manages the buffers used to display frames
ID3D11RenderTargetView* renderTargetView = nullptr; // Identifies the buffer where graphics are rendered 

//Initializes Direct 3D and prepares the window for rendering 
bool InitializeDirect3D(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC scd = {}; //Describes the swap chain configuration
    scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //Specifies the pixel color format
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Indicates the buffer is used as a render target
    scd.OutputWindow = hwnd; 
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //selects the swap effect for presention behavior 

	//Create the graphics of Direct3D device, device context, and swap chain
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, nullptr, &deviceContext);
   
	// Stop initialization if creating the Direct3D objects failed
    if (FAILED(result)) {
        return false;
    }

    //Gets the swap chain's back buffer.
    ID3D11Texture2D* backBuffer = nullptr; 

    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

    // Stop if the back buffer could not be retrieved.
    if (FAILED(result)) { return false; }

    // Creates a render target view for the back buffer.
    result = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

    // Releases the temporary back-buffer reference.
    backBuffer->Release();

    // Stop if the render target could not be created.
    if (FAILED(result)) return false; 

    // Sets the rendr target used by the output-merger stage
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    // Reports that initialization succeeded
    return true; 
}

//A Dark background color: 
void RenderFrame() {
    //RGBA color dark navy 
    float backgroundColor[] = {
        0.04F, 0.04f, 0.08f, 1.0f
    };

    //renders target with background color 
    deviceContext->ClearRenderTargetView(renderTargetView, backgroundColor);
    //presnets the completed frame to the window
    swapChain->Present(1, 0);
}

//Handles Windows messages sent to the application window
LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    //checks which type of windows message was received
    switch (uMsg) {
        // handles the window being destroyed
        case WM_DESTROY:{
            //tells the application's message loop to stop
            PostQuitMessage(0);
            return 0;
        }
    }

    //let windows handle message not handled above 
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//Application entry point: creates the window and runs the game
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    //defiens the properties of the application window
    WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc; // assign the function that handles window messages
    wc.hInstance = hInstance; // stores the application instance 
	wc.lpszClassName = CLASS_NAME; // assigns the window class name
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);//sets the mouse cursor to the standard arrow 

	//registers the window class with the operating system
    if(!RegisterClass(&wc)) {
        return 1;
	}

    //creates the CubeBeatz application window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"CubeBeatz",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        1280, 720, 
        nullptr, nullptr, hInstance, nullptr);

    //stops if the window could not be created 
    if (hwnd == nullptr) {
        return 0;
    }

    //intializes Direct3d before rendering 
    if (!InitializeDirect3D(hwnd)) {

		//displays an error message if Direct3D could not be initialized
        MessageBox(hwnd, L"Failed to initialize Direct3D", L"CubeBeatz Error", MB_OK | MB_ICONERROR);
		
        // releases Direct3D resources
		if (renderTargetView) renderTargetView->Release();
        if (swapChain) swapChain->Release();
        if (deviceContext) deviceContext->Release();
        if (device) device->Release();

		//closes the windoe and reports failure to the operating system
		DestroyWindow(hwnd);
		return 1;

    }
    //displays the application window
    ShowWindow(hwnd, nCmdShow);
    //stores message recieved from windows
    MSG msg = {};
    while (msg.message != WM_QUIT) // continues running until a quit msg pops up 
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
    //release direct3d resources before program exits 
	if (renderTargetView) renderTargetView->Release();
	if (swapChain) swapChain->Release();
	if (deviceContext) deviceContext->Release();
	if (device) device->Release();
    //reports id it exits successfully 
    return 0;
}
