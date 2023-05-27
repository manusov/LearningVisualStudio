/*
Header of application library helper class.
*/

#pragma once
#ifndef APPLIB_H
#define APPLIB_H

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

namespace APPCONST
{
	// Application strings and report file name.
	const char* const MSG_APPLICATION = "Debug template v0.00.04.";
	const char* const MSG_STARTING = "Starting...";
	// Parameters depend on x64 or ia32 build.
#if _WIN64
	#define NATIVE_WIDTH_64
	const char* const MSG_BUILD = "Windows x64";
	const char* const DEFAULT_OUT_NAME = "output64.txt";
#elif _WIN32
	#define NATIVE_WIDTH_32
	const char* const MSG_BUILD = "Windows ia32";
	const char* const DEFAULT_OUT_NAME = "output32.txt";
#endif
	// Console output parameters: colors.
	constexpr WORD PARM_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	constexpr WORD VALUE_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	constexpr WORD GROUP_COLOR = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	constexpr WORD NO_ERROR_COLOR = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	constexpr WORD ERROR_COLOR = FOREGROUND_RED | FOREGROUND_INTENSITY;
	constexpr WORD DUMP_ADDRESS_COLOR = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	constexpr WORD DUMP_DATA_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	constexpr WORD DUMP_TEXT_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	constexpr WORD TABLE_COLOR = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	// Console output parameters: sizes.
	constexpr int MAX_TEXT_STRING = 160;
	constexpr int DUMP_BYTES_PER_LINE = 16;
	constexpr int DUMP_WIDTH = 80;
	constexpr int TABLE_WIDTH = 80;
	constexpr size_t MAX_OUT_FILE_SIZE = 4 * 1024 * 1024;
	// Constants used for print memory block size.
	constexpr int KILO = 1024;
	constexpr int MEGA = 1024 * 1024;
	constexpr int GIGA = 1024 * 1024 * 1024;
}
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
	static BOOL initConsole(bool, bool, char*, size_t);
	static size_t getCurrentOutSize();
	// Console output and save report to file.
	static void write(const char* string);
	static void writeColor(const char* string, WORD color);
	static void writeParmGroup(const char* string);
	static void writeParmError(const char* string);
	static void writeParmAndValue(const char* parmName, const char* parmValue, int nameWidth);
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
	static void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);
private:
	static bool screenMode;
	static bool fileMode;
	static char* outPointer;
	static size_t maxOutSize;
	static size_t currentOutSize;
	static HANDLE hStdout;
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	static void colorHelper(WORD color);
	static void colorRestoreHelper();
	static void cellPrintHelper(char* buffer, size_t limit, size_t cell, size_t count);
};

#endif  // APPLIB_H

