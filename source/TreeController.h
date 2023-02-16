/* ----------------------------------------------------------------------------------------
Class for build constant tree as linked list of nodes descriptors.
At application debug, this class used as tree builder for data emulation.
At real system information show, this is parent class.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TREECONTROLLER_H
#define TREECONTROLLER_H

#include "TreeModel.h"

class TreeController
{
public:
	TreeController();
	virtual ~TreeController();
	// Don't use constructor and destructor for build and release tree, because
	// dynamical rebuild with model change and partial changes can be required.
	void SetAndInitModel(TreeModel* p);
	virtual PTREENODE BuildTree(UINT mode);
	virtual void ReleaseTree();
protected:
	static PTREENODE pTreeBase;
	static PTREENODE pTreeBaseBack;
	static TreeModel* pModel;
private:
	static LPCSTR EMULATED_NAME_1;
	static LPCSTR EMULATED_NAME_2;
};

#endif  // TREECONTROLLER_H

