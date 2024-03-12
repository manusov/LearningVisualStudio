/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class header.
Class used for threads management tests and benchmarking.
Parent class, common library, used by child classes.

*/

#pragma once
#ifndef TASKWORK_H
#define TASKWORK_H

#include <windows.h>
#include <intrin.h>
#include <vector>
#include <thread>
#include "Global.h"
#include "AppLib.h"
#include "Timer.h"

constexpr size_t LIST_ALIGNMENT = 64;

typedef struct {
	GROUP_AFFINITY groupAffinity;
	DWORD32 apicId;
	DWORD32 extendedApicId;
	DWORD64 workBase;
	DWORD64 workSize;
} THREAD_ENTRY;

extern "C" void __stdcall readAvx512(void* ptr, DWORD64 count, DWORD64 repeats, DWORD64 & deltaTsc);

class TaskWork
{
protected:
	static BOOL checkAPIC();
	static BOOL checkExtendedAPIC();
	static DWORD getAPICID();
	static DWORD getExtendedAPICID();
	static BOOL checkTSC();
	static BOOL checkAVX512();
	static void printThreadList(THREAD_ENTRY* pTlist, int tCount, BOOL extendedValid);

	static constexpr DWORD64 COUNT = 32;             // 32 * 64 * 16 = 32KB, this means stable cache hits for processor with L1data=48KB.
	static constexpr DWORD64 UNIT = 64 * 16;         // * 16 because cycle unrolled x16 yet without tails support.
	static constexpr DWORD64 REPEATS = 500000000;    // Repeats for measurement.
	static constexpr DWORD64 PAGE = 4096;            // Optimal allocation at page bound.

private:
	static constexpr int SMALL_TABLE_WIDTH = 31;
	static constexpr int BIG_TABLE_WIDTH = 76;

};

#endif  // TASKWORK_H


