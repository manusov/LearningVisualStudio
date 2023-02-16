/* ----------------------------------------------------------------------------------------
Class for tree data model storage:
icons, text strings, structures with tree nodes descriptors.
Include storage for visualized model.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <windows.h>
#include "resource.h"
#include "Global.h"

class TreeModel
{
public:
	TreeModel();
	virtual ~TreeModel();
	// Service functions for tree builder.
	UINT GetIconIdByIndex(int);
	HICON GetIconHandleByIndex(int);
	LPCSTR GetIconNameByIndex(int);
	// Getter and setter for pointer to tree.
	PTREENODE* GetTrees();
	void SetTrees(PTREENODE*);
	// Getter and setter for pointer to tree base coordinates.
	POINT GetBase();
	void SetBase(POINT);
	// Getters for emulated constant tree parameters
	PTREECHILDS getNodeChilds();
	int getNodeChildsCount();
	// Getter for initialization status
	BOOL getInitStatus();
private:
	static const char* ICON_NAMES[];
	static const int ICON_IDS[];
	static const int ICON_COUNT;
	static HICON iconHandles[];
	static PTREENODE* trees;
	static POINT base;
	// Pointer to array with child nodes per each category,
	// for emulated constant tree parameters.
	static TREECHILDS nodeChilds[];
	static int NODE_CHILDS_COUNT;
	// Resources initialization status
	static BOOL initStatus;
};

#endif  // TREEMODEL_H
