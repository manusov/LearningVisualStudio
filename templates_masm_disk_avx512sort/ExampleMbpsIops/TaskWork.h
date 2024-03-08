/*

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
Updates required from later projects:
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_single_source
https://github.com/manusov/LearningVisualStudio/tree/master/simple_test_disk_iops
https://github.com/manusov/LearningVisualStudio/tree/master/simple_test_disk_mbps
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class header.
Class used for mass storage read/write/copy
benchmarking scenarios.

TODO.
1) + Redesign measurement and statistics tables(colors, sizes, layouts).
2) + Verify files size : compare calculated and sum of sizes returned by Read / Write WinAPI.
3) + Support measurement repeats.
4) + Default path must be empty.
5) + Check support RDRAND instruction by CPUID instruction.At procedure void correctAfterParse().
6) + Support wait times options before read, write and copy.
7) + Errors handling, visual OS error code by existing helper.Restore state when return.After return from RunTask().Use RETURN_NAMES[].
8)  Add support maxCpu, maxDomain and min - max ranges.Yet only minCpu, minDomain supported.
9)  Add check affinity mask validity.
10) Add support > 64 logical processors by Processor Groups.
11) Add scenarios with I / O queues, especially for IOPS, especially at multithread test.Enumerate as IOPS_QUEUED.
12) Change queue strategy : add next request after one request executed.Don't wait all requests done, this prevents parallel works.
13) Support procedures : void resetBeforeParse(), void correctAfterParse().Check all options.
14) Error message and exit if wrong options(instead default scenario).
15) Close file group than measure integral time(? )
16) Support files and blocks size > 2GB / 4GB, use 64 - bit variables(Microsoft DWORD64) for all offsets and sizes.
17) Optimize by remove duplicated fragments by functions(subroutines).
18) Extended verification by compare read data and write data.
19) Wait key option : wait after actions.
20) Report option : file and /or console.
21) Support all other options.
22) Re - verify all options settings support.
23) Compare with NIOBench for some types of disks.
24) If divergent source, use Main.cpp module, Main.h header(? ) and classes: TaskParms.cpp, TaskParms.h, TaskWork.cpp, TaskWork.h.
    Projects names : template_single_file, template_classes. This variant is single source file(Main.cpp).

*/

#pragma once
#ifndef TASKWORK_H
#define TASKWORK_H

#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <immintrin.h>
#include "Global.h"
#include "AppLib.h"

typedef struct {
    DWORD32 random;      // Random data for make random order after sorting.
    DWORD32 blockIndex;  // Index of block inside one file.
    DWORD32 fileIndex;   // Index of file.
    DWORD32 flags;       // D0 encode 0=src, 1=dst. Other bits reserved.
} IO_DESCRIPTOR;

class TaskWork
{
public:
    // Target operations.
    static int runTaskMBPS(COMMAND_LINE_PARMS* p);
    static int runTaskMBPSoneFile(COMMAND_LINE_PARMS* p);
    static int runTaskMBPSmap(COMMAND_LINE_PARMS* p);
    static int runTaskIOPS(COMMAND_LINE_PARMS* p);
    static int runTaskIOPSqueued(COMMAND_LINE_PARMS* p);
    static int runTaskIOPSmap(COMMAND_LINE_PARMS* p);
private:
    static DWORD64 getHandle64(HANDLE handle);
    static void waitTime(char* msg, DWORD milliseconds, const char* operationName);
    static void writeStatistics(char* msg, const char* statisticsName, std::vector<double> &speeds, bool tableMode);
    static void buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize);
    static void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR>& list, unsigned int blocksPerFile, unsigned int fileCount);
    static bool ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2);
    static BOOL createEvents(char* msg, std::vector<HANDLE>& events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count);
    static BOOL closeEvents(std::vector<HANDLE>& events);
    static STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity);
    static STATUS_CODES closeContext(STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity);
};

#endif  // TASKWORK_H


