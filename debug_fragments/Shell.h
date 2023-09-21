/*
Class header for task shell.
*/

#pragma once
#ifndef SHELL_H
#define SHELL_H

#include "Task.h"

class Shell
{
public:
    int execute(int argc, char** argv, Task& task);
};

#endif  // SHELL_H

