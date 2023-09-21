/*

GPUID.
Instruction for build under Visual Studio 2022.
1) Create C++ console application project.
2) Load this source file (gpuid.cpp) as project main file.
3) Project properties \ Linker \ Input, add library Cfgmgr32.lib.
   Both for x86 and x64 versions. Both for Release and Debug versions.
4) Project properties \ C++ \ Code generation, set Runtime library = MT.
   Both for x86 and x64 versions. For Release, NOT FOR Debug versions.

Original example with GUID_DEVINTERFACE_VOLUME from MSDN.
https://docs.microsoft.com/en-us/windows/win32/api/cfgmgr32/nf-cfgmgr32-cm_get_device_interface_lista
Between calling CM_Get_Device_Interface_List_Size to get the size of the list and calling
CM_Get_Device_Interface_List to get the list, a new device interface can be added to the system
causing the size returned to no longer be valid.  Callers should be robust to that condition and
retry getting the size and the list if CM_Get_Device_Interface_List returns CR_BUFFER_SMALL.

Requests list:
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmthk/ne-d3dkmthk-_kmtqueryadapterinfotype

Enums required text names:
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dukmdt/ne-d3dukmdt-_d3dddiformat
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dukmdt/ne-d3dukmdt-_d3dddi_video_signal_scanline_ordering
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dukmdt/ne-d3dukmdt-_d3dddi_rotation
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmthk/ne-d3dkmthk-_qai_driverversion
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmthk/ne-d3dkmthk-_d3dkmdt_mode_pruning_reason
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmthk/ne-d3dkmthk-_d3dkmt_miracast_driver_type

Additional examples:
https://github.com/wuye9036/SalviaRenderer/blob/master/salvia_d3d_sw_driver/src/kmd_adapter.cpp

 TODO.

1)   At function printCurrentDisplayMode(), yet not all parameters visualized:
     Flags at D3DKMDT_DISPLAYMODE_FLAGS and D3DKMDT_MODE_PRUNING_REASON.
     Differrent depend on OS version.
     Why return zeroes?
     Some .Flags is [in], setup input parameters by "group..." functions.
     Why "Mode list" rejected?
2)   Total 107 requests, yet supported only part of requests, extend
     union GPU_DATA and GPU_INFO_SEQUENCE list[].
     Undocumented destination data format for some requests.
3)   Select adapter if >1 adapters.
     Select display per each adapter if >1 displays.
     Select video mode if >1 mode per modes list.
4)   Check PCIe and memory bandwidth. Why return zeroes?
5)+  Make subroutines for reduce code size, for example snprintf + parmValue.
6)   Write "n/a" if text string is empty, starts with byte 0.
7)+  Check IDE warnings.
8)+  Make this utility as single source file?
9)   Decode PCI Vendors and Devices, add data base.

*/

#pragma once
#ifndef TASK_50_H
#define TASK_50_H

#include <iostream>
#include <iomanip>
#include <windows.h>
#include <cfgmgr32.h>
#include <d3dkmthk.h>
#include <initguid.h>
#include <ntddvdeo.h>
#include "AppLib.h"
#include "Task.h"

