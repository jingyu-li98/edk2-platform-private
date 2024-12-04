/** @file

  Copyright (c) 2017 - 2021, Arm Limited. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
**/

#ifndef CONFIGURATION_MANAGER_H_
#define CONFIGURATION_MANAGER_H_

#include <Protocol/ConfigurationManagerProtocol.h>
#include <Uefi/UefiBaseType.h>
#include <AcpiPlatform.h>


/** The configuration manager version
*/
#define CONFIGURATION_MANAGER_REVISION CREATE_REVISION (1, 0)

/** The OEM ID
*/
#define CFG_MGR_OEM_ID    { 'P', 'H', 'Y', 'L', 'T', 'D' }

/** A function that prepares Configuration Manager Objects for returning.

  @param [in]  This        Pointer to the Configuration Manager Protocol.
  @param [in]  CmObjectId  The Configuration Manager Object ID.
  @param [in]  Token       A token for identifying the object.
  @param [out] CmObject    Pointer to the Configuration Manager Object
                           descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
typedef EFI_STATUS (*CM_OBJECT_HANDLER_PROC) (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  );

/** A structure describing the platform configuration
    manager repository information
*/
typedef struct PlatformRepositoryInfo {
  /// Configuration Manager Information
  CM_STD_OBJ_CONFIGURATION_MANAGER_INFO CmInfo;

  /// List of ACPI tables
  CM_STD_OBJ_ACPI_TABLE_INFO            CmAcpiTableList[PLAT_ACPI_TABLE_COUNT];

  /// Boot architecture information
  CM_ARM_BOOT_ARCH_INFO                 BootArchInfo;

  /// Power management profile information
  CM_ARM_POWER_MANAGEMENT_PROFILE_INFO  PmProfileInfo;

  /// GIC distributor information
  CM_ARM_GICD_INFO                      GicDInfo;

  /// Generic timer information
  CM_ARM_GENERIC_TIMER_INFO             GenericTimerInfo;

  /// Watchdog information
  CM_ARM_GENERIC_WATCHDOG_INFO          Watchdog;

  /** Serial port information for the
      serial port console redirection port
  */
  CM_ARM_SERIAL_PORT_INFO               SpcrSerialPort;

  /// Serial port information for the DBG2 UART port
  CM_ARM_SERIAL_PORT_INFO               DbgSerialPort;

  /// Socket count
  UINT8                                 SocketCount;

  /// GIG Cpu Interface Count
  UINT32                                GicCInfoCount;

  /// Pci Config Space Count
  UINT32                                PciConfigInfoCount;

  /// Memory Affinity Count
  UINT32                                MemoryAffinityInfoCount;

  CM_ARM_GICC_INFO                      GicCInfo[PLAT_CPU_COUNT];

  CM_ARM_GIC_ITS_INFO                   GicItsInfo[PLAT_ITS_COUNT];

  CM_ARM_PCI_CONFIG_SPACE_INFO          PciConfigInfo[PLAT_PCI_COUNT];

  CM_ARM_MEMORY_AFFINITY_INFO           MemoryAffinityInfo[PLAT_MEMORY_AFFINITY_COUNT];

} EDKII_PLATFORM_REPOSITORY_INFO;

#endif // CONFIGURATION_MANAGER_H__
