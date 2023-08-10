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

#define BIT(nr)              (1 << (nr))
// #define GENMASK(end, start)  (((1ULL << ((end) - (start) + 1)) - 1) << (start))
#define lower_32_bits(n)     ((UINT32)(n) & 0xffffffff)
#define upper_32_bits(n)     ((UINT32)((((n) >> 16) >> 16)))
//
// Under PCI, each device has 256 bytes of configuration address space,
// of which the first 64 bytes are standardized as follows:
//
#define PCI_VENDOR_ID           0x00    // 16 bits
#define PCI_DEVICE_ID           0x02    // 16 bits
#define PCI_CLASS_REVISION      0x08    // High 24 bits are class, low 8 revision
//
// Device classes and subclasses
//
#define PCI_CLASS_BRIDGE_PCI    0x0604
//
// Local Management Registers
//
#define CDNS_PCIE_LM_BASE       0x00100000

//
// Vendor ID Register
//
#define CDNS_PCIE_LM_ID               (CDNS_PCIE_LM_BASE + 0x0044)
#define  CDNS_PCIE_LM_ID_VENDOR_MASK  GENMASK(15, 0)
#define  CDNS_PCIE_LM_ID_VENDOR_SHIFT 0
#define  CDNS_PCIE_LM_ID_VENDOR(vid) \
         (((vid) << CDNS_PCIE_LM_ID_VENDOR_SHIFT) & CDNS_PCIE_LM_ID_VENDOR_MASK)
#define  CDNS_PCIE_LM_ID_SUBSYS_MASK  GENMASK(31, 16)
#define  CDNS_PCIE_LM_ID_SUBSYS_SHIFT 16
#define  CDNS_PCIE_LM_ID_SUBSYS(sub) \
         (((sub) << CDNS_PCIE_LM_ID_SUBSYS_SHIFT) & CDNS_PCIE_LM_ID_SUBSYS_MASK)

//
// Root Port Requestor ID Register
//
#define CDNS_PCIE_LM_RP_RID                              (CDNS_PCIE_LM_BASE + 0x0228)
#define  CDNS_PCIE_LM_RP_RID_MASK                        GENMASK(15, 0)
#define  CDNS_PCIE_LM_RP_RID_SHIFT                       0
#define  CDNS_PCIE_LM_RP_RID_(rid) \
         (((rid) << CDNS_PCIE_LM_RP_RID_SHIFT) & CDNS_PCIE_LM_RP_RID_MASK)

//
// Root Complex BAR Configuration Register
//
#define CDNS_PCIE_LM_RC_BAR_CFG                          (CDNS_PCIE_LM_BASE + 0x0300)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR0_APERTURE_MASK      GENMASK(5, 0)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR0_APERTURE(a) \
         (((a) << 0) & CDNS_PCIE_LM_RC_BAR_CFG_BAR0_APERTURE_MASK)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR0_CTRL_MASK          GENMASK(8, 6)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR0_CTRL(c) \
         (((c) << 6) & CDNS_PCIE_LM_RC_BAR_CFG_BAR0_CTRL_MASK)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR1_APERTURE_MASK      GENMASK(13, 9)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR1_APERTURE(a) \
         (((a) << 9) & CDNS_PCIE_LM_RC_BAR_CFG_BAR1_APERTURE_MASK)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR1_CTRL_MASK          GENMASK(16, 14)
#define  CDNS_PCIE_LM_RC_BAR_CFG_BAR1_CTRL(c) \
         (((c) << 14) & CDNS_PCIE_LM_RC_BAR_CFG_BAR1_CTRL_MASK)
#define  CDNS_PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE     BIT(17)
#define  CDNS_PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_32BITS     0
#define  CDNS_PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS     BIT(18)
#define  CDNS_PCIE_LM_RC_BAR_CFG_IO_ENABLE               BIT(19)
#define  CDNS_PCIE_LM_RC_BAR_CFG_IO_16BITS               0
#define  CDNS_PCIE_LM_RC_BAR_CFG_IO_32BITS               BIT(20)
#define  CDNS_PCIE_LM_RC_BAR_CFG_CHECK_ENABLE            BIT(31)

//
// BAR control values applicable to both Endpoint Function and Root Complex
//
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_DISABLED              0x0
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_IO_32BITS             0x1
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_32BITS            0x4
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_32BITS   0x5
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_64BITS            0x6
#define  CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_64BITS   0x7

