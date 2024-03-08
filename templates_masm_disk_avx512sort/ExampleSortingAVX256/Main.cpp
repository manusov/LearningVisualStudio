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

Main module.
AVX2 optimization enabled for compiling this project. Properties \ C/C++ \ Create code \ ...

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
            // Get scenario options for sorting algorithm.
            COMMAND_LINE_PARMS* pCommandLineParms = TaskParms::getCommandLineParms();

            // Target operation: check platform features, required for execute all versions of algorithms.
            int opStatus = TaskWork::checkFeatures(pCommandLineParms);
            if (opStatus == NO_ERRORS)
            {
                // Target operation: execute sorting algorithm.
                switch (pCommandLineParms->optionScenario)
                {
                case TEST_1:
                    opStatus = TaskWork::runTask(pCommandLineParms);
                    break;
                default:
                    opStatus = INTERNAL_ERROR;
                    break;
                }
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




