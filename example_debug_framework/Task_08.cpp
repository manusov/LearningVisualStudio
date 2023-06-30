/*
Class for AVX512 matrix transpose.
Step 4 = debug for asm cycle with gathered-scattered loads-stores, for x64 builds only.
This variant is without blocking, "naive" lines size selection by matrix rows/columns.
This variant for qudrat matrices only, nRows = nColumns.

TODO.
1) Optimize line size by cache size.
2) Optimize threads count for multi-thread variant.
3) Support non-quadrat matrices.

*/

#include "Task_08.h"

#ifdef NATIVE_WIDTH_64

extern "C" void __stdcall transposeGather(double* src, double* dst, DWORD64 * map, size_t count, size_t srcAdd, size_t dstAdd);
extern "C" void __stdcall transposeScatter(double* src, double* dst, DWORD64 * map, size_t count,  size_t srcAdd, size_t dstAdd);

constexpr size_t MATRIX_ROWS    = 1024; // 48 * 1024;        // 1024;   // 32768;   // 8192;   // 128;  //  32768;
constexpr size_t MATRIX_COLUMNS = 1024; // 48 * 1024;        // 1024;   // 32768;   // 8192;   // 128;  //  32768;
constexpr size_t ELEMENT_SIZE   = sizeof(double);
constexpr size_t ELEMENT_COUNT  = MATRIX_ROWS * MATRIX_COLUMNS;
constexpr size_t BLOCK_ALIGN    = 64;
constexpr size_t VECTOR_SIZE    = 128;

constexpr size_t MEASUREMENT_REPEATS = 10000; // 3;           // 10000;  // 3;  // 20;
constexpr size_t ALLOCATED_SIZE      = ELEMENT_COUNT * ELEMENT_SIZE;
constexpr size_t TRANSFERRED_SIZE    = ALLOCATED_SIZE * MEASUREMENT_REPEATS;
constexpr size_t MAP_SIZE            = 1024;

constexpr double GIGABYTES = static_cast<double>(TRANSFERRED_SIZE) / 1E9;
constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

#endif

