/** @file
Pssi(Phytium System service interface) Libary.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/ArmSmcLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Uefi/UefiBaseType.h>
#include <Library/PssiLib.h>
#include <Library/ParameterTable.h>

/**
  Get Pssi(Phytium System service interface) version.

  @param[in, out] MajorVer   A pointer to the Major version.

  @param[in, out] MinorVer   A pointer to the Minor version.

**/
VOID
PssiGetPssiVersion (
  IN OUT UINT16 *MajorVer,
  IN OUT UINT16 *MinorVer
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_VERSION;
  ArmCallSmc (&ArmSmcArgs);

  // bit[16:31] Major version, BIT[0:15] Minor version
  DEBUG ((DEBUG_INFO, "========PSSI Version========\n"));
  *MajorVer = (UINT16) (ArmSmcArgs.Arg0 >> 16);
  *MinorVer = (UINT16) (ArmSmcArgs.Arg0);
  DEBUG ((DEBUG_INFO, "  PSSI Version: %d.%d\n", *MajorVer, *MinorVer));
}

/**
  Get PBF(Processor Base Firmware) version.

  @param[in, out] MajorVer   A pointer to the Major version.

  @param[in, out] MinorVer   A pointer to the Minor version.

**/
VOID
PssiGetPbfVersion (
  IN OUT  UINT16 *MajorVer,
  IN OUT  UINT16 *MinorVer
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PBF_VERSION;
  ArmCallSmc (&ArmSmcArgs);

  // bit[16:31] Major version, BIT[0:15] Minor version
  DEBUG ((DEBUG_INFO, "========PBF Version========\n"));
  *MajorVer = (UINT16) (ArmSmcArgs.Arg0 >> 16);
  *MinorVer = (UINT16) (ArmSmcArgs.Arg0);
  DEBUG ((DEBUG_INFO, "  PBF Version: %d.%d\n", *MajorVer, *MinorVer));
}

/**
  Get PPB(Processor Base Firmware) Description.

  @param[in, out] DescrTableBase    A pointer to the PBF Description table base address.

  @param[in, out] DescrTableSize    A pointer to the PBF Description table size.

  @retval PSSI_SUCCESS               Get PBF Description sucessful.

  @retval PSSI_INVALID_PARAMETER    The DescrTableSize too small for the result.
                                    DescrTableSize has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetPbfVersionDescription (
  IN OUT VOID    *DescrTableBase,
  IN OUT UINT64  *DescrTableSize
  )
{
  ARM_SMC_ARGS  ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PBF_VERSION_DESCR;
  ArmSmcArgs.Arg1 = (UINT64)DescrTableBase;
  ArmSmcArgs.Arg2 = *DescrTableSize;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *DescrTableSize = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Print ChipId.

  @param[in] HighBytes      Chip Id HighField.

  @param[in] LowBytes       Chip Id LowField.

**/
VOID
PrintChipId (
  IN UINT64 HighBytes,
  IN UINT64 LowBytes
  )
{
  UINT8 Buffer[17];

  ZeroMem (Buffer, sizeof (Buffer));
  CopyMem (Buffer, &LowBytes, 8);
  CopyMem (&Buffer[8], &HighBytes, 8);
  DEBUG ((DEBUG_INFO, "  CHIP ID:%a\n", Buffer));
}

/**
  Get cpu version.

  @param[in]     SocketId       Socket Id in the system.

  @param[in, out] CpuVersion    A pointer to the cpu version.

  @retval PSSI_SUCCESS           Get cpu version sucessful.

  @retval PSSI_UNSUPPORT        Unsupport this call.
**/
PSSI_STATUS
PssiGetCpuVersion (
  IN     UINT8               SocketId,
  IN OUT PHYTIUM_CPU_VERSION *CpuVersion
  )
{
  ARM_SMC_ARGS    ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_CPU_VERSION;
  ArmSmcArgs.Arg1 = SocketId;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_SUCCESS == ArmSmcArgs.Arg0) {
    CpuVersion->CpuType = ArmSmcArgs.Arg1;
    CpuVersion->CpuId[0] = ArmSmcArgs.Arg2; //LowField
    CpuVersion->CpuId[1] = ArmSmcArgs.Arg3; //HighField
    DEBUG ((DEBUG_INFO, "========Cpu Version========\n"));
    DEBUG ((DEBUG_INFO, "    CpuType:0x%x \n", CpuVersion->CpuType));
    PrintChipId (CpuVersion->CpuId[1], CpuVersion->CpuId[0]);
  }

  // Currently(PBF 0.85 + UEFI 1.4.0-25) this
  // interface works not very well, so update the
  // CPU type by parameter table
  GetCpuTypeFromParTable(&CpuVersion->CpuType);
  DEBUG ((DEBUG_INFO, "========Cpu Version Updated========\n"));
  DEBUG ((DEBUG_INFO, "    CpuType:0x%x \n", CpuVersion->CpuType));

  return ArmSmcArgs.Arg0;
}

