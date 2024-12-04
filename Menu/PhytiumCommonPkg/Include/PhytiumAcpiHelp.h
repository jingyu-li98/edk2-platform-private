/** @file

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef PHYTIUM_ACPI_HELP_H_
#define PHYTIUM_ACPI_HELP_H_

#include <Uefi/UefiBaseType.h>
#include <IndustryStandard/Acpi.h>
#pragma pack (1)
typedef struct {
  UINT64 Mpidr;
  UINT32 AcpiProcessorUid;
  UINT32 ProximityDomain;
  UINT64 GicRBaseAddress;
  UINT8  ProcessorPowerEfficiencyClass;
} CPU_ACPI_NODE;

typedef struct {
  UINT32        NodeCount;
  CPU_ACPI_NODE Node[1];
} CPU_ACPI_INFO;

typedef struct {
  UINT32    GicItsId;
  UINT64    PhysicalBaseAddress;
  UINT32    ProximityDomain;
} ITS_ACPI_NODE;

typedef struct {
  UINT32        NodeCount;
  ITS_ACPI_NODE Node[1];
} ITS_ACPI_INFO;

#pragma pack ()

#endif

