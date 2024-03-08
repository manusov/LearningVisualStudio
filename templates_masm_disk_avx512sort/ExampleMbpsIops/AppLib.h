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

Application library class header.
Class used for color console support helpers functions.

*/

#pragma once
#ifndef APPLIB_H
#define APPLIB_H

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include "Global.h"

// Options for memory block size print as integer
typedef enum {
	PRINT_SIZE_AUTO,
	PRINT_SIZE_BYTES,
	PRINT_SIZE_KB,
	PRINT_SIZE_MB,
	PRINT_SIZE_GB
} PRINT_SIZE_MODES;

class AppLib
{
public:
	// Support color console.
	static BOOL initConsole(bool screenMode, bool fileMode, char* outPointer, size_t maxOutSize);
	// Console output and save report to file.
	static void write(const char* string);
	static void writeColor(const char* string, WORD color);
	static void writeParmGroup(const char* string);
	static void writeParmError(const char* string);
	static void writeParmAndValue(const char* parmName, const char* parmValue, int nameWidth, WORD valueColor = APPCONST::VALUE_COLOR);
	static void writeHexDump(byte* ptr, int size);
	static void writeHexDumpUp();
	static void writeLine(int count);
	static void writeColorLine(int count, WORD color);
	// Write strings to buffer.
	static size_t storeHex8(char* buffer, size_t limit, DWORD32 value, bool h);
	static size_t storeHex16(char* buffer, size_t limit, DWORD32 value, bool h);
	static size_t storeHex32(char* buffer, size_t limit, DWORD32 value, bool h);
	static size_t storeHex64(char* buffer, size_t limit, DWORD64 value, bool h);
	static size_t storeHexPointer(char* buffer, size_t limit, LPVOID ptr, bool h);
	static void storeBitsList(char* buffer, size_t limit, DWORD64 x);
	static void storeCellMemorySize(char* buffer, size_t limit, DWORD64 x, size_t cell);
	static void storeCellMemorySizeInt(char* buffer, size_t limit, DWORD64 x, size_t cell, int mode);
	static void storeBaseAndSize(char* buffer, size_t limit, DWORD64 blockBase, DWORD64 blockSize);
	// Build execution path for save report to application directory (not current directory).
	static BOOL storeExecReportPath(char* buffer, size_t limit, const char* name);
	// Decode Windows error code to error description string.
	static void storeSystemErrorName(char* buffer, size_t limit, DWORD errorCode);
	// Calculate statistics for results vector: min, max, average, median.
	static void calculateStatistics(std::vector<double> &data, double& min, double& max, double& average, double& median);
	static size_t getCurrentOutSize();
private:
	static void colorHelper(WORD color);
	static void colorRestoreHelper();
	static void cellPrintHelper(char* buffer, size_t limit, size_t cell, size_t count);
	// Color console control variables.
	static bool gScreenMode;
	static bool gFileMode;
	static char* gOutPointer;
	static size_t gMaxOutSize;
	static size_t gCurrentOutSize;
	static HANDLE gHstdout;
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
};

#endif  // APPLIB_H
