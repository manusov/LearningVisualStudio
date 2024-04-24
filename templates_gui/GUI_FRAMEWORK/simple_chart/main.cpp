/*

Simple GUI application: chart Y=F(X) function(s).
First step (this source file) is modeling and debug as single source file main.cpp,
second step is integration to classes or libraries.

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#if _WIN64
const char* const WIN_NAME = "Simple chart v0.00.01 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Simple chart v0.00.01 (ia32)";
#endif

constexpr int MAX_STRING = 160;

constexpr int WINDOW_BASE_X = 500;
constexpr int WINDOW_BASE_Y = 300;
constexpr int WINDOW_SIZE_X = 580;
constexpr int WINDOW_SIZE_Y = 482;
constexpr int WINDOW_MIN_X = 300;
constexpr int WINDOW_MIN_Y = 200;

constexpr int GRID_LEFT = 60;
constexpr int GRID_RIGHT = 7;
constexpr int GRID_TOP = 7;
constexpr int GRID_DOWN = 49;

constexpr COLORREF GRID_COUNT_COLOR = RGB(70, 70, 70);
constexpr COLORREF UNITS_NAMES_COLOR = RGB(90, 30, 90);

const char* const szClassName = "ClassSimpleChart";

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

// Function Y=F(X) parameters. TODO. Need transfer to function as structure.
const char* F_AXIS_NAME_X = "X";
const char* F_AXIS_NAME_Y = "Y=F(X)";
constexpr int F_GRID_COUNT_X = 10;
constexpr int F_GRID_COUNT_Y = 11;
constexpr double F_START_X = 0.0;
constexpr double F_GRID_X = 2.5;
constexpr double F_START_Y = 0.0;
constexpr double F_GRID_Y = 0.1;
constexpr int F_CURVES_COUNT = 3;
const char* F_CURVES_NAMES[] = { "Y=X/30", "Y=X/40", "Y=sin(X)/2+0.5" };
// Argument and functions vectors.
constexpr int N_F = 2000;
std::vector<double> vX;
std::vector<double> vy1;
std::vector<double> vy2;
std::vector<double> vy3;
std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };
// End of function(s) parameters block.

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL createContext();
BOOL deleteContext();
void paint(HDC& hdc, RECT& rc);

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    char buffer[MAX_STRING];

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    const double qX = 22.5 / N_F;
    double bX = 0.0;
    for (int i = 0; i < N_F; i++)
    {
        vX.push_back(bX);
        vY[0]->push_back(bX / 30.0);
        vY[1]->push_back(bX / 40.0);
        vY[2]->push_back(sin(bX) / 2.0 + 0.5);
        bX += qX;
    }

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
    // Build argument X statistics, calibrating X-axis, select output mode for X-axis.
    BOOL printTenthsX = FALSE;
    std::vector<double> gridX;
    double targetX = F_START_X;
    for (int i = 0; i < F_GRID_COUNT_X; i++)
    {
        int temp = static_cast<int>(targetX * 10);
        if (temp % 10)
        {
            printTenthsX = TRUE;
        }
        gridX.push_back(targetX);
        targetX += F_GRID_X;
    }

    // Build function Y=F(X) statistics, calibrating Y-axis, select output mode for Y-axis.
    BOOL printTenthsY = FALSE;
    std::vector<double> gridY;
    double targetY = F_START_Y;
    for (int i = 0; i < F_GRID_COUNT_Y; i++)
    {
        int temp = static_cast<int>(targetY * 10);
        if (temp % 10)
        {
            printTenthsY = TRUE;
        }
        gridY.push_back(targetY);
        targetY += F_GRID_Y;
    }

    // Calculate variables for display coordinates translation.
    const int winX = rc.right - rc.left;
    const int winY = rc.bottom - rc.top;

    // Set display coordinates translation and transparency option.
    SetMapMode(hdc, MM_ISOTROPIC);
    SetWindowExtEx(hdc, winX, winY, NULL);
    SetViewportExtEx(hdc, winX, -winY, NULL);
    SetViewportOrgEx(hdc, 0, winY, NULL);
    SetBkMode(hdc, TRANSPARENT);

    // Change display device context parameters, save original values for restore.
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPens[GRID_PEN]));

    // Calculations for draw horizontal grid lines, for vertical axis counts (Y).
    int hx1 = GRID_LEFT;
    int hx2 = winX - GRID_RIGHT;
    int hy1 = GRID_DOWN;
    int hy2 = hy1;
    int modX = (hx2 - hx1) % F_GRID_COUNT_X;
    hx2 -= modX;
    int dX = (hx2 - hx1) / F_GRID_COUNT_X;

    // Calculations for draw vertical grid lines, for horizontal axis counts (X).
    int vx1 = hx1;
    int vx2 = vx1;
    int vy1 = hy1;
    int vy2 = winY - GRID_TOP;
    int modY = (vy2 - vy1) % F_GRID_COUNT_Y;
    vy2 -= modY;
    int dY = (vy2 - vy1) / F_GRID_COUNT_Y;

    // Draw horizontal grid lines, for vertical axis counts (Y).
    for (int y = hy1; y <= vy2; y += dY)
    {
        MoveToEx(hdc, hx1, y, nullptr);
        LineTo(hdc, hx2, y);
    }

    // Draw vertical grid lines, for horizontal axis counts (X).
    for (int x = vx1; x <= hx2; x += dX)
    {
        MoveToEx(hdc, x, vy1, nullptr);
        LineTo(hdc, x, vy2);
    }

    // Transit buffer for text strings write and other variables.
    char buffer[MAX_STRING];
    SIZE labelSize;

    // Draw horizontal grid values, X counts.
    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hFonts[GRID_FONT]));
    SetTextColor(hdc, GRID_COUNT_COLOR);
    for (int i = 0, x = vx1; i < F_GRID_COUNT_X; i++, x += dX)
    {
        if (printTenthsX)
        {
            double a = gridX[i];
            snprintf(buffer, MAX_STRING, "%.1f", a);
        }
        else
        {
            unsigned int a = static_cast<unsigned int>(gridX[i]);
            snprintf(buffer, MAX_STRING, "%u", a);
        }
        int length = static_cast<int>(strlen(buffer));
        GetTextExtentPoint32(hdc, buffer, length, &labelSize);
        if ((labelSize.cx < dX) || (i % 2 == 0))
        {
            TextOut(hdc, x - 6, vy1 - 2, buffer, length);
        }
    }

    // Draw vertical grid values, Y counts.
    for (int i = 1, y = hy1 + dY; i < F_GRID_COUNT_Y; i++, y += dY)
    {
        if (printTenthsY)
        {
            double a = gridY[i];
            snprintf(buffer, MAX_STRING, "%.1f", a);
        }
        else
        {
            unsigned int a = static_cast<unsigned int>(gridY[i]);
            snprintf(buffer, MAX_STRING, "%u", a);
        }
        int length = static_cast<int>(strlen(buffer));
        GetTextExtentPoint32(hdc, buffer, length, &labelSize);
        TextOut(hdc, hx1 - labelSize.cx - 2, y + labelSize.cy / 2, buffer, length);
    }

    // Write X axis name.
    SelectObject(hdc, hFonts[AXIS_FONT]);
    SetTextColor(hdc, UNITS_NAMES_COLOR);
    int length = static_cast<int>(strlen(F_AXIS_NAME_X));
    GetTextExtentPoint32(hdc, F_AXIS_NAME_X, length, &labelSize);
    int labelX = winX - labelSize.cx - 5;
    int labelY = labelSize.cy + 5;
    TextOut(hdc, labelX, labelY, F_AXIS_NAME_X, length);

    // Write Y axis name.
    labelX = 5;
    labelY = winY - 5;
    length = static_cast<int>(strlen(F_AXIS_NAME_Y));
    TextOut(hdc, labelX, labelY, F_AXIS_NAME_Y, length);

    // Write curves names, down string.
    labelX = hx1;
    labelY = 0;
    for (int i = 0; i < F_CURVES_COUNT; i++)
    {
        SetTextColor(hdc, PEN_DATA[i + 1].color);
        int length = static_cast<int>(strlen(F_CURVES_NAMES[i]));
        GetTextExtentPoint32(hdc, F_CURVES_NAMES[i], length, &labelSize);
        labelY = labelSize.cy + 4;
        TextOut(hdc, labelX, labelY, F_CURVES_NAMES[i], length);
        labelX += labelSize.cx + 20;
    }

    // Draw curves Y=F(X).
    double pixelsPerX = (hx2 - hx1) / (targetX);
    double pixelsPerY = (vy2 - vy1) / (targetY);

    for (int i = 0; i < F_CURVES_COUNT; i++)
    {
        SelectObject(hdc, hPens[i + 1]);
        MoveToEx(hdc, static_cast<int>(vX[0] * pixelsPerX) + hx1, static_cast<int>((*(vY[i]))[0] * pixelsPerY) + vy1, NULL);
        for (int j = 0; j < N_F; j++)
        {
            LineTo(hdc, static_cast<int>(vX[j] * pixelsPerX) + hx1, static_cast<int>((*(vY[i]))[j] * pixelsPerY) + vy1);
        }
    }
    // Restore display device context parameters: default font, default pen, default brush.
    SelectObject(hdc, hOldFont);
    SelectObject(hdc, hOldPen);
}
