/*
Platform components tests. Experiments with differrent styles.
Main module.
*/

#include "Shell.h"
#include "Task_01.h"


int main(int argc, char** argv)
{
	Shell shell;
	Task_01 task;
	return shell.execute(argc, argv, task);
}
