/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Implementation of timer functions class.
Measure frequencies for:
OS Performance Counter, OS File Time counter, TSC.

*/

#include "Timer.h"

BOOL Timer::precisionMeasure(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc)
{
	BOOL status = FALSE;
	LARGE_INTEGER c1;
	LARGE_INTEGER c2;
	if (QueryPerformanceFrequency(&hzPc))  // Get reference frequency.
	{
		if (QueryPerformanceCounter(&c1))
		{
			c2.QuadPart = c1.QuadPart;
			while (c1.QuadPart == c2.QuadPart)
			{
				status = QueryPerformanceCounter(&c2);  // Wait for first timer change, for synchronization.
				if (!status) break;
			}
			if (status)
			{
				hzTsc.QuadPart = __rdtsc();
				c1.QuadPart = c2.QuadPart + hzPc.QuadPart;
				while (c2.QuadPart < c1.QuadPart)
				{
					status = QueryPerformanceCounter(&c2);  // Wait for increments count per 1 second.
					if (!status) break;
				}
				hzTsc.QuadPart = __rdtsc() - hzTsc.QuadPart;
			}
		}
	}
	return status;
}
BOOL Timer::precisionResolution(LARGE_INTEGER& min, LARGE_INTEGER& max)
{
	LARGE_INTEGER c1;
	LARGE_INTEGER c2;
	min.QuadPart = LLONG_MAX;
	max.QuadPart = 0;
	BOOL status = QueryPerformanceCounter(&c1);
	for (int i = 0; (i < 10) && status; i++)
	{
		c2.QuadPart = c1.QuadPart;
		while ((c1.QuadPart == c2.QuadPart) && status)
		{
			status = QueryPerformanceCounter(&c1);  // Wait for timer change.
		}
		LONGLONG delta = c1.QuadPart - c2.QuadPart;
		if (delta < min.QuadPart) min.QuadPart = delta;
		if (delta > max.QuadPart) max.QuadPart = delta;
	}
	return status;
}
void Timer::legacyMeasure(LARGE_INTEGER& hzFt, LARGE_INTEGER& hzTsc)
{
	T64 ft1;
	T64 ft2;
	hzFt.QuadPart = 10000000;                  // File time reference frequency is const = 10MHz.
	GetSystemTimeAsFileTime(&ft1.fileTime);
	ft2 = ft1;
	while (ft1.raw == ft2.raw)
	{
		GetSystemTimeAsFileTime(&ft2.fileTime);  // Wait for first timer change, for synchronization.
	}
	hzTsc.QuadPart = __rdtsc();
	ft1.raw = ft2.raw + hzFt.QuadPart;
	while (ft2.raw < ft1.raw)
	{
		GetSystemTimeAsFileTime(&ft2.fileTime);  // Wait for increments count per 1 second.
	}
	hzTsc.QuadPart = __rdtsc() - hzTsc.QuadPart;
}
void Timer::legacyResolution(LARGE_INTEGER& min, LARGE_INTEGER& max)
{
	T64 ft1;
	T64 ft2;
	min.QuadPart = LLONG_MAX;
	max.QuadPart = 0;
	GetSystemTimeAsFileTime(&ft1.fileTime);
	for (int i = 0; i < 10; i++)
	{
		ft2.raw = ft1.raw;
		while (ft1.raw == ft2.raw)
		{
			GetSystemTimeAsFileTime(&ft1.fileTime);  // Wait for timer change.
		}
		LONGLONG delta = ft1.raw - ft2.raw;
		if (delta < min.QuadPart) min.QuadPart = delta;
		if (delta > max.QuadPart) max.QuadPart = delta;
	}
}
int Timer::test()
{
	char msg[APPCONST::MAX_TEXT_STRING];
	int exitCode = 0;
	LARGE_INTEGER x1;
	LARGE_INTEGER x2;
	LARGE_INTEGER min;
	LARGE_INTEGER max;
	const char* tableForm = " %-14llu%-14llu%-13.1f%-14llu%-13.1f\r\n";

	// Start test OS Performance Counter.
	AppLib::writeColor("\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (precisionMeasure(x1, x2))
	{
		double perfMhz = static_cast<double>(x1.QuadPart) / 1E6;
		double perfNs = 1E9 / static_cast<double>(x1.QuadPart);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "[OS performance counter: F=%.3fMHz, T=%.3fns.]\r\n", perfMhz, perfNs);
		AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
		double tscMhz = static_cast<double>(x2.QuadPart) / 1E6;
		double tscNs = 1E9 / static_cast<double>(x2.QuadPart);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "[Measure TSC by OS performance counter: F=%.3fMHz, T=%.3fns.]\r\n", tscMhz, tscNs);
		AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
		AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
		AppLib::writeColor(" dTSC, clks    dPC min       ns           dPC max       ns\r\n", APPCONST::TABLE_COLOR);
		AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);

		// Test stability of OS Performance Counter, 10 measurements.
		for (int i = 0; i < 10; i++)
		{
			if ((precisionMeasure(x1, x2)) && (precisionResolution(min, max)))
			{
				double minNs = static_cast<double>(min.QuadPart) * perfNs;
				double maxNs = static_cast<double>(max.QuadPart) * perfNs;
				snprintf(msg, APPCONST::MAX_TEXT_STRING, tableForm,
					x2.QuadPart, min.QuadPart, minNs, max.QuadPart, maxNs);
				AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
			}
			else
			{
				AppLib::writeColor("Measurement failed.\r\n", APPCONST::ERROR_COLOR);
				exitCode = 2;
			}
		}
	}
	else
	{
		AppLib::writeColor("OS performance counter not detected or failed.\r\n", APPCONST::ERROR_COLOR);
		exitCode = 1;
	}

	// Start test OS File Time.
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	legacyMeasure(x1, x2);
	double ftMhz = static_cast<double>(x1.QuadPart) / 1E6;
	double ftNs = 1E9 / static_cast<double>(x1.QuadPart);
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "[OS file time: F=%.3fMHz, T=%.3fns.]\r\n", ftMhz, ftNs);
	AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
	double tscFtMhz = static_cast<double>(x2.QuadPart) / 1E6;
	double tscFtNs = 1E9 / static_cast<double>(x2.QuadPart);
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "[Measure TSC by OS file time: F=%.3fMHz, T=%.3fns.]\r\n", tscFtMhz, tscFtNs);
	AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" dTSC, clks    dFT min       ns           dFT max       ns\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);

	// Test stability of OS File Time, 10 measurements.
	for (int i = 0; i < 10; i++)
	{
		legacyMeasure(x1, x2);
		legacyResolution(min, max);
		double minNs = static_cast<double>(min.QuadPart) * ftNs;
		double maxNs = static_cast<double>(max.QuadPart) * ftNs;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, tableForm,
			x2.QuadPart, min.QuadPart, minNs, max.QuadPart, maxNs);
		AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
	}

	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	return exitCode;
}
