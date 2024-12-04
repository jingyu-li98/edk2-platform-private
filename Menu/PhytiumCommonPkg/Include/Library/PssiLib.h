/** @file
pssi libary header file.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PSSI_LIB_H_
#define PSSI_LIB_H_

#include <Uefi/UefiBaseType.h>

#pragma pack(1)

//
//Phytium Cpu Map Info
//
typedef struct {
  UINT64     CpuMapCount;
  UINT64     CpuMap[1];
} PHYTIUM_CPU_MAP_INFO;

//
//Phytium Cpu Config Info
//
typedef struct {
  UINT64     CpuFreq;             // Hz
  UINT64     CpuL3CacheSize;      // Byte
  UINT64     CpuL3CacheLineSize;  // Byte
} PHYTIUM_CPU_CONFIG_INFO;

//
//Phytium Cpu Version
//
typedef struct {
  UINT64 CpuType;
  UINT64 CpuId[2];
} PHYTIUM_CPU_VERSION;

//
//Phytium Memory Block
//
typedef struct {
  UINT64     MemStart;
  UINT64     MemSize;
  UINT64     MemNodeId;
} PHYTIUM_MEMORY_BLOCK;

//
//Phytium Memory Info
//
typedef struct {
  UINT64               MemBlockCount;
  PHYTIUM_MEMORY_BLOCK MemBlock[1];
} PHYTIUM_MEMORY_INFO;

//
//Phytium Pci Block
//
typedef struct {
  UINT8    PciLane;
  UINT8    PciSpeed;
  UINT8    Reserved[6];
} PHYTIUM_PCI_BLOCK;

//
//Phytium Pci Controller
//
typedef struct {
  UINT64               PciCount;
  PHYTIUM_PCI_BLOCK    PciBlock[1];
} PHYTIUM_PCI_CONTROLLER;

//
//Phytium Pci Host Block
//
typedef struct {
  UINT8    BusStart;
  UINT8    BusEnd;
  UINT8    Reserved[6];
  UINT64   PciConfigBase;
  UINT64   IoBase;
  UINT64   IoSize;
  UINT64   Mem32Base;
  UINT64   Mem32Size;
  UINT64   Mem64Base;
  UINT64   Mem64Size;
  UINT16   IntA;
  UINT16   IntB;
  UINT16   IntC;
  UINT16   IntD;
} PHYTIUM_PCI_HOST_BLOCK;

//
//Phytium Pci Host Bridge
//
typedef struct {
  UINT64                  PciHostCount;
  PHYTIUM_PCI_HOST_BLOCK  PciHostBlock[1];
} PHYTIUM_PCI_HOST_BRIDGE;

//
//Scrub Operation Code
//
typedef struct {
  UINT32 Mode;
  UINT32 IntervalPeriod;
  UINT64 StartAddr;
  UINT64 EndAddr;
} SCRUB_OPCODE;

#pragma pack ()


typedef UINT64 PSSI_STATUS;

typedef enum {
  SmmuRead,
  SmmuBypass,
  SmmuEnable
} SMMU_OP_MODE;

/**
  Returns TRUE if a specified StatusCode is an error code.

  This function returns TRUE if StatusCode has the high bit set.  Otherwise, FALSE is returned.

  @param  StatusCode    The status code value to evaluate.

  @retval TRUE          The high bit of StatusCode is set.
  @retval FALSE         The high bit of StatusCode is clear.

**/
#define PSSI_ERROR(StatusCode)  (((INTN)(PSSI_STATUS)(StatusCode)) < 0)



#define PSSI_SUCCESS               (0)
#define PSSI_UNSUPPORT             (-1)
#define PSSI_INVALID_PARAMETER     (-2)
#define PSSI_DENIED                (-3)
#define PSSI_ALREADY_ON            (-4)
#define PSSI_ON_PENDING            (-5)
#define PSSI_INTERNAL_FAILURE      (-6)
#define PSSI_NOT_PRESENT           (-7)
#define PSSI_DISABLED              (-8)
#define PSSI_INVALID_ADDRESS       (-9)
#define PSSI_INIT_FAIL             (-10)
#define PSSI_SERVICE_ERROR         (-11)
#define PSSI_VERSION_ERROR         (-12)


