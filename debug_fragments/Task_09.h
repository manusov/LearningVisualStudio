/*
Raw experiments.
Similar (but not equal) functionality with Task_06,
but one of classes for support both x64 and ia32 builds.
Yet only quadrat matrices supported.
Note about size limits for ia32 builds.

AVX512 matrix transpose, 8x16 base block, re-create threads at measurement cycle.
*/

#pragma once
#ifndef TASK_09_H
#define TASK_09_H

#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_09 : public Task
{
public:
    int execute(int argc, char** argv);

protected:
    BOOL detectAvx512();
    void fill(double* ptr, DWORD64 elementsCount, double x0, double dx);
    BOOL verify(double* ptr, DWORD64 columnsCount, DWORD64 rowsCount, BOOL transpose, double x0, double dx);
    void showGBPS(DWORD64 hzTSC, DWORD64 dTSC, double gigabytes);
    void showDump8x8(void* data);
    void showDump16x16(void* data);
    void showDump32x32(void* data);

private:
    static double* srcStore;
    static double* dstStore;
    static void threadFunction(void* ptr);
};

#endif  // TASK_09_H
