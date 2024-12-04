/** @file
  Configuration Manager Dxe

  Copyright (c) 2017 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
**/

#include <IndustryStandard/DebugPort2Table.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/HobLib.h>
#include <Library/PssiLib.h>
#include <PhytiumAcpiHelp.h>
#include <AcpiPlatform.h>
#include <Library/PL011UartClockLib.h>
#include <Library/PL011UartLib.h>
#include <Library/SerialPortLib.h>
#include "ConfigurationManager.h"

/** The platform configuration repository information.
*/
STATIC
EDKII_PLATFORM_REPOSITORY_INFO PhytiumPlatformRepositoryInfo = {
  /// Configuration Manager information
  { CONFIGURATION_MANAGER_REVISION, CFG_MGR_OEM_ID },

  // ACPI Table List
  {
    // FADT Table
    {
      EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE,
      EFI_ACPI_6_3_FIXED_ACPI_DESCRIPTION_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdFadt),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','F','A','D','T'),
    },
    // GTDT Table
    {
      EFI_ACPI_6_3_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE,
      EFI_ACPI_6_3_GENERIC_TIMER_DESCRIPTION_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdGtdt),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','G','T','D','T'),
    },
    // MADT Table
    {
      EFI_ACPI_6_3_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,
      EFI_ACPI_6_3_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdMadt),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','A','P','I','C'),
    },
    // SPCR Table
    {
      EFI_ACPI_6_3_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
      EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdSpcr),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','S','P','C','R'),
    },
    // DBG2 Table
    {
      EFI_ACPI_6_3_DEBUG_PORT_2_TABLE_SIGNATURE,
      EFI_ACPI_DBG2_DEBUG_DEVICE_INFORMATION_STRUCT_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdDbg2),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','D','B','G','2'),
    },
    // PCI MCFG Table
    {
      EFI_ACPI_6_3_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
      EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdMcfg),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','M','C','F','G'),
    },
    // SRAT table
    {
      EFI_ACPI_6_3_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE,
      EFI_ACPI_6_3_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION,
      CREATE_STD_ACPI_TABLE_GEN_ID (EStdAcpiTableIdSrat),
      NULL,
      SIGNATURE_64 ('P','H','Y','T','S','R','A','T'),
    }
  },

  // Boot architecture information
  { EFI_ACPI_6_3_ARM_PSCI_COMPLIANT },      // BootArchFlags

  // Power management profile information
  { EFI_ACPI_6_3_PM_PROFILE_MOBILE },       // PowerManagement Profile

  // GIC Distributor Info
  {
    FixedPcdGet64 (PcdGicDistributorBase),  // UINT64  PhysicalBaseAddress
    0,                                      // UINT32  SystemVectorBase
    3                                       // UINT8   GicVersion
  },

  // Generic Timer Info
  {
    // The physical base address for the counter control frame
    PHYTIUM_SYSTEM_TIMER_BASE_ADDRESS,
    // The physical base address for the counter read frame
    PHYTIUM_CNT_READ_BASE_ADDRESS,
    // The secure EL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerSecIntrNum),
    // The secure EL1 timer flags
    PHYTIUM_GTDT_GTIMER_FLAGS,
    // The non-secure EL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerIntrNum),
    // The non-secure EL1 timer flags
    PHYTIUM_GTDT_GTIMER_FLAGS,
    // The virtual timer interrupt
    FixedPcdGet32 (PcdArmArchTimerVirtIntrNum),
    // The virtual timer flags
    PHYTIUM_GTDT_GTIMER_FLAGS,
    // The non-secure EL2 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerHypIntrNum),
    // The non-secure EL2 timer flags
    PHYTIUM_GTDT_GTIMER_FLAGS
  },

  // Watchdog Info
  {
    // The physical base address of the SBSA Watchdog control frame
    FixedPcdGet64 (PcdGenericWatchdogControlBase),
    // The physical base address of the SBSA Watchdog refresh frame
    FixedPcdGet64 (PcdGenericWatchdogRefreshBase),
    // The watchdog interrupt
    FixedPcdGet32 (PcdGenericWatchdogEl2IntrNum),
    // The watchdog flags
    PHYTIUM_SBSA_WATCHDOG_FLAGS
  },

  // SPCR Serial Port
  {
    FixedPcdGet64 (PcdSerialRegisterBase),            // BaseAddress
    FixedPcdGet32 (PL011UartInterrupt),               // Interrupt
    FixedPcdGet64 (PcdUartDefaultBaudRate),           // BaudRate
    FixedPcdGet32 (PL011UartClkInHz),                 // Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART  // Port subtype
  },
  // Debug Serial Port
  {
    FixedPcdGet64 (PcdSerialDbgRegisterBase),         // BaseAddress
    DEBUG_SERIAL_INTERRUPT,                           // Interrupt
    FixedPcdGet64 (PcdSerialDbgUartBaudRate),         // BaudRate
    FixedPcdGet32 (PcdSerialDbgUartClkInHz),          // Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_PL011_UART  // Port subtype
  },
};

