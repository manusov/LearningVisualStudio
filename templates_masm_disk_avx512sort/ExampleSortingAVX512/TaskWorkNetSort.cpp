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

Task works class implementation.
Class used for sorting networks debug.

*/

#include "TaskWorkNetSort.h"

STATUS_CODES TaskWorkNetSort::runTask(COMMAND_LINE_PARMS* p)
{
/*
	// double testArray[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
       double testArray[] = { 17, 1, 16, 2, 10, 3, 5, 7, 9, 18, 31, 44, 1, 4, 8, 2 };
	// double testArray[] = { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	// double testArray[] = { 1, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2 };
	// double testArray[] = { 2,  16,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1 };
	// double testArray[] = { 1, 1, 1, 1, 1, 1, 1, 1, 7, 8, 9, 6, 5, 4, 3, 2 };
	// double testArray[] = { 1, 16, 1, 1, 1, 1, 1, 1, 9, 8, 7, 6, 5, 4, 3, 2 };
	//
	int M = 0;
	int N = sizeof(testArray) / sizeof(double) - M;
	lineDump(testArray, N);  // N must be 9-16.
	asmSortingNetworkX16(reinterpret_cast<__m512d*>(testArray), N);
	lineDump(testArray, N);
	return FUNCTION_UNDER_CONSTRUCTION;
*/
//
	// double testArray[] = { 10, 20, 30, 40, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
	   double testArray[] = { 40, 30, 20, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };	
	// double testArray[] = { 10, 40, 30, 20, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
	// double testArray[] = { 10, 30, 40, 20, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
	//
	int M = 13;
	int N = sizeof(testArray) / sizeof(double) - M;
	lineDump(testArray, N);  // N must be 0-4.
	asmSortingNetworkX4(reinterpret_cast<__m512d*>(testArray), N);
	lineDump(testArray, N);
	return FUNCTION_UNDER_CONSTRUCTION;
	//
}
void TaskWorkNetSort::lineDump(const double* p, const int n)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("{ ", APPCONST::VALUE_COLOR);
	for (int i = 0; i < 16; i++)
	{
		int a = static_cast<int>(p[i]);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%4d", a);
		AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
	}
	AppLib::writeColor("  }\r\n", APPCONST::VALUE_COLOR);
}