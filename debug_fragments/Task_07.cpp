/*
Class for AVX512 matrix transpose.
Step 3 = debug for cycle with cells 8x8, for x64 builds only.
Variant with manipulations at ZMM registers with 8x8 matrix,
load and store 512-bit operands without scatter-gather.
Row-columns cycles supported by assembler code.
*/

#include "Task_07.h"

#ifdef NATIVE_WIDTH_64

extern "C" void __stdcall transposeCycledTemporal(__m512d * src, __m512d * dst, size_t rowSize, size_t srcAdd, size_t dstAdd, size_t count);
extern "C" void __stdcall transposeCycledNonTemporal(__m512d * src, __m512d * dst, size_t rowSize, size_t srcAdd, size_t dstAdd, size_t count);

constexpr size_t MATRIX_ROWS    = 1024;  // 48 * 1024;  // 1024;   // 32768;   // 16; // 32768;
constexpr size_t MATRIX_COLUMNS = 1024;  //  48 * 1024;  // 1024;   // 32768;   // 16; // 32768;
constexpr size_t ELEMENT_SIZE   = sizeof(double);
constexpr size_t ELEMENT_COUNT  = MATRIX_ROWS * MATRIX_COLUMNS;
constexpr size_t BLOCK_ALIGN    = 64;
constexpr size_t VECTOR_SIZE    = 8;

constexpr size_t MEASUREMENT_REPEATS = 10000; //  3; // 10000;  // 3;
constexpr size_t ALLOCATED_SIZE      = ELEMENT_COUNT * ELEMENT_SIZE;
constexpr size_t TRANSFERRED_SIZE    = ALLOCATED_SIZE * MEASUREMENT_REPEATS;

constexpr double GIGABYTES = static_cast<double>(TRANSFERRED_SIZE) / 1E9;
constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

#endif

int Task_07::execute(int argc, char** argv)
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
					AppLib::writeColor("Transpose single-thread, internal asm cycle, temporal data...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512singleThread(srcPtr, dstPtr, MATRIX_ROWS, MATRIX_COLUMNS);
					}
					dtsc = __rdtsc() - dtsc;
					/* DEBUG
					AppLib::write("\r\n");
					dump16x16(srcPtr);
					AppLib::write("\r\n");
					dump16x16(dstPtr);
					DEBUG */
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
					AppLib::writeColor("Transpose single-thread, internal asm cycle, non-temporal data...\r\n", APPCONST::TABLE_COLOR);
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
					AppLib::writeColor("Transpose multi-thread, internal asm cycle, temporal data...\r\n", APPCONST::TABLE_COLOR);
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
					AppLib::writeColor("Transpose multi-thread, internal asm cycle, non-temporal data...\r\n", APPCONST::TABLE_COLOR);
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

void Task_07::transposeAvx512singleThread(double* src, double* dst, size_t rowCount, size_t columnCount)
{
	__m512d* const pSrc = reinterpret_cast<__m512d*>(src);
	__m512d* const pDst = reinterpret_cast<__m512d*>(dst);
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount / VECTOR_SIZE;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = bytesPerRow * VECTOR_SIZE;
	for (int i = 0; i < nC; i++)
	{
		__m512d* p1 = pSrc + i * rowCount;  // source pointer advance vertical by external C++ cycle, advance horizontal internal asm cycle
		__m512d* p2 = pDst + i;             // destination pointer vice versa
		transposeCycledTemporal(p1, p2, bytesPerRow, H_ADD, V_ADD, nR);
	}
}
void Task_07::transposeAvx512multiThread(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
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
void Task_07::threadFunction(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	__m512d* p1 = pSrcStore + i * rowCountStore;  // source pointer advance vertical by external C++ cycle, advance horizontal internal asm cycle
	__m512d* p2 = pDstStore + i;                  // destination pointer vice versa
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = bytesPerRowStore * VECTOR_SIZE;
	transposeCycledTemporal(p1, p2, bytesPerRowStore, H_ADD, V_ADD, nRstore);
}
void Task_07::transposeAvx512singleThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount)
{
	__m512d* const pSrc = reinterpret_cast<__m512d*>(src);
	__m512d* const pDst = reinterpret_cast<__m512d*>(dst);
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount / VECTOR_SIZE;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = bytesPerRow * VECTOR_SIZE;
	for (int i = 0; i < nC; i++)
	{
		__m512d* p1 = pSrc + i * rowCount;  // source pointer advance vertical by external C++ cycle, advance horizontal internal asm cycle
		__m512d* p2 = pDst + i;             // destination pointer vice versa
		transposeCycledNonTemporal(p1, p2, bytesPerRow, H_ADD, V_ADD, nR);
	}
}
void Task_07::transposeAvx512multiThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
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
void Task_07::threadFunctionNT(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	__m512d* p1 = pSrcStore + i * rowCountStore;  // source pointer advance vertical by external C++ cycle, advance horizontal internal asm cycle
	__m512d* p2 = pDstStore + i;                  // destination pointer vice versa
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = bytesPerRowStore * VECTOR_SIZE;
	transposeCycledNonTemporal(p1, p2, bytesPerRowStore, H_ADD, V_ADD, nRstore);
}

#endif