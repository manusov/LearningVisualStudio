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

#include "Task_50.h"
using namespace std;

Task_50::Task_50() :
    DeviceInterfaceList{ nullptr },
    DeviceInterfaceListLength{ 0 },
    nt{ 0 },
    open{ 0 },
    query{ 0 },
    adapterHandle{ 0 }
{
    memset(&gpuData, 0, sizeof(GPU_DATA));
}
Task_50::~Task_50()
{
}
int Task_50::execute(int argc, char** argv)
{
	int status = 0;
    AppLib::write(BUILD_STRING);
    AppLib::write("\r\n");
    if (!sequencerGpuInfoInit())
    {
        status = 1;
    }
    else
    {
        sequencerGpuInfoPrint();
    }
	return status;
}

#if _WIN64
const char* Task_50::BUILD_STRING = "GPU Info v0.00.03 (x64).";
#elif _WIN32
const char* Task_50::BUILD_STRING = "GPU Info v0.00.03 (ia32).";
#else
const char* Task_50::BUILD_STRING = "UNSUPPORTED PLATFORM.";
#endif
const int Task_50::PARM_WIDTH = 28;
const int Task_50::VALUE_WIDTH = 80;
const int Task_50::MAX_DRIVER_DESCRIPTION_LENGTH = 4096;

const char* Task_50::STRINGS_D3DDDIFORMAT[] =
{
    "D3DDDIFMT_UNKNOWN",
    "D3DDDIFMT_R8G8B8",
    "D3DDDIFMT_A8R8G8B8",
    "D3DDDIFMT_X8R8G8B8",
    "D3DDDIFMT_R5G6B5",
    "D3DDDIFMT_X1R5G5B5",
    "D3DDDIFMT_A1R5G5B5",
    "D3DDDIFMT_A4R4G4B4",
    "D3DDDIFMT_R3G3B2",
    "D3DDDIFMT_A8",
    "D3DDDIFMT_A8R3G3B2",
    "D3DDDIFMT_X4R4G4B4",
    "D3DDDIFMT_A2B10G10R10",
    "D3DDDIFMT_A8B8G8R8",
    "D3DDDIFMT_X8B8G8R8",
    "D3DDDIFMT_G16R16",
    "D3DDDIFMT_A2R10G10B10",
    "D3DDDIFMT_A16B16G16R16",
    "D3DDDIFMT_A8P8",
    "D3DDDIFMT_P8",
    "D3DDDIFMT_L8",
    "D3DDDIFMT_A8L8",
    "D3DDDIFMT_A4L4",
    "D3DDDIFMT_V8U8",
    "D3DDDIFMT_L6V5U5",
    "D3DDDIFMT_X8L8V8U8",
    "D3DDDIFMT_Q8W8V8U8",
    "D3DDDIFMT_V16U16",
    "D3DDDIFMT_W11V11U10",
    "D3DDDIFMT_A2W10V10U10",
    "D3DDDIFMT_UYVY",
    "D3DDDIFMT_R8G8_B8G8",
    "D3DDDIFMT_YUY2",
    "D3DDDIFMT_G8R8_G8B8",
    "D3DDDIFMT_DXT1",
    "D3DDDIFMT_DXT2",
    "D3DDDIFMT_DXT3",
    "D3DDDIFMT_DXT4",
    "D3DDDIFMT_DXT5",
    "D3DDDIFMT_D16_LOCKABLE",
    "D3DDDIFMT_D32",
    "D3DDDIFMT_D15S1",
    "D3DDDIFMT_D24S8",
    "D3DDDIFMT_D24X8",
    "D3DDDIFMT_D24X4S4",
    "D3DDDIFMT_D16",
    "D3DDDIFMT_D32F_LOCKABLE",
    "D3DDDIFMT_D24FS8",
    "D3DDDIFMT_D32_LOCKABLE",
    "D3DDDIFMT_S8_LOCKABLE",
    "D3DDDIFMT_S1D15",
    "D3DDDIFMT_S8D24",
    "D3DDDIFMT_X8D24",
    "D3DDDIFMT_X4S4D24",
    "D3DDDIFMT_L16",
    "D3DDDIFMT_G8R8",
    "D3DDDIFMT_R8",
    "D3DDDIFMT_VERTEXDATA",
    "D3DDDIFMT_INDEX16",
    "D3DDDIFMT_INDEX32",
    "D3DDDIFMT_Q16W16V16U16",
    "D3DDDIFMT_MULTI2_ARGB8",
    "D3DDDIFMT_R16F",
    "D3DDDIFMT_G16R16F",
    "D3DDDIFMT_A16B16G16R16F",
    "D3DDDIFMT_R32F",
    "D3DDDIFMT_G32R32F",
    "D3DDDIFMT_A32B32G32R32F",
    "D3DDDIFMT_CxV8U8",
    "D3DDDIFMT_A1",
    "D3DDDIFMT_A2B10G10R10_XR_BIAS",
    "D3DDDIFMT_DXVACOMPBUFFER_BASE",
    "D3DDDIFMT_PICTUREPARAMSDATA",
    "D3DDDIFMT_MACROBLOCKDATA",
    "D3DDDIFMT_RESIDUALDIFFERENCEDATA",
    "D3DDDIFMT_DEBLOCKINGDATA",
    "D3DDDIFMT_INVERSEQUANTIZATIONDATA",
    "D3DDDIFMT_SLICECONTROLDATA",
    "D3DDDIFMT_BITSTREAMDATA",
    "D3DDDIFMT_MOTIONVECTORBUFFER",
    "D3DDDIFMT_FILMGRAINBUFFER",
    "D3DDDIFMT_DXVA_RESERVED9",
    "D3DDDIFMT_DXVA_RESERVED10",
    "D3DDDIFMT_DXVA_RESERVED11",
    "D3DDDIFMT_DXVA_RESERVED12",
    "D3DDDIFMT_DXVA_RESERVED13",
    "D3DDDIFMT_DXVA_RESERVED14",
    "D3DDDIFMT_DXVA_RESERVED15",
    "D3DDDIFMT_DXVA_RESERVED16",
    "D3DDDIFMT_DXVA_RESERVED17",
    "D3DDDIFMT_DXVA_RESERVED18",
    "D3DDDIFMT_DXVA_RESERVED19",
    "D3DDDIFMT_DXVA_RESERVED20",
    "D3DDDIFMT_DXVA_RESERVED21",
    "D3DDDIFMT_DXVA_RESERVED22",
    "D3DDDIFMT_DXVA_RESERVED23",
    "D3DDDIFMT_DXVA_RESERVED24",
    "D3DDDIFMT_DXVA_RESERVED25",
    "D3DDDIFMT_DXVA_RESERVED26",
    "D3DDDIFMT_DXVA_RESERVED27",
    "D3DDDIFMT_DXVA_RESERVED28",
    "D3DDDIFMT_DXVA_RESERVED29",
    "D3DDDIFMT_DXVA_RESERVED30",
    "D3DDDIFMT_DXVA_RESERVED31",
    "D3DDDIFMT_DXVACOMPBUFFER_MAX",
    "D3DDDIFMT_BINARYBUFFER",
    "D3DDDIFMT_FORCE_UINT"
};
const char* Task_50::STRINGS_D3DDDI_VIDEO_SIGNAL_SCANLINE_ORDERING[] =
{
    "D3DDDI_VSSLO_UNINITIALIZED",
    "D3DDDI_VSSLO_PROGRESSIVE",
    "D3DDDI_VSSLO_INTERLACED_UPPERFIELDFIRST",
    "D3DDDI_VSSLO_INTERLACED_LOWERFIELDFIRST",
    "D3DDDI_VSSLO_OTHER"
};
const char* Task_50::STRINGS_D3DDDI_ROTATION[] =
{
    "D3DDDI_ROTATION_IDENTITY",
    "D3DDDI_ROTATION_90",
    "D3DDDI_ROTATION_180",
    "D3DDDI_ROTATION_270"
};
const char* Task_50::STRINGS_QAI_DRIVERVERSION[] =
{
    "KMT_DRIVERVERSION_WDDM_1_0",
    "KMT_DRIVERVERSION_WDDM_1_1_PRERELEASE",
    "KMT_DRIVERVERSION_WDDM_1_1",
    "KMT_DRIVERVERSION_WDDM_1_2",
    "KMT_DRIVERVERSION_WDDM_1_3",
    "KMT_DRIVERVERSION_WDDM_2_0",
    "KMT_DRIVERVERSION_WDDM_2_1",
    "KMT_DRIVERVERSION_WDDM_2_2",
    "KMT_DRIVERVERSION_WDDM_2_3",
    "KMT_DRIVERVERSION_WDDM_2_4",
    "KMT_DRIVERVERSION_WDDM_2_5",
    "KMT_DRIVERVERSION_WDDM_2_6",
    "KMT_DRIVERVERSION_WDDM_2_7",
    "KMT_DRIVERVERSION_WDDM_2_8",
    "KMT_DRIVERVERSION_WDDM_2_9",
    "KMT_DRIVERVERSION_WDDM_3_0",
    "KMT_DRIVERVERSION_WDDM_3_1"
};
const char* Task_50::STRINGS_D3DKMDT_MODE_PRUNING_REASON[] =
{
   "D3DKMDT_MPR_UNINITIALIZED",
    "D3DKMDT_MPR_ALLCAPS",
    "D3DKMDT_MPR_DESCRIPTOR_MONITOR_SOURCE_MODE",
    "D3DKMDT_MPR_DESCRIPTOR_MONITOR_FREQUENCY_RANGE",
    "D3DKMDT_MPR_DESCRIPTOR_OVERRIDE_MONITOR_SOURCE_MODE",
    "D3DKMDT_MPR_DESCRIPTOR_OVERRIDE_MONITOR_FREQUENCY_RANGE",
    "D3DKMDT_MPR_DEFAULT_PROFILE_MONITOR_SOURCE_MODE",
    "D3DKMDT_MPR_DRIVER_RECOMMENDED_MONITOR_SOURCE_MODE",
    "D3DKMDT_MPR_MONITOR_FREQUENCY_RANGE_OVERRIDE",
    "D3DKMDT_MPR_CLONE_PATH_PRUNED",
    "D3DKMDT_MPR_MAXVALID"
};
const char* Task_50::STRINGS_D3DKMT_MIRACAST_DRIVER_TYPE[] =
{
    "D3DKMT_MIRACAST_DRIVER_NOT_SUPPORTED",
    "D3DKMT_MIRACAST_DRIVER_IHV",
    "D3DKMT_MIRACAST_DRIVER_MS"
};

