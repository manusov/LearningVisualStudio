/*
Class header for AVX512 matrix transpose.
Step 4 = debug for asm cycle with gathered-scattered loads-stores, for x64 builds only.
This variant is without blocking, "naive" lines size selection by matrix rows/columns.
This variant for qudrat matrices only, nRows = nColumns.
*/

#pragma once
#ifndef TASK_08_H
#define TASK_08_H

#include "Task_06.h"

class Task_08 : public Task_06
{
public:
    int execute(int argc, char** argv);

#ifdef NATIVE_WIDTH_64

protected:
    void buildMap(DWORD64* ptr, size_t count, size_t step);
    static DWORD64* mapStore;

private:
    void transposeAvx512singleThreadG(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThreadG(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunctionG(void* ptr);

    void transposeAvx512singleThreadS(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount);
    void transposeAvx512multiThreadS(double* src, double* dst, DWORD64* map, size_t rowCount, size_t columnCount, size_t& hardwareThreads, size_t& usedThreads);
    static void threadFunctionS(void* ptr);

#endif

};

#endif  // TASK_08_H

