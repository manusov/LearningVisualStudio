/*
 
Context, 
data types, initialization, visualization
for vector data and intrinsics.

m512i = Integer data, 8/16/32/64 bits.
        Can be initialized as constant array of up to 64 BYTES.

m512  = Single-precision floating point data, 16 x 32-bit numbers per 512-bit vector.
        Can be initialized as constant array of up to 16 FLOATS.

m512d = Double-precision floating point data, 8 x 64-bit numbers per 512-bit vector.
        Can be initialized as constant array of up to 8 DOUBLES.

*/

#pragma once
#ifndef TASK_15_H
#define TASK_15_H

#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_15 : public Task
{
public:
    int execute(int argc, char** argv);

protected:
    void hexPrint512x8(const __m512i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint512x16(const __m512i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint512x32(const __m512i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint512x64(const __m512i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);

    void decimalPrint512x8(const __m512i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint512x16(const __m512i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint512x32(const __m512i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint512x64(const __m512i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);

    void floatPrint512x32(const __m512* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void doublePrint512x64(const __m512d* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
};

#endif  // TASK_15_H
