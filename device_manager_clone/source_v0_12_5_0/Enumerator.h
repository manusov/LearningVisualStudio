/* ----------------------------------------------------------------------------------------
Devices enumerator.
Special thanks to:
Microsoft enumerator sample:
https://github.com/microsoft/Windows-driver-samples/tree/main/setup/devcon
---------------------------------------------------------------------------------------- */

#pragma once
#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <strsafe.h>
#include <vector>
#include <algorithm>
#include "Global.h"

UINT EnumerateSystem(LPTSTR pBase, UINT64 pMax, 
	PGROUPSORT sControl, UINT sControlLength, 
	PGROUPSORT rsControl, UINT rsControlLength, 
	PRESOURCESORT trControl, UINT trControlLength);
LPTSTR GetDeviceStringProperty(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop);
LPTSTR GetDeviceDescription(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo);

void EnumerateDeviceResourcesToTransitList(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo, LPCSTR devName,
	PRESOURCESORT trControl, UINT trControlLength);
void EnumerateTransitListToGroupList(LPTSTR& pBase, UINT64& pMax,
	PRESOURCESORT trControl, UINT trControlLength, PGROUPSORT rsControl, UINT rsControlLength);
BOOL DumpDeviceResources(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo,
	LPCSTR devName, PRESOURCESORT trControl);
BOOL DumpDeviceResourcesOfType(_In_ DEVINST DevInst, _In_ HMACHINE MachineHandle, _In_ LOG_CONF Config, _In_ RESOURCEID ReqResId,
	LPCSTR devName, PRESOURCESORT trControl);

#endif  // ENUMERATOR_H