/** A helper function for returning the Configuration Manager Objects.

  @param [in]       CmObjectId     The Configuration Manager Object ID.
  @param [in]       Object         Pointer to the Object(s).
  @param [in]       ObjectSize     Total size of the Object(s).
  @param [in]       ObjectCount    Number of Objects.
  @param [in, out]  CmObjectDesc   Pointer to the Configuration Manager Object
                                   descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
**/
STATIC
EFI_STATUS
EFIAPI
HandleCmObject (
  IN  CONST CM_OBJECT_ID                CmObjectId,
  IN        VOID                *       Object,
  IN  CONST UINTN                       ObjectSize,
  IN  CONST UINTN                       ObjectCount,
  IN  OUT   CM_OBJ_DESCRIPTOR   * CONST CmObjectDesc
  )
{
  CmObjectDesc->ObjectId = CmObjectId;
  CmObjectDesc->Size = ObjectSize;
  CmObjectDesc->Data = (VOID*)Object;
  CmObjectDesc->Count = ObjectCount;
  DEBUG ((
    DEBUG_INFO,
    "INFO: CmObjectId = %x, Ptr = 0x%p, Size = %d, Count = %d\n",
    CmObjectId,
    CmObjectDesc->Data,
    CmObjectDesc->Size,
    CmObjectDesc->Count
    ));
  return EFI_SUCCESS;
}

