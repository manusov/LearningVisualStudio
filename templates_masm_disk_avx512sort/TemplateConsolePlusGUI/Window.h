/*
Header of GUI window class.
*/

#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "Windows.h"

class Window
{
public:
    BOOL create();
    BOOL destroy();
    BOOL update();
private:
    BOOL WndCreate(HINSTANCE hInstance, int nCmdShow);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static HWND hWnd;
    static const char* const MSG_HELLO;
    static const char* const szClassName;
    static const char* const szWindowName;
};

#endif  // WINDOW_H
