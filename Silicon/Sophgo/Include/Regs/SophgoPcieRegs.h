/** @file PcieCore.h

 Copyright (c) 2023, SOPHGO Inc. All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause

**/

#ifndef _MANGO_PCIE_H_
#define _MANGO_PCIE_H_

#define BIT(nr)              (1 << (nr))
#define GENMASK(end, start)  (((1ULL << ((end) - (start) + 1)) - 1) << (start))

#define PCIE0_CFG_BASE         0x7060000000
#define PCIE1_CFG_BASE         0x7062000000

#define PCIE_CFG_LINK0_APB     0x000000
#define PCIE_CFG_LINK1_APB     0x800000
#define PCIE_CFG_PHY_APB       0x1000000
#define PCIE_CFG_MANGO_APB     0x1800000

typedef enum {
  PCIE_ID_0 = 0x0,
  PCIE_ID_1,
  PCIE_ID_BUTT
} PCIE_PORT;

typedef enum {
  PCIE_LINK_0 = 0x0,
  PCIE_LINK_1,
  PCIE_LINK_BUTT
} PCIE_LINK_ID;

typedef enum {
  PCIE_LINK_NO_RECEIVERS_DETECTED = 0x0,
  PCIE_LINK_LINK_TRAINNING_IN_PROGRESS,
  PCIE_LINK_DL_INIT_IN_PROGRESS,
  PCIE_LINK_DL_INIT_COMPLETED,
  PCIE_LINK_STATUS_BUTT
} PCIE_LINK_STATUS;

typedef struct {
  UINT32 Pcie_addr_l;
  UINT32 Pcie_addr_h;
  UINT32 Dspt0;
  UINT32 Dspt1;
  UINT32 Cpu_addr_l;
  UINT32 Cpu_addr_h;
} PCIE_AXI_REGION;

//
// PCIe Controller Registers
//
//
// Local Management Registers
//
#define PCIE_LM_BASE       0x00100000
//
// Root Port Registers (PCI configuration space for the root port function)
//
#define PCIE_RP_BASE       0x00200000
#define PCIE_RP_CAP_OFFSET 0xc0
//
// Address Translation Registers
//
#define PCIE_AT_BASE 0x00400000
//
// AXI link down register
//
#define PCIE_AT_LINKDOWN (PCIE_AT_BASE + 0x0824)