GPU_DATA Task_50::gpuData;

const GPU_INFO_SEQUENCE Task_50::list[] =
{

    { groupUmdFileNameInfoDX9  , printUmdFileName         , KMTQAITYPE_UMDRIVERNAME                             , sizeof(D3DKMT_UMDFILENAMEINFO)                     },
    { groupUmdFileNameInfoDX10 , printUmdFileName         , KMTQAITYPE_UMDRIVERNAME                             , sizeof(D3DKMT_UMDFILENAMEINFO)                     },
    { groupUmdFileNameInfoDX11 , printUmdFileName         , KMTQAITYPE_UMDRIVERNAME                             , sizeof(D3DKMT_UMDFILENAMEINFO)                     },
    { groupUmdFileNameInfoDX12 , printUmdFileName         , KMTQAITYPE_UMDRIVERNAME                             , sizeof(D3DKMT_UMDFILENAMEINFO)                     },
    { groupOpenGlInfo          , printOpenGlInfo          , KMTQAITYPE_UMOPENGLINFO                             , sizeof(D3DKMT_OPENGLINFO)                          },
    { groupSegmentSizeInfo     , printSegmentSizeInfo     , KMTQAITYPE_GETSEGMENTSIZE                           , sizeof(D3DKMT_SEGMENTSIZEINFO)                     },
    { groupAdapterGuid         , printAdapterGuid         , KMTQAITYPE_ADAPTERGUID                              , sizeof(GUID)                                       },
    { groupFlipQueueInfo       , printFlipQueueInfo       , KMTQAITYPE_FLIPQUEUEINFO                            , sizeof(D3DKMT_FLIPQUEUEINFO)                       },
    { groupAdapterAddress      , printAdapterAddress      , KMTQAITYPE_ADAPTERADDRESS                           , sizeof(D3DKMT_ADAPTERADDRESS)                      },
    { groupWorkingSetInfo      , printWorkingSetInfo      , KMTQAITYPE_SETWORKINGSETINFO                        , sizeof(D3DKMT_WORKINGSETINFO)                      },
    { groupAdapterRegistryInfo , printAdapterRegistryInfo , KMTQAITYPE_ADAPTERREGISTRYINFO                      , sizeof(D3DKMT_ADAPTERREGISTRYINFO)                 },
    { groupCurrentDisplayMode  , printCurrentDisplayMode  , KMTQAITYPE_CURRENTDISPLAYMODE                       , sizeof(D3DKMT_CURRENTDISPLAYMODE)                  },
    { groupModeList            , printCurrentDisplayMode  , KMTQAITYPE_MODELIST                                 , sizeof(D3DKMT_DISPLAYMODE)                         },
    { groupDriverUpdateStatus  , printDriverUpdateStatus  , KMTQAITYPE_CHECKDRIVERUPDATESTATUS                  , sizeof(BOOL)                                       },
    { groupVirtualAddressInfo  , printVirtualAddressInfo  , KMTQAITYPE_VIRTUALADDRESSINFO                       , sizeof(D3DKMT_VIRTUALADDRESSFLAGS)                 },
    { groupDriverVersion       , printDriverVersion       , KMTQAITYPE_DRIVERVERSION                            , sizeof(D3DKMT_DRIVERVERSION)                       },
    { groupAdapterType         , printAdapterType         , KMTQAITYPE_ADAPTERTYPE                              , sizeof(D3DKMT_ADAPTERTYPE)                         },
    { groupOutputDup           , printOutputDup           , KMTQAITYPE_OUTPUTDUPLCONTEXTSCOUNT                  , sizeof(D3DKMT_OUTPUTDUPLCONTEXTSCOUNT)             },
    { groupWddm12caps          , printWddm12caps          , KMTQAITYPE_WDDM_1_2_CAPS                            , sizeof(D3DKMT_WDDM_1_2_CAPS)                       },
    { groupUmdDriverVersion    , printUmdDriverVersion    , KMTQAITYPE_UMD_DRIVER_VERSION                       , sizeof(D3DKMT_UMD_DRIVER_VERSION)                  },
    { groupDirectFlipSupport   , printDirectFlipSupport   , KMTQAITYPE_DIRECTFLIP_SUPPORT                       , sizeof(D3DKMT_DIRECTFLIP_SUPPORT)                  },
    { groupMpoSupport          , printMpoSupport          , KMTQAITYPE_MULTIPLANEOVERLAY_SUPPORT                , sizeof(D3DKMT_MULTIPLANEOVERLAY_SUPPORT)           },
    { groupDlistFileName       , printDlistFileName       , KMTQAITYPE_DLIST_DRIVER_NAME                        , sizeof(D3DKMT_DLIST_DRIVER_NAME)                   },
    { groupWddm13caps          , printWddm13caps          , KMTQAITYPE_WDDM_1_3_CAPS                            , sizeof(D3DKMT_WDDM_1_3_CAPS)                       },
    { groupMpoHudSupport       , printMpoHudSupport       , KMTQAITYPE_MULTIPLANEOVERLAY_HUD_SUPPORT            , sizeof(D3DKMT_MULTIPLANEOVERLAY_HUD_SUPPORT)       },
    { groupWddm20caps          , printWddm20caps          , KMTQAITYPE_WDDM_2_0_CAPS                            , sizeof(D3DKMT_WDDM_2_0_CAPS)                       },
    //  Unknown format for KMTQAITYPE_NODEMETADATA
    //  { groupNodeMetaData        , printNodeMetaData        , KMTQAITYPE_NODEMETADATA     //  Unknown format for KMTQAITYPE_NODEMETADATA
    { groupCpDriverName        , printCpDriverName        , KMTQAITYPE_CPDRIVERNAME                             , sizeof(D3DKMT_CPDRIVERNAME)                        },
    { groupIsXbox              , printIsXbox              , KMTQAITYPE_XBOX                                     , sizeof(D3DKMT_XBOX)                                },
    { groupIndFlip             , printIndFlip             , KMTQAITYPE_INDEPENDENTFLIP_SUPPORT                  , sizeof(D3DKMT_INDEPENDENTFLIP_SUPPORT)             },
    { groupMiraCastName        , printMiraCastName        , KMTQAITYPE_MIRACASTCOMPANIONDRIVERNAME              , sizeof(D3DKMT_MIRACASTCOMPANIONDRIVERNAME)         },
    { groupPhysicalAdapter     , printPhysicalAdapter     , KMTQAITYPE_PHYSICALADAPTERCOUNT                     , sizeof(D3DKMT_PHYSICAL_ADAPTER_COUNT)              },
    { groupPhysicalAdapterId   , printPhysicalAdapterId   , KMTQAITYPE_PHYSICALADAPTERDEVICEIDS                 , sizeof(D3DKMT_QUERY_DEVICE_IDS)                    },
    //  Unknown format for KMTQAITYPE_DRIVERCAPS_EXT
    { groupMiraCastType        , printMiraCastType        , KMTQAITYPE_QUERY_MIRACAST_DRIVER_TYPE               , sizeof(D3DKMT_MIRACAST_DRIVER_TYPE)                },
    { groupQueryGpuMmuCaps     , printQueryGpuMmuCaps     , KMTQAITYPE_QUERY_GPUMMU_CAPS                        , sizeof(D3DKMT_QUERY_GPUMMU_CAPS)                   },
    { groupMpoDecodeSupport    , printMpoDecodeSupport    , KMTQAITYPE_QUERY_MULTIPLANEOVERLAY_DECODE_SUPPORT   , sizeof(D3DKMT_MULTIPLANEOVERLAY_DECODE_SUPPORT)    },
    { groupIsBadDriverDisabled , printIsBadDriverDisabled , KMTQAITYPE_QUERY_ISBADDRIVERFORHWPROTECTIONDISABLED , sizeof(D3DKMT_ISBADDRIVERFORHWPROTECTIONDISABLED)  },
    { groupMpoSecondarySupport , printMpoSecondarySupport , KMTQAITYPE_MULTIPLANEOVERLAY_SECONDARY_SUPPORT      , sizeof(D3DKMT_MULTIPLANEOVERLAY_SECONDARY_SUPPORT) },
    { groupIndFlipSecSupport   , printIndFlipSecSupport   , KMTQAITYPE_INDEPENDENTFLIP_SECONDARY_SUPPORT        , sizeof(D3DKMT_INDEPENDENTFLIP_SECONDARY_SUPPORT)   },
    { groupPanelFitterSupport  , printPanelFitterSupport  , KMTQAITYPE_PANELFITTER_SUPPORT                      , sizeof(D3DKMT_PANELFITTER_SUPPORT)                 },
    // Unknown format for KMTQAITYPE_PHYSICALADAPTERPNPKEY
    //  { groupAdapterPnpKeyHw     , printAdapterPnpKey       , KMTQAITYPE_PHYSICALADAPTERPNPKEY                , sizeof(D3DKMT_QUERY_PHYSICAL_ADAPTER_PNP_KEY)      },
    //  { groupAdapterPnpKeySw     , printAdapterPnpKey       , KMTQAITYPE_PHYSICALADAPTERPNPKEY                , sizeof(D3DKMT_QUERY_PHYSICAL_ADAPTER_PNP_KEY)      },
    { groupSegmentGroupSize    , printSegmentGroupSize    , KMTQAITYPE_GETSEGMENTGROUPSIZE                      , sizeof(D3DKMT_SEGMENTGROUPSIZEINFO)                },
    { groupMpo3ddiSupport      , printMpo3ddiSupport      , KMTQAITYPE_MPO3DDI_SUPPORT                          , sizeof(D3DKMT_MPO3DDI_SUPPORT)                     },
    { groupHwDrmSupport        , printHwDrmSupport        , KMTQAITYPE_HWDRM_SUPPORT                            , sizeof(D3DKMT_HWDRM_SUPPORT)                       },
    { groupMpoKernelCaps       , printMpoKernelCaps       , KMTQAITYPE_MPOKERNELCAPS_SUPPORT                    , sizeof(D3DKMT_MPOKERNELCAPS_SUPPORT)               },
    // Wrong result, input required ?
    //  { groupMpoStretchSupport   , printMpoStretchSupport   , KMTQAITYPE_MULTIPLANEOVERLAY_STRETCH_SUPPORT    , sizeof(D3DKMT_MULTIPLANEOVERLAY_STRETCH_SUPPORT)   },
    //  { groupGetVidpnOvnership   , printGetVidpnOvnership   , KMTQAITYPE_GET_DEVICE_VIDPN_OWNERSHIP_INFO      , sizeof(D3DKMT_GET_DEVICE_VIDPN_OWNERSHIP_INFO)     },
    // Yet not used, input required:  D3DDDI_QUERYREGISTRY_INFO
    { groupKmdDriverVersion    , printKmdDriverVersion    , KMTQAITYPE_KMD_DRIVER_VERSION                       , sizeof(D3DKMT_KMD_DRIVER_VERSION)                  },
    // Wrong result, input required ?
    //  { groupBlockListKernel     , printBlockList           , KMTQAITYPE_BLOCKLIST_KERNEL                     , sizeof(D3DKMT_BLOCKLIST_INFO)                      },
    //  { groupBlockListRuntime    , printBlockList           , KMTQAITYPE_BLOCKLIST_RUNTIME                    , sizeof(D3DKMT_BLOCKLIST_INFO)                      },
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERGUID_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERADDRESS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERREGISTRYINFO_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_CHECKDRIVERUPDATESTATUS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_DRIVERVERSION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_ADAPTERTYPE_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_WDDM_1_2_CAPS_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_WDDM_1_3_CAPS_RENDER
    { groupAdapterUniqueGuid   , printAdapterUniqueGuid   , KMTQAITYPE_QUERY_ADAPTER_UNIQUE_GUID                , sizeof(D3DKMT_QUERY_ADAPTER_UNIQUE_GUID)           },
    { groupNodePerfData        , printNodePerfData        , KMTQAITYPE_NODEPERFDATA                             , sizeof(D3DKMT_NODE_PERFDATA)                       },
    { groupAdapterPerfData     , printAdapterPerfData     , KMTQAITYPE_ADAPTERPERFDATA                          , sizeof(D3DKMT_ADAPTER_PERFDATA)                    },
    { groupAdapterPerfDataCaps , printAdapterPerfDataCaps , KMTQAITYPE_ADAPTERPERFDATA_CAPS                     , sizeof(D3DKMT_ADAPTER_PERFDATACAPS)                },
    { groupGpuVersion          , printGpuVersion          , KMTQUITYPE_GPUVERSION                               , sizeof(D3DKMT_GPUVERSION)                          },
    { groupDriverDescription   , printDriverDescription   , KMTQAITYPE_DRIVER_DESCRIPTION                       , sizeof(D3DKMT_DRIVER_DESCRIPTION)                  },
    // Yet not used, unknown output format: KMTQAITYPE_DRIVER_DESCRIPTION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_SCANOUT_CAPS
    // Yet not used, unknown output format: KMTQAITYPE_DISPLAY_UMDRIVERNAME
    // Yet not used, unknown output format: KMTQAITYPE_PARAVIRTUALIZATION_RENDER
    // Yet not used, unknown output format: KMTQAITYPE_SERVICENAME
    { groupWddm27caps          , printWddm27caps          , KMTQAITYPE_WDDM_2_7_CAPS                            , sizeof(D3DKMT_WDDM_2_7_CAPS)                       },
    // Yet not used, unknown output format: KMTQAITYPE_TRACKEDWORKLOAD_SUPPORT
    // Yet not used, ID not defined:
    //  { groupHybridDlistSupport  , printHybridDlistSupport  , KMTQAITYPE_HYBRID_DLIST_DLL_SUPPORT             , sizeof(D3DKMT_HYBRID_DLIST_DLL_SUPPORT)            },
    // Yet not used, unknown output format: KMTQAITYPE_DISPLAY_CAPS
    // Yet not used, ID not defined: D3DKMT_WDDM_2_9_CAPS
    // Yet not used, ID not defined: D3DKMT_CROSSADAPTERRESOURCE_SUPPORT
    // Yet not used, ID not defined: D3DKMT_WDDM_3_0_CAPS
    // Yet not used, ID not defined: KMTQAITYPE_WSAUMDIMAGENAME
    // Yet not used, ID not defined: KMTQAITYPE_VGPUINTERFACEID
    // Yet not used, ID not defined: KMTQAITYPE_WDDM_3_1_CAPS
    { NULL                     , NULL                     , (KMTQUERYADAPTERINFOTYPE)0                          , 0                                                  }
};

