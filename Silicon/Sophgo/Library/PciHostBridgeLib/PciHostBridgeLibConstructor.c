/** @file PcieCore.c

  Copyright (c) 2023, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-3-Clause

**/
#include <Regs/SophgoPcieRegs.h>
#include <Library/IoLib.h>
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Library/TimerLib.h>
#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>

#define lower_32_bits(n)     ((UINT32)(n) & 0xffffffff)
#define upper_32_bits(n)     ((UINT32)((((n) >> 16) >> 16)))

// #define PLAT_CPU_TO_BUS_ADDR  0xCFFFFFFFFF

STATIC
VOID
PcieHostInitRootPort (
  IN PCIE_PORT    Port,
  IN PCIE_LINK_ID LinkId,
  IN UINT32       VendorId,
  IN UINT32       DeviceId
  )
{
  UINT32 Value;
  INT32 Ctrl;
  UINT32 Id;
  UINT64 ApbBase = 0;

  // 0x7062000000
  ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);

  /*
   * Set the root complex BAR configuration register:
   * - disable both BAR0 and BAR1.
   * - enable Prefetchable Memory Base and Limit registers in type 1
   *   config space (64 bits).
   * - enable IO Base and Limit registers in type 1 config
   *   space (32 bits).
   */
  Ctrl = PCIE_LM_BAR_CFG_CTRL_DISABLED;
  Value = PCIE_LM_RC_BAR_CFG_BAR0_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_BAR1_CTRL(Ctrl) |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE |
    PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS |
    PCIE_LM_RC_BAR_CFG_IO_ENABLE |
    PCIE_LM_RC_BAR_CFG_IO_32BITS;
  MmioWrite32 ((ApbBase + PCIE_LM_RC_BAR_CFG), Value);
  // MmioWrite32((ApbBase + PCIE_LM_RC_BAR_CFG), 0x1E0000);

  // // Root Complex
  // MmioWrite32((ApbBase + PCIE_LM_OFFSET + LINKX_LM_RC_BAR_CFG_REG), 0x1E0000);
  /* TBD */
  /* Set root port configuration space */
  if (VendorId != 0xffff) {
    Id = PCIE_LM_ID_VENDOR(VendorId) | PCIE_LM_ID_SUBSYS(VendorId);
    MmioWrite32((ApbBase + PCIE_LM_ID), Id);
  }

  if (DeviceId != 0xffff) {
    Value = MmioRead32 (ApbBase + PCIE_RP_BASE + PCI_VENDOR_ID);
    Value &= 0x0000FFFF;
    Value |= (DeviceId << 16);
    MmioWrite32 (ApbBase + PCIE_RP_BASE + PCI_VENDOR_ID, Value);
  }
  MmioWrite32 (ApbBase + PCIE_RP_BASE + PCI_CLASS_REVISION, PCI_CLASS_BRIDGE_PCI << 16);
}

