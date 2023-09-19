/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Class header for input parameters set and parse helpers library:
support display output and file report.
Realization important note.
For porting from command line to input configuration file,
create virtual command line by input file strings.

*/

#pragma once
#ifndef PARMLIB_H
#define PARMLIB_H

#include <windows.h>
#include <iostream>
#include "Constants.h"
#include "AppLib.h"

class ParmLib
{
public:
	// Unified methods.
	static void writeReport();
	static BOOL getStatus();
	static const char* const getStatusString();
	// Command line specific methods.
	static BOOL parseOptions(char* p, size_t n);
	static DWORD parseCommandLine(int argc, char** argv);  // Parse command line and extract parameters to structure.
	static COMMAND_LINE_PARMS* getCommandLineParms();      // Get pointer to structure with command line parameters values, variables.
	static const OPTION_ENTRY* getOptionsList();           // Get pointer to structures array with options descriptors, constants.
	static void resetBeforeParse();                        // Reset options (structure with command line parameters values) to defaults.
	static void correctAfterParse();                       // Verify options structure (reserved).
private:
	// Unified fields.
	static BOOL status;
	static char statusString[APPCONST::MAX_TEXT_STRING];
    // Options names strings.
	static const char* keysAddress[];
	static const char* keysData[];
	static const char* keysHt[];
	static const char* keysNuma[];
	static const char* keysPgroups[];
	// Common data.
	static const OPTION_ENTRY options[];
	static COMMAND_LINE_PARMS parms;
};

#endif  // PARMLIB_H