/**
  Get cpu maps information.

  @param[in, out]     CpuMapInfo    A pointer to the cpu maps struct

  @param[in, out]     Size          A pointer to the size of cpu maps struct.

  @retval PSSI_SUCCESS               Get cpu maps information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetCpuMapsInfo (
  IN OUT PHYTIUM_CPU_MAP_INFO *CpuMapInfo,
  IN OUT UINTN *Size
  )
{
  UINT8           MapsCount;
  UINT64          *CpuMap;
  UINT64          Index;
  ARM_SMC_ARGS    ArmSmcArgs;

  if ((CpuMapInfo == NULL) || (Size == NULL)) {
    return PSSI_INVALID_PARAMETER;
  }

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_CPU_CORE_MAPS;
  ArmSmcArgs.Arg1 = (UINTN) (CpuMapInfo);
  ArmSmcArgs.Arg2 = *Size;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 == 0) {
    CpuMap = &CpuMapInfo->CpuMap[0];
    MapsCount = (UINT8)CpuMapInfo->CpuMapCount;
    DEBUG ((DEBUG_INFO, "======== Cpu Core Maps ========\n"));
    DEBUG ((DEBUG_INFO, "MapsCount:%x\n", MapsCount));
    for (Index = 0; Index < MapsCount; Index++) {
      DEBUG ((DEBUG_INFO, "      CoreMap[%d] Maps: 0x%x\n", Index, CpuMap[Index]));
    }
  } else if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *Size = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get cpu maps information.

  @param[in, out]   CpuConfigInfo   A pointer to the cpu config struct

  @param[in, out]   Size            A pointer to the size of cpu config struct.

  @retval PSSI_SUCCESS               Get cpu config information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetCpuConfigInfo (
  IN OUT PHYTIUM_CPU_CONFIG_INFO *CpuConfigInfo,
  IN OUT UINTN *Size
  )
{
  ARM_SMC_ARGS    ArmSmcArgs;

  if ((CpuConfigInfo == NULL) || (Size == NULL)) {
    return  PSSI_INVALID_PARAMETER;
  }

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_CPU_CORE_CONF;
  ArmSmcArgs.Arg1 = (UINTN) (CpuConfigInfo);
  ArmSmcArgs.Arg2 = *Size;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 == 0) {
    DEBUG ((DEBUG_INFO, "========Cpu Config Info========\n"));
    DEBUG ((DEBUG_INFO, "        Cpu Frequency: %d(Hz)\n", CpuConfigInfo->CpuFreq));
    DEBUG ((DEBUG_INFO, "        Cpu L3 Cache Size: %d(bytes)\n", CpuConfigInfo->CpuL3CacheSize));
    DEBUG ((DEBUG_INFO, "        Cpu L3 Cache Line Size: %d(bytes)\n", CpuConfigInfo->CpuL3CacheLineSize));
  } else if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *Size = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get memory regions information.

  @param[in, out]     MemoryInfo    A pointer to the memory regions info struct.

  @param[in, out]     Size          A pointer to the size of memory regions info struct.

  @retval PSSI_SUCCESS               Get memory regions information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetMemRegions (
  IN OUT PHYTIUM_MEMORY_INFO *MemoryInfo,
  IN OUT UINTN *Size
  )
{
  PHYTIUM_MEMORY_BLOCK          *MemBlock;
  ARM_SMC_ARGS                  ArmSmcArgs;
  UINT64                        MemBlockCnt;
  UINT32                        Index;

  MemBlock = NULL;
  MemBlockCnt = 0;
  Index      = 0;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_MEM_REGIONS;
  ArmSmcArgs.Arg1 = (UINTN)MemoryInfo;
  ArmSmcArgs.Arg2 = *Size;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 == 0) {
    MemBlockCnt = MemoryInfo->MemBlockCount;
    MemBlock = MemoryInfo->MemBlock;
    DEBUG ((DEBUG_INFO, "======Memory Block Info========\n"));
    DEBUG ((DEBUG_INFO, "    Memory Block Count:%d\n", MemBlockCnt));
    DEBUG ((DEBUG_INFO, "    MemBlock:0x%p\n", MemBlock));
    for (Index = 0; Index < MemBlockCnt; Index++) {
      DEBUG ((DEBUG_INFO, "    MemBlock:0x%p\n", MemBlock));
      DEBUG ((DEBUG_INFO, "    Node[%d]:[0x%lx - 0x%lx]\n",
        MemBlock->MemNodeId,
        MemBlock->MemStart,
        MemBlock->MemStart + MemBlock->MemSize - 1));
      MemBlock++;
    }
  } else if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *Size = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get pci controller information.

  @param[in, out]    PciController  A pointer to the pci controller struct.

  @param[in, out]    Size           A pointer to the size of pci controller struct.

  @retval PSSI_SUCCESS               Get pci controller information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetPciControllerInfo (
  IN OUT PHYTIUM_PCI_CONTROLLER *PciController,
  IN OUT UINTN *Size
  )
{
  PHYTIUM_PCI_BLOCK    *PciBlock;
  ARM_SMC_ARGS         ArmSmcArgs;
  UINT8                Index;

  Index = 0;
  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PCI_CONTROLLER;
  ArmSmcArgs.Arg1 = (UINTN)PciController;
  ArmSmcArgs.Arg2 = *Size;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 == 0) {
    DEBUG ((DEBUG_INFO, "======Pci Controller Info========\n"));
    DEBUG ((DEBUG_INFO, "Pci Controller Count:%d\n", PciController->PciCount));
    PciBlock = PciController->PciBlock;
    for (Index = 0; Index < PciController->PciCount; Index ++) {
      DEBUG ((DEBUG_ERROR, "Controller[%d] PciLane:%d, PciSpeed:%d\n",
        Index, PciBlock->PciLane, PciBlock->PciSpeed));
      PciBlock ++;
    }
  } else if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *Size = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get pci hostbridge information.

  @param[in, out]    PciHostBridge  A pointer to the pci hostbridge struct.

  @param[in, out]    Size           A pointer to the size of pci hostbridge struct.

  @retval PSSI_SUCCESS               Get pci hostbridge information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetPciHostBridgeInfo (
  IN OUT PHYTIUM_PCI_HOST_BRIDGE *PciHostBridge,
  IN OUT UINTN *Size
  )
{
  UINT8                     Index;
  UINT64                    PciHostCount;
  ARM_SMC_ARGS              ArmSmcArgs;
  PHYTIUM_PCI_HOST_BLOCK    *PciHostBlock;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PCI_HOST_BRIDGE;
  ArmSmcArgs.Arg1 = (UINTN)PciHostBridge;
  ArmSmcArgs.Arg2 = *Size;
  ArmCallSmc (&ArmSmcArgs);

  if (ArmSmcArgs.Arg0 == 0) {
    PciHostCount = PciHostBridge->PciHostCount;
    PciHostBlock = PciHostBridge->PciHostBlock;
    DEBUG ((DEBUG_INFO, "========Pci Host Bridge========\n"));
    DEBUG ((DEBUG_INFO, "PciHostCount:%d\n", PciHostCount));
    for (Index = 0; Index < PciHostCount; Index++) {
      DEBUG ((DEBUG_INFO, "       PciHostIndex:%d\n", Index));
      DEBUG ((DEBUG_INFO, "           BusStart:%d\n", PciHostBlock->BusStart));
      DEBUG ((DEBUG_INFO, "           BusEnd:%d\n", PciHostBlock->BusEnd));
      DEBUG ((DEBUG_INFO, "           PciConfigBase:0x%lx\n", PciHostBlock->PciConfigBase));
      DEBUG ((DEBUG_INFO, "           IoBase:0x%lx\n", PciHostBlock->IoBase));
      DEBUG ((DEBUG_INFO, "           IoSize:0x%lx\n", PciHostBlock->IoSize));
      DEBUG ((DEBUG_INFO, "           Mem32Base:0x%lx\n", PciHostBlock->Mem32Base));
      DEBUG ((DEBUG_INFO, "           Mem32Size:0x%lx\n", PciHostBlock->Mem32Size));
      DEBUG ((DEBUG_INFO, "           Mem64Base:0x%lx\n", PciHostBlock->Mem64Base));
      DEBUG ((DEBUG_INFO, "           Mem64Size:0x%lx\n", PciHostBlock->Mem64Size));
      DEBUG ((DEBUG_INFO, "           IntA:%d\n", PciHostBlock->IntA));
      DEBUG ((DEBUG_INFO, "           IntB:%d\n", PciHostBlock->IntB));
      DEBUG ((DEBUG_INFO, "           IntC:%d\n", PciHostBlock->IntC));
      DEBUG ((DEBUG_INFO, "           IntD:%d\n", PciHostBlock->IntD));
    }
  } else if (PSSI_INVALID_PARAMETER == ArmSmcArgs.Arg0) {
    *Size = ArmSmcArgs.Arg2;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get system firmware base address on flash device.

  @param[in, out]    Base           A pointer to the system firmware base adress.

  @retval PSSI_SUCCESS               Get system firmware base address sucessful.
**/
PSSI_STATUS
PssiGetSfwBase (
  IN OUT UINT64 *Base
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_GET_SFW_BASE;
  ArmCallSmc (&ArmSmcArgs);

  if (0 == ArmSmcArgs.Arg0) {
    *Base = ArmSmcArgs.Arg1;
    DEBUG ((DEBUG_INFO, "========System Firmware Base========\n"));
    DEBUG ((DEBUG_INFO, "  System Firmware Base: 0x%lx\n", *Base));
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get config table base address on flash device.

  @param[in, out]    Base           A pointer to the config table base adress.

  @retval PSSI_SUCCESS               Get system firmware base address sucessful.

  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiGetConfigTableBase (
  IN OUT UINT64 *Base
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_GET_CFGTBL_INFO;
  ArmCallSmc (&ArmSmcArgs);

  if (0 == ArmSmcArgs.Arg0) {
    *Base = ArmSmcArgs.Arg1;
    DEBUG ((DEBUG_INFO, "========Config Table Base========\n"));
    DEBUG ((DEBUG_INFO, "  Config Table Base: 0x%lx\n", *Base));
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get boot mode.

  @param[in, out]    BootMode       A pointer to the boot mode.

  @retval PSSI_SUCCESS               Get boot mode sucessful.

  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiGetBootMode (
  IN OUT UINT64 *BootMode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_GET_RESET_MODE;
  ArmCallSmc (&ArmSmcArgs);

  if (0 == ArmSmcArgs.Arg0) {
    *BootMode = ArmSmcArgs.Arg1;
    DEBUG ((DEBUG_INFO, "========Boot Mode========\n"));
    switch (*BootMode) {
      case (0x55):
        DEBUG ((DEBUG_INFO, "  S3 resume\n"));
        break;
      case (0xAA):
        DEBUG ((DEBUG_INFO, "  S3 timeout resume\n"));
        break;
      default:
        DEBUG ((DEBUG_INFO, "  Unknown boot mode\n"));
        ASSERT (FALSE);
    }
  }

  return ArmSmcArgs.Arg0;
}

/**
  Set S3 suspend timer,system will resume form S3 when
  timer time out.

  @param[in]     TimeOut            The number of mimutes to set the S3 suspend timer to.
                                    A value of zero  disables the timer.

  @retval PSSI_SUCCESS               Set S3 suspend timer sucessful.
  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiSetSuspendTimeOut (
  IN UINT64 TimeOut
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_SET_SUSPEND_TIMEOUT;
  ArmSmcArgs.Arg1 = TimeOut;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Register PFDI(Phytium Firmware Dispatch Interface)

  @param[in]     PfdiTable          PFDI table

  @retval PSSI_SUCCESS               Register PFDI sucessful.
**/
PSSI_STATUS
PssiPfdiRegister (
  IN VOID *PfdiTable
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PFDI_REGISTER;
  ArmSmcArgs.Arg1 = (UINTN)PfdiTable;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  System firmware call this service to return PBF.

  @retval Nomal no retun, return 0 on error.
**/
PSSI_STATUS
PssiPfdiDone (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PFDI_DONE;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}


/**
  Get max version of parameter table.

  @param[in] ParameterId     Parameter table id.

  @param[in, out] MajorVer   A pointer to the Major version.

  @param[in, out] MinorVer   A pointer to the Minor version.

**/
VOID
PssiGetParameterVersion (
  IN     UINT8  ParameterId,
  IN OUT UINT16 *MajorVer,
  IN OUT UINT16 *MinorVer
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_GET_PARAMETER_VERSION;
  ArmSmcArgs.Arg1 = ParameterId;
  ArmCallSmc (&ArmSmcArgs);

  // bit[16:31] Major version, BIT[0:15] Minor version
  *MajorVer = (UINT16)(ArmSmcArgs.Arg0 >> 16);
  *MinorVer = (UINT16)(ArmSmcArgs.Arg0);
  DEBUG ((DEBUG_INFO, "========Parameter Version========\n"));
  DEBUG ((DEBUG_INFO, "  Parameter Version: %d.%d\n", *MajorVer, *MinorVer));
}

/**
  Get reset source.

  @retval   Reset source.
**/
UINT32
PssiGetResetSource (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_GET_RST_SOURCE;
  ArmCallSmc (&ArmSmcArgs);

  DEBUG ((DEBUG_INFO, "========Reset Source========\n"));
  switch (ArmSmcArgs.Arg0) {
    case (0x1):
      DEBUG ((DEBUG_INFO, "  PowerOn Reset\n"));
      break;
    case (0x4):
      DEBUG ((DEBUG_INFO, "  SoftWare Warm Reset\n"));
      break;
    case (0x80):
      DEBUG ((DEBUG_INFO, "  Watchdog Reset\n"));
      break;
    default:
      DEBUG ((DEBUG_INFO, "  Unknown Reset Source\n"));
      ASSERT (FALSE);
  }

  return ArmSmcArgs.Arg0;
}

/**
  Set chip frequency.

  @param[in]      Parameter   A pointer to the parameter table.

  @param[in, out]  ErrorCode  A pointer to the Error Code.

  @retval PSSI_SUCCESS         Set frequency sucessful.

  @retval PSSI_DENIED         Frequency error.
**/
PSSI_STATUS
PssiPllInit (
  IN     VOID   *Parameter,
  IN OUT UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PLL_INIT;
  ArmSmcArgs.Arg1 = 0x0;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_DENIED == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Init pcie controller.

  @param[in]    Parameter     A pointer to the parameter table.

  @param[in, out] ErrorCode   A pointer to the Error Code.

  @retval PSSI_SUCCESS         Pcie controller init successful.

  @retval PSSI_INIT_FAIL      Pcie controller init fail.
**/
PSSI_STATUS
PssiPcieInit (
  IN     VOID   *Parameter,
  IN OUT UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PCIE_INIT;
  ArmSmcArgs.Arg1 = 0x0;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Init DDR controller.

  @param[in]      Parameter     A pointer to the parameter table.

  @param[in, out]  ErrorCode    A pointer to the Error Code.

  @retval PSSI_SUCCESS           DDR controller init successful.

  @retval PSSI_INIT_FAIL        DDR controller init fail.
**/
PSSI_STATUS
PssiDdrInit (
  IN      VOID   *Parameter,
  IN OUT  UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS      ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_INIT;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Set DDR self-refresh dgree.

  @param[in] Dgree          DDR self-refresh Dgree.

  @retval PSSI_SUCCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiSetDdrSelfRefreshDgree (
  IN UINT64 Dgree
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_ENTRY;
  ArmSmcArgs.Arg2 = Dgree;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR self-refresh lock.

  @retval PSSI_SUCCESS       lock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshLock (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_LOCK;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR self-refresh unlock.

  @retval PSSI_SUCCESS       Unlock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshUnLock (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_UNLOCK;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set DDR secure regions, enable TZC required.

  @param[in]    Parameter   A pointer to the parameter table.

  @retval PSSI_SUCCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiDdrSecureInit (
  IN VOID *Parameter
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_SECURE_INIT;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Print DDR training limit value.

  @retval PSSI_SUCCESS       Print DDR training limit value successful.
**/
PSSI_STATUS
PssiPrintTrainingLimit (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_PRINT_TRAIN_WIN;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Clear memory region start startAddr ~ 2^AddrSapce

  @param[in]    AddrSpace   Clear memory region address space.

  @param[in]    StartAddr   Clear memory region start address.

  @retval PSSI_SUCCESS       Clear memory successful.
**/
PSSI_STATUS
PssiClearMemory (
  IN UINT64 AddrSpace,
  IN UINT64 StartAddr
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_CLEAR_MEM;
  ArmSmcArgs.Arg2 = AddrSpace;
  ArmSmcArgs.Arg3 = StartAddr;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Check DDR training information.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out] TrainingInfo       DDR training information.

  @retval PSSI_SUCCESS                Check DDR training information successful.
**/
PSSI_STATUS
PssiTrainingCheck (
  IN     VOID   *Parameter,
  IN     UINT64 TrainingParameter,
  IN OUT VOID   *TrainingInfo
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = TRAINING_CHECK;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmSmcArgs.Arg3 = TrainingParameter;
  ArmCallSmc (&ArmSmcArgs);

  if (!PSSI_ERROR (ArmSmcArgs.Arg0)) {
    TrainingInfo = (VOID *)ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Skip DDR training progress, init DDR controller fast.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCCESS                Skip DDR training information successful.
**/
PSSI_STATUS
PssiSkipTraning (
  IN      VOID   *Parameter,
  IN      VOID   *TrainingParameter,
  IN OUT  UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_INIT_SKIP_TRAIN;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmSmcArgs.Arg3 = (UINT64)TrainingParameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  DDR CTL bist test.

  @param[in]    Channel              Channel of DDR to be start bist test.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCCESS                Channel bist test successful.

  @retval PSSI_INIT_FAIL             Channel bist test fail.
**/
PSSI_STATUS
PssiDdrBist (
  IN     UINT64 Channel,
  IN OUT UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = MCU_CTL_BIST_FUN;
  ArmSmcArgs.Arg2 = Channel;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get DDR training parameter.

  @retval Training Parameter base address.
**/
UINT64
PssiGetTrainingParameter (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO;
  ArmSmcArgs.Arg1 = DDR_TRAIN_REG_GET;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Print DDR registers.

  @param[in] Channel    Channel of DDR to be print registers.

  @retval PSSI_SUCCESS   Print DDR registers successful.
**/
PSSI_STATUS
PssiPrintDdrReg (
  IN UINT64 Channel
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO;
  ArmSmcArgs.Arg1 = DDR_REG_PRINT;
  ArmSmcArgs.Arg2 = Channel;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR DQS/DQ connect test.

  @param[in]      Parameter   A pointer to parameter table.

  @param[in, out] Result      Result of connect test.

  @retval PSSI_SUCCESS         Get result of connect test successful.
**/
PSSI_STATUS
PssiDdrConnectTest (
  IN     VOID *Parameter,
  IN OUT VOID *Result
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO;
  ArmSmcArgs.Arg1 = MCU_CONNECT_TEST_FUN;
  ArmSmcArgs.Arg2 = (UINTN)Parameter;
  ArmSmcArgs.Arg3 = (UINTN)Result;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Relocate PBF,must be call after memory init.

  @param[in]      Type        Type of relocate

  @param[in]      Parameter   A pointer to parameter table.

  @retval PSSI_SUCCESS         Relocate PBF successful.
**/
PSSI_STATUS
PssiRelocate (
  IN UINT64 Type,
  IN VOID   *Parameter
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_RELOCATE;
  ArmSmcArgs.Arg1 = Type;
  ArmSmcArgs.Arg2 = (UINTN)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set PBF print rank.

  @param[in]     Rank    Rank of print.

  @retval PSSI_SUCCESS    Set PBF print rank successful.
**/
PSSI_STATUS
PssiSetDebugPrintRank (
  IN UINT64 Rank
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = PRINT_RANK;
  ArmSmcArgs.Arg2 = Rank;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set PBF uart baudrate.

  @param[in]  BaudRate   BaudRate to be set the uart.

  @retval PSSI_SUCCESS    Set PBF uart baudrate successful.
**/
UINTN
PssiDebugSetBaudRate (
  IN UINT64 BaudRate
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = SET_BAUDRATE;
  ArmSmcArgs.Arg2 = BaudRate;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set PBF log temporary storage adress and size.

  @param[in]  LogBase    Log temporary storage adress.

  @param[in]  LogSize    Log temporary storage size.

  @retval PSSI_SUCCESS    Set PBF log init successful.
**/
PSSI_STATUS
PssiDebugLogInit (
  IN UINT64 LogBase,
  IN UINT64 LogSize
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = LOG_INIT;
  ArmSmcArgs.Arg2 = LogBase;
  ArmSmcArgs.Arg3 = LogSize;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Get PBF log temporary storage address.

  @retval Log temporary storage address.
**/
UINT64
PssiDebugGetLogBase (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = GET_LOG_BASE;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Get PBF log temporary storage size.

  @retval Log temporary storage size.
**/
UINT64
PssiDebugGetLogSize (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = GET_LOG_SIZE;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set PBF log rank.

  @param[in]     Rank    Rank of log.

  @retval PSSI_SUCCESS    Set PBF log rank successful.
**/
PSSI_STATUS
PssiDebugSetLogRank (
  IN UINT64 Rank
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = SET_LOG_RANK;
  ArmSmcArgs.Arg2 = Rank;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  SSC config.

  @param[in]    Level    Level of SSC.

  @retval PSSI_SUCCESS    SSC config  successful.
**/
PSSI_STATUS
PssiDebugSscCfg (
  IN UINT64 Level
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = SSC_CONFIG;
  ArmSmcArgs.Arg2 = Level;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Secure config.

  @param[in]    Level    Level of secure.

  @retval PSSI_SUCCESS    secure config  successful.
**/
PSSI_STATUS
PssiSecureCfg (
  IN UINT64 Level
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DEBUG_INIT;
  ArmSmcArgs.Arg1 = SSC_CONFIG;
  ArmSmcArgs.Arg2 = Level;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set OTR(Over temperature recovery) value.

  @param[in]    Temperature  OTR value.

  @retval PSSI_SUCCESS        OTR config successful.
**/
PSSI_STATUS
PssiOtrCfg (
  IN UINT64 Temperature
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_OTP_CFG;
  ArmSmcArgs.Arg1 = 0;
  ArmSmcArgs.Arg2 = Temperature;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Init high speed phy.

  @param[in]    PhyDomain       Domain to be config.

  @param[in]    PhyMultiplex    Domain Multiplex config.

  @param[in]    PhyMode         Mode to be config.

  @param[in]    PhySpeed        Speed to be config.


  @retval PSSI_SUCCESS           Phy config successful.
**/
PSSI_STATUS
PssiPhyInit (
  IN UINT64 PhyDomain,
  IN UINT64 PhyMultiplex,
  IN UINT64 PhyMode,
  IN UINT64 PhySpeed
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_PHY_INIT;
  ArmSmcArgs.Arg1 = PhyDomain;
  ArmSmcArgs.Arg2 = PhyMultiplex;
  ArmSmcArgs.Arg3 = PhyMode;
  ArmSmcArgs.Arg4 = PhySpeed;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Init Chip to Chip module.

  @param[in]      Parameter   A pointer to the parameter table.

  @retval PSSI_SUCCESS         Init C2C sucessful.
**/
PSSI_STATUS
PssiC2cInit (
  IN VOID *Parameter
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_C2C_INIT;
  ArmSmcArgs.Arg1 = 0;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Init DDR controller.

  @param[in]  Parameter          A pointer to the parameter table.

  @param[in]  TrainingParameter  The training parameter table address.

  @param[in, out] ErrorCode      A pointer to the Error Code.

  @retval PSSI_SUCCESS           DDR controller init successful.

  @retval PSSI_INIT_FAIL         DDR controller init fail.
**/
PSSI_STATUS
PssiDdrInitV2 (
  IN      VOID   *Parameter,
  IN      UINT64  TrainingParameter,
  IN OUT  UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS      ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_INIT;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmSmcArgs.Arg3 = (UINT64)TrainingParameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Set DDR self-refresh dgree.

  @param[in] Dgree          DDR self-refresh Dgree.

  @retval PSSI_SUCCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiSetDdrSelfRefreshDgreeV2 (
  IN UINT64 Dgree
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_ENTRY;
  ArmSmcArgs.Arg2 = Dgree;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR self-refresh lock.

  @retval PSSI_SUCCESS       lock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshLockV2 (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_LOCK;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR self-refresh unlock.

  @retval PSSI_SUCCESS       Unlock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshUnLockV2 (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_SUSPEND_UNLOCK;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Set DDR secure regions, enable TZC required.

  @param[in]    Parameter   A pointer to the parameter table.

  @retval PSSI_SUCCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiDdrSecureInitV2 (
  IN VOID *Parameter
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_SECURE_INIT;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Print DDR training limit value.

  @retval PSSI_SUCCESS       Print DDR training limit value successful.
**/
PSSI_STATUS
PssiPrintTrainingLimitV2 (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_PRINT_TRAIN_WIN;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Clear memory region start startAddr ~ 2^AddrSapce

  @param[in]    AddrSpace   Clear memory region address space.

  @param[in]    StartAddr   Clear memory region start address.

  @retval PSSI_SUCCESS       Clear memory successful.
**/
PSSI_STATUS
PssiClearMemoryV2 (
  IN UINT64 AddrSpace,
  IN UINT64 StartAddr
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_CLEAR_MEM;
  ArmSmcArgs.Arg2 = AddrSpace;
  ArmSmcArgs.Arg3 = StartAddr;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Check DDR training information.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out] TrainingInfo       DDR training information.

  @retval PSSI_SUCCESS                Check DDR training information successful.
**/
PSSI_STATUS
PssiTrainingCheckV2 (
  IN     VOID   *Parameter,
  IN     UINT64 TrainingParameter,
  IN OUT VOID   *TrainingInfo
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = TRAINING_CHECK;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmSmcArgs.Arg3 = TrainingParameter;
  ArmCallSmc (&ArmSmcArgs);

  if (!PSSI_ERROR (ArmSmcArgs.Arg0)) {
    TrainingInfo = (VOID *)ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Skip DDR training progress, init DDR controller fast.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCCESS                Skip DDR training information successful.
**/
PSSI_STATUS
PssiSkipTraningV2 (
  IN VOID       *Parameter,
  IN UINT64      TrainingParameter,
  IN OUT UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = DDR_INIT_SKIP_TRAIN_V2;
  ArmSmcArgs.Arg2 = (UINT64)Parameter;
  ArmSmcArgs.Arg3 = (UINT64)TrainingParameter;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  DDR CTL bist test.

  @param[in]    Channel              Channel of DDR to be start bist test.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCCESS                Channel bist test successful.

  @retval PSSI_INIT_FAIL             Channel bist test fail.
**/
PSSI_STATUS
PssiDdrBistV2 (
  IN UINT64     Channel,
  IN OUT UINT64 *ErrorCode
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES_V2;
  ArmSmcArgs.Arg1 = MCU_CTL_BIST_FUN_V2;
  ArmSmcArgs.Arg2 = Channel;
  ArmCallSmc (&ArmSmcArgs);

  if (PSSI_INIT_FAIL == ArmSmcArgs.Arg0) {
    *ErrorCode = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Get DDR training parameter.

  @retval Training Parameter base address.
**/
UINT64
PssiGetTrainingParameterV2 (
  VOID
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO_V2;
  ArmSmcArgs.Arg1 = DDR_TRAIN_REG_GET;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Print DDR registers.

  @param[in] Channel    Channel of DDR to be print registers.

  @retval PSSI_SUCCESS   Print DDR registers successful.
**/
PSSI_STATUS
PssiPrintDdrRegV2 (
  IN UINT64 Channel
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO_V2;
  ArmSmcArgs.Arg1 = DDR_REG_PRINT;
  ArmSmcArgs.Arg2 = Channel;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  DDR DQS/DQ connect test.

  @param[in]      Parameter   A pointer to parameter table.

  @param[in, out]  Result     Result of connect test.

  @retval PSSI_SUCCESS         Get result of connect test successful.
**/
PSSI_STATUS
PssiDdrConnectTestV2 (
  IN     VOID *Parameter,
  IN OUT VOID *Result
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO_V2;
  ArmSmcArgs.Arg1 = MCU_CONNECT_TEST_FUN;
  ArmSmcArgs.Arg2 = (UINTN)Parameter;
  ArmSmcArgs.Arg3 = (UINTN)Result;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Print digtial eye chart

  @param[in]      EyeMode     Eye chart mode

  @param[in]      Channel     Channel to be print digtial eye chart

  @retval PSSI_SUCCESS         Print eye chart  successful.
**/
PSSI_STATUS
PssiDdrPrintEye (
  IN UINT64 EyeMode,
  IN UINT64 Channel
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_GET_INFO_V2;
  ArmSmcArgs.Arg1 = PRINT_EYE;
  ArmSmcArgs.Arg2 = EyeMode;
  ArmSmcArgs.Arg3 = Channel;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Scrub memory region

  @param[in]    ScrubOp     A pointer to scrub opcode struct.

  @param[in]    Size        Size of scrub opcode struct.

  @retval PSSI_SUCCESS       Clear memory successful.
**/
PSSI_STATUS
PssiScrubMemory (
  IN SCRUB_OPCODE *ScrubOp,
  IN UINT64 Size
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_DDR_INIT_SERVICES;
  ArmSmcArgs.Arg1 = DDR_SCRUB_MEM;
  ArmSmcArgs.Arg2 = (UINT64)ScrubOp;
  ArmSmcArgs.Arg3 = Size;
  ArmCallSmc (&ArmSmcArgs);

  return ArmSmcArgs.Arg0;
}

/**
  Config Smmu, applicable to E2000 platform.

  @param[in]    DomainBitMap  Smmu domain bitmap.
  @param[in]    Op            Operation Mode.
  @param[out]   Status        Smmu current config.

  @retval PSSI_SUCCESS        Clear memory successful.
**/
PSSI_STATUS
PssiSmmuConfigV1 (
  IN  UINT64        DomainBitMap,
  IN  SMMU_OP_MODE  Op,
  OUT UINT64        *Status
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_SMMU_CFG_V1;
  ArmSmcArgs.Arg1 = DomainBitMap;
  ArmSmcArgs.Arg2 = Op;
  ArmCallSmc (&ArmSmcArgs);

  if (NULL != Status) {
    *Status = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}

/**
  Config Smmu, applicable to Pd2308 platform.

  @param[in]    DomainBitMap  Smmu domain bitmap.
  @param[in]    Op            Operation Mode.
  @param[out]   Status        Smmu current config.

  @retval PSSI_SUCCESS       Clear memory successful.
**/
PSSI_STATUS
PssiSmmuConfigV2 (
  IN  UINT64        DomainBitMap,
  IN  SMMU_OP_MODE  Op,
  OUT UINT64        *Status
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;

  ZeroMem (&ArmSmcArgs, sizeof (ARM_SMC_ARGS));
  ArmSmcArgs.Arg0 = PSSI_SMMU_CFG_V2;
  ArmSmcArgs.Arg1 = DomainBitMap;
  ArmSmcArgs.Arg2 = Op;
  ArmCallSmc (&ArmSmcArgs);

  if (NULL != Status) {
    *Status = ArmSmcArgs.Arg1;
  }

  return ArmSmcArgs.Arg0;
}
