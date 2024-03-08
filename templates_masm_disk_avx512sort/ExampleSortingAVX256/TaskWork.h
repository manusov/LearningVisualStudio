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

class TaskWork
{
public:
    static STATUS_CODES checkFeatures(COMMAND_LINE_PARMS* p);
    static STATUS_CODES runTask(COMMAND_LINE_PARMS* p);         // Target operation.

protected:
    static void buildRandom(std::vector<int> &data1, std::vector<int>& data2, size_t blockCount, DATA_TYPE dataType);
    static BOOL verifyData(std::vector<int> &data1, std::vector<int>& data2, size_t blockCount);
    static void writeStatistics(char* msg, const char* statisticsName, std::vector<double> &values, bool tableMode);

    static constexpr size_t BLOCK_COUNT_MAX = 10 * 1024 * 1024;
    static constexpr int RND_MASK = 0x7FFFFFFF;
    static constexpr int TABLE_WIDTH = 76;

private:
    static BOOL detectRdrand();
    static BOOL detectAvx512();
    static BOOL detectAvx2();
};


#endif  // TASKWORK_H


