/*

Mass storage performance sample.

More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
Special thanks for MASM sources connect method:
https://dennisbabkin.com/blog/?t=cpp-and-assembly-coding-in-x86-x64-assembly-language-in-visual-studio

Main source file.

*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Constants.h"
#include "AppLib.h"
#include "ParmLib.h"
#include "Task.h"        // Application-specific target operation.

int main(int argc, char** argv)
{
	using namespace std;
	int status = 0;
	bool fileMode = true;
	bool screenMode = true;
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
	ParmLib::resetBeforeParse();  // Set default options values.
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
			char* inPtr = (char*)malloc(((size_t)(inSize) + 1));  // Reserve 1 byte for last 0.
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
					if (ParmLib::parseOptions(inPtr, (size_t)inSize))  // Parse input configuration file, set options values.
					{
						loadStatus = TRUE;
						loadString1 = "Input configuration file parsed OK.";
					}
					else
					{
						loadString1 = "Error parsing input file, default settings can be used for some options.";
						loadString2 = ParmLib::getStatusString();
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
	ParmLib::correctAfterParse();  // Correct options values.

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
			cout << endl;
			ParmLib::writeReport();
			cout << endl;
			

			COMMAND_LINE_PARMS* pCommandLineParms = ParmLib::getCommandLineParms();
			// int localStatus = Task::Run(nullptr);
			int localStatus = Task::Run(pCommandLineParms);    // Target operation.


			if (localStatus == 0)
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario OK.\r\n");
				AppLib::writeColor(msg, APPCONST::NO_ERROR_COLOR);
			}
			else
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nPerformance scenario FAILED (%d).\r\n", localStatus);
				AppLib::writeColor(msg, APPCONST::ERROR_COLOR);
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
