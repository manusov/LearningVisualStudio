/*

Mass storage performance test.
Classes source file edition.
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task works class implementation.
Class used for mass storage read/write/copy
benchmarking scenarios.

*/

#include "TaskWork.h"

int TaskWork::runTaskMBPS(COMMAND_LINE_PARMS* p)
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
    AppLib::write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    AppLib::write("\r\nMemory allocated, ");
    AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    AppLib::write(msg);
    AppLib::write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    AppLib::writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        AppLib::write(msg);
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
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            AppLib::write(msg);
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
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    // Table up for WRITE operation.
    AppLib::writeColor("\r\n Write file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Initializing for copy source files to destination files (re-positioning required for source files after it writes).
    std::vector<double> copySpeed;
    // Wait before COPY operation start, if selected by option.
    waitTime(msg, msWaitCopy, "copy");
    // Table up for COPY operation.
    AppLib::writeColor("\r\n Copy file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (copyError)
    {
        return closeContext(FILE_COPY_FAILED, fileData, previousAffinity);
    }
    // Initializing for read source files (re-positioning required for source files after it copy).
    std::vector<double> readSpeed;
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    // Table up for READ operation.
    AppLib::writeColor("\r\n Read file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // Close and delete files.
    AppLib::writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Delete source and destination files done.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    if ((totalBytes > 0) &&
        (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    {   // Write performance measurement statistics, if total size verification passed.
        AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        AppLib::writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readSpeed, true);
        writeStatistics(msg, "\r\n WRITE ", writeSpeed, true);
        writeStatistics(msg, "\r\n COPY  ", copySpeed, true);
        AppLib::write("\r\n");
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        AppLib::writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int TaskWork::runTaskMBPSoneFile(COMMAND_LINE_PARMS* p)
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
    AppLib::write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    AppLib::write("\r\nMemory allocated, ");
    AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    AppLib::write(msg);
    AppLib::write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    AppLib::writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    // Create source file.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, 0, nameExt);
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;
    if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
    if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
    if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    HANDLE srcHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
    DWORD64 numHandle = getHandle64(srcHandle);
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", 0, numHandle, path);
    AppLib::write(msg);
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
        DWORD64 numHandle = getHandle64(dstHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", 0, numHandle, path);
        AppLib::write(msg);
        if ((dstHandle == NULL) || (dstHandle == INVALID_HANDLE_VALUE))
        {
            createError = TRUE;
        }
    }
    // Both source and destination files created (yet zero size).
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    // Table up for WRITE, COPY, READ operations.
    AppLib::writeColor("\r\n Write, Copy, Read file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index      MBPS,  Write            Copy             Read\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::write(msg);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    AppLib::writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, 0, nameExt);
    if (!CloseHandle(srcHandle)) deleteError = TRUE;
    if (!DeleteFile(path)) deleteError = TRUE;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", 0, path);
    AppLib::write(msg);
    // Close and delete destination files.
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, 0, nameExt);
    if (!CloseHandle(dstHandle)) deleteError = TRUE;
    if (!DeleteFile(path)) deleteError = TRUE;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", 0, path);
    AppLib::write(msg);
    // Delete source and destination files done.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    if ((totalBytes > 0) &&
        (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    {   // Write performance measurement statistics, if total size verification passed.
        AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        AppLib::writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readSpeed, true);
        writeStatistics(msg, "\r\n WRITE ", writeSpeed, true);
        writeStatistics(msg, "\r\n COPY  ", copySpeed, true);
        AppLib::write("\r\n");
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        AppLib::writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
/*
int runTaskMBPSmap(COMMAND_LINE_PARMS* p)
{
    // (1) Initializing variables by constants.
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
    // (2) If input parameters structure passed, load parameters from this structure, override default constants.
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
    // (3) Initializing variables for read and write total sizes verification.
    DWORD64 totalRead = 0;
    DWORD64 totalWrite = 0;
    DWORD64 totalBytes = (DWORD64)fileSize * (DWORD64)fileCount * (DWORD64)repeats;
    // (4) Initializing timer (use OS performance counter).
    LARGE_INTEGER hz;
    BOOL status;
    status = QueryPerformanceFrequency(&hz);
    if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
    double MHz = (hz.QuadPart) / 1000000.0;
    double us = 1.0 / MHz;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.\r\n", MHz, us);
    write(msg);
    LARGE_INTEGER t1, t2, t3, t4;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    // (5) Create file.
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, 0, nameExt);
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;
    if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
    if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
    if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    HANDLE fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
    if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
    {
        DWORD64 numHandle = getHandle64(fileHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Open file for write, handle = %016llXh, path = %s.\r\n", numHandle, path);
        write(msg);
    }
    else
    {
        return FILE_CREATE_FAILED;
    }
    // (6) Create mapping object for file, message about mapping object handle.
    HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
    if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
    {
        DWORD64 numHandle = getHandle64(mapHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Mapping handle = %016llXh.\r\n", numHandle);
        write(msg);
    }
    else
    {
        return FILE_MAPPING_CREATE_FAILED;
    }
    // (7) Mapping created object to address space, message about mapped buffer address.
    LPVOID mapPointer = MapViewOfFile(mapHandle, FILE_MAP_WRITE, 0, 0, fileSize);
    if ((mapPointer != NULL) && (mapPointer != INVALID_HANDLE_VALUE))
    {
        DWORD64 numPointer = getHandle64(mapPointer);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Mapping address = %016llXh.\r\n", numPointer);
        write(msg);
    }
    else
    {
        return MAP_VIEW_OF_FILE_FAILED;
    }
    // (8) Fill mapped buffer.
    waitTime(msg, msWaitWrite, "write");    // Wait before WRITE operation start, if selected by option.
    writeColor(" Write and flush buffer.", APPCONST::VALUE_COLOR);
    memset(mapPointer, 0, fileSize);
    // (9) Flush mapped buffer to disk, this is WRITE operation, measure time, message about WRITE MBPS.
    QueryPerformanceCounter(&t3);
    BOOL flushStatus = (FlushViewOfFile(mapPointer, fileSize));
    QueryPerformanceCounter(&t4);
    if(flushStatus)
    {
        double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " MBPS = %.3f.\r\n", mbps);
        writeColor(msg, APPCONST::GROUP_COLOR);
    }
    else
    {
        return FLUSH_VIEW_OF_FILE_FAILED;
    }
    // (10) Close mapping object.
    if (!CloseHandle(mapHandle))
    {
        return HANDLE_CLOSE_FAILED;
    }
    // (11) Close file.
    if (!CloseHandle(fileHandle))
    {
        return HANDLE_CLOSE_FAILED;
    }
    // (12) Unmap view of file.
    if (!UnmapViewOfFile(mapPointer))
    {
        return UNMAP_VIEW_OF_FILE_FAILED;
    }
    // (13) Re-Open file for read.
    attributes = FILE_ATTRIBUTE_NORMAL;
    if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
    if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
    if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
    fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, attributes, nullptr);
    if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
    {
        DWORD64 numHandle = getHandle64(fileHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nRe-open file for read, handle = %016llXh, path = %s.\r\n", numHandle, path);
        write(msg);
    }
    else
    {
        return FILE_CREATE_FAILED;
    }
    // (14) Re-Create mapping object for file, message about mapping object handle.
    mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
    if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
    {
        DWORD64 numHandle = getHandle64(mapHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Mapping handle = %016llXh.\r\n", numHandle);
        write(msg);
    }
    else
    {
        return FILE_MAPPING_CREATE_FAILED;
    }
    // (15) Mapping created object to address space, message about mapped buffer address.
    mapPointer = MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, fileSize);
    if ((mapPointer != NULL) && (mapPointer != INVALID_HANDLE_VALUE))
    {
        DWORD64 numPointer = getHandle64(mapPointer);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Mapping address = %016llXh.\r\n", numPointer);
        write(msg);
    }
    else
    {
        return MAP_VIEW_OF_FILE_FAILED;
    }
    // (16) Read buffer with loads from disk, this is READ operation, measure time, message about READ MBPS.
    waitTime(msg, msWaitRead, "read");    // Wait before READ operation start, if selected by option.
    writeColor(" Buffer page walk.", APPCONST::VALUE_COLOR);
    int pageWalkCount = fileSize / APPCONST::PAGE_WALK_STEP;
    DWORD* pageWalkAddress = reinterpret_cast<DWORD*>(mapPointer);
    size_t pageWalkIncrement = APPCONST::PAGE_WALK_STEP / sizeof(DWORD);
    DWORD pageWalkData = 0;
    QueryPerformanceCounter(&t3);
    for (int i = 0; i < pageWalkCount; i++)
    {
        pageWalkData += *pageWalkAddress;
        pageWalkAddress += pageWalkIncrement;
    }
    QueryPerformanceCounter(&t4);
    double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
    double mbps = fileMegabytes / seconds;
    snprintf(msg, APPCONST::MAX_TEXT_STRING, " MBPS = %.3f (%d).\r\n", mbps, pageWalkData);
    writeColor(msg, APPCONST::GROUP_COLOR);
    // (17) Close mapping object.
    if (!CloseHandle(mapHandle))
    {
        return HANDLE_CLOSE_FAILED;
    }
    // (18) Close file.
    if (!CloseHandle(fileHandle))
    {
        return HANDLE_CLOSE_FAILED;
    }
    // (19) Unmap view of file.
    if (!UnmapViewOfFile(mapPointer))
    {
        return UNMAP_VIEW_OF_FILE_FAILED;
    }
    // (20) Delete file.
    if (!DeleteFile(path))
    {
        return FILE_DELETE_FAILED;
    }
    // (21) Done.
    return FUNCTION_UNDER_CONSTRUCTION;
}
*/
int TaskWork::runTaskMBPSmap(COMMAND_LINE_PARMS* p)
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
    AppLib::write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    AppLib::write("\r\nMemory allocated, ");
    AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    AppLib::write(msg);
    AppLib::write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files and mapping objects.
    AppLib::writeColor("\r\n\r\n Create files and mapping objects.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     File Handle (hex)   Map Handle (hex)    Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    char path[MAX_PATH];
    BOOL createError = FALSE;
    STATUS_CODES statusCode = INTERNAL_ERROR;
    // Create source files and mapping objects.
    std::vector<HANDLE> srcHandles;
    std::vector<HANDLE> srcMapHandles;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
        if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(fileHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_CREATE_FAILED;
            break;
        }
        HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
        if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
        {
            srcMapHandles.push_back(mapHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_MAPPING_CREATE_FAILED;
            break;
        }
        DWORD64 numFileHandle = getHandle64(fileHandle);
        DWORD64 numMapHandle = getHandle64(mapHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX    %016llX    %s\r\n", i, numFileHandle, numMapHandle, path);
        AppLib::write(msg);
    }
    // Create destination files and mapping objects.
    std::vector<HANDLE> dstHandles;
    std::vector<HANDLE> dstMapHandles;
    if (!createError)
    {
        for (unsigned int i = 0; i < fileCount; i++)
        {
            snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
            DWORD attributes = FILE_ATTRIBUTE_NORMAL;
            if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
            if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
            if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
            HANDLE fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, attributes, nullptr);
            if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
            {
                dstHandles.push_back(fileHandle);
            }
            else
            {
                createError = TRUE;
                statusCode = FILE_CREATE_FAILED;
                break;
            }
            HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
            if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
            {
                dstMapHandles.push_back(mapHandle);
            }
            else
            {
                createError = TRUE;
                statusCode = FILE_MAPPING_CREATE_FAILED;
                break;
            }
            DWORD64 numFileHandle = getHandle64(fileHandle);
            DWORD64 numMapHandle = getHandle64(mapHandle);
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX    %016llX    %s\r\n", i, numFileHandle, numMapHandle, path);
            AppLib::write(msg);
        }
    }
    // Both source and destination files and mapping objects created (yet zero size).
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(statusCode, fileData, previousAffinity);
    }
    // Initializing for timings measurements.
    LARGE_INTEGER t1, t2, t3, t4, t5;
    double timeUnitSeconds = 1.0 / hz.QuadPart;
    double fileMegabytes = fileSize * repeats / 1000000.0;
    double integralMegabytes = fileMegabytes * fileCount;
    std::vector<double> readSpeed;
    std::vector<double> writeSpeed;
    std::vector<double> copySpeed;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    // Table up for WRITE operation.
    AppLib::writeColor("\r\n Write memory-mapped file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    // Start cycle for WRITE source files as flush memory-mapped I/O.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        DWORD oneFileWrite = 0;
        t5.QuadPart = 0;
        // Start cycle for measurement repeats per each file.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE mapHandle = srcMapHandles[i];
            DWORD32 writeSize = fileSize;
            LPVOID dataPointer = fileData;
            // Start cycle for blocks in the file.
            while (writeSize)
            {
                LPVOID mapPointer = MapViewOfFile(mapHandle, FILE_MAP_WRITE, 0, oneFileWrite, blockSize);
                if (!mapPointer)
                {
                    writeError = TRUE;
                    break;
                }
                DWORD32 flushSize = blockSize;
                if (flushSize > writeSize)
                {
                    flushSize = writeSize;
                }
                memcpy(mapPointer, dataPointer, flushSize);
                QueryPerformanceCounter(&t3);  // Start timer for one block write.
                BOOL flushStatus = FlushViewOfFile(mapPointer, flushSize);
                QueryPerformanceCounter(&t4);  // Stop timer for one block write.
                t5.QuadPart += (t4.QuadPart - t3.QuadPart);
                writeSize -= flushSize;
                oneFileWrite += flushSize;
                totalWrite += flushSize;
                BOOL unmapStatus = UnmapViewOfFile(mapPointer);
                if ((!flushStatus) || (!unmapStatus))
                {
                    writeError = TRUE;
                    break;
                }
            } // End cycle for blocks in the file.
            if (writeError) break;
        }  // End cycle for measurement repeats.
        if (writeError) break;
        double seconds = t5.QuadPart * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        writeSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Close source files and mappings for invalidate OS file caches.
    BOOL closeError = FALSE;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcMapHandles[i])) closeError = TRUE;
        if (!CloseHandle(srcHandles[i])) closeError = TRUE;
    }
    // Delete source and destination files done.
    if (closeError)
    {
        return closeContext(HANDLE_CLOSE_FAILED, fileData, previousAffinity);
    }
    srcHandles.clear();
    srcMapHandles.clear();
    // Re-open source files and re-create mapping objects.
    AppLib::writeColor("\r\n Re-open files and re-create mapping objects.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     File Handle (hex)   Map Handle (hex)    Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, attributes, nullptr);
        if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(fileHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_OPEN_FAILED;
            break;
        }
        HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
        if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
        {
            srcMapHandles.push_back(mapHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_MAPPING_CREATE_FAILED;
            break;
        }
        DWORD64 numFileHandle = getHandle64(fileHandle);
        DWORD64 numMapHandle = getHandle64(mapHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX    %016llX    %s\r\n", i, numFileHandle, numMapHandle, path);
        AppLib::write(msg);
    }
    // Source files re-opened.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(statusCode, fileData, previousAffinity);
    }
    // Wait before COPY operation start, if selected by option.
    waitTime(msg, msWaitCopy, "copy");
    // Table up for WRITE operation.
    AppLib::writeColor("\r\n Copy memory-mapped file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL copyError = FALSE;
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
    // Start cycle for WRITE source files as flush memory-mapped I/O.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        DWORD oneFileCopy = 0;
        t5.QuadPart = 0;
        // Start cycle for measurement repeats per each file.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE srcMapHandle = srcMapHandles[i];
            HANDLE dstMapHandle = dstMapHandles[i];
            DWORD32 copySize = fileSize;
            LPVOID dataPointer = fileData;
            // Start cycle for blocks in the file.
            while (copySize)
            {
                LPVOID srcMapPointer = MapViewOfFile(srcMapHandle, FILE_MAP_READ, 0, oneFileCopy, blockSize);
                LPVOID dstMapPointer = MapViewOfFile(dstMapHandle, FILE_MAP_WRITE, 0, oneFileCopy, blockSize);
                if ((!srcMapPointer) || (!dstMapPointer))
                {
                    copyError = TRUE;
                    break;
                }
                DWORD32 flushSize = blockSize;
                if (flushSize > copySize)
                {
                    flushSize = copySize;
                }
                QueryPerformanceCounter(&t3);  // Start timer for one block copy.
                memcpy(dstMapPointer, srcMapPointer, flushSize);
                BOOL flushStatus = FlushViewOfFile(dstMapPointer, flushSize);
                QueryPerformanceCounter(&t4);  // Stop timer for one block copy.
                t5.QuadPart += (t4.QuadPart - t3.QuadPart);
                copySize -= flushSize;
                oneFileCopy += flushSize;
                totalCopyRead += flushSize;
                totalCopyWrite += flushSize;
                BOOL dstUnmapStatus = UnmapViewOfFile(dstMapPointer);
                BOOL srcUnmapStatus = UnmapViewOfFile(srcMapPointer);
                if ((!flushStatus) || (!dstUnmapStatus) || (!srcUnmapStatus))
                {
                    copyError = TRUE;
                    break;
                }
            } // End cycle for blocks in the file.
            if (copyError) break;
        }  // End cycle for measurement repeats.
        if (copyError) break;
        double seconds = t5.QuadPart * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        copySpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_COPY_FAILED, fileData, previousAffinity);
    }
    // Close source files and mappings for invalidate OS file caches.
    closeError = FALSE;
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcMapHandles[i])) closeError = TRUE;
        if (!CloseHandle(srcHandles[i])) closeError = TRUE;
    }
    // Delete source and destination files done.
    if (closeError)
    {
        return closeContext(HANDLE_CLOSE_FAILED, fileData, previousAffinity);
    }
    srcHandles.clear();
    srcMapHandles.clear();
    // Re-open source files and re-create mapping objects.
    AppLib::writeColor("\r\n Re-open files and re-create mapping objects.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     File Handle (hex)   Map Handle (hex)    Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        DWORD attributes = FILE_ATTRIBUTE_NORMAL;
        if (flagNoBuffering)   attributes |= FILE_FLAG_NO_BUFFERING;
        if (flagWriteThrough)  attributes |= FILE_FLAG_WRITE_THROUGH;
        if (flagSequentalScan) attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
        HANDLE fileHandle = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, attributes, nullptr);
        if ((fileHandle != NULL) && (fileHandle != INVALID_HANDLE_VALUE))
        {
            srcHandles.push_back(fileHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_OPEN_FAILED;
            break;
        }
        HANDLE mapHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READWRITE, 0, fileSize, nullptr);
        if ((mapHandle != NULL) && (mapHandle != INVALID_HANDLE_VALUE))
        {
            srcMapHandles.push_back(mapHandle);
        }
        else
        {
            createError = TRUE;
            statusCode = FILE_MAPPING_CREATE_FAILED;
            break;
        }
        DWORD64 numFileHandle = getHandle64(fileHandle);
        DWORD64 numMapHandle = getHandle64(mapHandle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX    %016llX    %s\r\n", i, numFileHandle, numMapHandle, path);
        AppLib::write(msg);
    }
    // Source files re-opened.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (createError)
    {
        return closeContext(statusCode, fileData, previousAffinity);
    }
    // Wait before READ operation start, if selected by option.
    waitTime(msg, msWaitRead, "read");
    // Table up for READ operation.
    AppLib::writeColor("\r\n Read memory-mapped file performance.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL readError = FALSE;
    // Start timer for integral time of read source files.
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
    // Start cycle for READ source files as read memory-mapped I/O.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        DWORD pageWalkData = 0;  // This for prevent speculative skips.
        DWORD oneFileRead = 0;
        t5.QuadPart = 0;
        // Start cycle for measurement repeats per each file.
        for (unsigned int j = 0; j < repeats; j++)
        {
            HANDLE mapHandle = srcMapHandles[i];
            DWORD32 readSize = fileSize;
            LPVOID dataPointer = fileData;
            // Start cycle for blocks in the file.
            while (readSize)
            {
                LPVOID mapPointer = MapViewOfFile(mapHandle, FILE_MAP_READ, 0, oneFileRead, blockSize);
                if (!mapPointer)
                {
                    readError = TRUE;
                    break;
                }
                DWORD32 flushSize = blockSize;
                if (flushSize > readSize)
                {
                    flushSize = readSize;
                }
                DWORD* pageWalkAddress = reinterpret_cast<DWORD*>(mapPointer);
                int pageWalkCount = flushSize / APPCONST::PAGE_WALK_STEP;
                if (!pageWalkCount)
                {
                    pageWalkCount = 1;
                }
                size_t pageWalkIncrement = APPCONST::PAGE_WALK_STEP / sizeof(DWORD);
                QueryPerformanceCounter(&t3);
                for (int i = 0; i < pageWalkCount; i++)
                {
                    pageWalkData += *pageWalkAddress;
                    pageWalkAddress += pageWalkIncrement;
                }
                QueryPerformanceCounter(&t4);
                t5.QuadPart += (t4.QuadPart - t3.QuadPart);
                readSize -= flushSize;
                oneFileRead += flushSize;
                totalRead += flushSize;
                BOOL unmapStatus = UnmapViewOfFile(mapPointer);
                if (!unmapStatus)
                {
                    readError = TRUE;
                    break;
                }
            } // End cycle for blocks in the file.
            if (readError) break;
        }  // End cycle for measurement repeats.
        if (readError) break;
        double seconds = t5.QuadPart * timeUnitSeconds;
        double mbps = fileMegabytes / seconds;
        readSpeed.push_back(mbps);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f (%d)\r\n", i, mbps, pageWalkData);
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // Close files and file mappings, delete files.
    AppLib::writeColor("\r\n Close files and file mappings, delete files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcMapHandles[i])) deleteError = TRUE;
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstMapHandles[i])) deleteError = TRUE;
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Delete source and destination files done.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (deleteError)
    {
        return closeContext(FILE_DELETE_FAILED, fileData, previousAffinity);
    }
    // Verify total read, write and copy sizes: compare returned by WinAPI and calculated values.
    if ((totalBytes > 0) &&
        (totalRead == totalBytes) && (totalWrite == totalBytes) && (totalCopyRead == totalBytes) && (totalCopyWrite == totalBytes))
    {   // Write performance measurement statistics, if total size verification passed.
        AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        AppLib::writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readSpeed, true);
        writeStatistics(msg, "\r\n WRITE ", writeSpeed, true);
        writeStatistics(msg, "\r\n COPY  ", copySpeed, true);
        AppLib::write("\r\n");
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        AppLib::writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int TaskWork::runTaskIOPS(COMMAND_LINE_PARMS* p)
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
    AppLib::write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    AppLib::write("\r\nBuffer memory allocated, ");
    AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    AppLib::write(msg);
    AppLib::write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create source and destination files.
    AppLib::writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        AppLib::write(msg);
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
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            AppLib::write(msg);
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
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    // Table up for SERVICE WRITE operation.
    AppLib::writeColor("\r\n Service write files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Re-open source and destination files.
    AppLib::writeColor("\r\n Re-open files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        DWORD64 numHandle = getHandle64(handle);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", j, numHandle, path);
        AppLib::write(msg);
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
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    AppLib::writeColor("\r\n Read IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
            AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // IOPS measurement for WRITE, both source and destination files zones (read any).
    std::vector<double> writeIops;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    AppLib::writeColor("\r\n Write IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
            AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // RESERVED: IOPS measurement for copy, from source to destination files zones (read src, write dst).
    // RESERVED: IOPS measurement for copy, both source and destination files zones (read any, write any).
    // Close and delete files.
    AppLib::writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Delete source and destination files done.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        AppLib::writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readIops, true);
        writeStatistics(msg, "\r\n WRITE ", writeIops, true);
        //      writeStatistics(msg, "\r\n COPY  ", copyIops, true);
        AppLib::write("\r\n");
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        AppLib::writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int TaskWork::runTaskIOPSqueued(COMMAND_LINE_PARMS* p)
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
    AppLib::write(msg);
    // Allocate memory for file I/O buffer, set CPU affinity and NUMA domain if required by scenario options settings.
    LPVOID fileData = nullptr;
    DWORD_PTR previousAffinity = 0;
    STATUS_CODES retStatus = openContext(fileSize, minCpu, maxCpu, minDomain, maxDomain, fileData, previousAffinity);
    if (retStatus != NO_ERRORS)
    {
        return closeContext(retStatus, fileData, previousAffinity);
    }
    AppLib::write("\r\nBuffer memory allocated, ");
    AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
    AppLib::write(msg);
    AppLib::write(".");
    // Initializing data for write files.
    buildData(msg, hz, dataType, fileData, fileSize);
    // Create events handles for asynchronous I/O.
    std::vector<HANDLE> eventHandles;
    std::vector<OVERLAPPED> overlaps;
    std::vector<DWORD> returns;
    createEvents(msg, eventHandles, overlaps, returns, queueDepth);
    // Create source and destination files.
    AppLib::writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
        AppLib::write(msg);
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
            snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", i, numHandle, path);
            AppLib::write(msg);
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
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    // Table up for SERVICE WRITE operation.
    AppLib::writeColor("\r\n Service write files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (writeError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_WRITE_FAILED, fileData, previousAffinity);
    }
    // Re-open source and destination files.
    AppLib::writeColor("\r\n Re-open files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%016llX     %s\r\n", j, numHandle, path);
        AppLib::write(msg);
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
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
    AppLib::writeColor("\r\n Read IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
            AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        closeEvents(eventHandles);
        return closeContext(FILE_READ_FAILED, fileData, previousAffinity);
    }
    // IOPS measurement for WRITE, both source and destination files zones (read any).
    std::vector<double> writeIops;
    // Wait before WRITE operation start, if selected by option.
    waitTime(msg, msWaitWrite, "write");
    AppLib::writeColor("\r\n Write IOPS measurement.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     IOPS\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
            AppLib::write(msg);
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
        AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
    if (readError)
    {
        closeEvents(eventHandles);
        return closeContext(TIMER_FAILED, fileData, previousAffinity);
    }
    // RESERVED: IOPS measurement for copy, from source to destination files zones (read src, write dst).
    // RESERVED: IOPS measurement for copy, both source and destination files zones (read any, write any).
    // Close and delete files.
    AppLib::writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    BOOL deleteError = FALSE;
    // Close and delete source files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
        if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Close and delete destination files.
    for (unsigned int i = 0; i < fileCount; i++)
    {
        snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
        if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
        if (!DeleteFile(path)) deleteError = TRUE;
        snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
        AppLib::write(msg);
    }
    // Delete source and destination files done.
    AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
        AppLib::writeColor("\r\n Performance Statistics.\r\n", APPCONST::VALUE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        AppLib::writeColor(" Operation      min           max         average       median\r\n", APPCONST::TABLE_COLOR);
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
        writeStatistics(msg, " READ  ", readIops, true);
        writeStatistics(msg, "\r\n WRITE ", writeIops, true);
        // writeStatistics(msg, "\r\n COPY  ", copyIops, true);
        AppLib::write("\r\n");
        AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
    }
    else
    {   // Write error message, if total read, write, copy, size verification failed.
        AppLib::writeColor("\r\nTotal read/write size verification error.", APPCONST::ERROR_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nCalculated = %016llXh,\r\n", totalBytes);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Read       = %016llXh", totalRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Write      = %016llXh,\r\n", totalWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "Copy read  = %016llXh", totalCopyRead);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        snprintf(msg, APPCONST::MAX_TEXT_STRING, ", Copy write = %016llXh.\r\n", totalCopyWrite);
        AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
        closeEvents(eventHandles);
        return closeContext(FILE_SIZE_MISMATCH, fileData, previousAffinity);
    }
    // Done, release resources and return.
    return closeContext(NO_ERRORS, fileData, previousAffinity);
}
int TaskWork::runTaskIOPSmap(COMMAND_LINE_PARMS* p)
{
    return FUNCTION_UNDER_CONSTRUCTION;
}
// Target operation-specific tasks (measure storage performance in this application).
DWORD64 TaskWork::getHandle64(HANDLE handle)
{
#ifdef NATIVE_WIDTH_64
    DWORD64 numHandle = (DWORD64)handle;
#else
    DWORD64 numHandle = (DWORD64)((DWORD32)handle);
#endif
    return numHandle;
}
void TaskWork::waitTime(char* msg, DWORD milliseconds, const char* operationName)
{
    if (milliseconds)
    {
        snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\n Wait before start %s (%d ms).", operationName, milliseconds);
        AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
        Sleep(milliseconds);
    }
}
void TaskWork::writeStatistics(char* msg, const char* statisticsName, std::vector<double> &speeds, bool tableMode)
{
    double min = 0.0, max = 0.0, average = 0.0, median = 0.0;
    AppLib::calculateStatistics(speeds, min, max, average, median);
    snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s", statisticsName);
    AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
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
    AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}
void TaskWork::buildData(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, LPVOID fileData, DWORD32 fileSize)
{
    const char* dataName = "?";
    BOOL dataStatus = FALSE;
    LARGE_INTEGER tm1, tm2;
    BOOL st1 = QueryPerformanceCounter(&tm1);
    switch (dataType)
    {
    case ALL_ZEROES:
        memset(fileData, 0, fileSize);
        dataName = APPCONST::DATA_NAMES[0];
        dataStatus = TRUE;
        break;
    case ALL_ONES:
        memset(fileData, -1, fileSize);
        dataName = APPCONST::DATA_NAMES[1];
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
    dataName = APPCONST::DATA_NAMES[2];
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
    dataName = APPCONST::DATA_NAMES[3];
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
    dataName = APPCONST::DATA_NAMES[4];
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
    AppLib::write(msg);
}
void TaskWork::buildAddress(char* msg, LARGE_INTEGER& hz, D_TYPE dataType, std::vector<IO_DESCRIPTOR>& list,
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
bool TaskWork::ioDescriptorComparator(IO_DESCRIPTOR d1, IO_DESCRIPTOR d2)
{
    return d1.random < d2.random;
}
BOOL TaskWork::createEvents(char* msg, std::vector<HANDLE>& events, std::vector<OVERLAPPED>& overlaps, std::vector<DWORD>& returns, unsigned int count)
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
        AppLib::write(msg);
    }
    return status;
}
BOOL TaskWork::closeEvents(std::vector<HANDLE>& events)
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
STATUS_CODES TaskWork::openContext(DWORD fileSize, DWORD minCpu, DWORD maxCpu, DWORD minDomain, DWORD maxDomain, LPVOID& fileData, DWORD_PTR& previousAffinity)
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
STATUS_CODES TaskWork::closeContext(STATUS_CODES operationStatus, LPVOID fileData, DWORD_PTR previousAffinity)
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
    return statusCode;
}
