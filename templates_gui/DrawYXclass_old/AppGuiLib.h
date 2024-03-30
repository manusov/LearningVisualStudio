/*

Application library class header.
Class used for GUI support helpers functions.
UNDER CONSTRUCTION.

*/

#pragma once
#ifndef APPGUILIB_H
#define APPGUILIB_H

#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include "DrawYX.h"

class AppGuiLib
{
public:
	static void testWindow1();
	static void testWindow2();
private:
	static void helperWindow(WINDOW_PARMS &wP, FUNCTION_PARMS &fP);
};

#endif  // APPGUILIB_H