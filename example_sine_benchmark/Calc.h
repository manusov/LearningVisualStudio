/*
Header of sine calculation benchmark functions class.
*/

#pragma once
#ifndef CALC_H
#define CALC_H

#include <windows.h>
#include <intrin.h>
#include <thread>
#include <vector>
#include "AppLib.h"
#include "Timer.h"

class Calc
{
public:
	Calc();
	int scenarioSingleThread(double& dummySum);
	int scenarioMultiThread(double& dummySum);
private:
	double sumHelper(double* p, size_t count);
};

#endif  // CALC_H


