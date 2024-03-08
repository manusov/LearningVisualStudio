/*
Class header for GUI window test.
*/

#pragma once
#ifndef TASK_02_H
#define TASK_02_H

#include "Windows.h"
#include "Task.h"
#include "AppLib.h"
#include "Window.h"

class Task_02 : public Task
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_02_H

