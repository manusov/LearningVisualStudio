/*

Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VNNI&ig_expand=2702

TODO.
Check RDTSC supported by CPU (include not locked by VMM).
Check AVX512F/VNNI/VL extensions supported by CPU.
Check AVX512 context supported by OS.
Interrogate effect of AVX512 optimization option at project settings.
Especially for:
__m512i sum1{ 0 }, sum2{ 0 }, sum3{ 0 }, sum4{ 0 }, sum5{ 0 }, sum6{ 0 }, sum7{ 0 }, sum8{ 0 };
Explicit use AVX512 intrinsics at critical fragments, for minimize dependency from AVX512 optimization option.

*/

#include "Task_27.h"

constexpr size_t BLOCK_SIZE = 16384;
constexpr size_t BLOCK_COUNT = BLOCK_SIZE / sizeof(__m512i);
constexpr size_t UNROLL_COUNT = BLOCK_COUNT / 8;
constexpr unsigned int REPEATS = 1000000;

int Task_27::execute(int argc, char** argv)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 VNNI benchmark (sample 1 of 2).]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	int exitCode = 0;
	Timer timer;
	LARGE_INTEGER fperf{ 0 };
	LARGE_INTEGER ftsc{ 0 };
	#define CPUID_TSC_MASK  0x20
	#define MASK_AVX512F    0x10000
	#define MASK_AVX512VNNI 0x800
	int a[4]{ 0,0,0,0 };
	__cpuid(a, 0);
	BOOL tscFlag = FALSE;
	if (a[0] >= 1)
	{
		__cpuid(a, 1);
		if (a[3] & CPUID_TSC_MASK)
		{
			tscFlag = TRUE;
		}
	}
	if(tscFlag)
	{
		__cpuid(a, 0);
		BOOL avxFlag = FALSE;
		if (a[0] > 7)
		{
			__cpuidex(a, 7, 0);
			if ((a[1] & MASK_AVX512F) && (a[2] & MASK_AVX512VNNI))
			{
				avxFlag = TRUE;
			}
		}
		if (avxFlag)
		{
			#define CONTEXT_AVX512 0xE6
			unsigned long long mask = _xgetbv(0);
			mask &= CONTEXT_AVX512;
			BOOL osFlag = FALSE;
			if (mask == CONTEXT_AVX512)
			{
				osFlag = TRUE;
			}
			if (osFlag)
			{
				AppLib::write("Initializing timers...\r\n");
				if (timer.precisionMeasure(fperf, ftsc))
				{
					double mhzPerf = fperf.QuadPart / 1000000.0;
					double mhzTsc = ftsc.QuadPart / 1000000.0;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "OS performance counter = %.3f MHz, CPU time stamp counter = %.3f MHz.", mhzPerf, mhzTsc);
					AppLib::write(msg);
					LPVOID p1 = VirtualAlloc(nullptr, BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					LPVOID p2 = VirtualAlloc(nullptr, BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					if (p1 && p2)
					{
						AppLib::write("\r\n1st source block: ");
						AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, reinterpret_cast<DWORD64>(p1), BLOCK_SIZE);
						AppLib::write(msg);
						AppLib::write(".\r\n2nd source block: ");
						AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, reinterpret_cast<DWORD64>(p2), BLOCK_SIZE);
						AppLib::write(msg);
						snprintf(msg, APPCONST::MAX_TEXT_STRING, ".\r\nRepeats = %d.\r\n", REPEATS);
						AppLib::write(msg);
						AppLib::write("Fill source blocks...\r\n");
						memset(p1, 1, BLOCK_SIZE);
						memset(p2, 2, BLOCK_SIZE);

						AppLib::write("Run AVX512 VNNI DOT product calculation...\r\n");
						__m512i sum{ 0 };
						__m512i sum1{ 0 }, sum2{ 0 }, sum3{ 0 }, sum4{ 0 }, sum5{ 0 }, sum6{ 0 }, sum7{ 0 }, sum8{ 0 };
						constexpr __m512i blank{ 0 };
						unsigned long long t = __rdtsc();  // Time start point.
						for (int j = 0; j < REPEATS; j++)
						{
							__m512i* ptrX1 = reinterpret_cast<__m512i*>(p1);
							__m512i* ptrX2 = reinterpret_cast<__m512i*>(p2);
							_mm512_storeu_epi32(&sum1, blank);
							_mm512_storeu_epi32(&sum2, blank);
							_mm512_storeu_epi32(&sum3, blank);
							_mm512_storeu_epi32(&sum4, blank);
							_mm512_storeu_epi32(&sum5, blank);
							_mm512_storeu_epi32(&sum6, blank);
							_mm512_storeu_epi32(&sum7, blank);
							_mm512_storeu_epi32(&sum8, blank);
							for (int i = 0; i < UNROLL_COUNT; i++)
							{
								sum1 = _mm512_dpbusd_epi32(sum1, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum2 = _mm512_dpbusd_epi32(sum2, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum3 = _mm512_dpbusd_epi32(sum3, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum4 = _mm512_dpbusd_epi32(sum4, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum5 = _mm512_dpbusd_epi32(sum5, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum6 = _mm512_dpbusd_epi32(sum6, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum7 = _mm512_dpbusd_epi32(sum7, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
								sum8 = _mm512_dpbusd_epi32(sum8, *ptrX1, *ptrX2);
								ptrX1++;
								ptrX2++;
							}
							sum1 = _mm512_add_epi32(sum1, sum2);
							sum3 = _mm512_add_epi32(sum3, sum4);
							sum5 = _mm512_add_epi32(sum5, sum6);
							sum7 = _mm512_add_epi32(sum7, sum8);
							sum1 = _mm512_add_epi32(sum1, sum3);
							sum5 = _mm512_add_epi32(sum5, sum7);
							sum = _mm512_add_epi32(sum, sum1);
							sum = _mm512_add_epi32(sum, sum5);
						}
						unsigned long long dt = __rdtsc() - t;  // Time end point.

						double tCLK = 1.0 / ftsc.QuadPart;
						double nanosecondsPerIteration = tCLK * dt * 1E9 / (BLOCK_COUNT * REPEATS);
						decimalPrint512x32(&sum);
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "[%.3f ns/vector instruction.]\r\n", nanosecondsPerIteration);
						AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);

						BOOL b1 = VirtualFree(p1, 0, MEM_RELEASE);
						BOOL b2 = VirtualFree(p2, 0, MEM_RELEASE);
						if ((!b1) || (!b2))
						{
							AppLib::writeColor("Memory release failed.", APPCONST::ERROR_COLOR);
							exitCode = 6;
						}
					}
					else
					{
						AppLib::writeColor("Memory allocation failed.", APPCONST::ERROR_COLOR);
						exitCode = 5;
					}
				}
				else
				{
					AppLib::writeColor("Performance counter failed.", APPCONST::ERROR_COLOR);
					exitCode = 4;
				}
			}
			else
			{
				AppLib::writeColor("Incompatible OS: AVX512 context not supported.", APPCONST::ERROR_COLOR);
				exitCode = 3;
			}
		}
		else
		{
			AppLib::writeColor("Incompatible CPU: AVX512F and AVX512VNNI required.", APPCONST::ERROR_COLOR);
			exitCode = 2;
		}
	}
	else
	{
		AppLib::writeColor("RDTSC instruction not supported or locked.", APPCONST::ERROR_COLOR);
		exitCode = 1;
	}
	return exitCode;
}
