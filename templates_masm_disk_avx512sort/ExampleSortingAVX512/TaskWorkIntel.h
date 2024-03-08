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

Task works class header.
Class used for array sorting benchmarking scenarios.
Compare std::sort and INTEL example.

*/

#pragma once
#ifndef TASKWORKINTEL_H
#define TASKWORKINTEL_H

#include "TaskWork.h"

class TaskWorkIntel : public TaskWork
{
public:
	static STATUS_CODES runTask(COMMAND_LINE_PARMS* p);         // Target operation.
};

#endif  // TASKWORKINTEL_H
