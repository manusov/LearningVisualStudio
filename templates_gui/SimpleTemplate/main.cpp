/*

Simple GUI application template.

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#if _WIN64
const char* const WIN_NAME = "Simple template v0.00.00 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Simple template v0.00.00 (ia32)";
#endif

constexpr int MAX_STRING = 160;

constexpr int WINDOW_BASE_X = 500;
constexpr int WINDOW_BASE_Y = 300;
constexpr int WINDOW_SIZE_X = 580;
constexpr int WINDOW_SIZE_Y = 482;
constexpr int WINDOW_MIN_X = 300;
constexpr int WINDOW_MIN_Y = 200;

const char* const szClassName = "ClassSimpleTemplate";

HWND hWnd;

typedef enum {
    BRUSH_BACKGROUND,
    MAX_BRUSH
} USED_BRUSHES;
constexpr COLORREF BRUSH_COLORS[] = { RGB(254, 254, 235) };
HBRUSH hBrushes[MAX_BRUSH];

typedef enum
{
    GRID_PEN,
    LINE_1_PEN,
    LINE_2_PEN,
    LINE_3_PEN,
    MAX_PEN
} USED_PENS;
typedef struct
{
    COLORREF color;
    int width;
}  PEN_ENTRY;
constexpr PEN_ENTRY PEN_DATA[] =
{
    { RGB(195, 195, 195), 0 },
    { RGB(254, 10, 10),   2 },
    { RGB(10, 160, 10),   2 },
    { RGB(10, 10, 254),   2 }
};
HPEN hPens[MAX_PEN];

typedef enum
{
    AXIS_FONT,
    GRID_FONT,
    FUNCTIONS_FONT,
    MAX_FONT
} USED_FONTS;
typedef struct
{
    const char* name;
    LONG height;
    LONG weight;
    BYTE italic;
    BYTE pitchAndFamily;
} FONT_ENTRY;
constexpr FONT_ENTRY FONT_DATA[] =
{
    { "Verdana", 15, FW_BOLD, TRUE,  FIXED_PITCH  },
    { "Verdana", 14, FW_THIN, FALSE, FIXED_PITCH  },
    { "Verdana", 15, FW_BOLD, FALSE, FIXED_PITCH  }
};
HFONT hFonts[MAX_FONT];

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL createContext();
BOOL deleteContext();
void paint(HDC& hdc, RECT& rc);

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    char buffer[MAX_STRING];

    // Initializing and show GUI window.
    if (!createContext())
    {
        wsprintf(buffer, "Cannot create GUI resources.");
        MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        WNDCLASSEX  WndClsEx = { 0 };
        WndClsEx.cbSize = sizeof(WNDCLASSEX);
        WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
        WndClsEx.lpfnWndProc = WndProc;
        WndClsEx.hInstance = hInstance;
        WndClsEx.hbrBackground = hBrushes[BRUSH_BACKGROUND]; // (HBRUSH)GetStockObject(WHITE_BRUSH);
        WndClsEx.lpszClassName = szClassName;
        WndClsEx.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
        WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);

        static ATOM a;
        if (!a)
        {
            a = RegisterClassEx(&WndClsEx);
            if (!a)
            {
                wsprintf(buffer, "Cannot register class: %s.", szClassName);
                MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
            }
        }

        if (a)
        {
            hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                szClassName,
                WIN_NAME,
                WS_OVERLAPPEDWINDOW,
                WINDOW_BASE_X,
                WINDOW_BASE_Y,
                WINDOW_SIZE_X,
                WINDOW_SIZE_Y,
                NULL,
                NULL,
                hInstance,
                NULL);

            if (!hWnd)
            {
                wsprintf(buffer, "Cannot create window: %s.", WIN_NAME);
                MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
            }
            else
            {
                ShowWindow(hWnd, nCmdShow);
                UpdateWindow(hWnd);
                MSG Msg;
                while (GetMessage(&Msg, NULL, 0, 0))
                {
                    TranslateMessage(&Msg);
                    DispatchMessage(&Msg);
                }
            }
        }
    }

    if (!deleteContext())
    {
        wsprintf(buffer, "Cannot delete GUI resources.");
        MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    MINMAXINFO* p = nullptr;
    LONG minX = 0, minY = 0;

    switch (Msg)
    {
        //
        //  case WM_CREATE:
        //      break;
        //
    case WM_DESTROY:
        PostQuitMessage(WM_QUIT);
        break;

    case WM_GETMINMAXINFO:
        p = reinterpret_cast<MINMAXINFO*>(lParam);
        minX = p->ptMinTrackSize.x;
        minY = p->ptMinTrackSize.y;
        if (minX < WINDOW_MIN_X)
        {
            p->ptMinTrackSize.x = WINDOW_MIN_X;
        }
        if (minY < WINDOW_MIN_Y)
        {
            p->ptMinTrackSize.y = WINDOW_MIN_Y;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC         hdc;
        RECT        rc{ 0 };
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rc);
        paint(hdc, rc);
        EndPaint(hWnd, &ps);
        break;
    }
    break;

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return 0;
}

BOOL createContext()
{
    BOOL bStatus = TRUE;

    for (int i = 0; i < MAX_BRUSH; i++)
    {
        hBrushes[i] = CreateSolidBrush(BRUSH_COLORS[i]);
        if (!(hBrushes[i]))
        {
            bStatus = FALSE;
        }
    }

    for (int i = 0; i < MAX_PEN; i++)
    {
        hPens[i] = CreatePen(PS_SOLID, PEN_DATA[i].width, PEN_DATA[i].color);
        if (!(hPens[i]))
        {
            bStatus = FALSE;
        }
    }

    LOGFONT lf;
    for (int i = 0; i < MAX_FONT; i++)
    {
        memset(&lf, 0, sizeof(LOGFONT));
        lf.lfPitchAndFamily = FONT_DATA[i].pitchAndFamily;
        lf.lfItalic = FONT_DATA[i].italic;
        lf.lfWeight = FONT_DATA[i].weight;
        lf.lfHeight = FONT_DATA[i].height;
        lf.lfCharSet = DEFAULT_CHARSET;
        lstrcpy((LPSTR)&lf.lfFaceName, FONT_DATA[i].name);
        hFonts[i] = CreateFontIndirect(&lf);
        if (!(hFonts[i]))
        {
            bStatus = FALSE;
        }
    }

    return bStatus;
}

BOOL deleteContext()
{
    BOOL bStatus = TRUE;

    for (int i = 0; i < MAX_BRUSH; i++)
    {
        if (!DeleteObject(hBrushes[i]))
        {
            bStatus = FALSE;
        }
    }

    for (int i = 0; i < MAX_PEN; i++)
    {
        if (!DeleteObject(hPens[i]))
        {
            bStatus = FALSE;
        }
    }

    for (int i = 0; i < MAX_FONT; i++)
    {
        if (!DeleteObject(hFonts[i]))
        {
            bStatus = FALSE;
        }
    }

    return bStatus;
}

void paint(HDC& hdc, RECT& rc)
{
    // Add draw here.
}
