/*

Multi-thread console application template. Threads management tests and benchmarking.

This template is PART, extracted from:
Mass storage performance test. Classes source file edition.
https://github.com/manusov/LearningVisualStudio/tree/master/framework_disk_test_classes
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

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
typedef enum
{
    SINGLE_THREAD,
    MULTI_THREAD
}  MODEL_TYPE;
// Performance pattern.
typedef enum
{
    CHECK_NO_AFFINITY,
    CHECK_AFFINITY,
    READ_AVX512
}  PATTERN_TYPE;
// Parameters Block for user input.
// This data = f ( user settings at command line or defaults ).
typedef struct {
    DWORD optionModel;
    DWORD optionPattern;
//  DWORD optionThreadCount;
//  DWORD64 optionStartBlock;
//  DWORD64 optionStopBlock;
//  DWORD64 optionStep;
//  DWORD optionRepeats;
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
typedef enum
{
    NO_ERRORS,
    TIMER_FAILED,
    MEMORY_ALLOCATION_FAILED,
    MEMORY_NUMA_ALLOCATION_FAILED,
    MEMORY_RELEASE_FAILED,
    SET_CPU_AFFINITY_FAILED,
    RESTORE_CPU_AFFINITY_FAILED,
    FILE_CREATE_FAILED,
    FILE_OPEN_FAILED,
    FILE_WRITE_FAILED,
    FILE_COPY_FAILED,
    FILE_READ_FAILED,
    FILE_DELETE_FAILED,
    FILE_SIZE_MISMATCH,
    EVENT_CREATE_FAILED,
    EVENT_CLOSE_FAILED,
    FILE_MAPPING_CREATE_FAILED,
    MAP_VIEW_OF_FILE_FAILED,
    FLUSH_VIEW_OF_FILE_FAILED,
    UNMAP_VIEW_OF_FILE_FAILED,
    HANDLE_CLOSE_FAILED,
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
    const char* const MSG_APPLICATION = "Threads management template v0.00.03 (classes source file edition)";
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
        "FILE CREATE FAILED",
        "FILE_OPEN_FAILED",
        "FILE WRITE FAILED",
        "FILE COPY FAILED",
        "FILE READ FAILED",
        "FILE DELETE FAILED",
        "FILE SIZE MISMATCH",
        "EVENT CREATE FAILED",
        "EVENT CLOSE FAILED",
        "FILE MAPPING CREATE FAILED",
        "MAP VIEW OF FILE FAILED",
        "FLUSH VIEW OF FILE FAILED",
        "UNMAP VIEW OF FILE FAILED",
        "HANDLE CLOSE FAILED",
        "FEATURE NOT SUPPORTED",
        "INTERNAL ERROR",
        "FUNCTION IS UNDER CONSTRUCTION",
    };
    // Constants used for mass storage benchmark.
    constexpr MODEL_TYPE DEFAULT_MODEL = SINGLE_THREAD;
    constexpr PATTERN_TYPE DEFAULT_PATTERN = CHECK_NO_AFFINITY;
//  constexpr DWORD DEFAULT_THREAD_COUNT = 0;                      // 0 means auto-detect.
//  constexpr DWORD DEFAULT_START_BLOCK = 2048;
//  constexpr DWORD DEFAULT_STOP_BLOCK = 8192;
//  constexpr DWORD DEFAULT_STEP = 512;
//  constexpr DWORD64 DEFAULT_REPEATS = 10000000;
}

#endif  // GLOBAL_H