BOOL Task_50::sequencerGpuInfoInit()
{
    //  The CM_Get_Device_Interface_List function retrieves a list of device interface instances that belong to a specified device interface class.
    CONFIGRET cr = CR_SUCCESS;
    do {
        cr = CM_Get_Device_Interface_List_Size(&DeviceInterfaceListLength,
            (LPGUID)&GUID_DISPLAY_DEVICE_ARRIVAL,
            NULL,
            CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);

        if (cr != CR_SUCCESS)
        {
            break;  // break if error
        }
        if (DeviceInterfaceList != NULL) {
            HeapFree(GetProcessHeap(),
                0,
                DeviceInterfaceList);
        }
        DeviceInterfaceList = (PWSTR)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            DeviceInterfaceListLength * sizeof(WCHAR));

        if (DeviceInterfaceList == NULL)
        {
            cr = CR_OUT_OF_MEMORY;
            break;  // break if error
        }

        cr = CM_Get_Device_Interface_ListW((LPGUID)&GUID_DISPLAY_DEVICE_ARRIVAL,
            NULL,
            (PZZWSTR)DeviceInterfaceList,
            DeviceInterfaceListLength,
            CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES);
    } while (cr == CR_BUFFER_SMALL);
    if (cr != CR_SUCCESS)
    {
        return FALSE;
    }
    //  The D3DKMTOpenAdapterFromDeviceName function maps a device name to a graphics adapter handle and,
    //  if the adapter contains multiple monitor outputs, to one of those outputs.
    open.pDeviceName = DeviceInterfaceList;
    nt = D3DKMTOpenAdapterFromDeviceName(&open);
    if (FAILED(nt))
    {
        return FALSE;
    }
    adapterHandle = open.hAdapter;
    if (adapterHandle == NULL)
    {
        return FALSE;
    }
    //  Prepare for the D3DKMTQueryAdapterInfo function retrieves various adapter information from an adapter handle.
    query.hAdapter = adapterHandle;
    query.pPrivateDriverData = &gpuData;
    return TRUE;
}
void Task_50::sequencerGpuInfoPrint()
{
    const GPU_INFO_SEQUENCE* p = list;
    while (p->printHandler != NULL)
    {
        query.PrivateDriverDataSize = p->privateDriverDataSize;
        query.Type = p->type;
        if ((*(p->groupHandler))())
        {
            if SUCCEEDED(D3DKMTQueryAdapterInfo(&query))
            {
                if (!(*(p->printHandler))())
                {
                    AppLib::writeParmError("Request done but data not valid");
                }
            }
            else
            {
                AppLib::writeParmError("Request rejected");
            }
        }
        else
        {
            AppLib::writeParmError("Request not supported");
        }
        p++;
    }
}

