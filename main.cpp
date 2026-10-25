/* Authors:
* Course:
* Date:
* 
* Project Name: CubeBeatz
*/

#include <windows.h> //Windows function and Windows management
#include <d3d11.h> // Direct3D 11 graphics functionality
#include <dxgi.h> // graphics display and swap-chain functions
#include <d3dcompiler.h> //compiles HLSL shaders
#include <sal.h>

// Links the Direct3D and DXGI Libraries 
#pragma comment(lib, "d3d11.lib")  
#pragma comment(lib, "dxgi.lib") 
#pragma comment(lib, "d3dcompiler.lib") 

// Identifies the application window
const wchar_t CLASS_NAME[] = L"CubeBeatzWindowClass"; 

ID3D11Device* device = nullptr; //Direct3D device used to create graphic resources
ID3D11DeviceContext* deviceContext = nullptr; // sends rendering commands to Direct3D
IDXGISwapChain* swapChain = nullptr; // Manages the buffers used to display frames
ID3D11RenderTargetView* renderTargetView = nullptr; // Identifies the buffer where graphics are rendered 
ID3DBlob* vertexShaderBlob = nullptr; // Stores the compiled vertex shader code
ID3DBlob* pixelShaderBlob = nullptr; // Stores the compiled pixel shader code

// Direct3D shader objects used by the GPU.
ID3D11VertexShader* vertexShader = nullptr;
ID3D11PixelShader* pixelShader = nullptr;

// Stores cube vertex data on the GPU.
ID3D11Buffer* vertexBuffer = nullptr;

// Stores triangle indices on the GPU.
ID3D11Buffer* indexBuffer = nullptr;

// Describes how Direct3D reads the cube's vertex data.
ID3D11InputLayout* inputLayout = nullptr;

//Stores the 3d position of our cube vertex
struct Vertex {
    float x, y, z; 
};

//The 8 corners of our cubes 
Vertex cubeVertices[] = {
    {-0.5f, -0.5f, -0.5f}, //back bottom left of cube
    {-0.5f, 0.5f, -0.5f}, // back top left of cube
    {0.5f, 0.5f, -0.5f},//back top right of cube
    {0.5f, -0.5f, -0.5f},// back bottom right of cube

    {-0.5f, -0.5f, 0.5f}, //front bottom left of cube
    {-0.5f, 0.5f, 0.5f}, //front top left of cube
    {0.5f, 0.5f, 0.5f}, //front top right of cube
    {0.5f, -0.5f, 0.5f}, // front botttom right of cube 
};

//The 12 triangles that makes the cubes 6 sides
unsigned int cubeIndices[] = {
    //Back side
    0,1,2, 
    0,2,3,

    //front side
    4,6,5,
    4,7,6,

    //left side
    4,5,1,
    4,1,0,

    //right side
    3,2,6,
    3,6,7,

    //top side
    1,5,6,
    1,6,2,

    //bottom side
    4,0,3,
    4,3,7

};

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

    // Sets the render target used by the output-merger stage
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

    // Defines the area of the window where Direct3D draws.
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = 1280.0f;
    viewport.Height = 720.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;


    // Applies the viewport settings.
    deviceContext->RSSetViewports(1, &viewport);

    // Creates a rasterizer state that draws both sides
    // of every triangle, for troubleshooting.
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthClipEnable = TRUE;

    ID3D11RasterizerState* rasterState = nullptr;

    HRESULT rasterResult =
        device->CreateRasterizerState(&rasterDesc, &rasterState);

    if (FAILED(rasterResult)) {
        return false;
    }

    // Applies the rasterizer state to the graphics pipeline.
    deviceContext->RSSetState(rasterState);

    // Releases our local reference; Direct3D retains its own.
    rasterState->Release();

    // Reports that initialization succeeded.
    return true;

}

bool CompileShader(const wchar_t* filename, const char* entryPoint, const char* shaderModel, ID3DBlob** shaderblob) {
	ID3DBlob* errorBlob = nullptr;

    HRESULT result = D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,D3DCOMPILE_ENABLE_STRICTNESS, 0, shaderblob, &errorBlob);

    if (FAILED(result)) {
        if (errorBlob != nullptr) {
            MessageBoxA(nullptr,(const char*)errorBlob->GetBufferPointer(),
                "Shader Compilation Error",MB_OK | MB_ICONERROR);
            errorBlob->Release();
        }
        return false;
	}

    if (errorBlob != nullptr) {
        errorBlob->Release();
    }
	return true;
}

//Creates Direct3D shader objects
bool CreateShaders() {
    if (device == nullptr ||
        vertexShaderBlob == nullptr ||
        pixelShaderBlob == nullptr) {
        return false;
    }

    HRESULT result = device -> CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(result)) {
        return false;
    }
    result = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &pixelShader);
    if (FAILED(result)) {
        return false;
    }
	return true;
}

//Creating the cubebuffers 
bool CreateCubebuffers() {

    //the vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(cubeVertices);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    // Tell Direct3D where the cube's vertex data is stored.
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = cubeVertices;

    // Create the vertex buffer on the GPU.
    HRESULT result = device->CreateBuffer(
        &vertexBufferDesc,
        &vertexData,
        &vertexBuffer
    );

    if (FAILED(result))
    {
        MessageBox(
            nullptr,
            L"Failed to create the cube vertex buffer.",
            L"Direct3D Error",
            MB_OK | MB_ICONERROR
        );
        return false;
    }

    // Describe the index buffer.
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(cubeIndices);
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    // Tell Direct3D where the cube's index data is stored.
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = cubeIndices;

    // Create the index buffer on the GPU.
    result = device->CreateBuffer(
        &indexBufferDesc,
        &indexData,
        &indexBuffer
    );

    if (FAILED(result))
    {
        MessageBox(
            nullptr,
            L"Failed to create the cube index buffer.",
            L"Direct3D Error",
            MB_OK | MB_ICONERROR
        );
        return false;
    }

    return true;
}

