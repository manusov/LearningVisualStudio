/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=7206,512,4940,5862,5876,5920,7018,7230,7199,937,5910&techs=AVX_512&cats=Set
Macro usage examples for set SSE/AVX/AVX512 variables and registers.
*/

#pragma once
#ifndef TASK_34_H
#define TASK_34_H

#include "Task_20.h"

class Task_34 : public Task_20
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_34_H

