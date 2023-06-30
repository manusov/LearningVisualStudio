/*
AVX512 matrix transpose, 8x16 base block, 
measurement scenario for small (temporal) and big (nontemporal) blocks.
Yet for quadrat matrices only.
TODO. Compare results Task_11.cpp and Task_10.cpp.
*/

#pragma once
#ifndef TASK_11_H
#define TASK_11_H

#include "Task_09.h"

class Task_11 : public Task_09
{
public:
    int execute(int argc, char** argv);

private:
    static void threadFunctionSimple(void* ptr);
    static void threadFunctionRepeats(void* ptr);
    static void threadFunctionNonTemporal(void* ptr);
    static void threadFunctionNonTemporalRepeats(void* ptr);

    static double* srcStore;
    static double* dstStore;
    static DWORD64 sizeStore;
    static DWORD64 srcAddStore;
    static DWORD64 dstAddStore;
    static DWORD64 repeatsStore;
};

#endif  // TASK_11_H