int Task_08::execute(int argc, char** argv)
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
			DWORD64* mapPtr = reinterpret_cast<DWORD64*>(_aligned_malloc(MAP_SIZE, BLOCK_ALIGN));
			if (srcPtr && dstPtr && mapPtr)
			{
				buildMap(mapPtr, VECTOR_SIZE, MATRIX_ROWS * ELEMENT_SIZE);
				fill(srcPtr, ELEMENT_COUNT, X0, DX);
				if (verify(srcPtr, MATRIX_ROWS, MATRIX_COLUMNS, FALSE, X0, DX))
				{

					// ---------- Single-thread, gathered read matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose single-thread, gathered...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512singleThreadG(srcPtr, dstPtr, mapPtr, MATRIX_ROWS, MATRIX_COLUMNS);
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

					// ---------- Single-thread, scattered write data matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose single-thread, scattered...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512singleThreadS(srcPtr, dstPtr, mapPtr, MATRIX_ROWS, MATRIX_COLUMNS);
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
						exitCode = 6;
					}

					// ---------- Multi-thread, gathered read matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose multi-thread, gathered...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					size_t hardThreads = 0;
					size_t softThreads = 0;
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512multiThreadG(srcPtr, dstPtr, mapPtr, MATRIX_ROWS, MATRIX_COLUMNS, hardThreads, softThreads);
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

					// ---------- Multi-thread, scattered write matrix transpose benchmark. --------------------------------------------------------------------------
					AppLib::writeColor("Transpose multi-thread, scattered...\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					hardThreads = 0;
					softThreads = 0;
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						transposeAvx512multiThreadS(srcPtr, dstPtr, mapPtr, MATRIX_ROWS, MATRIX_COLUMNS, hardThreads, softThreads);
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
			if (mapPtr) _aligned_free(mapPtr);
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

void Task_08::buildMap(DWORD64* ptr, size_t count, size_t step)
{
	DWORD64 a = 0;
	for (int i = 0; i < count; i++)
	{
		*ptr = a;
		ptr++;
		a += step;
	}
}
void Task_08::transposeAvx512singleThreadG(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount)
{
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = VECTOR_SIZE * bytesPerRow;
	for (int i = 0; i < nC; i++)
	{
		double* p1 = src + i;              // source pointer advance horizontal by external C++ cycle, advance vertical by internal asm cycle
		double* p2 = dst + i * rowCount;   // destination pointer vice versa
		transposeGather(p1, p2, map, nR, V_ADD, H_ADD);
	}
}
void Task_08::transposeAvx512multiThreadG(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
{
	mapStore = map;
	rowCountStore = rowCount;
	pSrcStore = reinterpret_cast<__m512d*>(src);
	pDstStore = reinterpret_cast<__m512d*>(dst);
	nRstore = rowCount / VECTOR_SIZE;
	bytesPerRowStore = columnCount * ELEMENT_SIZE;
	hardwareThreads = std::thread::hardware_concurrency();
	usedThreads = columnCount;

	std::vector<std::thread> threads = {};
	for (size_t i = 0; i < usedThreads; i++)
	{
		void* threadPtr = reinterpret_cast<void*>(i);
		threads.push_back(std::thread(threadFunctionG, threadPtr));  // Create and run N threads, N = software concurrency.
	}
	for (size_t i = 0; i < usedThreads; ++i)
	{
		threads[i].join();   // Wait for N threads.
	}
}
void Task_08::threadFunctionG(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	double* p1 = reinterpret_cast<double*>(pSrcStore) + i;                   // source pointer advance horizontal by external C++ cycle, advance vertical internal asm cycle
	double* p2 = reinterpret_cast<double*>(pDstStore) + i * rowCountStore;   // destination pointer vice versa
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = VECTOR_SIZE * bytesPerRowStore;
	transposeGather(p1, p2, mapStore, nRstore, V_ADD, H_ADD);
}
void Task_08::transposeAvx512singleThreadS(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount)
{
	const size_t nR = rowCount / VECTOR_SIZE;
	const size_t nC = columnCount;
	const size_t bytesPerRow = columnCount * ELEMENT_SIZE;
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = VECTOR_SIZE * bytesPerRow;
	for (int i = 0; i < nC; i++)
	{
		double* p1 = src + i * rowCount;   // source pointer advance vertical by external C++ cycle, advance horizontal by internal asm cycle
		double* p2 = dst + i;              // destination pointer vice versa
		transposeScatter(p1, p2, map, nR, V_ADD, H_ADD);
	}
}
void Task_08::transposeAvx512multiThreadS(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads)
{
	mapStore = map;
	rowCountStore = rowCount;
	pSrcStore = reinterpret_cast<__m512d*>(src);
	pDstStore = reinterpret_cast<__m512d*>(dst);
	nRstore = rowCount / VECTOR_SIZE;
	bytesPerRowStore = columnCount * ELEMENT_SIZE;
	hardwareThreads = std::thread::hardware_concurrency();
	usedThreads = columnCount;

	std::vector<std::thread> threads = {};
	for (size_t i = 0; i < usedThreads; i++)
	{
		void* threadPtr = reinterpret_cast<void*>(i);
		threads.push_back(std::thread(threadFunctionS, threadPtr));  // Create and run N threads, N = software concurrency.
	}
	for (size_t i = 0; i < usedThreads; ++i)
	{
		threads[i].join();   // Wait for N threads.
	}
}
void Task_08::threadFunctionS(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	double* p1 = reinterpret_cast<double*>(pSrcStore) + i * rowCountStore;   // source pointer advance vertical by external C++ cycle, advance horizontal internal asm cycle
	double* p2 = reinterpret_cast<double*>(pDstStore) + i;                   // destination pointer vice versa
	const size_t H_ADD = VECTOR_SIZE * ELEMENT_SIZE;
	const size_t V_ADD = VECTOR_SIZE * bytesPerRowStore;
	transposeScatter(p1, p2, mapStore, nRstore, V_ADD, H_ADD);
}

DWORD64* Task_08::mapStore;

#endif