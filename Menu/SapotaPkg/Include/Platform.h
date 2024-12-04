/** @file
  The definitions for ACPI tables.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  PLATFORM_H_
#define  PLATFORM_H_

#include <IndustryStandard/Acpi.h>

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

#define UART1_BASE                    FixedPcdGet64 (PcdSerialRegisterBase)
#define UART1_BASE_MAX                FixedPcdGet64 (PcdUart1AddressMax)

#define GICD_BASE                     FixedPcdGet64 (PcdGicDistributorBase)
#define GICR_BASE                     FixedPcdGet64 (PcdGicRedistributorsBase)

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
#define PLATFORM_GET_MPID(DieId, ClusterId, CoreId)   ((UINT64)DieId << 32 | ((ClusterId) << 16) | (CoreId << 8))
#define DIE_BASE_OFFSET    0x100000000000
#define EFI_GICR_BASE_OFFSET(DieId, ClusterId, CoreId)  ((16 * DieId + ClusterId) / 16 * DIE_BASE_OFFSET + ClusterId * 0x20000)

// EFI_ACPI_6_4_GIC_STRUCTURE
#define EFI_ACPI_6_4_GICC_STRUCTURE_INIT(GicId, AcpiCpuUid, Mpidr, Flags,      \
  PmuIrq, GicBase, GicVBase, GicHBase, GsivId, GicRBase, Efficiency,           \
  SpeOverflowInterrupt)                                                        \
  {                                                                            \
    EFI_ACPI_6_4_GIC,                     /* Type */                           \
    sizeof (EFI_ACPI_6_4_GIC_STRUCTURE),  /* Length */                         \
    EFI_ACPI_RESERVED_WORD,               /* Reserved */                       \
    GicId,                                /* CPUInterfaceNumber */             \
    AcpiCpuUid,                           /* AcpiProcessorUid */               \
    Flags,                                /* Flags */                          \
    0,                                    /* ParkingProtocolVersion */         \
    PmuIrq,                               /* PerformanceInterruptGsiv */       \
    0,                                    /* ParkedAddress */                  \
    GicBase,                              /* PhysicalBaseAddress */            \
    GicVBase,                             /* GICV */                           \
    GicHBase,                             /* GICH */                           \
    GsivId,                               /* VGICMaintenanceInterrupt */       \
    GicRBase,                             /* GICRBaseAddress */                \
    Mpidr,                                /* MPIDR */                          \
    Efficiency,                           /* ProcessorPowerEfficiencyClass */  \
    EFI_ACPI_RESERVED_BYTE,               /* Reserved2 */                      \
    SpeOverflowInterrupt                  /* SpeOverflowInterrupt */           \
  }

#define EFI_GICC_STRUCTURE(AcpiCpuUid, Mpidr, GicRBaseOffset)                  \
  EFI_ACPI_6_4_GICC_STRUCTURE_INIT(                                            \
    0, AcpiCpuUid, Mpidr, EFI_ACPI_6_4_GIC_ENABLED, 23,                        \
    0, 0, 0,                                                                   \
    25, GICR_BASE + GicRBaseOffset,                                            \
    0, 0                                                                       \
    )

// EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE
#define EFI_ACPI_6_4_GIC_DISTRIBUTOR_INIT(GicDistHwId, GicDistBase,            \
  GicDistVector, GicVersion)                                                   \
  {                                                                            \
    EFI_ACPI_6_4_GICD,                    /* Type */                           \
    sizeof (EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE),                           \
    EFI_ACPI_RESERVED_WORD,               /* Reserved1 */                      \
    GicDistHwId,                          /* GicId */                          \
    GicDistBase,                          /* PhysicalBaseAddress */            \
    GicDistVector,                        /* SystemVectorBase */               \
    GicVersion,                           /* GicVersion */                     \
    {                                                                          \
      EFI_ACPI_RESERVED_BYTE,             /* Reserved2[0] */                   \
      EFI_ACPI_RESERVED_BYTE,             /* Reserved2[1] */                   \
      EFI_ACPI_RESERVED_BYTE              /* Reserved2[2] */                   \
    }                                                                          \
  }

// EFI_ACPI_6_4_GICR_STRUCTURE
#define EFI_ACPI_6_4_GIC_REDISTRIBUTOR_INIT(RedisRegionAddr, RedisDiscLength)  \
  {                                                                            \
    EFI_ACPI_6_4_GICR,                    /* Type */                           \
    sizeof (EFI_ACPI_6_4_GICR_STRUCTURE), /* Length */                         \
    EFI_ACPI_RESERVED_WORD,               /* Reserved */                       \
    RedisRegionAddr,                      /* DiscoveryRangeBaseAddress */      \
    RedisDiscLength                       /* DiscoveryRangeLength */           \
  }

// EFI_ACPI_6_4_GIC_ITS_STRUCTURE
#define EFI_ACPI_6_4_GIC_ITS_INIT(GicItsId, GicItsBase)                        \
  {                                                                            \
    EFI_ACPI_6_4_GIC_ITS,                 /* Type */                           \
    sizeof (EFI_ACPI_6_4_GIC_ITS_STRUCTURE),                                   \
    EFI_ACPI_RESERVED_WORD,               /* Reserved */                       \
    GicItsId,                             /* GicItsId */                       \
    GicItsBase,                           /* PhysicalBaseAddress */            \
    EFI_ACPI_RESERVED_DWORD               /* DiscoveryRangeLength */           \
  }

#define ACPI_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'A', 'C', 'P', 'I', 'R', 'A', 'M')
#define CPUS_RAM_DATA_SIGNATURE  SIGNATURE_64('$', 'C', 'P', 'U', 'S', 'R', 'A', 'M')

//
// Multiple APIC Description Table
//
#pragma pack (1)

// EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE
#define EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE_INIT(                           \
          ProximityDomain, Base, Length, Flags)                                \
  {                                                                            \
    1, sizeof (EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE), ProximityDomain,       \
    EFI_ACPI_RESERVED_WORD, (Base) & 0xffffffff,                               \
    (Base) >> 32, (Length) & 0xffffffff,                                       \
    (Length) >> 32, EFI_ACPI_RESERVED_DWORD, Flags,                            \
    EFI_ACPI_RESERVED_QWORD                                                    \
  }

// EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE
#define EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE_INIT(                             \
          ProximityDomain, ACPIProcessorUID, Flags, ClockDomain)               \
  {                                                                            \
    3, sizeof (EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE), ProximityDomain,         \
    ACPIProcessorUID,  Flags,  ClockDomain                                     \
  }

typedef struct {
  EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER          Header;
  EFI_ACPI_6_4_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY_STRUCTURE  Apic;
  EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE                      Memory[1];
  EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE                        Gicc[1];
} EFI_ACPI_STATIC_RESOURCE_AFFINITY_TABLE;

#define GIC_INTERFACES_MAX_COUNT    4
#define CORCOUNT_PER_CLUSTER        4

typedef struct {
  EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER   Header;
  EFI_ACPI_6_4_GIC_STRUCTURE                            GicInterfaces[1];
  EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE                GicDistributor;
} EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE;

#pragma pack ()

#endif // PLATFORM_H
