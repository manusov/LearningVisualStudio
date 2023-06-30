/*
Class header for AVX512 matrix transpose.
Step 3 = debug for cycle with cells 8x8, for x64 builds only.
Variant with manipulations at ZMM registers with 8x8 matrix,
load and store 512-bit operands without scatter-gather.
Row-columns cycles supported by assembler code.
*/

#pragma once
#ifndef TASK_07_H
#define TASK_07_H

#include "Task_06.h"

class Task_07 : public Task_06
{
public:
    int execute(int argc, char** argv);

#ifdef NATIVE_WIDTH_64

private:
    void transposeAvx512singleThread(double* src, double* dst, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThread(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunction(void* ptr);

    void transposeAvx512singleThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunctionNT(void* ptr);

#endif

};

#endif  // TASK_07_H
