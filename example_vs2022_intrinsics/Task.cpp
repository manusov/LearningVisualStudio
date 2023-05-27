/*
Target functionality test routine.
*/

#include "Task.h"

int task()
{
	int exitCode = 0;
	if (detectAvx512())
	{
		constexpr DWORD64 BLOCK_SIZE = 32768;
		constexpr DWORD64 BLOCK_ALIGN = 64;
		constexpr DWORD64 REPEATS = 10000000;
		DWORD64 dTSC = 0;
		DWORD64 sumTSC = 0;
		double dummyData = 0;
		double dummySum = 0;
		LARGE_INTEGER hzPc;
		LARGE_INTEGER hzTsc;

		char msg[APPCONST::MAX_TEXT_STRING];
		Timer timer;
		Performance performance;

		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double clockTime = 1.0 / static_cast<double>(hzTsc.QuadPart);
			double megabytes = static_cast<double>(BLOCK_SIZE * REPEATS) / 1E6;

			void* ptr = _aligned_malloc(BLOCK_SIZE, BLOCK_ALIGN);
			if (ptr)
			{
				memset(ptr, 0, BLOCK_SIZE);  // This actual if "Debug" build, array not zeroed.
				*(reinterpret_cast<double*>(ptr)) = 1.0;  // This for verify cycles.
				for (DWORD64 i = 0; i < REPEATS; i++)
				{   // This call can be removed by optimization. Force get result by dummyData variable.

					// __debugbreak();
					performance.addAvx512(ptr, BLOCK_SIZE, dTSC, dummyData);

					sumTSC += dTSC;
					dummySum += dummyData;
				}
				_aligned_free(ptr);

				double seconds = clockTime * sumTSC;
				double mbps = megabytes / seconds;
				snprintf(msg, APPCONST::MAX_TEXT_STRING,
					"Delta TSC=%llu, MBPS=%.1f, dummy=%.3f.\r\n", sumTSC, mbps, dummySum);
				AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
			}
			else
			{
				exitCode = 1;
			}
		}
		else
		{
			exitCode = 2;
		}
	}
	else
	{
		AppLib::writeColor("AVX512 or OS ZMM-context management is not supported.", APPCONST::ERROR_COLOR);
		exitCode = 3;
	}
	return exitCode;
}

BOOL detectAvx512()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 7)
	{
		__cpuid(regs, 1);
		constexpr int MASK_AVX256 = (1 << 27) | (1 << 28);
		if ((regs[2] & MASK_AVX256) == MASK_AVX256)
		{
			__cpuidex(regs, 7, 0);
			constexpr int MASK_AVX512 = 1 << 16;
			if ((regs[1] & MASK_AVX512) == MASK_AVX512)
			{
				DWORD64 xcr0 = _xgetbv(0);
				constexpr DWORD64 MASK_XCR0 = 0xE6;
				if ((xcr0 & MASK_XCR0) == MASK_XCR0)
				{
					status = TRUE;
				}
			}
		}
	}
	return status;
}
