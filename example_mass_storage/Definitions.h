/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Application definitions header file.

*/

#pragma once
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <windows.h>

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
// Enumerations for options settings
// Options for memory block size print as integer
typedef enum {
	PRINT_SIZE_AUTO,
	PRINT_SIZE_BYTES,
	PRINT_SIZE_KB,
	PRINT_SIZE_MB,
	PRINT_SIZE_GB
} PRINT_SIZE_MODES;

enum D_TYPE
{
	ALL_ZEROES,
	ALL_ONES,
	INCREMENTAL,
	SOFT_RANDOM,
	HARD_RANDOM
};

enum A_TYPE
{
	MBPS,
	ONE_FILE_MBPS,
	IOPS
};

enum PAGING_TYPE
{
	STANDARD_PAGES,
	LARGE_PAGES
};

enum STATUS_CODES
{
	NO_ERRORS,
	TIMER_FAILED,
	MEMORY_ALLOCATION_FAILED,
	MEMORY_RELEASE_FAILED,
	FILE_CREATE_FAILED,
	FILE_WRITE_FAILED,
	FILE_COPY_FAILED,
	FILE_READ_FAILED,
	FILE_DELETE_FAILED,
};

const char* const DEFAULT_PATH_SRC = "F:\\Exchange\\";      // "E:\\temp\\";  // "F:\\Exchange\\";
const char* const DEFAULT_PATH_DST = "F:\\Exchange\\";      // "E:\\temp\\";  // "F:\\Exchange\\";
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

constexpr DWORD32 DEFAULT_THREAD_COUNT = 1;
constexpr PAGING_TYPE DEFAULT_PAGING_TYPE = STANDARD_PAGES;

constexpr BOOL DEFAULT_NUMA_DOMAIN_SELECT = FALSE;
constexpr BOOL DEFAULT_PROCESSOR_SELECT = FALSE;

const DWORD64 RANDOM_SEED = 3;
const DWORD64 RANDOM_MULTIPLIER = 0x00DEECE66D;
const DWORD64 RANDOM_ADDEND = 0x0B;

#endif  //  DEFINITIONS_H