STATIC
VOID
PcieHostInitAddressTranslation (
  IN PCIE_PORT     Port,
  IN PCIE_LINK_ID  LinkId,
  IN UINT64        PciAddr,
  IN UINT64        CpuAddr,
  IN BOOLEAN       IsMemory,
  IN INT32         RegionNumber,
  IN INT32         BusNumber,
  IN UINT32        Nbits)
{
  UINT64 ApbBase = 0;
  UINT32 Addr0;
  UINT32 Addr1;
  UINT32 Desc0;
  UINT32 Desc1;

  // 0x7062000000
  ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);

  /*
   * Reserve region 0 for PCI configure space accesses:
   * OB_REGION_PCI_ADDR0 and OB_REGION_DESC0 are updated dynamically by
   * cdns_pci_map_bus(), other region registers are set here once for all.
   */
  /* Set the PCI Address for Region 0 */
  Addr1 = 0; // Should be programmed to zero.
  Desc1 = PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_PCI_ADDR1(0), Addr1);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_DESC1(0), Desc1);

  DEBUG ((DEBUG_INIT, "\n----%a[%u]----ApbBase=0x%lx---\n", __FUNCTION__, __LINE__, ApbBase));
  DEBUG ((DEBUG_INIT, "----%a[%u]----Bus Number=%d---\n", __FUNCTION__, __LINE__, BusNumber));
  DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Desc1));
  DEBUG ((DEBUG_INIT, "%a[%u]----cpu_addr sc.AXI Base Address[63:0]=0x%lx---\n", __FUNCTION__, __LINE__, CpuAddr));

  /* Set the AXI Address for Region 0 */ 
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(12) | (lower_32_bits(CpuAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(CpuAddr);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_CPU_ADDR0(0), Addr0);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_CPU_ADDR1(0), Addr1);

  DEBUG ((DEBUG_INIT, "%a[%u]----Region 0: AXI Base Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Region 0: AXI Base Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));

  if (Nbits < 8) {
    Nbits = 8;
  }
  /* RegionNumber >= 1 */
  /* Set the PCI address */
  Addr0 = PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(Nbits) | (lower_32_bits(PciAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(PciAddr);
  DEBUG ((DEBUG_INIT, "\n%a[%u]----PCIe Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_PCI_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_PCI_ADDR1(RegionNumber), Addr1);

  /* Set the PCIe header descriptor */
  if (IsMemory)
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_MEM;
  else
    Desc0 = PCIE_AT_OB_REGION_DESC0_TYPE_IO;
  Desc1 = 0;

  Desc0 |= PCIE_AT_OB_REGION_DESC0_HARDCODED_RID | PCIE_AT_OB_REGION_DESC0_DEVFN(0);
  Desc1 |= PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_DESC0(RegionNumber), Desc0);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_DESC1(RegionNumber), Desc1);
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Desc0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Desc1));

  /* Set the CPU address */
  Addr0 = PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(Nbits) | (lower_32_bits(CpuAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(CpuAddr);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_CPU_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (ApbBase + PCIE_AT_OB_REGION_CPU_ADDR1(RegionNumber), Addr1);

  DEBUG ((DEBUG_INIT, "%a[%u]----Region i: AXI Base Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Region i: AXI Base Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  /*
   * Set Root Port no BAR match Inbound Translation registers: needed for MSI and DMA.
   * Root Port BAR0 and BAR1 are disabled, hence no need to set their inbound translation registers.
   */
  Addr0 = 0x2F;
  Addr1 = 0;
  MmioWrite32 (ApbBase + PCIE_AT_IB_RP_BAR_ADDR0(RP_NO_BAR), Addr0);
  MmioWrite32 (ApbBase + PCIE_AT_IB_RP_BAR_ADDR1(RP_NO_BAR), Addr1);
}

EFI_STATUS
EFIAPI
SG2042PcieHostBridgeLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  UINT32  PortId   = 1;
  // UINT32  PortId   = 0;
  UINT32  BusNumber = 0x80;
  UINT32  LinkId   = 0;
  UINT32  VendorId = 0x1E30;
  UINT32  DeviceId = 0x2042;

  DEBUG ((DEBUG_ERROR, "PCIe RootBridge constructor\n"));

  PcieHostInitRootPort(PortId, LinkId, VendorId, DeviceId);
  PcieHostInitAddressTranslation(PortId, LinkId, 0x80,         0x80,         TRUE,  1, BusNumber, 7);
  // IO Space
  PcieHostInitAddressTranslation(PortId, LinkId, 0x800000,     0x800000,     FALSE, 2, BusNumber, 23);
  // Memory Space
  PcieHostInitAddressTranslation(PortId, LinkId, 0x80000000,   0x4880000000, TRUE,  3, BusNumber, 32);
  // MemAbove4G Space
  PcieHostInitAddressTranslation(PortId, LinkId, 0x4900000000, 0x4900000000, TRUE,  4, BusNumber, 33);

  MicroSecondDelay(300);

  DEBUG ((DEBUG_ERROR, "PCIe Port 1, link 0 Init!\n"));

  return EFI_SUCCESS;
}