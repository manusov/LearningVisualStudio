/*

Raw experiments.
Similar (but not equal) functionality with Task_06,
but one of classes for support both x64 and ia32 builds.
Yet only quadrat matrices supported.
Note about size limits for ia32 builds.

AVX512 matrix transpose:
8x16 base block, re-create threads at measurement cycle,
8x16 base block, create threads one time, measurement cycle in the threads routines.

This class for interrogation of overheads by re-create threads in each measurement iteration.

*/

#include "Task_10.h"

constexpr DWORD64 MATRIX_ROWS    = 1024; // 2048;
constexpr DWORD64 MATRIX_COLUMNS = MATRIX_ROWS;     // Yet only quadrat matrices supported.
constexpr DWORD64 ELEMENT_SIZE   = sizeof(double);
constexpr DWORD64 ELEMENT_COUNT  = MATRIX_ROWS * MATRIX_COLUMNS;
constexpr DWORD32 BLOCK_ALIGN    = 64;

constexpr DWORD32 ROW_VECTOR_SIZE    = 16;
constexpr DWORD32 COLUMN_VECTOR_SIZE = 16;
constexpr DWORD64 ROW_ITERATIONS     = MATRIX_ROWS / ROW_VECTOR_SIZE;
constexpr DWORD64 COLUMN_ITERATIONS  = MATRIX_COLUMNS / COLUMN_VECTOR_SIZE;

constexpr DWORD64 MEASUREMENT_REPEATS = 200; // 10000;
constexpr DWORD64 ALLOCATED_SIZE      = ELEMENT_COUNT * ELEMENT_SIZE;
constexpr DWORD64 TRANSFERRED_SIZE    = ALLOCATED_SIZE * MEASUREMENT_REPEATS;

constexpr double GIGABYTES_MATRIX  = static_cast<double>(ALLOCATED_SIZE) / 1E9;
constexpr double GIGABYTES_REPEATS = static_cast<double>(TRANSFERRED_SIZE) / 1E9;

constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

// extern "C" BOOL __stdcall transposeAVX512(double* src, double* dst, size_t columnsCount, size_t rowsCount);
extern "C" BOOL __stdcall transposeAVX512NT(double* src, double* dst, size_t columnsCount, size_t rowsCount);

