/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Class for mass storage performance task.

*/

#include "Task.h"

extern "C" void __stdcall randomArray(void* ptr, DWORD32 count);

const char* DATA_NAMES[]
{	"ALL_ZEROES",
	"ALL_ONES",
	"INCREMENTAL",
	"SOFT_RANDOM",
	"HARD_RANDOM"  };

void writeStatistics(char* msg, const char* statisticsName, std::vector<double>& speeds)
{
	double min = 0.0, max = 0.0, average = 0.0, median = 0.0;
	AppLib::calculateStatistics(speeds, min, max, average, median);
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s", statisticsName);
	AppLib::writeColor(msg, APPCONST::GROUP_COLOR);
	snprintf(msg, APPCONST::MAX_TEXT_STRING,
		"max = %8.3f,  min = %8.3f,  average = %8.3f,  median = %8.3f.",
		max, min, average, median);
	AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
}


int Task::Run(COMMAND_LINE_PARMS* p)
{
	// Initializing variables.
	char msg[APPCONST::MAX_TEXT_STRING];

	const char* nameSrcPath = DEFAULT_PATH_SRC;
	const char* nameDstPath = DEFAULT_PATH_DST;
	const char* nameSrc = DEFAULT_NAME_SRC;
	const char* nameDst = DEFAULT_NAME_DST;
	const char* nameExt = DEFAULT_EXTENSION;
	DWORD32 fileCount = DEFAULT_FILE_COUNT;
	DWORD32 fileSize = DEFAULT_FILE_SIZE;
	DWORD32 blockSize = DEFAULT_BLOCK_SIZE;
	DWORD32 repeats = DEFAULT_REPEATS;
	D_TYPE dataType = DEFAULT_DATA_TYPE;
	A_TYPE addressType = DEFAULT_ADDRESS_TYPE;
	BOOL flagNoBuffering = DEFAULT_FLAG_NO_BUFFERING;
	BOOL flagWriteThrough = DEFAULT_FLAG_WRITE_THROUGH;
	BOOL flagSequentalScan = DEFAULT_FLAG_SEQUENTIAL_SCAN;

	if (p)
	{
		nameSrcPath = p->optionSrcPath;
		nameDstPath = p->optionDstPath;
		fileCount = p->optionFileCount;
		fileSize = static_cast<DWORD32>(p->optionFileSize);
		blockSize = static_cast<DWORD32>(p->optionBlockSize);
		repeats = 1; //  p->optionRepeats;
		dataType = static_cast<D_TYPE>(p->optionData);
		addressType = static_cast<A_TYPE>(p->optionAddress);
		flagNoBuffering = p->optionNoBuf;
		flagWriteThrough = p->optionWriteThr;
		flagSequentalScan = p->optionSequental;
	}
	
	// Initializing timer.
	LARGE_INTEGER hz;
	BOOL status;
	status = QueryPerformanceFrequency(&hz);
	if ((!status) || (hz.QuadPart == 0)) return TIMER_FAILED;
	double MHz = (hz.QuadPart) / 1000000.0;
	double us = 1.0 / MHz;
	snprintf(msg, APPCONST::MAX_TEXT_STRING, "Performance frequency %.3f MHz, period %.3f microseconds.", MHz, us);
	AppLib::write(msg);

	
	// Allocate memory.
	LPVOID fileData = VirtualAlloc(nullptr, fileSize, MEM_RESERVE + MEM_COMMIT, PAGE_READWRITE);
	if (!fileData) return MEMORY_ALLOCATION_FAILED;
	AppLib::write("\r\nMemory allocated, ");
	AppLib::storeBaseAndSize(msg, APPCONST::MAX_TEXT_STRING, (DWORD64)fileData, (DWORD64)fileSize);
	AppLib::write(msg);
	AppLib::write(".");


	// Initializing data for write files.
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
		case HARD_RANDOM:
		{
			unsigned int n = static_cast<unsigned int>(fileSize / 8);
			randomArray(fileData, n);
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
	AppLib::write(msg);

	
	// Create files.
	AppLib::writeColor("\r\n\r\n Create files.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Index     Handle (hex)         Path\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	char path[MAX_PATH];
	BOOL createError = FALSE;
	// Source files.
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
		AppLib::write(msg);
		if (handle)
		{
			srcHandles.push_back(handle);
		}
		else
		{
			createError = TRUE;
			break;
		}
	}
	// Destination files.
	std::vector<HANDLE> dstHandles;
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
		AppLib::write(msg);
		if (handle)
		{
			dstHandles.push_back(handle);
		}
		else
		{
			createError = TRUE;
			break;
		}
	}
	// Both source and destination files created.
	AppLib::writeColorLine(APPCONST::TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (createError)
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return FILE_CREATE_FAILED;
	}


	// Write files.
	LARGE_INTEGER t1, t2, t3, t4;
	double timeUnitSeconds = 1.0 / hz.QuadPart;
	double fileMegabytes = fileSize * repeats / 1000000.0;
	double integralMegabytes = fileMegabytes * fileCount;

	std::vector<double> writeSpeed;
	double writeSpeedIntegral = 0.0;
	AppLib::writeColor("\r\n Write file performance.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	BOOL writeError = FALSE;
	
	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;

	}
	t2.QuadPart = t1.QuadPart;
	while (t2.QuadPart == t1.QuadPart)
	{
		if (!QueryPerformanceCounter(&t2))
		{
			if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
			return TIMER_FAILED;

		}
	}

	for (unsigned int i = 0; i < fileCount; i++)
	{
		HANDLE hFile = srcHandles[i];
		DWORD32 writeSize = fileSize;
		BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
		DWORD nNumberOfBytesWritten = 0;
		LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;

		QueryPerformanceCounter(&t3);
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
			}
			if (writeError) break;
		}
		QueryPerformanceCounter(&t4);

		if (writeError) break;
		double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
		double mbps = fileMegabytes / seconds;
		writeSpeed.push_back(mbps);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
		AppLib::write(msg);
	}
	
	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;
	}
	else
	{
		double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
		double mbps = integralMegabytes / seconds;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
		AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);
	}
	
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (writeError)
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return FILE_WRITE_FAILED;
	}


	// Copy files (re-positioning required after write for source files).
	std::vector<double> copySpeed;
	double copySpeedIntegral = 0.0;
	AppLib::writeColor("\r\n Copy file performance.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	BOOL copyError = FALSE;

	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;

	}
	t2.QuadPart = t1.QuadPart;
	while (t2.QuadPart == t1.QuadPart)
	{
		if (!QueryPerformanceCounter(&t2))
		{
			if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
			return TIMER_FAILED;

		}
	}

	for (unsigned int i = 0; i < fileCount; i++)
	{
		// Read one source file.
		HANDLE hFile = srcHandles[i];
		if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
		{
			copyError = TRUE;
			break;
		}
		DWORD32 readSize = fileSize;
		BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
		DWORD nNumberOfBytesRead = 0;
		LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
		
		QueryPerformanceCounter(&t3);
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
			}
			if (copyError) break;
		}
		if (copyError) break;
		// Write one destination file.
		hFile = dstHandles[i];
		DWORD32 writeSize = fileSize;
		lpBuffer = reinterpret_cast<BYTE*>(fileData);
		DWORD nNumberOfBytesWritten = 0;
		LPDWORD lpNumberOfBytesWritten = &nNumberOfBytesWritten;
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
			}
			if (copyError) break;
		}
		QueryPerformanceCounter(&t4);

		// Copy one file (one source and one destination file) done.
		if (copyError) break;
		double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
		double mbps = fileMegabytes / seconds;
		copySpeed.push_back(mbps);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
		AppLib::write(msg);
	}

	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;
	}
	else
	{
		double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
		double mbps = integralMegabytes / seconds;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
		AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);
	}

	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (copyError)
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return FILE_COPY_FAILED;
	}


	// Read files (re-positioning required after read at copy phase for source files).
	std::vector<double> readSpeed;
	double readSpeedIntegral = 0.0;
	AppLib::writeColor("\r\n Read file performance.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Index     MBPS\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	BOOL readError = FALSE;

	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;

	}
	t2.QuadPart = t1.QuadPart;
	while (t2.QuadPart == t1.QuadPart)
	{
		if (!QueryPerformanceCounter(&t2))
		{
			if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
			return TIMER_FAILED;

		}
	}

	for (unsigned int i = 0; i < fileCount; i++)
	{
		HANDLE hFile = srcHandles[i];
		if (SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) != 0)
		{
			readError = TRUE;
			break;
		}
		DWORD32 readSize = fileSize;
		BYTE* lpBuffer = reinterpret_cast<BYTE*>(fileData);
		DWORD nNumberOfBytesRead = 0;
		LPDWORD lpNumberOfBytesRead = &nNumberOfBytesRead;
		
		QueryPerformanceCounter(&t3);
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
			}
			if (readError) break;
		}
		QueryPerformanceCounter(&t4);

		if (readError) break;
		double seconds = (t4.QuadPart - t3.QuadPart) * timeUnitSeconds;
		double mbps = fileMegabytes / seconds;
		readSpeed.push_back(mbps);
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%.3f\r\n", i, mbps);
		AppLib::write(msg);
	}

	if (!QueryPerformanceCounter(&t1))
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return TIMER_FAILED;
	}
	else
	{
		double seconds = (t1.QuadPart - t2.QuadPart) * timeUnitSeconds;
		double mbps = integralMegabytes / seconds;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " Integral  %.3f\r\n", mbps);
		AppLib::writeColor(msg, APPCONST::DUMP_ADDRESS_COLOR);
	}

	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (readError)
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return FILE_READ_FAILED;
	}


	// Close and delete files.
	AppLib::writeColor("\r\n Delete files.\r\n", APPCONST::VALUE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	AppLib::writeColor(" Index     Path\r\n", APPCONST::TABLE_COLOR);
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	BOOL deleteError = FALSE;
	// Delete source files.
	for (unsigned int i = 0; i < fileCount; i++)
	{
		snprintf(path, MAX_PATH, "%s%s%08X%s", nameSrcPath, nameSrc, i, nameExt);
		if (!CloseHandle(srcHandles[i])) deleteError = TRUE;
		if (!DeleteFile(path)) deleteError = TRUE;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
		AppLib::write(msg);
	}
	// Delete destination files.
	for (unsigned int i = 0; i < fileCount; i++)
	{
		snprintf(path, MAX_PATH, "%s%s%08X%s", nameDstPath, nameDst, i, nameExt);
		if (!CloseHandle(dstHandles[i])) deleteError = TRUE;
		if (!DeleteFile(path)) deleteError = TRUE;
		snprintf(msg, APPCONST::MAX_TEXT_STRING, " %-10d%s\r\n", i, path);
		AppLib::write(msg);
	}
	// Delete source and destination files done.
	AppLib::writeColorLine(APPCONST::HALF_TABLE_WIDTH, APPCONST::TABLE_COLOR);
	if (deleteError)
	{
		if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
		return FILE_DELETE_FAILED;
	}


	// Write performance measurement statistics.
	AppLib::writeColor("\r\nPerformance Statistics.\r\n", APPCONST::TABLE_COLOR);
	writeStatistics(msg, "\r\nREAD   ", readSpeed);
	writeStatistics(msg, "\r\nWRITE  ", writeSpeed);
	writeStatistics(msg, "\r\nCOPY   ", copySpeed);
	AppLib::write("\r\n");


	// Done.
	status = FALSE;
	if (fileData) status = VirtualFree(fileData, 0, MEM_RELEASE);
	if (!status) return MEMORY_RELEASE_FAILED;
	return NO_ERRORS;
}