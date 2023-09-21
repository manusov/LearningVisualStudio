/*
Class for AVX512 matrix transpose.
Step 2 = debug for cycle with cells 8x8, for x64 builds only.
Variant with manipulations at ZMM registers with 8x8 matrix,
load and store 512-bit operands without scatter-gather.
Row-columns cycles supported by C++ code, no internal cycles in assembler module.

TODO 1.
1)+ Why error in the "Release" mode? = Corruption of XMM6-XMM15 (must be non-volatile).
2)+ Measure timings with repeats.
3)+ Multithread mode.
4)+ Temporal vs non-temporal for read and write.
5)- Prefetch read.
6)- Minimize save-restore registers requirements, can not use non-volatile XMM6-XMM15. = Speed decreased or fluctuation?
7)+ Select vertical read horizontal write or vice versa. = Differrent effect for single/multi thread. Use adaptive.

TODO 2.
1)  Move row-column cycle to asm routine.
2)  Select base block 8x8 big-to-little or little-to-big.
3)  Optimize number of threads.
4)  Resource release rules for threads.

TODO summary.
1)  Block method required for L1 cache efficiency.
2)  Maximize linear read-write intervals.
3)  Try gather-scatter with vertical lines size by L1 size. Note about cache line size = 64 bytes.

*/

#include "Task_06.h"

#ifdef NATIVE_WIDTH_64


// extern "C" void __stdcall transposeBigToLittle(__m512d * src, __m512d * dst, size_t rowSize);
// extern "C" void __stdcall transposeBigToLittleNT(__m512d * src, __m512d * dst, size_t rowSize);

// extern "C" void __stdcall transposeLittleToBig(__m512d * src, __m512d * dst, size_t rowSize);
// extern "C" void __stdcall transposeLittleToBigNT(__m512d * src, __m512d * dst, size_t rowSize);

extern "C" void __stdcall transposeTemporal(__m512d * src, __m512d * dst, size_t rowSize);
extern "C" void __stdcall transposeNonTemporal(__m512d * src, __m512d * dst, size_t rowSize);

constexpr size_t MATRIX_ROWS = 32768;
constexpr size_t MATRIX_COLUMNS = 32768;
constexpr size_t ELEMENT_SIZE = sizeof(double);
constexpr size_t ELEMENT_COUNT = MATRIX_ROWS * MATRIX_COLUMNS;
constexpr size_t BLOCK_ALIGN = 64;
constexpr size_t VECTOR_SIZE = 8;
constexpr size_t MEASUREMENT_REPEATS = 3;

constexpr size_t ALLOCATED_SIZE = ELEMENT_COUNT * ELEMENT_SIZE;
constexpr size_t TRANSFERRED_SIZE = ALLOCATED_SIZE * MEASUREMENT_REPEATS;

constexpr double GIGABYTES = static_cast<double>(TRANSFERRED_SIZE) / 1E9;
constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

#endif

