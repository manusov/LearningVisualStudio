/*

Class header for draw functions Y=F(X).
UNDER CONSTRUCTION.

*/

#pragma once
#ifndef DRAWYX_H
#define DRAWYX_H

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

// #define _DEFAULT_TEMPLATE_MODE

constexpr int MAX_STRING = 80;

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

class DrawYX
{
public:
	DrawYX( WINDOW_PARMS* pW, FUNCTION_PARMS* pF);
	HWND GetHwnd();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static void PaintHandlerDefault(HDC hdc, RECT rc);
	static void InitHandlerDrawYX();
	static void DeInitHandlerDrawYX();
	static void PaintHandlerDrawYX(HDC hdc, RECT rc);
	static void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);

	static constexpr int LINES_SUPPORTED = 3;

	static WINDOW_PARMS* pW;
	static FUNCTION_PARMS* pF;

	static HWND hWnd;
	static FUNCTION_STATISTICS functionStatistics[LINES_SUPPORTED];
	static HBRUSH brushes[MAX_BRUSH];
	static HPEN pens[MAX_PEN];
	static HFONT fonts[MAX_FONT];

	static constexpr int MAX_STRING = 80;
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
};

#endif  // DRAWYX_H
