/*

Application library class implementation.
Class used for GUI support helpers functions.
UNDER CONSTRUCTION.

*/

#include "AppGuiLib.h"

#if _WIN64
const char* const WIN_NAME = "Draw function Y=F(X) v0.01.02 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Draw function Y=F(X) v0.01.02 (ia32)";
#endif

void AppGuiLib::testWindow1()
{
    // Argument and functions vectors.
    std::vector<double> vX;
    std::vector<double> vy1;
    std::vector<double> vy2;
    std::vector<double> vy3;
    std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

    // Constants.
    constexpr int N_F = 101;
    constexpr int GRID_X = 11;
    constexpr int GRID_Y = 11;
    const char* functionsNamesTest[3] = { "Y=X", "Y=5X", "Y=X^2+15" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_F; i++)
    {
        double a = static_cast<double>(i);
        vX.push_back(a);
        vY[0]->push_back(a);
        vY[1]->push_back(5 * a);
        vY[2]->push_back(a * a + 15);
    }

    // Setup chart window parameters.
    WINDOW_PARMS wP;
    wP.hInstance = GetModuleHandle(NULL);  // hInstance;
    wP.nCmdShow = SW_SHOWNORMAL;           // nCmdShow;
    wP.linesColors = nullptr;              // Default colors for lines.
    wP.winName = WIN_NAME;

    // Setup chart function parameters.
    FUNCTION_PARMS fP;
    fP.numberOfFunctions = 3;
    fP.numberOfValues = N_F;
    fP.fGridX = GRID_X;
    fP.fGridY = GRID_Y;
    fP.fUnitNameX = "X";
    fP.fUnitNameY = "F(X)";
    fP.fDownString1 = "This is engineering sample for draw functions Y=F(X).";
    fP.fDownString2 = "Variant for 3 math functions.";
    fP.functionsNames = functionsNamesTest;
    fP.vX = &vX;
    fP.vY = &vY[0];

    // Create and show GUI window with functions chart.
    helperWindow(wP, fP);
}
void AppGuiLib::testWindow2()
{
    // Argument and functions vectors.
    std::vector<double> vX;
    std::vector<double> vy1;
    std::vector<double> vy2;
    std::vector<double> vy3;
    std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

    // Constants.
    constexpr int N_F = 1001;
    constexpr int GRID_X = 11;
    constexpr int GRID_Y = 11;
    const char* functionsNamesTest[3] = { "Y=20+10*sin(X)", "Y=20+10*cos(X)", "Y=combined" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_F; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back(20 + 10 * sin(a));
        vY[1]->push_back(20 + 10 * cos(a));
        vY[2]->push_back(30 + 10 * sin(a) + cos(20 * a));
    }

    // Setup chart window parameters.
    WINDOW_PARMS wP;
    wP.hInstance = GetModuleHandle(NULL);  // hInstance;
    wP.nCmdShow = SW_SHOWNORMAL;           // nCmdShow;
    wP.linesColors = nullptr;              // Default colors for lines.
    wP.winName = WIN_NAME;

    // Setup chart function parameters.
    FUNCTION_PARMS fP;
    fP.numberOfFunctions = 3;
    fP.numberOfValues = N_F;
    fP.fGridX = GRID_X;
    fP.fGridY = GRID_Y;
    fP.fUnitNameX = "X";
    fP.fUnitNameY = "F(X)";
    fP.fDownString1 = "This is engineering sample for draw functions Y=F(X).";
    fP.fDownString2 = "Variant for 3 math functions.";
    fP.functionsNames = functionsNamesTest;
    fP.vX = &vX;
    fP.vY = &vY[0];

    // Create and show GUI window with functions chart.
    helperWindow(wP, fP);
}
void AppGuiLib::helperWindow(WINDOW_PARMS& wP, FUNCTION_PARMS& fP)
{
    DrawYX draw = DrawYX(&wP, &fP);
    HWND hWnd = draw.GetHwnd();

    if (!hWnd)
    {
        constexpr int MAX_STRING = 80;
        char buffer[MAX_STRING];
        wsprintf(buffer, "Cannot create window: %s.", WIN_NAME);
        MessageBox(NULL, buffer, "Error", MB_OK);
    }
    else
    {
        // ShowWindow(hWnd, nCmdShow);
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
        MSG Msg;
        while (GetMessage(&Msg, NULL, 0, 0))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }
}