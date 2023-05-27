/*
Header of processor performance functions class.
Benchmarking code with intrinsics for low-level coding.
*/

#pragma once
#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <windows.h>
#include "AppLib.h"
#include <intrin.h>

class Performance
{
public:
	Performance();
	void addAvx512(void* ptr, size_t length, DWORD64& dTSC, double& dummyData);
};

#endif  // PERFORMANCE_H


