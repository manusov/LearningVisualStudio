/*
Header for target functionality test routine.
*/

#pragma once
#ifndef TASK_H
#define TASK_H

#include <windows.h>
#include "AppLib.h"
#include "Timer.h"
#include "Performance.h"

int task();
BOOL detectAvx512();

#endif  // TASK_H
