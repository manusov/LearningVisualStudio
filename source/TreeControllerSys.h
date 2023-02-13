/* ----------------------------------------------------------------------------------------
Class for scan system information by WinAPI.
This class builds system information tree as linked list of nodes descriptors.
At application debug, this class not used.
At real system information show, this is child class of TreeController class.
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef TREECONTROLLERSYS_H
#define TREECONTROLLERSYS_H

#include "Enumerator.h"
#include "TreeController.h"

class TreeControllerSys :
    public TreeController
{
public:
    TreeControllerSys();
    ~TreeControllerSys();
    PTREENODE BuildTree(UINT mode);
    void ReleaseTree();
private:
    static LPCSTR MAIN_SYSTEM_NAME;
    static int MAIN_SYSTEM_ICON_INDEX;
    static GROUPSORT sortControl[];
    static const UINT SORT_CONTROL_LENGTH;
    static GROUPSORT resourceControl[];
    static const UINT RESOURCE_CONTROL_LENGTH;
    static RESOURCESORT transitControl[];
    static const UINT TRANSIT_CONTROL_LENGTH;
    static LPSTR pEnumBase;
};

#endif  // TREECONTROLLERSYS_H