#define PSSI_VERSION                0x8200FF03
#define PSSI_PBF_VERSION            0x82000001
#define PSSI_CPU_VERSION            0xC2000002
#define PSSI_CPU_CORE_MAPS          0xC2000003
#define PSSI_CPU_CORE_CONF          0xC2000004
#define PSSI_MEM_REGIONS            0xC2000005
#define PSSI_PCI_CONTROLLER         0xC2000007
#define PSSI_PCI_HOST_BRIDGE        0xC2000008
#define PSSI_GET_SFW_BASE           0xC2000009
#define PSSI_GET_CFGTBL_INFO        0xC200000A
#define PSSI_GET_RESET_MODE         0xC200000B
#define PSSI_SET_SUSPEND_TIMEOUT    0xC2000010
#define PSSI_PFDI_REGISTER          0xC2000012
#define PSSI_PFDI_DONE              0xC2000013
#define PSSI_PBF_VERSION_DESCR      0xC2000014
#define PSSI_PHY_INIT               0xC2000015

#define PSSI_GET_PARAMETER_VERSION  0xC2000F00
#define PSSI_GET_RST_SOURCE         0xC2000F01
#define PSSI_PLL_INIT               0xC2000F02
#define PSSI_PCIE_INIT              0xC2000F03
#define PSSI_DDR_INIT_SERVICES      0xC2000F04
#define PSSI_RELOCATE               0xC2000F05
#define PSSI_DEBUG_INIT             0xC2000F06
#define PSSI_SECURITY_CFG           0xC2000F07
#define PSSI_OTP_CFG                0xC2000F08
#define PSSI_C2C_INIT               0xC2000F09
#define PSSI_DDR_GET_INFO           0xC2000F14

#define PSSI_DDR_INIT_SERVICES_V2   0xC2010F04
#define PSSI_DDR_GET_INFO_V2        0xC2010F14
#define PSSI_SMMU_CFG_V1            0xC2000F09
#define PSSI_SMMU_CFG_V2            0xC2000F0B

#define DDR_INIT                    0x0
#define DDR_SUSPEND_ENTRY           0x1
#define DDR_SUSPEND_LOCK            0x2
#define DDR_SUSPEND_UNLOCK          0x3
#define DDR_SECURE_INIT             0x6
#define DDR_PRINT_TRAIN_WIN         0x7
#define DDR_CLEAR_MEM               0x8
#define TRAINING_CHECK              0x9
#define DDR_INIT_SKIP_TRAIN         0xA
#define DDR_INIT_SKIP_TRAIN_V2      0x4
#define MCU_CTL_BIST_FUN            0xB
#define MCU_CTL_BIST_FUN_V2         0x5
#define DDR_SCRUB_MEM               0xC

#define DDR_TRAIN_REG_GET           0x0
#define DDR_REG_PRINT               0x1
#define MCU_CONNECT_TEST_FUN        0x2
#define PRINT_EYE                   0x3

#define PRINT_RANK                  0x0
#define SET_BAUDRATE                0x1
#define LOG_INIT                    0x2
#define GET_LOG_BASE                0x3
#define GET_LOG_SIZE                0x4
#define SET_LOG_RANK                0x5
#define SSC_CONFIG                  0x6

//
// PLL init error code
//
#define PLL_CPU_FREQ_ERROR          0x1
#define PLL_DDR_FREQ_ERROR          0x2

//
// Reset source
//
#define RESET_SOURCE_POWER          0x1
#define RESET_SOURCE_PLL            0x4
#define RESET_SOURCE_WDT            0x80

