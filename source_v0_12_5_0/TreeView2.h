/* ----------------------------------------------------------------------------------------
Class for create GUI window with tree visualization.
Second implementation, special thanks to:
https://learn.microsoft.com/en-us/windows/win32/controls/create-scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/controls/scroll-bars
https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-createwindowexa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TREEVIEW2_H
#define TREEVIEW2_H

#include "TreeView1.h"

class TreeView2 :
    public TreeView1
{
public:
    TreeView2();
    virtual ~TreeView2() override;
    // Window callback procedure for device manager window.
    virtual LRESULT CALLBACK AppViewer(HWND, UINT, WPARAM, LPARAM) override;
protected:
	// Helper for adjust horizontal scrolling parameters.
    // The horizontal scrolling range is defined by 
    // (bitmap_width) - (client_width). The current horizontal 
    // scroll value remains within the horizontal scrolling range.
	virtual void HelperAdjustScrollX(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
		int xNewSize, int& xMaxScroll, int& xMinScroll, int& xCurrentScroll) override;
	// Helper for adjust vertical scrolling parameters.
	// The vertical scrolling range is defined by 
	// (bitmap_height) - (client_height). The current vertical 
	// scroll value remains within the vertical scrolling range. 
	virtual void HelperAdjustScrollY(HWND hWnd, SCROLLINFO& scrollInfo, RECT& treeDimension,
		int yNewSize, int& yMaxScroll, int& yMinScroll, int& yCurrentScroll) override;
	// Helper for make horizontal scrolling by given signed offset.
	virtual void HelperMakeScrollX(HWND hWnd, SCROLLINFO& scrollInfo,
		int xMaxScroll, int& xCurrentScroll, BOOL& fScroll, int addX) override;
	// Helper for make vertical scrolling by given signed offset.
	virtual void HelperMakeScrollY(HWND hWnd, SCROLLINFO& scrollInfo,
		int yMaxScroll, int& yCurrentScroll, BOOL& fScroll, int addY) override;
private:
    HWND CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight, int downY);
    HWND CreateAVerticalScrollBar(HWND hwndParent, int sbWidth, int upY, int downY);
};

#endif  // TREEVIEW2_H