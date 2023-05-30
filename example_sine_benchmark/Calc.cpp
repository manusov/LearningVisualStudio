/*
Realization of sine calculation benchmark functions class.
*/

#include "Calc.h"

extern "C" void __stdcall calcSine(void* ptr, DWORD64 count, DWORD64 repeats, DWORD64 & deltaTsc, double x);
void threadFunctionAsm(void* ptr);
void threadFunctionCpp(void* ptr);
void threadFunctionAvx512(void* ptr);
BOOL detectAvx512();

constexpr DWORD64 COUNT = 4096;      // 4096 * 8 = 32KB, this means stable cache hits for processor with L1data=48KB.
constexpr DWORD64 UNIT = 8;          // One data unit, double = 8 bytes.
constexpr DWORD64 REPEATS = 50000;   // Repeats for measurement.
constexpr DWORD64 PAGE = 4096;       // Optimal allocation at page bound.
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
			double mops = (COUNT * REPEATS) / (deltaTsc * tTsc) / 1E6;
			snprintf(msg, APPCONST::MAX_TEXT_STRING,
				"Single-thread asm x87 FSIN, nanoseconds = %.3f, mops = %.3f.\r\n",
				nanosecondsPerNumber, mops);
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
			mops = (COUNT * REPEATS) / (deltaTsc * tTsc) / 1E6;
			snprintf(msg, APPCONST::MAX_TEXT_STRING,
				"Single-thread C++ math, nanoseconds = %.3f, mops = %.3f.\r\n",
				nanosecondsPerNumber, mops);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT);

			if (detectAvx512())
			{
				deltaTsc = __rdtsc();
				__m512d x  = {   NUM, 2*NUM, 3*NUM, 4*NUM, 5*NUM, 6*NUM, 7*NUM, 8*NUM };
				__m512d dx = { 8*NUM, 8*NUM, 8*NUM, 8*NUM, 8*NUM, 8*NUM, 8*NUM, 8*NUM };
				for (int j = 0; j < REPEATS; j++)
				{
					__m512d* p = reinterpret_cast<__m512d*>(ptr);
					for (int i = 0; i < (COUNT / 8); i++)
					{
						*(p++) = _mm512_sin_pd(x);
						x = _mm512_add_pd(x, dx);
					}
				}
				deltaTsc = __rdtsc() - deltaTsc;
				nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT / 8 * REPEATS);
				mops = (COUNT * REPEATS) / (deltaTsc * tTsc) / 1E6;
				snprintf(msg, APPCONST::MAX_TEXT_STRING,
					"Single-thread AVX512 intrinsics, nanoseconds per 8 numbers = %.3f, mops = %.3f.\r\n",
					nanosecondsPerNumber, mops);
				AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
				dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT);
			}
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
			double mops = (COUNT * REPEATS * num_threads) / (deltaTsc * tTsc) / 1E6;
			snprintf(msg, APPCONST::MAX_TEXT_STRING,
				"%d-thread asm x87 FSIN, nanoseconds = %.3f, mops = %.3f.\r\n",
				(int)num_threads, nanosecondsPerNumber, mops);
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
			mops = (COUNT * REPEATS * num_threads) / (deltaTsc * tTsc) / 1E6;
			snprintf(msg, APPCONST::MAX_TEXT_STRING,
				"%d-thread C++ math, nanoseconds = %.3f, mops = %.3f.\r\n",
				(int)num_threads, nanosecondsPerNumber, mops);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT * num_threads);

			if (detectAvx512())
			{
				deltaTsc = __rdtsc();
				std::vector<std::thread> threadsAvx512 = {};
				for (size_t i = 0; i < num_threads; i++)
				{
					BYTE* threadPtr = reinterpret_cast<BYTE*>(ptr) + i * COUNT * UNIT;
					threadsAvx512.push_back(std::thread(threadFunctionAvx512, threadPtr));  // Create and run N threads, N = hardware concurrency.
				}
				for (size_t i = 0; i < num_threads; ++i)
				{
					threadsAvx512[i].join();   // Wait for N threads.
				}
				deltaTsc = __rdtsc() - deltaTsc;
				nanosecondsPerNumber = (1E9 * deltaTsc * tTsc) / (COUNT / 8 * REPEATS);
				mops = (COUNT * REPEATS * num_threads) / (deltaTsc * tTsc) / 1E6;
				snprintf(msg, APPCONST::MAX_TEXT_STRING,
					"%d-thread AVX512 intrinsics, nanoseconds per 8 numbers = %.3f, mops = %.3f.\r\n",
					(int)num_threads, nanosecondsPerNumber, mops);
				AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
				dummySum += sumHelper(reinterpret_cast<double*>(ptr), COUNT * num_threads);
			}
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

void threadFunctionAvx512(void* ptr)
{
	__m512d x = { NUM, 2 * NUM, 3 * NUM, 4 * NUM, 5 * NUM, 6 * NUM, 7 * NUM, 8 * NUM };
	__m512d dx = { 8 * NUM, 8 * NUM, 8 * NUM, 8 * NUM, 8 * NUM, 8 * NUM, 8 * NUM, 8 * NUM };
	for (int j = 0; j < REPEATS; j++)
	{
		__m512d* p = reinterpret_cast<__m512d*>(ptr);
		for (int i = 0; i < (COUNT / 8); i++)
		{
			*(p++) = _mm512_sin_pd(x);
			x = _mm512_add_pd(x, dx);
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