//
// Relocate ID
//
#define RELOCATE_POWERON            0x0
#define RELOCATE_RESUME             0x1

/**
  Get Pssi(Phytium System service interface) version.

  @param[in, out] MajorVer   A pointer to the Major version.

  @param[in, out] MinorVer   A pointer to the Minor version.

**/
VOID
PssiGetPssiVersion (
  IN OUT UINT16 *MajorVer,
  IN OUT UINT16 *MinorVer
  );

/**
  Get PBF(Processor Base Firmware) version.

  @param[in, out] MajorVer   A pointer to the Major version.

  @param[in, out] MinorVer   A pointer to the Minor version.

**/
VOID
PssiGetPbfVersion (
  IN OUT  UINT16 *MajorVer,
  IN OUT  UINT16 *MinorVer
  );

/**
  Get PPB(Processor Base Firmware) Description.

  @param[in, out] DescrTableBase    A pointer to the PBF Description table base address.

  @param[in, out] DescrTableSize    A pointer to the PBF Description table size.

  @retval PSSI_SUCESS               Get PBF Description sucessful.

  @retval PSSI_INVALID_PARAMETER    The DescrTableSize too small for the result.
                                    DescrTableSize has been updated with the size needed to
                                    complete the request.
**/

PSSI_STATUS
PssiGetPbfVersionDescription (
  IN OUT VOID    *DescrTableBase,
  IN OUT UINT64  *DescrTableSize
  );

/**
  Print ChipId

  @param[in] HighBytes      Chip Id HighField

  @param[in] LowBytes       Chip Id LowField

**/
VOID PrintChipId (
  IN UINT64 HighBytes,
  IN UINT64 LowBytes
  );

/**
  Get cpu version.

  @param[in]     SocketId       Socket Id in the system

  @param[in, out] CpuVersion    A pointer to the cpu version.

  @retval PSSI_SUCESS           Get cpu version sucessful.

  @retval PSSI_UNSUPPORT        Unsupport this call.
**/
PSSI_STATUS
PssiGetCpuVersion (
  IN     UINT8               SocketId,
  IN OUT PHYTIUM_CPU_VERSION *CpuVersion
  );

/**
  Get cpu maps information.

  @param[in, out]    CpuMapInfo     A pointer to the cpu maps struct

  @param[in, out]    Size           A pointer to the size of cpu maps struct.

  @retval PSSI_SUCESS               Get cpu maps information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetCpuMapsInfo (
  IN OUT PHYTIUM_CPU_MAP_INFO *CpuMapInfo,
  IN OUT UINTN *Size
  );

/**
  Get cpu maps information.

  @param[in, out]    CpuConfigInfo  A pointer to the cpu config struct

  @param[in, out]    Size           A pointer to the size of cpu config struct.

  @retval PSSI_SUCESS               Get cpu config information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetCpuConfigInfo (
  IN OUT PHYTIUM_CPU_CONFIG_INFO *CpuConfigInfo,
  IN OUT UINTN *Size
  );

/**
  Get memory regions information.

  @param[in, out]    MemoryInfo     A pointer to the memory regions info struct

  @param[in, out]    Size           A pointer to the size of memory regions info struct.

  @retval PSSI_SUCESS               Get memory regions information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetMemRegions (
  IN OUT PHYTIUM_MEMORY_INFO *MemoryInfo,
  IN OUT UINTN *Size
  );

/**
  Get pci controller information.

  @param[in, out]    PciController  A pointer to the pci controller struct.

  @param[in, out]    Size           A pointer to the size of pci controller struct.

  @retval PSSI_SUCESS               Get pci controller information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetPciControllerInfo (
  IN OUT PHYTIUM_PCI_CONTROLLER *PciController,
  IN OUT UINTN *Size
  );

/**
  Get pci hostbridge information.

  @param[in, out]    PciHostBridge  A pointer to the pci hostbridge struct.

  @param[in, out]    Size           A pointer to the size of pci hostbridge struct.

  @retval PSSI_SUCESS               Get pci hostbridge information sucessful.

  @retval PSSI_INVALID_PARAMETER    The Size too small for the result.
                                    Size has been updated with the size needed to
                                    complete the request.
**/
PSSI_STATUS
PssiGetPciHostBridgeInfo (
  IN OUT PHYTIUM_PCI_HOST_BRIDGE *PciHostBridge,
  IN OUT UINTN *Size
  );