union GPU_DATA
{
    D3DKMT_UMDFILENAMEINFO                      umdFileNameInfo;
    D3DKMT_OPENGLINFO                           openGlInfo;
    D3DKMT_SEGMENTSIZEINFO                      segmentSizeInfo;
    GUID                                        adapterGuid;
    D3DKMT_FLIPQUEUEINFO                        flipQueueInfo;
    D3DKMT_ADAPTERADDRESS                       adapterAddress;
    D3DKMT_WORKINGSETINFO                       workingSetInfo;
    D3DKMT_ADAPTERREGISTRYINFO                  adapterRegistryInfo;
    D3DKMT_CURRENTDISPLAYMODE                   currentDisplayMode;
    D3DKMT_DISPLAYMODE                          displayMode;
    BOOL                                        driverUpdateStatus;
    D3DKMT_VIRTUALADDRESSINFO                   virtualAddressInfo;
    D3DKMT_DRIVERVERSION                        driverVersion;
    D3DKMT_ADAPTERTYPE                          adapterType;
    D3DKMT_OUTPUTDUPLCONTEXTSCOUNT              outputDup;
    D3DKMT_WDDM_1_2_CAPS                        wddm12caps;
    D3DKMT_UMD_DRIVER_VERSION                   umdDriverVersion;
    D3DKMT_DIRECTFLIP_SUPPORT                   directFlipSupport;
    D3DKMT_MULTIPLANEOVERLAY_SUPPORT            mpoSupport;
    D3DKMT_DLIST_DRIVER_NAME                    dlistDriverName;
    D3DKMT_WDDM_1_3_CAPS                        wddm13caps;
    D3DKMT_MULTIPLANEOVERLAY_HUD_SUPPORT        mpoHudSupport;
    D3DKMT_WDDM_2_0_CAPS                        wddm20caps;
    //  Unknown format for KMTQAITYPE_NODEMETADATA  nodeMetaData;
    D3DKMT_CPDRIVERNAME                         cpDriverName;
    D3DKMT_XBOX                                 isXbox;
    D3DKMT_INDEPENDENTFLIP_SUPPORT              indFlipSupport;
    D3DKMT_MIRACASTCOMPANIONDRIVERNAME          miraCastName;
    D3DKMT_PHYSICAL_ADAPTER_COUNT               physicalAdapterCount;
    D3DKMT_QUERY_DEVICE_IDS                     physicalAdapterIds;
    // Unknown format for KMTQAITYPE_DRIVERCAPS_EXT
    D3DKMT_MIRACAST_DRIVER_TYPE                 miraCastType;
    D3DKMT_QUERY_GPUMMU_CAPS                    queryMmuCaps;
    D3DKMT_MULTIPLANEOVERLAY_DECODE_SUPPORT     mpoDecodeSupport;
    // Unknown format for KMTQAITYPE_QUERY_HW_PROTECTION_TEARDOWN_COUNT
    D3DKMT_ISBADDRIVERFORHWPROTECTIONDISABLED   isBadDriverDisabled;
    D3DKMT_MULTIPLANEOVERLAY_SECONDARY_SUPPORT  mpoSecondarySupport;
    D3DKMT_INDEPENDENTFLIP_SECONDARY_SUPPORT    indFlipSecSupport;
    D3DKMT_PANELFITTER_SUPPORT                  panelFitterSupport;
    // Unknown format for KMTQAITYPE_PHYSICALADAPTERPNPKEY
    //  D3DKMT_QUERY_PHYSICAL_ADAPTER_PNP_KEY       adapterPnpKey;
    D3DKMT_SEGMENTGROUPSIZEINFO                 segmentGroupSize;
    D3DKMT_MPO3DDI_SUPPORT                      mpo3ddiSupport;
    D3DKMT_HWDRM_SUPPORT                        hwDrmSupport;
    D3DKMT_MPOKERNELCAPS_SUPPORT                mpoKernelCaps;
    // Wrong result, input required ?
    //  D3DKMT_MULTIPLANEOVERLAY_STRETCH_SUPPORT    mpoStretchSupport;
    //  D3DKMT_GET_DEVICE_VIDPN_OWNERSHIP_INFO      getVidpnOvnership;
    // Yet not used, input required.
    //  D3DDDI_QUERYREGISTRY_INFO                   queryRegistryInfo;
    D3DKMT_KMD_DRIVER_VERSION                   kmdDriverVersion;
    // Wrong result, input required ?
    //  D3DKMT_BLOCKLIST_INFO                       blockListInfo;
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERGUID_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERADDRESS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_CHECKDRIVERUPDATESTATUS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_DRIVERVERSION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERTYPE_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_WDDM_1_2_CAPS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_WDDM_1_3_CAPS_RENDER
    D3DKMT_QUERY_ADAPTER_UNIQUE_GUID            adapterUniqueGuid;
    D3DKMT_NODE_PERFDATA                        nodePerfData;
    D3DKMT_ADAPTER_PERFDATA                     adapterPerfData;
    D3DKMT_ADAPTER_PERFDATACAPS                 adapterPerfDataCaps;
    D3DKMT_GPUVERSION                           gpuVersion;
    D3DKMT_DRIVER_DESCRIPTION                   driverDescription;
    // Yet not used, unknown output format: KMTQAITYPE_DRIVER_DESCRIPTION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_SCANOUT_CAPS
    // Yet not used, unknown output format: KMTQAITYPE_DISPLAY_UMDRIVERNAME
    // Yet not used, unknown output format: KMTQAITYPE_PARAVIRTUALIZATION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_SERVICENAME
    D3DKMT_WDDM_2_7_CAPS                        wddm27caps;
    // Yet not used, unknown output format: KMTQAITYPE_TRACKEDWORKLOAD_SUPPORT
    // Yet not used, ID not defined: D3DKMT_HYBRID_DLIST_DLL_SUPPORT  hybridDlist;
    // Yet not used, unknown output format: KMTQAITYPE_DISPLAY_CAPS
    // Yet not used, ID not defined: D3DKMT_WDDM_2_9_CAPS
    // Yet not used, ID not defined: D3DKMT_CROSSADAPTERRESOURCE_SUPPORT
    // Yet not used, ID not defined: D3DKMT_WDDM_3_0_CAPS
    // Yet not used, ID not defined: KMTQAITYPE_WSAUMDIMAGENAME
    // Yet not used, ID not defined: KMTQAITYPE_VGPUINTERFACEID
    // Yet not used, ID not defined: KMTQAITYPE_WDDM_3_1_CAPS
};

