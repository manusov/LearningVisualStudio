/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Application constants header file.

*/

#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>
#include "Definitions.h"

namespace APPCONST
{
	// Application strings and report file name.
	const char* const MSG_STARTING = "Starting...";
	const char* const ANY_KEY_STRING = "Press any key...";
	const char* const MSG_APPLICATION = "Mass storage performance sample v0.00.01";
	const char* const DEFAULT_IN_NAME = "input.txt";
	const char* const DEFAULT_OUT_NAME = "output.txt";

	// Parameters depend on x64 or ia32 build.
#if _WIN64
	#define NATIVE_WIDTH_64
	const char* const MSG_BUILD = "x64";
#elif _WIN32
	#define NATIVE_WIDTH_32
	const char* const MSG_BUILD = "ia32";
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
	constexpr int HALF_TABLE_WIDTH = 40;
	constexpr size_t MAX_IN_FILE_SIZE = 4 * 1024 * 1024;
	constexpr size_t MAX_OUT_FILE_SIZE = 4 * 1024 * 1024;
	constexpr size_t PATH_SIZE = 1024;
	// Constants used for print memory block size.
	constexpr int KILO = 1024;
	constexpr int MEGA = 1024 * 1024;
	constexpr int GIGA = 1024 * 1024 * 1024;
}

#endif  // CONSTANTS_H

