/*
AVX512 matrix transpose, 8x16 base block,
measurement scenario for small (temporal) and big (nontemporal) blocks.
Yet for quadrat matrices only.
TODO. Compare results Task_11.cpp and Task_10.cpp.
*/

#include "Task_11.h"

constexpr DWORD64 SMALL_MIN     = 128;
constexpr DWORD64 SMALL_MAX     = 2048;
constexpr DWORD64 SMALL_STEP    = 128;
constexpr DWORD64 SMALL_REPEATS = 300;

constexpr DWORD64 BIG_MIN       = 1024;
constexpr DWORD64 BIG_MAX       = 8192;
constexpr DWORD64 BIG_STEP      = 512;
constexpr DWORD64 BIG_REPEATS   = 100;

constexpr DWORD64 ELEMENT_SIZE   = sizeof(double);
constexpr DWORD32 VECTOR_SIZE    = 16;
constexpr DWORD32 BLOCK_ALIGN    = 64;
constexpr DWORD64 ALLOCATED_SIZE = BIG_MAX * BIG_MAX * ELEMENT_SIZE;

constexpr double X0 = 1.0;
constexpr double DX = 0.1;
constexpr double EPSILON = 1E-7;

extern "C" BOOL __stdcall transposeAVX512(double* src, double* dst, size_t columnsCount, size_t rowsCount);
extern "C" BOOL __stdcall transposeAVX512NT(double* src, double* dst, size_t columnsCount, size_t rowsCount);

