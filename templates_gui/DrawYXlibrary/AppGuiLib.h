/*

Application library class header.
Class used for GUI support helpers functions.
UNDER CONSTRUCTION.

*/

#pragma once
#ifndef APPGUILIB_H
#define APPGUILIB_H

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>

#if _WIN64
const char* const WIN_NAME = "Draw Y=F(X) v0.05.03 (x64)";
#elif _WIN32
const char* const WIN_NAME = "Draw Y=F(X) v0.05.03 (ia32)";
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

class AppGuiLib
{
public:
	static BOOL FunctionDraw(WINDOW_PARMS* pW, FUNCTION_PARMS* pF);
	static BOOL FunctionDraw(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow,
		std::vector<double>* vX, std::vector<double>** vY, int nF, const char** fNames, 
		WINDOW_PARMS* pW = nullptr, FUNCTION_PARMS* pF = nullptr);

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void PaintHandlerDefault(HDC hdc, RECT rc);
	static void InitHandlerDrawYX();
	static void DeInitHandlerDrawYX();
	static void PaintHandlerDrawYX(HDC hdc, RECT rc);
	static void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);

	static constexpr int MAX_STRING = 80;
	static constexpr int LINES_SUPPORTED = 3;
	static constexpr int WINDOW_BASE_X = 100;
	static constexpr int WINDOW_BASE_Y = 120;
	static constexpr int WINDOW_SIZE_X = 580;
	static constexpr int WINDOW_SIZE_Y = 480;
	static constexpr int WINDOW_MIN_X = 500;
	static constexpr int WINDOW_MIN_Y = 400;
	static constexpr int SX1 = 65;
	static constexpr int SX2 = 170;
	static constexpr int SY1 = 5;
	static constexpr int SY2 = 75;
	static constexpr int GRID_X = 11;
	static constexpr int GRID_Y = 11;

	static constexpr COLORREF DRAW_ZONE_COLOR = RGB(254, 254, 235);
	static constexpr COLORREF DRAW_FRAME_COLOR = RGB(195, 195, 195);
	static constexpr COLORREF AXIS_GRAD_COLOR = RGB(1, 1, 1);
	static constexpr COLORREF UNIT_NAME_COLOR = RGB(3, 3, 150);
	static constexpr COLORREF DRAW_LINE_COLORS[LINES_SUPPORTED] = { RGB(254, 10, 10), RGB(10, 160, 10), RGB(10, 10, 254) };
	static constexpr COLORREF DOWN_STRINGS_COLOR = RGB(1, 1, 1);

	static const char* const szClassName;
	static const char* const MIN_STAT;
	static const char* const MAX_STAT;
	static const char* const AVERAGE_STAT;
	static const char* const MEDIAN_STAT;

	static WINDOW_PARMS wP;
	static FUNCTION_PARMS fP;

	static HWND hWnd;
	static FUNCTION_STATISTICS fnStat[LINES_SUPPORTED];
	static HBRUSH brushes[MAX_BRUSH];
	static HPEN pens[MAX_PEN];
	static HFONT fonts[MAX_FONT];
};

#endif  // APPGUILIB_H