/*

Mass storage performance test.
Classes source file edition.
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class header.
Class used for mass storage read/write/copy
benchmarking scenarios.

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
    static void writeStatistics(char* msg, const char* statisticsName, std::vector<double> speeds, bool tableMode);
    static void buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize);
    static void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR>& list, unsigned int blocksPerFile, unsigned int fileCount);
    static bool ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2);
    static BOOL createEvents(char* msg, std::vector<HANDLE>& events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count);
    static BOOL closeEvents(std::vector<HANDLE>& events);
    static STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity);
    static STATUS_CODES closeContext(STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity);
};

#endif  // TASKWORK_H


