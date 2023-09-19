/*

Mass storage performance test.
Classes source file edition.
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/

Task parameters class implementation.
Class used for parse scenario file input.txt
and extract options settings variables.

*/

#include "TaskParms.h"

// Universal library of helpers functions.
void TaskParms::resetBeforeParse()                     // Reset options (structure with command line parameters values) to defaults.
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
void TaskParms::correctAfterParse()                    // Verify options structure (reserved).
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
                AppLib::writeColor("RDRAND instruction not supported by CPU or locked. Software RNG used.\r\n", APPCONST::ERROR_COLOR);
                p->optionData = SOFT_RANDOM;
            }
        }
    }
    // This place reserved for other checks.
}
BOOL TaskParms::parseOptions(char* p, size_t n)
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
DWORD TaskParms::parseCommandLine(int argc, char** argv)
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
void TaskParms::writeOptionReport()
{
    AppLib::writeColor(" Benchmark scenario options.\r\n", APPCONST::VALUE_COLOR);
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
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
                AppLib::storeCellMemorySize(msg, APPCONST::MAX_TEXT_STRING, a, 20);
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
        AppLib::writeColor(" ", APPCONST::TABLE_COLOR);
        AppLib::writeParmAndValue(p->name, msg, 12, APPCONST::GROUP_COLOR);
        p++;
    }
    AppLib::writeColorLine(APPCONST::SMALL_TABLE_WIDTH, APPCONST::TABLE_COLOR);
}
char* TaskParms::getOptionStatusString()
{
    return optionStatusString;
}
COMMAND_LINE_PARMS* TaskParms::getCommandLineParms()
{
    return &parms;
}
// Command line parameters parse control.
// Strings for command line options detect.
const char* TaskParms::keysAddress[] = { "mbps", "mbpsone", "mbpsmap", "iops", "iopsqueue", "iopsmap", nullptr };
const char* TaskParms::keysData[] = { "zeroes", "ones", "inc", "softrnd", "hardrnd", nullptr };
const char* TaskParms::keysHt[] = { "default", "off", "on", nullptr };
const char* TaskParms::keysNuma[] = { "unaware", "single", "optimal", "nonoptimal", nullptr };
const char* TaskParms::keysPgroups[] = { "unaware", "single", "all", nullptr };
// Pointer command line parameters structure.
COMMAND_LINE_PARMS TaskParms::parms;
// Option control list, used for command line parameters parsing (regular input).
const OPTION_ENTRY TaskParms::options[] = {
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
// Status string.
char TaskParms::optionStatusString[APPCONST::MAX_TEXT_STRING];
