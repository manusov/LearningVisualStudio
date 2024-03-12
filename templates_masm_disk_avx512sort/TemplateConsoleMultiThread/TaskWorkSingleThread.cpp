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
Child class, used for single-thread scenario,

*/

#include "TaskWorkSingleThread.h"

STATUS_CODES TaskWorkSingleThread::checkThreads(COMMAND_LINE_PARMS* pCommandLineParms)
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
		THREAD_ENTRY* pTlist = reinterpret_cast<THREAD_ENTRY*>(_aligned_malloc(sizeof(THREAD_ENTRY), LIST_ALIGNMENT));
		if (!pTlist)
		{
			return MEMORY_ALLOCATION_FAILED;
		}
		if (pattern == CHECK_AFFINITY)
		{
			DWORD_PTR oldAffinity = SetThreadAffinityMask(GetCurrentThread(), 1);
			pTlist->apicId = getAPICID();
			pTlist->extendedApicId = getExtendedAPICID();
			SetThreadAffinityMask(GetCurrentThread(), oldAffinity);
		}
		else
		{
			pTlist->apicId = getAPICID();
			pTlist->extendedApicId = getExtendedAPICID();
		}
		printThreadList(pTlist, 1, extendedValid);
		_aligned_free(pTlist);
		return NO_ERRORS;
	}
	else if (pattern == READ_AVX512)
	{
		if (!( (checkTSC() && checkAVX512() )))
		{
			return FEATURE_NOT_SUPPORTED;
		}
		AppLib::writeColor("Measure TSC clock...", APPCONST::TABLE_COLOR);
		LARGE_INTEGER hzPc{0}, hzTsc{0};
		if (!Timer::precisionMeasure(hzPc, hzTsc))
		{
			return TIMER_FAILED;
		}
		THREAD_ENTRY* pTlist = reinterpret_cast<THREAD_ENTRY*>(_aligned_malloc(sizeof(THREAD_ENTRY), LIST_ALIGNMENT));
		if (!pTlist)
		{
			return MEMORY_ALLOCATION_FAILED;
		}
		void* ptr = _aligned_malloc(COUNT * UNIT, PAGE);
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
		AppLib::writeColor("\r\nMeasuring AVX512 single-thread L1-ZMM read bandwidth...", APPCONST::TABLE_COLOR);

		DWORD64 deltaTsc = 0;
		readAvx512(ptr, COUNT, REPEATS, deltaTsc);

		double gigabytes = static_cast<double>(COUNT * UNIT * REPEATS) / (1024 * 1024 * 1024);
		double seconds = static_cast<double>(deltaTsc) * tTsc;
		double gbps = gigabytes / seconds;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nSingle-thread GBPS = %.3f GBPS.\r\n", gbps);
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