int Task_06::execute(int argc, char** argv)
{
#ifdef NATIVE_WIDTH_64
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

			double* srcPtr = reinterpret_cast<double*>(_aligned_malloc(ALLOCATED_SIZE, BLOCK_ALIGN));
			double* dstPtr = reinterpret_cast<double*>(_aligned_malloc(ALLOCATED_SIZE, BLOCK_ALIGN));
			if ((srcPtr) && (dstPtr))
			{
				fill(srcPtr, ELEMENT_COUNT, X0, DX);
				if (verify(srcPtr, MATRIX_ROWS, MATRIX_COLUMNS, FALSE, X0, DX))
				{
					
					// ---------- Single-thread, temporal data matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose single-thread, temporal data...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512singleThread(srcPtr, dstPtr, MATRIX_ROWS, MATRIX_COLUMNS);
					}
					dtsc = __rdtsc() - dtsc;
					if (verify(dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, TRUE, X0, DX))
					{
						showResults(dstPtr, hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 5;
					}

					// ---------- Single-thread, non-temporal data matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose single-thread, non-temporal data...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512singleThreadNT(srcPtr, dstPtr, MATRIX_ROWS, MATRIX_COLUMNS);
					}
					dtsc = __rdtsc() - dtsc;
					if (verify(dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, TRUE, X0, DX))
					{
						showResults(dstPtr, hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 6;
					}

					// ---------- Multi-thread, temporal data matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose multi-thread, temporal data...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					size_t hardThreads = 0;
					size_t softThreads = 0;
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512multiThread(srcPtr, dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, hardThreads, softThreads);
					}
					dtsc = __rdtsc() - dtsc;
					if (verify(dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, TRUE, X0, DX))
					{
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "Hardware threads = %llu, used = %llu.\r\n", hardThreads, softThreads);
						AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
						showResults(dstPtr, hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 7;
					}

					// ---------- Multi-thread, non-temporal data matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose multi-thread, non-temporal data...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					hardThreads = 0;
					softThreads = 0;
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512multiThreadNT(srcPtr, dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, hardThreads, softThreads);
					}
					dtsc = __rdtsc() - dtsc;
					if (verify(dstPtr, MATRIX_ROWS, MATRIX_COLUMNS, TRUE, X0, DX))
					{
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "Hardware threads = %llu, used = %llu.\r\n", hardThreads, softThreads);
						AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
						showResults(dstPtr, hzTsc.QuadPart, dtsc, GIGABYTES);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 8;
					}

					//----------------------------------------------------------------------------------------------------------------------------------------------------

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
#else
	AppLib::writeColor("\r\nAVX512 matrix transpose test is not supported at ia32 build.\r\n", APPCONST::ERROR_COLOR);
	return -1;
#endif
}

#ifdef NATIVE_WIDTH_64

