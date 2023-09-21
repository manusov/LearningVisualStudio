/*

Class header for AVX512 read bandwidth test.
Measure speed for memory read by AVX512 instructions.

Multi-thread L1 cache read bandwidth benchmark, required AVX512 instructions.
Demo for use MASM 32/64 at Visual Studio 2022.
Special thanks (there all instructions for configure VS2022 IDE):
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio
See also:
https://learn.microsoft.com/ru-ru/cpp/assembler/masm/masm-for-x64-ml64-exe?view=msvc-170
https://habr.com/ru/articles/111275/
http://lallouslab.net/2016/01/11/introduction-to-writing-x64-assembly-in-visual-studio/
https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/

*/

#pragma once
#ifndef TASK_02_H
#define TASK_02_H

#include <windows.h>
#include <intrin.h>
#include <thread>
#include <vector>
#include <iostream>
#include "Task.h"

class Task_02 : public Task
{
public:
    int execute(int argc, char** argv);
private:
    int singleThreadScenario();
    int multiThreadScenario();
    BOOL detectAvx512();
    BOOL measureTscClock(LARGE_INTEGER& hzPc, LARGE_INTEGER& hzTsc);
    static void threadFunction(void* ptr);
};

#endif  // TASK_02_H

