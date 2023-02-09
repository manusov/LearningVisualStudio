/* ----------------------------------------------------------------------------------------
Support GUI window element: tool bar (ToolBar), located above main window.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#define ID_TOOLBAR       501
#define NUM_BUTTONS      2
#define SEPARATOR_WIDTH  10

HWND InitToolBar(HWND hWnd);


#endif  // TOOLBAR_H
