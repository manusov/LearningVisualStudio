/*
Interrogate threads running timings by TSC.
*/

#pragma once
#ifndef TASK_12_H
#define TASK_12_H

#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_12 : public Task
{
public:
    int execute(int argc, char** argv);
private:
    static void threadFunction(void* ptr);
    static unsigned int* indexPtrStore;
    static DWORD64* timePtrStore;
};

#endif  // TASK_12_H


