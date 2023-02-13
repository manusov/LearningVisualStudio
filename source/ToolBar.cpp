/* ----------------------------------------------------------------------------------------
Support GUI window element: tool bar (ToolBar), located above main window.
---------------------------------------------------------------------------------------- */

#include "ToolBar.h"

HWND InitToolBar(HWND hWnd)
{
	HWND hToolBar;
	int btnID[NUM_BUTTONS] = { ID_TB_DEVICES, ID_TB_RESOURCES, ID_SEP, ID_TB_ABOUT, ID_TB_EXIT };
	int btnStyle[NUM_BUTTONS] = { TBSTYLE_CHECKGROUP, TBSTYLE_CHECKGROUP, TBSTYLE_SEP, TBSTYLE_BUTTON, TBSTYLE_BUTTON };
	TBBUTTON tbb[NUM_BUTTONS];
	memset(tbb, 0, sizeof(tbb));

	for (int i = 0; i < NUM_BUTTONS; ++i) 
	{
		if (btnID[i] == ID_SEP)
			tbb[i].iBitmap = SEPARATOR_WIDTH;
		else  tbb[i].iBitmap = i;

		tbb[i].idCommand = btnID[i];
		tbb[i].fsState = TBSTATE_ENABLED;
		tbb[i].fsStyle = btnStyle[i];
	}

	hToolBar = CreateToolbarEx(hWnd,
		WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_TOOLTIPS,
		ID_TOOLBAR, NUM_BUTTONS, GetModuleHandle(NULL), IDR_TOOLBAR,
		tbb, NUM_BUTTONS, 0, 0, 0, 0, sizeof(TBBUTTON));
	return hToolBar;
}