struct GPU_INFO_SEQUENCE
{
    BOOL(*groupHandler)();
    BOOL(*printHandler)();
    KMTQUERYADAPTERINFOTYPE  type;
    UINT                     privateDriverDataSize;
};

class Task_50 : public Task
{
public:
    Task_50();
    ~Task_50();
    int execute(int argc, char** argv);

private:
    PWSTR DeviceInterfaceList;
    ULONG DeviceInterfaceListLength;
    NTSTATUS nt;
    D3DKMT_OPENADAPTERFROMDEVICENAME open;
    D3DKMT_QUERYADAPTERINFO query;
    D3DKMT_HANDLE adapterHandle;

    static GPU_DATA gpuData;
    static const char* BUILD_STRING;
    static const int PARM_WIDTH;
    static const int VALUE_WIDTH;
    static const int MAX_DRIVER_DESCRIPTION_LENGTH;
    static const char* STRINGS_D3DDDIFORMAT[];
    static const char* STRINGS_D3DDDI_VIDEO_SIGNAL_SCANLINE_ORDERING[];
    static const char* STRINGS_D3DDDI_ROTATION[];
    static const char* STRINGS_QAI_DRIVERVERSION[];
    static const char* STRINGS_D3DKMDT_MODE_PRUNING_REASON[];
    static const char* STRINGS_D3DKMT_MIRACAST_DRIVER_TYPE[];
    static const GPU_INFO_SEQUENCE list[];

    BOOL sequencerGpuInfoInit();
    void sequencerGpuInfoPrint();

