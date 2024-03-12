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
Child class, used for single-thread scenario.

*/

#pragma once
#ifndef TASKWORKSINGLETHREAD_H
#define TASKWORKSINGLETHREAD_H

#include "Global.h"
#include "TaskWork.h"

class TaskWorkSingleThread : public TaskWork
{
public:
	static STATUS_CODES checkThreads(COMMAND_LINE_PARMS* pCommandLineParms);
};

#endif  // TASKWORKSINGLETHREAD_H

