/*
Class for timers precision test.
Unit test for Timer class.
Measure frequencies for:
OS Performance Counter, OS File Time counter, TSC.
*/

#include "Task_01.h"

int Task_01::execute(int argc, char** argv)
{
	Timer timer;
	return timer.test();
}
