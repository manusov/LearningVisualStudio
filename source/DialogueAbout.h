/* ----------------------------------------------------------------------------------------
Support "About" modal dialogue window.
TODO. Make this as class.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef DIALOGUEABOUT_H
#define DIALOGUEABOUT_H

#include <windows.h>
#include <windowsx.h>
#include <vector>
#include "TreeModel.h"
#include "resource.h"
#include "Main.h"

#define ABOUT_FONT_HEIGHT    16
#define ABOUT_YBASE1         38
#define ABOUT_YBASE2         100
#define ABOUT_YADD1          19
#define ABOUT_YADD2          19
#define ABOUT_YBOTTOM        47
#define ABOUT_ICONX          118
#define ABOUT_ICONY          2
#define ABOUT_ICONDX         32
#define ABOUT_ICONDY         32

#define ABOUT_FULL_SIZE_1    15
#define ABOUT_CLICK_START_1  8
#define ABOUT_CLICK_SIZE_1   7

#define ABOUT_FULL_SIZE_2    14
#define ABOUT_CLICK_START_2  -1
#define ABOUT_CLICK_SIZE_2   -1

#define ABOUT_FULL_SIZE_3    14
#define ABOUT_CLICK_START_3  0
#define ABOUT_CLICK_SIZE_3   14

#define ABOUT_FULL_SIZE_4    15
#define ABOUT_CLICK_START_4  0
#define ABOUT_CLICK_SIZE_4   15

#define ABOUT_CLICK_COLOR    0x00FF0000

typedef struct CLICKSTRING {
	LPCSTR stringPtr;
	int fullSize;
	int clickStart;
	int clickSize;
	int xmin;
	int xmax;
	int ymin;
	int ymax;
} *PCLICKSTRING;

void setTreeModel(TreeModel* tm);
INT_PTR DlgProc(HWND, UINT, WPARAM, LPARAM);
void HelperClose(HWND hWnd);

#endif  // DIALOGUEABOUT_H