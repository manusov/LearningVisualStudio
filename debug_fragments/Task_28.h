/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VNNI&ig_expand=2702
*/

#pragma once
#ifndef TASK_28_H
#define TASK_28_H

#include "Task_20.h"

typedef struct
{
	unsigned long long (*handler)(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum);
	unsigned int unit;
	unsigned int unrollFactor;
	unsigned int repeats;
	BOOL vlRequired;
	const char* name;
} HANDLER_DESCRIPTOR;

class Task_28 : public Task_20
{
public:
    int execute(int argc, char** argv);
private:
	void runScenario(int index, char* msg, HANDLER_DESCRIPTOR hdesc[], LPVOID p1, LPVOID p2, DWORD64 blockSize, double secondsTsc);
    static unsigned long long vnniPattern512(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum);
	static unsigned long long vnniPattern256(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum);
	static unsigned long long vnniPattern128(LPVOID p1, LPVOID p2, unsigned int unrollCount, unsigned int repeats, __m512i& sum);
    static HANDLER_DESCRIPTOR hdesc[];
};

#endif  // TASK_28_H


