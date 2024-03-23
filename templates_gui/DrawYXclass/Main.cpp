/*
Template for C++ GUI application.
Draw function Y=F(X).
UNDER CONSTRUCTION.
*/

#include <windows.h>
#include "DrawYX.h"

// Constants for function(s) Y=F(X).
constexpr int GRID_X = 11;
constexpr int GRID_Y = 10;
constexpr long long GRID_ZERO_X = 0;
constexpr long long GRID_ZERO_Y = 0;
constexpr long long GRID_UNIT_X = 1;
constexpr long long GRID_UNIT_Y = 150;
const char* functionsNames[3] = { "Y=200*sin(X)+500", "Y=200*cos(X)+500", "Y=50*cos(3*X)+700" };

// Function Y=F(X).
constexpr int N_F = 1000;
std::vector<double> vX;
std::vector<double> vy1;
std::vector<double> vy2;
std::vector<double> vy3;
std::vector<double>* vY[3] = { &vy1, &vy2, &vy3 };

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    for (int i = 0; i < N_F; i++)
    {
        double a = i / 100.0;
        vX.push_back(a);
        vY[0]->push_back(200 * sin(a) + 500);
        vY[1]->push_back(200 * cos(a) + 500);
        vY[2]->push_back(50 * cos(3 * a) + 700);
    }
    
    WINDOW_PARMS wP;
    wP.hInstance = hInstance;
    wP.nCmdShow = nCmdShow;

    FUNCTION_PARMS fP;
    fP.numberOfFunctions = 3;
    fP.numberOfValues = N_F;
    fP.fGridX = GRID_X;
    fP.fGridY = GRID_Y;
    fP.fGridZeroX = GRID_ZERO_X;
    fP.fGridZeroY = GRID_ZERO_Y;
    fP.fGridUnitX = GRID_UNIT_X;
    fP.fGridUnitY = GRID_UNIT_Y;
    fP.fUnitNameX = "X";
    fP.fUnitNameY = "F(X)";
    fP.fDownString1 = "This is engineering sample for draw functions Y=F(X).";
    fP.fDownString2 = "Variant for 3 math functions.";
    fP.functionsNames = functionsNames;
    fP.vX = &vX;
    fP.vY = &vY[0];

    DrawYX(&wP, &fP);
    return 0;
}



