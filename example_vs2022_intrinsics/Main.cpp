/*

Series of experiments for learn Microsoft Visual Studio and build library
of classes for system information and performance analysing application.

Example with intrinsics.
Special thanks to:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html

1) Use "Release" (not "Debug") builds for performance tests.
2) Use "__debugbreak();" macro (INT3, code 0CCh) for set breakpoints at "Release" builds.
3) Enable AVX512 at "C/C++" \ "Code generation" \ "Enable extended instructions set...",
   support registers ZMM16-ZMM31 can depend on this option.

*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "AppLib.h"
#include "Task.h"

int main(int argc, char** argv)
{
	using namespace std;
	int status = 0;
	bool fileMode = true;
	bool screenMode = true;
	char msg[APPCONST::MAX_TEXT_STRING];
	const char* outName = APPCONST::DEFAULT_OUT_NAME;
	char* outPtr = (char*)malloc(APPCONST::MAX_OUT_FILE_SIZE);
	char* const backOutPtr = outPtr;
	cout << APPCONST::MSG_STARTING << endl;
	if (backOutPtr)
	{
		if (AppLib::initConsole(screenMode, fileMode, backOutPtr, APPCONST::MAX_OUT_FILE_SIZE))
		{
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s (%s).\r\n", APPCONST::MSG_APPLICATION, APPCONST::MSG_BUILD);
			AppLib::write(msg);
			
			int localStatus = task();
			if (localStatus == 0)
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nOK.\r\n");
				AppLib::writeColor(msg, APPCONST::NO_ERROR_COLOR);
			}
			else
			{
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "\r\nFAILED (%d).\r\n", localStatus);
				AppLib::writeColor(msg, APPCONST::ERROR_COLOR);
			}

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
						snprintf(msg, APPCONST::MAX_TEXT_STRING, "%d bytes saved to output file.", n);
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
