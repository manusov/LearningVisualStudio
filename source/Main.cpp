/* ----------------------------------------------------------------------------------------

ENGINEERING SAMPLE.
Clone of Windows Device Manager.
Main source file (Head Block).

Project context and settings:

  1) Add resources for application icon and tree nodes icons.
  2) Required differentiation application icon for ia32 and x64,
     both for Debug and Release,
     Build events \ Event before build \ command line:
	 set icon for ia32 command line:
	   "copy Icons_App\app32.ico Icons_App\app.ico"
	 set icon for x64 command line:
       "copy Icons_App\app64.ico Icons_App\app.ico"

This settings both for x86 and x64, both for Debug and Release:
  3) Additional \ Char set \ Multi-byte encoding.
  4) Linker \ System \ Windows.
  5) Linker \ Input \ Additional dependences \ Add library: setupapi.lib.
  6) Linker \ Input \ Additional dependences \ Add library: Comctl32.lib.

This settings both for x86 and x64, for Release only:
  7) C++ \ Code build \ Runtime library \ Multi-thread (MT).

Special thanks to:

Microsoft enumerator sample:
https://github.com/microsoft/Windows-driver-samples/tree/main/setup/devcon

Windows GUI examples:
https://learn.microsoft.com/en-us/windows/win32/controls/scroll-a-bitmap-in-scroll-bars
https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta
https://www.piter.com/page/filesforbooks
https://storage.piter.com/support_insale/files_list.php?code=978546901361

Icons library and editor:
http://ru.softoware.org/apps/download-open-icon-library-for-windows.html
https://sourceforge.net/projects/openiconlibrary/
http://www.sibcode.com/junior-icon-editor/

TODO.

BUG1: Mouse move tracking when _NEW_GUI (at Main.h) enabled.
      Questions about offsetY.
	  InvalidateRect must add toolY and subtract (toolY + statusY) instead rect = NULL?
	  InvalidateRect must be called conditionally for prevent flip.

BUG2: Screen flip when tree open-close, required correction of double bufferring logic.
      Caused by multi-invalidations instead final-only invalidation?
	  Occurred when scroll bar show status changed?

BUG3: Scroll bar below status bar, required swap, use SB_CTL instead SB_HORZ, SB_VERT?
      Fix this bug when refactoring by build class TreeViewDebug?

---------------------------------------------------------------------------------------- */

#include <windows.h>
#include "KWnd.h"
#include "TreeModel.h"
#include "TreeView.h"
#include "TreeViewDebug.h"
#include "TreeController.h"
#include "TreeControllerExt.h"
#include "TreeControllerSys.h"
#include "resource.h"
#include "Main.h"

TreeModel* pModel = NULL;
TreeView* pView = NULL;
TreeController* pController = NULL;

LRESULT CALLBACK TransitWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return pView->AppViewer(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Initialize common controls.
	int status = 0;
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_WIN95_CLASSES;
	if (InitCommonControlsEx(&icc))
	{
		// Initialize application and system context.
		pModel = new TreeModel();

#ifdef _NEW_GUI
		pView = new TreeViewDebug();
#else
		pView = new TreeView();
#endif

#ifdef _EMULATED_MODE
#ifdef _EXT_EMULATION
		pController = new TreeControllerExt();
#else
		pController = new TreeController();
#endif
#else
		pController = new TreeControllerSys();
#endif

		if (pModel && pView && pController)
		{
			if (pModel->getInitStatus())
			{
				pController->SetAndInitModel(pModel);
				pView->SetAndInitModel(pModel);
				// Set tree base coordinates.
				POINT treeBase = { X_BASE_TREE, Y_BASE_TREE };
				pModel->SetBase(treeBase);
				// Set tree data.
				PTREENODE tree = pController->BuildTree();
				pModel->SetTree(tree);
				if (tree)
				{
					// Write build name string.
					CHAR buildName[BUILD_NAME_MAX];
					_snprintf_s(buildName, BUILD_NAME_MAX, _TRUNCATE, "%s %s%s %s %s",
						ABOUT_TEXT_1, ABOUT_TEXT_2_1, ABOUT_TEXT_2_2, ABOUT_TEXT_2_3, ABOUT_TEXT_2_4);
					// Show application window = Device Manager tree.
					KWnd mainWnd((LPCSTR)buildName, hInstance, nCmdShow,
						TransitWndProc,
						MAKEINTRESOURCE(IDR_MAIN_MENU),
						590, 280, 800, 640,
						CS_HREDRAW | CS_VREDRAW,
						WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
						NULL);
					// Handling user events
					MSG msg;
					while (GetMessage(&msg, NULL, 0, 0))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else
				{
					status = 4;
				}
			}
			else
			{
				status = 3;
			}
		}
		else
		{
			status = 2;
		}
	}
	else
	{
		status = 1;
	}

	// Messages if errors.
	if (status)
	{
		LPCSTR s = "?";
		switch (status)
		{
		case 1:
			s = "Common controls initialization failed.";
			break;
		case 2:
			s = "Classes initialization failed.";
			break;
		case 3:
			s = "Resources initialization failed.";
			break;
		case 4:
			s = "Build system tree failed.";
			break;
		default:
			s = "Unknown error.";
			break;
		}
		MessageBox(NULL, s, "Application starting error.", MB_OK | MB_ICONERROR);
	}

	// Restore system context and exit application.
	if (pController)
	{
		pController->ReleaseTree();
		delete pController;
	}
	if (pView) delete pView;
	if (pModel) delete pModel;
	return status;
}