/** A helper function for returning the Configuration Manager Objects that
    match the token.

  @param [in]  This               Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId         The Configuration Manager Object ID.
  @param [in]  Token              A token identifying the object.
  @param [in]  HandlerProc        A handler function to search the object
                                  referenced by the token.
  @param [in, out]  CmObjectDesc  Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
STATIC
EFI_STATUS
EFIAPI
HandleCmObjectRefByPlatform (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token,
  IN  CONST CM_OBJECT_HANDLER_PROC                        HandlerProc,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObjectDesc
  )
{
  EFI_STATUS  Status;

  Status = HandlerProc (This, CmObjectId, Token, CmObjectDesc);

  DEBUG ((
    DEBUG_INFO,
    "INFO: CmObjectId = %x, Ptr = 0x%p, Size = %d, Count = %d\n",
    CmObjectId,
    CmObjectDesc->Data,
    CmObjectDesc->Size,
    CmObjectDesc->Count
    ));
  return Status;
}

/** A helper function for returning the Configuration Manager Objects that
    match the token.

  @param [in]  This               Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId         The Configuration Manager Object ID.
  @param [in]  Object             Pointer to the Object(s).
  @param [in]  ObjectSize         Total size of the Object(s).
  @param [in]  ObjectCount        Number of Objects.
  @param [in]  Token              A token identifying the object.
  @param [in]  HandlerProc        A handler function to search the object
                                  referenced by the token.
  @param [in, out]  CmObjectDesc  Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
STATIC
EFI_STATUS
EFIAPI
HandleCmObjectRefByToken (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN        VOID                                  *       Object,
  IN  CONST UINTN                                         ObjectSize,
  IN  CONST UINTN                                         ObjectCount,
  IN  CONST CM_OBJECT_TOKEN                               Token,
  IN  CONST CM_OBJECT_HANDLER_PROC                        HandlerProc,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObjectDesc
  )
{
  EFI_STATUS  Status;
  CmObjectDesc->ObjectId = CmObjectId;
  if (Token == CM_NULL_TOKEN) {
    CmObjectDesc->Size = ObjectSize;
    CmObjectDesc->Data = (VOID*)Object;
    CmObjectDesc->Count = ObjectCount;
    Status = EFI_SUCCESS;
  } else {
    Status = HandlerProc (This, CmObjectId, Token, CmObjectDesc);
  }

  DEBUG ((
    DEBUG_INFO,
    "INFO: Token = 0x%p, CmObjectId = %x, Ptr = 0x%p, Size = %d, Count = %d\n",
    (VOID*)Token,
    CmObjectId,
    CmObjectDesc->Data,
    CmObjectDesc->Size,
    CmObjectDesc->Count
    ));
  return Status;
}

/** Return GIC CPU Interface Info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId     The Object ID of the CM object requested
  @param [in]      SearchToken    A unique token for identifying the requested
                                  CM_ARM_GICC_INFO object.
  @param [in, out] CmObject       Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.
  @retval EFI_NOT_FOUND           The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetGicCInfo (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               SearchToken,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;
  UINT32                            TotalObjCount;
  UINT32                            ObjIndex;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  PlatformRepo = This->PlatRepoInfo;

  TotalObjCount = ARRAY_SIZE (PlatformRepo->GicCInfo);

  for (ObjIndex = 0; ObjIndex < TotalObjCount; ObjIndex++) {
    if (SearchToken == (CM_OBJECT_TOKEN)&PlatformRepo->GicCInfo[ObjIndex]) {
      CmObject->ObjectId = CmObjectId;
      CmObject->Size = sizeof (PlatformRepo->GicCInfo[ObjIndex]);
      CmObject->Data = (VOID*)&PlatformRepo->GicCInfo[ObjIndex];
      CmObject->Count = 1;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
/** Return GIC Its Info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId     The Object ID of the CM object requested
  @param [in]      SearchToken    A unique token for identifying the requested
                                  CM_ARM_GIC_ITS_INFO object.
  @param [in, out] CmObject       Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS             Success.
  @retval EFI_NOT_FOUND           The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetItsInfo (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               SearchToken,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO    *PlatformRepo;
  CM_ARM_GIC_ITS_INFO               *Info;
  EFI_HOB_GUID_TYPE                 *GuidHob;
  ITS_ACPI_INFO                     *ItsAcpiInfo;
  ITS_ACPI_NODE                     *ItsAcpiNode;
  UINT32                            ItsAcpiNodeCount;
  UINT32                            Index;

  GuidHob = GetFirstGuidHob (&gPlatformItsAcpiInfoGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }

  ItsAcpiInfo = (ITS_ACPI_INFO *) GET_GUID_HOB_DATA(GuidHob);
  ItsAcpiNodeCount = ItsAcpiInfo->NodeCount;
  ItsAcpiNode = ItsAcpiInfo->Node;

  PlatformRepo = This->PlatRepoInfo;
  Info = PlatformRepo->GicItsInfo;

  for (Index = 0; Index < ItsAcpiNodeCount; Index++) {
    Info[Index].GicItsId = ItsAcpiNode[Index].GicItsId;
    Info[Index].PhysicalBaseAddress= ItsAcpiNode[Index].PhysicalBaseAddress;
    Info[Index].ProximityDomain = ItsAcpiNode[Index].ProximityDomain;
  }

  CmObject->ObjectId = CmObjectId;
  CmObject->Size = ItsAcpiNodeCount * sizeof (PlatformRepo->GicItsInfo[0]);
  CmObject->Data = (VOID*)&PlatformRepo->GicItsInfo[0];
  CmObject->Count = ItsAcpiNodeCount;

  return EFI_SUCCESS;
}

/**
  Init GIC CPU Interface Info and GIC CPU Affinity Info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS             Success.
  @retval EFI_NOT_FOUND           The required information is not found.
**/
EFI_STATUS
InitGicCInfo (
  IN CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO    *PlatformRepo;
  CM_ARM_GICC_INFO                  *GicCInfo;
  EFI_HOB_GUID_TYPE                 *GuidHob;
  CPU_ACPI_INFO                     *CpuAcpiInfo;
  CPU_ACPI_NODE                     *CpuAcpiNode;
  UINT32                            CpuAcpiNodeCount;
  UINT32                            Index;

  GuidHob = GetFirstGuidHob(&gPlatformCpuAcpiInfoGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }
  CpuAcpiInfo = (CPU_ACPI_INFO *) GET_GUID_HOB_DATA(GuidHob);
  CpuAcpiNodeCount = CpuAcpiInfo->NodeCount;
  CpuAcpiNode = CpuAcpiInfo->Node;

  PlatformRepo = This->PlatRepoInfo;
  GicCInfo = PlatformRepo->GicCInfo;

  for (Index = 0; Index < CpuAcpiNodeCount; Index++) {
    GicCInfo[Index].CPUInterfaceNumber = 0;
    GicCInfo[Index].AcpiProcessorUid = CpuAcpiNode[Index].AcpiProcessorUid;
    GicCInfo[Index].Flags = EFI_ACPI_6_3_GIC_ENABLED;
    GicCInfo[Index].ParkingProtocolVersion = 0x0;
    GicCInfo[Index].PerformanceInterruptGsiv = PMU_INTERRUPT;
    GicCInfo[Index].ParkedAddress = 0x0;
    GicCInfo[Index].PhysicalBaseAddress = 0;
    GicCInfo[Index].GICV = 0;
    GicCInfo[Index].GICH = 0;
    GicCInfo[Index].VGICMaintenanceInterrupt = GIC_VIRTUAL_INTERRUPT;
    GicCInfo[Index].GICRBaseAddress = CpuAcpiNode[Index].GicRBaseAddress;
    GicCInfo[Index].MPIDR = CpuAcpiNode[Index].Mpidr;
    GicCInfo[Index].ProcessorPowerEfficiencyClass = CpuAcpiNode[Index].ProcessorPowerEfficiencyClass;
    GicCInfo[Index].ProximityDomain = CpuAcpiNode[Index].ProximityDomain;
    GicCInfo[Index].Flags = EFI_ACPI_6_3_GICC_ENABLED;
    GicCInfo[Index].ClockDomain = 0x0;
    GicCInfo[Index].AffinityFlags = 0x1;
  }
  PlatformRepo->GicCInfoCount = CpuAcpiNodeCount;

  return EFI_SUCCESS;
}