    static BOOL groupUmdFileNameInfoDX9();
    static BOOL groupUmdFileNameInfoDX10();
    static BOOL groupUmdFileNameInfoDX11();
    static BOOL groupUmdFileNameInfoDX12();
    static BOOL printUmdFileName();
    static BOOL groupOpenGlInfo();
    static BOOL printOpenGlInfo();
    static BOOL groupSegmentSizeInfo();
    static BOOL printSegmentSizeInfo();
    static BOOL groupAdapterGuid();
    static BOOL printAdapterGuid();
    static BOOL groupFlipQueueInfo();
    static BOOL printFlipQueueInfo();
    static BOOL groupAdapterAddress();
    static BOOL printAdapterAddress();
    static BOOL groupWorkingSetInfo();
    static BOOL printWorkingSetInfo();
    static BOOL groupAdapterRegistryInfo();
    static BOOL printAdapterRegistryInfo();
    static BOOL groupCurrentDisplayMode();
    static BOOL printCurrentDisplayMode();
    static BOOL groupModeList();
    static BOOL groupDriverUpdateStatus();
    static BOOL printDriverUpdateStatus();
    static BOOL groupVirtualAddressInfo();
    static BOOL printVirtualAddressInfo();
    static BOOL groupDriverVersion();
    static BOOL printDriverVersion();
    static BOOL groupAdapterType();
    static BOOL printAdapterType();
    static BOOL groupOutputDup();
    static BOOL printOutputDup();
    static BOOL groupWddm12caps();
    static BOOL printWddm12caps();
    static BOOL groupUmdDriverVersion();
    static BOOL printUmdDriverVersion();
    static BOOL groupDirectFlipSupport();
    static BOOL printDirectFlipSupport();
    static BOOL groupMpoSupport();
    static BOOL printMpoSupport();
    static BOOL groupDlistFileName();
    static BOOL printDlistFileName();
    static BOOL groupWddm13caps();
    static BOOL printWddm13caps();
    static BOOL groupMpoHudSupport();
    static BOOL printMpoHudSupport();
    static BOOL groupWddm20caps();
    static BOOL printWddm20caps();
    static BOOL groupNodeMetaData();
    static BOOL printNodeMetaData();
    static BOOL groupCpDriverName();
    static BOOL printCpDriverName();
    static BOOL groupIsXbox();
    static BOOL printIsXbox();
    static BOOL groupIndFlip();
    static BOOL printIndFlip();
    static BOOL groupMiraCastName();
    static BOOL printMiraCastName();
    static BOOL groupPhysicalAdapter();
    static BOOL printPhysicalAdapter();
    static BOOL groupPhysicalAdapterId();
    static BOOL printPhysicalAdapterId();
    static BOOL groupMiraCastType();
    static BOOL printMiraCastType();
    static BOOL groupQueryGpuMmuCaps();
    static BOOL printQueryGpuMmuCaps();
    static BOOL groupMpoDecodeSupport();
    static BOOL printMpoDecodeSupport();
    static BOOL groupIsBadDriverDisabled();
    static BOOL printIsBadDriverDisabled();
    static BOOL groupMpoSecondarySupport();
    static BOOL printMpoSecondarySupport();
    static BOOL groupIndFlipSecSupport();
    static BOOL printIndFlipSecSupport();
    static BOOL groupPanelFitterSupport();
    static BOOL printPanelFitterSupport();
    static BOOL groupAdapterPnpKeyHw();
    static BOOL groupAdapterPnpKeySw();
    static BOOL printAdapterPnpKey();
    static BOOL groupSegmentGroupSize();
    static BOOL printSegmentGroupSize();
    static BOOL groupMpo3ddiSupport();
    static BOOL printMpo3ddiSupport();
    static BOOL groupHwDrmSupport();
    static BOOL printHwDrmSupport();
    static BOOL groupMpoKernelCaps();
    static BOOL printMpoKernelCaps();
    static BOOL groupMpoStretchSupport();
    static BOOL printMpoStretchSupport();
    static BOOL groupGetVidpnOvnership();
    static BOOL printGetVidpnOvnership();
    static BOOL groupKmdDriverVersion();
    static BOOL printKmdDriverVersion();
    static BOOL groupBlockListKernel();
    static BOOL groupBlockListRuntime();
    static BOOL printBlockList();
    static BOOL groupAdapterUniqueGuid();
    static BOOL printAdapterUniqueGuid();
    static BOOL groupNodePerfData();
    static BOOL printNodePerfData();
    static BOOL groupAdapterPerfData();
    static BOOL printAdapterPerfData();
    static BOOL groupAdapterPerfDataCaps();
    static BOOL printAdapterPerfDataCaps();
    static BOOL groupGpuVersion();
    static BOOL printGpuVersion();
    static BOOL groupDriverDescription();
    static BOOL printDriverDescription();
    static BOOL groupWddm27caps();
    static BOOL printWddm27caps();
    static BOOL groupHybridDlistSupport();
    static BOOL printHybridDlistSupport();

    static void helperPrintString(const CHAR* name, WCHAR* value);
    static void helperUnicodeToAscii(CHAR* dst, WCHAR* src);
    static void helperPrintDecimal(const CHAR* name, ULONG value);
    static void helperPrintHex32(const CHAR* name, ULONG value);
    static void helperPrintHex64(const CHAR* name, ULONGLONG value);
    static void helperPrintSize(const CHAR* name, ULONGLONG value);
    static void helperWriteSize(char* buf, ULONGLONG bytes);
    static void helperPrintFrequency(const CHAR* name, double value);
    static void helperPrintLatency(const CHAR* name, double value);
    static void helperPrintVoltage(const CHAR* name, double value);
    static void helperPrintBandwidth(const CHAR* name, double value);
    static void helperPrintTemperature(const CHAR* name, double value);
    static void helperPrintRpm(const CHAR* name, ULONG value);
};

#endif  // TASK_50_H