int Task_10::execute(int argc, char** argv)
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
			AppLib::writeColor("Scenario for interrogation of create threads overhead.\r\n", APPCONST::DUMP_ADDRESS_COLOR);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Rows=%lld, columns=%lld, repeats=%lld,\r\none of two matrix size=%.3fGB, include repeats=%.3fGB.\r\n",
				MATRIX_ROWS, MATRIX_COLUMNS, MEASUREMENT_REPEATS, GIGABYTES_MATRIX, GIGABYTES_REPEATS);
			AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);

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
					double summarySingleThread   = -1.0;
					double summaryMultiThreadExt = -1.0;
					double summaryMultiThreadInt = -1.0;
					
					AppLib::writeColor("Source matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					AppLib::writeColor("Destination matrix fill done.\r\n", APPCONST::TABLE_COLOR);

					// ---------- Single-thread ------------------------------------------------------------------------------------------------

					AppLib::writeColor("Transpose single-thread...", APPCONST::TABLE_COLOR);
					// Timing interval start.
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						double* src = srcPtr;
						double* dst = dstPtr;
						for (int j = 0; j < ROW_ITERATIONS; j++)
						{
							transposeAVX512NT(src, dst, MATRIX_COLUMNS, MATRIX_ROWS);
							src += MATRIX_COLUMNS * COLUMN_VECTOR_SIZE;
							dst += ROW_VECTOR_SIZE;
						}
					}
					dtsc = __rdtsc() - dtsc;
					// Timing interval end.
					AppLib::writeColor("done.\r\n", APPCONST::TABLE_COLOR);
					if (verify(dstPtr, MATRIX_COLUMNS, MATRIX_ROWS, TRUE, X0, DX))  // This takes additional time.
					{
						showAndGetGBPS(hzTsc.QuadPart, dtsc, GIGABYTES_REPEATS, summarySingleThread);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 5;
					}

					// ---------- Multi-thread, re-create threads at measurement cycle ---------------------------------------------------------

					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					AppLib::writeColor("Destination matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					size_t hardwareThreads = std::thread::hardware_concurrency();
					size_t softwareThreads = ROW_ITERATIONS;
					srcStore = srcPtr;
					dstStore = dstPtr;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "%zd hardware threads, transpose by %zd threads, external measurement cycle...", hardwareThreads, softwareThreads);
					AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
					// Timing interval start.
					dtsc = __rdtsc();
					for (int i = 0; i < MEASUREMENT_REPEATS; i++)
					{
						std::vector<std::thread> threads = {};
						for (size_t j = 0; j < softwareThreads; j++)
						{
							void* threadPtr = reinterpret_cast<void*>(j);
							threads.push_back(std::thread(threadFunction1, threadPtr));  // Create and run N threads, N = software concurrency.
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
						showAndGetGBPS(hzTsc.QuadPart, dtsc, GIGABYTES_REPEATS, summaryMultiThreadExt);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 5;
					}

					// ---------- Multi-thread, create threads one time, measurement cycle in the threads routines -----------------------------

					fill(dstPtr, ELEMENT_COUNT, 0, 0);
					AppLib::writeColor("Destination matrix fill done.\r\n", APPCONST::TABLE_COLOR);
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "%zd hardware threads, transpose by %zd threads, internal measurement cycle...", hardwareThreads, softwareThreads);
					AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
					// Timing interval start.
					dtsc = __rdtsc();
					std::vector<std::thread> threads = {};
					for (size_t j = 0; j < softwareThreads; j++)
					{
						void* threadPtr = reinterpret_cast<void*>(j);
						threads.push_back(std::thread(threadFunction2, threadPtr));  // Create and run N threads, N = software concurrency.
					}
					for (size_t j = 0; j < softwareThreads; ++j)
					{
						threads[j].join();   // Wait for N threads.
					}
					dtsc = __rdtsc() - dtsc;
					// Timing interval end.
					AppLib::writeColor("done.\r\n", APPCONST::TABLE_COLOR);
					if (verify(dstPtr, MATRIX_COLUMNS, MATRIX_ROWS, TRUE, X0, DX))  // This takes additional time.
					{
						showAndGetGBPS(hzTsc.QuadPart, dtsc, GIGABYTES_REPEATS, summaryMultiThreadInt);
					}
					else
					{
						AppLib::writeParmError("destination matrix elements transpose failed");
						exitCode = 6;
					}

				// ---------- Summary comparisions ---------------------------------------------------------------------------------------------

					if ((summarySingleThread > 0) && (summaryMultiThreadExt > 0) && (summaryMultiThreadInt > 0))
					{
						double pRatio = summaryMultiThreadInt / summarySingleThread;
						double tRatio = summaryMultiThreadInt / summaryMultiThreadExt;
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "Parallelism ratio=%.3f, threads ratio=%.3f.\r\n", pRatio, tRatio);
						AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);
					}
					else
					{
						AppLib::writeParmError("summary information not available because transpose failed");
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
double* Task_10::srcStore;
double* Task_10::dstStore;
void Task_10::threadFunction1(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	double* p1 = srcStore + i * MATRIX_COLUMNS * COLUMN_VECTOR_SIZE;
	double* p2 = dstStore + i * ROW_VECTOR_SIZE;
	transposeAVX512NT(p1, p2, MATRIX_COLUMNS, MATRIX_ROWS);
}
void Task_10::threadFunction2(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	double* p1 = srcStore + i * MATRIX_COLUMNS * COLUMN_VECTOR_SIZE;
	double* p2 = dstStore + i * ROW_VECTOR_SIZE;
	for (int j = 0; j < MEASUREMENT_REPEATS; j++)
	{
		transposeAVX512NT(p1, p2, MATRIX_COLUMNS, MATRIX_ROWS);
	}
}
void Task_10::showAndGetGBPS(DWORD64 hzTSC, DWORD64 dTSC, double gigabytes, double& gbps)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double seconds = 1.0 / static_cast<double>(hzTSC) * dTSC;
	gbps = gigabytes / seconds;
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "%.3f GB per %.3f seconds = %.3f GBPS.\r\n", gigabytes, seconds, gbps);
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}