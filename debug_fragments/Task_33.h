/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#othertechs=GFNI

Arbitrtry bit permutter example.
https://gist.github.com/IJzerbaard/c5c7bcfae2910e714605ba7bf2ca9b42
See document:
TECHNOLOGY GUIDE. Intel Corporation. Galois Field New Instructions (GFNI).
644497-002US.
galois-field-new-instructions-gfni-technology-guide-1-1639042826.PDF
*/

#pragma once
#ifndef TASK_33_H
#define TASK_33_H

#include "Task_20.h"

class Task_33 : public Task_20
{
public:
    int execute(int argc, char** argv);
};

#endif  // TASK_33_H