//
// Region r Outbound AXI to PCIe Address Translation Register 0
//
#define PCIE_AT_OB_REGION_PCI_ADDR0(r) \
        (PCIE_AT_BASE + 0x0000 + ((r) & 0x1f) * 0x0020)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK    GENMASK(5, 0)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK    GENMASK(19, 12)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(devfn) \
         (((devfn) << 12) & PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK      GENMASK(27, 20)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_BUS(bus) \
         (((bus) << 20) & PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK)

//
// Region r Outbound PCIe Descriptor Register 0
//
#define PCIE_AT_OB_REGION_DESC0(r) \
        (PCIE_AT_BASE + 0x0008 + ((r) & 0x1f) * 0x0020)
/* Bit 23 MUST be set in RC mode. */
#define  PCIE_AT_OB_REGION_DESC0_HARDCODED_RID   BIT(23)
#define  PCIE_AT_OB_REGION_DESC0_DEVFN_MASK      GENMASK(31, 24)
#define  PCIE_AT_OB_REGION_DESC0_DEVFN(devfn) \
         (((devfn) << 24) & PCIE_AT_OB_REGION_DESC0_DEVFN_MASK)

#define  PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0 0xa
#define  PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1 0xb


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
#define PCIE_LM_BASE       0x00100000

//
// Vendor ID Register
//
#define PCIE_LM_ID               (PCIE_LM_BASE + 0x0044)
#define  PCIE_LM_ID_VENDOR_MASK  GENMASK(15, 0)
#define  PCIE_LM_ID_VENDOR_SHIFT 0
#define  PCIE_LM_ID_VENDOR(vid) \
         (((vid) << PCIE_LM_ID_VENDOR_SHIFT) & PCIE_LM_ID_VENDOR_MASK)
#define  PCIE_LM_ID_SUBSYS_MASK  GENMASK(31, 16)
#define  PCIE_LM_ID_SUBSYS_SHIFT 16
#define  PCIE_LM_ID_SUBSYS(sub) \
         (((sub) << PCIE_LM_ID_SUBSYS_SHIFT) & PCIE_LM_ID_SUBSYS_MASK)

//
// Root Port Requestor ID Register
//
#define PCIE_LM_RP_RID                              (PCIE_LM_BASE + 0x0228)
#define  PCIE_LM_RP_RID_MASK                        GENMASK(15, 0)
#define  PCIE_LM_RP_RID_SHIFT                       0
#define  PCIE_LM_RP_RID_(rid) \
         (((rid) << PCIE_LM_RP_RID_SHIFT) & PCIE_LM_RP_RID_MASK)

//
// Root Complex BAR Configuration Register
//
#define PCIE_LM_RC_BAR_CFG                          (PCIE_LM_BASE + 0x0300)
#define  PCIE_LM_RC_BAR_CFG_BAR0_APERTURE_MASK      GENMASK(5, 0)
#define  PCIE_LM_RC_BAR_CFG_BAR0_APERTURE(a) \
         (((a) << 0) & PCIE_LM_RC_BAR_CFG_BAR0_APERTURE_MASK)
#define  PCIE_LM_RC_BAR_CFG_BAR0_CTRL_MASK          GENMASK(8, 6)
#define  PCIE_LM_RC_BAR_CFG_BAR0_CTRL(c) \
         (((c) << 6) & PCIE_LM_RC_BAR_CFG_BAR0_CTRL_MASK)
#define  PCIE_LM_RC_BAR_CFG_BAR1_APERTURE_MASK      GENMASK(13, 9)
#define  PCIE_LM_RC_BAR_CFG_BAR1_APERTURE(a) \
         (((a) << 9) & PCIE_LM_RC_BAR_CFG_BAR1_APERTURE_MASK)
#define  PCIE_LM_RC_BAR_CFG_BAR1_CTRL_MASK          GENMASK(16, 14)
#define  PCIE_LM_RC_BAR_CFG_BAR1_CTRL(c) \
         (((c) << 14) & PCIE_LM_RC_BAR_CFG_BAR1_CTRL_MASK)
#define  PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_ENABLE     BIT(17)
#define  PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_32BITS     0
#define  PCIE_LM_RC_BAR_CFG_PREFETCH_MEM_64BITS     BIT(18)
#define  PCIE_LM_RC_BAR_CFG_IO_ENABLE               BIT(19)
#define  PCIE_LM_RC_BAR_CFG_IO_16BITS               0
#define  PCIE_LM_RC_BAR_CFG_IO_32BITS               BIT(20)
#define  PCIE_LM_RC_BAR_CFG_CHECK_ENABLE            BIT(31)

//
// BAR control values applicable to both Endpoint Function and Root Complex
//
#define  PCIE_LM_BAR_CFG_CTRL_DISABLED              0x0
#define  PCIE_LM_BAR_CFG_CTRL_IO_32BITS             0x1
#define  PCIE_LM_BAR_CFG_CTRL_MEM_32BITS            0x4
#define  PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_32BITS   0x5
#define  PCIE_LM_BAR_CFG_CTRL_MEM_64BITS            0x6
#define  PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_64BITS   0x7

#define LM_RC_BAR_CFG_CTRL_DISABLED(bar) 	\
      (PCIE_LM_BAR_CFG_CTRL_DISABLED << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_IO_32BITS(bar) 	\
      (PCIE_LM_BAR_CFG_CTRL_IO_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_MEM_32BITS(bar) 	\
      (PCIE_LM_BAR_CFG_CTRL_MEM_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_PREF_MEM_32BITS(bar) \
      (PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_32BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_MEM_64BITS(bar) 	\
      (PCIE_LM_BAR_CFG_CTRL_MEM_64BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_CTRL_PREF_MEM_64BITS(bar) \
 (PCIE_LM_BAR_CFG_CTRL_PREFETCH_MEM_64BITS << (((bar) * 8) + 6))
#define LM_RC_BAR_CFG_APERTURE(bar, aperture) 	\
      (((aperture) - 2) << ((bar) * 8))

//
// Root Port Registers (PCI configuration space for the root port function)
//
#define PCIE_RP_BASE       0x00200000
#define PCIE_RP_CAP_OFFSET 0xc0

//
// Address Translation Registers
//
#define PCIE_AT_BASE 0x00400000

//
// Region r Outbound AXI to PCIe Address Translation Register 0
//
#define PCIE_AT_OB_REGION_PCI_ADDR0(r) \
        (PCIE_AT_BASE + 0x0000 + ((r) & 0x1f) * 0x0020)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK    GENMASK(5, 0)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & PCIE_AT_OB_REGION_PCI_ADDR0_NBITS_MASK)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK    GENMASK(19, 12)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(devfn) \
         (((devfn) << 12) & PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN_MASK)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK      GENMASK(27, 20)
#define  PCIE_AT_OB_REGION_PCI_ADDR0_BUS(bus) \
         (((bus) << 20) & PCIE_AT_OB_REGION_PCI_ADDR0_BUS_MASK)

//
// Region r Outbound AXI to PCIe Address Translation Register 1
//
#define PCIE_AT_OB_REGION_PCI_ADDR1(r) \
        (PCIE_AT_BASE + 0x0004 + ((r) & 0x1f) * 0x0020)

//
// Region r Outbound PCIe Descriptor Register 0
//
#define PCIE_AT_OB_REGION_DESC0(r) \
        (PCIE_AT_BASE + 0x0008 + ((r) & 0x1f) * 0x0020)
#define  PCIE_AT_OB_REGION_DESC0_TYPE_MASK       GENMASK(3, 0)
#define  PCIE_AT_OB_REGION_DESC0_TYPE_MEM        0x2
#define  PCIE_AT_OB_REGION_DESC0_TYPE_IO         0x6
#define  PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0 0xa
#define  PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1 0xb
#define  PCIE_AT_OB_REGION_DESC0_TYPE_NORMAL_MSG 0xc
#define  PCIE_AT_OB_REGION_DESC0_TYPE_VENDOR_MSG 0xd
/* Bit 23 MUST be set in RC mode. */
#define  PCIE_AT_OB_REGION_DESC0_HARDCODED_RID   BIT(23)
#define  PCIE_AT_OB_REGION_DESC0_DEVFN_MASK      GENMASK(31, 24)
#define  PCIE_AT_OB_REGION_DESC0_DEVFN(devfn) \
         (((devfn) << 24) & PCIE_AT_OB_REGION_DESC0_DEVFN_MASK)

//
// Region r Outbound PCIe Descriptor Register 1
//
#define PCIE_AT_OB_REGION_DESC1(r) \
        (PCIE_AT_BASE + 0x000c + ((r) & 0x1f) * 0x0020)
#define  PCIE_AT_OB_REGION_DESC1_BUS_MASK        GENMASK(7, 0)
#define  PCIE_AT_OB_REGION_DESC1_BUS(bus) \
         ((bus) & PCIE_AT_OB_REGION_DESC1_BUS_MASK)

//
// Region r AXI Region Base Address Register 0
//
#define PCIE_AT_OB_REGION_CPU_ADDR0(r) \
         (PCIE_AT_BASE + 0x0018 + ((r) & 0x1f) * 0x0020)
#define  PCIE_AT_OB_REGION_CPU_ADDR0_NBITS_MASK  GENMASK(5, 0)
#define  PCIE_AT_OB_REGION_CPU_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & PCIE_AT_OB_REGION_CPU_ADDR0_NBITS_MASK)

//
// Region r AXI Region Base Address Register 1
//
#define PCIE_AT_OB_REGION_CPU_ADDR1(r) \
        (PCIE_AT_BASE + 0x001c + ((r) & 0x1f) * 0x0020)

//
// Root Port BAR Inbound PCIe to AXI Address Translation Register
//
#define PCIE_AT_IB_RP_BAR_ADDR0(bar) \
        (PCIE_AT_BASE + 0x0800 + (bar) * 0x0008)
#define  PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK    GENMASK(5, 0)
#define  PCIE_AT_IB_RP_BAR_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK)
#define PCIE_AT_IB_RP_BAR_ADDR1(bar) \
        (PCIE_AT_BASE + 0x0804 + (bar) * 0x0008)

//
// Root Port BAR Inbound PCIe to AXI Address Translation Register
//
#define PCIE_AT_IB_RP_BAR_ADDR0(bar) \
        (PCIE_AT_BASE + 0x0800 + (bar) * 0x0008)
#define  PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK      GENMASK(5, 0)
#define  PCIE_AT_IB_RP_BAR_ADDR0_NBITS(nbits) \
         (((nbits) - 1) & PCIE_AT_IB_RP_BAR_ADDR0_NBITS_MASK)
#define PCIE_AT_IB_RP_BAR_ADDR1(bar) \
        (PCIE_AT_BASE + 0x0804 + (bar) * 0x0008)

//
// AXI link down register
//
#define PCIE_AT_LINKDOWN (PCIE_AT_BASE + 0x0824)

//
// LTSSM Capabilities register
//
#define PCIE_LTSSM_CONTROL_CAP             (PCIE_LM_BASE + 0x0054)
#define  PCIE_DETECT_QUIET_MIN_DELAY_MASK  GENMASK(2, 1)
#define  PCIE_DETECT_QUIET_MIN_DELAY_SHIFT 1
#define  PCIE_DETECT_QUIET_MIN_DELAY(delay) \
         (((delay) << PCIE_DETECT_QUIET_MIN_DELAY_SHIFT) & \
         PCIE_DETECT_QUIET_MIN_DELAY_MASK)

enum pcie_rp_bar {
  RP_BAR_UNDEFINED = -1,
  RP_BAR0,
  RP_BAR1,
  RP_NO_BAR
};

#endif /* _MANGO_PCIE_H_ */