/**
  Get system firmware base address on flash device.

  @param[in out]     Base           A pointer to the system firmware base adress.

  @retval PSSI_SUCESS               Get system firmware base address sucessful.
**/
PSSI_STATUS
PssiGetSfwBase (
  IN OUT UINT64 *Base
  );

/**
  Get config table base address on flash device.

  @param[in, out]    Base           A pointer to the config table base adress.

  @retval PSSI_SUCESS               Get system firmware base address sucessful.

  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiGetConfigTableBase (
  IN OUT UINT64 *Base
  );

/**
  Get boot mode.

  @param[in, out]    BootMode       A pointer to the boot mode.

  @retval PSSI_SUCESS               Get boot mode sucessful.

  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiGetBootMode (
  IN OUT UINT64 *BootMode
  );

/**
  Set S3 suspend timer,system will resume form S3 when
  timer time out.

  @param[in]     TimeOut            The number of mimutes to set the S3 suspend timer to.
                                    A value of zero  disables the timer.

  @retval PSSI_SUCESS               Set S3 suspend timer sucessful.
  @retval PSSI_UNSUPPORT            Unspport this call.
**/
PSSI_STATUS
PssiSetSuspendTimeOut (
  IN UINT64 TimeOut
  );

/**
  Register PFDI(Phytium Firmware Dispatch Interface)

  @param[in]     PfdiTable          PFDI table

  @retval PSSI_SUCESS               Register PFDI sucessful.
**/
PSSI_STATUS
PssiPfdiRegister (
  IN VOID *PfdiTable
  );

/**
  System firmware call this service to return PBF.

  @retval Nomal no retun, return 0 on error.
**/
PSSI_STATUS
PssiPfdiDone (
  VOID
  );

/**
  Get max version of parameter table.

  @param[in] ParameterId    Parameter table id.

  @param[in, out] MajorVer  A pointer to the Major version.

  @param[in, out] MinorVer  A pointer to the Minor version.

**/
VOID
PssiGetParameterVersion (
  IN     UINT8  ParameterId,
  IN OUT UINT16 *MajorVer,
  IN OUT UINT16 *MinorVer
  );

/**
  Get reset source.

  @param    none

  @retval   Reset source.
**/
UINT32
PssiGetResetSource (
  VOID
  );

/**
  Set chip frequency.

  @param[in]      Parameter   A pointer to the parameter table.

  @param[in, out]  ErrorCode  A pointer to the Error Code.

  @retval PSSI_SUCESS         Set frequency sucessful.

  @retval PSSI_DENIED         Frequency error.
**/
PSSI_STATUS
PssiPllInit (
  IN     VOID   *Parameter,
  IN OUT UINT64 *ErrorCode
  );

/**
  Init pcie controller.

  @param[in]    Parameter     A pointer to the parameter table.

  @param[in, out]  ErrorCode  A pointer to the Error Code.

  @retval PSSI_SUCESS         Pcie controller init successful.

  @retval PSSI_INIT_FAIL      Pcie controller init fail.
**/
PSSI_STATUS
PssiPcieInit (
  IN     VOID   *Parameter,
  IN OUT UINT64 *ErrorCode
  );

/**
  Init DDR controller.

  @param[in]      Parameter     A pointer to the parameter table.

  @param[in, out] ErrorCode     A pointer to the Error Code.

  @retval PSSI_SUCESS           DDR controller init successful.

  @retval PSSI_INIT_FAIL        DDR controller init fail.
**/
PSSI_STATUS
PssiDdrInit (
  IN      VOID   *Parameter,
  IN OUT  UINT64 *ErrorCode
  );

