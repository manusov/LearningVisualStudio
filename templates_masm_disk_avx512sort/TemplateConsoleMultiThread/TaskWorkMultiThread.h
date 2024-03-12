/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class header.
Class used for threads management tests and benchmarking.
Child class, used for multi-thread scenario.

*/

#pragma once
#ifndef TASKWORKMULTITHREAD_H
#define TASKWORKMULTITHREAD_H

#include "Global.h"
#include "TaskWork.h"

class TaskWorkMultiThread : public TaskWork
{
public:
	static STATUS_CODES checkThreads(COMMAND_LINE_PARMS* pCommandLineParms);
private:
	static void threadFunction(void* ptr);
	static void threadFunctionWithAffinity(void* ptr);
	static void threadFunctionAVX512(void* ptr);
};

#endif  // TASKWORKMULTITHREAD_H

