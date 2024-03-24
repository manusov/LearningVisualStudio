/*

Sorting algorithm debug.
Quick sort and sorting networks with AVX512 instructions.
Starts with variant for double precision numbers.

This project based on Intel sources:
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

Task parameters class header.
Class used for parse scenario file input.txt
and extract options settings variables.

*/

#pragma once
#ifndef TASKPARMS_H
#define TASKPARMS_H

#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <immintrin.h>
#include "Global.h"
#include "AppLib.h"

class TaskParms
{
public:
    // Support command line options, input configuration and output report files.
    static void resetBeforeParse();
    static void correctAfterParse();
    static BOOL parseOptions(char* p, size_t n);
    static DWORD parseCommandLine(int argc, char** argv);
    static void writeOptionReport();
    static char* getOptionStatusString();
    static COMMAND_LINE_PARMS* getCommandLineParms();
private:
    // Command line parameters parse control.
    // Strings for command line options detect.
    static const char* keysScenario[];
    static const char* keysData[];
    static const char* keysGui[];
    // Pointer command line parameters structure.
    static COMMAND_LINE_PARMS parms;
    // Option control list, used for command line parameters parsing (regular input).
    static const OPTION_ENTRY options[];
    // Status string.
    static char optionStatusString[APPCONST::MAX_TEXT_STRING];
};

#endif  // TASKPARMS_H