/**
  Init Memory Affinity Info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS             Success.
  @retval EFI_NOT_FOUND           The required information is not found.
**/
EFI_STATUS
InitMemoryAffinityInfo (
  IN CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
)
{
  EDKII_PLATFORM_REPOSITORY_INFO  *PlatformRepo;
  CM_ARM_MEMORY_AFFINITY_INFO     *Info;
  EFI_HOB_GUID_TYPE               *GuidHob;
  PHYTIUM_MEMORY_INFO             *MemoryInfo;
  PHYTIUM_MEMORY_BLOCK            *MemBlock;
  UINT32                          Index;

  GuidHob = GetFirstGuidHob(&gPlatformMemoryInforGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }
  MemoryInfo = (PHYTIUM_MEMORY_INFO *) GET_GUID_HOB_DATA(GuidHob);
  MemBlock = MemoryInfo->MemBlock;

  PlatformRepo = This->PlatRepoInfo;
  Info = PlatformRepo->MemoryAffinityInfo;

  for (Index = 0; Index < MemoryInfo->MemBlockCount; Index++) {
    Info[Index].ProximityDomain = MemBlock->MemNodeId;
    Info[Index].BaseAddress = MemBlock->MemStart;
    Info[Index].Length = MemBlock->MemSize;
    Info[Index].Flags = 0x1;
    MemBlock ++;
  }

  PlatformRepo->MemoryAffinityInfoCount = MemoryInfo->MemBlockCount;

  return EFI_SUCCESS;
}

