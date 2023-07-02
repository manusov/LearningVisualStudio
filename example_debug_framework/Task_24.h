/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=5121,5122,5103&avx512techs=AVX512_VBMI
*/

#pragma once
#ifndef TASK_24_H
#define TASK_24_H

#include "Task_20.h"

class Task_24 : public Task_20
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_24_H

