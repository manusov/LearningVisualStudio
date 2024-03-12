/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

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
	static BOOL precisionMeasure(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc);
	static BOOL precisionResolution(LARGE_INTEGER& min, LARGE_INTEGER& max);
	static void legacyMeasure(LARGE_INTEGER& hzFt, LARGE_INTEGER& hzTsc);
	static void legacyResolution(LARGE_INTEGER& min, LARGE_INTEGER& max);
	static int test();
};

#endif  // TIMER_H

