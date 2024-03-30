/*
Template for C++ GUI application.
Draw function Y=F(X).
UNDER CONSTRUCTION.
*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

// #define _DEFAULT_TEMPLATE_MODE

// Constants for function draw GUI application.
constexpr int MAX_STRING = 80;
#if _WIN64
const char* const MSG_HELLO = "Draw function Y=F(X) template v0.00.00 (x64)";
#elif _WIN32
const char* const MSG_HELLO = "Draw function Y=F(X) template v0.00.00 (ia32)";
#endif
constexpr int WINDOW_BASE_X = 100;
constexpr int WINDOW_BASE_Y = 120;
constexpr int WINDOW_SIZE_X = 580;  // 790;
constexpr int WINDOW_SIZE_Y = 480;  // 560;
constexpr int WINDOW_MIN_X = 500;   // 395;
constexpr int WINDOW_MIN_Y = 400;   // 230;
constexpr int SX1 = 65;
constexpr int SX2 = 170;
constexpr int SY1 = 5;
constexpr int SY2 = 75;
constexpr COLORREF DRAW_ZONE_COLOR = RGB(254, 254, 235);
constexpr COLORREF DRAW_FRAME_COLOR = RGB(195, 195, 195);
constexpr COLORREF AXIS_GRAD_COLOR = RGB(1, 1, 1);
constexpr COLORREF UNIT_NAME_COLOR = RGB(3, 3, 150);
const char* const UNIT_NAME_X = "X";
const char* const UNIT_NAME_Y = "F(X)";
constexpr COLORREF DRAW_LINE_COLORS[3] = { RGB(254, 10, 10), RGB(10, 254, 10), RGB(10, 10, 254) };
const char* const FUNCTIONS_NAMES[3] = { "Y=200*sin(X)+500", "Y=200*cos(X)+500", "Y=50*cos(3*X)+700" };
const char* const MIN_STAT     = "min     %.3f";
const char* const MAX_STAT     = "max     %.3f";
const char* const AVERAGE_STAT = "average %.3f";
const char* const MEDIAN_STAT  = "median  %.3f";
const char* const DOWN_STRING_1 = "This is engineering sample for draw functions Y=F(X).";
const char* const DOWN_STRING_2 = "Variant for 3 math functions.";
constexpr COLORREF DOWN_STRINGS_COLOR = RGB(1, 1, 1);
const char* const szClassName = "ClassNameDrawYX";
const char* const szWindowName = MSG_HELLO;

// TODO. Use this from AppLib:: when integration.
// Constants used for print memory block size.
constexpr int KILO = 1024;
constexpr int MEGA = 1024 * 1024;
constexpr int GIGA = 1024 * 1024 * 1024;

// Constants for function(s) Y=F(X).
constexpr int GRID_X = 11;
constexpr int GRID_Y = 10;
constexpr long long GRID_ZERO_X = 0;
constexpr long long GRID_ZERO_Y = 0;
constexpr long long GRID_UNIT_X = 1;
constexpr long long GRID_UNIT_Y = 150;

// Function Y=F(X).
constexpr int N_F = 1000;
std::vector<double> vX;
std::vector<double> vy1;
std::vector<double> vy2;
std::vector<double> vy3;
std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

// Statistic parameters.
typedef struct {
    double min;
    double max;
    double average;
    double median;
} FUNCTION_STATISTICS;
FUNCTION_STATISTICS functionStatistics[3];

// Variables and declarations.
HWND hwnd;
LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void PaintHandlerDefault(HDC hdc, RECT rc);
void PaintHandlerDrawXY(HDC hdc, RECT rc);
// TODO. Get this from AppLib::, but parameter-vector transfer by value.
void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);

// GUI application entry point and functions.
int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    for (int i = 0; i < N_F; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back (200 * sin(a) + 500);
        vY[1]->push_back(200 * cos(a) + 500);
        vY[2]->push_back(50 * cos(3 * a) + 700);
    }
    for (int i = 0; i < 3; i++)
    {
        functionStatistics[i].min = 0;
        functionStatistics[i].max = 0;
        functionStatistics[i].average = 0;
        functionStatistics[i].median = 0;
    }

    MSG         Msg;
    WNDCLASSEX  WndClsEx = { 0 };

    WndClsEx.cbSize = sizeof(WNDCLASSEX);
    WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc = WndProc;
    WndClsEx.hInstance = hInstance;
    WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClsEx.lpszClassName = szClassName;
    WndClsEx.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
    WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&WndClsEx);

    hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        szClassName,
        szWindowName,
        WS_OVERLAPPEDWINDOW,
        WINDOW_BASE_X,
        WINDOW_BASE_Y,
        WINDOW_SIZE_X,
        WINDOW_SIZE_Y,
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
    MINMAXINFO* p = nullptr;
    LONG minX = 0, minY = 0;
    switch (Msg)
    {
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
        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
#ifdef _DEFAULT_TEMPLATE_MODE
        PaintHandlerDefault(hdc, rc);
#else
        PaintHandlerDrawXY(hdc, rc);
#endif
        EndPaint(hwnd, &ps);
        break;
    }
    break;
    default:
        return DefWindowProc(hwnd, Msg, wParam, lParam);
    }
    return 0;
}
void PaintHandlerDefault(HDC hdc, RECT rc)
{
    SetTextColor(hdc, 0);
    SetBkMode(hdc, TRANSPARENT);
    char buffer[MAX_STRING];
    wsprintf(buffer, "%s.", MSG_HELLO);
    DrawText(hdc, buffer, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
}
void PaintHandlerDrawXY(HDC hdc, RECT rc)
{
    const int dX = rc.right - rc.left;
    const int dY = rc.bottom - rc.top;
    const int x1 = SX1;
    const int y1 = SY1;
    const int x2 = dX - SX2;
    const int y2 = dY - SY2;
    const int rX = x2 - x1;
    const int rY = y2 - y1;

    SetMapMode(hdc, MM_ISOTROPIC);
    SetWindowExtEx(hdc, dX, dY, NULL);
    SetViewportExtEx(hdc, dX, -dY, NULL);
    SetViewportOrgEx(hdc, x1, y2, NULL);

    HBRUSH hBrush = CreateSolidBrush(DRAW_ZONE_COLOR);       // TODO. Optimize, this operation better at window create.
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, hBrush));
    HPEN hPen0 = CreatePen(PS_SOLID, 0, DRAW_FRAME_COLOR);   // TODO. Optimize, this operation better at window create.
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, hPen0));
    
    static LOGFONT lf;                                       // TODO. Optimize, this operation better at window create.
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    lf.lfItalic = TRUE;
    lf.lfWeight = FW_MEDIUM;
    lf.lfHeight = 16;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    HFONT hFont0 = CreateFontIndirect(&lf);                 // TODO. Optimize, this operation better at window create. This font for X,Y axis graduation.
    
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lf.lfItalic = TRUE;
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = 17;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    HFONT hFont1 = CreateFontIndirect(&lf);                 // TODO. Optimize, this operation better at window create.  This fonts for axis names.

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    // lf.lfItalic = TRUE;
    lf.lfWeight = FW_THIN;
    lf.lfHeight = 20;
    lf.lfCharSet = DEFAULT_CHARSET;
    //lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    HFONT hFont2 = CreateFontIndirect(&lf);                 // TODO. Optimize, this operation better at window create. This font for statistics.

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    // lf.lfItalic = TRUE;
    lf.lfWeight = FW_THIN;
    lf.lfHeight = 19;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    HFONT hFont3 = CreateFontIndirect(&lf);                  // TODO. Optimize, this operation better at window create. This font for down strings.

    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, hFont0));
    SetTextColor(hdc, AXIS_GRAD_COLOR);
    SetBkMode(hdc, TRANSPARENT);

    Rectangle(hdc, 0, 0, rX, rY);  // Rectangle color = current selected brush, border = current selected pen.

    char buffer[MAX_STRING];

    int gridStepX = rX / GRID_X;
    int gridX = 0;
    long long gridValueX = GRID_ZERO_X;
    for (int i = 0; i < GRID_X; i++)
    {
        MoveToEx(hdc, gridX, 0, nullptr);
        LineTo(hdc, gridX, rY);
        snprintf(buffer, MAX_STRING, "%llu", gridValueX);
        
        // TextOut(hdc, gridX - 6, -6, buffer, static_cast<int>(strlen(buffer)));

        SIZE textSize;
        GetTextExtentPoint32(hdc, buffer, static_cast<int>(strlen(buffer)), &textSize);
        if ((textSize.cx < gridStepX)||(( i % 2 == 0) && (i != GRID_X - 1)))
        {
            TextOut(hdc, gridX - 6, -6, buffer, static_cast<int>(strlen(buffer)));
        }
        
        gridX += gridStepX;
        gridValueX += GRID_UNIT_X;
    }
    double pixelsPerX = static_cast<double>(gridX) / gridValueX;

    int gridStepY = rY / GRID_Y;
    int gridY = gridStepY;
    long long gridValueY = GRID_UNIT_Y;
    for (int i = 0; i < (GRID_Y - 1); i++)
    {
        MoveToEx(hdc, 0, gridY, nullptr);
        LineTo(hdc, rX, gridY);
        snprintf(buffer, MAX_STRING, "%llu", gridValueY);
        SIZE textSize;
        GetTextExtentPoint32(hdc, buffer, static_cast<int>(strlen(buffer)), &textSize);
        TextOut(hdc, -textSize.cx - 5, gridY + 8, buffer, static_cast<int>(strlen(buffer)));
        gridY += gridStepY;
        gridValueY += GRID_UNIT_Y;
    }
    double pixelsPerY = static_cast<double>(gridY) / gridValueY;

    SelectObject(hdc, hFont1);
    SetTextColor(hdc, UNIT_NAME_COLOR);
    TextOut(hdc, gridX + 3, -6, UNIT_NAME_X, static_cast<int>(strlen(UNIT_NAME_X)));
    TextOut(hdc, -50, gridY + 8, UNIT_NAME_Y, static_cast<int>(strlen(UNIT_NAME_Y)));

    HPEN hPens[3];
    for (int i = 0; i < 3; i++)
    {
        hPens[i] = CreatePen(PS_SOLID, 2, DRAW_LINE_COLORS[i]);
        SelectObject(hdc, hPens[i]);
        MoveToEx(hdc, static_cast<int>(vX[0] * pixelsPerX), static_cast<int>((*(vY[i]))[0] * pixelsPerY), NULL);
        for (int j = 0; j < N_F; j++)
        {
            LineTo(hdc, static_cast<int>(vX[j] * pixelsPerX), static_cast<int>((*(vY[i]))[j] * pixelsPerY));
        }
    }

    SelectObject(hdc, hFont2);
    int shiftY = 6;
    for (int i = 0; i < 3; i++)
    {
        calculateStatistics(*(vY[i]), functionStatistics[i].min, functionStatistics[i].max, functionStatistics[i].average, functionStatistics[i].median);
        SetTextColor(hdc, DRAW_LINE_COLORS[i]);
        TextOut(hdc, gridX + 20, gridY + shiftY, FUNCTIONS_NAMES[i], static_cast<int>(strlen(FUNCTIONS_NAMES[i])));
        snprintf(buffer, MAX_STRING, MIN_STAT, functionStatistics[i].min);
        TextOut(hdc, gridX + 20, gridY - 16 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, MAX_STAT, functionStatistics[i].max);
        TextOut(hdc, gridX + 20, gridY - 32 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, AVERAGE_STAT, functionStatistics[i].average);
        TextOut(hdc, gridX + 20, gridY - 48 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, MEDIAN_STAT, functionStatistics[i].median);
        TextOut(hdc, gridX + 20, gridY - 64 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        shiftY -= 96;
    }

    SetTextColor(hdc, DOWN_STRINGS_COLOR);
    SelectObject(hdc, hFont3);
    TextOut(hdc, -50, -32, DOWN_STRING_1, static_cast<int>(strlen(DOWN_STRING_1)));
    TextOut(hdc, -50, -48, DOWN_STRING_2, static_cast<int>(strlen(DOWN_STRING_2)));

    SelectObject(hdc, hOldFont);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hFont0);                                   // TODO. Optimize, this operation better at window destroy. + Make array of fonts.
    DeleteObject(hFont1);
    DeleteObject(hFont2);
    DeleteObject(hFont3);
    DeleteObject(hPen0);                                    // TODO. Optimize, this operation better at window destroy.
    for (int i = 0; i < 3; i++)
    {
        DeleteObject(hPens[i]);
    }
    DeleteObject(hBrush);                                   // TODO. Optimize, this operation better at window destroy.
}
// TODO. Get this from AppLib::, but parameter-vector transfer by value.
void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median)
{
    size_t n = data.size();
    if (n)
    {
        std::sort(data.begin(), data.end());
        double sum = std::accumulate(data.begin(), data.end(), double(0));  // BUG FIXED: double(0).
        min = data[0];
        max = data[n - 1];
        average = sum / n;
        median = (n % 2) ? (data[n / 2]) : ((data[n / 2 - 1] + data[n / 2]) / 2.0);
    }
    else
    {
        min = 0.0;
        max = 0.0;
        average = 0.0;
        median = 0.0;
    }
}