int Task_11::execute(int argc, char** argv)
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
			AppLib::writeColor("AVX512 matrix transpose benchmark.\r\n", APPCONST::DUMP_ADDRESS_COLOR);
			double perfMhz = static_cast<double>(hzPc.QuadPart) / 1E6;
			double perfNs = 1E9 / static_cast<double>(hzPc.QuadPart);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "OS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", perfMhz, perfNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			double tscMhz = static_cast<double>(hzTsc.QuadPart) / 1E6;
			double tscNs = 1E9 / static_cast<double>(hzTsc.QuadPart);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Measure TSC by OS performance counter: F=%.3f MHz, T=%.3f ns.\r\n", tscMhz, tscNs);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
			size_t hardwareThreads = std::thread::hardware_concurrency();
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%zd hardware threads.\r\n", hardwareThreads);
			AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

			double* srcPtr = reinterpret_cast<double*>(_aligned_malloc(static_cast<size_t>(ALLOCATED_SIZE), BLOCK_ALIGN));
			double* dstPtr = reinterpret_cast<double*>(_aligned_malloc(static_cast<size_t>(ALLOCATED_SIZE), BLOCK_ALIGN));
			if ((srcPtr) && (dstPtr))
			{
				
				// ---------- Test 1. ----------------------------------------------------------------------------------------------------------
				
				AppLib::writeColor("\r\nTest 1 of 2, small temporal data.\r\n", APPCONST::DUMP_ADDRESS_COLOR);
				AppLib::writeLine(78);
				AppLib::writeColor("  #      Rows    Columns    GB per matrix    ST GBPS    MTE GBPS   MTI GBPS\r\n", APPCONST::TABLE_COLOR);
				AppLib::writeLine(78);
				DWORD64 lineSize = SMALL_MIN;
				int index = 1;
				do {
					DWORD64 rows = lineSize;
					DWORD64 columns = lineSize;
					DWORD64 scalarLength = lineSize * lineSize;
					double matrixGb = static_cast<double>(scalarLength * ELEMENT_SIZE) / 1E9;
					double stGbps = 0.0;
					double mteGbps = 0.0;
					double mtiGbps = 0.0;
					snprintf(msg, APPCONST::MAX_TEXT_STRING,
						" %3d    %5lld    %5lld      %.6f",
						index, lineSize, lineSize, matrixGb);
					AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

					// Test 1.1.

					fill(srcPtr, scalarLength, X0, DX);
					if (!verify(srcPtr, columns, rows, FALSE, X0, DX))
					{
						AppLib::writeParmError("source matrix verification failed, test interrupted");
						exitCode = 4;
						break;
					}
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < SMALL_REPEATS; i++)
					{
						double* src = srcPtr;
						double* dst = dstPtr;
						for (int j = 0; j < (rows / VECTOR_SIZE); j++)
						{
							transposeAVX512(src, dst, columns, rows);
							src += columns * VECTOR_SIZE;
							dst += VECTOR_SIZE;
						}
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 11;
						break;
					}
					double gigabytes = static_cast<double>(columns * rows * ELEMENT_SIZE * SMALL_REPEATS) / 1E9;
					double seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					double gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "         %-6.3f", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// Test 1.2.
					
					fill(dstPtr, scalarLength, 0, 0);
					srcStore = srcPtr;
					dstStore = dstPtr;
					srcAddStore = columns * VECTOR_SIZE;
					dstAddStore = VECTOR_SIZE;
					sizeStore = lineSize;
					dtsc = __rdtsc();
					for (int i = 0; i < SMALL_REPEATS; i++)
					{
						std::vector<std::thread> threads = {};
						for (size_t j = 0; j < (rows / VECTOR_SIZE); j++)
						{
							void* threadPtr = reinterpret_cast<void*>(j);
							threads.push_back(std::thread(threadFunctionSimple, threadPtr));  // Create and run N threads, N = software concurrency.
						}
						for (size_t j = 0; j < (rows / VECTOR_SIZE); ++j)
						{
							threads[j].join();   // Wait for N threads.
						}
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 12;
						break;
					}
					seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "     %-6.3f", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// Test 1.3.

					fill(dstPtr, scalarLength, 0, 0);
					repeatsStore = SMALL_REPEATS;
					dtsc = __rdtsc();
					std::vector<std::thread> threads = {};
					for (size_t j = 0; j < (rows / VECTOR_SIZE); j++)
					{
						void* threadPtr = reinterpret_cast<void*>(j);
						threads.push_back(std::thread(threadFunctionRepeats, threadPtr));  // Create and run N threads, N = software concurrency.
					}
					for (size_t j = 0; j < (rows / VECTOR_SIZE); ++j)
					{
						threads[j].join();   // Wait for N threads.
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 13;
						break;
					}
					seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "     %-6.3f\r\n", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// End of 3 tests: 1.1, 1.2, 1.3.
					
					lineSize += SMALL_STEP;
					index++;
					} while (lineSize <= SMALL_MAX);
				AppLib::writeLine(78);

				// ---------- End of test 1. ---------------------------------------------------------------------------------------------------


				// ---------- Test 2. ----------------------------------------------------------------------------------------------------------

				AppLib::writeColor("\r\nTest 2 of 2, big non-temporal data.\r\n", APPCONST::DUMP_ADDRESS_COLOR);
				AppLib::writeLine(78);
				AppLib::writeColor("  #      Rows    Columns    GB per matrix    ST GBPS    MTE GBPS   MTI GBPS\r\n", APPCONST::TABLE_COLOR);
				AppLib::writeLine(78);
				lineSize = BIG_MIN;
				index = 1;
				do {
					DWORD64 rows = lineSize;
					DWORD64 columns = lineSize;
					DWORD64 scalarLength = lineSize * lineSize;
					double matrixGb = static_cast<double>(scalarLength * ELEMENT_SIZE) / 1E9;
					double stGbps = 0.0;
					double mteGbps = 0.0;
					double mtiGbps = 0.0;
					snprintf(msg, APPCONST::MAX_TEXT_STRING,
						" %3d    %5lld    %5lld      %.6f",
						index, lineSize, lineSize, matrixGb);
					AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

					// Test 2.1.

					fill(srcPtr, scalarLength, X0, DX);
					if (!verify(srcPtr, columns, rows, FALSE, X0, DX))
					{
						AppLib::writeParmError("source matrix verification failed, test interrupted");
						exitCode = 4;
						break;
					}
					DWORD64 dtsc = __rdtsc();
					for (int i = 0; i < BIG_REPEATS; i++)
					{
						double* src = srcPtr;
						double* dst = dstPtr;
						for (int j = 0; j < (rows / VECTOR_SIZE); j++)
						{
							transposeAVX512NT(src, dst, columns, rows);
							src += columns * VECTOR_SIZE;
							dst += VECTOR_SIZE;
						}
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 21;
						break;
					}
					double gigabytes = static_cast<double>(columns * rows * ELEMENT_SIZE * BIG_REPEATS) / 1E9;
					double seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					double gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "         %-6.3f", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// Test 2.2.

					fill(dstPtr, scalarLength, 0, 0);
					srcStore = srcPtr;
					dstStore = dstPtr;
					srcAddStore = columns * VECTOR_SIZE;
					dstAddStore = VECTOR_SIZE;
					sizeStore = lineSize;
					dtsc = __rdtsc();
					for (int i = 0; i < BIG_REPEATS; i++)
					{
						std::vector<std::thread> threads = {};
						for (size_t j = 0; j < (rows / VECTOR_SIZE); j++)
						{
							void* threadPtr = reinterpret_cast<void*>(j);
							threads.push_back(std::thread(threadFunctionNonTemporal, threadPtr));  // Create and run N threads, N = software concurrency.
						}
						for (size_t j = 0; j < (rows / VECTOR_SIZE); ++j)
						{
							threads[j].join();   // Wait for N threads.
						}
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 22;
						break;
					}
					seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "     %-6.3f", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// Test 2.3.

					fill(dstPtr, scalarLength, 0, 0);
					repeatsStore = BIG_REPEATS;
					dtsc = __rdtsc();
					std::vector<std::thread> threads = {};
					for (size_t j = 0; j < (rows / VECTOR_SIZE); j++)
					{
						void* threadPtr = reinterpret_cast<void*>(j);
						threads.push_back(std::thread(threadFunctionNonTemporalRepeats, threadPtr));  // Create and run N threads, N = software concurrency.
					}
					for (size_t j = 0; j < (rows / VECTOR_SIZE); ++j)
					{
						threads[j].join();   // Wait for N threads.
					}
					dtsc = __rdtsc() - dtsc;
					if (!verify(dstPtr, columns, rows, TRUE, X0, DX))
					{
						AppLib::writeParmError("destination matrix verification failed, test interrupted");
						exitCode = 23;
						break;
					}
					seconds = 1.0 / static_cast<double>(hzTsc.QuadPart) * dtsc;
					gbps = gigabytes / seconds;
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "     %-6.3f\r\n", gbps);
					AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

					// End of 3 tests: 2.1, 2.2, 2.3.

					lineSize += BIG_STEP;
					index++;
				} while (lineSize <= BIG_MAX);
				AppLib::writeLine(78);

				// ---------- End of test 2. ---------------------------------------------------------------------------------------------------

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

