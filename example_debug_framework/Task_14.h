/*
Learn some intrinsics. VPTERNLOG.
*/

#pragma once
#ifndef TASK_14_H
#define TASK_14_H

#include <intrin.h>
#include <immintrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_14 : public Task
{
public:
    int execute(int argc, char** argv);
protected:
    void hexPrint32(DWORD32* data, WORD color);
    void hexPrint64(DWORD64* data, WORD color);
};

#endif  // TASK_14_H

