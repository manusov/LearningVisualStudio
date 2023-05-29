/*
Unit test for Timer class.
Measure frequencies for:
OS Performance Counter, OS File Time counter, TSC.
*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "AppLib.h"
#include "Calc.h"

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
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "%s (%s).\r\n\r\n", APPCONST::MSG_APPLICATION, APPCONST::MSG_BUILD);
			AppLib::write(msg);
			
			Calc calc;
			double dummySum = 0;
			int localStatus = calc.scenarioSingleThread(dummySum);
			snprintf(msg, APPCONST::MAX_TEXT_STRING, "Dummy sum = %.3f.\r\n", dummySum);
			AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
			if (localStatus == 0)
			{
				AppLib::writeColor("\r\n", APPCONST::TABLE_COLOR);
				localStatus = calc.scenarioMultiThread(dummySum);
				snprintf(msg, APPCONST::MAX_TEXT_STRING, "Dummy sum = %.3f.\r\n", dummySum);
				AppLib::writeColor(msg, APPCONST::VALUE_COLOR);
			}

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
