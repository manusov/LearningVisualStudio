/*
Interrogate threads running timings by TSC.
*/

#include "Task_12.h"

constexpr int MEASUREMENT_REPEATS = 500;  // measurement cycle for get last result, dramatically faster than one result

int Task_12::execute(int argc, char** argv)
{
	int exitCode = 0;
	char msg[APPCONST::MAX_TEXT_STRING];
	size_t N = std::thread::hardware_concurrency();
	unsigned int* ids = new unsigned int[N];
	DWORD64* timePoints = new DWORD64[N];

	if (ids && timePoints)
	{
		indexPtrStore = ids;
		timePtrStore = timePoints;
		memset(ids, 0, N * sizeof(int));
		memset(timePoints, 0, N * sizeof(DWORD64));

		Timer timer;
		LARGE_INTEGER hzPc;
		LARGE_INTEGER hzTsc;
		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double perfMhz = static_cast<double>(hzPc.QuadPart) / 1E6;
			double perfNs = 1E9 / static_cast<double>(hzPc.QuadPart);
			double tscMhz = static_cast<double>(hzTsc.QuadPart) / 1E6;
			double tscNs = 1E9 / static_cast<double>(hzTsc.QuadPart);
			double msTsc = 1E3 / hzTsc.QuadPart;

			snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nOS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", perfMhz, perfNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Measure TSC by OS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", tscMhz, tscNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%zd hardware threads.\r\n", N);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

			DWORD64 baseTsc = 0;
			DWORD64 dtsc = 0;
			for (int j = 0; j < MEASUREMENT_REPEATS; j++)  // measurement cycle for get last result, dramatically faster than one result
			{
				int dummy[4];
				__cpuid(dummy, 0);  // for serialization only
				_mm_mfence();
				baseTsc = __rdtsc();
				std::vector<std::thread> threads = {};
				for (size_t i = 0; i < N; i++)
				{
					void* threadPtr = reinterpret_cast<void*>(i);
					threads.push_back(std::thread(threadFunction, threadPtr));
				}
				for (size_t i = 0; i < N; i++)
				{
					threads[i].join();
				}
				__cpuid(dummy, 0);  // for serialization only
				_mm_mfence();
				dtsc = __rdtsc() - baseTsc;
			}

			AppLib::writeLine(24);
			for (int i = 0; i < N; i++)
			{
				double delay = (timePoints[i] - baseTsc) * msTsc;
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "id[%-2d]  %-2d  %.3f ms\r\n", i, ids[i], delay);
				AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
			}
			AppLib::writeLine(24);

			double clocks = static_cast<double>(dtsc);
			double msWork = (clocks * msTsc);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Last iteration work time %.1f clocks = %.3f ms.\r\n", clocks, msWork);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		}
		else
		{
			AppLib::writeParmError("TSC clock measurement failed");
			exitCode = 2;
		}
	}
	else
	{
		AppLib::writeParmError("memory allocation failed");
		exitCode = 1;
	}

	delete[] ids;
	delete[] timePoints;
	indexPtrStore = ids = nullptr;
	timePtrStore = timePoints = nullptr;
	return exitCode;
}
void Task_12::threadFunction(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	unsigned int* p1 = indexPtrStore + i;
	DWORD64* p2      = timePtrStore + i;
	DWORD_PTR mask = 1LL << i;
	SetThreadAffinityMask(GetCurrentThread(), mask);
	*p2 = __rdtscp(p1);
}
unsigned int* Task_12::indexPtrStore;
DWORD64* Task_12::timePtrStore;