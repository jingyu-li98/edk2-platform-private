/** @file
*
*  Copyright (c) 2013-2017, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef __ARM_PHYT_H__
#define __ARM_PHYT_H__
#include <IndustryStandard/Acpi.h>
#include <Pcie.h>

#define EFI_ACPI_6_1_GIC_ITS_INIT(GicITSHwId, GicITSBase) \
  { \
    EFI_ACPI_6_1_GIC_ITS, sizeof (EFI_ACPI_6_1_GIC_ITS_STRUCTURE), EFI_ACPI_RESERVED_WORD, \
    GicITSHwId, GicITSBase, EFI_ACPI_RESERVED_DWORD\
  }

#define EFI_ACPI_5_1_GICR_STRUCTURE_INIT(    \
    GicRBase, GicRlength)                                   \
  {                                                                                  \
    EFI_ACPI_5_1_GICR, sizeof (EFI_ACPI_5_1_GICR_STRUCTURE), EFI_ACPI_RESERVED_WORD,   \
     GicRBase, GicRlength \
  }

#define EFI_ACPI_6_1_GICC_AFFINITY_STRUCTURE_INIT(                                              \
    ProximityDomain, ACPIProcessorUID, Flags, ClockDomain)                                      \
  {                                                                                             \
    3, sizeof (EFI_ACPI_6_1_GICC_AFFINITY_STRUCTURE),ProximityDomain ,                          \
     ACPIProcessorUID,  Flags,  ClockDomain                                                     \
  }

#define EFI_ACPI_6_1_MEMORY_AFFINITY_STRUCTURE_INIT(                                              \
    ProximityDomain, AddressBaseLow, AddressBaseHigh, LengthLow, LengthHigh, Flags)               \
  {                                                                                               \
    1, sizeof (EFI_ACPI_6_1_MEMORY_AFFINITY_STRUCTURE),ProximityDomain , EFI_ACPI_RESERVED_WORD,  \
    AddressBaseLow, AddressBaseHigh, LengthLow, LengthHigh, EFI_ACPI_RESERVED_DWORD, Flags,       \
    EFI_ACPI_RESERVED_QWORD                                                                       \
  }

#define EFI_ACPI_6_1_GICC_STRUCTURE_INIT(GicId, AcpiCpuUid, Mpidr, Flags, PmuIrq,    \
    GicBase, GicVBase, GicHBase, GsivId, GicRBase, ProcessorPowerEfficiencyClass)    \
  {                                                                                  \
    EFI_ACPI_6_1_GIC, sizeof (EFI_ACPI_6_1_GIC_STRUCTURE), EFI_ACPI_RESERVED_WORD,   \
    GicId, AcpiCpuUid, Flags, 0, PmuIrq, 0, GicBase, GicVBase, GicHBase,             \
    GsivId, GicRBase, Mpidr, ProcessorPowerEfficiencyClass, {0, 0, 0}                \
  }

#define EFI_ACPI_6_1_GIC_DISTRIBUTOR_INIT(GicDistHwId, GicDistBase, GicDistVector, GicVersion) \
  { \
    EFI_ACPI_6_1_GICD, sizeof (EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE), EFI_ACPI_RESERVED_WORD, \
    GicDistHwId, GicDistBase, GicDistVector, GicVersion, \
    {EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE} \
  }

//
// Define the number of each table type.
// This is where the table layout is modified.
//
#define EFI_ACPI_PROCESSOR_LOCAL_GICC_AFFINITY_STRUCTURE_COUNT  64
#define EFI_ACPI_MEMORY_AFFINITY_STRUCTURE_COUNT                10

//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_ARM_OEM_ID           'P','H','Y','L','T','D'   // OEMID 6 bytes long
#define EFI_ACPI_ARM_OEM_TABLE_ID     SIGNATURE_64('P','H','Y','T','I','U','M','.') // OEM table id 8 bytes long
#define EFI_ACPI_ARM_OEM_REVISION     0x1
#define EFI_ACPI_ARM_CREATOR_ID       SIGNATURE_32('P','H','Y','T')
#define EFI_ACPI_ARM_CREATOR_REVISION 0x1

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define ARM_ACPI_HEADER(Signature, Type, Revision) {              \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    { EFI_ACPI_ARM_OEM_ID },        /* UINT8   OemId[6] */        \
    EFI_ACPI_ARM_OEM_TABLE_ID,      /* UINT64  OemTableId */      \
    EFI_ACPI_ARM_OEM_REVISION,      /* UINT32  OemRevision */     \
    EFI_ACPI_ARM_CREATOR_ID,        /* UINT32  CreatorId */       \
    EFI_ACPI_ARM_CREATOR_REVISION   /* UINT32  CreatorRevision */ \
  }

#define ACPI_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'A', 'C', 'P', 'I', 'R', 'A', 'M')
#define CPUS_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'C', 'P', 'U', 'S', 'R', 'A', 'M')

//
// Multiple APIC Description Table
//
#pragma pack (1)

typedef struct {
  EFI_ACPI_6_0_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER          Header;
  EFI_ACPI_6_0_MEMORY_AFFINITY_STRUCTURE                      Memory[EFI_ACPI_MEMORY_AFFINITY_STRUCTURE_COUNT];
  EFI_ACPI_6_0_GICC_AFFINITY_STRUCTURE                        Gicc[EFI_ACPI_PROCESSOR_LOCAL_GICC_AFFINITY_STRUCTURE_COUNT];
} EFI_ACPI_STATIC_RESOURCE_AFFINITY_TABLE;

//FT-2000/4 cpu core max count
#define GIC_INTERFACES_MAX_COUNT		8
#define	CORCOUNT_PER_CLUSTER			4

typedef struct {
  EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER   Header;
  EFI_ACPI_6_1_GIC_STRUCTURE                            GicInterfaces[GIC_INTERFACES_MAX_COUNT];
  EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE                GicDistributor;
  EFI_ACPI_6_1_GIC_ITS_STRUCTURE                        GicITS[1];
} EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE;

typedef struct {
  UINT64      Signature;
  UINT32      RpIoBase[PCI_HOST_MAX_COUNT];
  UINT32      RpIoSize[PCI_HOST_MAX_COUNT];
  UINT32      RpMmioBase[PCI_HOST_MAX_COUNT];
  UINT32      RpMmioSize[PCI_HOST_MAX_COUNT];
  UINT64      RpMmio64Base[PCI_HOST_MAX_COUNT];
  UINT64      RpMmio64Size[PCI_HOST_MAX_COUNT];
  UINT8       RpCount;
  UINT8       RpBusBase[PCI_HOST_MAX_COUNT];
  UINT8       RpBusLimit[PCI_HOST_MAX_COUNT];
} EFI_ACPI_RAM_DATA;

#pragma pack ()


#endif
