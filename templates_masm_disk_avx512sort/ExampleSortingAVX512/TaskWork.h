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

Task works class header.
Class used for array sorting benchmarking scenarios.
Compare std::sort and fragment under debug.

*/

#pragma once
#ifndef TASKWORK_H
#define TASKWORK_H

#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <immintrin.h>
#include "Global.h"
#include "AppLib.h"
#include "AppGuiLib.h"

#define _USE_ASM_V3
#define _USE_ASM_V2

#ifdef _USE_ASM_V3
extern "C" int __stdcall asmSortingPartitioned_v3(__m512d * ptr, int count);
#elif defined _USE_ASM_V2
extern "C" int __stdcall asmSortingPartitioned_v2(__m512d * ptr, int count);
#else
extern "C" int __stdcall asmSortingPartitioned_v1(__m512d * ptr, int count);
#endif

class TaskWork
{
public:
    static STATUS_CODES checkFeatures(COMMAND_LINE_PARMS* p);
    static STATUS_CODES runTask(COMMAND_LINE_PARMS* p);         // Target operation.

protected:
    static void buildRandom(std::vector<double> &dataVector, double* dataRaw, size_t blockCount, DATA_TYPE dataType);
    static BOOL verifyData(std::vector<double> &dataVector, double* dataRaw, size_t blockCount);
    static void writeStatistics(char* msg, const char* statisticsName, std::vector<double> &values, bool tableMode);

    static constexpr size_t BLOCK_COUNT_MAX = 10 * 1024 * 1024;
    static constexpr size_t BLOCK_BYTES = BLOCK_COUNT_MAX * sizeof(double);
    static constexpr size_t AVX512_ALIGNMENT = 64;
    static constexpr int RND_MASK = 0x7FFFFFFF;
    static constexpr int TABLE_WIDTH = 76;

private:
    static BOOL detectRdrand();
    static BOOL detectAvx512();
    static BOOL detectAvx512vl();
};


#endif  // TASKWORK_H


