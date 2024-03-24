/*

Sorting algorithm debug.
Quick sort and sorting networks with AVX512 instructions.
Starts with variant for double precision numbers.

This project based on Intel sources:
https://github.com/intel/x86-simd-sort
https://github.com/berenger-eu/avx-512-sort/tree/master
Use example at document:
1704.08579.pdf. Appendix B, code 2.
See also:
LIPIcs.SEA.2021.3.pdf.

Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Same template used by: Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories

Application library class implementation.
Class used for GUI support helpers functions.
UNDER CONSTRUCTION.

*/

#include "AppGuiLib.h"

#if _WIN64
const char* const WIN_NAME = "Draw function Y=F(X) template v0.00.03 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Draw function Y=F(X) template v0.00.03 (ia32)";
#endif
const char* functionsNames[3] = { "std::vector", "INTEL AVX512" };

void AppGuiLib::windowFunction(size_t startCount, size_t end_count, size_t deltaCount, std::vector<double> vY1, std::vector<double> vY2)
{

    std::vector<double> vX;
    std::vector<double>* vY[2];
    vY[0] = &vY1;
    vY[1] = &vY2;

    int n = vY1.size();
    for (int i = 0; i < n; i++)
    {
        vX.push_back(i);
        vY1[i] = vY1[i] * 1000.0;  // TODO. Auto-scale required.
        vY2[i] = vY2[i] * 1000.0;  // TODO. Auto-scale required.
    }
    vY[0] = &vY1;
    vY[1] = &vY2;

    WINDOW_PARMS wP;
    wP.hInstance = GetModuleHandle(NULL);  // hInstance;
    wP.nCmdShow = SW_SHOWNORMAL;           // nCmdShow;
    wP.winName = WIN_NAME;

    FUNCTION_PARMS fP;
    fP.numberOfFunctions = 2;
    fP.numberOfValues = n;
    fP.fGridX = n;
    fP.fGridY = 10;
    fP.fGridZeroX = 0;    // TODO. Auto-scale required.
    fP.fGridZeroY = 0;    // TODO. Auto-scale required.
    fP.fGridUnitX = 1;    // TODO. Auto-scale required.
    fP.fGridUnitY = 100;  // TODO. Auto-scale required.
    fP.fUnitNameX = "N";
    fP.fUnitNameY = "ms";
    fP.fDownString1 = "Compare doubles sorting performance:";
    fP.fDownString2 = "std::vector vs INTEL AVX512 algorithm.";
    fP.functionsNames = functionsNames;
    fP.vX = &vX;
    fP.vY = &vY[0];

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