/**
  Init pci config space info & pci interrupt map info &
  pci address map info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS             Success.
  @retval EFI_NOT_FOUND           The required information is not found.
**/
EFI_STATUS
InitPciInfo (
  IN CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
)
{
  EDKII_PLATFORM_REPOSITORY_INFO    *PlatformRepo;
  CM_ARM_PCI_CONFIG_SPACE_INFO      *PciConfigInfo;
  EFI_HOB_GUID_TYPE                 *GuidHob;
  PHYTIUM_PCI_HOST_BRIDGE           *PciHostBridge;
  PHYTIUM_PCI_HOST_BLOCK            *PciHostBlock;
  UINT8                             Index;
  UINT64                            PciHostCount;

  GuidHob = GetFirstGuidHob(&gPlatformPciHostInforGuid);
  if (GuidHob == NULL) {
    DEBUG ((DEBUG_ERROR,"Not Find PciHostInfo\n"));
    return EFI_NOT_FOUND;
  }

  PciHostBridge = (PHYTIUM_PCI_HOST_BRIDGE *) GET_GUID_HOB_DATA(GuidHob);
  PciHostCount = PciHostBridge->PciHostCount;
  PciHostBlock = PciHostBridge->PciHostBlock;

  PlatformRepo = This->PlatRepoInfo;
  PciConfigInfo = PlatformRepo->PciConfigInfo;
  for (Index = 0; Index < PciHostCount; Index++) {
    PciConfigInfo->BaseAddress = PciHostBlock->PciConfigBase;
    PciConfigInfo->PciSegmentGroupNumber = Index;
    PciConfigInfo->StartBusNumber = PciHostBlock->BusStart;
    PciConfigInfo->EndBusNumber = PciHostBlock->BusEnd;
    PciConfigInfo++;
    PciHostBlock++;
  }

  PlatformRepo->PciConfigInfoCount = PciHostCount;

  return EFI_SUCCESS;
}

/**
  Init Socket Count.

  @param [in]      This           Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS             Success.
  @retval EFI_NOT_FOUND           The required information is not found.
**/
EFI_STATUS
InitSocketCount (
  IN CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO   *PlatformRepo;
  EFI_HOB_GUID_TYPE                *GuidHob;
  PHYTIUM_CPU_MAP_INFO             *CpuMapInfo;
  UINT8                            SocketCount;

  PlatformRepo = This->PlatRepoInfo;
  GuidHob = GetFirstGuidHob(&gPlatformCpuInforGuid);
  if (GuidHob == NULL) {
    DEBUG ((DEBUG_ERROR,"Not Find CpuMapInfo\n"));
     return EFI_NOT_FOUND;
  }
  CpuMapInfo = (PHYTIUM_CPU_MAP_INFO *) GET_GUID_HOB_DATA(GuidHob);
  SocketCount = CpuMapInfo->CpuMapCount;
  PhytiumPlatformRepositoryInfo.SocketCount = SocketCount;

  PlatformRepo->SocketCount = SocketCount;

  return EFI_SUCCESS;
}

/**
  Print Pci related info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.
**/
VOID
DumpPciInfo (
  IN CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO    *PlatformRepo;
  CM_ARM_PCI_CONFIG_SPACE_INFO      *PciConfigInfo;
  UINT32                            Index;
  UINT32                            SocketCount;

  PlatformRepo = This->PlatRepoInfo;
  SocketCount = PlatformRepo->SocketCount;
  PciConfigInfo = PlatformRepo->PciConfigInfo;

  DEBUG((DEBUG_INFO, "========pci config space info========\n"));
  for (Index = 0; Index < SocketCount; Index++) {
    DEBUG((DEBUG_INFO, "Socket%d\n", Index));
    DEBUG((DEBUG_INFO, "BaseAddr:0x%lx\n", PciConfigInfo->BaseAddress));
    DEBUG((DEBUG_INFO, "Segment:%d\n", PciConfigInfo->PciSegmentGroupNumber));
    DEBUG((DEBUG_INFO, "BusStart:%d\n", PciConfigInfo->StartBusNumber));
    DEBUG((DEBUG_INFO, "BusEnd:%d\n", PciConfigInfo->EndBusNumber));
    PciConfigInfo++;
    DEBUG((DEBUG_INFO, "\n"));
  }
}

