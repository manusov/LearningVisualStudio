/*
Class for GUI window.
*/

#include "Window.h"

HWND Window::hWnd = NULL;

#if _WIN64
const char* const Window::MSG_HELLO = "GUI combined template (x64)";
#elif _WIN32
const char* const Window::MSG_HELLO = "GUI combined template (ia32)";
#endif

const char* const Window::szClassName = "ClassName";
const char* const Window::szWindowName = "Platform framework";

BOOL Window::create()
{
    HINSTANCE h = GetModuleHandle(NULL);
    return WndCreate(h, SW_SHOWNORMAL);
}

BOOL Window::destroy()
{
	BOOL a = DestroyWindow(hWnd);
    return a;
}

BOOL Window::update()
{
	// Reserved for GUI support.
	return FALSE;
}

BOOL Window::WndCreate(HINSTANCE hInstance, int nCmdShow)
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

    hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        szClassName,
        szWindowName,
        WS_OVERLAPPEDWINDOW,
        100,
        120,
        400,
        300,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return TRUE;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
