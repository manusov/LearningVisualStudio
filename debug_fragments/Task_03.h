/*
Class header for sine calculation speed test.
*/

#pragma once
#ifndef TASK_03_H
#define TASK_03_H

#include <windows.h>
#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_03 : public Task
{
public:
    int execute(int argc, char** argv);
private:
    int scenarioSingleThread(double& dummySum);
    int scenarioMultiThread(double& dummySum);
    double sumHelper(double* p, size_t count);
    BOOL detectAvx512();
    static void threadFunctionAsm(void* ptr);
    static void threadFunctionCpp(void* ptr);
    static void threadFunctionAvx512(void* ptr);
};

#endif  // TASK_03_H


