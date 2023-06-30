/*
Class header for AVX512 matrix transpose.
Step 2 = debug for cycle with cells 8x8, for x64 builds only.
Variant with manipulations at ZMM registers with 8x8 matrix,
load and store 512-bit operands without scatter-gather.
Row-columns cycles supported by C++ code, no internal cycles in assembler module.
*/

#pragma once
#ifndef TASK_06_H
#define TASK_06_H

#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_06 : public Task
{
public:
    int execute(int argc, char** argv);

#ifdef NATIVE_WIDTH_64

protected:
    void fill(double* ptr, size_t elementCount, double x0, double dx);
    BOOL verify(double* ptr, size_t rowCount, size_t columnCount, BOOL transpose, double x0, double dx);
    BOOL detectAvx512();
    
    void showResults(double* ptr, DWORD64 hzTSC, DWORD64 dTSC, double gigabytes);
    void dump8x8(void* data);
    void dump16x16(void* data);

    static __m512d* pSrcStore;
    static __m512d* pDstStore;
    static size_t rowCountStore;
    static size_t nRstore;
    static size_t bytesPerRowStore;

private:
    void transposeAvx512singleThread(double* src, double* dst, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThread(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunction(void* ptr);

    void transposeAvx512singleThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThreadNT(double* src, double* dst, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunctionNT(void* ptr);

#endif

};

#endif  // TASK_06_H
