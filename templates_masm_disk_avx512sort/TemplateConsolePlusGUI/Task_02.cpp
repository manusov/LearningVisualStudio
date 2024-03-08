/*
Class for GUI window test.
*/

#include "Task_02.h"

//  This useable for MODAL window.
int Task_02::execute(int argc, char** argv)
{
	Window window;
	AppLib::writeColor("Waiting 5 seconds...", APPCONST::GROUP_COLOR);
	Sleep(5000);
	AppLib::writeColor("\r\nCreate window...", APPCONST::GROUP_COLOR);
	if (window.create())
	{
		AppLib::writeColor("\r\nWindow closed OK.", APPCONST::GROUP_COLOR);
		return 0;
	}
	else
	{
		AppLib::writeColor("\r\nWindow failed.", APPCONST::ERROR_COLOR);
		return -1;
	}
}

/*
//  This useable for NON-MODAL window.
int Task_02::execute(int argc, char** argv)
{
	int status = -1;
	Window window;
	if (window.create())
	{
		Sleep(1500);
		if (window.destroy())
		{
			status = 0;
		}
	}
	return status;
}
*/

