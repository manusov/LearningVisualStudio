/*

Mass storage performance test.
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

TODO.
1)  + Redesign measurement and statistics tables (colors, sizes, layouts).
2)  + Verify files size: compare calculated and sum of sizes returned by Read/Write WinAPI.
3)  + Support measurement repeats.
4)  + Default path must be empty.
5)  + Check support RDRAND instruction by CPUID instruction. At procedure void correctAfterParse().
6)  + Support wait times options before read, write and copy.
7)  + Errors handling, visual OS error code by existing helper. Restore state when return. After return from RunTask(). Use RETURN_NAMES[].
8)  Add support maxCpu, maxDomain and min-max ranges. Yet only minCpu, minDomain supported.
9)  Add check affinity mask validity.
10) Add support >64 logical processors by Processor Groups.
11) Add scenarios with I/O queues, especially for IOPS, especially at multithread test. Enumerate as IOPS_QUEUED.
12) Support procedures: void resetBeforeParse(), void correctAfterParse(). Check all options.
13) Error message and exit if wrong options (instead default scenario).
14) Close file group than measure integral time (?)
15) Extended verification by compare read data and write data.
16) Wait key option: wait after actions.
17) Report option: file and/or console.
18) Support all other options.
19) Re-verify all options settings support.
20) Compare with NIOBench for some types of disks.

*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include <intrin.h>
#include <immintrin.h>

#if _WIN64
#define NATIVE_WIDTH_64
#elif _WIN32
#define NATIVE_WIDTH_32
#endif

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
typedef struct {
    DWORD32 random;      // Random data for make random order after sorting.
    DWORD32 blockIndex;  // Index of block inside one file.
    DWORD32 fileIndex;   // Index of file.
    DWORD32 flags;       // D0 encode 0=src, 1=dst. Other bits reserved.
} IO_DESCRIPTOR;

// Enumerations for options settings
// Options for memory block size print as integer
typedef enum {
    PRINT_SIZE_AUTO,
    PRINT_SIZE_BYTES,
    PRINT_SIZE_KB,
    PRINT_SIZE_MB,
    PRINT_SIZE_GB
} PRINT_SIZE_MODES;
typedef enum
{
    MBPS,
    MBPS_ONE_FILE,
    IOPS,
    IOPS_QUEUED
    
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
    INTERNAL_ERROR,
    STATUS_MAX
}  STATUS_CODES;

namespace APPCONST
{
	// Application strings and report file name.
	const char* const MSG_STARTING = "Starting...";
	const char* const ANY_KEY_STRING = "Press any key...";
	const char* const MSG_APPLICATION = "Mass storage performance test v0.01.10";
	const char* const DEFAULT_IN_NAME = "input.txt";
	const char* const DEFAULT_OUT_NAME = "output.txt";
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
	constexpr int MAX_TEXT_STRING = MAX_PATH;
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
    const DWORD64 RANDOM_SEED = 3;
    const DWORD64 RANDOM_MULTIPLIER = 0x00DEECE66D;
    const DWORD64 RANDOM_ADDEND = 0x0B;
    const DWORD WAIT_LIMIT = 600000;     // 10 minutes maximum timeout.
}

// Command line parameters parse control.
// Strings for command line options detect.
const char* keysAddress[] = { "mbps", "mbpsone", "iops", "iopsqueue", nullptr};
const char* keysData[] = { "zeroes", "ones", "inc", "softrnd", "hardrnd", nullptr };
const char* keysHt[] = { "default", "off", "on", nullptr };
const char* keysNuma[] = { "unaware", "single", "optimal", "nonoptimal", nullptr };
const char* keysPgroups[] = { "unaware", "single", "all", nullptr };
const char* DATA_NAMES[]
{ 
    "ALL_ZEROES",
    "ALL_ONES",
    "INCREMENTAL",
    "SOFT_RANDOM",
    "HARD_RANDOM" 
};
const char* RETURN_NAMES[]
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
    "INTERNAL ERROR"
};
// Pointer command line parameters structure.
COMMAND_LINE_PARMS parms;
// Option control list, used for command line parameters parsing (regular input).
const OPTION_ENTRY options[] = {
    { "srcpath"         , nullptr         , &parms.optionSrcPath     , STRPARM } ,
    { "dstpath"         , nullptr         , &parms.optionDstPath     , STRPARM } ,
    { "filesize"        , nullptr         , &parms.optionFileSize    , MEMPARM } ,
    { "blocksize"       , nullptr         , &parms.optionBlockSize   , MEMPARM } ,
    { "filecount"       , nullptr         , &parms.optionFileCount   , INTPARM } ,
    { "repeats"         , nullptr         , &parms.optionRepeats     , INTPARM } ,
    { "address"         , keysAddress     , &parms.optionAddress     , SELPARM } ,
    { "data"            , keysData        , &parms.optionData        , SELPARM } ,
    { "nobuf"           , nullptr         , &parms.optionNoBuf       , INTPARM } ,
    { "writethr"        , nullptr         , &parms.optionWriteThr    , INTPARM } ,
    { "sequental"       , nullptr         , &parms.optionSequental   , INTPARM } ,
    { "waitread"        , nullptr         , &parms.optionWaitRead    , INTPARM } ,
    { "waitwrite"       , nullptr         , &parms.optionWaitWrite   , INTPARM } ,
    { "waitcopy"        , nullptr         , &parms.optionWaitCopy    , INTPARM } ,
    { "queue"           , nullptr         , &parms.optionQueue       , INTPARM } ,
//  { "threads"         , nullptr         , &parms.optionThreads     , INTPARM } ,
//  { "ht"              , keysHt          , &parms.optionHt          , SELPARM } ,
//  { "largepages"      , nullptr         , &parms.optionLargePages  , INTPARM } ,
//  { "numa"            , keysNuma        , &parms.optionNuma        , SELPARM } ,
//  { "pgroups"         , keysPgroups     , &parms.optionPgroups     , SELPARM } ,
    { "mincpu"          , nullptr         , &parms.optionMinCpu      , INTPARM } ,
    { "maxcpu"          , nullptr         , &parms.optionMaxCpu      , INTPARM } ,
    { "mindomain"       , nullptr         , &parms.optionMinDomain   , INTPARM } ,
    { "maxdomain"       , nullptr         , &parms.optionMaxDomain   , INTPARM } ,
    { nullptr           , nullptr         , nullptr                  , NOPARM  }
};
// Color console control.
bool gScreenMode = false;
bool gFileMode = false;
char* gOutPointer = nullptr;
size_t gMaxOutSize = 0;
size_t gCurrentOutSize = 0;
HANDLE gHstdout = nullptr;
CONSOLE_SCREEN_BUFFER_INFO csbi{ 0 };
// Support command line options, input configuration and output report files.
char optionStatusString[APPCONST::MAX_TEXT_STRING];
void resetBeforeParse();
void correctAfterParse();
BOOL parseOptions(char* p, size_t n);
DWORD parseCommandLine(int argc, char** argv);
void writeOptionReport();
// Support color console.
BOOL initConsole(bool screenMode, bool fileMode, char* outPointer, size_t maxOutSize);
// Console output and save report to file.
void write(const char* string);
void writeColor(const char* string, WORD color);
void writeParmGroup(const char* string);
void writeParmError(const char* string);
void writeParmAndValue(const char* parmName, const char* parmValue, int nameWidth, WORD valueColor = APPCONST::VALUE_COLOR);
void writeHexDump(byte* ptr, int size);
void writeHexDumpUp();
void writeLine(int count);
void writeColorLine(int count, WORD color);
// Write strings to buffer.
size_t storeHex8(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex16(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex32(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex64(char* buffer, size_t limit, DWORD64 value, bool h);
size_t storeHexPointer(char* buffer, size_t limit, LPVOID ptr, bool h);
void storeBitsList(char* buffer, size_t limit, DWORD64 x);
void storeCellMemorySize(char* buffer, size_t limit, DWORD64 x, size_t cell);
void storeCellMemorySizeInt(char* buffer, size_t limit, DWORD64 x, size_t cell, int mode);
void storeBaseAndSize(char* buffer, size_t limit, DWORD64 blockBase, DWORD64 blockSize);
// Build execution path for save report to application directory (not current directory).
BOOL storeExecReportPath(char* buffer, size_t limit, const char* name);
// Decode Windows error code to error description string.
void storeSystemErrorName(char* buffer, size_t limit, DWORD errorCode);
// Calculate statistics for results vector: min, max, average, median.
void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median);
void colorHelper(WORD color);
void colorRestoreHelper();
void cellPrintHelper(char* buffer, size_t limit, size_t cell, size_t count);
// Target operation-specific tasks.
void waitTime(char* msg, DWORD milliseconds, const char* operationName);
void writeStatistics(char* msg, const char* statisticsName, std::vector<double> speeds, bool tableMode);
void buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize);
void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR> &list, unsigned int blocksPerFile, unsigned int fileCount);
bool ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2);
BOOL createEvents(char* msg, std::vector<HANDLE> &events, std::vector<OVERLAPPED> &overlaps, std::vector<DWORD> &returns, unsigned int count);
BOOL closeEvents(std::vector<HANDLE> &events);
STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity);
STATUS_CODES closeContext(STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity);
// Target operations.
int runTaskMBPS(COMMAND_LINE_PARMS* p);
int runTaskMBPSoneFile(COMMAND_LINE_PARMS* p);
int runTaskIOPS(COMMAND_LINE_PARMS* p);
int runTaskIOPSqueued(COMMAND_LINE_PARMS* p);

// Application entry point.
int main(int argc, char** argv)
{
	using namespace std;
	int status = 0;
    bool screenMode = true;
    bool fileMode = true;
	char msg[APPCONST::MAX_TEXT_STRING];
	cout << APPCONST::MSG_STARTING << endl;
	// Assign input and output files names, can override from command line.
	const char* inName = APPCONST::DEFAULT_IN_NAME;
	const char* outName = APPCONST::DEFAULT_OUT_NAME;
	if (argc > 1)
	{
		inName = argv[1];
	}
	if (argc > 2)
	{
		outName = argv[2];
	}
	// Load input configuration file, override scenario options by loaded configuration.
	resetBeforeParse();                                  // Set default options values.
	BOOL loadStatus = FALSE;
	const char* loadString1 = nullptr;
	const char* loadString2 = nullptr;
	ifstream in;
	in.open(inName, ios::binary | ios::in);
	if (in.is_open())
	{
		streampos inSize = in.tellg();
		in.seekg(0, ios::end);
		inSize = in.tellg() - inSize;
		if ((inSize > 0) && ((unsigned long long)inSize <= APPCONST::MAX_IN_FILE_SIZE))
		{
			char* inPtr = (char*)malloc(((size_t)(inSize)+1));  // Reserve 1 byte for last 0.
			if (inPtr)
			{
				in.seekg(0, ios::beg);
				in.read(inPtr, inSize);
				if (in.fail())
				{
					loadString1 = "Error reading input file, default scenario used.";
				}
				else
				{
					snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d bytes loaded from input configuration file.", (int)inSize);
					cout << msg << endl;
					if (parseOptions(inPtr, (size_t)inSize))          // Parse input configuration file, set options values.
					{
						loadStatus = TRUE;
						loadString1 = "Input configuration file parsed OK.";
					}
					else
					{
						loadString1 = "Error parsing input file, default settings can be used for some options.";
						loadString2 = optionStatusString;
					}
				}
				in.close();
				free(inPtr);
			}
		}
		else
		{
			loadString1 = "Input file is invalid, default scenario used.";
		}
	}
	else
	{
		loadString1 = "Input file not found, default scenario used.";
	}
	// Create output report buffer, initializing color console, start performance scenario.
	char* outPtr = (char*)malloc(APPCONST::MAX_OUT_FILE_SIZE);
	char* const backOutPtr = outPtr;
	if (backOutPtr)
	{
		if (initConsole(screenMode, fileMode, backOutPtr, APPCONST::MAX_OUT_FILE_SIZE))
		{
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s (%s).\r\n", APPCONST::MSG_APPLICATION, APPCONST::MSG_BUILD);
			writeColor(msg, APPCONST::TABLE_COLOR);
			WORD color = APPCONST::NO_ERROR_COLOR;
			if (!loadStatus)
			{
				color = APPCONST::ERROR_COLOR;
			}
			if (loadString1)
			{
				writeColor(loadString1, color);
				write("\r\n");
			}
			if (loadString2)
			{
				writeColor(loadString2, color);
				write("\r\n");
			}
            write("\r\n");
            writeOptionReport();    // Show options values.
            write("\r\n");
            correctAfterParse();    // Verify and correct (if required) options values.
            // Get scenario options for mass storage performance test.
			COMMAND_LINE_PARMS* pCommandLineParms = &parms;
            // Target operation: execute mass storage performance test.
            int opStatus;
            switch (pCommandLineParms->optionAddress)
            {
            case MBPS:
                opStatus = runTaskMBPS(pCommandLineParms);
                break;
            case MBPS_ONE_FILE:
                opStatus = runTaskMBPSoneFile(pCommandLineParms);
                break;
            case IOPS:
                opStatus = runTaskIOPS(pCommandLineParms);
                break;
            case IOPS_QUEUED:
                opStatus = runTaskIOPSqueued(pCommandLineParms);
                break;
            default:
                opStatus = INTERNAL_ERROR;
                break;
            }
            // Interpreting status.
			if (opStatus == NO_ERRORS)
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario OK.\r\n");
				writeColor(msg, APPCONST::NO_ERROR_COLOR);
			}
			else
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario error %d", opStatus);
				writeColor(msg, APPCONST::ERROR_COLOR);
                if (opStatus < STATUS_MAX)
                {
                    const char* returnName = RETURN_NAMES[opStatus];
                    snprintf(msg, APPCONST::MAX_TEXT_STRING, " = %s.\r\n", returnName);
                    writeColor(msg, APPCONST::ERROR_COLOR);
                    DWORD osError = GetLastError();
                    if (osError)
                    {
                        storeSystemErrorName(msg, APPCONST::MAX_TEXT_STRING, osError);
                        writeColor(msg, APPCONST::ERROR_COLOR);
                    }
                }
                else
                {
                    writeColor(" = Unknown error.\r\n", APPCONST::ERROR_COLOR);
                }
                write("\r\n");
			}
			// Save report buffer to output file, if file mode enabled.
			if (fileMode)
			{
				ofstream out;
				out.open(outName, ios::binary | ios::out);
				if (out.is_open())
				{
					out.write(backOutPtr, gCurrentOutSize);
					if (out.fail())
					{
						cout << "Error writing output file." << endl;
						status = 4;
					}
					else
					{
						int n = (int)out.tellp();
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d bytes saved to output report file.", n);
						cout << msg << endl;
					}
					out.close();
				}
				else
				{
					cout << "Output file create error." << endl;
					status = 3;
				}
			}
			else
			{
				cout << "Report file disabled by option." << endl;
			}
		}
		else
		{
			cout << "Console initialization error." << endl;
			status = 2;
		}
		free(backOutPtr);
	}
	else
	{
		cout << "Memory allocation error." << endl;
		status = 1;
	}
	return status;
}

// Universal library of helpers functions.
void resetBeforeParse()                     // Reset options (structure with command line parameters values) to defaults.
{
	snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "No data.");
    COMMAND_LINE_PARMS* p = &parms;
    strcpy(p->optionSrcPath, APPCONST::DEFAULT_PATH_SRC);
    strcpy(p->optionDstPath, APPCONST::DEFAULT_PATH_DST);
    p->optionFileCount = APPCONST::DEFAULT_FILE_COUNT;
    p->optionFileSize = APPCONST::DEFAULT_FILE_SIZE;
    p->optionBlockSize = APPCONST::DEFAULT_BLOCK_SIZE;
    p->optionRepeats = APPCONST::DEFAULT_REPEATS;
    p->optionData = APPCONST::DEFAULT_DATA_TYPE;
    p->optionAddress = APPCONST::DEFAULT_ADDRESS_TYPE;
    p->optionNoBuf = APPCONST::DEFAULT_FLAG_NO_BUFFERING;
    p->optionWriteThr = APPCONST::DEFAULT_FLAG_WRITE_THROUGH;
    p->optionSequental = APPCONST::DEFAULT_FLAG_SEQUENTIAL_SCAN;
    p->optionWaitRead = APPCONST::DEFAULT_WAIT_READ;
    p->optionWaitWrite = APPCONST::DEFAULT_WAIT_WRITE;
    p->optionWaitCopy = APPCONST::DEFAULT_WAIT_COPY;
    p->optionQueue = APPCONST::DEFAULT_QUEUE_DEPTH;
    p->optionMinCpu = APPCONST::DEFAULT_CPU_SELECT;
    p->optionMaxCpu = APPCONST::DEFAULT_CPU_SELECT;
    p->optionMinDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    p->optionMaxDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
}
#define RDRAND_MASK 0x40000000
void correctAfterParse()                    // Verify options structure (reserved).
{
    COMMAND_LINE_PARMS* p = &parms;
    if (p->optionData == HARD_RANDOM)
    {   // If hardware random number generator selected by option, required check RDRAND instruction support.
        int cpudata[4];
        __cpuid(cpudata, 0);
        if (cpudata[0] >= 1)
        {
            __cpuid(cpudata, 1);
            if (!(cpudata[2] & RDRAND_MASK))
            {
                writeColor("RDRAND instruction not supported by CPU or locked. Software RNG used.\r\n", APPCONST::ERROR_COLOR);
                p->optionData = SOFT_RANDOM;
            }
        }
    }
    // This place reserved for other checks.
}
BOOL parseOptions(char* p, size_t n)
{
    // Pre-processing input file.
    size_t count = 0;
    char* src = p;
    char* dst = p;
    bool comment = false;
    for (size_t i = 0; i < n; i++)
    {
        char c = *(src++);
        if (c == ';')
        {
            comment = true;
        }
        else if ((c == 0xA) || (c == 0xD))
        {
            comment = false;
        }
        if ((c != ' ') && !comment)
        {
            *(dst++) = c;
            count++;
        }
    }
    *dst = 0;

    // Build array of virtual command line parameters.
    int optionsCount = 0;
    bool afterString = true;
    std::vector<char*> optionsPointers;
    for (size_t i = 0; i < count; i++)
    {
        if ((*p == 0xA) || (*p == 0xD))
        {
            *p = 0;
            afterString = true;
        }
        else if (afterString)
        {
            optionsPointers.push_back(p);
            afterString = false;
        }
        p++;
    }

    // Create and parse virtual command line.
    BOOL status = FALSE;
    int m = (int)optionsPointers.size();
    if (m > 0)
    {
        int virtualArgc = m + 1;
        char** virtualArgv = new char* [virtualArgc];
        virtualArgv[0] = nullptr;
        for (int i = 1; i < virtualArgc; i++)
        {
            virtualArgv[i] = optionsPointers[i - 1];
        }
        if (parseCommandLine(virtualArgc, virtualArgv) == 0)
        {
            status = TRUE;
        }
        delete[] virtualArgv;
    }
    return status;
}
// Definitions for parse command line
constexpr int SMIN = 3;      // Minimum option string length, example a=b.
constexpr int SMAX = 81;     // Maximum option and status string length.
// Parse command line, extract parameters, override defaults by user settings.
DWORD parseCommandLine(int argc, char** argv)
{
    // Regular data input support: command line parameters extract and parsing.
    int i = 0, j = 0, k = 0, k1 = 0, k2 = 0;  // miscellaneous counters and variables
    int recognized = 0;             // result of strings comparision, 0=match 
    OPTION_TYPES t = INTPARM;       // enumeration of parameters types for accept
    char* pAll = nullptr;           // pointer to option full string NAME=VALUE
    char* pName = nullptr;          // pointer to sub-string NAME
    char* pValue = nullptr;         // pointer to sub-string VALUE
    char* pPattern = nullptr;       // pointer to compared pattern string
    char** pPatterns = nullptr;     // pointer to array of pointers to pat. strings
    DWORD* pDword = nullptr;        // pointer to integer (32b) for variable store
    DWORDLONG* pLong = nullptr;     // pointer to long (64b) for variable store
    DWORDLONG k64;                  // transit variable for memory block size
    char c = 0;                     // transit storage for char
    char cmdName[SMAX];             // extracted NAME of option string
    char cmdValue[SMAX];            // extracted VALUE of option string
    int MSG_MAX = 80 + SMAX;
    // Verify command line format and extract values	
    for (i = 1; i < argc; i++)  // cycle for command line options
    {
        // initializing for parsing current string
        // because element [0] is application name, starts from [1]
        pAll = argv[i];
        for (j = 0; j < SMAX; j++)  // clear buffers
        {
            cmdName[j] = 0;
            cmdValue[j] = 0;
        }
        // check option sub-string length
        k = (int)strlen(pAll);                   // k = length of one option sub-string
        if (k < SMIN)
        {
            snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Option too short: %s.", pAll);
            return -1;
        }
        if (j > SMAX)
        {
            snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Option too long: %s.", pAll);
            return -1;
        }
        // Parse command line parameters, setup input parameters, visual
        // extract option name and option value substrings
        pName = cmdName;
        pValue = cmdValue;
        strcpy(pName, pAll);                 // store option sub-string to pName
        char* dummy = strtok(pName, "=");    // pName = pointer to fragment before "="
        pValue = strtok(nullptr, "?");       // pValue = pointer to fragment after "="
        // check option name and option value substrings
        k1 = 0;
        k2 = 0;
        if (pName) { k1 = (int)strlen(pName); }
        if (pValue) { k2 = (int)strlen(pValue); }
        if ((k1 == 0) || (k2 == 0))
        {
            snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Invalid option: %s.", pAll);
            return -1;
        }
        // detect option by comparision from list, cycle for supported options
        j = 0;
        while (options[j].name != NULL)
        {
            pPattern = (char*)options[j].name;
            recognized = strcmp(pName, pPattern);
            if (recognized == 0)
            {
                // option-type specific handling, run if name match
                t = options[j].routine;
                switch (t)
                {
                case INTPARM:  // support integer parameters
                {
                    k1 = (int)strlen(pValue);
                    for (k = 0; k < k1; k++)
                    {
                        if (isdigit(pValue[k]) == 0)
                        {
                            snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Not a number: %s.", pValue);
                            return -1;
                        }
                    }
                    k = atoi(pValue);   // convert string to integer
                    pDword = (DWORD*)(options[j].data);
                    *pDword = k;
                    break;
                }
                case MEMPARM:  // support memory block size parameters
                {
                    k1 = 0;
                    k2 = (int)strlen(pValue);
                    c = pValue[k2 - 1];
                    if (isdigit(c) != 0)
                    {
                        k1 = 1;             // no units kilo, mega, giga
                    }
                    else if (c == 'K')    // K means kilobytes
                    {
                        k2--;               // last char not a digit K/M/G
                        k1 = 1024;
                    }
                    else if (c == 'M')    // M means megabytes
                    {
                        k2--;
                        k1 = 1024 * 1024;
                    }
                    else if (c == 'G')    // G means gigabytes
                    {
                        k2--;
                        k1 = 1024 * 1024 * 1024;
                    }
                    for (k = 0; k < k2; k++)
                    {
                        if (isdigit(pValue[k]) == 0)
                        {
                            k1 = 0;
                        }
                    }
                    if (k1 == 0)
                    {
                        snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Not a block size: %s.", pValue);
                        return -1;
                    }
                    k = atoi(pValue);   // convert string to integer
                    k64 = k;
                    k64 *= k1;
                    pLong = (DWORDLONG*)(options[j].data);
                    *pLong = k64;
                    break;
                }
                case SELPARM:    // support parameters selected from text names
                {
                    k = 0;
                    k2 = 0;
                    pPatterns = (char**)options[j].values;
                    while (pPatterns[k] != NULL)
                    {
                        pPattern = pPatterns[k];
                        k2 = strcmp(pValue, pPattern);
                        if (k2 == 0)
                        {
                            pDword = (DWORD*)(options[j].data);
                            *pDword = k;
                            break;
                        }
                        k++;
                    }
                    if (k2 != 0)
                    {
                        snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Value invalid: %s.", pAll);
                        return -1;
                    }
                    break;
                }
                case STRPARM:    // support parameter as text string
                {
                    // pPatterns = path = pointer to pathBuffer
                    // pValue = pointer to source temp parsing buffer
                    pPatterns = (CHAR**)(&options[j].data);    // OLD:  pPatterns = (CHAR**)(options[j].data);
                    strcpy(*pPatterns, pValue);                // OLD: strcpy(*pPatterns, pValue);
                    break;
                }
                }
                break;
            }
            j++;
        }
        // check option name recognized or not
        if (recognized != 0)
        {
            snprintf(optionStatusString, APPCONST::MAX_TEXT_STRING, "Option not recognized: %s.", pName);
            return -1;
        }
    }
    return 0;
}
void writeOptionReport()
{
    writeColor(" Benchmark scenario options.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);

    char msg[APPCONST::MAX_TEXT_STRING];
    const OPTION_ENTRY* p = options;
    while (p->name)
    {
        switch (p->routine)
        {
        case INTPARM:
            snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d", *((int*)(p->data)));
            break;
        case SELPARM:
        {
            int index = *((int*)(p->data));
            if (index < 0)
            {
                snprintf(msg, APPCONST::MAX_TEXT_STRING, "default");
            }
            else
            {
                const char** keyNames = p->values;
                const char* name = keyNames[index];
                snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s", name);;
            }
            break;
        }
        case MEMPARM:
        {
            DWORD64 a = *((DWORD64*)(p->data));
            if (a == 0xFFFFFFFFFFFFFFFFL)
            {
                snprintf(msg, APPCONST::MAX_TEXT_STRING, "default");
            }
            else
            {
                storeCellMemorySize(msg, APPCONST::MAX_TEXT_STRING, a, 20);
            }
            break;
        }
        case STRPARM:
            snprintf(msg, APPCONST::MAX_TEXT_STRING, (char*)(p->data));
            break;
        case NOPARM:
        default:
            snprintf(msg, APPCONST::MAX_TEXT_STRING, "?");
            break;
        }
        writeColor(" ", APPCONST::TABLE_COLOR);
        writeParmAndValue(p->name, msg, 12, APPCONST::GROUP_COLOR);
        p++;
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
}
BOOL initConsole(bool screenMode, bool fileMode, char* outPointer, size_t maxOutSize)
{
    gScreenMode = screenMode;
    gFileMode = fileMode;
    gOutPointer = outPointer;
    gMaxOutSize = maxOutSize;
    gCurrentOutSize = 0;
    gHstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (gHstdout == INVALID_HANDLE_VALUE) return FALSE;
    return GetConsoleScreenBufferInfo(gHstdout, &csbi);
}
void write(const char* string)
{
    if (gScreenMode)
    {
        using namespace std;
        cout << string;
    }
    if (gFileMode)
    {
        size_t limit = gMaxOutSize - gCurrentOutSize;
        size_t addend = snprintf(gOutPointer, limit, string);
        gOutPointer += addend;
        gCurrentOutSize += addend;
    }
}
void writeColor(const char* string, WORD color)
{
    colorHelper(color);
    write(string);
    colorRestoreHelper();
}
void writeParmGroup(const char* string)
{
    char temp[APPCONST::MAX_TEXT_STRING];
    snprintf(temp, APPCONST::MAX_TEXT_STRING, "[%s]\r\n", string);
    colorHelper(APPCONST::GROUP_COLOR);
    write(temp);
    colorRestoreHelper();
}
void writeParmError(const char* string)
{
    char temp[APPCONST::MAX_TEXT_STRING];
    snprintf(temp, APPCONST::MAX_TEXT_STRING, "ERROR: %s.\r\n", string);
    colorHelper(APPCONST::ERROR_COLOR);
    write(temp);
    colorRestoreHelper();
}
void writeParmAndValue(const char* parmName, const char* parmValue, int nameWidth, WORD valueColor)
{
    char temp[APPCONST::MAX_TEXT_STRING];
    int n = snprintf(temp, APPCONST::MAX_TEXT_STRING, "%s", parmName);
    int m = nameWidth - n;
    int limit = APPCONST::MAX_TEXT_STRING - 1;
    if ((n + m) > limit)
    {
        m = limit - n;
    }
    memset(temp + n, ' ', m);
    temp[m + n] = 0;
    colorHelper(APPCONST::PARM_COLOR);
    write(temp);
    snprintf(temp, APPCONST::MAX_TEXT_STRING, "%s\r\n", parmValue);
    colorHelper(valueColor);
    write(temp);
    colorRestoreHelper();
}
void writeHexDump(byte* ptr, int size)
{
    // initialization
    char temp[APPCONST::MAX_TEXT_STRING];
    int address = 0;
    byte* ptr1;
    char* temp1;
    int n;
    int m;
    int k;
    // cycle for dump strings
    int lines = size / APPCONST::DUMP_BYTES_PER_LINE;
    if ((size % APPCONST::DUMP_BYTES_PER_LINE) > 0)
    {
        lines++;
    }
    for (int i = 0; i < lines; i++)
    {
        // print address part of dump string
        colorHelper(APPCONST::DUMP_ADDRESS_COLOR);
        temp1 = temp;
        temp1 += snprintf(temp, APPCONST::MAX_TEXT_STRING, " %08X  ", address);
        write(temp);
        // print data part of dump string
        colorHelper(APPCONST::DUMP_DATA_COLOR);
        if (size > APPCONST::DUMP_BYTES_PER_LINE)
        {
            n = APPCONST::DUMP_BYTES_PER_LINE;
        }
        else
        {
            n = size;
        }
        m = APPCONST::MAX_TEXT_STRING;
        ptr1 = ptr;
        temp1 = temp;
        for (int j = 0; j < n; j++)
        {
            k = snprintf(temp1, m, "%02X ", *ptr1++);
            temp1 += k;
            m -= k;
        }
        for (int j = 0; j < (APPCONST::DUMP_BYTES_PER_LINE - n); j++)
        {
            k = snprintf(temp1, m, "   ");
            temp1 += k;
            m -= k;
        }
        k = snprintf(temp1, m, " ");
        temp1 += k;
        m -= k;
        write(temp);
        // print text part of dump string
        colorHelper(APPCONST::DUMP_TEXT_COLOR);
        m = APPCONST::MAX_TEXT_STRING;
        ptr1 = ptr;
        temp1 = temp;
        for (int j = 0; j < n; j++)
        {
            char c = *ptr1++;
            if ((c < ' ') || (c > '}'))
            {
                c = '.';
            }
            k = snprintf(temp1, m, "%c", c);
            temp1 += k;
            m -= k;
        }
        for (int j = 0; j < (APPCONST::DUMP_BYTES_PER_LINE - n); j++)
        {
            k = snprintf(temp1, m, " ");
            temp1 += k;
            m -= k;
        }
        write(temp);
        write("\r\n");
        // cycle for dump strings
        address += APPCONST::DUMP_BYTES_PER_LINE;
        ptr += APPCONST::DUMP_BYTES_PER_LINE;
        size -= APPCONST::DUMP_BYTES_PER_LINE;
    }
    // restore color
    colorRestoreHelper();
}
void writeHexDumpUp()
{
    writeLine(APPCONST::DUMP_WIDTH);
    writeColor(" Offset    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF\r\n",
        APPCONST::TABLE_COLOR);
    writeLine(APPCONST::DUMP_WIDTH);
}
void writeLine(int count)
{
    int limit = APPCONST::MAX_TEXT_STRING - 3;
    if (count > limit)
    {
        count = limit;
    }
    char temp[APPCONST::MAX_TEXT_STRING];
    memset(temp, '-', count);
    temp[count] = '\r';
    temp[count + 1] = '\n';
    temp[count + 2] = 0;
    write(temp);
}
void writeColorLine(int count, WORD color)
{
    colorHelper(color);
    writeLine(count);
    colorRestoreHelper();
}
size_t storeHex8(char* buffer, size_t limit, DWORD32 value, bool h)
{
    value &= 0xFF;
    if (h)
    {
        return snprintf(buffer, limit, "%02Xh", value);
    }
    else
    {
        return snprintf(buffer, limit, "%02X", value);
    }
}
size_t storeHex16(char* buffer, size_t limit, DWORD32 value, bool h)
{
    value &= 0xFFFF;
    if (h)
    {
        return snprintf(buffer, limit, "%04Xh", value);
    }
    else
    {
        return snprintf(buffer, limit, "%04X", value);
    }
}
size_t storeHex32(char* buffer, size_t limit, DWORD32 value, bool h)
{
    if (h)
    {
        return snprintf(buffer, limit, "%08Xh", value);
    }
    else
    {
        return snprintf(buffer, limit, "%08X", value);
    }
}
size_t storeHex64(char* buffer, size_t limit, DWORD64 value, bool h)
{
    DWORD32 low = value & 0xFFFFFFFFL;
    DWORD32 high = (value >> 32) & 0xFFFFFFFFL;
    if (h)
    {
        return snprintf(buffer, limit, "%08X%08Xh", high, low);
    }
    else
    {
        return snprintf(buffer, limit, "%08X%08X", high, low);
    }
}
size_t storeHexPointer(char* buffer, size_t limit, LPVOID ptr, bool h)
{
#ifdef NATIVE_WIDTH_64
    DWORD64 value = (DWORD64)ptr;
    return storeHex64(buffer, limit, value, h);
#endif
#ifdef NATIVE_WIDTH_32
    DWORD32 value = (DWORD32)ptr;
    return storeHex32(buffer, limit, value, h);
#endif
}
// Print list of "1" bits.
void storeBitsList(char* buffer, size_t limit, DWORD64 x)
{
    int count = 0;
    int i = 0;
    int j = 0;
    DWORD64 mask = 1;
    BOOL flag = FALSE;
    // This required if no "1" bits
    *buffer = 0;
    // Print cycle
    while (i < 64)
    {
        // Detect group of set bits
        while ((!(x & mask)) && (i < 64))
        {
            i++;
            mask = (DWORD64)1 << i;
        }
        j = i;
        while ((x & mask) && (j < 64))
        {
            j++;
            mask = (DWORD64)1 << j;
        }
        j--;
        // Print group of set bits
        if (i < 64)
        {
            if (flag)
            {
                count = snprintf(buffer, limit, ", ");
                buffer += count;
                limit -= count;
            }
            if (limit <= 0) break;
            if (i == j)
            {
                count = snprintf(buffer, limit, "%d", i);
            }
            else
            {
                count = snprintf(buffer, limit, "%d-%d", i, j);
            }
            buffer += count;
            limit -= count;
            if (limit <= 0) break;
            j++;
            i = j;
            flag = TRUE;
        }
    }
    return;
}
// Print memory size string (Bytes, KB, MB, GB) to buffer.
void storeCellMemorySize(char* buffer, size_t limit, DWORD64 x, size_t cell)
{
    // Print number at left of fixed size cell.
    double xd = static_cast<double>(x);
    int count = 0;
    if ((x < APPCONST::KILO) || ((x < APPCONST::KILO) && (x % APPCONST::KILO)))
    {
        int xi = (int)x;
        count = snprintf(buffer, limit, "%d bytes", xi);
    }
    else if (x < APPCONST::MEGA)
    {
        xd /= APPCONST::KILO;
        count = snprintf(buffer, limit, "%.3f KB", xd);
    }
    else if (x < APPCONST::GIGA)
    {
        xd /= APPCONST::MEGA;
        count = snprintf(buffer, limit, "%.3f MB", xd);
    }
    else
    {
        xd /= APPCONST::GIGA;
        count = snprintf(buffer, limit, "%.3f GB", xd);
    }
    buffer += count;
    limit -= count;
    // Print extra spaces for fill fixed size cell.
    cellPrintHelper(buffer, limit, cell, count);
}
void storeCellMemorySizeInt(char* buffer, size_t limit, DWORD64 x, size_t cell, int mode)
{
    // Print number at left of fixed size cell.
    int count = 0;
    if ((mode == PRINT_SIZE_BYTES) || ((mode == PRINT_SIZE_AUTO) && ((x < APPCONST::KILO) || (x % APPCONST::KILO))))
    {
        count = snprintf(buffer, limit, "%llu", x);
    }
    else if ((mode == PRINT_SIZE_KB) || ((mode == PRINT_SIZE_AUTO) && ((x < APPCONST::MEGA) || (x % APPCONST::MEGA))))
    {
        count = snprintf(buffer, limit, "%lluK", x / APPCONST::KILO);
    }
    else if ((mode == PRINT_SIZE_MB) || ((mode == PRINT_SIZE_AUTO) && ((x < APPCONST::GIGA) || (x % APPCONST::GIGA))))
    {
        count = snprintf(buffer, limit, "%lluM", x / APPCONST::MEGA);
    }
    else if (mode == PRINT_SIZE_GB)
    {
        count = snprintf(buffer, limit, "%lluG", x / APPCONST::GIGA);
    }
    else
    {
        count = snprintf(buffer, limit, "?");
    }
    buffer += count;
    limit -= count;
    // Print extra spaces for fill fixed size cell.
    cellPrintHelper(buffer, limit, cell, count);
}
void storeBaseAndSize(char* buffer, size_t limit, DWORD64 blockBase, DWORD64 blockSize)
{
    size_t count = 0;
    count = snprintf(buffer, limit, "base=");
    buffer += count;
    limit -= count;
    if (limit > 0)
    {
        count = storeHex64(buffer, limit, blockBase, TRUE);
        buffer += count;
        limit -= count;
        if (limit > 0)
        {
            count = snprintf(buffer, limit, ", size=");
            buffer += count;
            limit -= count;
            if (limit > 0)
            {
                storeHex64(buffer, limit, blockSize, TRUE);
            }
        }
    }
}
// Build executable path at destination buffer, use const file name.
// This function used for save text report in the executable file directory,
// not in the current directory.
BOOL storeExecReportPath(char* buffer, size_t limit, const char* name)
{
    memset(buffer, 0, limit);                               // blank buffer
    int n = GetModuleFileName(NULL, buffer, (DWORD)limit);  // get this executable file path by WinAPI
    int i;
    LPSTR p = buffer + n - 1;
    for (i = 0; i < n; i++)     // cycle for remove this executable file name
    {
        if (*p == '\\') break;  // detect path separator
        *p = 0;
        p--;
    }
    if (i >= n) return FALSE;  // return with error if path separator '\' not found
    p++;
    while (*p++ = *name++);    // copy report file name instead just blanked executable file name
    return TRUE;
}
void storeSystemErrorName(char* buffer, size_t limit, DWORD errorCode)
{
    // This required if no strings stored.
    *buffer = 0;
    // Reject 0 = no OS errors, -1 = error without OS API code.
    if ((errorCode > 0) && (errorCode < 0x7FFFFFFF))
    {
        // Local variables
        LPVOID lpvMessageBuffer;
        DWORD status;
        // Build message string = f (error code)
        status = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode,
            MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),  // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpvMessageBuffer, 0, NULL);
        if (status)
        {
            // This visualized if error string build OK.
            snprintf(buffer, limit, "OS API reports error %d = %s", errorCode, (char*)lpvMessageBuffer);
        }
        else
        {
            DWORD dwError = GetLastError();
            // This visualized if build error string FAILED.
            snprintf(buffer, limit, "Decode OS error failed, format message error %d", dwError);
        }
        // Free the buffer allocated by the system API function
        LocalFree(lpvMessageBuffer);
    }
}
void calculateStatistics(std::vector<double> data, double& min, double& max, double& average, double& median)
{
    size_t n = data.size();
    if (n)
    {
        std::sort(data.begin(), data.end());
        double sum = std::accumulate(data.begin(), data.end(), 0);
        min = data[0];
        max = data[n - 1];
        average = sum / n;
        median = (n % 2) ? (data[n / 2]) : ((data[n / 2 - 1] + data[n / 2]) / 2.0);
    }
    else
    {
        min = 0.0;
        max = 0.0;
        average = 0.0;
        median = 0.0;
    }
}
void colorHelper(WORD color)
{
    if (gScreenMode)
    {
        SetConsoleTextAttribute(gHstdout, (csbi.wAttributes & 0xF0) | (color & 0x0F));
    }
}
void colorRestoreHelper()
{
    if (gScreenMode)
    {
        SetConsoleTextAttribute(gHstdout, csbi.wAttributes);
    }
}
void cellPrintHelper(char* buffer, size_t limit, size_t cell, size_t count)
{
    int extra = (int)(cell - count);
    if ((extra > 0) && (limit > 0))
    {
        for (int i = 0; i < extra; i++)
        {
            count = snprintf(buffer, limit, " ");
            buffer += count;
            limit -= count;
            if (limit <= 0) break;
        }
    }
}
// Target operation-specific tasks (measure storage performance in this application).
void waitTime(char* msg, DWORD milliseconds, const char* operationName)
{
    if (milliseconds)
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\n Wait before start %s (%d ms).", operationName, milliseconds);
        writeColor(msg, APPCONST::VALUE_COLOR);
        Sleep(milliseconds);
    }
}
void writeStatistics(char* msg, const char* statisticsName, std::vector<double> speeds, bool tableMode)
{
    double min = 0.0, max = 0.0, average = 0.0, median = 0.0;
    calculateStatistics(speeds, min, max, average, median);
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s", statisticsName);
    writeColor(msg, APPCONST::GROUP_COLOR);
    if (tableMode)
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING,
            "%14.3f%14.3f%14.3f%14.3f", min, max, average, median);
    }
    else
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING,
            "min = %8.3f,  max = %8.3f,  average = %8.3f,  median = %8.3f.",
            min, max, average, median);
    }
    writeColor(msg, APPCONST::VALUE_COLOR);
}
void buildData(char* msg, LARGE_INTEGER &hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize)
{
    const char* dataName = "?";
    BOOL dataStatus = FALSE;
    LARGE_INTEGER tm1, tm2;
    BOOL st1 = QueryPerformanceCounter(&tm1);
    switch (dataType)
    {
    case ALL_ZEROES:
        memset(fileData, 0, fileSize);
        dataName = DATA_NAMES[0];
        dataStatus = TRUE;
        break;
    case ALL_ONES:
        memset(fileData, -1, fileSize);
        dataName = DATA_NAMES[1];
        dataStatus = TRUE;
        break;
    case INCREMENTAL:
    {
        int* p = reinterpret_cast<int*>(fileData);
        int n = static_cast<int>(fileSize / 4);
        for (int i = 0; i < n; i++)
        {
            *p = i;
            p++;
        }
    }
    dataName = DATA_NAMES[2];
    dataStatus = TRUE;
    break;
    case SOFT_RANDOM:
    {
        DWORD64* p = reinterpret_cast<DWORD64*>(fileData);
        int n = static_cast<int>(fileSize / 8);
        DWORD64 seed = APPCONST::RANDOM_SEED;
        for (int i = 0; i < n; i++)
        {
            seed *= APPCONST::RANDOM_MULTIPLIER;
            seed += APPCONST::RANDOM_ADDEND;
            *p = seed;
            p++;
        }
    }
    dataName = DATA_NAMES[3];
    dataStatus = TRUE;
    break;
    case HARD_RANDOM:  // For hardware randomization, use x86/x64-specific RDRAND instruction.
    {
#ifdef NATIVE_WIDTH_64
        DWORD64* qwordPtr = reinterpret_cast<DWORD64*>(fileData);
        unsigned int qwordCount = static_cast<unsigned int>(fileSize / 8);
        for (unsigned int i = 0; i < qwordCount; i++)
        {
            while (!_rdrand64_step(qwordPtr));
            qwordPtr++;
        }
#else
        DWORD32* dwordPtr = reinterpret_cast<DWORD32*>(fileData);
        unsigned int dwordCount = static_cast<unsigned int>(fileSize / 4);
        for (unsigned int i = 0; i < dwordCount; i++)
        {
            while (!_rdrand32_step(dwordPtr));
            dwordPtr++;
        }
#endif
    }
    dataName = DATA_NAMES[4];
    dataStatus = TRUE;
    break;
    default:
        dataName = "Invalid selection for file data";
        dataStatus = FALSE;
        break;
    }
    BOOL st2 = QueryPerformanceCounter(&tm2);
    if (!dataStatus)
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\n%s", dataName);
    }
    else if (!(st1 && st2))
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nMemory initializing timings failed.");
    }
    else
    {
        double unit = 1.0 / hz.QuadPart;
        double seconds = (tm2.QuadPart - tm1.QuadPart) * unit;
        double megabytes = fileSize / 1000000.0;
        double mbps = megabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nMemory initializing (%s), fill rate = %.3f MBPS.", dataName, mbps);
    }
    write(msg);
}
void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR> &list, 
    unsigned int blocksPerFile, unsigned int fileCount)
{
    list.clear();
    DWORD32 randomNumber;
    DWORD32 seed = APPCONST::RANDOM_SEED;
    for (unsigned int i = 0; i < 2; i++)  // Required 2 passes for src and dst files groups.
    {
        for (unsigned int j = 0; j < fileCount; j++)
        {
            for (unsigned int k = 0; k < blocksPerFile; k++)
            {
                switch (dataType)
                {
                case HARD_RANDOM:  // For hardware randomization, use x86/x64-specific RDRAND instruction.
                    while (!_rdrand32_step(&randomNumber));
                    break;
                default:
                    seed *= APPCONST::RANDOM_MULTIPLIER;
                    seed += APPCONST::RANDOM_ADDEND;
                    randomNumber = seed;
                    break;
                }
                IO_DESCRIPTOR iodesc{ randomNumber, k, j, i };
                list.push_back(iodesc);
            }
        }
    }
    std::sort(list.begin(), list.end(), ioDescriptorComparator);
}
bool ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2)
{
    return d1.random < d2.random;
}
BOOL createEvents(char* msg, std::vector<HANDLE> &events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count)
{
    BOOL status = TRUE;
    for (unsigned int i = 0; i < count; i++)
    {
        HANDLE h = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (h)
        {
            events.push_back(h);
            OVERLAPPED overlapped{ 0 };
            overlapped.hEvent = h;
            overlaps.push_back(overlapped);
            returns.push_back(0);
        }
        else
        {
            status = FALSE;
            break;
        }
    }
    if (status)
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nAsync I/O initializing, %d events created.", count);
        write(msg);
    }
    return status;
}
BOOL closeEvents(std::vector<HANDLE> &events)
{
    BOOL status = TRUE;
    size_t n = events.size();
    for (size_t i = 0; i < n; i++)
    {
        HANDLE h = events[i];
        if (h)
        {
            if (!CloseHandle(h))
            {
                status = FALSE;
            }
        }
    }
    return status;
}
STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity)
{
    STATUS_CODES statusCode = NO_ERRORS;
    // Memory allocation.
    // TODO. Support minDomain-maxDomain range required. Yet minDomain only used, single domain only.
    if (minDomain != APPCONST::DEFAULT_DOMAIN_SELECT)
    {
        fileData = VirtualAllocExNuma(GetCurrentProcess(), nullptr, fileSize, MEM_RESERVE + MEM_COMMIT, PAGE_READWRITE, minDomain);
        if (!fileData) statusCode = MEMORY_NUMA_ALLOCATION_FAILED;
    }
    else
    {
        fileData = VirtualAllocEx(GetCurrentProcess(), nullptr, fileSize, MEM_RESERVE + MEM_COMMIT, PAGE_READWRITE);
        if (!fileData) statusCode = MEMORY_ALLOCATION_FAILED;
    }
    // Set thread affinity.
    // TODO. Support minCpu-maxCpu range required. Yet minCpu only used, single logical CPU only.
    // TODO. Check affinity mask validity.
    // TODO. Support >64 logical processors by Processor Groups.
    if ((statusCode == NO_ERRORS) && (minCpu != APPCONST::DEFAULT_CPU_SELECT))
    {
        {
            DWORD_PTR mask = 1;
            mask = mask << minCpu;
            previousAffinity = SetThreadAffinityMask(GetCurrentThread(), mask);
            if (!previousAffinity)
            {
                statusCode = SET_CPU_AFFINITY_FAILED;
            }
        }
    }
    return statusCode;
}
STATUS_CODES closeContext(STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity)
{
    STATUS_CODES statusCode = operationStatus;
    if (previousAffinity)
    {
        DWORD_PTR statusAffinity = SetThreadAffinityMask(GetCurrentThread(), previousAffinity);
        if ((statusCode != NO_ERRORS) && (!statusAffinity)) statusCode = RESTORE_CPU_AFFINITY_FAILED;
    }
    if (!fileData)
    {
        if(statusCode != NO_ERRORS) statusCode = MEMORY_RELEASE_FAILED;
    }
    else
    {
        if (!VirtualFreeEx(GetCurrentProcess(), fileData, 0, MEM_RELEASE))
        {
            if (statusCode != NO_ERRORS) statusCode = MEMORY_RELEASE_FAILED;
        }
    }
    return statusCode;
}
int runTaskMBPS(COMMAND_LINE_PARMS* p)
{
    // Initializing variables by constants.
    char msg[APPCONST::MAX_TEXT_STRING];
    const char* nameSrcPath = APPCONST::DEFAULT_PATH_SRC;
    const char* nameDstPath = APPCONST::DEFAULT_PATH_DST;
    const char* nameSrc = APPCONST::DEFAULT_NAME_SRC;
    const char* nameDst = APPCONST::DEFAULT_NAME_DST;
    const char* nameExt = APPCONST::DEFAULT_EXTENSION;
    DWORD32 fileCount = APPCONST::DEFAULT_FILE_COUNT;
    DWORD32 fileSize = APPCONST::DEFAULT_FILE_SIZE;
    DWORD32 blockSize = APPCONST::DEFAULT_BLOCK_SIZE;
    DWORD32 repeats = APPCONST::DEFAULT_REPEATS;
    D_TYPE dataType = APPCONST::DEFAULT_DATA_TYPE;
    A_TYPE addressType = APPCONST::DEFAULT_ADDRESS_TYPE;
    BOOL flagNoBuffering = APPCONST::DEFAULT_FLAG_NO_BUFFERING;
    BOOL flagWriteThrough = APPCONST::DEFAULT_FLAG_WRITE_THROUGH;
    BOOL flagSequentalScan = APPCONST::DEFAULT_FLAG_SEQUENTIAL_SCAN;
    DWORD32 msWaitRead = APPCONST::DEFAULT_WAIT_READ;
    DWORD32 msWaitWrite = APPCONST::DEFAULT_WAIT_WRITE;
    DWORD32 msWaitCopy = APPCONST::DEFAULT_WAIT_COPY;
    DWORD32 minCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 maxCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 minDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    DWORD32 maxDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    // If input parameters structure passed, load parameters from this structure.
    if (p)
    {
        nameSrcPath = p->optionSrcPath;
        nameDstPath = p->optionDstPath;
        fileCount = p->optionFileCount;
        fileSize = static_cast<DWORD32>(p->optionFileSize);
        blockSize = static_cast<DWORD32>(p->optionBlockSize);
        repeats = p->optionRepeats;
        dataType = static_cast<D_TYPE>(p->optionData);
        addressType = static_cast<A_TYPE>(p->optionAddress);
        flagNoBuffering = p->optionNoBuf;
        flagWriteThrough = p->optionWriteThr;
        flagSequentalScan = p->optionSequental;
        msWaitRead = p->optionWaitRead;
        msWaitWrite = p->optionWaitWrite;
        msWaitCopy = p->optionWaitCopy;
        minCpu = p->optionMinCpu;
        maxCpu = p->optionMaxCpu;
        minDomain = p->optionMinDomain;
        maxDomain = p->optionMaxDomain;
    }
    // Initializing variables for read, write and copy total sizes verification.
    DWORD64 totalRead = 0;
    DWORD64 totalWrite = 0;
    DWORD64 totalCopyRead = 0;
    DWORD64 totalCopyWrite = 0;
    DWORD64 totalBytes = (DWORD64)fileSize * (DWORD64)fileCount * (DWORD64)repeats;
    // Initializing timer (use OS performance counter).
    LARGE_INTEGER hz;
    BOOL status;
    status = QueryPerformanceFrequency(&hz);
    if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
    write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    write("\r\nMemory allocated, ");
    storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    write(msg);
    write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    // Create source files.
    std::vector<HANDLE> srcHandles;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)handle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        write(msg);
        if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(handle);
        }
        else
        {
            createError = TRUE;
            break;
        }
    }
    // Create destination files.
    std::vector<HANDLE> dstHandles;
    if (!createError)
    {
        for (unsigned int i = 0; i < fileCount; i++)
        {
            snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
            DWORD attributes = FILE_ATTRIBUTE_NORMAL;
            if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
            if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
            if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
            HANDLE handle = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
            DWORD64 numHandle = (DWORD64)handle;
#else
            DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            write(msg);
            if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
            {
                dstHandles.push_back(handle);
            }
            else
            {
                createError = TRUE;
                break;
            }
        }
    }
    // Both source and destination files created (yet zero size).
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(FILE_CREATE_FAILED, fileData, previousAffinity);
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // Initializing for write source files.
    std::vector<double> writeSpeed;
    double writeSpeedIntegral = 0.0;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    // Table up for WRITE operation.
    writeColor("\r\n Write file performance.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL writeError = FALSE;
    // Start timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Start cycle for write source files (re-positioning file pointer required because repeats).
    for (unsigned int i = 0; i < fileCount; i++)
    {
        // Start timer for one source file write.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandles[i];
            DWORD32 writeSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesWritten = 0;
            LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
            // Seek (restore file pointer to 0) and write one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                writeError = TRUE;
                break;
            }
            while (writeSize)
            {
                DWORD nNumberOfBytesToWrite = blockSize;
                if (writeSize >= blockSize)
                {
                    writeSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToWrite = writeSize;
                    writeSize = 0;
                }
                while (nNumberOfBytesToWrite > 0)
                {
                    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                    {
                        writeError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                    {
                        writeError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesWritten;
                    nNumberOfBytesToWrite -= nNumberOfBytesWritten;
                    totalWrite += nNumberOfBytesWritten;
                }
                if (writeError) break;
            }
            if (writeError) break;
        }  // End cycle for measurement repeats.
        // Stop timer for one source file write.
        QueryPerformanceCounter(&t4);
        if (writeError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        writeSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
        write(msg);
    }
    // Stop timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Initializing for copy source files to destination files (re-positioning required for source files after it writes).
    std::vector<double> copySpeed;
    double copySpeedIntegral = 0.0;
    // Wait before COPY operation start, if selected by option.
    waitTime(msg, msWaitCopy, "copy");
    // Table up for COPY operation.
    writeColor("\r\n Copy file performance.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL copyError = FALSE;
    // Start timer for integral time of copy files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Start cycle for copy files  (re-positioning file pointer required because repeats).
    for (unsigned int i = 0; i < fileCount; i++)
    {
        // Start timer for one source file write to destination file.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandles[i];
            DWORD32 readSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesRead = 0;
            LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
            // Seek (restore file pointer to 0) and read one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                copyError = TRUE;
                break;
            }
            // Read one source file.
            while (readSize)
            {
                DWORD nNumberOfBytesToRead = blockSize;
                if (readSize >= blockSize)
                {
                    readSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToRead = readSize;
                    readSize = 0;
                }
                while (nNumberOfBytesToRead > 0)
                {
                    if (!ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr))
                    {
                        copyError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesRead <= 0) || (nNumberOfBytesRead > nNumberOfBytesToRead))
                    {
                        copyError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesRead;
                    nNumberOfBytesToRead -= nNumberOfBytesRead;
                    totalCopyRead += nNumberOfBytesRead;
                }
                if (copyError) break;
            }
            if (copyError) break;
            // Variables for write one destination file.
            hFile = dstHandles[i];
            DWORD32 writeSize = fileSize;
            lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesWritten = 0;
            LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
            // Seek (restore file pointer to 0) and read one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                copyError = TRUE;
                break;
            }
            // Write one destination file.
            while (writeSize)
            {
                DWORD nNumberOfBytesToWrite = blockSize;
                if (writeSize >= blockSize)
                {
                    writeSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToWrite = writeSize;
                    writeSize = 0;
                }

                while (nNumberOfBytesToWrite > 0)
                {
                    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                    {
                        copyError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                    {
                        copyError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesWritten;
                    nNumberOfBytesToWrite -= nNumberOfBytesWritten;
                    totalCopyWrite += nNumberOfBytesWritten;
                }
                if (copyError) break;
            }
        }  // End cycle for measurement repeats.
        // Stop timer for one source file write to destination file.
        QueryPerformanceCounter(&t4);
        // End of copy one file (read one source file and write one destination file).
        if (copyError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        copySpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
        write(msg);
    }
    // Stop timer for integral time of copy files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (copyError)
    {
        return closeContext(FILE_COPY_FAILED, fileData, previousAffinity);
    }
    // Initializing for read source files (re-positioning required for source files after it copy).
    std::vector<double> readSpeed;
    double readSpeedIntegral = 0.0;
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    // Table up for READ operation.
    writeColor("\r\n Read file performance.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL readError = FALSE;
    // Start timer for integral time of read files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Read source files - start read cycle  (re-positioning file pointer required because repeats).
    for (unsigned int i = 0; i < fileCount; i++)
    {
        // Start timer for one source file read.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandles[i];
            DWORD32 readSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesRead = 0;
            LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                readError = TRUE;
                break;
            }
            while (readSize)
            {
                DWORD nNumberOfBytesToRead = blockSize;
                if (readSize >= blockSize)
                {
                    readSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToRead = readSize;
                    readSize = 0;
                }

                while (nNumberOfBytesToRead > 0)
                {
                    if (!ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr))
                    {
                        readError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesRead <= 0) || (nNumberOfBytesRead > nNumberOfBytesToRead))
                    {
                        readError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesRead;
                    nNumberOfBytesToRead -= nNumberOfBytesRead;
                    totalRead += nNumberOfBytesRead;
                }
                if (readError) break;
            }
        }  // End cycle for measurement repeats.
        // Stop timer for one source file read.
        QueryPerformanceCounter(&t4);
        if (readError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        readSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
        write(msg);
    }
    // Stop timer for integral time of read source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // Close and delete files.
    writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Delete source and destination files done.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    if ((totalBytes > 0) && 
        (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    {   // Write performance measurement statistics, if total size verification passed.
        writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readSpeed, true);
        writeStatistics(msg, "\r\n WRITE ", writeSpeed, true);
        writeStatistics(msg, "\r\n COPY  ", copySpeed, true);
        write("\r\n");
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int runTaskMBPSoneFile(COMMAND_LINE_PARMS* p)
{
    // Initializing variables by constants.
    char msg[APPCONST::MAX_TEXT_STRING];
    const char* nameSrcPath = APPCONST::DEFAULT_PATH_SRC;
    const char* nameDstPath = APPCONST::DEFAULT_PATH_DST;
    const char* nameSrc = APPCONST::DEFAULT_NAME_SRC;
    const char* nameDst = APPCONST::DEFAULT_NAME_DST;
    const char* nameExt = APPCONST::DEFAULT_EXTENSION;
    DWORD32 fileCount = APPCONST::DEFAULT_FILE_COUNT;
    DWORD32 fileSize = APPCONST::DEFAULT_FILE_SIZE;
    DWORD32 blockSize = APPCONST::DEFAULT_BLOCK_SIZE;
    DWORD32 repeats = APPCONST::DEFAULT_REPEATS;
    D_TYPE dataType = APPCONST::DEFAULT_DATA_TYPE;
    A_TYPE addressType = APPCONST::DEFAULT_ADDRESS_TYPE;
    BOOL flagNoBuffering = APPCONST::DEFAULT_FLAG_NO_BUFFERING;
    BOOL flagWriteThrough = APPCONST::DEFAULT_FLAG_WRITE_THROUGH;
    BOOL flagSequentalScan = APPCONST::DEFAULT_FLAG_SEQUENTIAL_SCAN;
    DWORD32 msWaitRead = APPCONST::DEFAULT_WAIT_READ;
    DWORD32 msWaitWrite = APPCONST::DEFAULT_WAIT_WRITE;
    DWORD32 msWaitCopy = APPCONST::DEFAULT_WAIT_COPY;
    DWORD32 minCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 maxCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 minDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    DWORD32 maxDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    // If input parameters structure passed, load parameters from this structure.
    if (p)
    {
        nameSrcPath = p->optionSrcPath;
        nameDstPath = p->optionDstPath;
        fileCount = p->optionFileCount;
        fileSize = static_cast<DWORD32>(p->optionFileSize);
        blockSize = static_cast<DWORD32>(p->optionBlockSize);
        repeats = p->optionRepeats;
        dataType = static_cast<D_TYPE>(p->optionData);
        addressType = static_cast<A_TYPE>(p->optionAddress);
        flagNoBuffering = p->optionNoBuf;
        flagWriteThrough = p->optionWriteThr;
        flagSequentalScan = p->optionSequental;
        msWaitRead = p->optionWaitRead;
        msWaitWrite = p->optionWaitWrite;
        msWaitCopy = p->optionWaitCopy;
        minCpu = p->optionMinCpu;
        maxCpu = p->optionMaxCpu;
        minDomain = p->optionMinDomain;
        maxDomain = p->optionMaxDomain;
    }
    // Initializing variables for read, write and copy total sizes verification.
    DWORD64 totalRead = 0;
    DWORD64 totalWrite = 0;
    DWORD64 totalCopyRead = 0;
    DWORD64 totalCopyWrite = 0;
    DWORD64 totalBytes = (DWORD64)fileSize * (DWORD64)fileCount * (DWORD64)repeats;
    // Initializing timer (use OS performance counter).
    LARGE_INTEGER hz;
    BOOL status;
    status = QueryPerformanceFrequency(&hz);
    if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
    write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    write("\r\nMemory allocated, ");
    storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    write(msg);
    write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    // Create source file.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, 0, nameExt);
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;
    if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
    if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
    if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    HANDLE srcHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
    DWORD64 numHandle = (DWORD64)srcHandle;
#else
    DWORD64 numHandle = (DWORD64)((DWORD32)srcHandle);
#endif
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", 0, numHandle, path);
    write(msg);
    if ((srcHandle == NULL) || (srcHandle == INVALID_HANDLE_VALUE))
    {
        createError = TRUE;
    }
    // Create destination file.
    HANDLE dstHandle = NULL;
    if (!createError)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, 0, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        dstHandle = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)dstHandle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)dstHandle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", 0, numHandle, path);
        write(msg);
        if ((dstHandle == NULL) || (dstHandle == INVALID_HANDLE_VALUE))
        {
            createError = TRUE;
        }
    }
    // Both source and destination files created (yet zero size).
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(FILE_CREATE_FAILED, fileData, previousAffinity);
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // Initializing for write source, copy source to destination and read source files.
    std::vector<double> writeSpeed;
    std::vector<double> copySpeed;
    std::vector<double> readSpeed;
    double speedIntegral = 0.0;
    // Table up for WRITE, COPY, READ operations.
    writeColor("\r\n Write, Copy, Read file performance.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index      MBPS,  Write            Copy             Read\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL writeError = FALSE;
    BOOL copyError = FALSE;
    BOOL readError = FALSE;
    // Start timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Start cycle for write, copy, read files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        // WRITE operation begin, start timer for one source file write.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandle;
            DWORD32 writeSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesWritten = 0;
            LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
            // Seek (restore file pointer to 0) and write one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                writeError = TRUE;
                break;
            }
            while (writeSize)
            {
                DWORD nNumberOfBytesToWrite = blockSize;
                if (writeSize >= blockSize)
                {
                    writeSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToWrite = writeSize;
                    writeSize = 0;
                }
                while (nNumberOfBytesToWrite > 0)
                {
                    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                    {
                        writeError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                    {
                        writeError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesWritten;
                    nNumberOfBytesToWrite -= nNumberOfBytesWritten;
                    totalWrite += nNumberOfBytesWritten;
                }
                if (writeError) break;
            }
            if (writeError) break;
        }  // End cycle for measurement repeats.
        // Stop timer for one source file write.
        QueryPerformanceCounter(&t4);
        if (writeError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        writeSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-7d%17.3f", i, mbps);
        write(msg);
        // COPY operation begin, start timer for copy one source file to one destination file.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandle;
            DWORD32 readSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesRead = 0;
            LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
            // Seek (restore file pointer to 0) and read one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                copyError = TRUE;
                break;
            }
            // Read one source file.
            while (readSize)
            {
                DWORD nNumberOfBytesToRead = blockSize;
                if (readSize >= blockSize)
                {
                    readSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToRead = readSize;
                    readSize = 0;
                }
                while (nNumberOfBytesToRead > 0)
                {
                    if (!ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr))
                    {
                        copyError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesRead <= 0) || (nNumberOfBytesRead > nNumberOfBytesToRead))
                    {
                        copyError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesRead;
                    nNumberOfBytesToRead -= nNumberOfBytesRead;
                    totalCopyRead += nNumberOfBytesRead;
                }
                if (copyError) break;
            }
            if (copyError) break;
            // Variables for write one destination file.
            hFile = dstHandle;
            DWORD32 writeSize = fileSize;
            lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesWritten = 0;
            LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
            // Seek (restore file pointer to 0) and read one source file.
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                copyError = TRUE;
                break;
            }
            // Write one destination file.
            while (writeSize)
            {
                DWORD nNumberOfBytesToWrite = blockSize;
                if (writeSize >= blockSize)
                {
                    writeSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToWrite = writeSize;
                    writeSize = 0;
                }
                while (nNumberOfBytesToWrite > 0)
                {
                    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                    {
                        copyError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                    {
                        copyError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesWritten;
                    nNumberOfBytesToWrite -= nNumberOfBytesWritten;
                    totalCopyWrite += nNumberOfBytesWritten;
                }
                if (copyError) break;
            }
        }  // End cycle for measurement repeats.
        // Stop timer for one source file write to destination file.
        QueryPerformanceCounter(&t4);
        // End of copy one file (read one source file and write one destination file).
        if (copyError) break;
        seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        mbps = fileMegabytes / seconds;
        copySpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "%17.3f", mbps);
        write(msg);
        // READ operation begin, start timer for one source file read.
        QueryPerformanceCounter(&t3);
        // Start cycle for measurement repeats.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE hFile = srcHandle;
            DWORD32 readSize = fileSize;
            BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
            DWORD nNumberOfBytesRead = 0;
            LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
            if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
            {
                readError = TRUE;
                break;
            }
            while (readSize)
            {
                DWORD nNumberOfBytesToRead = blockSize;
                if (readSize >= blockSize)
                {
                    readSize -= blockSize;
                }
                else
                {
                    nNumberOfBytesToRead = readSize;
                    readSize = 0;
                }

                while (nNumberOfBytesToRead > 0)
                {
                    if (!ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr))
                    {
                        readError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesRead <= 0) || (nNumberOfBytesRead > nNumberOfBytesToRead))
                    {
                        readError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesRead;
                    nNumberOfBytesToRead -= nNumberOfBytesRead;
                    totalRead += nNumberOfBytesRead;
                }
                if (readError) break;
            }
        }  // End cycle for measurement repeats.
        // Stop timer for one source file read.
        QueryPerformanceCounter(&t4);
        if (readError) break;
        seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        mbps = fileMegabytes / seconds;
        readSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "%17.3f\r\n", mbps);
        write(msg);
    }
    // Stop timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral%16.3f%17.3f%17.3f\r\n", mbps, mbps, mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError || copyError || readError)
    {
        if (writeError)
            return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
        else if (copyError)
            return closeContext(FILE_COPY_FAILED, fileData, previousAffinity);
        else
            return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // Close and delete files.
    writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, 0, nameExt);
    if (!CloseHandle(srcHandle)) deleteError = TRUE;
    if (!DeleteFile(path)) deleteError = TRUE;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", 0, path);
    write(msg);
    // Close and delete destination files.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, 0, nameExt);
    if (!CloseHandle(dstHandle)) deleteError = TRUE;
    if (!DeleteFile(path)) deleteError = TRUE;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", 0, path);
    write(msg);
    // Delete source and destination files done.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    if ((totalBytes > 0) &&
        (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    {   // Write performance measurement statistics, if total size verification passed.
        writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readSpeed, true);
        writeStatistics(msg, "\r\n WRITE ", writeSpeed, true);
        writeStatistics(msg, "\r\n COPY  ", copySpeed, true);
        write("\r\n");
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int runTaskIOPS(COMMAND_LINE_PARMS* p)
{
    // Initializing variables by constants.
    char msg[APPCONST::MAX_TEXT_STRING];
    const char* nameSrcPath = APPCONST::DEFAULT_PATH_SRC;
    const char* nameDstPath = APPCONST::DEFAULT_PATH_DST;
    const char* nameSrc = APPCONST::DEFAULT_NAME_SRC;
    const char* nameDst = APPCONST::DEFAULT_NAME_DST;
    const char* nameExt = APPCONST::DEFAULT_EXTENSION;
    DWORD32 fileCount = APPCONST::DEFAULT_FILE_COUNT;
    DWORD32 fileSize = APPCONST::DEFAULT_FILE_SIZE;
    DWORD32 blockSize = APPCONST::DEFAULT_BLOCK_SIZE;
    DWORD32 serviceBlockSize = APPCONST::DEFAULT_BLOCK_SIZE;  // This required for fill block before IOPS test.
    DWORD32 repeats = APPCONST::DEFAULT_REPEATS;
    D_TYPE dataType = APPCONST::DEFAULT_DATA_TYPE;
    A_TYPE addressType = APPCONST::DEFAULT_ADDRESS_TYPE;
    BOOL flagNoBuffering = APPCONST::DEFAULT_FLAG_NO_BUFFERING;
    BOOL flagWriteThrough = APPCONST::DEFAULT_FLAG_WRITE_THROUGH;
    BOOL flagSequentalScan = APPCONST::DEFAULT_FLAG_SEQUENTIAL_SCAN;
    DWORD32 msWaitRead = APPCONST::DEFAULT_WAIT_READ;
    DWORD32 msWaitWrite = APPCONST::DEFAULT_WAIT_WRITE;
    DWORD32 msWaitCopy = APPCONST::DEFAULT_WAIT_COPY;
    DWORD32 minCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 maxCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 minDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    DWORD32 maxDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    // If input parameters structure passed, load parameters from this structure.
    if (p)
    {
        nameSrcPath = p->optionSrcPath;
        nameDstPath = p->optionDstPath;
        fileCount = p->optionFileCount;
        fileSize = static_cast<DWORD32>(p->optionFileSize);
        blockSize = static_cast<DWORD32>(p->optionBlockSize);
        repeats = p->optionRepeats;
        dataType = static_cast<D_TYPE>(p->optionData);
        addressType = static_cast<A_TYPE>(p->optionAddress);
        flagNoBuffering = p->optionNoBuf;
        flagWriteThrough = p->optionWriteThr;
        flagSequentalScan = p->optionSequental;
        msWaitRead = p->optionWaitRead;
        msWaitWrite = p->optionWaitWrite;
        msWaitCopy = p->optionWaitCopy;
        minCpu = p->optionMinCpu;
        maxCpu = p->optionMaxCpu;
        minDomain = p->optionMinDomain;
        maxDomain = p->optionMaxDomain;
    }
    // Initializing variables for read, write and copy total sizes verification.
    DWORD64 totalRead = 0;
    DWORD64 totalWrite = 0;
    DWORD64 totalCopyRead = 0;
    DWORD64 totalCopyWrite = 0;
    DWORD64 totalBytes = (DWORD64)fileSize * (DWORD64)fileCount * (DWORD64)repeats * 2;  // *2 because src and dst for IOPS scenario.
    // Initializing timer (use OS performance counter).
    LARGE_INTEGER hz;
    BOOL status;
    status = QueryPerformanceFrequency(&hz);
    if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
    write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    write("\r\nBuffer memory allocated, ");
    storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    write(msg);
    write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    // Create source files.
    std::vector<HANDLE> srcHandles;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)handle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        write(msg);
        if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(handle);
        }
        else
        {
            createError = TRUE;
            break;
        }
    }
    // Create destination files.
    std::vector<HANDLE> dstHandles;
    if (!createError)
    {
        for (unsigned int i = 0; i < fileCount; i++)
        {
            snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
            DWORD attributes = FILE_ATTRIBUTE_NORMAL;
            if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
            if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
            if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
            HANDLE handle = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
            DWORD64 numHandle = (DWORD64)handle;
#else
            DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            write(msg);
            if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
            {
                dstHandles.push_back(handle);
            }
            else
            {
                createError = TRUE;
                break;
            }
        }
    }
    // Both source and destination files created (yet zero size).
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(FILE_CREATE_FAILED, fileData, previousAffinity);
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // Initializing for service write source files.
    std::vector<double> writeSpeed;
    double writeSpeedIntegral = 0.0;
    // Table up for SERVICE WRITE operation.
    writeColor("\r\n Service write files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL writeError = FALSE;
    // Start timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Start cycle for service write source files.
    for (unsigned int i = 0; i < (fileCount * 2); i++)
    {
        // Start timer for one file service write.
        QueryPerformanceCounter(&t3);
        HANDLE hFile = nullptr;
        int j = 0;
        if (i < fileCount)
        {
            j = i;
            hFile = srcHandles[j];
        }
        else
        {
            j = i - fileCount;
            hFile = dstHandles[j];
        }
        DWORD32 writeSize = fileSize;
        BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
        DWORD nNumberOfBytesWritten = 0;
        LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
        while (writeSize)
        {
            DWORD nNumberOfBytesToWrite = serviceBlockSize;
            if (writeSize >= serviceBlockSize)
            {
                writeSize -= serviceBlockSize;
            }
            else
            {
                nNumberOfBytesToWrite = writeSize;
                writeSize = 0;
            }
            while (nNumberOfBytesToWrite > 0)
            {
                if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                {
                    writeError = TRUE;
                    break;
                }
                if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                {
                    writeError = TRUE;
                    break;
                }
                lpBuffer += nNumberOfBytesWritten;
                nNumberOfBytesToWrite -= nNumberOfBytesWritten;
            }
            if (writeError) break;
        }
        if (writeError) break;
        // Stop timer for one file service write.
        QueryPerformanceCounter(&t4);
        if (!CloseHandle(hFile)) writeError = TRUE;
        if (writeError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        writeSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, mbps);
        write(msg);
    }
    // Stop timer for integral time of service write files.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes * 2 / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Re-open source and destination files.
    writeColor("\r\n Re-open files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL openError = FALSE;
    for (unsigned int i = 0; i < (fileCount * 2); i++)
    {
        const char* namePath = nullptr;
        const char* nameName = nullptr;
        int j = 0;
        if (i < fileCount)
        {
            j = i;
            namePath = nameSrcPath;
            nameName = nameSrc;
        }
        else
        {
            j = i - fileCount;
            namePath = nameDstPath;
            nameName = nameDst;
        }
        snprintf(path, MAX_PATH, "%s%s%08X%s", namePath, nameName, j, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)handle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", j, numHandle, path);
        write(msg);
        if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
        {
            if (i < fileCount)
            {
                srcHandles[j] = handle;
            }
            else
            {
                dstHandles[j] = handle;
            }
        }
        else
        {
            openError = TRUE;
            break;
        }
    }
    // Both source and destination files re-opened and has valid sizes.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (openError)
    {
        return closeContext(FILE_OPEN_FAILED, fileData, previousAffinity);
    }
    // Initializing I/O list for random access files and blocks.
    std::vector<IO_DESCRIPTOR> list;
    unsigned int blockCount = fileSize / blockSize;
    buildAddress(msg, hz, dataType, list, blockCount, fileCount);
    // IOPS measurement for READ, both source and destination files zones (read any).
    std::vector<double> readIops;
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    writeColor("\r\n Read IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL readError = FALSE;
    // Start timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    unsigned int index = 0;
    for (unsigned int i = 0; i < 2; i++)                     // Cycle for src and dst files groups.
    {
        for (unsigned int j = 0; j < fileCount; j++)         // Cycle for files.
        {
            // Start timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t3);
            for (unsigned int k = 0; k < blockCount; k++)    // Cycle for blocks in the file.
            {
                IO_DESCRIPTOR d = list[index];
                HANDLE hFile = NULL;
                if (d.flags)
                {
                    hFile = dstHandles[j];
                }
                else
                {
                    hFile = srcHandles[j];
                }
                LONG offset = d.blockIndex * blockSize;
                BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData) + offset;
                DWORD nNumberOfBytesRead = 0;
                LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
                if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset)
                {
                    readError = TRUE;
                    break;
                }
                DWORD nNumberOfBytesToRead = blockSize;
                while (nNumberOfBytesToRead > 0)
                {
                    if (!ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr))
                    {
                        readError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesRead <= 0) || (nNumberOfBytesRead > nNumberOfBytesToRead))
                    {
                        readError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesRead;
                    nNumberOfBytesToRead -= nNumberOfBytesRead;
                    totalRead += nNumberOfBytesRead;
                }
                index++;
                if (readError) break;
            }                           // End of cycle for blocks in the file.
            // Stop timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t4);
            if (readError) break;
            double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
            double iops = blockCount / seconds;
            readIops.push_back(iops);
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, iops);
            write(msg);
        }                               // End of cycle for files.
        if (readError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", iops);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // IOPS measurement for WRITE, both source and destination files zones (read any).
    std::vector<double> writeIops;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    writeColor("\r\n Write IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeError = FALSE;
    // Start timer for integral time of write IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    index = 0;
    for (unsigned int i = 0; i < 2; i++)                     // Cycle for src and dst files groups.
    {
        for (unsigned int j = 0; j < fileCount; j++)         // Cycle for files.
        {
            // Start timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t3);
            for (unsigned int k = 0; k < blockCount; k++)    // Cycle for blocks in the file.
            {
                IO_DESCRIPTOR d = list[index];
                HANDLE hFile = NULL;
                if (d.flags)
                {
                    hFile = dstHandles[j];
                }
                else
                {
                    hFile = srcHandles[j];
                }
                LONG offset = d.blockIndex * blockSize;
                BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData) + offset;
                DWORD nNumberOfBytesWritten = 0;
                LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
                if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset)
                {
                    writeError = TRUE;
                    break;
                }
                DWORD nNumberOfBytesToWrite = blockSize;
                while (nNumberOfBytesToWrite > 0)
                {
                    if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                    {
                        writeError = TRUE;
                        break;
                    }
                    if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                    {
                        writeError = TRUE;
                        break;
                    }
                    lpBuffer += nNumberOfBytesWritten;
                    nNumberOfBytesToWrite -= nNumberOfBytesWritten;
                    totalWrite += nNumberOfBytesWritten;
                }
                index++;
                if (writeError) break;
            }                           // End of cycle for blocks in the file.
            // Stop timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t4);
            if (writeError) break;
            double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
            double iops = blockCount / seconds;
            writeIops.push_back(iops);
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, iops);
            write(msg);
        }                               // End of cycle for files.
        if (writeError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", iops);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // RESERVED: IOPS measurement for copy, from source to destination files zones (read src, write dst).
    // RESERVED: IOPS measurement for copy, both source and destination files zones (read any, write any).
    // Close and delete files.
    writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Delete source and destination files done.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
//  if ((totalBytes > 0) &&
//      (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    if ((totalBytes > 0) && (totalRead == totalBytes) && (totalWrite == totalBytes))
        // Yet only READ and WRITE for IOPS test.
    {   // Write performance measurement statistics, if total size verification passed.
        writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readIops, true);
        writeStatistics(msg, "\r\n WRITE ", writeIops, true);
        //      writeStatistics(msg, "\r\n COPY  ", copyIops, true);
        write("\r\n");
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int runTaskIOPSqueued(COMMAND_LINE_PARMS* p)
{
    // Initializing variables by constants.
    char msg[APPCONST::MAX_TEXT_STRING];
    const char* nameSrcPath = APPCONST::DEFAULT_PATH_SRC;
    const char* nameDstPath = APPCONST::DEFAULT_PATH_DST;
    const char* nameSrc = APPCONST::DEFAULT_NAME_SRC;
    const char* nameDst = APPCONST::DEFAULT_NAME_DST;
    const char* nameExt = APPCONST::DEFAULT_EXTENSION;
    DWORD32 fileCount = APPCONST::DEFAULT_FILE_COUNT;
    DWORD32 fileSize = APPCONST::DEFAULT_FILE_SIZE;
    DWORD32 blockSize = APPCONST::DEFAULT_BLOCK_SIZE;
    DWORD32 serviceBlockSize = APPCONST::DEFAULT_BLOCK_SIZE;  // This required for fill block before IOPS test.
    DWORD32 repeats = APPCONST::DEFAULT_REPEATS;
    D_TYPE dataType = APPCONST::DEFAULT_DATA_TYPE;
    A_TYPE addressType = APPCONST::DEFAULT_ADDRESS_TYPE;
    BOOL flagNoBuffering = APPCONST::DEFAULT_FLAG_NO_BUFFERING;
    BOOL flagWriteThrough = APPCONST::DEFAULT_FLAG_WRITE_THROUGH;
    BOOL flagSequentalScan = APPCONST::DEFAULT_FLAG_SEQUENTIAL_SCAN;
    DWORD32 msWaitRead = APPCONST::DEFAULT_WAIT_READ;
    DWORD32 msWaitWrite = APPCONST::DEFAULT_WAIT_WRITE;
    DWORD32 msWaitCopy = APPCONST::DEFAULT_WAIT_COPY;
    DWORD32 minCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 maxCpu = APPCONST::DEFAULT_CPU_SELECT;
    DWORD32 minDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    DWORD32 maxDomain = APPCONST::DEFAULT_DOMAIN_SELECT;
    DWORD32 queueDepth = APPCONST::DEFAULT_QUEUE_DEPTH;
    // If input parameters structure passed, load parameters from this structure.
    if (p)
    {
        nameSrcPath = p->optionSrcPath;
        nameDstPath = p->optionDstPath;
        fileCount = p->optionFileCount;
        fileSize = static_cast<DWORD32>(p->optionFileSize);
        blockSize = static_cast<DWORD32>(p->optionBlockSize);
        repeats = p->optionRepeats;
        dataType = static_cast<D_TYPE>(p->optionData);
        addressType = static_cast<A_TYPE>(p->optionAddress);
        flagNoBuffering = p->optionNoBuf;
        flagWriteThrough = p->optionWriteThr;
        flagSequentalScan = p->optionSequental;
        msWaitRead = p->optionWaitRead;
        msWaitWrite = p->optionWaitWrite;
        msWaitCopy = p->optionWaitCopy;
        queueDepth = p->optionQueue;
        minCpu = p->optionMinCpu;
        maxCpu = p->optionMaxCpu;
        minDomain = p->optionMinDomain;
        maxDomain = p->optionMaxDomain;
    }
    // Initializing variables for read, write and copy total sizes verification.
    DWORD64 totalRead = 0;
    DWORD64 totalWrite = 0;
    DWORD64 totalCopyRead = 0;
    DWORD64 totalCopyWrite = 0;
    DWORD64 totalBytes = (DWORD64)fileSize * (DWORD64)fileCount * (DWORD64)repeats * 2;  // *2 because src and dst for IOPS scenario.
    // Initializing timer (use OS performance counter).
    LARGE_INTEGER hz;
    BOOL status;
    status = QueryPerformanceFrequency(&hz);
    if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
    write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    write("\r\nBuffer memory allocated, ");
    storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    write(msg);
    write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create events handles for asynchronous I/O.
    std::vector<HANDLE> eventHandles;
    std::vector<OVERLAPPED> overlaps;
    std::vector<DWORD> returns;
    createEvents(msg, eventHandles, overlaps, returns, queueDepth);
    // Create source and destination files.
    writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    // Create source files.
    std::vector<HANDLE> srcHandles;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)handle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        write(msg);
        if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(handle);
        }
        else
        {
            createError = TRUE;
            break;
        }
    }
    // Create destination files.
    std::vector<HANDLE> dstHandles;
    if (!createError)
    {
        for (unsigned int i = 0; i < fileCount; i++)
        {
            snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
            DWORD attributes = FILE_ATTRIBUTE_NORMAL;
            if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
            if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
            if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
            HANDLE handle = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
            DWORD64 numHandle = (DWORD64)handle;
#else
            DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            write(msg);
            if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
            {
                dstHandles.push_back(handle);
            }
            else
            {
                createError = TRUE;
                break;
            }
        }
    }
    // Both source and destination files created (yet zero size).
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_CREATE_FAILED, fileData, previousAffinity);
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // Initializing for service write source files.
    std::vector<double> writeSpeed;
    double writeSpeedIntegral = 0.0;
    // Table up for SERVICE WRITE operation.
    writeColor("\r\n Service write files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL writeError = FALSE;
    // Start timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    // Start cycle for write source files.
    for (unsigned int i = 0; i < (fileCount * 2); i++)
    {
        // Start timer for one file service write.
        QueryPerformanceCounter(&t3);
        HANDLE hFile = nullptr;
        int j = 0;
        if (i < fileCount)
        {
            j = i;
            hFile = srcHandles[j];
        }
        else
        {
            j = i - fileCount;
            hFile = dstHandles[j];
        }
        DWORD32 writeSize = fileSize;
        BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
        DWORD nNumberOfBytesWritten = 0;
        LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
        while (writeSize)
        {
            DWORD nNumberOfBytesToWrite = serviceBlockSize;
            if (writeSize >= serviceBlockSize)
            {
                writeSize -= serviceBlockSize;
            }
            else
            {
                nNumberOfBytesToWrite = writeSize;
                writeSize = 0;
            }
            while (nNumberOfBytesToWrite > 0)
            {
                if (!WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr))
                {
                    writeError = TRUE;
                    break;
                }
                if ((nNumberOfBytesWritten <= 0) || (nNumberOfBytesWritten > nNumberOfBytesToWrite))
                {
                    writeError = TRUE;
                    break;
                }
                lpBuffer += nNumberOfBytesWritten;
                nNumberOfBytesToWrite -= nNumberOfBytesWritten;
            }
            if (writeError) break;
        }
        if (writeError) break;
        // Stop timer for one file service write.
        QueryPerformanceCounter(&t4);
        if (!CloseHandle(hFile)) writeError = TRUE;
        if (writeError) break;
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        writeSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, mbps);
        write(msg);
    }
    // Stop timer for integral time of service write files.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes * 2 / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Re-open source and destination files.
    writeColor("\r\n Re-open files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL openError = FALSE;
    for (unsigned int i = 0; i < (fileCount * 2); i++)
    {
        const char* namePath = nullptr;
        const char* nameName = nullptr;
        int j = 0;
        if (i < fileCount)
        {
            j = i;
            namePath = nameSrcPath;
            nameName = nameSrc;
        }
        else
        {
            j = i - fileCount;
            namePath = nameDstPath;
            nameName = nameDst;
        }
        snprintf(path, MAX_PATH, "%s%s%08X%s", namePath, nameName, j, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;    // Changed for overlapped I/O (asynchronous with queue).
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE handle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, attributes, nullptr);
#ifdef NATIVE_WIDTH_64
        DWORD64 numHandle = (DWORD64)handle;
#else
        DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", j, numHandle, path);
        write(msg);
        if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE))
        {
            if (i < fileCount)
            {
                srcHandles[j] = handle;
            }
            else
            {
                dstHandles[j] = handle;
            }
        }
        else
        {
            openError = TRUE;
            break;
        }
    }
    // Both source and destination files re-opened and has valid sizes.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (openError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_OPEN_FAILED, fileData, previousAffinity);
    }
    // Initializing I/O list for random access files and blocks.
    std::vector<IO_DESCRIPTOR> list;
    unsigned int blockCount = fileSize / blockSize;
    buildAddress(msg, hz, dataType, list, blockCount, fileCount);
    // IOPS measurement for READ, both source and destination files zones (read any).
    std::vector<double> readIops;
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    writeColor("\r\n Read IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL readError = FALSE;
    // Start timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    unsigned int index = 0;
    for (unsigned int i = 0; i < 2; i++)                                 // Cycle for src and dst files groups.
    {
        for (unsigned int j = 0; j < fileCount; j++)                     // Cycle for files.
        {
            // Start timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t3);
            for (unsigned int k = 0; k < blockCount; k += queueDepth)    // Cycle for blocks in the file.
            {
                unsigned int iopool = blockCount - k;
                if (iopool > queueDepth)
                {
                    iopool = queueDepth;
                }
                for (unsigned int m = 0; m < iopool; m++)                // Cycle for requests in the single depth.
                {
                    IO_DESCRIPTOR d = list[index];
                    HANDLE hFile = NULL;
                    if (d.flags)
                    {
                        hFile = dstHandles[j];
                    }
                    else
                    {
                        hFile = srcHandles[j];
                    }
                    LONG offset = d.blockIndex * blockSize;
                    BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData) + offset;
                    //if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset)
                    //{
                    //    readError = TRUE;
                    //    break;
                    //}
                    overlaps[m].Offset = offset;  // File seek by this parameter.
                    if (!ReadFile(hFile, lpBuffer, blockSize, &returns[m], &overlaps[m]))
                    {
                        //readError = TRUE;      // Status ignored because asynchronous termination.
                        //break;
                    }
                    index++;
                    if (readError) break;
                }                               // End of cycle for requests in the single depth.
                if (readError) break;
                DWORD waitStatus = WaitForMultipleObjects(iopool, &eventHandles[0], TRUE, APPCONST::WAIT_LIMIT);
                if ((waitStatus >= 0) && (waitStatus < iopool))
                {
                    totalRead += (iopool * blockSize);
                }
                else
                {
                    readError = TRUE;
                    break;
                }
                if (readError) break;
            }                                   // End of cycle for blocks in the file.
            // Stop timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t4);
            if (readError) break;
            double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
            double iops = blockCount / seconds;
            readIops.push_back(iops);
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, iops);
            write(msg);
        }                               // End of cycle for files.
        if (readError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", iops);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // IOPS measurement for WRITE, both source and destination files zones (read any).
    std::vector<double> writeIops;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    writeColor("\r\n Write IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeError = FALSE;
    // Start timer for integral time of write IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            return closeContext(TIMER_FAILED, fileData, previousAffinity);
        }
    }
    index = 0;
    for (unsigned int i = 0; i < 2; i++)                                 // Cycle for src and dst files groups.
    {
        for (unsigned int j = 0; j < fileCount; j++)                     // Cycle for files.
        {
            // Start timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t3);
            for (unsigned int k = 0; k < blockCount; k += queueDepth)    // Cycle for blocks in the file.
            {
                unsigned int iopool = blockCount - k;
                if (iopool > queueDepth)
                {
                    iopool = queueDepth;
                }
                for (unsigned int m = 0; m < iopool; m++)                // Cycle for requests in the single depth.
                {
                    IO_DESCRIPTOR d = list[index];
                    HANDLE hFile = NULL;
                    if (d.flags)
                    {
                        hFile = dstHandles[j];
                    }
                    else
                    {
                        hFile = srcHandles[j];
                    }
                    LONG offset = d.blockIndex * blockSize;
                    BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData) + offset;
                    //if (SetFilePointer(hFile, offset, nullptr, FILE_BEGIN) != offset)
                    //{
                    //    writeError = TRUE;
                    //    break;
                    //}
                    overlaps[m].Offset = offset;  // File seek by this parameter.
                    if (!WriteFile(hFile, lpBuffer, blockSize, &returns[m], &overlaps[m]))
                    {
                        //writeError = TRUE;      // Status ignored because asynchronous termination.
                        //break;
                    }
                    index++;
                    if (writeError) break;
                }                               // End of cycle for requests in the single depth.
                if (writeError) break;
                DWORD waitStatus = WaitForMultipleObjects(iopool, &eventHandles[0], TRUE, APPCONST::WAIT_LIMIT);
                if ((waitStatus >= 0) && (waitStatus < iopool))
                {
                    totalWrite += (iopool * blockSize);
                }
                else
                {
                    writeError = TRUE;
                    break;
                }
                if (writeError) break;
            }                                   // End of cycle for blocks in the file.
            // Stop timer for one unit for IOPS measurement, unit same as file size.
            QueryPerformanceCounter(&t4);
            if (writeError) break;
            double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
            double iops = blockCount / seconds;
            writeIops.push_back(iops);
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", j, iops);
            write(msg);
        }                               // End of cycle for files.
        if (writeError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of write IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", iops);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    // RESERVED: IOPS measurement for copy, from source to destination files zones (read src, write dst).
    // RESERVED: IOPS measurement for copy, both source and destination files zones (read any, write any).
    // Close and delete files.
    writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        write(msg);
    }
    // Delete source and destination files done.
    writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
//  if ((totalBytes > 0) &&
//      (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    if ((totalBytes > 0) && (totalRead == totalBytes) && (totalWrite == totalBytes))
        // Yet only READ and WRITE for IOPS test.
    {   // Write performance measurement statistics, if total size verification passed.
        writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readIops, true);
        writeStatistics(msg, "\r\n WRITE ", writeIops, true);
        //      writeStatistics(msg, "\r\n COPY  ", copyIops, true);
        write("\r\n");
        writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        writeColor(msg, APPCONST::TABLE_COLOR);
        closeEvents(eventHandles);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}