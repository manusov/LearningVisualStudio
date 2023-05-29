/*
Realization of sine calculation benchmark functions class.
*/

#include "Calc.h"

extern "C" void __stdcall calcSine(void* ptr, DWORD64 count, DWORD64 repeats, DWORD64 & deltaTsc, double x);
void threadFunctionAsm(void* ptr);
void threadFunctionCpp(void* ptr);

constexpr DWORD64 COUNT = 4096;           // 4096 * 8 = 32KB, this means stable cache hits for processor with L1data=48KB.
constexpr DWORD64 UNIT = 8;               // One data unit, double = 8 bytes.
constexpr DWORD64 REPEATS = 10000;        // Repeats for measurement.
constexpr DWORD64 PAGE = 4096;            // Optimal allocation at page bound.
constexpr double NUM = 0.001;

Calc::Calc()
{
}
int Calc::scenarioSingleThread(double& dummySum)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	int exitCode = 0;
	dummySum = 0;
	Timer timer;
	LARGE_INTEGER hzPc;
	LARGE_INTEGER hzTsc;
	void* ptr = _aligned_malloc(COUNT * UNIT, PAGE);
	if (ptr)
	{
		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double fPc = static_cast<double>(hzPc.QuadPart);
			double tPc = 1.0 / fPc;
			double fTsc = static_cast<double>(hzTsc.QuadPart);
			double tTsc = 1.0 / fTsc;
			double fPcMhz = fPc / 1E6;
			double tPcNs = 1E9 / fPc;
			double fTscMhz = fTsc / 1E6;
			double tTscNs = 1E9 / fTsc;

			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance counter : F = %.1f MHz, T = %.3f ns.\r\n", fPcMhz, tPcNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Time stamp counter  : F = %.1f MHz, T = %.3f ns.\r\n", fTscMhz, tTscNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

			DWORD64 deltaTsc = 0;
			calcSine(ptr, COUNT, REPEATS, deltaTsc, NUM);
			double nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT * REPEATS);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Single-thread asm x87 FSIN, nanoseconds = %.3f.\r\n", nanosecondsPerNumber);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT);

			deltaTsc = __rdtsc();
			double a = NUM;
			for (int j = 0; j < REPEATS; j++)
			{
				double* p = reinterpret_cast<double*>(ptr);
				for (int i = 0; i < COUNT; i++)
				{
					*(p++) = sin(a);
					a += NUM;
				}
			}
			deltaTsc = __rdtsc() - deltaTsc;
			nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT * REPEATS);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Single-thread C++ math, nanoseconds = %.3f.\r\n", nanosecondsPerNumber);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT);
		}
		else
		{
			AppLib::writeParmError("performance counter not detected or failed");
			exitCode = 2;
		}
		_aligned_free(ptr);
	}
	else
	{
		AppLib::writeParmError("memory allocation failed");
		exitCode = 1;
	}
	return exitCode;
}
int Calc::scenarioMultiThread(double& dummySum)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	int exitCode = 0;
	dummySum = 0;
	Timer timer;
	LARGE_INTEGER hzPc;
	LARGE_INTEGER hzTsc;
	size_t num_threads = std::thread::hardware_concurrency();
	void* ptr = _aligned_malloc(COUNT * UNIT * num_threads, PAGE);
	if (ptr)
	{
		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double fPc = static_cast<double>(hzPc.QuadPart);
			double tPc = 1.0 / fPc;
			double fTsc = static_cast<double>(hzTsc.QuadPart);
			double tTsc = 1.0 / fTsc;
			double fPcMhz = fPc / 1E6;
			double tPcNs = 1E9 / fPc;
			double fTscMhz = fTsc / 1E6;
			double tTscNs = 1E9 / fTsc;

			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance counter : F = %.1f MHz, T = %.3f ns.\r\n", fPcMhz, tPcNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Time stamp counter  : F = %.1f MHz, T = %.3f ns.\r\n", fTscMhz, tTscNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			
			DWORD64 deltaTsc = __rdtsc();
			std::vector<std::thread> threadsAsm = {};
			for (size_t i = 0; i < num_threads; i++)
			{
				BYTE* threadPtr = reinterpret_cast<BYTE*>(ptr) + i * COUNT * UNIT;
				threadsAsm.push_back(std::thread(threadFunctionAsm, threadPtr));  // Create and run N threads, N = hardware concurrency.
			}
			for (size_t i = 0; i < num_threads; ++i)
			{
				threadsAsm[i].join();   // Wait for N threads.
			}
			deltaTsc = __rdtsc() - deltaTsc;
			double nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT * REPEATS);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d-thread asm x87 FSIN, nanoseconds = %.3f.\r\n", (int)num_threads, nanosecondsPerNumber);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT * num_threads);

			deltaTsc = __rdtsc();
			std::vector<std::thread> threadsCpp = {};
			for (size_t i = 0; i < num_threads; i++)
			{
				BYTE* threadPtr = reinterpret_cast<BYTE*>(ptr) + i * COUNT * UNIT;
				threadsCpp.push_back(std::thread(threadFunctionCpp, threadPtr));  // Create and run N threads, N = hardware concurrency.
			}
			for (size_t i = 0; i < num_threads; ++i)
			{
				threadsCpp[i].join();   // Wait for N threads.
			}
			deltaTsc = __rdtsc() - deltaTsc;
			nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT * REPEATS);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d-thread C++ math, nanoseconds = %.3f.\r\n", (int)num_threads, nanosecondsPerNumber);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT * num_threads);
		}
		else
		{
			AppLib::writeParmError("performance counter not detected or failed");
			exitCode = 2;
		}
		_aligned_free(ptr);
	}
	else
	{
		AppLib::writeParmError("memory allocation failed");
		exitCode = 1;
	}
	return exitCode;
}

void threadFunctionAsm(void* ptr)
{
	DWORD64 deltaTsc = 0;
	calcSine(ptr, COUNT, REPEATS, deltaTsc, NUM);
}

void threadFunctionCpp(void* ptr)
{
	double a = NUM;
	for (int j = 0; j < REPEATS; j++)
	{
		double* p = reinterpret_cast<double*>(ptr);
		for (int i = 0; i < COUNT; i++)
		{
			*(p++) = sin(a);
			a += NUM;
		}
	}
}

double Calc::sumHelper(double* p, size_t count)
{
	double sum = 0;
	for (int i = 0; i < count; i++)
	{
		sum += *(p++);
	}
	return sum;
}