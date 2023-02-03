/* ----------------------------------------------------------------------------------------
Support "About" modal dialogue window.
TODO. Make this as class.
---------------------------------------------------------------------------------------- */

#include "DialogueAbout.h"

TreeModel* appModel;
HCURSOR hCursorAbout;
HFONT hFontAbout;
HICON hIconAbout;

LPCSTR szAbout      = "Program info";
LPCSTR szClickable1 = "More at GitHub.";
LPCSTR szClickable2 = "Developed with";
LPCSTR szClickable3 = "Visual Studio.";
LPCSTR szClickable4 = "Flat Assembler.";
LPCSTR szLink1      = "https://github.com/manusov?tab=repositories";
LPCSTR szLink2      = "https://visualstudio.microsoft.com/ru/downloads/";
LPCSTR szLink3      = "https://flatassembler.net/";
LPCSTR szError      = "Shell error.";

CLICKSTRING cs[4] = {
	{ szClickable1, ABOUT_FULL_SIZE_1, ABOUT_CLICK_START_1, ABOUT_CLICK_SIZE_1, -1, -1, -1, -1 },
	{ szClickable2, ABOUT_FULL_SIZE_2, ABOUT_CLICK_START_2, ABOUT_CLICK_SIZE_2, -1, -1, -1, -1 },
	{ szClickable3, ABOUT_FULL_SIZE_3, ABOUT_CLICK_START_3, ABOUT_CLICK_SIZE_3, -1, -1, -1, -1 },
	{ szClickable4, ABOUT_FULL_SIZE_4, ABOUT_CLICK_START_4, ABOUT_CLICK_SIZE_4, -1, -1, -1, -1 }
};

const int N1 = 3;
const int N2 = sizeof(cs) / sizeof(CLICKSTRING);
LPCSTR webLinks[] = { szLink1, NULL, szLink2, szLink3 };

void setTreeModel(TreeModel* tm)
{
	appModel = tm;
}

