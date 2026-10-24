/* Authors:
* Course:
* Date:
* 
* Project Name: CubeBeatz
*/

#include <windows.h>

const wchar_t CLASS_NAME[] = L"CubeBeatzWindowClass";

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {
        case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
		GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		EndPaint(hwnd, &ps);
        return 0; 
    }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"CubeBeatz",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        1280, 720, 
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
        return 0;

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
            // Direct3D rendering will go here.
        }
    }

    return 0;
}
