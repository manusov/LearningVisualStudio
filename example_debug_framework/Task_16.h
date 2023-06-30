/*
 
Learn some intrinsics.
VPTERNLOG in the new context (context by Task_15).

VPTERNLOG instruction:
multiplexor with 3 inverted address bits from 3 source operands,
8 data bits from immediate data.

*/

#pragma once
#ifndef TASK_16_H
#define TASK_16_H

#include "Task_15.h"

class Task_16 : public Task_15
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_15_H

