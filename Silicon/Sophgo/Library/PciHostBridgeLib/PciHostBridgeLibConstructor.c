/** @file PciHostBridgeLibConstructor.c

  Copyright (c) 2023, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Uefi/UefiSpec.h>
#include <Uefi/UefiBaseType.h>
#include <IndustryStandard/Pci22.h>
#include <Regs/SophgoPciRegs.h>
#include <Library/PlatformPciLib.h>

#define PLAT_CPU_TO_BUS_ADDR  0xCFFFFFFFFF

#define UPPER_32_BITS(n)      ((UINT32)((n) >> 32))
#define LOWER_32_BITS(n)      ((UINT32)((n) & 0xffffffff))

STATIC PCIE_LINK_DESCRIPTOR myPciLinkCfgInfo[] = {
  // RootComplex 0
  {
    PCIE_ID_0,                // PortIndex
    PCIE_LINK_0,              // LinkIndex
    0xfffffff060000000,       // CfgBase
    {                         // Regions
      { 0x0,          0x0,           TRUE,  1, 0x0,  0x40        },
      { 0x0,          0x0,           FALSE, 2, 0x0,  0x100000    },
      { 0x50000000,   0x4050000000,  TRUE,  3, 0x0,  0x10000000  },
      { 0x4100000000, 0x4100000000,  TRUE,  4, 0x0,  0x200000000 }
    }
  },

  // RootComplex 1
  {
    PCIE_ID_0,
    PCIE_LINK_1,
    0xfffffff060800000,
    {
      { 0x40,         0x40,          TRUE,  1, 0x40, 0x40        },
      { 0x400000,     0x400000,      FALSE, 2, 0x40, 0x100000    },
      { 0x80000000,   0x4480000000,  TRUE,  3, 0x40, 0x10000000  },
      { 0x4500000000, 0x4500000000,  TRUE,  4, 0x40, 0x200000000 }
    }
  },

  // RootComplex 2
  {
    PCIE_ID_1,
    PCIE_LINK_0,
    0xfffffff062000000,
    {
      { 0x80,         0x80,          TRUE,  1, 0x80, 0x40        },
      { 0x800000,     0x800000,      FALSE, 2, 0x80, 0x100000    },
      { 0xe0000000,   0x48e0000000,  TRUE,  3, 0x80, 0x10000000  },
      { 0x4900000000, 0x4900000000,  TRUE,  4, 0x80, 0x200000000 }
    }
  },

  // RootComplex 3
  {
    PCIE_ID_1,
    PCIE_LINK_0,
    0xfffffff062800000,
    {
      { 0xc0,         0xc0,          TRUE,  1, 0xc0, 0x40        },
      { 0xc00000,     0x800000,      FALSE, 2, 0xc0, 0x100000    },
      { 0xf0000000,   0x4cf0000000,  TRUE,  3, 0xc0, 0x10000000  },
      { 0x4d00000000, 0x4d00000000,  TRUE,  4, 0xc0, 0x200000000 }
    }
  }
};

STATIC
VOID
PcieHostInitRootPort (
  IN UINT32                   VendorId,
  IN UINT32                   DeviceId,
  IN PCIE_LINK_DESCRIPTOR     *PcieCfg
  )
{
  INT32    Ctrl;
  UINT32   Id;
  UINT32   Value;
  UINT64   CfgBase;

  CfgBase = PcieCfg->CfgBase;
  //
  // Set the root complex BAR configuration register:
  // - disable both BAR0 and BAR1.
  // - enable Prefetchable Memory Base and Limit registers in type 1
  //   config space (64 bits).
  // - enable IO Base and Limit registers in type 1 config
  //   space (32 bits).
  //
  Ctrl = PCIE_LM_BAR_CFG_CTRL_DISABLED;
  Value = PCIE_LM_RC_BAR_CFG_BAR0_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_BAR1_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS |
    PCIE_LM_RC_BAR_CFG_IO_ENABLE |
    PCIE_LM_RC_BAR_CFG_IO_32BITS;
  MmioWrite32 ((CfgBase + PCIE_LM_RC_BAR_CFG), Value);

  //
  // Set root port configuration space
  //
  if (VendorId != 0xffff) {
    Id = PCIE_LM_ID_VENDOR(VendorId) | PCIE_LM_ID_SUBSYS(VendorId);
    MmioWrite32((CfgBase + PCIE_LM_ID), Id);
  }

  if (DeviceId != 0xffff) {
    Value = MmioRead32 (CfgBase + PCIE_RP_BASE + PCI_VENDOR_ID_OFFSET);
    Value &= 0x0000FFFF;
    Value |= (DeviceId << 16);
    MmioWrite32 (CfgBase + PCIE_RP_BASE + PCI_VENDOR_ID_OFFSET, Value);
  }
  MmioWrite32 (CfgBase + PCIE_RP_BASE + PCI_REVISION_ID_OFFSET, PCI_CLASS_BRIDGE_PCI << 16);
}

STATIC
VOID
PcieHostInitAddressTranslation (
  IN PCIE_LINK_DESCRIPTOR      *PcieCfg,
  IN PCIE_AT_INFO              *Region,
  IN UINT32                    NoBarNbits
  )
{
  UINT64        CfgBase;
  UINT32        Addr0;
  UINT32        Addr1;
  UINT32        Desc0;
  UINT32        Desc1;
  UINT64        PciAddr;
  UINT64        CpuAddr;
  BOOLEAN       IsMemory;
  INT32         RegionNumber;
  INT32         BusNumber;
  UINT64        RegionSize;
  UINT32        Nbits;

  CfgBase       = PcieCfg->CfgBase;
  PciAddr       = Region->PciAddr;
  CpuAddr       = Region->CpuAddr;
  IsMemory      = Region->IsMemory;
  RegionNumber  = Region->RegionNumber;
  BusNumber     = Region->BusNumber;
  RegionSize    = Region->RegionSize;
  Nbits         = 0;

  //
  // Get shift amount
  //
  while (RegionSize > 1) {
    RegionSize >>= 1;
    Nbits++;
  }
  //
  // Set the PCI Address for Region 0
  // Reserve region 0 for PCI configure space accesses:
  // OB_REGION_PCI_ADDR0 and OB_REGION_DESC0 are updated dynamically by
  // PciMapBus (), other region registers are set here once for all.
  //
  Addr1 = 0; // Should be programmed to zero.
  Desc1 = PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_PCI_ADDR1(0), Addr1);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_DESC1(0), Desc1);

  //
  // Set the AXI Address for Region 0
  //
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(12) |
          (LOWER_32_BITS(CpuAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(CpuAddr);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_CPU_ADDR0(0), Addr0);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_CPU_ADDR1(0), Addr1);

  if (Nbits < 8) {
    Nbits = 8;
  }

  //
  // Set the PCI address, RegionNumber >= 1
  //
  Addr0 = PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(Nbits) |
          (LOWER_32_BITS(PciAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(PciAddr);

  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_PCI_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_PCI_ADDR1(RegionNumber), Addr1);

  //
  // Set the PCIe header descriptor
  //
  if (IsMemory) {
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_MEM;
  } else {
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_IO;
  }
  Desc1 = 0;
  //
  // In Root Complex mode, the function number is always 0.
  //
  Desc0 |= PCIE_AT_OB_REGION_DESC0_HARDCODED_RID |
           PCIE_AT_OB_REGION_DESC0_DEVFN(0);
  Desc1 |= PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_DESC0(RegionNumber), Desc0);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_DESC1(RegionNumber), Desc1);

  //
  // Set the CPU address
  //
  CpuAddr = CpuAddr & PLAT_CPU_TO_BUS_ADDR; // cpu address fixup
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(Nbits) |
          (LOWER_32_BITS(CpuAddr) & GENMASK(31, 8));
  Addr1 = UPPER_32_BITS(CpuAddr);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_CPU_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (CfgBase + PCIE_AT_OB_REGION_CPU_ADDR1(RegionNumber), Addr1);

  //
  // Set Root Port no BAR match Inbound Translation registers:
  // needed for MSI and DMA.
  // Root Port BAR0 and BAR1 are disabled, hence no need to set
  // their inbound translation registers.
  //
  Addr0 = PCIE_AT_IB_RP_BAR_ADDR0_NBITS (NoBarNbits);
  Addr1 = 0;
  MmioWrite32 (CfgBase + PCIE_AT_IB_RP_BAR_ADDR0(RP_NO_BAR), Addr0);
  MmioWrite32 (CfgBase + PCIE_AT_IB_RP_BAR_ADDR1(RP_NO_BAR), Addr1);
}

EFI_STATUS
EFIAPI
MangoPcieHostBridgeLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UINT32  PortIndex;
  UINT32  LinkIndex;
  UINT32  RegionNum;
  UINT32  VendorId;
  UINT32  DeviceId;
  UINT32  NoBarNbits;

  VendorId = 0x17CD;
  DeviceId = 0x2042;
  //
  // the number least significant bits kept during
  // inbound (PCIe -> AXI) address translations
  //
  NoBarNbits = 0x30;

  DEBUG ((DEBUG_INFO, "Mango PCIe HostBridgeLib constructor:\n"));
  for (PortIndex = 0; PortIndex < PCIE_MAX_PORT; PortIndex++) {
    for (LinkIndex = 0; LinkIndex < PCIE_MAX_LINK; LinkIndex++) {
      if (!((PcdGet8(PcdMangoPcieEnableMask) >>
            ((PCIE_MAX_PORT * PortIndex) + LinkIndex)) & 0x01)) {
        continue;
      }
      PcieHostInitRootPort (VendorId,
                            DeviceId,
                            &myPciLinkCfgInfo[(PCIE_MAX_PORT * PortIndex) + LinkIndex]);
      for (RegionNum = 0; RegionNum < 4; RegionNum++) {
        PcieHostInitAddressTranslation (&myPciLinkCfgInfo[(PCIE_MAX_PORT * PortIndex) + LinkIndex],
                                        &myPciLinkCfgInfo[(PCIE_MAX_PORT * PortIndex) + LinkIndex].Region[RegionNum],
                                        NoBarNbits);
      }
      DEBUG ((
        DEBUG_INFO,
        "%a: PCIe Port %d, Link %d initialization success.\n",
        __func__,
        PortIndex,
        LinkIndex
      ));
    }
  }

  MicroSecondDelay(300);

  return EFI_SUCCESS;
}