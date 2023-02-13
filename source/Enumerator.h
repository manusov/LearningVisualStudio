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
#include "EnumRes.h"
#include "Global.h"

UINT EnumerateSystem(LPTSTR pBase, UINT64 pMax, 
	PGROUPSORT sControl, UINT sControlLength, 
	PGROUPSORT rsControl, UINT rsControlLength, 
	PRESOURCESORT trControl, UINT trControlLength);
LPTSTR GetDeviceStringProperty(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop);
LPTSTR GetDeviceDescription(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo);

#endif  // ENUMERATOR_H

