/*
Platform components tests and benchmarks. 
Experiments with differrent styles.
Main module.
*/

#include "Shell.h"
#include "Window.h"
#include "Task_01.h"
#include "Task_02.h"

int main(int argc, char** argv)
{
	Shell shell;
	//
	// Task_01 task;    // Timer precision test: GetSystemTimeAsFileTime() vs QueryPerformanceCounter().
	Task_02 task;       // GUI window create (after wait time), wait user close window. For MODAL window.
	//
	return shell.execute(argc, argv, task);
}
