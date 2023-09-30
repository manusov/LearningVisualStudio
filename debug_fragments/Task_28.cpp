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

#include "Task_28.h"

constexpr DWORD64 BLOCK_SIZE = 16384;
BOOL use512 = TRUE;
BOOL use256 = TRUE;
BOOL use128 = TRUE;

HANDLER_DESCRIPTOR Task_28::hdesc[] =
{
	{ vnniPattern512, sizeof(__m512i), 8, 3000000, FALSE, "_mm512_dpbusd_epi32" },   // Index = 0, 512-bit variant.
	{ vnniPattern256, sizeof(__m256i), 8, 3000000, FALSE, "_mm256_dpbusd_epi32" },   // Index = 1, 256-bit variant.
	{ vnniPattern128, sizeof(__m128i), 8, 3000000, FALSE, "_mm_dpbusd_epi32" }       // Index = 2, 128-bit variant.
};
#define CPUID_TSC_MASK  0x20
#define MASK_AVX512F    0x10000
#define MASK_AVX512VNNI 0x800
#define MASK_AVX512VL   0x80000000
#define CONTEXT_AVX512  0xE6

int Task_28::execute(int argc, char** argv)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 VNNI benchmark (sample 2 of 2).]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	int exitCode = 0;
	DWORD64 blockSize = BLOCK_SIZE;
	BOOL vlRequired = FALSE;
	if (use512) vlRequired |= hdesc[0].vlRequired;
	if (use256) vlRequired |= hdesc[1].vlRequired;
	if (use128) vlRequired |= hdesc[2].vlRequired;

	Timer timer;
	LARGE_INTEGER fperf{ 0 };
	LARGE_INTEGER ftsc{ 0 };
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
	if (tscFlag)
	{
		__cpuid(a, 0);
		BOOL avxFlag = FALSE;
		BOOL vlFlag = FALSE;
		if (a[0] > 7)
		{
			__cpuidex(a, 7, 0);
			if ((a[1] & MASK_AVX512F) && (a[2] & MASK_AVX512VNNI))
			{
				avxFlag = TRUE;
			}
			if (a[1] & MASK_AVX512VL)
			{
				vlFlag = TRUE;
			}
		}
		if (avxFlag)
		{
			if ((vlFlag) || (!vlRequired))
			{
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
						double secondsTsc = 1.0 / ftsc.QuadPart;
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
							AppLib::write(".");

							if (use512) runScenario(0, msg, hdesc, p1, p2, blockSize, secondsTsc);
							if (use256) runScenario(1, msg, hdesc, p1, p2, blockSize, secondsTsc);
							if (use128) runScenario(2, msg, hdesc, p1, p2, blockSize, secondsTsc);

							BOOL b1 = VirtualFree(p1, 0, MEM_RELEASE);
							BOOL b2 = VirtualFree(p2, 0, MEM_RELEASE);
							if ((!b1) || (!b2))
							{
								AppLib::writeColor("Memory release failed.", APPCONST::ERROR_COLOR);
								exitCode = 7;
							}
						}
						else
						{
							AppLib::writeColor("Memory allocation failed.", APPCONST::ERROR_COLOR);
							exitCode = 6;
						}
					}
					else
					{
						AppLib::writeColor("Performance counter failed.", APPCONST::ERROR_COLOR);
						exitCode = 5;
					}
				}
				else
				{
					AppLib::writeColor("Incompatible OS: AVX512 context not supported.", APPCONST::ERROR_COLOR);
					exitCode = 4;
				}
			}
			else
			{
				AppLib::writeColor("AVX512VL required for selected scenario.", APPCONST::ERROR_COLOR);
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
void Task_28::runScenario(int index, char* msg, HANDLER_DESCRIPTOR hdesc[], LPVOID p1, LPVOID p2, DWORD64 blockSize, double secondsTsc)
{
	HANDLER_DESCRIPTOR* hd = &hdesc[index];
	unsigned long long (*handler)(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i & sum) = hd->handler;
	unsigned int unit = hd->unit;
	unsigned int unrollCount = static_cast<unsigned int>(blockSize / unit / hd->unrollFactor);
	unsigned int repeats = hd->repeats;
	const char* name = hd->name;
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\n\r\nRun (%s, data unit=%d bytes, unroll cycle count=%d, repeats=%d) ...\r\n",
		name, unit, unrollCount, repeats);
	AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
	memset(p1, 2, BLOCK_SIZE);  // Fill first source block, use bytes = 02h.
	memset(p2, 3, BLOCK_SIZE);  // Fill second source block, use bytes = 03h.
	__m512i sum{ 0 };
	unsigned long long dTSC = (handler)(p1, p2, unrollCount, repeats, sum);  // Target measured operation
	double gigabytes = blockSize * repeats / 1E9;
	double seconds = secondsTsc * dTSC;
	double gigabytesPerSecond = gigabytes / seconds;
	double nanosecondsElapsed = secondsTsc * dTSC * 1E9;
	double instructionsExecuted = static_cast<double>(blockSize / unit * repeats);
	double nanosecondsPerInstruction = nanosecondsElapsed / instructionsExecuted;
	decimalPrint512x32(&sum);  // This for prevent speculative skip execution if result not used.
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "Bandwidth is %.3f GBPS, %.3f ns/vector instruction.\r\n",
		gigabytesPerSecond, nanosecondsPerInstruction);
	AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
}
unsigned long long Task_28::vnniPattern512(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum)
{
	__m512i sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	constexpr __m512i blank{ 0 };
	unsigned long long t = __rdtsc();  // Time start point.
	for (unsigned int j = 0; j < repeats; j++)
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
		for (unsigned int i = 0; i < unrollCount; i++)
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
	unsigned long long dTSC = __rdtsc() - t;  // Time end point.
	return dTSC;
}
unsigned long long Task_28::vnniPattern256(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum)
{
	__m256i sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	constexpr __m256i blank{ 0 };
	unsigned long long t = __rdtsc();  // Time start point.
	for (unsigned int j = 0; j < repeats; j++)
	{
		__m256i* ptrX1 = reinterpret_cast<__m256i*>(p1);
		__m256i* ptrX2 = reinterpret_cast<__m256i*>(p2);
		_mm256_storeu_epi32(&sum1, blank);
		_mm256_storeu_epi32(&sum2, blank);
		_mm256_storeu_epi32(&sum3, blank);
		_mm256_storeu_epi32(&sum4, blank);
		_mm256_storeu_epi32(&sum5, blank);
		_mm256_storeu_epi32(&sum6, blank);
		_mm256_storeu_epi32(&sum7, blank);
		_mm256_storeu_epi32(&sum8, blank);
		for (unsigned int i = 0; i < unrollCount; i++)
		{
			sum1 = _mm256_dpbusd_epi32(sum1, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum2 = _mm256_dpbusd_epi32(sum2, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum3 = _mm256_dpbusd_epi32(sum3, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum4 = _mm256_dpbusd_epi32(sum4, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum5 = _mm256_dpbusd_epi32(sum5, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum6 = _mm256_dpbusd_epi32(sum6, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum7 = _mm256_dpbusd_epi32(sum7, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum8 = _mm256_dpbusd_epi32(sum8, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
		}
		sum1 = _mm256_add_epi32(sum1, sum2);
		sum3 = _mm256_add_epi32(sum3, sum4);
		sum5 = _mm256_add_epi32(sum5, sum6);
		sum7 = _mm256_add_epi32(sum7, sum8);
		sum1 = _mm256_add_epi32(sum1, sum3);
		sum5 = _mm256_add_epi32(sum5, sum7);
		__m256i* ptrSum = reinterpret_cast<__m256i*>(&sum);
		*ptrSum = _mm256_add_epi32(*ptrSum, sum1);
		*ptrSum = _mm256_add_epi32(*ptrSum, sum5);
	}
	unsigned long long dTSC = __rdtsc() - t;  // Time end point.
	return dTSC;
}
unsigned long long Task_28::vnniPattern128(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum)
{
	__m128i sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8;
	constexpr __m128i blank{ 0 };
	unsigned long long t = __rdtsc();  // Time start point.
	for (unsigned int j = 0; j < repeats; j++)
	{
		__m128i* ptrX1 = reinterpret_cast<__m128i*>(p1);
		__m128i* ptrX2 = reinterpret_cast<__m128i*>(p2);
		_mm_storeu_epi32(&sum1, blank);
		_mm_storeu_epi32(&sum2, blank);
		_mm_storeu_epi32(&sum3, blank);
		_mm_storeu_epi32(&sum4, blank);
		_mm_storeu_epi32(&sum5, blank);
		_mm_storeu_epi32(&sum6, blank);
		_mm_storeu_epi32(&sum7, blank);
		_mm_storeu_epi32(&sum8, blank);
		for (unsigned int i = 0; i < unrollCount; i++)
		{
			sum1 = _mm_dpbusd_epi32(sum1, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum2 = _mm_dpbusd_epi32(sum2, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum3 = _mm_dpbusd_epi32(sum3, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum4 = _mm_dpbusd_epi32(sum4, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum5 = _mm_dpbusd_epi32(sum5, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum6 = _mm_dpbusd_epi32(sum6, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum7 = _mm_dpbusd_epi32(sum7, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
			sum8 = _mm_dpbusd_epi32(sum8, *ptrX1, *ptrX2);
			ptrX1++;
			ptrX2++;
		}
		sum1 = _mm_add_epi32(sum1, sum2);
		sum3 = _mm_add_epi32(sum3, sum4);
		sum5 = _mm_add_epi32(sum5, sum6);
		sum7 = _mm_add_epi32(sum7, sum8);
		sum1 = _mm_add_epi32(sum1, sum3);
		sum5 = _mm_add_epi32(sum5, sum7);
		__m128i* ptrSum = reinterpret_cast<__m128i*>(&sum);
		*ptrSum = _mm_add_epi32(*ptrSum, sum1);
		*ptrSum = _mm_add_epi32(*ptrSum, sum5);
	}
	unsigned long long dTSC = __rdtsc() - t;  // Time end point.
	return dTSC;
}
