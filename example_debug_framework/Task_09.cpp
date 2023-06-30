/*
Raw experiments.
Similar (but not equal) functionality with Task_06, 
but one of classes for support both x64 and ia32 builds.
Yet only quadrat matrices supported.
Note about size limits for ia32 builds.

AVX512 matrix transpose, 8x16 base block, re-create threads at measurement cycle.
*/

#include "Task_09.h"

constexpr DWORD64 MATRIX_ROWS         = 32768;   // 1024;  // 32768;  // 48 * 1024;  // 32768;  // 32768;  // 1024;  // 32;  // 16;
constexpr DWORD64 MATRIX_COLUMNS      = 32768;   // 1024;  // 32768;  // 48 * 1024;  // 32768;  // 32768;  // 1024;  // 32;  // 16;
constexpr DWORD64 ELEMENT_SIZE        = sizeof(double);
constexpr DWORD64 ELEMENT_COUNT       = MATRIX_ROWS * MATRIX_COLUMNS;
constexpr DWORD32 BLOCK_ALIGN         = 64;

constexpr DWORD32 ROW_VECTOR_SIZE     = 16;
constexpr DWORD32 COLUMN_VECTOR_SIZE  = 16;
constexpr DWORD64 ROW_ITERATIONS      = MATRIX_ROWS / ROW_VECTOR_SIZE;
constexpr DWORD64 COLUMN_ITERATIONS   = MATRIX_COLUMNS / COLUMN_VECTOR_SIZE;

constexpr DWORD64 MEASUREMENT_REPEATS = 3;  // 10000; // 3;   // 10000;
constexpr DWORD64 ALLOCATED_SIZE      = ELEMENT_COUNT * ELEMENT_SIZE;
constexpr DWORD64 TRANSFERRED_SIZE    = ALLOCATED_SIZE * MEASUREMENT_REPEATS;

constexpr double GIGABYTES = static_cast<double>(TRANSFERRED_SIZE) / 1E9;

constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

// extern "C" BOOL __stdcall transposeAVX512(double* src, double* dst, size_t columnsCount, size_t rowsCount);
   extern "C" BOOL __stdcall transposeAVX512NT(double* src, double* dst, size_t columnsCount, size_t rowsCount);
// extern "C" BOOL __stdcall transposeAVX256(double* src, double* dst, size_t columnsCount, size_t rowsCount);