/** Return a standard namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetStandardNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS                        Status;
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;
  UINT32                            TableCount;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;
  PlatformRepo = This->PlatRepoInfo;

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    case EStdObjCfgMgrInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->CmInfo,
                 sizeof (PlatformRepo->CmInfo),
                 1,
                 CmObject
                 );
      break;

    case EStdObjAcpiTableList:
      TableCount = ARRAY_SIZE (PlatformRepo->CmAcpiTableList);
      Status = HandleCmObject (
                 CmObjectId,
                 PlatformRepo->CmAcpiTableList,
                 (sizeof (PlatformRepo->CmAcpiTableList[0]) * TableCount),
                 TableCount,
                 CmObject
                 );
      break;

    default: {
      Status = EFI_NOT_FOUND;
      DEBUG (( DEBUG_ERROR, "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** Return an ARM namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetArmNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS                        Status;
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_NOT_FOUND;
  PlatformRepo = This->PlatRepoInfo;

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    case EArmObjBootArchInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->BootArchInfo,
                 sizeof (PlatformRepo->BootArchInfo),
                 1,
                 CmObject
                 );
      break;
    case EArmObjPowerManagementProfileInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->PmProfileInfo,
                 sizeof (PlatformRepo->PmProfileInfo),
                 1,
                 CmObject
                 );
      break;

    case EArmObjGenericTimerInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->GenericTimerInfo,
                 sizeof (PlatformRepo->GenericTimerInfo),
                 1,
                 CmObject
                 );
      break;

    case EArmObjPlatformGenericWatchdogInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->Watchdog,
                 sizeof (PlatformRepo->Watchdog),
                 1,
                 CmObject
                 );
      break;

    case EArmObjGicItsInfo:
      Status = HandleCmObjectRefByPlatform (
                 This,
                 CmObjectId,
                 Token,
                 GetItsInfo,
                 CmObject
                 );
      break;

    case EArmObjGicCInfo:
      Status = HandleCmObjectRefByToken (
                 This,
                 CmObjectId,
                 PlatformRepo->GicCInfo,
                 sizeof (PlatformRepo->GicCInfo),
                 PlatformRepo->GicCInfoCount,
                 Token,
                 GetGicCInfo,
                 CmObject
                 );
      break;

    case EArmObjGicDInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->GicDInfo,
                 sizeof (PlatformRepo->GicDInfo),
                 1,
                 CmObject
                 );
      break;

    case EArmObjSerialConsolePortInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->SpcrSerialPort,
                 sizeof (PlatformRepo->SpcrSerialPort),
                 1,
                 CmObject
                 );
      break;

    case EArmObjSerialDebugPortInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->DbgSerialPort,
                 sizeof (PlatformRepo->DbgSerialPort),
                 1,
                 CmObject
                 );
      break;

    case EArmObjPciConfigSpaceInfo:
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->PciConfigInfo,
                 sizeof (PlatformRepo->PciConfigInfo),
                 PlatformRepo->PciConfigInfoCount,
                 CmObject
                 );
      break;

    case EArmObjMemoryAffinityInfo :
      Status = HandleCmObject (
                 CmObjectId,
                 &PlatformRepo->MemoryAffinityInfo,
                 sizeof (PlatformRepo->MemoryAffinityInfo),
                 PlatformRepo->MemoryAffinityInfoCount,
                 CmObject
                 );
      break;

    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_INFO,
        "INFO: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }//switch

  return Status;
}

/** Initialize All Uarts of active die

  @retval EFI_SUCCESS           Uarts' initialization success or Uarts have been initialized
  
  @retval RETURN_INVALID_PARAMETER  One or more of the attributes has an
                                    unsupported value.
**/
STATIC
EFI_STATUS
EFIAPI
InitUarts()
{
  UINT64                BaudRate;
  UINT32                ReceiveFifoDepth;
  EFI_PARITY_TYPE       Parity;
  UINT8                 DataBits;
  EFI_STOP_BITS_TYPE    StopBits;

  UINT8                 Die;
  UINT8                 Uart;
  UINT64                DieOffset;
  UINT64                UartBase;
  UINT64                UartSize;

  EFI_HOB_GUID_TYPE     *GuidHob;
  PHYTIUM_CPU_MAP_INFO  *CpuMapInfo;
  UINT8                 DieCount;
  UINT64                *CoreMap;

  EFI_STATUS          Status;
  
  BaudRate          = FixedPcdGet64 (PcdUartDefaultBaudRate);
  ReceiveFifoDepth  = 0;         // Use default FIFO depth
  Parity            = (EFI_PARITY_TYPE)FixedPcdGet8 (PcdUartDefaultParity);
  DataBits          = FixedPcdGet8 (PcdUartDefaultDataBits);
  StopBits          = (EFI_STOP_BITS_TYPE)FixedPcdGet8 (PcdUartDefaultStopBits);

  DieOffset         = 0x100000000000;
  UartBase          = 0x20000000;
  UartSize          = 0x1000;

  GuidHob           = GetFirstGuidHob(&gPlatformCpuInforGuid);  
  CpuMapInfo        = (PHYTIUM_CPU_MAP_INFO *)GET_GUID_HOB_DATA(GuidHob);
  DieCount          = CpuMapInfo->CpuMapCount;
  CoreMap            = &CpuMapInfo->CpuMap[0];
  
  // Init Uarts
  DEBUG((DEBUG_INFO, "InitUarts, DieCount: %d\n", DieCount));
  for (Die = 0; Die < DieCount; ++Die) {
    for (Uart = Die == 0 ? 2 : 0; Uart < 3; ++Uart) {
      Status = PL011UartInitializePort(
          Die * DieOffset + UartBase + Uart * UartSize, 
          PL011UartClockGetFreq(),
          &BaudRate, 
          &ReceiveFifoDepth, 
          &Parity, 
          &DataBits, 
          &StopBits
          );

      if (Status != EFI_SUCCESS) {
        DEBUG((DEBUG_ERROR, "InitUarts Failed: %r\n", Status));
        return Status;
      }
    }
  }

  return EFI_SUCCESS;
}

