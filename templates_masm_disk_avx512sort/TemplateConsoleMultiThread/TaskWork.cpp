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
Parent class, common library, used by child classes.

*/

#include "TaskWork.h"

BOOL TaskWork::checkAPIC()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 1)
	{
		__cpuid(regs, 1);
		constexpr int MASK_APIC = 1 << 9;
		status = ((regs[3] & MASK_APIC) == MASK_APIC);
	}
	return status;
}
BOOL TaskWork::checkExtendedAPIC()
{
	int regs[4];
	__cpuid(regs, 0);
	return ((regs[0]) >= 0x0B);
}
DWORD TaskWork::getAPICID()
{
	int regs[4];
	__cpuid(regs, 1);
	return regs[1] >> 24;
}
DWORD TaskWork::getExtendedAPICID()
{
	int regs[4];
	__cpuidex(regs, 0x0B, 0);
	return regs[3];
}
BOOL TaskWork::checkTSC()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 1)
	{
		__cpuid(regs, 1);
		constexpr int MASK_TSC = 1 << 4;
		status = ((regs[3] & MASK_TSC) == MASK_TSC);
	}
	return status;

}
BOOL TaskWork::checkAVX512()
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
void TaskWork::printThreadList(THREAD_ENTRY* pTlist, int tCount, BOOL extendedValid)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor(" Threads list.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (extendedValid)
	{
		
		AppLib::writeColor(" #    APIC ID, 8/32-bit (hex) \r\n", APPCONST::TABLE_COLOR);
		AppLib::writeColorLine(SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
		for (int i = 0; i < tCount; i++)
		{
			DWORD apicId = pTlist[i].apicId;
			DWORD extendedApicId = pTlist[i].extendedApicId;
			snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-5d%02X %08X\r\n", i, apicId, extendedApicId);
			AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
		}
	}
	else
	{
		AppLib::writeColor(" #    APIC ID, 8-bit (hex) \r\n", APPCONST::TABLE_COLOR);
		AppLib::writeColorLine(SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
		for (int i = 0; i < tCount; i++)
		{
			DWORD apicId = pTlist[i].apicId;
			snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-5d%02X\r\n", i, apicId);
			AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
		}
	}
	AppLib::writeColorLine(SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
}