int Task_09::execute(int argc, char** argv)
{
	int exitCode = 0;
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::write("\r\n");
	if (detectAvx512())
	{
		Timer timer;
		LARGE_INTEGER hzPc;
		LARGE_INTEGER hzTsc;
		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double perfMhz = static_cast<double>(hzPc.QuadPart) / 1E6;
			double perfNs = 1E9 / static_cast<double>(hzPc.QuadPart);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "OS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", perfMhz, perfNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			double tscMhz = static_cast<double>(hzTsc.QuadPart) / 1E6;
			double tscNs = 1E9 / static_cast<double>(hzTsc.QuadPart);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Measure TSC by OS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", tscMhz, tscNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

			double* srcPtr = reinterpret_cast<double*>(_aligned_malloc(static_cast<size_t>(ALLOCATED_SIZE), BLOCK_ALIGN));
			double* dstPtr = reinterpret_cast<double*>(_aligned_malloc(static_cast<size_t>(ALLOCATED_SIZE), BLOCK_ALIGN));
			if ((srcPtr) && (dstPtr))
			{
				fill(srcPtr, ELEMENT_COUNT, X0, DX);
				if (verify(srcPtr, MATRIX_COLUMNS, MATRIX_ROWS, FALSE, X0, DX))
				{
					AppLib::writeColor("Source matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					AppLib::writeColor("Destination matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					
					// ---------- Single-thread --------------------------------------------------------------------------------------------------------
					
					AppLib::writeColor("Transpose single-thread...", APPCONST::TABLE_COLOR);

					// Timing interval start.
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						// transposeAVX512(srcPtr, dstPtr, MATRIX_COLUMNS, MATRIX_ROWS);
						// transposeAVX512(srcPtr + 32*16, dstPtr + 16, MATRIX_COLUMNS, MATRIX_ROWS);

						double* src = srcPtr;
						double* dst = dstPtr;
						for (int j = 0; j < ROW_ITERATIONS; j++)
						{
							// transposeAVX512(src, dst, MATRIX_COLUMNS, MATRIX_ROWS);
							transposeAVX512NT(src, dst, MATRIX_COLUMNS, MATRIX_ROWS);
							//
							src += MATRIX_COLUMNS * COLUMN_VECTOR_SIZE;
							dst += ROW_VECTOR_SIZE;
						}
					}
					dtsc = __rdtsc() - dtsc;
					// Timing interval end.

					AppLib::writeColor("done.\r\n", APPCONST::TABLE_COLOR);
					

					/* DEBUG
					AppLib::write("\r\n");
					showDump16x16(srcPtr);
					AppLib::write("\r\n");
					showDump16x16(dstPtr);
					AppLib::write("\r\n");
					DEBUG */
					/* DEBUG
					AppLib::write("\r\n");
					showDump32x32(srcPtr);
					AppLib::write("\r\n");
					showDump32x32(dstPtr);
					AppLib::write("\r\n");
					DEBUG */

					if (verify(dstPtr, MATRIX_COLUMNS, MATRIX_ROWS, TRUE, X0, DX))  // This takes additional time.
					{
						showGBPS(hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 5;
					}

					// ---------- Multi-thread --------------------------------------------------------------------------------------------------------
					
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					AppLib::writeColor("Destination matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					size_t hardwareThreads = std::thread::hardware_concurrency();
					size_t softwareThreads = ROW_ITERATIONS;
					srcStore = srcPtr;
					dstStore = dstPtr;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "%zd hardware threads, transpose by %zd threads...", hardwareThreads, softwareThreads);
					
					AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

					// Timing interval start.
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						std::vector<std::thread> threads = {};
						for (size_t j = 0; j < softwareThreads; j++)
						{
							void* threadPtr = reinterpret_cast<void*>(j);
							threads.push_back(std::thread(threadFunction, threadPtr));  // Create and run N threads, N = software concurrency.
						}
						for (size_t j = 0; j < softwareThreads; ++j)
						{
							threads[j].join();   // Wait for N threads.
						}
					}
					dtsc = __rdtsc() - dtsc;
					// Timing interval end.

					AppLib::writeColor("done.\r\n", APPCONST::TABLE_COLOR);

					if (verify(dstPtr, MATRIX_COLUMNS, MATRIX_ROWS, TRUE, X0, DX))  // This takes additional time.
					{
						showGBPS(hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 5;
					}

				}
				else
				{
					AppLib::writeParmError("source matrix elements calculation failed");
					exitCode = 4;
				}
			}
			else
			{
				AppLib::writeParmError("matrix memory allocation failed");
				exitCode = 3;
			}
			if (srcPtr) _aligned_free(srcPtr);
			if (dstPtr) _aligned_free(dstPtr);
		}
		else
		{
			AppLib::writeParmError("TSC clock measurement failed");
			exitCode = 2;
		}
	}
	else
	{
		AppLib::writeParmError("AVX512 or OS ZMM-context management is not supported");
		exitCode = 1;
	}
	return exitCode;
}
double* Task_09::srcStore;
double* Task_09::dstStore;
void Task_09::threadFunction(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	double* p1 = srcStore + i * MATRIX_COLUMNS * COLUMN_VECTOR_SIZE;
	double* p2 = dstStore + i * ROW_VECTOR_SIZE;
	// transposeAVX512(p1, p2, MATRIX_COLUMNS, MATRIX_ROWS);
	transposeAVX512NT(p1, p2, MATRIX_COLUMNS, MATRIX_ROWS);
	//
}
BOOL Task_09::detectAvx512()
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
void Task_09::fill(double* ptr, DWORD64 elementsCount, double x0 = X0, double dx = DX)
{
	for (size_t i = 0; i < elementsCount; i++)
	{
		*ptr = x0;
		ptr++;
		x0 += dx;
	}
}
BOOL Task_09::verify(double* ptr, DWORD64 columnsCount, DWORD64 rowsCount, BOOL transpose, double x0 = X0, double dx = DX)
{
	BOOL passed = TRUE;
	if (transpose)
	{
		for (size_t i = 0; (i < rowsCount) && passed; i++)
		{
			for (size_t j = 0; (j < columnsCount) && passed; j++)
			{
				double* ptr1 = ptr + i + j * columnsCount;
				double a = *ptr1;
				if ((a < (x0 - EPSILON)) || (a > (x0 + EPSILON)))
				{
					passed = FALSE;
				}
				x0 += dx;
			}
		}
	}
	else
	{
		for (size_t i = 0; (i < rowsCount) && passed; i++)
		{
			for (size_t j = 0; (j < columnsCount) && passed; j++)
			{
				double a = *ptr;
				if ((a < (x0 - EPSILON)) || (a > (x0 + EPSILON)))
				{
					passed = FALSE;
				}
				ptr++;
				x0 += dx;
			}
		}
	}
	return passed;
}
void Task_09::showGBPS(DWORD64 hzTSC, DWORD64 dTSC, double gigabytes)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double seconds = 1.0 / static_cast<double>(hzTSC) * dTSC;
	double gbps = gigabytes / seconds;
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "%.3f GB per %.3f seconds = %.3f GBPS.\r\n", gigabytes, seconds, gbps);
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}
void Task_09::showDump8x8(void* data)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double* p = reinterpret_cast<double*>(data);
	for (int i = 0; i < 8; i++)
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		p += 8;
	}
}
void Task_09::showDump16x16(void* data)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double* p = reinterpret_cast<double*>(data);
	for (int i = 0; i < 16; i++)
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		p += 16;
	}
}
void Task_09::showDump32x32(void* data)
{
	constexpr int TEMP_MAX = APPCONST::MAX_TEXT_STRING * 2;  // For debug only.
	char msg[TEMP_MAX];      
	double* p = reinterpret_cast<double*>(data);
	for (int i = 0; i < 32; i++)
	{
		snprintf(msg, TEMP_MAX,
			"%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f%5.1f\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
			p[16], p[17], p[18], p[19], p[20], p[21], p[22], p[23], p[24], p[25], p[26], p[27], p[28], p[29], p[30], p[31]);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		p += 32;
	}
}
