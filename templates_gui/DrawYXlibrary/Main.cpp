/*

Template for C++ GUI application.
Draw function Y=F(X).
Variant without Window Class (KWnd) but with Application GUI library.

TODO.
Overload AppGuiLib::FunctionDraw(), required variant with skip auto-calibration X, Y axies.

Override parameters groups:
1) fnStat                  : override statistics if fnStat = nullptr.
2) fGridUnitX, targetX     : override X if fGridUnitX = 0.
3) fGridUnitY, targetY     : override Y if fGridUnitY = 0.
4) fUnitNameX              : override X units name if fUnitNameX = nullptr.
5) fUnitNameY              : override Y units name if fUnitNameY = nullptr.
6) linesColors             : override function colors if p = nullptr.
7) Inspect structures WINDOW_PARMS, FUNCTION_PARMS for unused or non-optimal used members.
8) Use regular method: structures WINDOW_PARMS, FUNCTION_PARMS for all parameters.
9) Alternative entry point: BOOL FunctionDraw(WINDOW_PARMS* pW, UNCTION_PARMS* pF).

*/

#include <windows.h>
#include <vector>
#include "AppGuiLib.h"

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // ---------- 2 variants with default structures WINDOW_PARMS, FUNCTION_PARMS inside function. ----------
 
    // Argument and functions vectors.
    std::vector<double> vX;
    std::vector<double> vy1;
    std::vector<double> vy2;
    std::vector<double> vy3;
    std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

    // Constants.
    int N_FV = 101;  //  Number of points (function values).
    const char* functionsNames1[3] = { "Y=X", "Y=5X", "Y=X^2+500" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_FV; i++)
    {
        double a = static_cast<double>(i);
        vX.push_back(a);
        vY[0]->push_back(a);
        vY[1]->push_back(5 * a);
        vY[2]->push_back(a * a + 500);
    }

    if (!AppGuiLib::FunctionDraw(hInstance, hPrevInstance, lpCmdLine, nCmdShow, &vX, vY, N_FV, functionsNames1))
    {
        return 1;
    }

    // Clear argument and functions vectors for second draw.
    vX.clear();
    vy1.clear();
    vy2.clear();
    vy3.clear();

    // Constants.
    N_FV = 1001;  // Number of points.
    const char* functionsNames2[3] = { "Y=20+10*sin(X)", "Y=20+10*cos(X)", "Y=combined" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_FV; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back(20 + 10 * sin(a));
        vY[1]->push_back(20 + 10 * cos(a));
        vY[2]->push_back(30 + 10 * sin(a) + cos(20 * a));
    }

    Sleep(400);
    if (!AppGuiLib::FunctionDraw(hInstance, hPrevInstance, lpCmdLine, nCmdShow, &vX, vY, N_FV, functionsNames2))
    {
        return 1;
    }

    // ---------- 1 variant with custom structures WINDOW_PARMS, FUNCTION_PARMS provided by caller. ----------
     
    // Clear argument and functions vectors for second draw.
    vX.clear();
    vy1.clear();
    vy2.clear();
    vy3.clear();

    // Constants.
    N_FV = 1001;  // Number of points.
    const char* functionsNames3[3] = { "Y=X/2", "Y=sqrt(10*X)", "Y=sqrt(20*X)" };

    // Generating arguments and functions vectors: X, Y1(X), Y2(X), Y3(X). This is measurement results emulation for debug.
    for (int i = 0; i < N_FV; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back(a / 2.0);
        vY[1]->push_back(sqrt(10 * a));
        vY[2]->push_back(sqrt(20 * a));
    }

    WINDOW_PARMS uWp;
    FUNCTION_PARMS uFp;

    constexpr COLORREF USER_LINE_COLORS[3] = { RGB(80, 80, 80), RGB(140, 140, 140), RGB(190, 190, 190) };
    
    uWp.hInstance = hInstance;                  // Module handle, GetModuleHandle(NULL);
    uWp.nCmdShow = nCmdShow;                    // GUI window show mode, nCmdShow;  SW_SHOWNORMAL;
    uWp.linesColors = &USER_LINE_COLORS[0];     // User-defined colors for lines.
    uWp.winName = WIN_NAME;                     // Window name, up (capture) string.

    int GRID_X = 11;
    int GRID_Y = 11;
    int NUMBER_OF_FUNCTIONS = 3;

    uFp.numberOfFunctions = NUMBER_OF_FUNCTIONS;  // Number of functions, lines Y=F(X).
    uFp.numberOfValues = N_FV;                    // Number of values (function points).
    uFp.fGridX = GRID_X;                          // Total number of grid intervals per X axis.
    uFp.fGridY = GRID_Y;                          // Total number of grid intervals per Y axis.
    uFp.fUnitNameX = "X";                         // Argument X or it units name, for X axis.
    uFp.fUnitNameY = "F(X)";                      // Function Y=F(X) or it units name, for Y axis.
    uFp.fDownString1 = "This is engineering sample for draw functions Y=F(X).";  // First down string.
    uFp.fDownString2 = "Variant for 3 math functions.";                          // Second down string.
    uFp.functionsNames = functionsNames3;         // Names for 3 functions, used at statistics tables.
    uFp.vX = &vX;                                 // Pointer to X vector, argument X.
    uFp.vY = vY;                                  // Pointer to array of up to 3 Y vectors, functions Y=F(X).

    Sleep(400);
    if (!AppGuiLib::FunctionDraw(&uWp, &uFp))
    {
        return 1;
    }

    return 0;
}
