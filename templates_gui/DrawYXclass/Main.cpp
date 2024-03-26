/*

Template for C++ GUI application.
Draw function Y=F(X).

*/

#include <windows.h>
#include "AppGuiLib.h"

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    AppGuiLib::testWindow1();
    Sleep(400);
    AppGuiLib::testWindow2();
    return 0;
}



