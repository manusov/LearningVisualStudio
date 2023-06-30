/*
Learn some intrinsics,
use lambdas for create scenarios useable for many instructions.
Special thanks to:
https://ravesli.com/lyambda-vyrazheniya-anonimnye-funktsii-v-s/
https://ravesli.com/lyambda-zahvaty-v-s/
*/

#pragma once
#ifndef TASK_19_H
#define TASK_19_H

#include <functional>
#include "Task_15.h"

class Task_19 : public Task_15
{
public:
    int execute(int argc, char** argv);
protected:
    int context(const std::function<void(__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py)>& instruction);
};

#endif  // TASK_19_H

