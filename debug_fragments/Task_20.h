/*

Context,
data types, initialization, visualization
for vector data and intrinsics.

m128i, m256i, m512i.
Integer data, 8/16/32/64 bits.
Can be initialized as constant array of up to 64 BYTES.

m128, m256, m512.
Single-precision floating point data,
4 x 32-bit numbers per 128-bit vector, 8 x 32-bit numbers per 256-bit vector, 16 x 32-bit numbers per 512-bit vector.
Can be initialized as constant array of up to 4/8/16 FLOATS.

m128d, m256d, m512d.
Double-precision floating point data,
2 x 64-bit numbers per 128-bit vector, 4 x 64-bit numbers per 256-bit vector, 8 x 64-bit numbers per 512-bit vector.
Can be initialized as constant array of up to 2/4/8 DOUBLES.

*/

#pragma once
#ifndef TASK_20_H
#define TASK_20_H

#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"
#include "Task.h"

class Task_20 : public Task
{
public:
    int execute(int argc, char** argv);

protected:
    void hexPrintByte(const BYTE* data, WORD color = APPCONST::VALUE_COLOR);
    void hexPrintWord(const WORD* data, WORD color = APPCONST::VALUE_COLOR);
    void hexPrintDword(const DWORD* data, WORD color = APPCONST::VALUE_COLOR);
    void hexPrintQword(const DWORD64* data, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrintByte(const BYTE* data, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrintWord(const WORD* data, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrintDword(const DWORD* data, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrintQword(const DWORD64* data, WORD color = APPCONST::VALUE_COLOR);
    void floatPrintDword(const float* data, WORD color = APPCONST::VALUE_COLOR);
    void doublePrintQword(const double* data, WORD color = APPCONST::VALUE_COLOR);

    void hexPrint64x8(const __m64* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint64x16(const __m64* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint64x32(const __m64* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint64x64(const __m64* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint64x8(const __m64* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint64x16(const __m64* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint64x32(const __m64* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint64x64(const __m64* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void floatPrint64x32(const __m64* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void doublePrint64x64(const __m64* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);

    void hexPrint128x8(const __m128i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint128x16(const __m128i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint128x32(const __m128i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint128x64(const __m128i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint128x8(const __m128i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint128x16(const __m128i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint128x32(const __m128i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint128x64(const __m128i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void floatPrint128x32(const __m128* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void doublePrint128x64(const __m128d* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);

    void hexPrint256x8(const __m256i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint256x16(const __m256i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint256x32(const __m256i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void hexPrint256x64(const __m256i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint256x8(const __m256i* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint256x16(const __m256i* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint256x32(const __m256i* data, int columns = 4, WORD color = APPCONST::VALUE_COLOR);
    void decimalPrint256x64(const __m256i* data, int columns = 2, WORD color = APPCONST::VALUE_COLOR);
    void floatPrint256x32(const __m256* data, int columns = 16, WORD color = APPCONST::VALUE_COLOR);
    void doublePrint256x64(const __m256d* data, int columns = 8, WORD color = APPCONST::VALUE_COLOR);

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

private:
    void helperHexBytes(const BYTE* p, unsigned int n, int columns, WORD color);
    void helperHexWords(const WORD* p, unsigned int n, int columns, WORD color);
    void helperHexDwords(const DWORD* p, unsigned int n, int columns, WORD color);
    void helperHexQwords(const DWORD64* p, unsigned int n, int columns, WORD color);
    void helperDecimalBytes(const BYTE* p, unsigned int n, int columns, WORD color);
    void helperDecimalWords(const WORD* p, unsigned int n, int columns, WORD color);
    void helperDecimalDwords(const DWORD* p, unsigned int n, int columns, WORD color);
    void helperDecimalQwords(const DWORD64* p, unsigned int n, int columns, WORD color);
    void helperFloat(const float* p, unsigned int n, int columns, WORD color);
    void helperDouble(const double* p, unsigned int n, int columns, WORD color);
};

#endif  // TASK_20_H

