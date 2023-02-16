/* ----------------------------------------------------------------------------------------
Class for create GUI window with tree visualization.
Second implementation, special thanks to:
https://learn.microsoft.com/en-us/windows/win32/controls/create-scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/controls/scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-createwindowexa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
---------------------------------------------------------------------------------------- */

#include "TreeView2.h"

TreeView2::TreeView2()
{
	// Reserved functionality.
}
TreeView2::~TreeView2()
{
	// Reserved functionality.
}

LRESULT CALLBACK TreeView2::AppViewer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// These variables are required by BeginPaint, EndPaint, BitBlt. 
	PAINTSTRUCT ps;              // Temporary storage for paint info  : ps.hdc = window, can be resized by user.
	static HDC hdcScreen;        // DC for entire screen              : hdcScreen = full screen.
	static HDC hdcScreenCompat;  // memory DC for screen              : hdcScreenCompat = bufferred copy of full screen.
	static HBITMAP hbmpCompat;   // bitmap handle to old DC 
	static BITMAP bmp;           // bitmap data structure 
	static BOOL fBlt;            // TRUE if BitBlt occurred 
	static BOOL fScroll;         // TRUE if scrolling occurred 
	static BOOL fSize;           // TRUE if fBlt & WM_SIZE 
	// This variable used for horizontal and vertical scrolling both.
	SCROLLINFO si;               // Temporary storage for scroll info
	// These variables are required for horizontal scrolling. 
	static int xMinScroll;       // minimum horizontal scroll value 
	static int xCurrentScroll;   // current horizontal scroll value 
	static int xMaxScroll;       // maximum horizontal scroll value 
	// These variables are required for vertical scrolling. 
	static int yMinScroll;       // minimum vertical scroll value 
	static int yCurrentScroll;   // current vertical scroll value 
	static int yMaxScroll;       // maximum vertical scroll value 
	// This variable are required for adjust scrolling by visualized tree size.
	static RECT treeDimension;
	// This variables are required for nodes selection by TAB key, use background color.
	static BOOL fTab;            // TRUE if selection mode activated
	static PTREENODE openNode;   // This node opened if SPACE, Gray+, Gray- pressed at selection mode
	// This variables are required for text font and background brush
	static HFONT hFont;          // Font handle
	static HBRUSH bgndBrush;     // Brush handle for background
	// This variables are required for restore context
	static HGDIOBJ backupBmp;
	static HBRUSH backupBrush;
	// Application tool bar and status bar handles.
	static HWND hWndToolBar;
	static HWND hWndStatusBar;
	// Application tool bar and status bar Y-sizes for viewer settings adjust.
	static int toolY;
	static int statusY;
	// Application main menu handle.
	static HMENU hMenu;
	// Information type selector
	static int selector;
	// Horizontal and vertical scroll bars handles and geometry, added at TreeView2
	// instead acroll bars as window properties.
	static HWND hScrollBarH;
	static HWND hScrollBarV;
	static int sbHeight;
	static int sbWidth;

    // Window callback procedure entry point.
    switch (uMsg)
    {
	case WM_CREATE:
	{
		// Pre-clear window area invalidation request.
		ClearInvalidation();
		// Fill window with background color, important for prevent blinking when window resize.
		bgndBrush = CreateSolidBrush(BACKGROUND_BRUSH);
		SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)bgndBrush);
		// Create font.
		hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, TEXT("System monospace"));
		// Create a normal DC and a memory DC for the entire 
        // screen. The normal DC provides a snapshot of the 
        // screen contents. The memory DC keeps a copy of this 
        // snapshot in the associated bitmap. 
		hdcScreen = CreateDC("DISPLAY", (PCTSTR)NULL, (PCTSTR)NULL, (CONST DEVMODE*) NULL);
		hdcScreenCompat = CreateCompatibleDC(hdcScreen);
		// Retrieve the metrics for the bitmap associated with the 
		// regular device context. 
		bmp.bmBitsPixel =
			(BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
		bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
		bmp.bmWidth = GetDeviceCaps(hdcScreen, HORZRES);
		bmp.bmHeight = GetDeviceCaps(hdcScreen, VERTRES);
		// The width must be byte-aligned. 
		bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;
		// Create a bitmap for the compatible DC. 
		hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
			bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID*) NULL);
		// Select the bitmap for the compatible DC.
		backupBmp = SelectObject(hdcScreenCompat, hbmpCompat);
		// Select the brush for the compatible DC.
		backupBrush = (HBRUSH)SelectObject(hdcScreenCompat, bgndBrush);
		// Create application tool bar and status bar.
		hWndToolBar = InitToolBar(hWnd);
		hWndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, " Ready...", hWnd, ID_STATUSBAR);
		// Adjust tool bar and status bar position and size.
		RECT r;
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
			GetWindowRect(hWndToolBar, &r);
			toolY = r.bottom - r.top;        // Note static variable toolY = 0 if tool bar not initialized.
		}
		if (hWndStatusBar)
		{
			SendMessage(hWndStatusBar, WM_SIZE, wParam, lParam);
			GetWindowRect(hWndStatusBar, &r);
			statusY = r.bottom - r.top;      // Note static variable statusY = 0 if status bar not initialized.
		}
		// Initialize the flags. 
		fBlt = FALSE;
		fScroll = FALSE;
		fSize = FALSE;
		// Initialize the horizontal scrolling variables. 
		xMinScroll = 0;
		xCurrentScroll = 0;
		xMaxScroll = 0;
		// Initialize the vertical scrolling variables. 
		yMinScroll = 0;
		yCurrentScroll = 0;
		yMaxScroll = 0;
		fSize = TRUE;
		// Initialize pointer for open items by SPACE, Gray+, Gray- keys.
		openNode = pModel->GetTrees()[selector];
		// This for compatibility with MSDN example.
		fBlt = TRUE;
		// Get application main menu descriptor and default setup menu items.
		hMenu = GetMenu(hWnd);
		if (hMenu)
		{
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, IDM_VIEW_DEVICES, MF_BYCOMMAND);
		}
		// Default setup tool bar items.
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, TRUE);
		}
		// Create horizontal and vertical scroll bars, added at TreeView2
		// instead acroll bars as window properties.
		sbHeight = SCROLLBAR_HEIGHT;
		sbWidth = SCROLLBAR_WIDTH;
		hScrollBarH = CreateAHorizontalScrollBar(hWnd, sbHeight, statusY);
		hScrollBarV = CreateAVerticalScrollBar(hWnd, sbWidth, toolY, sbHeight + statusY);
	}
	break;

	case WM_PAINT:
	{
		// Open paint context.
		BeginPaint(hWnd, &ps);
		// Paint bufferred copy.
		// Logic for prevent status bar blinking and correct revisual selections when mouse cursor moved.
		RECT r;
		GetClientRect(hWnd, &r);
		int dy1 = r.bottom - r.top;
		int dy2 = ps.rcPaint.bottom;
		if (dy1 == dy2)
		{   // This nranch for repaint all window
			BitBlt(ps.hdc, 0, toolY, ps.rcPaint.right, ps.rcPaint.bottom - toolY - statusY - sbHeight,
				hdcScreenCompat, 0, 0, SRCCOPY);
		}
		else
		{   // This branch for repaint part of window, for example selections area near mouse cursor move.
			BitBlt(ps.hdc, 0, toolY, ps.rcPaint.right, ps.rcPaint.bottom,
				hdcScreenCompat, 0, 0, SRCCOPY);

		}
		// Close paint context.
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_SIZE:
	{
		ClearInvalidation();
		int xNewSize = GET_X_LPARAM(lParam) - sbWidth;
		int yNewSize = GET_Y_LPARAM(lParam) - toolY - statusY - sbHeight;
		// Construction from original MSDN source, inspect it.
		if (fBlt)
			fSize = TRUE;
		// The horizontal scrolling range is defined by 
		// (tree_width) - (client_width). The current horizontal 
		// scroll value remains within the horizontal scrolling range.
		HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
		// The vertical scrolling range is defined by 
		// (tree_height) - (client_height). The current vertical 
		// scroll value remains within the vertical scrolling range. 
		HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);
		BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
		int dy = 0;
		treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
			xCurrentScroll, yCurrentScroll, dy);
		// Adjust tool bar and status bar position and size when main window size changed.
		RECT r;
		if (hWndToolBar)
		{
			SendMessage(hWndToolBar, TB_AUTOSIZE, 0, 0);
			GetWindowRect(hWndToolBar, &r);
			toolY = r.bottom - r.top;
		}
		if (hWndStatusBar)
		{
			SendMessage(hWndStatusBar, WM_SIZE, wParam, lParam);
			GetWindowRect(hWndStatusBar, &r);
			statusY = r.bottom - r.top;
		}
		if (GetClientRect(hWnd, &r))
		{
			if (hScrollBarH)
			{
				SetWindowPos(hScrollBarH, NULL, 
					r.left, r.bottom - sbHeight - statusY, r.right, sbHeight, 
					SWP_SHOWWINDOW);
			}
			if (hScrollBarV)
			{
				SetWindowPos(hScrollBarV, NULL, 
					r.right - sbWidth, r.top + toolY, sbWidth, r.bottom - toolY - statusY, 
					SWP_SHOWWINDOW);
			}
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_LBUTTONUP:
	{
		ClearInvalidation();
		int mouseX = GET_X_LPARAM(lParam);
		int mouseY = GET_Y_LPARAM(lParam);
		PTREENODE p = pModel->GetTrees()[selector];
		POINT b = pModel->GetBase();

		RECT t = HelperRecursiveMouseClick(p, b, hWnd, hdcScreenCompat, fSize,
			openNode, fTab, bmp, hFont,
			mouseX, mouseY - toolY, xCurrentScroll, yCurrentScroll, selector);
		if (t.right && t.bottom)
		{
			treeDimension = t;
		}

		RECT r = { 0,0,0,0 };
		if (GetClientRect(hWnd, &r))
		{
			int xNewSize = r.right - r.left - sbWidth;
			int yNewSize = r.bottom - r.top - toolY - statusY - sbHeight;
			// Construction from original MSDN source, inspect it.
			if (fBlt)
				fSize = TRUE;
			// The horizontal scrolling range is defined by 
			// (tree_width) - (client_width). The current horizontal 
			// scroll value remains within the horizontal scrolling range.
			HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
			// The vertical scrolling range is defined by 
			// (tree_height) - (client_height). The current vertical 
			// scroll value remains within the vertical scrolling range. 
			HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);
		}

		// Restore Open/Close icon lighting by mouse cursor after node clicked.
		HelperRecursiveMouseMove(p, hWnd, hdcScreenCompat, fSize, TRUE,
			mouseX, mouseY, xCurrentScroll, yCurrentScroll, toolY);
		MakeInvalidation(hWnd);
	}
	break;

	case WM_MOUSEMOVE:
	{
		ClearInvalidation();
		HelperRecursiveMouseMove(pModel->GetTrees()[selector], hWnd, hdcScreenCompat, fSize, FALSE,
			GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), xCurrentScroll, yCurrentScroll, toolY);
		MakeInvalidation(hWnd);
	}
	break;

	case WM_HSCROLL:
	{
		ClearInvalidation();
		int addX = 0;
		int scrollType = LOWORD(wParam);
		int value = HIWORD(wParam);
		switch (scrollType)
		{
		case SB_PAGEUP:
			addX = -50;  // User clicked the scroll bar shaft left the scroll box. 
			break;
		case SB_PAGEDOWN:
			addX = 50;  // User clicked the scroll bar shaft right the scroll box. 
			break;
		case SB_LINEUP:
			addX = -3;  // User clicked the left arrow. 
			break;
		case SB_LINEDOWN:
			addX = 3;   // User clicked the right arrow. 
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			addX = value - xCurrentScroll;  // User dragged the scroll box.
			break;
		default:
			addX = 0;
			break;
		}
		if (addX != 0)
		{
			HelperMakeScrollX(hScrollBarH, si, xMaxScroll, xCurrentScroll, fScroll, addX);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_VSCROLL:
	{
		ClearInvalidation();
		int addY = 0;
		int scrollType = LOWORD(wParam);
		int value = HIWORD(wParam);
		switch (scrollType)
		{
		case SB_PAGEUP:
			addY = -50;  // User clicked the scroll bar shaft above the scroll box. 
			break;
		case SB_PAGEDOWN:
			addY = 50;   // User clicked the scroll bar shaft below the scroll box. 
			break;
		case SB_LINEUP:
			addY = -3;   // User clicked the top arrow. 
			break;
		case SB_LINEDOWN:
			addY = 3;    // User clicked the bottom arrow. 
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			addY = value - yCurrentScroll;  // User dragged the scroll box.
			break;
		default:
			addY = 0;
			break;
		}
		if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_MOUSEWHEEL:
	{
		ClearInvalidation();
		int addY = -(short)HIWORD(wParam) / WHEEL_DELTA * 30;
		if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			int dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_KEYDOWN:
	{
		ClearInvalidation();
		int addX = 0;
		int addY = 0;
		int dy = 0;

		switch (wParam)
		{
		case VK_LEFT:
			addX = -3;
			break;
		case VK_RIGHT:
			addX = 3;
			break;
		case VK_UP:
			if (fTab)
			{
				openNode = HelperRecursiveMarkNode(FALSE, selector);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			}
			else
			{
				addY = -3;
			}
			break;
		case VK_DOWN:
			if (fTab)
			{
				openNode = HelperRecursiveMarkNode(TRUE, selector);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			}
			else
			{
				addY = 3;
			}
			break;
		case VK_PRIOR:
			addY = -50;
			break;
		case VK_NEXT:
			addY = 50;
			break;
		case VK_HOME:
			addY = -yCurrentScroll;
			break;
		case VK_END:
			addY = yMaxScroll;
			break;
		case VK_TAB:
			fTab = ~fTab;
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			fSize = TRUE;
			SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
			break;

		case VK_ADD:
		case VK_SUBTRACT:
		case VK_SPACE:
			if (fTab && openNode && (openNode->openable))
			{
				openNode->opened = ~(openNode->opened);
				HelperMarkedClosedChilds(openNode, openNode, fTab);
				BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
				dy = 0;
				treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
					xCurrentScroll, yCurrentScroll, dy);
				fSize = TRUE;
				SetInvalidation();  // InvalidateRect(hWnd, NULL, false);

				RECT r = { 0,0,0,0 };
				if (GetClientRect(hWnd, &r))
				{
					int xNewSize = r.right - r.left;
					int yNewSize = r.bottom - r.top;
					if (fBlt)
						fSize = TRUE;
					HelperAdjustScrollX(hScrollBarH, si, treeDimension, xNewSize, xMaxScroll, xMinScroll, xCurrentScroll);
					HelperAdjustScrollY(hScrollBarV, si, treeDimension, yNewSize, yMaxScroll, yMinScroll, yCurrentScroll);
				}
			}
			break;
		default:
			break;
		}
		if (addX != 0)
		{
			HelperMakeScrollX(hScrollBarH, si, xMaxScroll, xCurrentScroll, fScroll, addX);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		else if (addY != 0)
		{
			HelperMakeScrollY(hScrollBarV, si, yMaxScroll, yCurrentScroll, fScroll, addY);
			BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, NULL, 0, 0, PATCOPY);  // This for blank background
			dy = 0;
			HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_COMMAND:
	{
		ClearInvalidation();
		RECT r;
		GetClientRect(hWnd, &r);
		int sx = r.right - r.left;
		int sy = r.bottom - r.top;
		DWORD sysx = (sy << 16) + sx;
		int dy = 0;
		switch (LOWORD(wParam))
		{
		case ID_TB_ABOUT:
		case IDM_HELP_ABOUT:
			setTreeModel(pModel);
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)DlgProc, 0);
			break;

		case ID_TB_EXIT:
		case IDM_FILE_EXIT:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;

		case ID_TB_DEVICES:
			selector = 0;
			treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			if (hMenu)
			{
				CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
					IDM_VIEW_RESOURCES, IDM_VIEW_DEVICES, MF_BYCOMMAND);
			}
			break;

		case ID_TB_RESOURCES:
			selector = 1;
			treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			if (hMenu)
			{
				CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
					IDM_VIEW_RESOURCES, IDM_VIEW_RESOURCES, MF_BYCOMMAND);
			}
			break;

		case IDM_VIEW_DEVICES:
			selector = 0;
			treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, LOWORD(wParam), MF_BYCOMMAND);
			if (hWndToolBar)
			{
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, TRUE);
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_RESOURCES, FALSE);
			}
			break;

		case IDM_VIEW_RESOURCES:
			selector = 1;
			treeDimension = HelperRecursiveDrawTree(pModel->GetTrees()[selector], pModel->GetBase(), fTab, hdcScreenCompat, hFont,
				xCurrentScroll, yCurrentScroll, dy);
			SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, sysx);
			invalidationRequest = TRUE;
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), IDM_VIEW_DEVICES,
				IDM_VIEW_RESOURCES, LOWORD(wParam), MF_BYCOMMAND);
			if (hWndToolBar)
			{
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_DEVICES, FALSE);
				SendMessage(hWndToolBar, TB_CHECKBUTTON, ID_TB_RESOURCES, TRUE);
			}
			break;

		default:
			break;
		}
		MakeInvalidation(hWnd);
	}
	break;

	case WM_NOTIFY:
	{
		LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT)lParam;
		if (lpTTT->hdr.code == TTN_NEEDTEXT)
		{
			lpTTT->hinst = GetModuleHandle(NULL);
			lpTTT->lpszText = MAKEINTRESOURCE(lpTTT->hdr.idFrom);
		}
	}
	break;

    case WM_DESTROY:
    {
		if (backupBrush) SelectObject(hdcScreenCompat, backupBrush);
        if (backupBmp) SelectObject(hdcScreenCompat, backupBmp);
        if (hbmpCompat) DeleteObject(hbmpCompat);
        if (hdcScreenCompat) DeleteDC(hdcScreenCompat);
        if (hdcScreen) DeleteDC(hdcScreen);
        if (hFont) DeleteObject(hFont);
		if (bgndBrush) DeleteObject(bgndBrush);
        PostQuitMessage(0);
    }
    break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Helpers.
