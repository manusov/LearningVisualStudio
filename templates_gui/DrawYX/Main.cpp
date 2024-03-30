/*

Template for C++ GUI application.
Draw function Y=F(X).
Variant without Window Class (KWnd).

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#if _WIN64
const char* const WIN_NAME = "Draw Y=F(X) v0.05.00 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Draw Y=F(X) v0.05.00 (ia32)";
#endif

// #define _DEFAULT_TEMPLATE_MODE

typedef struct {
	HINSTANCE hInstance;
	int nCmdShow;
	const COLORREF* linesColors;
	const char* winName;
} WINDOW_PARMS;

typedef struct {
	int numberOfFunctions;
	int numberOfValues;
	std::vector<double>* vX;
	std::vector<double>** vY;
	int fGridX;
	int fGridY;
	const char* fUnitNameX;
	const char* fUnitNameY;
	const char* fDownString1;
	const char* fDownString2;
	const char** functionsNames;
} FUNCTION_PARMS;

typedef struct {
	double min;
	double max;
	double average;
	double median;
} FUNCTION_STATISTICS;

typedef enum {
	ZONE_BRUSH,
	MAX_BRUSH
} USED_BRUSHES;

typedef enum {
	FRAME_PEN,
	LINE_1_PEN,
	LINE_2_PEN,
	LINE_3_PEN,
	MAX_PEN
} USED_PENS;

typedef enum {
	GRAD_FONT,
	AXIS_FONT,
	STAT_FONT,
	FUNC_FONT,
	DOWN_FONT,
	MAX_FONT
} USED_FONTS;

constexpr int MAX_STRING = 80;
constexpr int LINES_SUPPORTED = 3;
constexpr int WINDOW_BASE_X = 100;
constexpr int WINDOW_BASE_Y = 120;
constexpr int WINDOW_SIZE_X = 580;
constexpr int WINDOW_SIZE_Y = 480;
constexpr int WINDOW_MIN_X = 500;
constexpr int WINDOW_MIN_Y = 400;
constexpr int SX1 = 65;
constexpr int SX2 = 170;
constexpr int SY1 = 5;
constexpr int SY2 = 75;

constexpr COLORREF DRAW_ZONE_COLOR = RGB(254, 254, 235);
constexpr COLORREF DRAW_FRAME_COLOR = RGB(195, 195, 195);
constexpr COLORREF AXIS_GRAD_COLOR = RGB(1, 1, 1);
constexpr COLORREF UNIT_NAME_COLOR = RGB(3, 3, 150);
constexpr COLORREF DRAW_LINE_COLORS[LINES_SUPPORTED] = { RGB(254, 10, 10), RGB(10, 160, 10), RGB(10, 10, 254) };
constexpr COLORREF DOWN_STRINGS_COLOR = RGB(1, 1, 1);

const char* const szClassName = "ClassNameDrawYX";
const char* const MIN_STAT = "min     %.3f";
const char* const MAX_STAT = "max     %.3f";
const char* const AVERAGE_STAT = "average %.3f";
const char* const MEDIAN_STAT = "median  %.3f";

WINDOW_PARMS wP;
FUNCTION_PARMS fP;
WINDOW_PARMS* pW = &wP;
FUNCTION_PARMS* pF = &fP;

HWND hWnd;
FUNCTION_STATISTICS fnStat[LINES_SUPPORTED];
HBRUSH brushes[MAX_BRUSH];
HPEN pens[MAX_PEN];
HFONT fonts[MAX_FONT];

BOOL FunctionDraw(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow,
    std::vector<double>* vX, std::vector<double>** vY, int nF, int gridX, int gridY, const char** fNames);
LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void PaintHandlerDefault(HDC hdc, RECT rc);
void InitHandlerDrawYX();
void DeInitHandlerDrawYX();
void PaintHandlerDrawYX(HDC hdc, RECT rc);
void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Argument and functions vectors.
    std::vector<double> vX;
    std::vector<double> vy1;
    std::vector<double> vy2;
    std::vector<double> vy3;
    std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

    // Constants.
    int N_F = 101;
    int GRID_X = 11;
    int GRID_Y = 11;
    const char* functionsNames1[3] = { "Y=X", "Y=5X", "Y=X^2+500" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_F; i++)
    {
        double a = static_cast<double>(i);
        vX.push_back(a);
        vY[0]->push_back(a);
        vY[1]->push_back(5 * a);
        vY[2]->push_back(a * a + 500);
    }

    if (!FunctionDraw(hInstance, hPrevInstance, lpCmdLine, nCmdShow, &vX, vY, N_F, GRID_X, GRID_Y, functionsNames1))
    {
        return 1;
    }

    // Clear argument and functions vectors for second draw.
    vX.clear();
    vy1.clear();
    vy2.clear();
    vy3.clear();

    // Constants.
    N_F = 1001;
    GRID_X = 11;
    GRID_Y = 11;
    const char* functionsNames2[3] = { "Y=20+10*sin(X)", "Y=20+10*cos(X)", "Y=combined" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_F; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back(20 + 10 * sin(a));
        vY[1]->push_back(20 + 10 * cos(a));
        vY[2]->push_back(30 + 10 * sin(a) + cos(20 * a));
    }

    Sleep(400);
    if (!FunctionDraw(hInstance, hPrevInstance, lpCmdLine, nCmdShow, &vX, vY, N_F, GRID_X, GRID_Y, functionsNames2))
    {
        return 1;
    }

    return 0;
}

BOOL FunctionDraw(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow,
    std::vector<double>* vX, std::vector<double>** vY, int nF, int gridX, int gridY, const char** fNames)
{
    // Setup chart window parameters.
    wP.hInstance = hInstance;         // GetModuleHandle(NULL);
    wP.nCmdShow = nCmdShow;           // nCmdShow;  SW_SHOWNORMAL;
    wP.linesColors = nullptr;         // Default colors for lines.
    wP.winName = WIN_NAME;

    // Setup chart function parameters.
    fP.numberOfFunctions = 3;
    fP.numberOfValues = nF;
    fP.fGridX = gridX;
    fP.fGridY = gridY;
    fP.fUnitNameX = "X";
    fP.fUnitNameY = "F(X)";
    fP.fDownString1 = "This is engineering sample for draw functions Y=F(X).";
    fP.fDownString2 = "Variant for 3 math functions.";
    fP.functionsNames = fNames;
    fP.vX = vX;
    fP.vY = vY;

    WNDCLASSEX  WndClsEx = { 0 };

    WndClsEx.cbSize = sizeof(WNDCLASSEX);
    WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc = WndProc;
    WndClsEx.hInstance = pW->hInstance;
    WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClsEx.lpszClassName = szClassName;
    WndClsEx.hIconSm = LoadIcon(pW->hInstance, IDI_APPLICATION);
    WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);

    static ATOM a;
    if (!a)
    {
        a = RegisterClassEx(&WndClsEx);
        if (!a)
        {
            constexpr int MAX_STRING = 80;
            char buffer[MAX_STRING];
            wsprintf(buffer, "Cannot register class: %s.", szClassName);
            MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
            return FALSE;
        }
    }

    hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
        szClassName,
        pW->winName,
        WS_OVERLAPPEDWINDOW,
        WINDOW_BASE_X,
        WINDOW_BASE_Y,
        WINDOW_SIZE_X,
        WINDOW_SIZE_Y,
        NULL,
        NULL,
        pW->hInstance,
        NULL);

    if (!hWnd)
    {
        constexpr int MAX_STRING = 80;
        char buffer[MAX_STRING];
        wsprintf(buffer, "Cannot create window: %s.", WIN_NAME);
        MessageBox(NULL, buffer, "Error", MB_OK | MB_ICONERROR);
        return FALSE;
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
        return TRUE;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    MINMAXINFO* p = nullptr;
    LONG minX = 0, minY = 0;
    switch (Msg)
    {

    case WM_CREATE:
#ifndef _DEFAULT_TEMPLATE_MODE
        InitHandlerDrawYX();
#endif
        break;

    case WM_DESTROY:
#ifndef _DEFAULT_TEMPLATE_MODE
        DeInitHandlerDrawYX();
#endif
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
#ifdef _DEFAULT_TEMPLATE_MODE
        PaintHandlerDefault(hdc, rc);
#else
        PaintHandlerDrawYX(hdc, rc);
#endif
        EndPaint(hWnd, &ps);
        break;
    }
    break;

    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    return 0;
}

void PaintHandlerDefault(HDC hdc, RECT rc)
{
    SetTextColor(hdc, 0);
    SetBkMode(hdc, TRANSPARENT);
    char buffer[MAX_STRING];
    wsprintf(buffer, "%s.", pW->winName);
    DrawText(hdc, buffer, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
}

void InitHandlerDrawYX()
{
    brushes[ZONE_BRUSH] = CreateSolidBrush(DRAW_ZONE_COLOR);  // Brush for chart drawings zone.

    const COLORREF* linesColors = pW->linesColors;  // Get user colors for line from input structure, or use defaults.
    if (!linesColors)
    {
        linesColors = &DRAW_LINE_COLORS[0];
    }

    pens[FRAME_PEN] = CreatePen(PS_SOLID, 0, DRAW_FRAME_COLOR);   // Pen for chart frame.
    for (int i = 0; i < LINES_SUPPORTED; i++)                     // Pens for functions charts lines.
    {
        pens[i + 1] = CreatePen(PS_SOLID, 2, linesColors[i]);
    }

    static LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    lf.lfItalic = TRUE;
    lf.lfWeight = FW_MEDIUM;
    lf.lfHeight = 16;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    fonts[GRAD_FONT] = CreateFontIndirect(&lf);  // Font for numbers, X,Y axis graduation.

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lf.lfItalic = TRUE;
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = 17;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    fonts[AXIS_FONT] = CreateFontIndirect(&lf);  // Font for X,Y values (axis names), for example X=KB, Y=GBPS at Bandwidth=F(Block size).

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    // lf.lfItalic = TRUE;
    lf.lfWeight = FW_NORMAL;
    lf.lfHeight = 16;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Courier");   // https://en.wikipedia.org/wiki/List_of_monospaced_typefaces
    fonts[STAT_FONT] = CreateFontIndirect(&lf);  // Font for statistics texts: min, max, average, median. Required monospace font.

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    // lf.lfItalic = TRUE;
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = 16;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Courier");   // https://en.wikipedia.org/wiki/List_of_monospaced_typefaces
    fonts[FUNC_FONT] = CreateFontIndirect(&lf);  // Font for functions names at statistics texts.

    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfPitchAndFamily = FIXED_PITCH;
    // lf.lfItalic = TRUE;
    lf.lfWeight = FW_THIN;
    lf.lfHeight = 19;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpy((LPSTR)&lf.lfFaceName, "Verdana");
    fonts[DOWN_FONT] = CreateFontIndirect(&lf);  // Font for down strings, drawings description.
}

void DeInitHandlerDrawYX()
{
    for (int i = 0; i < MAX_BRUSH; i++)
    {
        DeleteObject(brushes[i]);
    }
    for (int i = 0; i < MAX_PEN; i++)
    {
        DeleteObject(pens[i]);
    }
    for (int i = 0; i < MAX_FONT; i++)
    {
        DeleteObject(fonts[i]);
    }
}

void PaintHandlerDrawYX(HDC hdc, RECT rc)
{
    // Set number of Y=F(X) chart lines, input parameter limited by LINES_SUPPORTED constant.
    int linesCount = pF->numberOfFunctions;
    if (linesCount > LINES_SUPPORTED)
    {
        linesCount = LINES_SUPPORTED;
    }

    // Pre-clear statistics array.
    for (int i = 0; i < LINES_SUPPORTED; i++)
    {
        fnStat[i].min = 0;
        fnStat[i].max = 0;
        fnStat[i].average = 0;
        fnStat[i].median = 0;
    }

    // Calculate statistics (min, max, average, median).
    for (int i = 0; i < linesCount; i++)
    {
        calculateStatistics(*(pF->vY[i]), fnStat[i].min, fnStat[i].max, fnStat[i].average, fnStat[i].median);
    }

    // Calibrate X-axis.
    std::vector<double>::iterator imax = max_element((*(pF->vX)).begin(), (*(pF->vX)).end());
    double max = *imax;
    long long lmax = static_cast<long long>(max);
    while (lmax % 10) { lmax++; }
    long long fGridUnitX = lmax / 10;
    double targetX = static_cast<double>(lmax + fGridUnitX);

    // Calibrate Y-axis.
    max = 0.0;
    for (int i = 0; i < 3; i++)
    {
        double a = fnStat[i].max;
        if (a > max) max = a;
    }
    lmax = static_cast<long long>(max);
    while (lmax % 10) { lmax++; }
    long long fGridUnitY = lmax / 10;
    double targetY = static_cast<double>(lmax + fGridUnitY);

    // Calculate variables for display coordinates translation.
    const int dX = rc.right - rc.left;
    const int dY = rc.bottom - rc.top;
    const int x1 = SX1;
    const int y1 = SY1;
    const int x2 = dX - SX2;
    const int y2 = dY - SY2;
    const int rX = (x2 - x1) / (pF->fGridX) * (pF->fGridX);  // This for stabilization last X-interval size.
    const int rY = (y2 - y1) / (pF->fGridY) * (pF->fGridY);  // This for stabilization last Y-interval size.

    // Set display coordinates translation.
    SetMapMode(hdc, MM_ISOTROPIC);
    SetWindowExtEx(hdc, dX, dY, NULL);
    SetViewportExtEx(hdc, dX, -dY, NULL);
    SetViewportOrgEx(hdc, x1, y2, NULL);

    // Change display device context parameters, save original values for restore.
    HBRUSH hOldBrush = static_cast<HBRUSH>(SelectObject(hdc, brushes[ZONE_BRUSH]));
    HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc, pens[FRAME_PEN]));
    HFONT hOldFont = static_cast<HFONT>(SelectObject(hdc, fonts[GRAD_FONT]));
    SetTextColor(hdc, AXIS_GRAD_COLOR);
    SetBkMode(hdc, TRANSPARENT);

    // Draw rectangle for chart zone.
    Rectangle(hdc, 0, 0, rX, rY);  // Rectangle color = current selected brush, border = current selected pen.

    // Transit buffer for text strings write.
    char buffer[MAX_STRING];

    // Write graduation numeric labels for X-axis.
    int gridStepX = rX / pF->fGridX;
    int gridX = 0;
    long long gridValueX = 0;
    for (int i = 0; i < pF->fGridX; i++)
    {
        MoveToEx(hdc, gridX, 0, nullptr);
        LineTo(hdc, gridX, rY);
        snprintf(buffer, MAX_STRING, "%llu", gridValueX);
        SIZE textSize;
        GetTextExtentPoint32(hdc, buffer, static_cast<int>(strlen(buffer)), &textSize);
        if ((textSize.cx < gridStepX) || ((i % 2 == 0) && (i != pF->fGridX - 1)))
        {
            TextOut(hdc, gridX - 6, -2, buffer, static_cast<int>(strlen(buffer)));
        }
        gridX += gridStepX;
        gridValueX += fGridUnitX;
    }

    // Calculate pixels per X-value at input function vector.
    double pixelsPerX = static_cast<double>(gridX) / (targetX);

    // Write graduation numeric labels for Y-axis.
    int gridStepY = rY / pF->fGridY;
    int gridY = gridStepY;
    long long gridValueY = fGridUnitY;
    for (int i = 0; i < (pF->fGridY - 1); i++)
    {
        MoveToEx(hdc, 0, gridY, nullptr);
        LineTo(hdc, rX, gridY);
        snprintf(buffer, MAX_STRING, "%llu", gridValueY);
        SIZE textSize;
        GetTextExtentPoint32(hdc, buffer, static_cast<int>(strlen(buffer)), &textSize);
        TextOut(hdc, -textSize.cx - 5, gridY + 8, buffer, static_cast<int>(strlen(buffer)));
        gridY += gridStepY;
        gridValueY += fGridUnitY;
    }

    // Calculate pixels per Y-value at input function vector.
    double pixelsPerY = static_cast<double>(gridY) / (targetY);

    // Write units names for X and Y=F(X) at X and Y axises.
    SelectObject(hdc, fonts[AXIS_FONT]);
    SetTextColor(hdc, UNIT_NAME_COLOR);
    TextOut(hdc, gridX + 3, -1, pF->fUnitNameX, static_cast<int>(strlen(pF->fUnitNameX)));
    SIZE textSize;
    GetTextExtentPoint32(hdc, pF->fUnitNameY, static_cast<int>(strlen(pF->fUnitNameY)), &textSize);
    TextOut(hdc, -textSize.cx - 7, gridY + 8, pF->fUnitNameY, static_cast<int>(strlen(pF->fUnitNameY)));

    // Select colors for function charts: user defined or defaults.
    const COLORREF* linesColors = pW->linesColors;
    if (!linesColors)
    {
        linesColors = &DRAW_LINE_COLORS[0];
    }

    // Write functions charts, maximum 3 functions.
    HPEN hPens[LINES_SUPPORTED];
    for (int i = 0; i < linesCount; i++)
    {
        hPens[i] = CreatePen(PS_SOLID, 2, linesColors[i]);
        SelectObject(hdc, hPens[i]);
        MoveToEx(hdc, static_cast<int>((*(pF->vX))[0] * pixelsPerX), static_cast<int>((*(pF->vY[i]))[0] * pixelsPerY), NULL);
        for (int j = 0; j < pF->numberOfValues; j++)
        {
            LineTo(hdc, static_cast<int>((*(pF->vX))[j] * pixelsPerX), static_cast<int>((*(pF->vY[i]))[j] * pixelsPerY));
        }
    }

    // Write statistics strings (min, max, average, median) for all functions, maximum 3 functions.
    int shiftY = 6;
    for (int i = 0; i < linesCount; i++)
    {
        SetTextColor(hdc, linesColors[i]);
        SelectObject(hdc, fonts[FUNC_FONT]);
        TextOut(hdc, gridX + 13, gridY + shiftY, pF->functionsNames[i], static_cast<int>(strlen(pF->functionsNames[i])));
        SelectObject(hdc, fonts[STAT_FONT]);
        snprintf(buffer, MAX_STRING, MIN_STAT, fnStat[i].min);
        TextOut(hdc, gridX + 13, gridY - 16 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, MAX_STAT, fnStat[i].max);
        TextOut(hdc, gridX + 13, gridY - 32 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, AVERAGE_STAT, fnStat[i].average);
        TextOut(hdc, gridX + 13, gridY - 48 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        snprintf(buffer, MAX_STRING, MEDIAN_STAT, fnStat[i].median);
        TextOut(hdc, gridX + 13, gridY - 64 + shiftY, buffer, static_cast<int>(strlen(buffer)));
        shiftY -= 96;
    }

    // Write 2 down strings, comment about this drawings.
    SetTextColor(hdc, DOWN_STRINGS_COLOR);
    SelectObject(hdc, fonts[DOWN_FONT]);
    TextOut(hdc, -50, -30, pF->fDownString1, static_cast<int>(strlen(pF->fDownString1)));
    TextOut(hdc, -50, -46, pF->fDownString2, static_cast<int>(strlen(pF->fDownString2)));

    // Restore display device context parameters: default font, default pen, default brush.
    SelectObject(hdc, hOldFont);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
}

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