/**
  Set DDR self-refresh dgree.

  @param[in] Dgree          DDR self-refresh Dgree.

  @retval PSSI_SUCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiSetDdrSelfRefreshDgree (
  IN UINT64 Dgree
  );

/**
  DDR self-refresh lock.

  @retval PSSI_SUCESS       lock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshLock (
  VOID
  );

/**
  DDR self-refresh unlock.

  @retval PSSI_SUCESS       Unlock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshUnLock (
  VOID
  );

/**
  Set DDR secure regions, enable TZC required.

  @param[in]    Parameter   A pointer to the parameter table.

  @retval PSSI_SUCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiDdrSecureInit (
  IN VOID *Parameter
  );

/**
  Print DDR training limit value.

  @retval PSSI_SUCESS       Print DDR training limit value successful.
**/
PSSI_STATUS
PssiPrintTrainingLimit (
  VOID
  );

/**
  Clear memory region start startAddr ~ 2^AddrSapce

  @param[in]    AddrSpace   Clear memory region address space.

  @param[in]    StartAddr   Clear memory region start address.

  @retval PSSI_SUCESS       Clear memory successful.
**/
PSSI_STATUS
PssiClearMemory (
  IN UINT64 AddrSpace,
  IN UINT64 StartAddr
  );

/**
  Check DDR training information.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out] TrainingInfo       DDR training information.

  @retval PSSI_SUCESS                Check DDR training information successful.
**/
PSSI_STATUS
PssiTrainingCheck (
  IN     VOID   *Parameter,
  IN     UINT64 TrainingParameter,
  IN OUT VOID   *TrainingInfo
  );

/**
  Skip DDR training progress, init DDR controller fast.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TrainingParameter    DDR training parameter.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCESS                Skip DDR training information successful.
**/
PSSI_STATUS
PssiSkipTraning (
  IN     VOID   *Parameter,
  IN     VOID   *TrainingParameter,
  IN OUT UINT64 *ErrorCode
  );

/**
  DDR CTL bist test.

  @param[in]    Channel              Channel of DDR to be start bist test.

  @param[in, out]  ErrorCode         A pointer to the Error Code.

  @retval PSSI_SUCESS                Channel bist test successful.

  @retval PSSI_INIT_FAIL             Channel bist test fail.
**/
PSSI_STATUS
PssiDdrBist (
  IN UINT64     Channel,
  IN OUT UINT64 *ErrorCode
  );

/**
  Get DDR training parameter.

  @retval Training Parameter base address.
**/
UINT64
PssiGetTrainingParameter (
  VOID
  );

/**
  Print DDR registers.

  @param[in] Channel    Channel of DDR to be print registers.

  @retval PSSI_SUCESS   Print DDR registers successful.
**/
PSSI_STATUS
PssiPrintDdrReg (
  IN UINT64 Channel
  );

/**
  DDR DQS/DQ connect test.

  @param[in]      Parameter   A pointer to parameter table.

  @param[in out]  Result      Result of connect test.

  @retval PSSI_SUCESS         Get result of connect test successful.
**/
PSSI_STATUS
PssiDdrConnectTest (
  IN VOID     *Parameter,
  IN OUT VOID *Result
  );

/**
  Relocate PBF,must be call after memory init.

  @param[in]      Type        Type of relocate

  @param[in]      Parameter   A pointer to parameter table.

  @retval PSSI_SUCESS         Relocate PBF successful.
**/
PSSI_STATUS
PssiRelocate (
  IN UINT64 Type,
  IN VOID   *Parameter
  );

/**
  Set PBF print rank.

  @param[in]     Rank    Rank of print.

  @retval PSSI_SUCESS    Set PBF print rank successful.
**/
PSSI_STATUS
PssiSetDebugPrintRank (
  IN UINT64 Rank
  );