// Helper for adjust horizontal scrolling parameters.
// The horizontal scrolling range is defined by 
// (bitmap_width) - (client_width). The current horizontal 
// scroll value remains within the horizontal scrolling range.
void TreeView2::HelperAdjustScrollX(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
	int xNewSize, int& xMaxScroll, int& xMinScroll, int& xCurrentScroll)
{
	int tempSize = treeDimension.right - treeDimension.left;     // added
	xMaxScroll = max(tempSize - xNewSize, 0);                    // max(bmp.bmWidth - xNewSize, 0);
	xCurrentScroll = min(xCurrentScroll, xMaxScroll);
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
	scrollInfo.nMin = xMinScroll;
	scrollInfo.nMax = tempSize;                                  // bmp.bmWidth;
	scrollInfo.nPage = xNewSize;
	scrollInfo.nPos = xCurrentScroll;
	SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
}
// Helper for adjust vertical scrolling parameters.
// The vertical scrolling range is defined by 
// (bitmap_height) - (client_height). The current vertical 
// scroll value remains within the vertical scrolling range. 
void TreeView2::HelperAdjustScrollY(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
	int yNewSize, int& yMaxScroll, int& yMinScroll, int& yCurrentScroll)
{
	int tempSize = treeDimension.bottom - treeDimension.top;     // added
	yMaxScroll = max(tempSize - yNewSize, 0);                    // max(bmp.bmHeight - yNewSize, 0);
	yCurrentScroll = min(yCurrentScroll, yMaxScroll);
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
	scrollInfo.nMin = yMinScroll;
	scrollInfo.nMax = tempSize;                                  // bmp.bmHeight;
	scrollInfo.nPage = yNewSize;
	scrollInfo.nPos = yCurrentScroll;
	SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
}
// Helper for make horizontal scrolling by given signed offset.
void TreeView2::HelperMakeScrollX(HWND hWnd, SCROLLINFO& scrollInfo,
	int xMaxScroll, int& xCurrentScroll, BOOL& fScroll, int addX)
{
	int xNewPos = xCurrentScroll + addX;
	// New position must be between 0 and the screen width. 
	xNewPos = max(0, xNewPos);
	xNewPos = min(xMaxScroll, xNewPos);
	// If the current position does not change, do not scroll.
	if (xNewPos != xCurrentScroll)
	{
		// Set the scroll flag to TRUE. 
		fScroll = TRUE;
		// Update the current scroll position. 
		xCurrentScroll = xNewPos;
		// Update the scroll bar position.
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_POS | SIF_DISABLENOSCROLL;
		scrollInfo.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
		// Request for all window repaint
		SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
	}
}
// Helper for make vertical scrolling by given signed offset.
void TreeView2::HelperMakeScrollY(HWND hWnd, SCROLLINFO& scrollInfo,
	int yMaxScroll, int& yCurrentScroll, BOOL& fScroll, int addY)
{
	int yNewPos = yCurrentScroll + addY;
	// New position must be between 0 and the screen height. 
	yNewPos = max(0, yNewPos);
	yNewPos = min(yMaxScroll, yNewPos);
	// If the current position does not change, do not scroll.
	if (yNewPos != yCurrentScroll)
	{
		// Set the scroll flag to TRUE. 
		fScroll = TRUE;
		// Update the current scroll position. 
		yCurrentScroll = yNewPos;
		// Update the scroll bar position.
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_POS | SIF_DISABLENOSCROLL;
		scrollInfo.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_CTL, &scrollInfo, TRUE);
		// Request for all window repaint
		SetInvalidation();  // InvalidateRect(hWnd, NULL, false);
	}
}
// Description:
//   Creates a horizontal scroll bar along the bottom of the parent 
//   window's area.
// Parameters:
//   hwndParent = handle to the parent window.
//   sbHeight   = height, in pixels, of the scroll bar.
//   downY      = size reserved below scroll bar for status bar.
// Returns:
//   The handle to the scroll bar.
HWND TreeView2::CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight, int downY)
{
    RECT rect;
    // Get the dimensions of the parent window's client area;
    if (!GetClientRect(hwndParent, &rect))
        return NULL;
    // Create the scroll bar.
    return (CreateWindowEx(
        0,                               // no extended styles 
        "SCROLLBAR",                     // scroll bar control class 
        (PTSTR)NULL,                     // no window text 
        WS_CHILD | WS_VISIBLE            // window styles  
        | SBS_HORZ,                      // horizontal scroll bar style 
        rect.left,                       // horizontal position 
        rect.bottom - sbHeight - downY,  // vertical position 
        rect.right,                      // width of the scroll bar 
        sbHeight,                        // height of the scroll bar
        hwndParent,                      // handle to main window 
        (HMENU)NULL,                     // no menu 
        GetModuleHandle(NULL),           // instance owning this window 
        (PVOID)NULL                      // pointer not needed 
    ));
}
// Description:
//   Creates a vertical scroll bar along the right of the parent 
//   window's area.
// Parameters:
//   hwndParent = handle to the parent window.
//   sbWidth    = width, in pixels, of the scroll bar.
//   upY        = size reserved above scroll bar for tool bar.
//   downY      = size reserved below scroll bar for horizontal scroll bar and status bar.
// Returns:
//   The handle to the scroll bar.
HWND TreeView2::CreateAVerticalScrollBar(HWND hwndParent, int sbWidth, int upY, int downY)
{
	RECT rect;
	// Get the dimensions of the parent window's client area;
	if (!GetClientRect(hwndParent, &rect))
		return NULL;
	// Create the scroll bar.
	return (CreateWindowEx(
		0,                          // no extended styles 
		"SCROLLBAR",                // scroll bar control class 
		(PTSTR)NULL,                // no window text 
		WS_CHILD | WS_VISIBLE       // window styles  
		| SBS_VERT,                 // vertical scroll bar style 
		rect.right - sbWidth,       // horizontal position 
		rect.top + upY,             // vertical position 
		sbWidth,                    // width of the scroll bar 
		rect.bottom - upY - downY,  // height of the scroll bar
		hwndParent,                 // handle to main window 
		(HMENU)NULL,                // no menu 
		GetModuleHandle(NULL),      // instance owning this window 
		(PVOID)NULL                 // pointer not needed 
	));
}