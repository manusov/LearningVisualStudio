/*
AVX256 matrix transpose, 4x8 base block,
measurement scenario for small (temporal) and big (nontemporal) blocks.
Yet for quadrat matrices only.
*/

#pragma once
#ifndef TASK_13_H
#define TASK_13_H

#include "Task_09.h"

class Task_13 : public Task_09
{
public:
    int execute(int argc, char** argv);

protected:
    BOOL detectAvx256();

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

#endif  // TASK_13_H


