/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class implementation.
Class used for threads management tests and benchmarking.
Child class, used for multi-thread scenario,

*/

#include "TaskWorkMultiThread.h"

STATUS_CODES TaskWorkMultiThread::checkThreads(COMMAND_LINE_PARMS* pCommandLineParms)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	DWORD pattern = pCommandLineParms->optionPattern;

	if ((pattern == CHECK_NO_AFFINITY) || (pattern == CHECK_AFFINITY))
	{
		if (!checkAPIC())
		{
			return FEATURE_NOT_SUPPORTED;
		}
		BOOL extendedValid = checkExtendedAPIC();
		size_t tCount = std::thread::hardware_concurrency();
		if (tCount <= 0)
		{
			return INTERNAL_ERROR;
		}
		THREAD_ENTRY* pTlist = reinterpret_cast<THREAD_ENTRY*>(_aligned_malloc(sizeof(THREAD_ENTRY) * tCount, LIST_ALIGNMENT));
		if (!pTlist)
		{
			return MEMORY_ALLOCATION_FAILED;
		}
		std::vector<std::thread> threads = {};
		for (size_t i = 0; i < tCount; i++)           // Create and run N threads, N = hardware concurrency.
		{
			pTlist[i].groupAffinity.Group = 0;        // TODO. Processor groups support required for <64 logical CPUs..
			pTlist[i].groupAffinity.Mask = 1LL << i;
			THREAD_ENTRY* threadPtr = reinterpret_cast<THREAD_ENTRY*>(pTlist) + i;

			if (pattern == CHECK_AFFINITY)
			{
				threads.push_back(std::thread(threadFunctionWithAffinity, threadPtr));
			}
			else
			{
				threads.push_back(std::thread(threadFunction, threadPtr));
			}
		}
		for (size_t i = 0; i < tCount; ++i)
		{
			threads[i].join();   // Wait for N threads.
		}
		printThreadList(pTlist, static_cast<int>(tCount), extendedValid);
		_aligned_free(pTlist);
		return NO_ERRORS;
	}
	else if (pattern == READ_AVX512)
	{
		if (!((checkTSC() && checkAVX512())))
		{
			return FEATURE_NOT_SUPPORTED;
		}
		AppLib::writeColor("Measure TSC clock...", APPCONST::TABLE_COLOR);
		LARGE_INTEGER hzPc{ 0 }, hzTsc{ 0 };
		if (!Timer::precisionMeasure(hzPc, hzTsc))
		{
			return TIMER_FAILED;
		}
		size_t tCount = std::thread::hardware_concurrency();
		if (tCount <= 0)
		{
			return INTERNAL_ERROR;
		}
		THREAD_ENTRY* pTlist = reinterpret_cast<THREAD_ENTRY*>(_aligned_malloc(sizeof(THREAD_ENTRY) * tCount, LIST_ALIGNMENT));
		if (!pTlist)
		{
			return MEMORY_ALLOCATION_FAILED;
		}
		void* ptr = _aligned_malloc(COUNT * UNIT * tCount, PAGE);
		if (!ptr)
		{
			_aligned_free(pTlist);
			return MEMORY_ALLOCATION_FAILED;
		}

		double fPc = static_cast<double>(hzPc.QuadPart);
		double tPc = 1.0 / fPc;
		double fTsc = static_cast<double>(hzTsc.QuadPart);
		double tTsc = 1.0 / fTsc;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nfPC=%.3f MHz, tPC=%.3f ns, fTSC=%.3f MHz, tTSC=%.3f ns.",
			fPc / 1E6, tPc * 1E9, fTsc / 1E6, tTsc * 1E9);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nMeasuring AVX512 %d-thread L1-ZMM read bandwidth...", static_cast<int>(tCount));
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);

		DWORD64 deltaTsc = __rdtsc();
		std::vector<std::thread> threads = {};
		for (size_t i = 0; i < tCount; i++)
		{
			BYTE* threadPtr = reinterpret_cast<BYTE*>(ptr) + i * COUNT * UNIT;
			threads.push_back(std::thread(threadFunctionAVX512, threadPtr));  // Create and run N threads, N = hardware concurrency.
		}
		for (size_t i = 0; i < tCount; ++i)
		{
			threads[i].join();   // Wait for N threads.
		}
		deltaTsc = __rdtsc() - deltaTsc;

		double gigabytes = static_cast<double>(COUNT * UNIT * REPEATS * tCount) / (1024 * 1024 * 1024);
		double seconds = static_cast<double>(deltaTsc) * tTsc;
		double gbps = gigabytes / seconds;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\n%d-thread GBPS = %.3f GBPS.\r\n", static_cast<int>(tCount), gbps);
		AppLib::writeColor(msg, APPCONST::VALUE_COLOR);

		_aligned_free(ptr);
		_aligned_free(pTlist);
		return NO_ERRORS;
	}
	else
	{
		return INTERNAL_ERROR;   // Unknown selector.
	}
}

void TaskWorkMultiThread::threadFunction(void* ptr)
{
	THREAD_ENTRY* ptrEntry = reinterpret_cast<THREAD_ENTRY*>(ptr);
	ptrEntry->apicId = getAPICID();
	ptrEntry->extendedApicId = getExtendedAPICID();
}

void TaskWorkMultiThread::threadFunctionWithAffinity(void* ptr)
{
	THREAD_ENTRY* ptrEntry = reinterpret_cast<THREAD_ENTRY*>(ptr);
	DWORD_PTR oldAffinity = SetThreadAffinityMask(GetCurrentThread(), ptrEntry->groupAffinity.Mask);
	ptrEntry->apicId = getAPICID();
	ptrEntry->extendedApicId = getExtendedAPICID();
	SetThreadAffinityMask(GetCurrentThread(), oldAffinity);
}

void TaskWorkMultiThread::threadFunctionAVX512(void* ptr)
{
	DWORD64 deltaTsc = 0;
	readAvx512(ptr, COUNT, REPEATS, deltaTsc);
}