INT_PTR DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR status = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
	{
		SetWindowText(hWnd, szAbout);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)(appModel->GetIconHandleByIndex(ID_APP)));
		hCursorAbout = LoadCursor(NULL, IDC_HAND);
		hFontAbout = CreateFont(ABOUT_FONT_HEIGHT, 0, 0, 0, FW_DONTCARE, 0, 0, 0,
			DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, NULL);
		hIconAbout = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BOOKS),
			IMAGE_ICON, ABOUT_ICONDX, ABOUT_ICONDX, LR_DEFAULTCOLOR);
		status = TRUE;
	}
	break;

	case WM_CLOSE:
	{
		HelperClose(hWnd);
		status = FALSE;
	}
	break;

	case WM_COMMAND:
	{
		if (wParam == ID_ABOUT_OK)
		{
			HelperClose(hWnd);
		}
		status = FALSE;
	}
	break;

	case WM_LBUTTONDOWN:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < N2; i++)
		{
			int xmin = cs[i].xmin;
			int xmax = cs[i].xmax;
			int ymin = cs[i].ymin;
			int ymax = cs[i].ymax;
			if ((xmin >= 0) && (xmax >= 0) && (ymin >= 0) && (ymax >= 0) &&
				(x > xmin) && (x < xmax) && (y > ymin) && (y < ymax))
			{
				if (hCursorAbout) SetCursor(hCursorAbout);
				LPCSTR s = webLinks[i];
				if (s)
				{
					HINSTANCE h = ShellExecute(NULL, 0, s, NULL, NULL, SW_NORMAL);
					if ((INT_PTR)h <= 32)
					{
						MessageBox(NULL, szError, NULL, MB_ICONERROR);
					}
				}
			}
		}
		status = FALSE;
	}
	break;

	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		for (int i = 0; i < N2; i++)
		{
			int xmin = cs[i].xmin;
			int xmax = cs[i].xmax;
			int ymin = cs[i].ymin;
			int ymax = cs[i].ymax;
			if ((xmin >= 0) && (xmax >= 0) && (ymin >= 0) && (ymax >= 0) && 
				(x > xmin) && (x < xmax) && (y > ymin) && (y < ymax))
			{
				if(hCursorAbout) SetCursor(hCursorAbout);
			}
		}
		status = FALSE;
	}
	break;

	case WM_PAINT:
	{
		status = TRUE;
		PAINTSTRUCT ps;
		RECT r1, r2;
		HFONT hFontBack;
		TEXTMETRIC tm;
		SIZE s1; // , s2;
		HDC hDC = BeginPaint(hWnd, &ps);
		if (hDC && hCursorAbout && hFontAbout && hIconAbout)
		{
			if (GetClientRect(hWnd, &r1))
			{
				r2.left = r1.left;
				r2.right = r1.right;
				r2.top = r1.top;
				r2.bottom = r1.bottom - ABOUT_YBOTTOM;
				int bkMode = SetBkMode(hDC, TRANSPARENT);
				if (bkMode)
				{
					if (FillRect(hDC, &r2, (HBRUSH)(COLOR_WINDOW + 1)))
					{
						if (DrawIcon(hDC, ABOUT_ICONX, ABOUT_ICONY, hIconAbout))
						{
							hFontBack = (HFONT)SelectObject(hDC, hFontAbout);
							if (hFontBack)
							{
								if (GetTextMetrics(hDC, &tm))
								{
									r2.top = ABOUT_YBASE1;
									r2.bottom = ABOUT_YBASE1 + ABOUT_YADD1;
									CHAR buffer[BUILD_NAME_MAX];
									for (int i = 0; i < N1; i++)
									{
										switch (i)
										{
										case 0:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s", ABOUT_TEXT_1);
											break;
										case 1:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s%s %s %s",
												ABOUT_TEXT_2_1, ABOUT_TEXT_2_2, ABOUT_TEXT_2_3, ABOUT_TEXT_2_4);
											break;
										case 2:
											_snprintf_s(buffer, BUILD_NAME_MAX, _TRUNCATE, "%s", ABOUT_TEXT_3);
											break;
										}
										if (!DrawText(hDC, buffer, -1, &r2, DT_CENTER + DT_NOPREFIX)) break;
										r2.top += ABOUT_YADD1;
										r2.bottom += ABOUT_YADD1;
									}

									int y = ABOUT_YBASE2;
									r2.top = r1.top;
									r2.bottom = r1.bottom;
									for (int i = 0; i < N2; i++)
									{
										cs[i].ymin = y;
										cs[i].ymax = y + tm.tmHeight;
										if (!GetTextExtentPoint32(hDC, cs[i].stringPtr, cs[i].fullSize, &s1)) break;  // full string width, prefix part + clickable part
										r2.left = (r1.right - r1.left - s1.cx) / 2;
										r2.right = r1.right;
										r2.top = y;
										r2.bottom = y + ABOUT_YADD2;
										if (cs[i].fullSize != cs[i].clickSize)
										{
											if (!DrawText(hDC, cs[i].stringPtr, cs[i].clickStart, &r2, DT_LEFT + DT_NOPREFIX)) break;
										}
										if (cs[i].clickStart >= 0)
										{
											cs[i].xmax = r2.left + s1.cx;
											if (!GetTextExtentPoint32(hDC, cs[i].stringPtr, cs[i].clickStart, &s1)) break;  // prefix part width, left from clickable
											r2.left += s1.cx;
											cs[i].xmin = r2.left;
											COLORREF backColor = SetTextColor(hDC, ABOUT_CLICK_COLOR);
											if (!DrawText(hDC, cs[i].stringPtr + cs[i].clickStart, cs[i].clickSize, &r2, DT_LEFT + DT_NOPREFIX)) break;
											SetTextColor(hDC, backColor);
										}
										y += ABOUT_YADD2;
									}
								}
								else
								{
									status = FALSE;
								}
								SelectObject(hDC, hFontBack);
							}
							else
							{
								status = FALSE;
							}
						}
						else
						{
							status = FALSE;
						}
					}
					else
					{
						status = FALSE;
					}
					SetBkMode(hDC, bkMode);
				}
				else
				{
					status = FALSE;
				}
			}
			else
			{
				status = FALSE;
			}
			EndPaint(hWnd, &ps);
		}
		else
		{
			status = FALSE;
		}
	}
	break;

	default:
	{
	status = FALSE;
	}
	break;

	}
	return status;
}

// Helpers

void HelperClose(HWND hWnd)
{
	if (hFontAbout) DeleteObject(hFontAbout);
	EndDialog(hWnd, 0);
}