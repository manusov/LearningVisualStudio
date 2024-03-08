/*

Simple benchmark for mass storage performance.
Use Read and Write operations.
Measure IOPS = IO operations per Second.

Series of simple examples for Microsoft Visual Studio:
https://github.com/manusov/LearningVisualStudio
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <intrin.h>
#include <immintrin.h>

using namespace std;

#if _WIN64
#define NATIVE_WIDTH_64
#define ARCH_NAME "x64"
#elif _WIN32
#define ARCH_NAME "ia32"
#define NATIVE_WIDTH_32
#endif

// Test options enumerations.
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
// I/O request descriptor.
typedef struct {
    DWORD32 random;      // Random data for make random order after sorting.
    DWORD32 blockIndex;  // Index of block inside one file.
    DWORD32 fileIndex;   // Index of file.
    DWORD32 flags;       // D0 encode 0=src, 1=dst. Other bits reserved.
} IO_DESCRIPTOR;
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
// Status codes.
const char* const STATUS_NAMES[]
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
// Text string size limit.
constexpr int MAX_TEXT_STRING = MAX_PATH;
// Constants used for mass storage benchmark.
const char* const DEFAULT_PATH_SRC = "F:\\Backup\\";        // "" // for current directory.
const char* const DEFAULT_PATH_DST = "F:\\Exchange\\";      // "" // for current directory.
const char* const DEFAULT_NAME_SRC = "src_";
const char* const DEFAULT_NAME_DST = "dst_";
const char* const DEFAULT_EXTENSION = ".bin";
constexpr DWORD32 DEFAULT_FILE_COUNT = 20;
constexpr DWORD32 DEFAULT_FILE_SIZE = 64 * 1024 * 1024;     // 32 * 1024 * 1024;  // 1024 * 1024 * 1024;
constexpr DWORD32 DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024;
constexpr DWORD32 SERVICE_BLOCK_SIZE = 16 * 1024 * 1024;
constexpr DWORD32 DEFAULT_REPEATS = 1;
constexpr D_TYPE DEFAULT_DATA_TYPE = ALL_ZEROES;
constexpr BOOL DEFAULT_FLAG_NO_BUFFERING = TRUE;
constexpr BOOL DEFAULT_FLAG_WRITE_THROUGH = TRUE;
constexpr BOOL DEFAULT_FLAG_SEQUENTIAL_SCAN = FALSE;
constexpr DWORD32 DEFAULT_WAIT_READ = 20000;
constexpr DWORD32 DEFAULT_WAIT_WRITE = 10000;
constexpr DWORD32 DEFAULT_WAIT_COPY = 0;
constexpr DWORD32 DEFAULT_QUEUE_DEPTH = 16;
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

// Helpers functions declaration.
// Show return status
void showReturn(char* msg, STATUS_CODES opStatus);                               // Show return type before return: NO_ERRORS or error name.
// Write strings to buffer.
size_t storeHex8(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex16(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex32(char* buffer, size_t limit, DWORD32 value, bool h);
size_t storeHex64(char* buffer, size_t limit, DWORD64 value, bool h);
size_t storeHexPointer(char* buffer, size_t limit, LPVOID ptr, bool h);
void storeSystemErrorName(char* buffer, size_t limit, DWORD errorCode);          // Decode Windows error code to error description string.
void storeBaseAndSize(char* buffer, size_t limit, DWORD64 blockBase, DWORD64 blockSize);  // Store base address and size for memory block.
// Calculate statistics for results vector: min, max, average, median.
void calculateStatistics(std::vector<double>& data, double& min, double& max, double& average, double& median);
// Support application context.
STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity);
STATUS_CODES closeContext(char* msg, STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity);
// Operations for support disk benchmarks.
DWORD64 getHandle64(HANDLE handle);
void writeStatistics(char* msg, const char* statisticsName, std::vector<double>& speeds, bool tableMode);
void waitTime(char* msg, DWORD milliseconds, const char* operationName);
void buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize);
void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR>& list, unsigned int blocksPerFile, unsigned int fileCount);
bool ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2);
BOOL createEvents(char* msg, std::vector<HANDLE>& events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count);
BOOL closeEvents(std::vector<HANDLE>& events);

// Application entry point.
int main(int argc, char** argv)
{
    cout << "Example with disk drive Read/Write performance measurement (" << ARCH_NAME << ")." << endl;
    // Initializing variables by constants.
    char msg[MAX_TEXT_STRING];
    const char* nameSrcPath = DEFAULT_PATH_SRC;
    const char* nameDstPath = DEFAULT_PATH_DST;
    const char* nameSrc = DEFAULT_NAME_SRC;
    const char* nameDst = DEFAULT_NAME_DST;
    const char* nameExt = DEFAULT_EXTENSION;
    DWORD32 fileCount = DEFAULT_FILE_COUNT;
    DWORD32 fileSize = DEFAULT_FILE_SIZE;
    DWORD32 blockSize = DEFAULT_BLOCK_SIZE;
    DWORD32 serviceBlockSize = SERVICE_BLOCK_SIZE;
    DWORD32 repeats = DEFAULT_REPEATS;
    D_TYPE dataType = DEFAULT_DATA_TYPE;
    BOOL flagNoBuffering = DEFAULT_FLAG_NO_BUFFERING;
    BOOL flagWriteThrough = DEFAULT_FLAG_WRITE_THROUGH;
    BOOL flagSequentalScan = DEFAULT_FLAG_SEQUENTIAL_SCAN;
    DWORD32 msWaitRead = DEFAULT_WAIT_READ;
    DWORD32 msWaitWrite = DEFAULT_WAIT_WRITE;
    DWORD32 msWaitCopy = DEFAULT_WAIT_COPY;
    DWORD32 queueDepth = DEFAULT_QUEUE_DEPTH;
    DWORD32 minCpu = DEFAULT_CPU_SELECT;
    DWORD32 maxCpu = DEFAULT_CPU_SELECT;
    DWORD32 minDomain = DEFAULT_DOMAIN_SELECT;
    DWORD32 maxDomain = DEFAULT_DOMAIN_SELECT;
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
    if ((!status) || (hz.QuadPart == 0))
    {
        showReturn(msg, TIMER_FAILED);
        return 1;
    }
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
    cout << msg;
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        closeContext(msg, retStatus, fileData, previousAffinity);
        return 1;
    }
    cout << endl << "Buffer memory allocated, ";
    storeBaseAndSize(msg, MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    cout << msg << "." << endl;
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create events handles for asynchronous I/O.
    std::vector<HANDLE> eventHandles;
    std::vector<OVERLAPPED> overlaps;
    std::vector<DWORD> returns;
    createEvents(msg, eventHandles, overlaps, returns, queueDepth);
    // Create source and destination files.
    cout << endl << endl << " Create files." << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << " Index     Handle (hex)         Path" << endl;
    cout << "-------------------------------------------------------------------" << endl;
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
        DWORD64 numHandle = getHandle64(handle);
        snprintf(msg, MAX_TEXT_STRING, " %-10d%016llX     %s", i, numHandle, path);
        cout << msg << endl;
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
            DWORD64 numHandle = getHandle64(handle);
            snprintf(msg, MAX_TEXT_STRING, " %-10d%016llX     %s", i, numHandle, path);
            cout << msg << endl;
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
    cout << "-------------------------------------------------------------------" << endl;
    if (createError)
    {
        closeEvents(eventHandles);
        closeContext(msg, FILE_CREATE_FAILED, fileData, previousAffinity);
        return 1;
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // Initializing for service write source files.
    std::vector<double> writeSpeed;
    // Table up for SERVICE WRITE operation.
    cout << endl << " Service write files." << endl;
    cout << "------------------------------" << endl;
    cout << " Index     MBPS" << endl;
    cout << "------------------------------" << endl;
    BOOL writeError = FALSE;
    // Start timer for integral time of write source files.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
            return 1;
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
        snprintf(msg, MAX_TEXT_STRING, " %-10d%.3f", j, mbps);
        cout << msg << endl;
    }
    // Stop timer for integral time of service write files.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double mbps = integralMegabytes * 2 / seconds;
        snprintf(msg, MAX_TEXT_STRING, " Integral  %.3f", mbps);
        cout << msg << endl;
    }
    cout << "------------------------------" << endl;
    if (writeError)
    {
        closeEvents(eventHandles);
        closeContext(msg, FILE_WRITE_FAILED, fileData, previousAffinity);
        return 1;
    }
    // Re-open source and destination files.
    cout << endl << " Re-open files." << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << " Index     Handle (hex)         Path" << endl;
    cout << "-------------------------------------------------------------------" << endl;
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
        DWORD64 numHandle = getHandle64(handle);
        snprintf(msg, MAX_TEXT_STRING, " %-10d%016llX     %s", j, numHandle, path);
        cout << msg << endl;
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
    cout << "-------------------------------------------------------------------" << endl;
    if (openError)
    {
        closeEvents(eventHandles);
        closeContext(msg, FILE_OPEN_FAILED, fileData, previousAffinity);
        return 1;
    }
    // Initializing I/O list for random access files and blocks.
    std::vector<IO_DESCRIPTOR> list;
    unsigned int blockCount = fileSize / blockSize;
    buildAddress(msg, hz, dataType, list, blockCount, fileCount);
    // IOPS measurement for READ, both source and destination files zones (read any).
    std::vector<double> readIops;
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    cout << endl << " Read IOPS measurement." << endl;
    cout << "------------------------------" << endl;
    cout << " Index     IOPS" << endl;
    cout << "------------------------------" << endl;
    BOOL readError = FALSE;
    // Start timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
            return 1;
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
                DWORD waitStatus = WaitForMultipleObjects(iopool, &eventHandles[0], TRUE, WAIT_LIMIT);
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
            snprintf(msg, MAX_TEXT_STRING, " %-10d%.3f", j, iops);
            cout << msg << endl;
        }                               // End of cycle for files.
        if (readError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of read IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, MAX_TEXT_STRING, " Integral  %.3f", iops);
        cout << msg << endl;
    }
    cout << "------------------------------" << endl;
    if (readError)
    {
        closeEvents(eventHandles);
        closeContext(msg, FILE_READ_FAILED, fileData, previousAffinity);
        return 1;
    }
    // IOPS measurement for WRITE, both source and destination files zones (read any).
    std::vector<double> writeIops;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    cout << endl << " Write IOPS measurement." << endl;
    cout << "------------------------------" << endl;
    cout << " Index     IOPS" << endl;
    cout << "------------------------------" << endl;
    writeError = FALSE;
    // Start timer for integral time of write IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    t2.QuadPart = t1.QuadPart;
    while (t2.QuadPart == t1.QuadPart)
    {
        if (!QueryPerformanceCounter(&t2))
        {
            closeEvents(eventHandles);
            closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
            return 1;
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
                DWORD waitStatus = WaitForMultipleObjects(iopool, &eventHandles[0], TRUE, WAIT_LIMIT);
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
            snprintf(msg, MAX_TEXT_STRING, " %-10d%.3f", j, iops);
            cout << msg << endl;
        }                               // End of cycle for files.
        if (writeError) break;
    }                                   // End of cycle for src and dst files groups.
    // Stop timer for integral time of write IOPS.
    if (!QueryPerformanceCounter(&t1))
    {
        closeEvents(eventHandles);
        return closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    else
    {
        double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
        double iops = index / seconds;
        snprintf(msg, MAX_TEXT_STRING, " Integral  %.3f", iops);
        cout << msg << endl;
    }
    cout << "------------------------------" << endl;
    if (readError)
    {
        closeEvents(eventHandles);
        closeContext(msg, TIMER_FAILED, fileData, previousAffinity);
        return 1;
    }
    // RESERVED: IOPS measurement for copy, from source to destination files zones (read src, write dst).
    // RESERVED: IOPS measurement for copy, both source and destination files zones (read any, write any).
    // Close and delete files.
    cout << endl << " Delete files." << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << " Index     Handle (hex)         Path" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, MAX_TEXT_STRING, " %-10d%s", i, path);
        cout << msg << endl;
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, MAX_TEXT_STRING, " %-10d%s", i, path);
        cout << msg << endl;
    }
    // Delete source and destination files done.
    cout << "-------------------------------------------------------------------" << endl;
    if (deleteError)
    {
        closeEvents(eventHandles);
        closeContext(msg, FILE_DELETE_FAILED, fileData, previousAffinity);
        return 1;
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    // if ((totalBytes > 0) &&
    //    (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    if ((totalBytes > 0) && (totalRead == totalBytes) && (totalWrite == totalBytes))
        // Yet only READ and WRITE for IOPS test.
    {   // Write performance measurement statistics, if total size verification passed.
        cout << endl << " Performance Statistics." << endl;
        cout << "-------------------------------------------------------------------" << endl;
        cout << " Operation      min           max         average       median" << endl;
        cout << "-------------------------------------------------------------------" << endl;
        writeStatistics(msg, " READ  ", readIops, true);
        writeStatistics(msg, "\r\n WRITE ", writeIops, true);
        cout << endl << "-------------------------------------------------------------------" << endl;
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        cout << endl << "Total read/write size verification error." << endl;
        snprintf(msg, MAX_TEXT_STRING, "Calculated = %016llXh,", totalBytes);
        cout << endl << msg << endl;
        snprintf(msg, MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        cout << endl << msg << endl;
        snprintf(msg, MAX_TEXT_STRING, ", Write      = %016llXh,", totalWrite);
        cout << endl << msg << endl;
        closeContext(msg, FILE_SIZE_MISMATCH, fileData, previousAffinity);
        return 1;
    }
    // Done, release resources and return.
    closeContext(msg, NO_ERRORS, fileData, previousAffinity);
    return 0;
}

// Helpers functions implementation.
void showReturn(char* msg, STATUS_CODES opStatus)
{
    cout << endl << "Operation status = " << opStatus;
    if (opStatus == NO_ERRORS)
    {
        cout << " = No errors.";
    }
    else if ((opStatus > 0) && (opStatus < STATUS_MAX))
    {
        const char* returnName = STATUS_NAMES[opStatus];
        snprintf(msg, MAX_TEXT_STRING, " = %s.", returnName);
        cout << msg;
        DWORD osError = GetLastError();
        if (osError)
        {
            storeSystemErrorName(msg, MAX_TEXT_STRING, osError);
            cout << endl << msg;
        }
    }
    else
    {
        cout << " = Unknown error.";
    }
    cout << endl;
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
void calculateStatistics(std::vector<double>& data, double& min, double& max, double& average, double& median)
{
    size_t n = data.size();
    if (n)
    {
        std::sort(data.begin(), data.end());
        double sum = std::accumulate(data.begin(), data.end(), double(0));
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
STATUS_CODES openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity)
{
    STATUS_CODES statusCode = NO_ERRORS;
    // Memory allocation.
    // TODO. Support minDomain-maxDomain range required. Yet minDomain only used, single domain only.
    if (minDomain != DEFAULT_DOMAIN_SELECT)
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
    if ((statusCode == NO_ERRORS) && (minCpu != DEFAULT_CPU_SELECT))
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
STATUS_CODES closeContext(char* msg, STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity)
{
    STATUS_CODES statusCode = operationStatus;
    if (previousAffinity)
    {
        DWORD_PTR statusAffinity = SetThreadAffinityMask(GetCurrentThread(), previousAffinity);
        if ((statusCode != NO_ERRORS) && (!statusAffinity)) statusCode = RESTORE_CPU_AFFINITY_FAILED;
    }
    if (!fileData)
    {
        if (statusCode != NO_ERRORS) statusCode = MEMORY_RELEASE_FAILED;
    }
    else
    {
        if (!VirtualFreeEx(GetCurrentProcess(), fileData, 0, MEM_RELEASE))
        {
            if (statusCode != NO_ERRORS) statusCode = MEMORY_RELEASE_FAILED;
        }
    }
    showReturn(msg, statusCode);
    return statusCode;
}
DWORD64 getHandle64(HANDLE handle)
{
#ifdef NATIVE_WIDTH_64
    DWORD64 numHandle = (DWORD64)handle;
#else
    DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
    return numHandle;
}
void waitTime(char* msg, DWORD milliseconds, const char* operationName)
{
    if (milliseconds)
    {
        snprintf(msg, MAX_TEXT_STRING, " Wait before start %s (%d ms).", operationName, milliseconds);
        cout << endl << msg;
        Sleep(milliseconds);
    }
}
void writeStatistics(char* msg, const char* statisticsName, std::vector<double>& speeds, bool tableMode)
{
    double min = 0.0, max = 0.0, average = 0.0, median = 0.0;
    calculateStatistics(speeds, min, max, average, median);
    snprintf(msg, MAX_TEXT_STRING, "%s", statisticsName);
    cout << msg;
    if (tableMode)
    {
        snprintf(msg, MAX_TEXT_STRING,
            "%14.3f%14.3f%14.3f%14.3f", min, max, average, median);
    }
    else
    {
        snprintf(msg, MAX_TEXT_STRING,
            "min = %8.3f,  max = %8.3f,  average = %8.3f,  median = %8.3f.",
            min, max, average, median);
    }
    cout << msg;
}
void buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize)
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
        DWORD64 seed = RANDOM_SEED;
        for (int i = 0; i < n; i++)
        {
            seed *= RANDOM_MULTIPLIER;
            seed += RANDOM_ADDEND;
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
        snprintf(msg, MAX_TEXT_STRING, "%s", dataName);
    }
    else if (!(st1 && st2))
    {
        snprintf(msg, MAX_TEXT_STRING, "Memory initializing timings failed.");
    }
    else
    {
        double unit = 1.0 / hz.QuadPart;
        double seconds = (tm2.QuadPart - tm1.QuadPart) * unit;
        double megabytes = fileSize / 1000000.0;
        double mbps = megabytes / seconds;
        snprintf(msg, MAX_TEXT_STRING, "Memory initializing (%s), fill rate = %.3f MBPS.", dataName, mbps);
    }
    cout << msg;
}
void buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR>& list,
    unsigned int blocksPerFile, unsigned int fileCount)
{
    list.clear();
    DWORD32 randomNumber;
    DWORD32 seed = RANDOM_SEED;
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
                    seed *= RANDOM_MULTIPLIER;
                    seed += RANDOM_ADDEND;
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
BOOL createEvents(char* msg, std::vector<HANDLE>& events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count)
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
        snprintf(msg, MAX_TEXT_STRING, "Async I/O initializing, %d events created.", count);
        cout << endl << msg;
    }
    return status;
}
BOOL closeEvents(std::vector<HANDLE>& events)
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