/** Initialize the platform configuration repository.

  @param [in]  This        Pointer to the Configuration Manager Protocol.

  @retval EFI_SUCCESS   Success
**/
STATIC
EFI_STATUS
EFIAPI
InitializePlatformRepository (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This
  )
{

  InitGicCInfo (This);
  InitMemoryAffinityInfo (This);
  InitPciInfo (This);
  InitSocketCount (This);
  DumpPciInfo (This);

  InitUarts();

  return EFI_SUCCESS;
}

/** Return an OEM namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetOemNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The GetObject function defines the interface implemented by the
    Configuration Manager Protocol for returning the Configuration
    Manager Objects.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
PhytiumPlatformGetObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS  Status;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_NAMESPACE_ID (CmObjectId)) {
    case EObjNameSpaceStandard:
      Status = GetStandardNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceArm:
      Status = GetArmNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceOem:
      Status = GetOemNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    default: {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Unknown Namespace Object = 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The SetObject function defines the interface implemented by the
    Configuration Manager Protocol for updating the Configuration
    Manager Objects.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in]      CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the Object.

  @retval EFI_UNSUPPORTED  This operation is not supported.
**/
EFI_STATUS
EFIAPI
PhytiumPlatformSetObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN        CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  return EFI_UNSUPPORTED;
}

/** A structure describing the configuration manager protocol interface.
*/
STATIC
CONST
EDKII_CONFIGURATION_MANAGER_PROTOCOL PhytiumPlatformConfigManagerProtocol = {
  CREATE_REVISION (1, 0),
  PhytiumPlatformGetObject,
  PhytiumPlatformSetObject,
  &PhytiumPlatformRepositoryInfo
};

/**
  Entrypoint of Configuration Manager Dxe.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
ConfigurationManagerDxeInitialize (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE  * SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkiiConfigurationManagerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID*)&PhytiumPlatformConfigManagerProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to get Install Configuration Manager Protocol." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  DEBUG ((DEBUG_INFO, "fun:%a(),line=%d\n", __FUNCTION__, __LINE__));
  Status = InitializePlatformRepository (
    &PhytiumPlatformConfigManagerProtocol
    );
  DEBUG ((DEBUG_INFO, "fun:%a(),line=%d\n", __FUNCTION__, __LINE__));
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to initialize the Platform Configuration Repository." \
      " Status = %r\n",
      Status
      ));
  }

error_handler:
  return Status;
}