BOOL Task_50::groupUmdFileNameInfoDX9()
{
    AppLib::writeParmGroup("DirectX 9.0 UMD file");
    gpuData.umdFileNameInfo.Version = KMTUMDVERSION_DX9;
    return TRUE;
}
BOOL Task_50::groupUmdFileNameInfoDX10()
{
    AppLib::writeParmGroup("DirectX 10.0 UMD file");
    gpuData.umdFileNameInfo.Version = KMTUMDVERSION_DX10;
    return TRUE;
}
BOOL Task_50::groupUmdFileNameInfoDX11()
{
    AppLib::writeParmGroup("DirectX 11.0 UMD file");
    gpuData.umdFileNameInfo.Version = KMTUMDVERSION_DX11;
    return TRUE;
}
BOOL Task_50::groupUmdFileNameInfoDX12()
{
    AppLib::writeParmGroup("DirectX 12.0 UMD file");
    gpuData.umdFileNameInfo.Version = KMTUMDVERSION_DX12;
    return TRUE;
}
BOOL Task_50::printUmdFileName()
{
    helperPrintString("UMD file name", gpuData.umdFileNameInfo.UmdFileName);
    return TRUE;
}
BOOL Task_50::groupOpenGlInfo()
{
    AppLib::writeParmGroup("OpenGL info");
    gpuData.openGlInfo.Flags = 0;
    return TRUE;
}
BOOL Task_50::printOpenGlInfo()
{
    helperPrintHex32("OpenGL version", gpuData.openGlInfo.Version);
    helperPrintString("OpenGL file name", gpuData.openGlInfo.UmdOpenGlIcdFileName);
    return TRUE;
}
BOOL Task_50::groupSegmentSizeInfo()
{
    AppLib::writeParmGroup("Segment size");
    return TRUE;
}
BOOL Task_50::printSegmentSizeInfo()
{
    helperPrintSize("Dedicated system memory", gpuData.segmentSizeInfo.DedicatedSystemMemorySize);
    helperPrintSize("Dedicated video memory", gpuData.segmentSizeInfo.DedicatedVideoMemorySize);
    helperPrintSize("Shared system memory", gpuData.segmentSizeInfo.SharedSystemMemorySize);
    return TRUE;
}
BOOL Task_50::groupAdapterGuid()
{
    AppLib::writeParmGroup("Adapter GUID");
    return TRUE;
}
BOOL Task_50::printAdapterGuid()
{
    char s[APPCONST::VALUE_WIDTH];
    unsigned long data1 = gpuData.adapterGuid.Data1;
    unsigned short data2 = gpuData.adapterGuid.Data2;
    unsigned short data3 = gpuData.adapterGuid.Data3;
    unsigned char* pdata4 = gpuData.adapterGuid.Data4;
    unsigned long long data4 = *(unsigned long long*)pdata4;
    unsigned short data4a = data4 & 0xFFFFL;
    data4 >>= 16;
    snprintf(s, VALUE_WIDTH, "%08X-%04X-%04X-%04X-%012llX", data1, data2, data3, data4a, data4);
    AppLib::writeParmAndValue("Adapter GUID", s, APPCONST::PARM_WIDTH);
    return TRUE;
}
BOOL Task_50::groupFlipQueueInfo()
{
    AppLib::writeParmGroup("Flip queue info");
    return TRUE;
}
BOOL Task_50::printFlipQueueInfo()
{
    helperPrintDecimal("Max hardware flip queue", gpuData.flipQueueInfo.MaxHardwareFlipQueueLength);
    helperPrintDecimal("Max software flip queue", gpuData.flipQueueInfo.MaxSoftwareFlipQueueLength);
    helperPrintHex32("Flip flags", gpuData.flipQueueInfo.FlipFlags.FlipInterval);
    return TRUE;
}
BOOL Task_50::groupAdapterAddress()
{
    AppLib::writeParmGroup("Adapter address");
    return TRUE;
}
BOOL Task_50::printAdapterAddress()
{
    helperPrintDecimal("Bus", gpuData.adapterAddress.BusNumber);
    helperPrintDecimal("Device", gpuData.adapterAddress.DeviceNumber);
    helperPrintDecimal("Function", gpuData.adapterAddress.FunctionNumber);
    return TRUE;
}
BOOL Task_50::groupWorkingSetInfo()
{
    AppLib::writeParmGroup("Working set info");
    return TRUE;
}
BOOL Task_50::printWorkingSetInfo()
{
    helperPrintHex32("Working set flags", gpuData.workingSetInfo.Flags.UseDefault);
    helperPrintDecimal("Min working set percentile", gpuData.workingSetInfo.MinimumWorkingSetPercentile);
    helperPrintDecimal("Max working set percentile", gpuData.workingSetInfo.MaximumWorkingSetPercentile);
    return TRUE;
}
BOOL Task_50::groupAdapterRegistryInfo()
{
    AppLib::writeParmGroup("Adapter registry info");
    return TRUE;
}
BOOL Task_50::printAdapterRegistryInfo()
{
    helperPrintString("Adapter", gpuData.adapterRegistryInfo.AdapterString);
    helperPrintString("BIOS", gpuData.adapterRegistryInfo.BiosString);
    helperPrintString("Chip", gpuData.adapterRegistryInfo.ChipType);
    helperPrintString("DAC", gpuData.adapterRegistryInfo.DacType);
    return TRUE;
}
BOOL Task_50::groupCurrentDisplayMode()
{
    AppLib::writeParmGroup("Current display mode");
    gpuData.currentDisplayMode.VidPnSourceId = 0;
    // TODO. Setup ...Flags required
    return TRUE;
}
BOOL Task_50::printCurrentDisplayMode()
{
    char s[APPCONST::MAX_TEXT_STRING];
    helperPrintDecimal("Width", gpuData.currentDisplayMode.DisplayMode.Width);
    helperPrintDecimal("Height", gpuData.currentDisplayMode.DisplayMode.Height);

    int N = sizeof(STRINGS_D3DDDIFORMAT) / sizeof(char*);
    int i = gpuData.currentDisplayMode.DisplayMode.Format;
    char* s1;
    if (i < N)
    {
        s1 = (char*)STRINGS_D3DDDIFORMAT[i];
    }
    else
    {
        s1 = (char*)"?";
    }
    AppLib::writeParmAndValue("Format", s1, APPCONST::PARM_WIDTH);

    helperPrintDecimal("Integer refresh rate", gpuData.currentDisplayMode.DisplayMode.IntegerRefreshRate);

    int numerator = gpuData.currentDisplayMode.DisplayMode.RefreshRate.Numerator;
    int denominator = gpuData.currentDisplayMode.DisplayMode.RefreshRate.Denominator;
    double hz = (double)numerator / (double)denominator;
    snprintf(s, VALUE_WIDTH, "%d/%d = %.3f", numerator, denominator, hz);
    AppLib::writeParmAndValue("Rational refresh rate", s, APPCONST::PARM_WIDTH);

    N = sizeof(STRINGS_D3DDDI_VIDEO_SIGNAL_SCANLINE_ORDERING) / sizeof(char*);
    i = gpuData.currentDisplayMode.DisplayMode.ScanLineOrdering;
    if (i < N)
    {
        s1 = (char*)STRINGS_D3DDDI_VIDEO_SIGNAL_SCANLINE_ORDERING[i];
    }
    else
    {
        s1 = (char*)"?";
    }
    AppLib::writeParmAndValue("Scan line ordering", s1, APPCONST::PARM_WIDTH);

    N = sizeof(STRINGS_D3DDDI_ROTATION) / sizeof(char*);
    i = gpuData.currentDisplayMode.DisplayMode.DisplayOrientation;
    if (i < N)
    {
        s1 = (char*)STRINGS_D3DDDI_ROTATION[i];
    }
    else
    {
        s1 = (char*)"?";
    }
    AppLib::writeParmAndValue("Display orientation", s1, APPCONST::PARM_WIDTH);

    helperPrintDecimal("Display fixed output", gpuData.currentDisplayMode.DisplayMode.DisplayFixedOutput);

    // Flags
    helperPrintDecimal("Validated against monitor", gpuData.currentDisplayMode.DisplayMode.Flags.ValidatedAgainstMonitorCaps);
    helperPrintDecimal("Rounded fake mode", gpuData.currentDisplayMode.DisplayMode.Flags.RoundedFakeMode);

    N = sizeof(STRINGS_D3DKMDT_MODE_PRUNING_REASON) / sizeof(char*);
    i = gpuData.currentDisplayMode.DisplayMode.Flags.ModePruningReason;
    if (i < N)
    {
        snprintf(s, VALUE_WIDTH, "%s", (char*)STRINGS_D3DKMDT_MODE_PRUNING_REASON[i]);
    }
    else
    {
        snprintf(s, VALUE_WIDTH, "? (%d)", i);
    }
    AppLib::writeParmAndValue("Mode pruning reason", s, APPCONST::PARM_WIDTH);

    helperPrintDecimal("Stereo", gpuData.currentDisplayMode.DisplayMode.Flags.Stereo);
    helperPrintDecimal("Advanced scan capable", gpuData.currentDisplayMode.DisplayMode.Flags.AdvancedScanCapable);
    helperPrintDecimal("Preferred timing", gpuData.currentDisplayMode.DisplayMode.Flags.PreferredTiming);
    helperPrintDecimal("Physical mode supported", gpuData.currentDisplayMode.DisplayMode.Flags.PhysicalModeSupported);
    return TRUE;
}
BOOL Task_50::groupModeList()
{
    AppLib::writeParmGroup("Mode list");
    // TODO. This assignment valid and required ?
    gpuData.displayMode.Flags.ModePruningReason = D3DKMDT_MPR_ALLCAPS; // D3DKMDT_MPR_DESCRIPTOR_MONITOR_SOURCE_MODE;
    // TODO. Additional setup ...Flags required
    return TRUE;
}
BOOL Task_50::groupDriverUpdateStatus()
{
    AppLib::writeParmGroup("Driver update status");
    return TRUE;
}
BOOL Task_50::printDriverUpdateStatus()
{
    helperPrintDecimal("Driver update status", gpuData.driverUpdateStatus);
    return TRUE;
}
BOOL Task_50::groupVirtualAddressInfo()
{
    AppLib::writeParmGroup("Virtual address info");
    return TRUE;
}
BOOL Task_50::printVirtualAddressInfo()
{
    helperPrintDecimal("Virtual address supported", gpuData.virtualAddressInfo.VirtualAddressFlags.VirtualAddressSupported);
    return TRUE;
}
BOOL Task_50::groupDriverVersion()
{
    AppLib::writeParmGroup("Driver version");
    return TRUE;
}
BOOL Task_50::printDriverVersion()
{
    char s[MAX_PATH];
    int N = sizeof(STRINGS_QAI_DRIVERVERSION) / sizeof(char*);
    int i = gpuData.driverVersion;
    if (i < N)
    {
        snprintf(s, VALUE_WIDTH, "%s", (char*)STRINGS_QAI_DRIVERVERSION[i]);
    }
    else
    {
        snprintf(s, VALUE_WIDTH, "? (%d)", i);
    }
    AppLib::writeParmAndValue("Driver version", s, APPCONST::PARM_WIDTH);
    return TRUE;
}
BOOL Task_50::groupAdapterType()
{
    AppLib::writeParmGroup("Adapter type");
    return TRUE;
}
BOOL Task_50::printAdapterType()
{
    helperPrintDecimal("Render supported", gpuData.adapterType.RenderSupported);
    helperPrintDecimal("Display supported", gpuData.adapterType.DisplaySupported);
    helperPrintDecimal("Software device", gpuData.adapterType.SoftwareDevice);
    helperPrintDecimal("POST device", gpuData.adapterType.PostDevice);
    helperPrintDecimal("Hybrid discrete", gpuData.adapterType.HybridDiscrete);
    helperPrintDecimal("Hybrid integrated", gpuData.adapterType.HybridIntegrated);
    helperPrintDecimal("Indirect display device", gpuData.adapterType.IndirectDisplayDevice);
    helperPrintDecimal("Paravirtualized", gpuData.adapterType.Paravirtualized);
    helperPrintDecimal("Arbitrary Code Guard", gpuData.adapterType.ACGSupported);
    helperPrintDecimal("Set timings from VidPn", gpuData.adapterType.SupportSetTimingsFromVidPn);
    helperPrintDecimal("Detachable", gpuData.adapterType.Detachable);
    helperPrintDecimal("Compute only", gpuData.adapterType.ComputeOnly);
    helperPrintDecimal("Prototype", gpuData.adapterType.Prototype);
    helperPrintHex32("Value", gpuData.adapterType.Value);
    return TRUE;
}
BOOL Task_50::groupOutputDup()
{
    AppLib::writeParmGroup("Desktop duplication API");
    gpuData.outputDup.VidPnSourceId = 0;
    return TRUE;
}
BOOL Task_50::printOutputDup()
{
    helperPrintDecimal("Output duplication count", gpuData.outputDup.OutputDuplicationCount);
    return TRUE;
}
BOOL Task_50::groupWddm12caps()
{
    AppLib::writeParmGroup("WDDM 1.2 capabilities");
    return TRUE;
}
BOOL Task_50::printWddm12caps()
{
    helperPrintDecimal("Support non VGA", gpuData.wddm12caps.SupportNonVGA);
    helperPrintDecimal("Smooth rotation", gpuData.wddm12caps.SupportSmoothRotation);
    helperPrintDecimal("Per engine TDR", gpuData.wddm12caps.SupportPerEngineTDR);
    helperPrintDecimal("Kernel mode command buffer", gpuData.wddm12caps.SupportKernelModeCommandBuffer);
    helperPrintDecimal("CCD", gpuData.wddm12caps.SupportCCD);
    helperPrintDecimal("Software device bitmaps", gpuData.wddm12caps.SupportSoftwareDeviceBitmaps);
    helperPrintDecimal("Gamma ramp", gpuData.wddm12caps.SupportGammaRamp);
    helperPrintDecimal("HW Cursor", gpuData.wddm12caps.SupportHWCursor);
    helperPrintDecimal("HW VSync", gpuData.wddm12caps.SupportHWVSync);
    helperPrintDecimal("Surprise removal in hibrn.", gpuData.wddm12caps.SupportSurpriseRemovalInHibernation);
    helperPrintHex32("Value", gpuData.wddm12caps.Value);
    return TRUE;
}
BOOL Task_50::groupUmdDriverVersion()
{
    AppLib::writeParmGroup("UMD driver version");
    return TRUE;
}
BOOL Task_50::printUmdDriverVersion()
{
    helperPrintHex64("UMD driver version", gpuData.umdDriverVersion.DriverVersion.QuadPart);
    return TRUE;
}
BOOL Task_50::groupDirectFlipSupport()
{
    AppLib::writeParmGroup("Direct flip support");
    return TRUE;
}
BOOL Task_50::printDirectFlipSupport()
{
    helperPrintDecimal("Direct flip", gpuData.directFlipSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupMpoSupport()
{
    AppLib::writeParmGroup("MP overlay support");
    return TRUE;
}
BOOL Task_50::printMpoSupport()
{
    helperPrintDecimal("Multiplane overlay", gpuData.mpoSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupDlistFileName()
{
    AppLib::writeParmGroup("Device list file name");
    return TRUE;
}
BOOL Task_50::printDlistFileName()
{
    helperPrintString("Device list file name", gpuData.dlistDriverName.DListFileName);
    return TRUE;
}
BOOL Task_50::groupWddm13caps()
{
    AppLib::writeParmGroup("WDDM 1.3 capabilities");
    return TRUE;
}
BOOL Task_50::printWddm13caps()
{
    helperPrintDecimal("Miracast", gpuData.wddm13caps.SupportMiracast);
    helperPrintDecimal("Hybrid integrated GPU", gpuData.wddm13caps.IsHybridIntegratedGPU);
    helperPrintDecimal("Hybrid discrete GPU", gpuData.wddm13caps.IsHybridDiscreteGPU);
    helperPrintDecimal("PM PStates", gpuData.wddm13caps.SupportPowerManagementPStates);
    helperPrintDecimal("Virtual modes", gpuData.wddm13caps.SupportVirtualModes);
    helperPrintDecimal("Cross adapter resources", gpuData.wddm13caps.SupportCrossAdapterResource);
    helperPrintHex32("Value", gpuData.wddm13caps.Value);
    return TRUE;
}
BOOL Task_50::groupMpoHudSupport()
{
    AppLib::writeParmGroup("MPO HUD support");
    gpuData.mpoHudSupport.VidPnSourceId = 0;
    return TRUE;
}
BOOL Task_50::printMpoHudSupport()
{
    helperPrintDecimal("Update", gpuData.mpoHudSupport.Update);
    helperPrintDecimal("Kernel", gpuData.mpoHudSupport.KernelSupported);
    helperPrintDecimal("HUD", gpuData.mpoHudSupport.HudSupported);
    return TRUE;
}
BOOL Task_50::groupWddm20caps()
{
    AppLib::writeParmGroup("WDDM 2.0 capabilities");
    return TRUE;
}
BOOL Task_50::printWddm20caps()
{
    helperPrintDecimal("64-bit atomics", gpuData.wddm20caps.Support64BitAtomics);
    helperPrintDecimal("GPUMMU", gpuData.wddm20caps.GpuMmuSupported);
    helperPrintDecimal("IOMMU", gpuData.wddm20caps.IoMmuSupported);
    helperPrintDecimal("Flip owerwrite", gpuData.wddm20caps.FlipOverwriteSupported);
    helperPrintDecimal("Contextless present", gpuData.wddm20caps.SupportContextlessPresent);
    helperPrintDecimal("Surprise removal", gpuData.wddm20caps.SupportSurpriseRemoval);
    helperPrintHex32("Value", gpuData.wddm20caps.Value);
    return TRUE;
}
BOOL Task_50::groupNodeMetaData()
{
    AppLib::writeParmGroup("Node metadata");
    return TRUE;
}
BOOL Task_50::printNodeMetaData()
{
    // char s[MAX_PATH];
    // TODO.
    return TRUE;
}
BOOL Task_50::groupCpDriverName()
{
    AppLib::writeParmGroup("CP driver name");
    return TRUE;
}
BOOL Task_50::printCpDriverName()
{
    helperPrintString("CP driver name", gpuData.cpDriverName.ContentProtectionFileName);
    return TRUE;
}
BOOL Task_50::groupIsXbox()
{
    AppLib::writeParmGroup("Detect XBOX");
    return TRUE;
}
BOOL Task_50::printIsXbox()
{
    helperPrintDecimal("Is XBOX", gpuData.isXbox.IsXBOX);
    return TRUE;
}
BOOL Task_50::groupIndFlip()
{
    AppLib::writeParmGroup("Independent flip");
    return TRUE;
}
BOOL Task_50::printIndFlip()
{
    helperPrintDecimal("Independent flip", gpuData.indFlipSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupMiraCastName()
{
    AppLib::writeParmGroup("Miracast name");
    return TRUE;
}
BOOL Task_50::printMiraCastName()
{
    helperPrintString("Miracast driver name", gpuData.miraCastName.MiracastCompanionDriverName);
    return TRUE;
}
BOOL Task_50::groupPhysicalAdapter()
{
    AppLib::writeParmGroup("Adapter count");
    return TRUE;
}
BOOL Task_50::printPhysicalAdapter()
{
    helperPrintDecimal("Physical adapter count", gpuData.physicalAdapterCount.Count);
    return TRUE;
}
BOOL Task_50::groupPhysicalAdapterId()
{
    AppLib::writeParmGroup("Adapter ID");
    gpuData.physicalAdapterIds.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printPhysicalAdapterId()
{
    helperPrintHex32("Vendor", gpuData.physicalAdapterIds.DeviceIds.VendorID);
    helperPrintHex32("Device", gpuData.physicalAdapterIds.DeviceIds.DeviceID);
    helperPrintHex32("Sub vendor", gpuData.physicalAdapterIds.DeviceIds.SubVendorID);
    helperPrintHex32("Sub system", gpuData.physicalAdapterIds.DeviceIds.SubSystemID);
    helperPrintHex32("Revision", gpuData.physicalAdapterIds.DeviceIds.RevisionID);
    helperPrintHex32("Bus type", gpuData.physicalAdapterIds.DeviceIds.BusType);
    return TRUE;
}
BOOL Task_50::groupMiraCastType()
{
    AppLib::writeParmGroup("Miracast type");
    return TRUE;
}
BOOL Task_50::printMiraCastType()
{
    int N = sizeof(STRINGS_D3DKMT_MIRACAST_DRIVER_TYPE) / sizeof(char*);
    int i = gpuData.miraCastType;
    char* s1;
    if (i < N)
    {
        s1 = (char*)STRINGS_D3DKMT_MIRACAST_DRIVER_TYPE[i];
    }
    else
    {
        s1 = (char*)"?";
    }
    AppLib::writeParmAndValue("Miracast driver type", s1, APPCONST::PARM_WIDTH);
    return TRUE;
}
BOOL Task_50::groupQueryGpuMmuCaps()
{
    AppLib::writeParmGroup("GPU MMU capabilities");
    gpuData.queryMmuCaps.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printQueryGpuMmuCaps()
{
    helperPrintDecimal("Read only memory", gpuData.queryMmuCaps.Caps.Flags.ReadOnlyMemorySupported);
    helperPrintDecimal("No execute memory", gpuData.queryMmuCaps.Caps.Flags.NoExecuteMemorySupported);
    helperPrintDecimal("Cache coherent memory", gpuData.queryMmuCaps.Caps.Flags.CacheCoherentMemorySupported);
    helperPrintDecimal("Virtual address bit count", gpuData.queryMmuCaps.Caps.VirtualAddressBitCount);
    return TRUE;
}
BOOL Task_50::groupMpoDecodeSupport()
{
    AppLib::writeParmGroup("MPO decode support");
    return TRUE;
}
BOOL Task_50::printMpoDecodeSupport()
{
    helperPrintDecimal("MPO decode", gpuData.mpoDecodeSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupIsBadDriverDisabled()
{
    AppLib::writeParmGroup("Bad driver disabled");
    return TRUE;
}
BOOL Task_50::printIsBadDriverDisabled()
{
    helperPrintDecimal("Is bad driver disabled", gpuData.isBadDriverDisabled.Disabled);
    return TRUE;
}
BOOL Task_50::groupMpoSecondarySupport()
{
    AppLib::writeParmGroup("MPO secondary");
    return TRUE;
}
BOOL Task_50::printMpoSecondarySupport()
{
    helperPrintDecimal("MPO secondary support", gpuData.mpoSecondarySupport.Supported);
    return TRUE;
}
BOOL Task_50::groupIndFlipSecSupport()
{
    AppLib::writeParmGroup("Independent flip");
    return TRUE;
}
BOOL Task_50::printIndFlipSecSupport()
{
    helperPrintDecimal("Independent flip secondary", gpuData.indFlipSecSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupPanelFitterSupport()
{
    AppLib::writeParmGroup("Panel fitter");
    return TRUE;
}
BOOL Task_50::printPanelFitterSupport()
{
    helperPrintDecimal("Panel fitter support", gpuData.panelFitterSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupAdapterPnpKeyHw()
{
    AppLib::writeParmGroup("Adapter hardware PnP key");
    // TODO.
    // gpuData.adapterPnpKey.PhysicalAdapterIndex = 0;
    // gpuData.adapterPnpKey.PnPKeyType = D3DKMT_PNP_KEY_HARDWARE;
    return TRUE;
}
BOOL Task_50::groupAdapterPnpKeySw()
{
    AppLib::writeParmGroup("Adapter software PnP key");
    // TODO.
    // gpuData.adapterPnpKey.PhysicalAdapterIndex = 0;
    // gpuData.adapterPnpKey.PnPKeyType = D3DKMT_PNP_KEY_SOFTWARE;
    return TRUE;
}
BOOL Task_50::printAdapterPnpKey()
{
    // TODO.
    return TRUE;
}
BOOL Task_50::groupSegmentGroupSize()
{
    AppLib::writeParmGroup("Segment group size info");
    gpuData.segmentGroupSize.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printSegmentGroupSize()
{
    helperPrintSize("Dedicated system memory", gpuData.segmentGroupSize.LegacyInfo.DedicatedSystemMemorySize);
    helperPrintSize("Dedicated video memory", gpuData.segmentGroupSize.LegacyInfo.DedicatedVideoMemorySize);
    helperPrintSize("Shared system memory", gpuData.segmentGroupSize.LegacyInfo.SharedSystemMemorySize);
    helperPrintSize("Non budget memory", gpuData.segmentGroupSize.NonBudgetMemory);
    helperPrintSize("Local memory", gpuData.segmentGroupSize.LocalMemory);
    helperPrintSize("Non local memory", gpuData.segmentGroupSize.NonLocalMemory);
    return TRUE;
}
BOOL Task_50::groupMpo3ddiSupport()
{
    AppLib::writeParmGroup("MPO3DDI");
    return TRUE;
}
BOOL Task_50::printMpo3ddiSupport()
{
    helperPrintDecimal("MPO3DDI support", gpuData.mpo3ddiSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupHwDrmSupport()
{
    AppLib::writeParmGroup("HW DRM");
    return TRUE;
}
BOOL Task_50::printHwDrmSupport()
{
    helperPrintDecimal("HW DRM support", gpuData.hwDrmSupport.Supported);
    return TRUE;
}
BOOL Task_50::groupMpoKernelCaps()
{
    AppLib::writeParmGroup("MPO kernel");
    return TRUE;
}
BOOL Task_50::printMpoKernelCaps()
{
    helperPrintDecimal("MPO kernel capabilities", gpuData.mpoKernelCaps.Supported);
    return TRUE;
}
BOOL Task_50::groupMpoStretchSupport()
{
    // TODO.
    // parmGroup("MPO stretch");
    // gpuData.mpoStretchSupport.VidPnSourceId = 0;
    return TRUE;
}
BOOL Task_50::printMpoStretchSupport()
{
    // TODO (with optimization by print helpers use).
    // char s[MAX_PATH];
    // // snprintf(s, VALUE_WIDTH, "%d", gpuData.mpoStretchSupport.Update);
    // snprintf(s, VALUE_WIDTH, "%08Xh", gpuData.mpoStretchSupport.Update);
    // parmValue("MPO stretch update", s);
    // // snprintf(s, VALUE_WIDTH, "%d", gpuData.mpoStretchSupport.Supported);
    // snprintf(s, VALUE_WIDTH, "%08Xh", gpuData.mpoStretchSupport.Supported);
    // parmValue("MPO stretch support", s);
    return TRUE;
}
BOOL Task_50::groupGetVidpnOvnership()
{
    AppLib::writeParmGroup("VidPn ovnership");
    return TRUE;
}
BOOL Task_50::printGetVidpnOvnership()
{
    // TODO (with optimization by print helpers use).
    // char s[MAX_PATH];
    // snprintf(s, VALUE_WIDTH, "%08Xh", gpuData.getVidpnOvnership.hDevice);
    // parmValue("Device handle", s);
    // snprintf(s, VALUE_WIDTH, "%d", gpuData.getVidpnOvnership.bFailedDwmAcquireVidPn);
    // parmValue("Acquire failed flag", s);
    return TRUE;
}
BOOL Task_50::groupKmdDriverVersion()
{
    AppLib::writeParmGroup("KMD driver version");
    return TRUE;
}
BOOL Task_50::printKmdDriverVersion()
{
    helperPrintHex64("KMD driver version", gpuData.kmdDriverVersion.DriverVersion.QuadPart);
    return TRUE;
}
BOOL Task_50::groupBlockListKernel()
{
    AppLib::writeParmGroup("Block list kernel");
    return TRUE;
}
BOOL Task_50::groupBlockListRuntime()
{
    AppLib::writeParmGroup("Block list runtime");
    return TRUE;
}
BOOL Task_50::printBlockList()
{
    // TODO...
    return TRUE;
}
BOOL Task_50::groupAdapterUniqueGuid()
{
    AppLib::writeParmGroup("Adapter unique GUID");
    return TRUE;
}
BOOL Task_50::printAdapterUniqueGuid()
{
    helperPrintString("Adapter unique GUID", gpuData.adapterUniqueGuid.AdapterUniqueGUID);
    return TRUE;
}
BOOL Task_50::groupNodePerfData()
{
    AppLib::writeParmGroup("Node performance data");
    gpuData.nodePerfData.NodeOrdinal = 0;
    gpuData.nodePerfData.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printNodePerfData()
{
    helperPrintFrequency("Frequency", gpuData.nodePerfData.Frequency / 1000000.0);
    helperPrintFrequency("Maximum frequency", gpuData.nodePerfData.MaxFrequency / 1000000.0);
    helperPrintFrequency("OC maximum frequency", gpuData.nodePerfData.MaxFrequencyOC / 1000000.0);
    helperPrintVoltage("Voltage", gpuData.nodePerfData.Voltage / 1000.0);
    helperPrintVoltage("Maximum voltage", gpuData.nodePerfData.VoltageMax / 1000.0);
    helperPrintVoltage("OC maximum voltage", gpuData.nodePerfData.VoltageMaxOC / 1000.0);
    helperPrintLatency("Max transition latency", gpuData.nodePerfData.MaxTransitionLatency / 10.0);
    return TRUE;
}
BOOL Task_50::groupAdapterPerfData()
{
    AppLib::writeParmGroup("Performance data");
    gpuData.adapterPerfData.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printAdapterPerfData()
{
    helperPrintFrequency("Memory frequency", gpuData.adapterPerfData.MemoryFrequency / 1000000.0);
    helperPrintFrequency("Maximum memory frequency", gpuData.adapterPerfData.MaxMemoryFrequency / 1000000.0);
    helperPrintFrequency("OC memory frequency", gpuData.adapterPerfData.MaxMemoryFrequencyOC / 1000000.0);
    helperPrintBandwidth("Memory bandwidth", gpuData.adapterPerfData.MemoryBandwidth / 1000000.0);
    helperPrintBandwidth("PCIe bandwidth", gpuData.adapterPerfData.PCIEBandwidth / 1000000.0);
    helperPrintTemperature("Temperature", gpuData.adapterPerfData.Temperature / 10.0);
    helperPrintRpm("Fan RPM", gpuData.adapterPerfData.FanRPM);
    return TRUE;
}
BOOL Task_50::groupAdapterPerfDataCaps()
{
    AppLib::writeParmGroup("Performance capabilities");
    gpuData.adapterPerfDataCaps.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printAdapterPerfDataCaps()
{
    helperPrintBandwidth("Maximum memory bandwidth", gpuData.adapterPerfDataCaps.MaxMemoryBandwidth / 1000000.0);
    helperPrintBandwidth("Maximum PCIe bandwidth", gpuData.adapterPerfDataCaps.MaxPCIEBandwidth / 1000000.0);
    helperPrintRpm("Maximum Fan RPM", gpuData.adapterPerfDataCaps.MaxFanRPM);
    helperPrintTemperature("Maximum temperature", gpuData.adapterPerfDataCaps.TemperatureMax / 10.0);
    helperPrintTemperature("Warning temperature", gpuData.adapterPerfDataCaps.TemperatureWarning / 10.0);
    return TRUE;
}
BOOL Task_50::groupGpuVersion()
{
    AppLib::writeParmGroup("GPU version");
    gpuData.gpuVersion.PhysicalAdapterIndex = 0;
    return TRUE;
}
BOOL Task_50::printGpuVersion()
{
    helperPrintString("GPU architecture", gpuData.gpuVersion.GpuArchitecture);
    helperPrintString("BIOS version", gpuData.gpuVersion.BiosVersion);
    return TRUE;
}
BOOL Task_50::groupDriverDescription()
{
    AppLib::writeParmGroup("Driver description");
    return TRUE;
}
BOOL Task_50::printDriverDescription()
{
    helperPrintString("Driver description", gpuData.driverDescription.DriverDescription);
    return TRUE;
}
BOOL Task_50::groupWddm27caps()
{
    AppLib::writeParmGroup("WDDM 2.7 capabilities");
    return TRUE;
}
BOOL Task_50::printWddm27caps()
{
    helperPrintDecimal("HW scheduling supported", gpuData.wddm27caps.HwSchSupported);
    helperPrintDecimal("HW scheduling enabled", gpuData.wddm27caps.HwSchEnabled);
    helperPrintDecimal("HW scheduling default", gpuData.wddm27caps.HwSchEnabledByDefault);
    helperPrintDecimal("Independent VidPn VSync", gpuData.wddm27caps.IndependentVidPnVSyncControl);
    helperPrintHex32("Value", gpuData.wddm27caps.Value);
    return TRUE;
}
BOOL Task_50::groupHybridDlistSupport()
{
    AppLib::writeParmGroup("Hybrid Dlist");
    return TRUE;
}
BOOL Task_50::printHybridDlistSupport()
{
    // TODO (with optimizations by print helpers).
    // char s[MAX_PATH];
    // snprintf(s, VALUE_WIDTH, "%d", gpuData.hybridDlist.Supported);
    // parmValue("Hybrid Dlist support", s);
    return TRUE;
}

void Task_50::helperPrintString(const CHAR* name, WCHAR* value)
{
    char s[MAX_PATH];
    helperUnicodeToAscii(s, value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperUnicodeToAscii(CHAR* dst, WCHAR* src)
{
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, MAX_PATH, NULL, NULL);
}
void Task_50::helperPrintDecimal(const CHAR* name, ULONG value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%d", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintHex32(const CHAR* name, ULONG value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%08Xh", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintHex64(const CHAR* name, ULONGLONG value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%016llXh", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintSize(const CHAR* name, ULONGLONG value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    helperWriteSize(s, value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperWriteSize(char* buf, ULONGLONG bytes)
{
    constexpr int SMALL_STRING = 30;
    const char* suffixes[7];
    suffixes[0] = "Bytes";
    suffixes[1] = "KB";
    suffixes[2] = "MB";
    suffixes[3] = "GB";
    suffixes[4] = "TB";
    suffixes[5] = "PB";
    suffixes[6] = "EB";
    int s = 0;             // which suffix to use
    double count = (double)bytes;
    while (count >= 1024 && s < 7)
    {
        s++;
        count /= 1024;
    }
    if (count - floor(count) == 0.0)
        snprintf(buf, SMALL_STRING, "%d %s", (int)count, suffixes[s]);
    else
        snprintf(buf, SMALL_STRING, "%.2f %s", count, suffixes[s]);
}
void Task_50::helperPrintFrequency(const CHAR* name, double value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%.1f MHz", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintLatency(const CHAR* name, double value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%.1f uS", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintVoltage(const CHAR* name, double value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%.3f V", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintBandwidth(const CHAR* name, double value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%.1f MBPS", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintTemperature(const CHAR* name, double value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%.1f C", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
void Task_50::helperPrintRpm(const CHAR* name, ULONG value)
{
    char s[APPCONST::MAX_TEXT_STRING];
    snprintf(s, APPCONST::VALUE_WIDTH, "%d RPM", value);
    AppLib::writeParmAndValue(name, s, APPCONST::PARM_WIDTH);
}
