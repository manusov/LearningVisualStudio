/*
Class header for timers precision test.
Unit test for Timer class.
Measure frequencies for:
OS Performance Counter, OS File Time counter, TSC.
*/

#pragma once
#ifndef TASK_01_H
#define TASK_01_H

#include "Task.h"
#include "Timer.h"

class Task_01 : public Task
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_01_H