#define LM_RC_BAR_CFG_CTRL_DISABLED(bar) 	\
      (CDNS_PCIE_LM_BAR_CFG_CTRL_DISABLED << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_IO_32BITS(bar) 	\
      (CDNS_PCIE_LM_BAR_CFG_CTRL_IO_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_MEM_32BITS(bar) 	\
      (CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_PREF_MEM_32BITS(bar) \
      (CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_MEM_64BITS(bar) 	\
      (CDNS_PCIE_LM_BAR_CFG_CTRL_MEM_64BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_PREF_MEM_64BITS(bar) \
 (CDNS_PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_64BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_APERTURE(bar, aperture) 	\
      (((aperture) - 2) << ((bar) * 8))

//
// Root Port Registers (PCI configuration space for the root port function)
//
#define CDNS_PCIE_RP_BASE       0x00200000
#define CDNS_PCIE_RP_CAP_OFFSET 0xc0

//
// Address Translation Registers
//
#define CDNS_PCIE_AT_BASE 0x00400000

//
// Region r Outbound AXI to PCIe Address Translation Register 0
//
#define CDNS_PCIE_AT_OB_REGION_PCI_ADDR0(r) \
        (CDNS_PCIE_AT_BASE + 0x0000 + ((r) & 0x1f) * 0x0020)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK    GENMASK(5, 0)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK    GENMASK(19, 12)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(devfn) \
         (((devfn) << 12) & CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK      GENMASK(27, 20)
#define  CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS(bus) \
         (((bus) << 20) & CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK)

//
// Region r Outbound AXI to PCIe Address Translation Register 1
//
#define CDNS_PCIE_AT_OB_REGION_PCI_ADDR1(r) \
        (CDNS_PCIE_AT_BASE + 0x0004 + ((r) & 0x1f) * 0x0020)

//
// Region r Outbound PCIe Descriptor Register 0
//
#define CDNS_PCIE_AT_OB_REGION_DESC0(r) \
        (CDNS_PCIE_AT_BASE + 0x0008 + ((r) & 0x1f) * 0x0020)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_MASK       GENMASK(3, 0)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_MEM        0x2
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_IO         0x6
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0 0xa
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1 0xb
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_NORMAL_MSG 0xc
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_VENDOR_MSG 0xd
/* Bit 23 MUST be set in RC mode. */
#define  CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID   BIT(23)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN_MASK      GENMASK(31, 24)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(devfn) \
         (((devfn) << 24) & CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN_MASK)

//
// Region r Outbound PCIe Descriptor Register 1
//
#define CDNS_PCIE_AT_OB_REGION_DESC1(r) \
        (CDNS_PCIE_AT_BASE + 0x000c + ((r) & 0x1f) * 0x0020)
#define  CDNS_PCIE_AT_OB_REGION_DESC1_BUS_MASK        GENMASK(7, 0)
#define  CDNS_PCIE_AT_OB_REGION_DESC1_BUS(bus) \
         ((bus) & CDNS_PCIE_AT_OB_REGION_DESC1_BUS_MASK)

//
// Region r AXI Region Base Address Register 0
//
#define CDNS_PCIE_AT_OB_REGION_CPU_ADDR0(r) \
         (CDNS_PCIE_AT_BASE + 0x0018 + ((r) & 0x1f) * 0x0020)
#define  CDNS_PCIE_AT_OB_REGION_CPU_ADDR0_NBITS_MASK  GENMASK(5, 0)
#define  CDNS_PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & CDNS_PCIE_AT_OB_REGION_CPU_ADDR0_NBITS_MASK)

//
// Region r AXI Region Base Address Register 1
//
#define CDNS_PCIE_AT_OB_REGION_CPU_ADDR1(r) \
        (CDNS_PCIE_AT_BASE + 0x001c + ((r) & 0x1f) * 0x0020)

//
// Root Port BAR Inbound PCIe to AXI Address Translation Register
//
#define CDNS_PCIE_AT_IB_RP_BAR_ADDR0(bar) \
        (CDNS_PCIE_AT_BASE + 0x0800 + (bar) * 0x0008)
#define  CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK    GENMASK(5, 0)
#define  CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK)
#define CDNS_PCIE_AT_IB_RP_BAR_ADDR1(bar) \
        (CDNS_PCIE_AT_BASE + 0x0804 + (bar) * 0x0008)

//
// Root Port BAR Inbound PCIe to AXI Address Translation Register
//
#define CDNS_PCIE_AT_IB_RP_BAR_ADDR0(bar) \
        (CDNS_PCIE_AT_BASE + 0x0800 + (bar) * 0x0008)
#define  CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK      GENMASK(5, 0)
#define  CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & CDNS_PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK)
#define CDNS_PCIE_AT_IB_RP_BAR_ADDR1(bar) \
        (CDNS_PCIE_AT_BASE + 0x0804 + (bar) * 0x0008)

//
// AXI link down register
//
#define CDNS_PCIE_AT_LINKDOWN (CDNS_PCIE_AT_BASE + 0x0824)

//
// LTSSM Capabilities register
//
#define CDNS_PCIE_LTSSM_CONTROL_CAP             (CDNS_PCIE_LM_BASE + 0x0054)
#define  CDNS_PCIE_DETECT_QUIET_MIN_DELAY_MASK  GENMASK(2, 1)
#define  CDNS_PCIE_DETECT_QUIET_MIN_DELAY_SHIFT 1
#define  CDNS_PCIE_DETECT_QUIET_MIN_DELAY(delay) \
         (((delay) << CDNS_PCIE_DETECT_QUIET_MIN_DELAY_SHIFT) & \
         CDNS_PCIE_DETECT_QUIET_MIN_DELAY_MASK)

enum cdns_pcie_rp_bar {
  RP_BAR_UNDEFINED = -1,
  RP_BAR0,
  RP_BAR1,
  RP_NO_BAR
};  

// #define CDNS_PLAT_CPU_TO_BUS_ADDR  0xCFFFFFFFFF

// STATIC
// VOID
// MangoPcieMapBus (
//   IN PCIE_PORT Port,
//   IN PCIE_LINK_ID LinkId,
//   IN UINT32 Func,
//   IN UINT32 BusNumber
//   )
// {
//   UINT32 ApbBase = 0;
//   UINT32 Value = 0;
//   UINT32 LinkStatus = 0;

//   ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);
//   if (Func > 7)
//     return NULL;
//   /* Check that the link is up */
//   if (!(MmioRead32 (ApbBase + PCIE_LM_OFFSET) & 0x1))
//     return NULL;
  
//   /* Clear AXI link-down status */
//   MmioWrite32 (ApbBase + PCIE_AXI_OFFSET + 0x0824, 0x0);

//   /* Update Output registers for AXI region 0. */
//   MmioWrite32 ((ApbBase + PCIE_AXI_OFFSET + LINKX_AXI_R0_OB_AT_REG_0), (100000 + 0x13));
//   MmioWrite32 ((ApbBase + PCIE_AXI_OFFSET + LINKX_AXI_R0_OB_AT_REG_1), 0);

//   /* Configuration Type 0 or Type 1 access. */
//   // 1010 = Type 0 config;
//   // 1011 = Type 1 config;
//   if (BusNumber == 1)
//     MmioWrite32 ((ApbBase + PCIE_AXI_OFFSET + LINKX_AXI_R0_OB_DSPT_REG_0), 0x80000a);
//   else
//     MmioWrite32 ((ApbBase + PCIE_AXI_OFFSET + LINKX_AXI_R0_OB_DSPT_REG_0), 0x80000b);
// }

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
  Ctrl = CDNS_PCIE_LM_BAR_CFG_CTRL_DISABLED;
  Value = CDNS_PCIE_LM_RC_BAR_CFG_BAR0_CTRL(Ctrl) |
    CDNS_PCIE_LM_RC_BAR_CFG_BAR1_CTRL(Ctrl) |
    CDNS_PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE |
    CDNS_PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS |
    CDNS_PCIE_LM_RC_BAR_CFG_IO_ENABLE |
    CDNS_PCIE_LM_RC_BAR_CFG_IO_32BITS;
  MmioWrite32 ((ApbBase + CDNS_PCIE_LM_RC_BAR_CFG), Value);
  // MmioWrite32((ApbBase + CDNS_PCIE_LM_RC_BAR_CFG), 0x1E0000);
  DEBUG ((DEBUG_INIT, "%a[%u]----LM: Root Complex BAR Configuration=0x%lx---\n", __FUNCTION__, __LINE__, Value));

  // // Root Complex
  // MmioWrite32((ApbBase + PCIE_LM_OFFSET + LINKX_LM_RC_BAR_CFG_REG), 0x1E0000);
  /* TBD */
  /* Set root port configuration space */
  if (VendorId != 0xffff) {
    Id = CDNS_PCIE_LM_ID_VENDOR(VendorId) | CDNS_PCIE_LM_ID_SUBSYS(VendorId);
    DEBUG ((DEBUG_INIT, "%a[%u]----LM: Vendor ID=0x%lx---\n", __FUNCTION__, __LINE__, Id));
    MmioWrite32((ApbBase + CDNS_PCIE_LM_ID), Id);
  }

  if (DeviceId != 0xffff) {
    Value = MmioRead32 (ApbBase + CDNS_PCIE_RP_BASE + PCI_VENDOR_ID);
    DEBUG ((DEBUG_INIT, "%a[%u]----RP: after read=0x%lx---\n", __FUNCTION__, __LINE__, Value));
    Value &= 0x0000FFFF;
    Value |= (DeviceId << 16);
    MmioWrite32 (ApbBase + CDNS_PCIE_RP_BASE + PCI_VENDOR_ID, Value);
    DEBUG ((DEBUG_INIT, "%a[%u]----RP: PCI_VENDOR_ID=0x%lx---\n", __FUNCTION__, __LINE__, Value));
  }
  MmioWrite32 (ApbBase + CDNS_PCIE_RP_BASE + PCI_CLASS_REVISION, PCI_CLASS_BRIDGE_PCI << 16);
  DEBUG ((DEBUG_INIT, "%a[%u]----RP: Sub-Class Code=0x%lx---\n", __FUNCTION__, __LINE__, Value));
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
  Desc1 = CDNS_PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_PCI_ADDR1(0), Addr1);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_DESC1(0), Desc1);

  DEBUG ((DEBUG_INIT, "\n----%a[%u]----ApbBase=0x%lx---\n", __FUNCTION__, __LINE__, ApbBase));
  DEBUG ((DEBUG_INIT, "----%a[%u]----Bus Number=%d---\n", __FUNCTION__, __LINE__, BusNumber));
  DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Desc1));
  DEBUG ((DEBUG_INIT, "%a[%u]----cpu_addr sc.AXI Base Address[63:0]=0x%lx---\n", __FUNCTION__, __LINE__, CpuAddr));

  /* Set the AXI Address for Region 0 */ 
  Addr0 = CDNS_PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(12) | (lower_32_bits(CpuAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(CpuAddr);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_CPU_ADDR0(0), Addr0);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_CPU_ADDR1(0), Addr1);

  DEBUG ((DEBUG_INIT, "%a[%u]----Region 0: AXI Base Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Region 0: AXI Base Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));

  if (Nbits < 8) {
    Nbits = 8;
  }
  /* RegionNumber >= 1 */
  /* Set the PCI address */
  Addr0 = CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(Nbits) | (lower_32_bits(PciAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(PciAddr);
  DEBUG ((DEBUG_INIT, "\n%a[%u]----PCIe Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_PCI_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_PCI_ADDR1(RegionNumber), Addr1);

  /* Set the PCIe header descriptor */
  if (IsMemory)
    Desc0 = CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_MEM;
  else
    Desc0 = CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_IO;
  Desc1 = 0;

  Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID | CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(0);
  Desc1 |= CDNS_PCIE_AT_OB_REGION_DESC1_BUS(BusNumber);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_DESC0(RegionNumber), Desc0);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_DESC1(RegionNumber), Desc1);
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Desc0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Desc1));

  /* Set the CPU address */
  Addr0 = CDNS_PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(Nbits) | (lower_32_bits(CpuAddr) & GENMASK(31, 8));
  Addr1 = upper_32_bits(CpuAddr);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_CPU_ADDR0(RegionNumber), Addr0);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_CPU_ADDR1(RegionNumber), Addr1);

  DEBUG ((DEBUG_INIT, "%a[%u]----Region i: AXI Base Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----Region i: AXI Base Address[63:32]=0x%lx---\n", __FUNCTION__, __LINE__, Addr1));
  /*
   * Set Root Port no BAR match Inbound Translation registers: needed for MSI and DMA.
   * Root Port BAR0 and BAR1 are disabled, hence no need to set their inbound translation registers.
   */
  Addr0 = 0x2F;
  Addr1 = 0;
  DEBUG ((DEBUG_INIT, "%a[%u]----BAR0 Root Port Inbound PCIe to AXI Address Translation[31:0]=0x%X---\n", __FUNCTION__, __LINE__, Addr0));
  DEBUG ((DEBUG_INIT, "%a[%u]----BAR0 Root Port Inbound PCIe to AXI Address Translation[63:32]=0x%X---\n", __FUNCTION__, __LINE__, Addr1));
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_IB_RP_BAR_ADDR0(RP_NO_BAR), Addr0);
  MmioWrite32 (ApbBase + CDNS_PCIE_AT_IB_RP_BAR_ADDR1(RP_NO_BAR), Addr1);
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
  UINT32  LinkId   = 0;
  UINT32  VendorId = 0x1E30;
  UINT32  DeviceId = 0x2042;

  DEBUG ((DEBUG_ERROR, "PCIe RootBridge constructor\n"));

  PcieHostInitRootPort(PortId, LinkId, VendorId, DeviceId);
  PcieHostInitAddressTranslation(PortId, LinkId, 0x80,         0x80,         TRUE,  1, 0, 7);
  // IO Space
  PcieHostInitAddressTranslation(PortId, LinkId, 0x800000,     0x800000,     FALSE, 2, 0, 23);
  // Memory Space
  PcieHostInitAddressTranslation(PortId, LinkId, 0x80000000, 0x4880000000, TRUE, 3, 0, 32);

  // MemAbove4G Space
  // PcieHostInitAddressTranslation(PortId, LinkId, 0x4b00000000, 0x4b00000000, TRUE, 3, 0, 32);
  PcieHostInitAddressTranslation(PortId, LinkId, 0x4900000000, 0x4900000000, TRUE, 4, 0, 33);

  // PcieHostInitAddressTranslation(0, 0, 0x00,         0x00,         TRUE,  1, 0, 7);
  // PcieHostInitAddressTranslation(0, 0, 0x4300000000, 0x44FFFFFFFF, TRUE,  6, 0, 32);
  // PcieHostInitAddressTranslation(0, 0, 0x4300000000, 0x44FFFFFFFF, TRUE,  6, 0, 32);
  // PcieHostInitAddressTranslation(PortId, LinkId, 0x7063800000, 0x7063800000, FALSE, 2, 0, 23);

  // PcieHostInitAddressTranslation(PortId, LinkId, 0x800000,     0x4810800000, FALSE, 2, 0, 23);
  // PcieHostInitAddressTranslation(PortId, LinkId, 0x90000000,   0x4890000000, TRUE,  3, 0, 31);
  // PcieHostInitAddressTranslation(PortId, LinkId, 0xe0000000,   0x48e0000000, TRUE,  4, 0, 29);
  // PcieHostInitAddressTranslation(PortId, LinkId, 0x4900000000, 0x4900000000, TRUE,  5, 0, 33);
  // PcieHostInitAddressTranslation(PortId, LinkId, 0x4b00000000, 0x4b00000000, TRUE,  6, 0, 32);

  MicroSecondDelay(300);

  DEBUG ((DEBUG_ERROR, "PCIe Port 1, link 0 Init!\n"));

  return EFI_SUCCESS;
}


// { 0x80, 0xff, 0 },
  //  { 0x800000, 0xffffff, 0 }, // IO
  //  { 0xe0000000, 0xefffffff, 0 }, // Mem   < 0x1 0000 0000
  //  { 0x4b00000000, 0x4c00000000, 0 }, // MemAbove4G
  //  { MAX_UINT64, 0, 0 }, // PMem  < 0x1 0000 0000
  //  { MAX_UINT64, 0, 0 }, // PMemAbove4G


//  {
//    0, // Segment
//    0, // Supports
//    0, // Attributes
//    FALSE, // DmaAbove4G
//    FALSE, // NoExtendedConfigSpace
//    FALSE, // ResourceAssigned
//    EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |
//    EFI_PCI_HOST_BRIDGE_MEM64_DECODE, // AllocationAttributes
//    { 0x00, 0x7F, 0 }, // Bus: Base, Limit, Translation
//    { 0x400000, 0x7fffff, 0 }, // IO
//    { 0x4480000000, 0x448fffffff, 0 }, // Mem
//    { 0x4700000000, 0x47ffffffff, 0 }, // MemAbove4G
//    { 0x4460000000, 0x447fffffff, 0 }, // PMem
//    { 0x4500000000, 0x46ffffffff, 0 }, // PMemAbove4G
//    (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath[0] // *DevicePath
//  },