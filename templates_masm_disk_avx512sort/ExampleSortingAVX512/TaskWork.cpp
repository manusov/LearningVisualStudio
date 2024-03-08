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

TODO.

 1.1) BUG(!):
      Sorting too long execution if all elements is equal, can view as hung, but too long execution.
      Detect situation: next pivot = previous pivot,  or  next pivot <= previous pivot,
	  this can also increase performance for random.
	  This is performance problem, not hung. Possible solutions:
	  Checks required for instruction:
	  vbroadcastsd zmm0,qword ptr [r12]    ; XMM0[63-0] = Pivot value. *_v1.asm line 91-93.

 1.2) Verification required at regular patterns, for example: all equal, all incremental, all decremental.
      BUG(!):
	  Stack overflow at all incremental, all decremental. 
	  Recursion problem, don't occured at small data.

 2.1) Statistics calculation, use AppLib functions.
 2.2) Texts, colors corrections, write ratio.

 3.1) Algorithm optimization. For example sorting network with 16 elements instead 8 elements at current version.
 3.2) Coding optimization.
 3.3) Check support other required CPU features, yet AVX512F only. Check for support RDRAND instruction if required.
 3.4) Enable AVX512 instruction set at Visual Studio settings if use non pure-assembler fragments.

AVX512 optimization enabled for compiling this project. Properties \ C/C++ \ Create code \ ...
Some speed differences detected for C++ compiled code.

*/

#include "TaskWork.h"

STATUS_CODES TaskWork::checkFeatures(COMMAND_LINE_PARMS* p)
{
	STATUS_CODES status = FEATURE_NOT_SUPPORTED;
	if (detectAvx512() && detectAvx512vl())
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
		AppLib::writeParmError("AVX512 + AVX512VL or OS ZMM-context management is not supported or disabled");
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
	AppLib::writeColor(" #     Count        Status       std::sort     AVX512 (seconds)   Ratio\r\n", APPCONST::TABLE_COLOR);
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
					
					std::sort(dataVector.begin(), dataVector.end());       // Sort by std::sort.
					
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

#ifdef _USE_ASM_V3
					asmSortingPartitioned_v3(arrayBase, static_cast<int>(blockCount));       // Sort by algorithms under debug.
#elif defined _USE_ASM_V2
					asmSortingPartitioned_v2(arrayBase, static_cast<int>(blockCount));       // Sort by algorithms under debug.
#else
					asmSortingPartitioned_v1(arrayBase, static_cast<int>(blockCount));       // Sort by algorithms under debug.
#endif

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
		if ( verifyData(dataVector, dataRaw, blockCount) )
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
	writeStatistics(msg, "AVX512 qsort", avxStatistics, true);
	writeStatistics(msg, "Time ratio", ratioStatistics, true);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);

	// Exit.
	if (dataRaw) _aligned_free(dataRaw);
	return exitCode;
}
void TaskWork::buildRandom(std::vector<double> &dataVector, double* dataRaw, size_t blockCount, DATA_TYPE dataType)
{
	int n = static_cast<int>(blockCount);
	switch (dataType)
	{
	case DATA_ZERO:
		for (int i = 0; i < n; i++) { dataVector[i] = dataRaw[i] = 0.0; }
		break;
	case DATA_ONES:
		for (int i = 0; i < n; i++) { dataVector[i] = dataRaw[i] = 1.0; }
		break;
	case DATA_INC:
		for (int i = 0; i < n; i++) { dataVector[i] = dataRaw[i] = static_cast<double>(i); }
		break;
	case DATA_DEC:
		for (int i = 0; i < n; i++) { dataVector[i] = dataRaw[i] = static_cast<double>(n - i); }
		break;
	case DATA_SRND:
		{
			DWORD64 seed = APPCONST::RANDOM_SEED;
			for (int i = 0; i < n; i++)
			{
				seed *= APPCONST::RANDOM_MULTIPLIER;
				seed += APPCONST::RANDOM_ADDEND;
				dataVector[i] = dataRaw[i] = static_cast<double>(seed);
			}
		}
		break;
	case DATA_HRND:
		for (int i = 0; i < n; i++)
		{
			unsigned int rndValue = 0;
			while (_rdrand32_step(&rndValue) == 0);
			double a = static_cast<double>(rndValue & RND_MASK);
			dataVector[i] = dataRaw[i] = a;
		}
		break;
	default:
		break;
	}
}
BOOL TaskWork::verifyData(std::vector<double>& dataVector, double* dataRaw, size_t blockCount)
{
	BOOL error = FALSE;
	int n = static_cast<int>(blockCount);
	for (int i = 0; i < n; i++)
	{
		if (dataRaw[i] != dataVector[i]) error = TRUE;
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
BOOL TaskWork::detectAvx512vl()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 7)
	{
		__cpuidex(regs, 7, 0);
		constexpr int MASK_AVX512VL = 1 << 31;
		if ((regs[1] & MASK_AVX512VL) == MASK_AVX512VL)
		{
			status = TRUE;
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
