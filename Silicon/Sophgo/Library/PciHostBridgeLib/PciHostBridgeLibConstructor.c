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

STATIC
VOID
PcieHostInitRootPort (
  IN UINT32                   VendorId,
  IN UINT32                   DeviceId,
  IN UINT64                   CfgBase
  )
{
  INT32    Ctrl;
  UINT32   Id;
  UINT32   Value;

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
  IN UINT32                   NoBarNbits,
  IN UINT64                   CfgBase,
  IN INT32                    BusNumber,
  IN INT32                    RegionNumber,
  IN BOOLEAN                  IsMemory,
  IN UINT64                   PciAddr,
  IN UINT64                   CpuAddr,
  IN UINT64                   RegionSize
  )
{
  UINT32        Addr0;
  UINT32        Addr1;
  UINT32        Desc0;
  UINT32        Desc1;
  UINT32        Nbits;

  //
  // Get shift amount
  //
  Nbits         = 0;
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
                            mPciResource[PortIndex][LinkIndex].ConfigSpaceAddress + 0xffffff8000000000);

      //
      // Bus
      //
      PcieHostInitAddressTranslation (NoBarNbits,
                                      mPciResource[PortIndex][LinkIndex].ConfigSpaceAddress + 0xffffff8000000000,
                                      1,
                                      TRUE,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      mPciResource[PortIndex][LinkIndex].BusSize);
      //
      // IO
      //
      PcieHostInitAddressTranslation (NoBarNbits,
                                      mPciResource[PortIndex][LinkIndex].ConfigSpaceAddress + 0xffffff8000000000,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      2,
                                      FALSE,
                                      mPciResource[PortIndex][LinkIndex].IoBase,
                                      mPciResource[PortIndex][LinkIndex].IoBase -
                                      mPciResource[PortIndex][LinkIndex].IoTranslation,
                                      mPciResource[PortIndex][LinkIndex].IoSize);
      //
      // Mem32
      //
      PcieHostInitAddressTranslation (NoBarNbits,
                                      mPciResource[PortIndex][LinkIndex].ConfigSpaceAddress + 0xffffff8000000000,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      3,
                                      TRUE,
                                      mPciResource[PortIndex][LinkIndex].Mmio32Base,
                                      mPciResource[PortIndex][LinkIndex].Mmio32Base -
                                      mPciResource[PortIndex][LinkIndex].Mmio32Translation,
                                      mPciResource[PortIndex][LinkIndex].Mmio32Size);
      //
      // MemAbove4G
      //
      PcieHostInitAddressTranslation (NoBarNbits,
                                      mPciResource[PortIndex][LinkIndex].ConfigSpaceAddress + 0xffffff8000000000,
                                      mPciResource[PortIndex][LinkIndex].BusBase,
                                      4,
                                      TRUE,
                                      mPciResource[PortIndex][LinkIndex].Mmio64Base,
                                      mPciResource[PortIndex][LinkIndex].Mmio64Base -
                                      mPciResource[PortIndex][LinkIndex].Mmio64Translation,
                                      mPciResource[PortIndex][LinkIndex].Mmio64Size);

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