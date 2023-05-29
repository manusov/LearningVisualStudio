/*

Multi-thread L1 cache read bandwidth benchmark, required AVX512 instructions.
Demo for use MASM 32/64 at Visual Studio 2022.

Special thanks (there all instructions for configure VS2022 IDE):
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

See also:
https://learn.microsoft.com/ru-ru/cpp/assembler/masm/masm-for-x64-ml64-exe?view=msvc-170
https://habr.com/ru/articles/111275/
http://lallouslab.net/2016/01/11/introduction-to-writing-x64-assembly-in-visual-studio/
https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/

*/

#include <windows.h>
#include <intrin.h>
#include <thread>
#include <vector>
#include <iostream>

BOOL detectAvx512();
int singleThreadScenario();
int multiThreadScenario();
void threadFunction(void *ptr);
BOOL measureTscClock(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc);
extern "C" void __stdcall readAvx512(void* ptr, DWORD64 count, DWORD64 repeats, DWORD64 & deltaTsc);

constexpr DWORD64 COUNT = 32;             // 32 * 64 * 16 = 32KB, this means stable cache hits for processor with L1data=48KB.
constexpr DWORD64 UNIT = 64 * 16;         // * 16 because cycle unrolled x16 yet without tails support.
constexpr DWORD64 REPEATS = 500000000;    // Repeats for measurement.
constexpr DWORD64 PAGE = 4096;            // Optimal allocation at page bound.

int main()
{
	int exitCode = 0;
	if (detectAvx512())
	{
		exitCode = singleThreadScenario();
		if (!exitCode)
		{
			std::cout << std::endl;
			exitCode = multiThreadScenario();
		}
	}
	else
	{
		std::cout << "AVX512 or OS ZMM-context management is not supported." << std::endl;
		exitCode = -1;
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

int singleThreadScenario()
{
	LARGE_INTEGER hzPc;
	LARGE_INTEGER hzTsc;
	int exitCode = 0;
	
	std::cout << "Start single-thread scenario, measuring TSC clock..." << std::endl;
	void* ptr = _aligned_malloc(COUNT * UNIT, PAGE);
	if (ptr)
	{
		if (measureTscClock(hzPc, hzTsc))
		{
			double fPc = static_cast<double>(hzPc.QuadPart);
			double tPc = 1.0 / fPc;
			double fTsc = static_cast<double>(hzTsc.QuadPart);
			double tTsc = 1.0 / fTsc;
			std::cout << "fPc=" << fPc << ", tPc=" << tPc << ", fTsc=" << fTsc << ", tTsc=" << tTsc << "." << std::endl;
			std::cout << "Measuring AVX512 single-thread L1-ZMM read bandwidth..." << std::endl;

			DWORD64 deltaTsc = 0;
			readAvx512(ptr, COUNT, REPEATS, deltaTsc);

			double gigabytes = static_cast<double>(COUNT * UNIT * REPEATS) / (1024 * 1024 * 1024);
			double seconds = static_cast<double>(deltaTsc) * tTsc;
			double gbps = gigabytes / seconds;
			std::cout << "Single-thread GBPS=" << gbps << "." << std::endl;
		}
		else
		{
			std::cout << "TSC clock measurement failed." << std::endl;
			exitCode = 2;
		}
		_aligned_free(ptr);
	}
	else
	{
		std::cout << "Memory allocation failed." << std::endl;
		exitCode = 1;
	}
	std::cout << "Done." << std::endl;
	return exitCode;
}

int multiThreadScenario()
{
	LARGE_INTEGER hzPc;
	LARGE_INTEGER hzTsc;
	int exitCode = 0;

	std::cout << "Start multi-thread scenario, measuring TSC clock..." << std::endl;
	size_t num_threads = std::thread::hardware_concurrency();
	void* ptr = _aligned_malloc(COUNT * UNIT * num_threads, PAGE);
	if (ptr)
	{
		if (measureTscClock(hzPc, hzTsc))
		{
			double fPc = static_cast<double>(hzPc.QuadPart);
			double tPc = 1.0 / fPc;
			double fTsc = static_cast<double>(hzTsc.QuadPart);
			double tTsc = 1.0 / fTsc;
			std::cout << "fPc=" << fPc << ", tPc=" << tPc << ", fTsc=" << fTsc << ", tTsc=" << tTsc << "." << std::endl;
			std::cout << "Measuring AVX512 " << num_threads << "-thread L1-ZMM read bandwidth..." << std::endl;

			DWORD64 deltaTsc = __rdtsc();
			std::vector<std::thread> threads = {};
			for (size_t i = 0; i < num_threads; i++)
			{
				BYTE* threadPtr = reinterpret_cast<BYTE*>(ptr) + i * COUNT * UNIT;
				threads.push_back(std::thread(threadFunction, threadPtr));  // Create and run N threads, N = hardware concurrency.
			}
			for (size_t i = 0; i < num_threads; ++i)
			{
				threads[i].join();   // Wait for N threads.
			}
			deltaTsc = __rdtsc() - deltaTsc;

			double gigabytes = static_cast<double>(COUNT * UNIT * REPEATS * num_threads) / (1024 * 1024 * 1024);
			double seconds = static_cast<double>(deltaTsc) * tTsc;
			double gbps = gigabytes / seconds;
			std::cout << "Multi-thread GBPS=" << gbps << "." << std::endl;
		}
		else
		{
			std::cout << "TSC clock measurement failed." << std::endl;
			exitCode = 2;
		}
		_aligned_free(ptr);
	}
	else
	{
		std::cout << "Memory allocation failed." << std::endl;
		exitCode = 1;
	}
	std::cout << "Done." << std::endl;
	return exitCode;
}

void threadFunction(void* ptr)
{
	DWORD64 deltaTsc = 0;
	readAvx512(ptr, COUNT, REPEATS, deltaTsc);
}

BOOL measureTscClock(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc)
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