/**
  Set PBF uart baudrate.

  @param[in]  BaudRate   BaudRate to be set the uart.

  @retval PSSI_SUCESS    Set PBF uart baudrate successful.
**/
UINTN
PssiDebugSetBaudRate (
  IN UINT64 BaudRate
  );

/**
  Set PBF log temporary storage adress and size.

  @param[in]  LogBase    Log temporary storage adress.

  @param[in]  LogSize    Log temporary storage size.

  @retval PSSI_SUCESS    Set PBF log init successful.
**/
PSSI_STATUS
PssiDebugLogInit (
  IN UINT64 LogBase,
  IN UINT64 LogSize
  );

/**
  Get PBF log temporary storage address.

  @retval Log temporary storage address.
**/
UINT64
PssiDebugGetLogBase (
  VOID
  );

/**
  Get PBF log temporary storage size.

  @retval Log temporary storage size.
**/
UINT64
PssiDebugGetLogSize (
  VOID
  );

/**
  Set PBF log rank.

  @param[in]     Rank    Rank of log.

  @retval PSSI_SUCESS    Set PBF log rank successful.
**/
PSSI_STATUS
PssiDebugSetLogRank (
  IN UINT64 Rank
  );

/**
  SSC config.

  @param[in]    Level    Level of SSC.

  @retval PSSI_SUCESS    SSC config  successful.
**/
PSSI_STATUS
PssiDebugSscCfg (
  IN UINT64 Level
  );

/**
  Secure config.

  @param[in]    Level    Level of secure.

  @retval PSSI_SUCESS    secure config  successful.
**/
PSSI_STATUS
PssiSecureCfg (
  IN UINT64 Level
  );

/**
  Set OTR(Over temperature recovery) value.

  @param[in]    Temperature  OTR value.

  @retval PSSI_SUCESS        OTR config successful.
**/
PSSI_STATUS
PssiOtrCfg (
  IN UINT64 Temperature
  );

/**
  Init high speed phy.

  @param[in]    PhyDomain       Domain to be config.

  @param[in]    PhyMultiplex    Domain Multiplex config.

  @param[in]    PhyMode         Mode to be config.

  @param[in]    PhySpeed        Speed to be config.


  @retval PSSI_SUCESS           Phy config successful.
**/
PSSI_STATUS
PssiPhyInit (
  IN UINT64 PhyDomain,
  IN UINT64 PhyMultiplex,
  IN UINT64 PhyMode,
  IN UINT64 PhySpeed
  );

/**
  Init Chip to Chip module.

  @param[in]      Parameter   A pointer to the parameter table.

  @retval PSSI_SUCESS         Init C2C sucessful.
**/
PSSI_STATUS
PssiC2cInit (
  IN VOID *Parameter
  );

/**
  Init DDR controller.

  @param[in]      Parameter     A pointer to the parameter table.

  @param[in, out]  ErrorCode    A pointer to the Error Code.

  @retval PSSI_SUCESS           DDR controller init successful.

  @retval PSSI_INIT_FAIL        DDR controller init fail.
**/
PSSI_STATUS
PssiDdrInitV2 (
  IN     VOID   *Parameter,
  IN     UINT64  TrainingParameter,
  IN OUT UINT64 *ErrorCode
  );

/**
  Set DDR self-refresh dgree.

  @param[in] Dgree          DDR self-refresh Dgree.

  @retval PSSI_SUCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiSetDdrSelfRefreshDgreeV2 (
  IN UINT64 Dgree
  );

/**
  DDR self-refresh lock.

  @retval PSSI_SUCESS       lock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshLockV2 (
  VOID
  );

/**
  DDR self-refresh unlock.

  @retval PSSI_SUCESS       Unlock DDR self-refresh successful.
**/
PSSI_STATUS
PssiDdrSelfRefreshUnLockV2 (
  VOID
  );

