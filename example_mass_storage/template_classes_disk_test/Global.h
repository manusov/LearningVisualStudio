/*

Mass storage performance test.
Classes source file edition.
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
    MBPS,
    MBPS_ONE_FILE,
    MBPS_MAP,
    IOPS,
    IOPS_QUEUED,
    IOPS_MAP
}  A_TYPE;
typedef enum
{
    ALL_ZEROES,
    ALL_ONES,
    INCREMENTAL,
    SOFT_RANDOM,
    HARD_RANDOM
}  D_TYPE;
typedef enum
{
    STANDARD_PAGES,
    LARGE_PAGES
} PAGING_TYPE;
// Parameters Block for user input.
// This data = f ( user settings at command line or defaults ).
typedef struct {
    char optionSrcPath[MAX_PATH];
    char optionDstPath[MAX_PATH];
    DWORD64 optionFileSize;
    DWORD64 optionBlockSize;
    DWORD optionFileCount;
    DWORD optionRepeats;
    DWORD optionAddress;
    DWORD optionData;
    DWORD optionNoBuf;
    DWORD optionWriteThr;
    DWORD optionSequental;
    DWORD optionWaitRead;
    DWORD optionWaitWrite;
    DWORD optionWaitCopy;
    DWORD optionQueue;
    DWORD optionThreads;
    DWORD optionHt;
    DWORD optionLargePages;
    DWORD optionNuma;
    DWORD optionPgroups;
    DWORD optionMinCpu;
    DWORD optionMaxCpu;
    DWORD optionMinDomain;
    DWORD optionMaxDomain;
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
    INTERNAL_ERROR,
    FUNCTION_UNDER_CONSTRUCTION,
    STATUS_MAX
}  STATUS_CODES;

namespace APPCONST
{
    // Application strings and report file name.
    const char* const MSG_STARTING = "Starting...";
    const char* const ANY_KEY_STRING = "Press any key...";
    const char* const MSG_APPLICATION = "Mass storage performance test v0.02.00 (classes source file edition)";
    const char* const DEFAULT_IN_NAME = "input.txt";
    const char* const DEFAULT_OUT_NAME = "output.txt";
    constexpr int MAX_TEXT_STRING = MAX_PATH;
#if _WIN64
    const char* const MSG_BUILD = "x64";
#elif _WIN32
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
        "INTERNAL ERROR",
        "FUNCTION IS UNDER CONSTRUCTION",
    };
    const char* const DATA_NAMES[]
    {
        "ALL_ZEROES",
        "ALL_ONES",
        "INCREMENTAL",
        "SOFT_RANDOM",
        "HARD_RANDOM"
    };
    // Constants used for mass storage benchmark.
    const char* const DEFAULT_PATH_SRC = "";
    const char* const DEFAULT_PATH_DST = "";
    const char* const DEFAULT_NAME_SRC = "src_";
    const char* const DEFAULT_NAME_DST = "dst_";
    const char* const DEFAULT_EXTENSION = ".bin";
    constexpr DWORD32 DEFAULT_FILE_COUNT = 20;
    constexpr DWORD32 DEFAULT_FILE_SIZE = 64 * 1024 * 1024;      // 64 * 1024 * 1024;  // 1024 * 1024 * 1024;
    constexpr DWORD32 DEFAULT_BLOCK_SIZE = 16 * 1024 * 1024;
    constexpr DWORD32 DEFAULT_REPEATS = 1;
    constexpr D_TYPE DEFAULT_DATA_TYPE = ALL_ZEROES;
    constexpr A_TYPE DEFAULT_ADDRESS_TYPE = MBPS;
    constexpr BOOL DEFAULT_FLAG_NO_BUFFERING = TRUE;
    constexpr BOOL DEFAULT_FLAG_WRITE_THROUGH = TRUE;
    constexpr BOOL DEFAULT_FLAG_SEQUENTIAL_SCAN = FALSE;
    constexpr DWORD32 DEFAULT_WAIT_READ = 0;
    constexpr DWORD32 DEFAULT_WAIT_WRITE = 0;
    constexpr DWORD32 DEFAULT_WAIT_COPY = 0;
    constexpr DWORD32 DEFAULT_QUEUE_DEPTH = 1;
    constexpr DWORD32 DEFAULT_THREAD_COUNT = 1;
    constexpr PAGING_TYPE DEFAULT_PAGING_TYPE = STANDARD_PAGES;
    constexpr DWORD DEFAULT_CPU_SELECT = -1;
    constexpr DWORD DEFAULT_DOMAIN_SELECT = -1;
    // System parameters.
    const DWORD WAIT_LIMIT = 600000;     // 10 minutes maximum timeout.
    const DWORD PAGE_WALK_STEP = 4096;
    const DWORD64 RANDOM_SEED = 3;
    const DWORD64 RANDOM_MULTIPLIER = 0x00DEECE66D;
    const DWORD64 RANDOM_ADDEND = 0x0B;
}

#endif  // GLOBAL_H


