/*

Sorting algorithm debug.
Sorting networks with AVX2 instructions.
Variant for integers.

This project based on sources:
https://github.com/simd-sorting/fast-and-robust

See also Intel sources:
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

AVX2 optimization enabled for compiling this project. Properties \ C/C++ \ Create code \ ...

*/

#include "avx2sort.h"
#include "TaskWork.h"

STATUS_CODES TaskWork::checkFeatures(COMMAND_LINE_PARMS* p)
{
	STATUS_CODES status = FEATURE_NOT_SUPPORTED;
	if (detectAvx2())
	{
		if ((p->optionData != DATA_HRND) || (detectRdrand()))
		{
			status = NO_ERRORS;
		}
		else
		{
			AppLib::writeParmError("RDRAND instruction is not supported or disabled");
		}
	}
	else
	{
		AppLib::writeParmError("AVX2 or OS YMM-context management is not supported or disabled");
	}
	return status;
}
STATUS_CODES TaskWork::runTask(COMMAND_LINE_PARMS* p)
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
	// Initializing measurement cycle.
	STATUS_CODES exitCode = NO_ERRORS;
	const char* szError = "ERROR";
	const char* szOk = "OK";
	AppLib::writeColor(" Sorting timings comparision.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" #     Count        Status       std::sort     AVX2 (seconds)     Ratio\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(TABLE_WIDTH, APPCONST::TABLE_COLOR);
	size_t blockCount;
	int index = 1;

	std::vector<double> stdStatistics;
	std::vector<double> avxStatistics;
	std::vector<double> ratioStatistics;
	
	// Measurement cycle.
	for (blockCount = startCount; blockCount <= endCount; blockCount += deltaCount)
	{
		std::vector<int> dataVector1 = std::vector<int>(blockCount);
		std::vector<int> dataVector2 = std::vector<int>(blockCount);

		// Generate random data.
		buildRandom(dataVector1, dataVector2, blockCount, dataMode);

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
					
					std::sort(dataVector1.begin(), dataVector1.end());        // Sort by std::sort.
					
					timerStatus = QueryPerformanceCounter(&t1);  // end interval
					if (timerStatus)
					{
						stdSeconds = (t1.QuadPart - t2.QuadPart) * perfSeconds;
					}
				}
			}
		}

		// Measure AVX2 sorting performance.
		const char* avxStatus = szOk;
		double avxSeconds = 0.0;
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

					/* sort with avx2 */
					avx2::quicksort(dataVector2.data(), blockCount);        // Sort by AVX2.

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
		if ( verifyData(dataVector1, dataVector2, blockCount) )
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
	writeStatistics(msg, "AVX2 sort", avxStatistics, true);
	writeStatistics(msg, "Time ratio", ratioStatistics, true);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	return exitCode;
}
void TaskWork::buildRandom(std::vector<int> &data1, std::vector<int> &data2, size_t blockCount, DATA_TYPE dataType)
{
	int n = static_cast<int>(blockCount);
	switch (dataType)
	{
	case DATA_ZERO:
		for (int i = 0; i < n; i++) { data1[i] = data2[i] = 0; }
		break;
	case DATA_ONES:
		for (int i = 0; i < n; i++) { data1[i] = data2[i] = 1; }
		break;
	case DATA_INC:
		for (int i = 0; i < n; i++) { data1[i] = data2[i] = i; }
		break;
	case DATA_DEC:
		for (int i = 0; i < n; i++) { data1[i] = data2[i] = n - i; }
		break;
	case DATA_SRND:
		{
			DWORD64 seed = APPCONST::RANDOM_SEED;
			for (int i = 0; i < n; i++)
			{
				seed *= APPCONST::RANDOM_MULTIPLIER;
				seed += APPCONST::RANDOM_ADDEND;
				data1[i] = data2[i] = static_cast<int>(seed & RND_MASK);
			}
		}
		break;
	case DATA_HRND:
		for (int i = 0; i < n; i++)
		{
			unsigned int rndValue = 0;
			while (_rdrand32_step(&rndValue) == 0);
			int a = static_cast<int>(rndValue & RND_MASK);
			data1[i] = data2[i] = a;
		}
		break;
	default:
		break;
	}
}
BOOL TaskWork::verifyData(std::vector<int> &data1, std::vector<int> &data2, size_t blockCount)
{
	BOOL error = FALSE;
	int n = static_cast<int>(blockCount);
	for (int i = 0; i < n; i++)
	{
		if (data1[i] != data2[i]) error = TRUE;
	}
	return error;
}
BOOL TaskWork::detectRdrand()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 1)
	{
		__cpuid(regs, 1);
		constexpr int MASK_RDRAND = 1 << 30;
		if ((regs[2] & MASK_RDRAND) == MASK_RDRAND)
		{
			status = TRUE;
		}
	}
	return status;
}
BOOL TaskWork::detectAvx512()
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
BOOL TaskWork::detectAvx2()
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
			constexpr int MASK_AVX2 = 1 << 5;
			if ((regs[1] & MASK_AVX2) == MASK_AVX2)
			{
				DWORD64 xcr0 = _xgetbv(0);
				constexpr DWORD64 MASK_XCR0 = 0x06;
				if ((xcr0 & MASK_XCR0) == MASK_XCR0)
				{
					status = TRUE;
				}
			}
		}
	}
	return status;
}
void TaskWork::writeStatistics(char* msg, const char* statisticsName, std::vector<double> &values, bool tableMode)
{
	double min = 0.0, max = 0.0, average = 0.0, median = 0.0;
	AppLib::calculateStatistics(values, min, max, average, median);
	snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-13s", statisticsName);
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
	if (tableMode)
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"%12.3f%12.3f%12.3f%12.3f\r\n", min, max, average, median);
	}
	else
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"min = %.3f,  max = %.3f,  average = %.3f,  median = %.3f.\r\n",
			min, max, average, median);
	}
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}
