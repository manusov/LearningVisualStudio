/*
Class header for intrinsics speed test.
Benchmarking code with intrinsics for low-level coding.
*/

#pragma once
#ifndef TASK_04_H
#define TASK_04_H

#include <windows.h>
#include <intrin.h>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_04 : public Task
{
public:
    int execute(int argc, char** argv);
private:
    void addAvx512(void* ptr, size_t length, DWORD64& dTSC, double& dummyData);
    BOOL detectAvx512();
};

#endif  // TASK_04_H

