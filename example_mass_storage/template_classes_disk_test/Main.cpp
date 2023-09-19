/*

Mass storage performance test.
Classes source file edition.
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Main module.

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
12) Change queue strategy: add next request after one request executed. Don't wait all requests done, this prevents parallel works.
13) Support procedures: void resetBeforeParse(), void correctAfterParse(). Check all options.
14) Error message and exit if wrong options (instead default scenario).
15) Close file group than measure integral time (?)
16) Support files and blocks size > 2GB/4GB, use 64-bit variables (Microsoft DWORD64) for all offsets and sizes.
17) Optimize by remove duplicated fragments by functions (subroutines).
18) Extended verification by compare read data and write data.
19) Wait key option: wait after actions.
20) Report option: file and/or console.
21) Support all other options.
22) Re-verify all options settings support.
23) Compare with NIOBench for some types of disks.
24) If divergent source, use Main.cpp module, Main.h header (?) and classes: TaskParms.cpp, TaskParms.h, TaskWork.cpp, TaskWork.h.
    Projects names: template_single_file, template_classes. This variant is single source file (Main.cpp).

*/

#include "Global.h"
#include "AppLib.h"
#include "TaskParms.h"
#include "TaskWork.h"

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
    TaskParms::resetBeforeParse();                                  // Set default options values.
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
                    if (TaskParms::parseOptions(inPtr, (size_t)inSize))          // Parse input configuration file, set options values.
                    {
                        loadStatus = TRUE;
                        loadString1 = "Input configuration file parsed OK.";
                    }
                    else
                    {
                        loadString1 = "Error parsing input file, default settings can be used for some options.";
                        loadString2 = TaskParms::getOptionStatusString();
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
        if (AppLib::initConsole(screenMode, fileMode, backOutPtr, APPCONST::MAX_OUT_FILE_SIZE))
        {
            snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s (%s).\r\n", APPCONST::MSG_APPLICATION, APPCONST::MSG_BUILD);
            AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
            WORD color = APPCONST::NO_ERROR_COLOR;
            if (!loadStatus)
            {
                color = APPCONST::ERROR_COLOR;
            }
            if (loadString1)
            {
                AppLib::writeColor(loadString1, color);
                AppLib::write("\r\n");
            }
            if (loadString2)
            {
                AppLib::writeColor(loadString2, color);
                AppLib::write("\r\n");
            }
            AppLib::write("\r\n");
            TaskParms::writeOptionReport();    // Show options values.
            AppLib::write("\r\n");
            TaskParms::correctAfterParse();    // Verify and correct (if required) options values.
            // Get scenario options for mass storage performance test.
            COMMAND_LINE_PARMS* pCommandLineParms = TaskParms::getCommandLineParms();
            // Target operation: execute mass storage performance test.
            int opStatus;
            switch (pCommandLineParms->optionAddress)
            {
            case MBPS:
                opStatus = TaskWork::runTaskMBPS(pCommandLineParms);
                break;
            case MBPS_ONE_FILE:
                opStatus = TaskWork::runTaskMBPSoneFile(pCommandLineParms);
                break;
            case MBPS_MAP:
                opStatus = TaskWork::runTaskMBPSmap(pCommandLineParms);
                break;
            case IOPS:
                opStatus = TaskWork::runTaskIOPS(pCommandLineParms);
                break;
            case IOPS_QUEUED:
                opStatus = TaskWork::runTaskIOPSqueued(pCommandLineParms);
                break;
            case IOPS_MAP:
                opStatus = TaskWork::runTaskIOPSmap(pCommandLineParms);
                break;
            default:
                opStatus = INTERNAL_ERROR;
                break;
            }
            // Interpreting status.
            if (opStatus == NO_ERRORS)
            {
                snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario OK.\r\n");
                AppLib::writeColor(msg, APPCONST::NO_ERROR_COLOR);
            }
            else
            {
                snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario error %d", opStatus);
                AppLib::writeColor(msg, APPCONST::ERROR_COLOR);
                if (opStatus < STATUS_MAX)
                {
                    const char* returnName = APPCONST::RETURN_NAMES[opStatus];
                    snprintf(msg, APPCONST::MAX_TEXT_STRING, " = %s.\r\n", returnName);
                    AppLib::writeColor(msg, APPCONST::ERROR_COLOR);
                    DWORD osError = GetLastError();
                    if (osError)
                    {
                        AppLib::storeSystemErrorName(msg, APPCONST::MAX_TEXT_STRING, osError);
                        AppLib::writeColor(msg, APPCONST::ERROR_COLOR);
                    }
                }
                else
                {
                    AppLib::writeColor(" = Unknown error.\r\n", APPCONST::ERROR_COLOR);
                }
                AppLib::write("\r\n");
            }
            // Save report buffer to output file, if file mode enabled.
            if (fileMode)
            {
                ofstream out;
                out.open(outName, ios::binary | ios::out);
                if (out.is_open())
                {
                    out.write(backOutPtr, AppLib::getCurrentOutSize());
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




