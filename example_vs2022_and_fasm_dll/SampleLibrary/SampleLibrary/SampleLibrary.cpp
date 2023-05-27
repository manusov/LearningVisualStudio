#include <windows.h>
#include <stdio.h>

#ifdef _WIN64
#define DLLNAME "SAMPLELIBRARY64.DLL"
#else
#define DLLNAME "SAMPLELIBRARY32.DLL"
#endif

typedef void(__stdcall* MSGPROC)(LPCSTR);
typedef void(__stdcall* DBGPROC)();

int main(void)
{
    HINSTANCE hinstDll;
    MSGPROC msgProc;
    DBGPROC dbgProc;
    
    hinstDll = LoadLibrary(TEXT(DLLNAME));
    if (hinstDll != NULL)
    {
        printf("Library loaded OK.\n");

        // message function example
        msgProc = (MSGPROC)GetProcAddress(hinstDll, "BoxOutput");
        if (msgProc != NULL)
        {
            printf("Message function address get OK.\n");
            (msgProc)("This string send to DLL function.");
        }
        else
        {
            printf("Message function not found.\n");
        }

        // function under debug example
        dbgProc = (DBGPROC)GetProcAddress(hinstDll, "DebugCode");
        if (dbgProc != NULL)
        {
            printf("Debug function address get OK.\n");
            // can set breakpoint here
            // (dbgProc)();
        }
        else
        {
            printf("Debug function not found.\n");
        }

        // end of examples
    }
    else
    {
        printf("Library not found.\n");
    }
    return 0;
}
