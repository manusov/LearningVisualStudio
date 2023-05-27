/*
Header of timer functions class.
Measure frequencies for:
OS Performance Counter, OS File Time counter, TSC.
*/

#pragma once
#ifndef TIMER_H
#define TIMER_H

#include <windows.h>
#include "AppLib.h"

union T64
{
	FILETIME fileTime;
	DWORD64 raw;
};

class Timer
{
public:
	Timer();
	BOOL precisionMeasure(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc);
	BOOL precisionResolution(LARGE_INTEGER& min, LARGE_INTEGER& max);
	void legacyMeasure(LARGE_INTEGER& hzFt, LARGE_INTEGER& hzTsc);
	void legacyResolution(LARGE_INTEGER& min, LARGE_INTEGER& max);
	int test();
};

#endif  // TIMER_H