/**
  Set DDR secure regions, enable TZC required.

  @param[in]    Parameter   A pointer to the parameter table.

  @retval PSSI_SUCESS       Set DDR self-refresh dgree successful.
**/
PSSI_STATUS
PssiDdrSecureInitV2 (
  IN VOID *Parameter
  );

/**
  Print DDR training limit value.

  @retval PSSI_SUCESS       Print DDR training limit value successful.
**/
PSSI_STATUS
PssiPrintTrainingLimitV2 (
  VOID
  );

/**
  Clear memory region start startAddr ~ 2^AddrSapce

  @param[in]    AddrSpace   Clear memory region address space.

  @param[in]    StartAddr   Clear memory region start address.

  @retval PSSI_SUCESS       Clear memory successful.
**/
PSSI_STATUS
PssiClearMemoryV2 (
  IN UINT64 AddrSpace,
  IN UINT64 StartAddr
  );

/**
  Check DDR training information.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TraniningParameter   DDR training parameter.

  @param[in, out] TrainingInfo       DDR training information.

  @retval PSSI_SUCESS                Check DDR training information successful.
**/
PSSI_STATUS
PssiTrainingCheckV2 (
  IN     VOID   *Parameter,
  IN     UINT64 TrainingParameter,
  IN OUT VOID   *TrainingInfo
  );

/**
  Skip DDR training progress, init DDR controller fast.

  @param[in]    Parameter            A pointer to the parameter table.

  @param[in]    TraniningParameter   DDR training parameter.

  @param[in, out] ErrorCode          A pointer to the Error Code.

  @retval PSSI_SUCESS                Skip DDR training information successful.
**/
PSSI_STATUS
PssiSkipTraningV2 (
  IN     VOID   *Parameter,
  IN     UINT64  TrainingParameter,
  IN OUT UINT64 *ErrorCode
  );

/**
  DDR CTL bist test.

  @param[in]    Channel              Channel of DDR to be start bist test.

  @param[in, out] ErrorCode          A pointer to the Error Code.

  @retval PSSI_SUCESS                Channel bist test successful.

  @retval PSSI_INIT_FAIL             Channel bist test fail.
**/
PSSI_STATUS
PssiDdrBistV2 (
  IN     UINT64 Channel,
  IN OUT UINT64 *ErrorCode
  );

/**
  Get DDR training parameter.

  @retval Training Parameter base address.
**/
UINT64
PssiGetTrainingParameterV2 (
  VOID
  );

/**
  Print DDR registers.

  @param[in] Channel    Channel of DDR to be print registers.

  @retval PSSI_SUCESS   Print DDR registers successful.
**/
PSSI_STATUS
PssiPrintDdrRegV2 (
  IN UINT64 Channel
  );

/**
  DDR DQS/DQ connect test.

  @param[in]      Parameter   A pointer to parameter table.

  @param[in, out] Result      Result of connect test.

  @retval PSSI_SUCESS         Get result of connect test successful.
**/
PSSI_STATUS
PssiDdrConnectTestV2 (
  IN     VOID *Parameter,
  IN OUT VOID *Result
  );

/**
  Print digtial eye chart

  @param[in]      EyeMode     Eye chart mode

  @param[in]      Channel     Channel to be print digtial eye chart

  @retval PSSI_SUCESS         Print eye chart  successful.
**/
PSSI_STATUS
PssiDdrPrintEye (
  IN UINT64 EyeMode,
  IN UINT64 Channel
  );

/**
  Scrub memory region

  @param[in]    ScrubOp     A pointer to scrub opcode struct.

  @param[in]    Size        Size of scrub opcode struct.

  @retval PSSI_SUCESS       Clear memory successful.
**/
PSSI_STATUS
PssiScrubMemory (
  IN SCRUB_OPCODE *ScrubOp,
  IN UINT64 Size
  );

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
  );

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
  );

#endif