void Task_06::transposeAvx512singleThread(double* src, double* dst, size_t rowCount, size_t columnCount)
{
	__m512d* const pSrc = reinterpret_cast<__m512d*>(src);
	__m512d* const pDst = reinterpret_cast<__m512d*>(dst);
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount / VECTOR_SIZE;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;

	for (int i = 0; i < nC; i++)
	{
		for (int j = 0; j < nR; j++)
		{
			//
			__m512d* p1 = pSrc + (i * rowCount) + j;  // source pointer use internal counter = j for horizontal advance, external counter = i for vertical advance
			__m512d* p2 = pDst + (j * rowCount) + i;  // destination pointer vice versa
			//
			// __m512d* p1 = pSrc + (j * rowCount) + i;     // vice versa
			// __m512d* p2 = pDst + (i * rowCount) + j;     // vice versa
			//
			   transposeTemporal(p1, p2, bytesPerRow);
			// transposeBigToLittle(p1, p2, bytesPerRow);
			// transposeLittleToBig(p1, p2, bytesPerRow);
			//
		}
	}
}
void Task_06::transposeAvx512multiThread(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
{
	rowCountStore = rowCount;
	pSrcStore = reinterpret_cast<__m512d*>(src);
	pDstStore = reinterpret_cast<__m512d*>(dst);
	nRstore = rowCount / VECTOR_SIZE;
	bytesPerRowStore = columnCount * ELEMENT_SIZE;
	hardwareThreads = std::thread::hardware_concurrency();
	usedThreads = columnCount / VECTOR_SIZE;

	std::vector<std::thread> threads = {};
	for (size_t i = 0; i < usedThreads; i++)
	{
		void* threadPtr = reinterpret_cast<void*>(i);
		threads.push_back(std::thread(threadFunction, threadPtr));  // Create and run N threads, N = software concurrency.
	}
	for (size_t i = 0; i < usedThreads; ++i)
	{
		threads[i].join();   // Wait for N threads.
	}
}
void Task_06::threadFunction(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	for (int j = 0; j < nRstore; j++)
	{
		//
		// __m512d* p1 = pSrcStore + (i * rowCountStore) + j;  // source pointer use internal counter = j for horizontal advance, THREAD counter = i for vertical advance
		// __m512d* p2 = pDstStore + (j * rowCountStore) + i;  // destination pointer vice versa
		//
		__m512d* p1 = pSrcStore + (j * rowCountStore) + i;     // vice versa
		__m512d* p2 = pDstStore + (i * rowCountStore) + j;     // vice versa
		//
		   transposeTemporal(p1, p2, bytesPerRowStore);
		// transposeBigToLittle(p1, p2, bytesPerRowStore);
		// transposeLittleToBig(p1, p2, bytesPerRow);
		//
	}
}
void Task_06::transposeAvx512singleThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount)
{
	__m512d* const pSrc = reinterpret_cast<__m512d*>(src);
	__m512d* const pDst = reinterpret_cast<__m512d*>(dst);
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount / VECTOR_SIZE;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;

	for (int i = 0; i < nC; i++)
	{
		for (int j = 0; j < nR; j++)
		{
			//
			__m512d* p1 = pSrc + (i * rowCount) + j;  // source pointer use internal counter = j for horizontal advance, external counter = i for vertical advance
			__m512d* p2 = pDst + (j * rowCount) + i;  // destination pointer vice versa
			//
			// __m512d* p1 = pSrc + (j * rowCount) + i;     // vice versa
			// __m512d* p2 = pDst + (i * rowCount) + j;     // vice versa
			//
			   transposeNonTemporal(p1, p2, bytesPerRow);
			// transposeBigToLittleNT(p1, p2, bytesPerRow);
			// transposeLittleToBigNT(p1, p2, bytesPerRow);
			//
		}
	}
}
void Task_06::transposeAvx512multiThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
{
	rowCountStore = rowCount;
	pSrcStore = reinterpret_cast<__m512d*>(src);
	pDstStore = reinterpret_cast<__m512d*>(dst);
	nRstore = rowCount / VECTOR_SIZE;
	bytesPerRowStore = columnCount * ELEMENT_SIZE;
	hardwareThreads = std::thread::hardware_concurrency();
	usedThreads = columnCount / VECTOR_SIZE;

	std::vector<std::thread> threads = {};
	for (size_t i = 0; i < usedThreads; i++)
	{
		void* threadPtr = reinterpret_cast<void*>(i);
		threads.push_back(std::thread(threadFunctionNT, threadPtr));  // Create and run N threads, N = software concurrency.
	}
	for (size_t i = 0; i < usedThreads; ++i)
	{
		threads[i].join();   // Wait for N threads.
	}
}
void Task_06::threadFunctionNT(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	for (int j = 0; j < nRstore; j++)
	{
		//
		// __m512d* p1 = pSrcStore + (i * rowCountStore) + j;  // source pointer use internal counter = j for horizontal advance, THREAD counter = i for vertical advance
		// __m512d* p2 = pDstStore + (j * rowCountStore) + i;  // destination pointer vice versa
		//
		__m512d* p1 = pSrcStore + (j * rowCountStore) + i;     // vice versa
		__m512d* p2 = pDstStore + (i * rowCountStore) + j;     // vice versa
		//
		   transposeNonTemporal(p1, p2, bytesPerRowStore);
		// transposeBigToLittleNT(p1, p2, bytesPerRowStore);
		// transposeLittleToBigNT(p1, p2, bytesPerRow);
		//
	}
}
void Task_06::showResults(double* ptr, DWORD64 hzTSC, DWORD64 dTSC, double gigabytes)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double seconds = 1.0 / static_cast<double>(hzTSC) * dTSC;
	double gbps = gigabytes / seconds;
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "%.3f GB per %.3f seconds = %.3f GBPS.\r\n", gigabytes, seconds, gbps);
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}
void Task_06::fill(double* ptr, size_t elementCount, double x0, double dx)
{
	for (size_t i = 0; i < elementCount; i++)
	{
		*ptr = x0;
		ptr++;
		x0 += dx;
	}
}
BOOL Task_06::verify(double* ptr, size_t rowCount, size_t columnCount, BOOL transpose, double x0, double dx)
{
	BOOL passed = TRUE;
	if (transpose)
	{
		for (int i = 0; (i < rowCount) && passed; i++)
		{
			for (int j = 0; (j < columnCount) && passed; j++)
			{
				double* ptr1 = ptr + i + j * columnCount;
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
		for (int i = 0; (i < rowCount) && passed; i++)
		{
			for (int j = 0; (j < columnCount) && passed; j++)
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
BOOL Task_06::detectAvx512()
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
void Task_06::dump8x8(void* data)
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
void Task_06::dump16x16(void* data)
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

__m512d* Task_06::pSrcStore;
__m512d* Task_06::pDstStore;
size_t Task_06::rowCountStore;
size_t Task_06::nRstore;
size_t Task_06::bytesPerRowStore;

#endif