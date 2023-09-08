/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Class header for mass storage performance task.

*/

#pragma once
#ifndef TASK_H
#define TASK_H

#include <windows.h>
#include <iostream>
#include "Constants.h"
#include "AppLib.h"

class Task
{
public:
	static int Run(COMMAND_LINE_PARMS* pCommandLineParms);
};

#endif  // TASK_H
