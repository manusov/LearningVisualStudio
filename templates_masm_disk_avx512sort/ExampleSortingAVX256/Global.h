/*

Sorting algorithm debug.
Sorting networks with AVX2 instructions.
Variant for integers.

This project based on sources:
https://github.com/simd-sorting/fast-and-robust

See also Intel sources:
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

Global constants definitions.

*/

#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <windows.h>

#if _WIN64
#define NATIVE_WIDTH_64
#elif _WIN32
#define NATIVE_WIDTH_32
#endif

// Enumerations for options settings.
typedef enum {
    TEST_1,
    TEST_2,
} SCENARIO_TYPE;
typedef enum {
    DATA_ZERO,
    DATA_ONES,
    DATA_INC,
    DATA_DEC,
    DATA_SRND,
    DATA_HRND
} DATA_TYPE;
// Parameters Block for user input.
// This data = f ( user settings at command line or defaults ).
typedef struct {
    DWORD optionScenario;
    DWORD optionData;
    DWORD optionStartCount;
    DWORD optionEndCount;
    DWORD optionDeltaCount;
} COMMAND_LINE_PARMS;
// Types of command line options.
typedef enum {
    NOPARM, INTPARM, MEMPARM, SELPARM, STRPARM
} OPTION_TYPES;
// Command line option description entry.
typedef struct {
    const char* name;             // Pointer to parm. name for recognition NAME=VALUE.
    const char** values;          // Pointer to array of strings pointers, text opt.
    const void* data;             // Pointer to updated option variable.
    const OPTION_TYPES routine;   // Select handling method for this entry.
} OPTION_ENTRY;
// Status codes.
typedef enum {
    NO_ERRORS,
    TIMER_FAILED,
    MEMORY_ALLOCATION_FAILED,
    MEMORY_NUMA_ALLOCATION_FAILED,
    MEMORY_RELEASE_FAILED,
    SET_CPU_AFFINITY_FAILED,
    RESTORE_CPU_AFFINITY_FAILED,
    SORTING_DATA_MISMATCH,
    FEATURE_NOT_SUPPORTED,
    INTERNAL_ERROR,
    FUNCTION_UNDER_CONSTRUCTION,
    STATUS_MAX
}  STATUS_CODES;
namespace APPCONST
{
    // Application strings and report file name.
    const char* const MSG_STARTING = "Starting...";
    const char* const ANY_KEY_STRING = "Press any key...";
    const char* const MSG_APPLICATION = "AVX2 sorting test v0.00.00.";
    constexpr int MAX_TEXT_STRING = MAX_PATH;
#if _WIN64
    const char* const MSG_BUILD = "x64";
    const char* const DEFAULT_IN_NAME = "input64.txt";
    const char* const DEFAULT_OUT_NAME = "output64.txt";
#elif _WIN32
    const char* const MSG_BUILD = "ia32";
    const char* const DEFAULT_IN_NAME = "input32.txt";
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
    constexpr int DUMP_BYTES_PER_LINE = 16;
    constexpr int DUMP_WIDTH = 80;
    constexpr int TABLE_WIDTH = 68;
    constexpr int SMALL_TABLE_WIDTH = 31;
    constexpr size_t MAX_IN_FILE_SIZE = 4 * 1024 * 1024;
    constexpr size_t MAX_OUT_FILE_SIZE = 4 * 1024 * 1024;
    constexpr size_t PATH_SIZE = 1024;
    // Constants used for print memory block size.
    constexpr int KILO = 1024;
    constexpr int MEGA = 1024 * 1024;
    constexpr int GIGA = 1024 * 1024 * 1024;
    // Status codes.
    const char* const RETURN_NAMES[]
    {
        "NO ERRORS",
        "TIMER FAILED",
        "MEMORY ALLOCATION FAILED",
        "MEMORY NUMA ALLOCATION FAILED",
        "MEMORY RELEASE FAILED",
        "SET CPU AFFINITY FAILED",
        "RESTORE CPU AFFINITY FAILED",
        "SORTING DATA MISMATCH",
        "FEATURE NOT SUPPORTED",
        "INTERNAL ERROR",
        "FUNCTION IS UNDER CONSTRUCTION",
    };
    // Constants used for mass storage benchmark.
    constexpr SCENARIO_TYPE DEFAULT_ALGORITHM = TEST_1;
    constexpr DATA_TYPE DEFAULT_DATA = DATA_HRND;
    constexpr DWORD DEFAULT_START_COUNT = 100000;
    constexpr DWORD DEFAULT_END_COUNT = 1500000;
    constexpr DWORD DEFAULT_DELTA_COUNT = 100000;
    // System parameters.
    constexpr DWORD WAIT_LIMIT = 600000;     // 10 minutes maximum timeout.
    constexpr DWORD PAGE_WALK_STEP = 4096;
    constexpr DWORD64 RANDOM_SEED = 3;
    constexpr DWORD64 RANDOM_MULTIPLIER = 0x00DEECE66D;
    constexpr DWORD64 RANDOM_ADDEND = 0x0B;
}

#endif  // GLOBAL_H


