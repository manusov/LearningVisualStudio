/*
 
Raw experiments.
Similar (but not equal) functionality with Task_06,
but one of classes for support both x64 and ia32 builds.
Yet only quadrat matrices supported.
Note about size limits for ia32 builds.

AVX512 matrix transpose:
8x16 base block, re-create threads at measurement cycle,
8x16 base block, create threads one time, measurement cycle in the threads routines.

This class for interrogation of overheads by re-create threads in each measurement iteration.

*/

#pragma once
#ifndef TASK_10_H
#define TASK_10_H

#include "Task_09.h"

class Task_10 : public Task_09
{
public:
    int execute(int argc, char** argv);
protected:
    void showAndGetGBPS(DWORD64 hzTSC, DWORD64 dTSC, double gigabytes, double& gbps);

private:
    static double* srcStore;
    static double* dstStore;
    static void threadFunction1(void* ptr);
    static void threadFunction2(void* ptr);
};

#endif  // TASK_10_H

