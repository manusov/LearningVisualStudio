/*
Template for C++ GUI application.
*/

#include <windows.h>

#if _WIN64
const char* const MSG_HELLO = "GUI template v0.01.00 (x64)";
#elif _WIN32
const char* const MSG_HELLO = "GUI template v0.01.00 (ia32)";
#endif

const char* const szClassName = "ClassName";
const char* const szWindowName = "Platform framework";
HWND hwnd;
LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    MSG         Msg;
    WNDCLASSEX  WndClsEx = { 0 };

    WndClsEx.cbSize = sizeof(WNDCLASSEX);
    WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc = WndProc;
    WndClsEx.hInstance = hInstance;
    WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClsEx.lpszClassName = szClassName;
    WndClsEx.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

    RegisterClassEx(&WndClsEx);

    hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        szClassName,
        szWindowName,
        WS_OVERLAPPEDWINDOW,
        100,
        120,
        600,
        100,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return 0;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_DESTROY:
        PostQuitMessage(WM_QUIT);
        break;
    case WM_PAINT:
    {
        /* */
        PAINTSTRUCT ps;
        HDC         hdc;
        RECT        rc;
        hdc = BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rc);
        SetTextColor(hdc, 0);
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, MSG_HELLO, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        EndPaint(hwnd, &ps);
        /* */
        break;
    }
    break;
    default:
        return DefWindowProc(hwnd, Msg, wParam, lParam);
    }
    return 0;
}