//Creates the input layout 
bool CreateInputLayout() {

    //format of each of the cube vertex
    D3D11_INPUT_ELEMENT_DESC layoutDesc = {};
    layoutDesc.SemanticName = "POSITION";
    layoutDesc.SemanticIndex = 0;
    layoutDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT; 
    layoutDesc.InputSlot = 0;
    layoutDesc.AlignedByteOffset = 0;
    layoutDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layoutDesc.InstanceDataStepRate = 0;

    //input layouy using the compiled vertex shader 
    HRESULT result = device->CreateInputLayout(&layoutDesc, 1, 
        vertexShaderBlob->GetBufferPointer(), 
        vertexShaderBlob->GetBufferSize(), &inputLayout);
    if (FAILED(result)) {
        MessageBox(nullptr, L"Failed to create the cube input layout.",
            L"Direct3D Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}


// Cleans up Direct3D resources and compiled shader bytecode.
void Cleanup()
{
    //releases the cube's buffers 
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }
    if (vertexBuffer) {
        vertexBuffer->Release(); 
        vertexBuffer = nullptr;
    }

    // Releases the input layout.
    if (inputLayout) {
        inputLayout->Release();
        inputLayout = nullptr;
    }

    // Releases the pixel shader object.
    if (pixelShader) {
        pixelShader->Release();
        pixelShader = nullptr;
    }

    // Releases the vertex shader object.
    if (vertexShader) {
        vertexShader->Release();
        vertexShader = nullptr;
    }

    // Releases the compiled pixel shader bytecode.
    if (pixelShaderBlob) {
        pixelShaderBlob->Release();
        pixelShaderBlob = nullptr;
    }

    // Releases the compiled vertex shader bytecode.
    if (vertexShaderBlob) {
        vertexShaderBlob->Release();
        vertexShaderBlob = nullptr;
    }

    // Releases the render target view.
    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }

    // Releases the swap chain.
    if (swapChain) {
        swapChain->Release();
        swapChain = nullptr;
    }

    // Releases the Direct3D device context.
    if (deviceContext) {
        deviceContext->Release();
        deviceContext = nullptr;
    }

    // Releases the Direct3D device.
    if (device) {
        device->Release();
        device = nullptr;
    }
}


//A Dark background color: 
void RenderFrame() {
    //RGBA color dark navy 
    float backgroundColor[] = {
        0.04F, 0.04f, 0.08f, 1.0f
    };

    //clears the render target with background color 
    deviceContext->ClearRenderTargetView(renderTargetView, backgroundColor);

    //direct3d uses the vertex shader
    deviceContext->VSSetShader(vertexShader, nullptr, 0); 

    //direct3d uses the pixel shader
    deviceContext->PSSetShader(pixelShader, nullptr, 0);

    //Interprets the cube vertex
    deviceContext->IASetInputLayout(inputLayout);

    //Defines the size of one vertex
    UINT vertexStride = sizeof(Vertex);

    UINT vertexOffset = 0; 

    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->DrawIndexed(36, 0, 0);

    //presents the completed frame to the window
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
    //defines the properties of the application window
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

    //initializes Direct3d before rendering 
    if (!InitializeDirect3D(hwnd)) {

		//displays an error message if Direct3D could not be initialized
        MessageBox(hwnd, L"Failed to initialize Direct3D", L"CubeBeatz Error", MB_OK | MB_ICONERROR);
		
        // releases Direct3D resources
        Cleanup();

		//closes the window and reports failure to the operating system
		DestroyWindow(hwnd);
		return 1;

    }

    //Compiles the vertex shader
    if (!CompileShader(L"VertexShader.hlsl", "main", "vs_5_0", &vertexShaderBlob)) {
        MessageBox(hwnd, L"Failed to compile vertex shader", L"CubeBeatz Error", MB_OK | MB_ICONERROR);
        
        Cleanup();
        DestroyWindow(hwnd); 
        return 1;
    }

    //Compiles the pixel shader
    if (!CompileShader(L"PixelShader.hlsl", "main", "ps_5_0", &pixelShaderBlob)) {
        MessageBox(hwnd, L"Failed to compile pixel shader", L"CubeBeatz Error", MB_OK | MB_ICONERROR);

        Cleanup();
        DestroyWindow(hwnd);
        return 1;
    }


    // Creates Direct3D shader objects from the compiled bytecode.
    if (!CreateShaders()) {
        MessageBox(hwnd, L"Failed to create Direct3D shaders",
            L"CubeBeatz Error", MB_OK | MB_ICONERROR);

        Cleanup();
        DestroyWindow(hwnd);
        return 1;
    }


    // Creates the cube's vertex and index buffers.
    if (!CreateCubebuffers()) {
        MessageBox(hwnd, L"Failed to create cube buffers",
            L"CubeBeatz Error", MB_OK | MB_ICONERROR);

        Cleanup();
        DestroyWindow(hwnd);
        return 1;
    }

    // Creates the input layout for cube vertices.
    if (!CreateInputLayout()) {
        MessageBox(hwnd, L"Failed to create input layout",
            L"CubeBeatz Error", MB_OK | MB_ICONERROR);

        Cleanup();
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
    Cleanup();

    //reports id it exits successfully 
    return 0;
}
