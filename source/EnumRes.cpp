/* ----------------------------------------------------------------------------------------
Device resources enumerator.
Special thanks to:
Microsoft enumerator sample:
https://github.com/microsoft/Windows-driver-samples/tree/main/setup/devcon
---------------------------------------------------------------------------------------- */

#include "EnumRes.h"

void EnumerateDeviceResourcesToTransitList(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo, LPCSTR devName, PRESOURCESORT trControl, UINT trControlLength)
{
    if (trControl && (trControlLength >= RES_MAX))
    {
        // Return BOOL value yet not used, resource not logged if failure.
        DumpDeviceResources(Devs, DevInfo, devName, trControl);
    }
}
void EnumerateTransitListToGroupList(LPTSTR& pBase, UINT64& pMax, 
    PRESOURCESORT trControl, UINT trControlLength, PGROUPSORT rsControl, UINT rsControlLength)
{
    if (trControl && (trControlLength >= RES_MAX) && rsControl && (rsControlLength >= RES_MAX))
    {
        // Cycle for resource types, i = resource type, select one af resource lists (MEM, LARGE MEM, IO, IRQ, DMA).
        for (int i = 0; i < RES_MAX; i++)
        {
            if (pMax <= 0) break;
            // Source data: get resource list structure from array of lists (MEM, LARGE MEM, IO, IRQ, DMA).
            RESOURCESORT rs = trControl[i];
            // Sorting resource list with comparator as lambda expression.
            std::sort(rs.childRanges->begin(), rs.childRanges->end(), 
                [](RESOURCEENTRY x, RESOURCEENTRY y) 
                { 
                    bool flag;
                    if (x.dataL == y.dataL)  // compare start address
                    {   // variant with end address if start address match
                        flag = x.dataH < y.dataH;
                    }
                    else
                    {   // variant with start address if it differrent
                        flag = x.dataL < y.dataL;
                    }
                    return flag;
                });
            // Get size of resource list and iterator.
            size_t n = rs.childRanges->size();
            std::vector<RESOURCEENTRY>::iterator vit = rs.childRanges->begin();
            // Destination data: get node structure from tree control structures array.
            GROUPSORT gs = rsControl[i];
            // Cycle for resources ranges per type, j = index in the resources lists per each type.
            for (size_t j = 0; j < n; j++)
            {
                RESOURCEENTRY re = *vit++;
                UINT adv = 0;
                int a;
                switch (i)
                {
                case RES_MEM:
                    adv = snprintf(pBase, (size_t)pMax, "%08I64Xh-%08I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_LMEM:
                    adv = snprintf(pBase, (size_t)pMax, "%016I64Xh-%016I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_IO:
                    adv = snprintf(pBase, (size_t)pMax, "%04I64Xh-%04I64Xh : %s", re.dataL, re.dataH, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_IRQ:
                    a = (int)re.dataL;
                    adv = snprintf(pBase, (size_t)pMax, "%d : %s", a, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                case RES_DMA:
                    a = (int)re.dataL;
                    adv = snprintf(pBase, (size_t)pMax, "%d : %s", a, re.deviceName);
                    gs.childStrings->push_back(pBase);
                    break;
                default:
                    break;
                }
                // Modify pointer and size limit, add 0 termination char.
                pBase += adv;
                pMax -= adv;
                if (pMax <= 0) break;
                *pBase = 0;
                pBase++;
                pMax--;
                if (pMax <= 0) break;
            }
        }
    }
}
BOOL DumpDeviceResources(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, 
    LPCSTR devName, PRESOURCESORT trControl)
/*++

Routine Description:

    Dump Resources to stdout

Arguments:

    Devs    )_ uniquely identify device
    DevInfo )

Return Value:

    none

--*/
{
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    ULONG status = 0;
    ULONG problem = 0;
    LOG_CONF config = 0;
    BOOL haveConfig = FALSE;

    //
    // see what state the device is in
    //
    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if ((!SetupDiGetDeviceInfoListDetail(Devs, &devInfoListDetail)) ||
        (CM_Get_DevNode_Status_Ex(&status, &problem, DevInfo->DevInst, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS)) {
        return FALSE;
    }

    //
    // see if the device is running and what resources it might be using
    //
    if (!(status & DN_HAS_PROBLEM)) {
        //
        // If this device is running, does this devinst have a ALLOC log config?
        //
        if (CM_Get_First_Log_Conf_Ex(&config,
            DevInfo->DevInst,
            ALLOC_LOG_CONF,
            devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }
    if (!haveConfig) {
        //
        // If no config so far, does it have a FORCED log config?
        // (note that technically these resources might be used by another device
        // but is useful info to show)
        //
        if (CM_Get_First_Log_Conf_Ex(&config,
            DevInfo->DevInst,
            FORCED_LOG_CONF,
            devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
            haveConfig = TRUE;
        }
    }

    if (!haveConfig) {
        //
        // if there's a hardware-disabled problem, boot-config isn't valid
        // otherwise use this if we don't have anything else
        //
        if (!(status & DN_HAS_PROBLEM) || (problem != CM_PROB_HARDWARE_DISABLED)) {
            //
            // Does it have a BOOT log config?
            //
            if (CM_Get_First_Log_Conf_Ex(&config,
                DevInfo->DevInst,
                BOOT_LOG_CONF,
                devInfoListDetail.RemoteMachineHandle) == CR_SUCCESS) {
                haveConfig = TRUE;
            }
        }
    }

    if (!haveConfig) {
        //
        // if we don't have any configuration, display an apropriate message
        //
/*
        Padding(1);
        FormatToStream(stdout, (status & DN_STARTED) ? MSG_DUMP_NO_RESOURCES : MSG_DUMP_NO_RESERVED_RESOURCES);
*/
        return TRUE;
    }
/*
    Padding(1);
    FormatToStream(stdout, (status & DN_STARTED) ? MSG_DUMP_RESOURCES : MSG_DUMP_RESERVED_RESOURCES);
*/
    //
    // dump resources
    //
    DumpDeviceResourcesOfType(DevInfo->DevInst, devInfoListDetail.RemoteMachineHandle, config, ResType_All, 
        devName, trControl);

    //
    // release handle
    //
    CM_Free_Log_Conf_Handle(config);

    return TRUE;
}
BOOL DumpDeviceResourcesOfType(_In_ DEVINST DevInst, _In_ HMACHINE MachineHandle, _In_ LOG_CONF Config, _In_ RESOURCEID ReqResId, 
    LPCSTR devName, PRESOURCESORT trControl)
{
    RES_DES prevResDes = (RES_DES)Config;
    RES_DES resDes = 0;
    RESOURCEID resId = ReqResId;
    ULONG dataSize;
    PBYTE resDesData;
    BOOL  retval = FALSE;

    UNREFERENCED_PARAMETER(DevInst);

    while (CM_Get_Next_Res_Des_Ex(&resDes, prevResDes, ReqResId, &resId, 0, MachineHandle) == CR_SUCCESS) {
        if (prevResDes != Config) {
            CM_Free_Res_Des_Handle(prevResDes);
        }
        prevResDes = resDes;
        if (CM_Get_Res_Des_Data_Size_Ex(&dataSize, resDes, 0, MachineHandle) != CR_SUCCESS) {
            continue;
        }
        resDesData = new BYTE[dataSize];
        if (!resDesData) {
            continue;
        }
        if (CM_Get_Res_Des_Data_Ex(resDes, resDesData, dataSize, 0, MachineHandle) != CR_SUCCESS) {
            delete[] resDesData;
            continue;
        }
        switch (resId) 
        {
            case ResType_Mem: 
            {
                PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)resDesData;
                if (pMemData->MEM_Header.MD_Alloc_End - pMemData->MEM_Header.MD_Alloc_Base + 1) 
                {
                    // Padding(2);
                    // _tprintf(TEXT("MEM : %08I64x-%08I64x\n"), pMemData->MEM_Header.MD_Alloc_Base, pMemData->MEM_Header.MD_Alloc_End);
                    //
                    DWORDLONG base = pMemData->MEM_Header.MD_Alloc_Base;
                    DWORDLONG end = pMemData->MEM_Header.MD_Alloc_End;
                    DWORDLONG limit = 0xFFFFFFFFL;
                    int index = RES_MEM;
                    if ((base > limit) || (end > limit)) index = RES_LMEM;
                    RESOURCEENTRY e;
                    e.deviceName = devName;
                    e.dataL = base;
                    e.dataH = end;
                    trControl[index].childRanges->push_back(e);
                    //
                    retval = TRUE;
                }
                break;
            }
            case ResType_MemLarge:
            {
                PMEM_LARGE_RESOURCE  pMemData = (PMEM_LARGE_RESOURCE)resDesData;
                DWORDLONG base = pMemData->MEM_LARGE_Header.MLD_Alloc_Base;
                DWORDLONG end = pMemData->MEM_LARGE_Header.MLD_Alloc_End;
                DWORDLONG limit = 0xFFFFFFFFL;
                int index = RES_MEM;
                if ((base > limit) || (end > limit)) index = RES_LMEM;
                RESOURCEENTRY e;
                e.deviceName = devName;
                e.dataL = base;
                e.dataH = end;
                trControl[index].childRanges->push_back(e);
                break;
            }

            case ResType_IO: 
            {
                PIO_RESOURCE   pIoData = (PIO_RESOURCE)resDesData;
                if (pIoData->IO_Header.IOD_Alloc_End - pIoData->IO_Header.IOD_Alloc_Base + 1) 
                {
                    // Padding(2);
                    // _tprintf(TEXT("IO  : %04I64x-%04I64x\n"), pIoData->IO_Header.IOD_Alloc_Base, pIoData->IO_Header.IOD_Alloc_End);
                    //
                    DWORDLONG base = pIoData->IO_Header.IOD_Alloc_Base;
                    DWORDLONG end = pIoData->IO_Header.IOD_Alloc_End;
                    DWORDLONG limit = 0xFFFFFFFFL;
                    int index = RES_IO;
                    if (pIoData->IO_Header.IOD_DesFlags & fIOD_Memory)
                    {
                        index = RES_MEM;
                        if ((base > limit) || (end > limit)) index = RES_LMEM;
                    }
                    RESOURCEENTRY e;
                    e.deviceName = devName;
                    e.dataL = base;
                    e.dataH = end;
                    trControl[index].childRanges->push_back(e);
                    //
                    retval = TRUE;
                }
                break;
            }

            case ResType_DMA: 
            {
                PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)resDesData;
                // Padding(2);
                // _tprintf(TEXT("DMA : %u\n"), pDmaData->DMA_Header.DD_Alloc_Chan);
                //
                DWORDLONG dma = pDmaData->DMA_Header.DD_Alloc_Chan;
                RESOURCEENTRY e;
                e.deviceName = devName;
                e.dataL = dma;
                e.dataH = 0;
                trControl[RES_DMA].childRanges->push_back(e);
                //
                retval = TRUE;
                break;
            }

            case ResType_IRQ: 
            {
                PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)resDesData;
                // Padding(2);
                // _tprintf(TEXT("IRQ : %u\n"), pIrqData->IRQ_Header.IRQD_Alloc_Num);
                //
                DWORDLONG irq = pIrqData->IRQ_Header.IRQD_Alloc_Num;
                RESOURCEENTRY e;
                e.deviceName = devName;
                e.dataL = irq;
                e.dataH = 0;
                trControl[RES_IRQ].childRanges->push_back(e);
                //
                retval = TRUE;
                break;
            }
        }
        delete[] resDesData;
    }
    if (prevResDes != Config) 
    {
        CM_Free_Res_Des_Handle(prevResDes);
    }
    return retval;
}
