/*
Class header for AVX512 matrix transpose.
Step 1 = debug for one cell 8x8 and 16x16, for x64 builds only.
*/

#pragma once
#ifndef TASK_05_H
#define TASK_05_H

#include <intrin.h>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_05 : public Task
{
public:
    int execute(int argc, char** argv);
private:
    void transposeAvx512(__m512d* src, __m512d* dst, DWORD32 cellsCount, size_t srcAdd, size_t dstAdd);
    BOOL detectAvx512();
    void dump8x8(void* data);
    void dump16x16(void* data);
};

#endif  // TASK_05_H