void Task_11::threadFunctionSimple(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	// DWORD_PTR mask = 1LL << (i & 0xF);                     // This for debug: for 16 threads only.
	// SetThreadAffinityMask(GetCurrentThread(), mask);
	double* p1 = srcStore + i * srcAddStore;
	double* p2 = dstStore + i * dstAddStore;
	transposeAVX512(p1, p2, sizeStore, sizeStore);
}
void Task_11::threadFunctionRepeats(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	// DWORD_PTR mask = 1LL << (i & 0xF);                     // This for debug: for 16 threads only.
	// SetThreadAffinityMask(GetCurrentThread(), mask);
	double* p1 = srcStore + i * srcAddStore;
	double* p2 = dstStore + i * dstAddStore;
	for (int j = 0; j < repeatsStore; j++)
	{
		transposeAVX512(p1, p2, sizeStore, sizeStore);
	}
}
void Task_11::threadFunctionNonTemporal(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	// DWORD_PTR mask = 1LL << (i & 0xF);                     // This for debug: for 16 threads only.
	// SetThreadAffinityMask(GetCurrentThread(), mask);
	double* p1 = srcStore + i * srcAddStore;
	double* p2 = dstStore + i * dstAddStore;
	transposeAVX512NT(p1, p2, sizeStore, sizeStore);
}
void Task_11::threadFunctionNonTemporalRepeats(void* ptr)
{
	size_t i = reinterpret_cast<size_t>(ptr);
	// DWORD_PTR mask = 1LL << (i & 0xF);                     // This for debug: for 16 threads only.
	// SetThreadAffinityMask(GetCurrentThread(), mask);
	double* p1 = srcStore + i * srcAddStore;
	double* p2 = dstStore + i * dstAddStore;
	for (int j = 0; j < repeatsStore; j++)
	{
		transposeAVX512NT(p1, p2, sizeStore, sizeStore);
	}
}

double* Task_11::srcStore;
double* Task_11::dstStore;
DWORD64 Task_11::sizeStore;
DWORD64 Task_11::srcAddStore;
DWORD64 Task_11::dstAddStore;
DWORD64 Task_11::repeatsStore;


