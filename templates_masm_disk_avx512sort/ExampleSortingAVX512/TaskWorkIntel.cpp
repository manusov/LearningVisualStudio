/*

Sorting algorithm debug.
Quick sort and sorting networks with AVX512 instructions.
Starts with variant for double precision numbers.

This project based on Intel sources:
https://github.com/intel/x86-simd-sort
https://github.com/berenger-eu/avx-512-sort/tree/master
Use example at document:
1704.08579.pdf. Appendix B, code 2.
See also:
LIPIcs.SEA.2021.3.pdf.

Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Same template used by: Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories

Task works class implementation.
Class used for array sorting benchmarking scenarios.
Compare std::sort and INTEL example.

*/

#include "INTEL\avx512-64bit-qsort.hpp"
#include "TaskWorkIntel.h"

STATUS_CODES TaskWorkIntel::runTask(COMMAND_LINE_PARMS* p)
{
	char msg[APPCONST::MAX_TEXT_STRING];

	// Initializing timer.
	LARGE_INTEGER f{ 0 };
	if ((!QueryPerformanceFrequency(&f)) || (f.QuadPart <= 0))
	{
		AppLib::writeParmError("OS performance counter not detected or failed at initialization");
		return TIMER_FAILED;

	}
	double perfSeconds = 1.0 / f.QuadPart;

	// Initializing memory.
	size_t startCount = p->optionStartCount;
	size_t endCount = p->optionEndCount;
	size_t deltaCount = p->optionDeltaCount;
	DATA_TYPE dataMode = static_cast<DATA_TYPE>(p->optionData);
	if ((startCount >= BLOCK_COUNT_MAX) || (endCount >= BLOCK_COUNT_MAX) || (deltaCount >= BLOCK_COUNT_MAX))
	{
		AppLib::writeParmError("Elements count is too big");
		return INTERNAL_ERROR;
	}
	double* dataRaw = reinterpret_cast<double*>(_aligned_malloc(BLOCK_BYTES, AVX512_ALIGNMENT));
	if (!dataRaw)
	{
		AppLib::writeParmError("Memory allocation error");
		return MEMORY_ALLOCATION_FAILED;
	}

	// Initializing measurement cycle.
	STATUS_CODES exitCode = NO_ERRORS;
	const char* szError = "ERROR";
	const char* szOk = "OK";
	AppLib::writeColor(" Sorting timings comparision.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" #     Count        Status       std::sort     INTEL (seconds)    Ratio\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(TABLE_WIDTH, APPCONST::TABLE_COLOR);
	size_t blockCount;
	int index = 1;

	std::vector<double> stdStatistics;
	std::vector<double> avxStatistics;
	std::vector<double> ratioStatistics;

	// Measurement cycle.
	for (blockCount = startCount; blockCount <= endCount; blockCount += deltaCount)
	{
		std::vector<double> dataVector = std::vector<double>(blockCount);

		// Generate random data.
		buildRandom(dataVector, dataRaw, blockCount, dataMode);

		// Measure std::sort performance.
		double stdSeconds = 0.0;
		LARGE_INTEGER t1{ 0 }, t2{ 0 };
		BOOL timerStatus = QueryPerformanceCounter(&t1);  // value for wait start interval
		if (timerStatus)
		{
			t2.QuadPart = t1.QuadPart;
			timerStatus = TRUE;
			while (timerStatus && (t2.QuadPart == t1.QuadPart))
			{
				timerStatus = QueryPerformanceCounter(&t2);  // wait start interval
			}
			if (timerStatus)
			{
				timerStatus = QueryPerformanceCounter(&t2);  // start interval
				if (timerStatus)
				{
					
					std::sort(dataVector.begin(), dataVector.end());           // Sort by std::sort.
					
					timerStatus = QueryPerformanceCounter(&t1);  // end interval
					if (timerStatus)
					{
						stdSeconds = (t1.QuadPart - t2.QuadPart) * perfSeconds;
					}
				}
			}
		}

		// Measure AVX512 QSORT performance.
		const char* avxStatus = szOk;
		double avxSeconds = 0.0;
		if (timerStatus)
		{
			__m512d* arrayBase = reinterpret_cast<__m512d*>(dataRaw);
			t2.QuadPart = t1.QuadPart;
			timerStatus = TRUE;
			while (timerStatus && (t2.QuadPart == t1.QuadPart))
			{
				timerStatus = QueryPerformanceCounter(&t2);  // wait start interval
			}
			if (timerStatus)
			{
				timerStatus = QueryPerformanceCounter(&t2);  // start interval
				if (timerStatus)
				{
					
					avx512_qsort(dataRaw, blockCount);           // Sort by advanced INTEL library.
					
					timerStatus = QueryPerformanceCounter(&t1);  // end interval
					if (timerStatus)
					{
						avxSeconds = (t1.QuadPart - t2.QuadPart) * perfSeconds;
					}
				}
			}
		}

		if (!timerStatus)
		{
			printf("\r\nOS performance counter failed at runtime.\r\n");
			exitCode = TIMER_FAILED;
			break;
		}

		// Check is sorted data correct.
		if (verifyData(dataVector, dataRaw, blockCount))
		{
			avxStatus = szError;
			exitCode = SORTING_DATA_MISMATCH;
		}

		// Store statistics.
		double ratioSeconds = stdSeconds / avxSeconds;
		stdStatistics.push_back(stdSeconds);
		avxStatistics.push_back(avxSeconds);
		ratioStatistics.push_back(ratioSeconds);

		// Write results of iteration.
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			" %-6d%-13d%-13s%-14.7f%-19.7f%-12.3f\r\n",
			index, static_cast<int>(blockCount), avxStatus, stdSeconds, avxSeconds, ratioSeconds);
		AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
		index++;
	}

	// Close measurement cycle.
	AppLib::writeColorLine(TABLE_WIDTH, APPCONST::TABLE_COLOR);

	// Write performance statistics.
	AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Operation           min         max         average     median\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	writeStatistics(msg, "std::sort", stdStatistics, true);
	writeStatistics(msg, "INTEL AVX512", avxStatistics, true);
	writeStatistics(msg, "Time ratio", ratioStatistics, true);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);

	// Exit.
	if (dataRaw) _aligned_free(dataRaw);
	return exitCode;
}
