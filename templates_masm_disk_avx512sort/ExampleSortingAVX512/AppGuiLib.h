/*

Sorting algorithm debug.
Quick sort and sorting networks with AVX512 instructions.
Starts with variant for double precision numbers.

This project based on Intel sources:
https://github.com/intel/x86-simd-sort
https://github.com/berenger-eu/avx-512-sort/tree/master
Use example at document:
1704.08579.pdf. Appendix B, code 2.
See also:
LIPIcs.SEA.2021.3.pdf.

Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Same template used by: Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories

Application library class header.
Class used for GUI support helpers functions.
UNDER CONSTRUCTION.

*/

#pragma once
#ifndef APPGUILIB_H
#define APPGUILIB_H

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include "Global.h"
#include "DrawYX.h"

class AppGuiLib
{
public:
	static void windowFunction(size_t startCount, size_t end_count, size_t deltaCount, std::vector<double> vY1, std::vector<double> vY2);
};

#endif  // APPGUILIB_H