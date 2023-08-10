/** @file

  PCI Segment Library instance for SOPHGO SG2042 with multiple Root Complexes.

  Copyright (c) 2023. SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
// #include <Library/PcdLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/UefiLib.h>
#include <Regs/SophgoPcieRegs.h>
// #include <IndustryStandard/JH7110.h>
// #include <IndustryStandard/Pci30.h>

typedef enum {
  PciCfgWidthUint8 = 0,
  PciCfgWidthUint16,
  PciCfgWidthUint32,
  PciCfgWidthMax
} PCI_CFG_WIDTH;

#define PCIE_RC_CONFIG_ADDR  0x7062000000
#define PCIE_EP_CONFIG_ADDR  0x4900000000

// #define PCIE_RC_CONFIG_ADDR  0x7060000000
// #define PCIE_EP_CONFIG_ADDR  0x4880000000
/**
 
 * 
 */
// #define GET_SEG_NUM(Address)    (((Address) >> 32) & 0xFFFF)
// #define GET_BUS_NUM(Address)    (((Address) >> 20) & 0x7F)
// #define GET_DEV_NUM(Address)    (((Address) >> 15) & 0x1F)
// #define GET_FUNC_NUM(Address)   (((Address) >> 12) & 0x07)
// #define GET_REG_NUM(Address)    ((Address) & 0xFFF)

/**
  Assert the validity of a PCI Segment address.
  A valid PCI Segment address should not contain 1's in bits 28..31 and 48..63

  @param  A The address to validate.
  @param  M Additional bits to assert to be zero.

  PCIe Memory-­‐Mapped Conﬁguration Space Access:
 ---------------------------------------------------------------------------------------------
 |  PCIEXBAR’s Bits  |  Bus(8-bit)  |  Device(5-bit)  |  Function(3-bit)  |  Offset(12-bit)  |
 |       35:28       |    27:20     |      19:15      |       14:12       |       11:0       |
 ---------------------------------------------------------------------------------------------

**/
#define ASSERT_INVALID_PCI_SEGMENT_ADDRESS(A,M) \
  ASSERT (((A) & (0xffff0000f0000000ULL | (M))) == 0)

#define EXTRACT_PCIE_ADDRESS(Address, Segment, Bus, Device, Function, Register) \
{ \
  (Segment)  = (RShiftU64 (Address, 32) & 0xffff);   \
  (Bus)      = (((Address) >> 20) & 0xff);   \
  (Device)   = (((Address) >> 15) & 0x1f);   \
  (Function) = (((Address) >> 12) & 0x07);   \
  (Register) = ((Address)       & 0xfff);  \
}

/**
  Given the nature of how we access PCI devices, we ensure that
  read/write accesses are serialized through the use of a lock.
**/
// STATIC
// EFI_LOCK mPciSegmentReadWriteLock = EFI_INITIALIZE_LOCK_VARIABLE (TPL_HIGH_LEVEL);

//STATIC UINT64 mPciSegmentLastAccess;     /* Avoid repeat CFG_INDEX updates */

#define BIT(nr)              (1 << (nr))
#define GENMASK(end, start)  (((1ULL << ((end) - (start) + 1)) - 1) << (start))
//
// Local Management Registers
//
#define CDNS_PCIE_LM_BASE       0x00100000
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
// AXI link down register
//
#define CDNS_PCIE_AT_LINKDOWN (CDNS_PCIE_AT_BASE + 0x0824)

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
// Region r Outbound PCIe Descriptor Register 0
//
#define CDNS_PCIE_AT_OB_REGION_DESC0(r) \
        (CDNS_PCIE_AT_BASE + 0x0008 + ((r) & 0x1f) * 0x0020)
/* Bit 23 MUST be set in RC mode. */
#define  CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID   BIT(23)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN_MASK      GENMASK(31, 24)
#define  CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(devfn) \
         (((devfn) << 24) & CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN_MASK)

#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0 0xa
#define  CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1 0xb

BOOLEAN PcieIsLinkUp (
  UINTN     ApbBase
  )
{
  UINT32 Value;

  Value = MmioRead32(ApbBase + CDNS_PCIE_LM_BASE);
  if ((Value & 0x1)) {
    return TRUE;
  }
  return FALSE;
}

// STATIC
// VOID
// PciMapBus (
//   IN UINT32       Port,
//   IN UINT32       LinkId,
//   IN UINT32       BusNumber,
//   IN UINT32       DevFn,
//   IN UINT64       Offset
//   )
// {
//   UINT64     ApbBase;
//   UINT32     Addr0;
//   UINT32     Desc0;

//   // ApbBase = PCIE0_CFG_BASE + LinkId * 0x2000000;
//   ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);

//   // Only the root port (DevFn == 0) is connected to this bus.
//   // All other PCI devices are behind some bridge hence on another bus.
//   if (BusNumber == 0x00 ||  BusNumber == 0x40 || BusNumber == 0x80 || BusNumber == 0xc0) {
//     if (DevFn) {
//       return NULL;
//     }

//     return ApbBase + CDNS_PCIE_RP_BASE + (Offset & 0xfff);
//   }

//   if (!(PcieIsLinkUp(ApbBase)))
//     return NULL;

//   // Clear AXI link-down status
//   MmioWrite32 (ApbBase + CDNS_PCIE_AT_LINKDOWN, 0x0);

//   // Update Output registers for AXI region 0.
//   Addr0 = CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(12) |
//           CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(DevFn) |
//           CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS(BusNumber);
//   MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_PCI_ADDR0(0), Addr0);

//   // Configuration Type 0 or Type 1 access.
//   Desc0 = CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID |
//           CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(0);

//   // The bus number was alreadly set once for all in Desc1
//   // by PciHostInitAddressTranslation().
//   // if (BusNumber == BusNumber + 1)
//   //   Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0;
//   // else
//   Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1;

//   MmioWrite32 (ApbBase + CDNS_PCIE_AT_OB_REGION_DESC0(0), Desc0);
//   // return ep_confbase + (Offset & 0xfff);
// }

/**
  Internal worker function to obtain config space base address.

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @return The value read from the PCI configuration register.

**/

// STATIC
// UINT64
// PciSegmentLibGetConfigBase (
//   IN  UINT64                      Address,
//   IN  BOOLEAN                     IsWrite,
//   IN  PCIE_PORT                   Port,
//   IN  PCIE_LINK_ID                LinkId
//   // IN  PCI_CFG_WIDTH               Width
//   )
// {
//   UINT32    Segment;
//   UINT8     Bus;
//   UINT8     Device;
//   UINT8     Function;
//   UINT32    Register;
//   UINT64    Base;
//   UINT64    Offset;

//   Base = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);
//   // Base = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_MANGO_APB;
//   // Base = PCIE_EP_CONFIG_ADDR;
//   Offset = Address & 0xFFF;         /* Pick off the 4k register offset, 0x100 ~ 0xFFF */
//   Address &= 0xFFFF000;            /* Clear the offset leave only the BDF */
//   // 256-Byte: Offset = Address & 0xFF;

//   /* The root port is at the base of the PCIe register space */
//   if (Address != 0) {
//       EXTRACT_PCIE_ADDRESS (Address, Segment, Bus, Device, Function, Register);
//       /*
//        * There can only be a single device on bus 0x80 (downstream of root).
//        * Subsequent busses (behind a PCIe switch) can have more.
//        */
//       DEBUG ((DEBUG_ERROR, "--%a[%u]: 1 - Base=0x%lx, Offset=0x%lx, Address=0x%lx\n", __FUNCTION__, __LINE__, Base, Offset, Address));
//       if (Bus != 0x0 && Bus != 0x40 && Bus != 0x80 && Bus != 0xc0)
//       if (Bus != 0x80 && IsWrite && ((Offset == 0x10) || (Offset == 0x14))) {
//          return 0xFFFFFFFF;
//       }
//       // ignore device > 0 or function > 0 on base bus
//       if (Device != 0 || Function != 0)
//       	return 0xFFFFFFFF;

//       DEBUG ((DEBUG_ERROR, "--%a[%u]: 2 - Base=0x%lx, Offset=0x%lx, Address=0x%lx\n", __FUNCTION__, __LINE__, Base, Offset, Address));
//       return Base + Address + Offset;
//   }
//   DEBUG ((DEBUG_ERROR, "--%a[%u]: 3 - Base=0x%lx, Offset=0x%lx, Address=0x%lx\n", __FUNCTION__, __LINE__, Base, Offset, Address));
//   return Base + Offset;
// }

STATIC
UINT32
CpuMemoryServiceRead (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width
  )
{
  UINT32   Uint32Buffer;

  if (Width == PciCfgWidthUint8) {
    Uint32Buffer = MmioRead32((UINTN)(Address & (~0x3)));
    return BitFieldRead32 (Uint32Buffer, (Address & 0x3) * 8, (Address & 0x3) * 8 + 7);
  } else if (Width == PciCfgWidthUint16) {
    if (((Address & 0x3) == 1) || ((Address & 0x3) == 3)) {
      return 0xffff;
    }
    Uint32Buffer = MmioRead32((UINTN)(Address & (~0x3)));
    return BitFieldRead32 (Uint32Buffer, (Address & 0x3) * 8, (Address & 0x3) * 8 + 15);
  } else if (Width == PciCfgWidthUint32) {
    return MmioRead32 ((UINTN)Address);
  } else {
    return 0xffffffff;
  }
}

STATIC
UINT32
CpuMemoryServiceWrite (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width,
  IN  UINT32                      Data
  )
{
  UINT32                     Uint32Buffer;

  if (Width == PciCfgWidthUint8) {
    Uint32Buffer = MmioRead32((UINTN)(Address & (~0x3)));
    Uint32Buffer = BitFieldWrite32 (Uint32Buffer, (Address & 0x3) * 8, (Address & 0x3) * 8 + 7, Data);
    MmioWrite32 ((UINTN)(Address & (~0x3)), Uint32Buffer);
  } else if (Width == PciCfgWidthUint16) {
    if (((Address & 0x3) == 1) || ((Address & 0x3) == 3)) {
      return 0xffffffff;
    }
    Uint32Buffer = MmioRead32((UINTN)(Address & (~0x3)));
    Uint32Buffer = BitFieldWrite32 (Uint32Buffer, (Address & 0x3) * 8, (Address & 0x3) * 8 + 15, Data);
    MmioWrite32 ((UINTN)(Address & (~0x3)), Uint32Buffer);
  } else if (Width == PciCfgWidthUint32) {
    MmioWrite32 ((UINTN)Address, Data);
  } else {
    return 0xffffffff;
  }
  return Data;
}

STATIC
UINT32
PciGenericConfigRead (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width
  )
{
  if (Width == PciCfgWidthUint8) {
    return MmioRead8 ((UINTN)Address);
  } else if (Width == PciCfgWidthUint16) {
    return MmioRead16 ((UINTN)Address);
  } else if (Width == PciCfgWidthUint32) {
    return MmioRead32 ((UINTN)Address);
  } else {
    return 0xffffffff;
  }
}

STATIC
UINT32
PciGenericConfigWrite (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width,
  IN  UINT32                      Data
  )
{
  if (Width == PciCfgWidthUint8) {
    MmioWrite8 ((UINTN)Address, Data);
  } else if (Width == PciCfgWidthUint16) {
    MmioWrite16 ((UINTN)Address, Data);
  } else if (Width == PciCfgWidthUint32) {
    MmioWrite32 ((UINTN)Address, Data);
  } else {
    return 0xffffffff;
  }
  return Data;
}

/**
  Internal worker function to read a PCI configuration register.

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  Width   The width of data to readif (Bus == 0x00 ||  Bus == 0x40 || Bus == 0x80 || Bus == 0xc0) {

  @return The value read from the PCI configuration register.

**/
STATIC
UINT32
PciSegmentLibReadWorker (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width
  )
{
  UINT32    Segment;
  UINT8     Bus;
  UINT8     Device;
  UINT8     Function;
  UINT32    Register;

  UINT64    MmioAddress;

  // UINT64     ApbBase;
  UINT32     Addr0;
  UINT32     Desc0;

  // ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);

  EXTRACT_PCIE_ADDRESS (Address, Segment, Bus, Device, Function, Register);
  // DEBUG ((DEBUG_WARN, "--%a[%d]: Address=%lx, Segment=%d, Bus=%d, Device=%d, Function=%d, Register=%lx!\n", __FUNCTION__, __LINE__, Address, Segment, Bus, Device, Function, Register));

  if (Bus == 0x00 ||  Bus == 0x40 || Bus == 0x80 || Bus == 0xc0) {
    // ignore device > 0 or function > 0 on base bus
    if (Device != 0 || Function != 0) {
      return 0xffffffff;
    }
    // MmioAddress = PCIE_RC_CONFIG_ADDR + Register;
    MmioAddress = PCIE_RC_CONFIG_ADDR + CDNS_PCIE_RP_BASE + Register;
    // DEBUG ((DEBUG_WARN, "--%a[%d]: RootBus: MmioAddress=%lx, Width=%d\n", __FUNCTION__, __LINE__, MmioAddress, Width));
    return CpuMemoryServiceRead (MmioAddress, Width);
  }

  if (!(PcieIsLinkUp(PCIE_RC_CONFIG_ADDR))) {
    DEBUG ((DEBUG_WARN, "--%a[%d]: Cannot read from device under root port when link is not up!\n", __FUNCTION__, __LINE__));
    return 0xffffffff;
  }

  // Clear AXI link-down status
  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_LINKDOWN, 0x0);

  // Update Output registers for AXI region 0.
  Addr0 = CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(12) |
          CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(Function) |
          CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS(Bus);
  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_OB_REGION_PCI_ADDR0(0), Addr0);

  // Configuration Type 0 or Type 1 access.
  Desc0 = CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID |
          CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(0);

  // The bus number was alreadly set once for all in Desc1
  // by PciHostInitAddressTranslation().
  // if (BusNumber == BusNumber + 1)
    Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0;
  // else
  // Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1;

  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_OB_REGION_DESC0(0), Desc0);

  // DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  // DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Desc0));

  MmioAddress = PCIE_EP_CONFIG_ADDR + Register;
  // DEBUG ((DEBUG_WARN, "--%a[%d]: EP: MmioAddress=%lx, Width=%d\n", __FUNCTION__, __LINE__, MmioAddress, Width));
  // }
  // DEBUG ((DEBUG_WARN, "--%a[%d]: PciGenericConfigRead (MmioAddress, Width)=%lx!\n", __FUNCTION__, __LINE__, PciGenericConfigRead (MmioAddress, Width)));
  return PciGenericConfigRead (MmioAddress, Width);
}

/**
  Internal worker function to writes a PCI configuration register.

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.
  @param  Width   The width of data to write
  @param  Data    The value to write.

  @return The value written to the PCI configuration register.

**/
STATIC
UINT32
PciSegmentLibWriteWorker (
  IN  UINT64                      Address,
  IN  PCI_CFG_WIDTH               Width,
  IN  UINT32                      Data
  )
{
  UINT32    Segment;
  UINT8     Bus;
  UINT8     Device;
  UINT8     Function;
  UINT32    Register;

  UINT64    MmioAddress;
  // UINT64    ApbBase;
  UINT32    Addr0;
  UINT32    Desc0;

  // ApbBase = PCIE0_CFG_BASE + (Port * 0x02000000) + PCIE_CFG_LINK0_APB + (LinkId * 0x800000);

  EXTRACT_PCIE_ADDRESS (Address, Segment, Bus, Device, Function, Register);
  // DEBUG ((DEBUG_WARN, "--%a[%d]: Address=%lx, Segment=%d, Bus=%d, Device=%d, Function=%d, Register=%lx!\n", __FUNCTION__, __LINE__, Address, Segment, Bus, Device, Function, Register));

  if (Bus == 0x00 ||  Bus == 0x40 || Bus == 0x80 || Bus == 0xc0) {
    // ignore device > 0 or function > 0 on base bus
    if (Device != 0 || Function != 0) {
      DEBUG ((DEBUG_WARN, "----error!!!enter DEVICE!=0 FUNCTION!=0 branch: Data=0x%lx\n\n", Data));
      // return 0xffffffff;
      return Data;
    }
    /*
     * Ignore writing to root port BAR registers, in case we get wrong BAR length.
     * There can only be a single device on bus 1 (downstream of root).
     * Subsequent busses (behind a PCIe switch) can have more.
     */
    if ((Register & ~0x3) == 0x10 || (Register & ~0x3) == 0x14) {
      DEBUG ((DEBUG_WARN, "----error!!!enter 0x10 0x14 branch: Data=0x%lx\n\n", Data));
      return Data;
    }
    // MmioAddress = PCIE_RC_CONFIG_ADDR + Register;
    MmioAddress = PCIE_RC_CONFIG_ADDR + CDNS_PCIE_RP_BASE + Register;
    // DEBUG ((DEBUG_WARN, "--%a[%d]: RootBus: MmioAddress=%lx, Width=%d, Data=%lx!\n", __FUNCTION__, __LINE__, MmioAddress, Width, Data));
    return CpuMemoryServiceWrite (MmioAddress, Width, Data);
  }

  if (!(PcieIsLinkUp(PCIE_RC_CONFIG_ADDR))) {
    DEBUG ((DEBUG_WARN, "--%a[%d]: Cannot read from device under root port when link is not up!\n", __FUNCTION__, __LINE__));
    return 0xffffffff;
  }

  // Clear AXI link-down status
  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_LINKDOWN, 0x0);

  // Update Output registers for AXI region 0.
  Addr0 = CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_NBITS(12) |
          CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_DEVFN(Function) |
          CDNS_PCIE_AT_OB_REGION_PCI_ADDR0_BUS(Bus);
  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_OB_REGION_PCI_ADDR0(0), Addr0);

  // Configuration Type 0 or Type 1 access.
  Desc0 = CDNS_PCIE_AT_OB_REGION_DESC0_HARDCODED_RID |
          CDNS_PCIE_AT_OB_REGION_DESC0_DEVFN(0);

  // The bus number was alreadly set once for all in Desc1
  // by PciHostInitAddressTranslation().
  // if (BusNumber == BusNumber + 1)
    Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE0;
  // else
  // Desc0 |= CDNS_PCIE_AT_OB_REGION_DESC0_TYPE_CONF_TYPE1;

  MmioWrite32 (PCIE_RC_CONFIG_ADDR + CDNS_PCIE_AT_OB_REGION_DESC0(0), Desc0);

  // DEBUG ((DEBUG_INIT, "%a[%u]----PCIe Address[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Addr0));
  // DEBUG ((DEBUG_INIT, "%a[%u]----Descriptor[31:0]=0x%lx---\n", __FUNCTION__, __LINE__, Desc0));

  MmioAddress = PCIE_EP_CONFIG_ADDR + Register;
  // DEBUG ((DEBUG_WARN, "--%a[%d]: EP: PciGenericConfigWrite (MmioAddress, Width, Data)=%lx!\n", __FUNCTION__, __LINE__, PciGenericConfigWrite (MmioAddress, Width, Data)));
  return PciGenericConfigWrite (MmioAddress, Width, Data);
}

/**
  Register a PCI device so PCI configuration registers may be accessed after
  SetVirtualAddressMap().

  If any reserved bits in Address are set, then ASSERT().

  @param  Address The address that encodes the PCI Bus, Device, Function and
                  Register.

  @retval RETURN_SUCCESS           The PCI device was registered for runtime access.
  @retval RETURN_UNSUPPORTED       An attempt was made to call this function
                                   after ExitBootServices().
  @retval RETURN_UNSUPPORTED       The resources required to access the PCI device
                                   at runtime could not be mapped.
  @retval RETURN_OUT_OF_RESOURCES  There are not enough resources available to
                                   complete the registration.

**/
RETURN_STATUS
EFIAPI
PciSegmentRegisterForRuntimeAccess (
  IN UINTN  Address
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 0);
  return RETURN_UNSUPPORTED;
}

/**
  Reads an 8-bit PCI configuration register.

  Reads and returns the 8-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function,
                    and Register.

  @return The 8-bit PCI configuration register specified by Address.

**/
UINT8
EFIAPI
PciSegmentRead8 (
  IN UINT64                    Address
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 0);

  return (UINT8) PciSegmentLibReadWorker (Address, PciCfgWidthUint8);
}

/**
  Writes an 8-bit PCI configuration register.

  Writes the 8-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address     The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  Value       The value to write.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentWrite8 (
  IN UINT64                    Address,
  IN UINT8                     Value
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 0);

  return (UINT8) PciSegmentLibWriteWorker (Address, PciCfgWidthUint8, Value);
}

/**
  Performs a bitwise OR of an 8-bit PCI configuration register with an 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentOr8 (
  IN UINT64                    Address,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) (PciSegmentRead8 (Address) | OrData));
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.
  If any reserved bits in Address are set, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentAnd8 (
  IN UINT64                    Address,
  IN UINT8                     AndData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) (PciSegmentRead8 (Address) & AndData));
}

/**
  Performs a bitwise AND of an 8-bit PCI configuration register with an 8-bit value,
  followed a  bitwise OR with another 8-bit value.

  Reads the 8-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 8-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData    The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentAndThenOr8 (
  IN UINT64                    Address,
  IN UINT8                     AndData,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (Address, (UINT8) ((PciSegmentRead8 (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in an 8-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldRead8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead8 (PciSegmentRead8 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  8-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldWrite8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     Value
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldWrite8 (PciSegmentRead8 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 8-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldOr8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldOr8 (PciSegmentRead8 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in an 8-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 8-bit register.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND between the read result and the value specified by AndData, and
  writes the result to the 8-bit PCI configuration register specified by
  Address. The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are
  serialized. Extra left bits in AndData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldAnd8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     AndData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldAnd8 (PciSegmentRead8 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in an 8-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  8-bit port.

  Reads the 8-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 8-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..7.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..7.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT8
EFIAPI
PciSegmentBitFieldAndThenOr8 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT8                     AndData,
  IN UINT8                     OrData
  )
{
  return PciSegmentWrite8 (
           Address,
           BitFieldAndThenOr8 (PciSegmentRead8 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 16-bit PCI configuration register.

  Reads and returns the 16-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.

  @return The 16-bit PCI configuration register specified by Address.

**/
UINT16
EFIAPI
PciSegmentRead16 (
  IN UINT64                    Address
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 1);

  return (UINT16) PciSegmentLibReadWorker (Address, PciCfgWidthUint16);
}

/**
  Writes a 16-bit PCI configuration register.

  Writes the 16-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address     The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  Value       The value to write.

  @return The parameter of Value.

**/
UINT16
EFIAPI
PciSegmentWrite16 (
  IN UINT64                    Address,
  IN UINT16                    Value
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 1);

  return (UINT16) PciSegmentLibWriteWorker (Address, PciCfgWidthUint16, Value);
}

/**
  Performs a bitwise OR of a 16-bit PCI configuration register with
  a 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 16-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address The address that encodes the PCI Segment, Bus, Device, Function and
                  Register.
  @param  OrData  The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentOr16 (
  IN UINT64                    Address,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) (PciSegmentRead16 (Address) | OrData));
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentAnd16 (
  IN UINT64                    Address,
  IN UINT16                    AndData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) (PciSegmentRead16 (Address) & AndData));
}

/**
  Performs a bitwise AND of a 16-bit PCI configuration register with a 16-bit value,
  followed a  bitwise OR with another 16-bit value.

  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentAndThenOr16 (
  IN UINT64                    Address,
  IN UINT16                    AndData,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (Address, (UINT16) ((PciSegmentRead16 (Address) & AndData) | OrData));
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 16-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldRead16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead16 (PciSegmentRead16 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  16-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldWrite16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    Value
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldWrite16 (PciSegmentRead16 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 16-bit PCI configuration register specified by Address.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldOr16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldOr16 (PciSegmentRead16 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 16-bit PCI configuration, performs a bitwise OR,
  and writes the result back to the bit field in the 16-bit port.

  Reads the 16-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 16-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.
  Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().
  If StartBit is greater than 7, then ASSERT().
  If EndBit is greater than 7, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    The ordinal of the least significant bit in a byte is bit 0.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    The ordinal of the most significant bit in a byte is bit 7.
  @param  AndData   The value to AND with the read value from the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldAnd16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    AndData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldAnd16 (PciSegmentRead16 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 16-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  16-bit port.

  Reads the 16-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 16-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 15, then ASSERT().
  If EndBit is greater than 15, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..15.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..15.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT16
EFIAPI
PciSegmentBitFieldAndThenOr16 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT16                    AndData,
  IN UINT16                    OrData
  )
{
  return PciSegmentWrite16 (
           Address,
           BitFieldAndThenOr16 (PciSegmentRead16 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a 32-bit PCI configuration register.

  Reads and returns the 32-bit PCI configuration register specified by Address.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function,
                    and Register.

  @return The 32-bit PCI configuration register specified by Address.

**/
UINT32
EFIAPI
PciSegmentRead32 (
  IN UINT64                    Address
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 3);

  return PciSegmentLibReadWorker (Address, PciCfgWidthUint32);
}

/**
  Writes a 32-bit PCI configuration register.

  Writes the 32-bit PCI configuration register specified by Address with the value specified by Value.
  Value is returned.  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address     The address that encodes the PCI Segment, Bus, Device,
                      Function, and Register.
  @param  Value       The value to write.

  @return The parameter of Value.

**/
UINT32
EFIAPI
PciSegmentWrite32 (
  IN UINT64                    Address,
  IN UINT32                    Value
  )
{
  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (Address, 3);

  return PciSegmentLibWriteWorker (Address, PciCfgWidthUint32, Value);
}

/**
  Performs a bitwise OR of a 32-bit PCI configuration register with a 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise OR between the read result and the value specified by OrData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function, and Register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentOr32 (
  IN UINT64                    Address,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (Address, PciSegmentRead32 (Address) | OrData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function,
                    and Register.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentAnd32 (
  IN UINT64                    Address,
  IN UINT32                    AndData
  )
{
  return PciSegmentWrite32 (Address, PciSegmentRead32 (Address) & AndData);
}

/**
  Performs a bitwise AND of a 32-bit PCI configuration register with a 32-bit value,
  followed a  bitwise OR with another 32-bit value.

  Reads the 32-bit PCI configuration register specified by Address,
  performs a bitwise AND between the read result and the value specified by AndData,
  performs a bitwise OR between the result of the AND operation and the value specified by OrData,
  and writes the result to the 32-bit PCI configuration register specified by Address.
  The value written to the PCI configuration register is returned.
  This function must guarantee that all PCI read and write operations are serialized.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param  Address   The address that encodes the PCI Segment, Bus, Device, Function,
                    and Register.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentAndThenOr32 (
  IN UINT64                    Address,
  IN UINT32                    AndData,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (Address, (PciSegmentRead32 (Address) & AndData) | OrData);
}

/**
  Reads a bit field of a PCI configuration register.

  Reads the bit field in a 32-bit PCI configuration register. The bit field is
  specified by the StartBit and the EndBit. The value of the bit field is
  returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().

  @param  Address   The PCI configuration register to read.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.

  @return The value of the bit field read from the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldRead32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit
  )
{
  return BitFieldRead32 (PciSegmentRead32 (Address), StartBit, EndBit);
}

/**
  Writes a bit field to a PCI configuration register.

  Writes Value to the bit field of the PCI configuration register. The bit
  field is specified by the StartBit and the EndBit. All other bits in the
  destination PCI configuration register are preserved. The new value of the
  32-bit register is returned.

  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If Value is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  Value     The new value of the bit field.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldWrite32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    Value
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldWrite32 (PciSegmentRead32 (Address), StartBit, EndBit, Value)
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration, performs a bitwise OR, and
  writes the result back to the bit field in the 32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise OR between the read result and the value specified by
  OrData, and writes the result to the 32-bit PCI configuration register
  specified by Address. The value written to the PCI configuration register is
  returned. This function must guarantee that all PCI read and write operations
  are serialized. Extra left bits in OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  OrData    The value to OR with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldOr32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldOr32 (PciSegmentRead32 (Address), StartBit, EndBit, OrData)
           );
}

/**
  Reads a bit field in a 32-bit PCI configuration register, performs a bitwise
  AND, and writes the result back to the bit field in the 32-bit register.


  Reads the 32-bit PCI configuration register specified by Address, performs a bitwise
  AND between the read result and the value specified by AndData, and writes the result
  to the 32-bit PCI configuration register specified by Address. The value written to
  the PCI configuration register is returned.  This function must guarantee that all PCI
  read and write operations are serialized.  Extra left bits in AndData are stripped.
  If any reserved bits in Address are set, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldAnd32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    AndData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldAnd32 (PciSegmentRead32 (Address), StartBit, EndBit, AndData)
           );
}

/**
  Reads a bit field in a 32-bit port, performs a bitwise AND followed by a
  bitwise OR, and writes the result back to the bit field in the
  32-bit port.

  Reads the 32-bit PCI configuration register specified by Address, performs a
  bitwise AND followed by a bitwise OR between the read result and
  the value specified by AndData, and writes the result to the 32-bit PCI
  configuration register specified by Address. The value written to the PCI
  configuration register is returned. This function must guarantee that all PCI
  read and write operations are serialized. Extra left bits in both AndData and
  OrData are stripped.

  If any reserved bits in Address are set, then ASSERT().
  If StartBit is greater than 31, then ASSERT().
  If EndBit is greater than 31, then ASSERT().
  If EndBit is less than StartBit, then ASSERT().
  If AndData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().
  If OrData is larger than the bitmask value range specified by StartBit and EndBit, then ASSERT().

  @param  Address   The PCI configuration register to write.
  @param  StartBit  The ordinal of the least significant bit in the bit field.
                    Range 0..31.
  @param  EndBit    The ordinal of the most significant bit in the bit field.
                    Range 0..31.
  @param  AndData   The value to AND with the PCI configuration register.
  @param  OrData    The value to OR with the result of the AND operation.

  @return The value written back to the PCI configuration register.

**/
UINT32
EFIAPI
PciSegmentBitFieldAndThenOr32 (
  IN UINT64                    Address,
  IN UINTN                     StartBit,
  IN UINTN                     EndBit,
  IN UINT32                    AndData,
  IN UINT32                    OrData
  )
{
  return PciSegmentWrite32 (
           Address,
           BitFieldAndThenOr32 (PciSegmentRead32 (Address), StartBit, EndBit, AndData, OrData)
           );
}

/**
  Reads a range of PCI configuration registers into a caller supplied buffer.

  Reads the range of PCI configuration registers specified by StartAddress and
  Size into the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be read. Size is
  returned. When possible 32-bit PCI configuration read cycles are used to read
  from StartAdress to StartAddress + Size. Due to alignment restrictions, 8-bit
  and 16-bit PCI configuration read cycles may be used at the beginning and the
  end of the range.

  If any reserved bits in StartAddress are set, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  The starting address that encodes the PCI Segment, Bus,
                        Device, Function and Register.
  @param  Size          The size in bytes of the transfer.
  @param  Buffer        The pointer to a buffer receiving the data read.

  @return Size

**/
UINTN
EFIAPI
PciSegmentReadBuffer (
  IN  UINT64                   StartAddress,
  IN  UINTN                    Size,
  OUT VOID                     *Buffer
  )
{
  UINTN                             ReturnValue;

  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (StartAddress, 0);
  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return Size;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & BIT0) != 0) {
    //
    // Read a byte if StartAddress is byte aligned
    //
    *(volatile UINT8 *)Buffer = PciSegmentRead8 (StartAddress);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & BIT1) != 0) {
    //
    // Read a word if StartAddress is word aligned
    //
    WriteUnaligned16 (Buffer, PciSegmentRead16 (StartAddress));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Read as many double words as possible
    //
    WriteUnaligned32 (Buffer, PciSegmentRead32 (StartAddress));
    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Read the last remaining word if exist
    //
    WriteUnaligned16 (Buffer, PciSegmentRead16 (StartAddress));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Read the last remaining byte if exist
    //
    *(volatile UINT8 *)Buffer = PciSegmentRead8 (StartAddress);
  }

  return ReturnValue;
}


/**
  Copies the data in a caller supplied buffer to a specified range of PCI
  configuration space.

  Writes the range of PCI configuration registers specified by StartAddress and
  Size from the buffer specified by Buffer. This function only allows the PCI
  configuration registers from a single PCI function to be written. Size is
  returned. When possible 32-bit PCI configuration write cycles are used to
  write from StartAdress to StartAddress + Size. Due to alignment restrictions,
  8-bit and 16-bit PCI configuration write cycles may be used at the beginning
  and the end of the range.

  If any reserved bits in StartAddress are set, then ASSERT().
  If ((StartAddress & 0xFFF) + Size) > 0x1000, then ASSERT().
  If Size > 0 and Buffer is NULL, then ASSERT().

  @param  StartAddress  The starting address that encodes the PCI Segment, Bus,
                        Device, Function and Register.
  @param  Size          The size in bytes of the transfer.
  @param  Buffer        The pointer to a buffer containing the data to write.

  @return The parameter of Size.

**/
UINTN
EFIAPI
PciSegmentWriteBuffer (
  IN UINT64                    StartAddress,
  IN UINTN                     Size,
  IN VOID                      *Buffer
  )
{
  UINTN                             ReturnValue;

  ASSERT_INVALID_PCI_SEGMENT_ADDRESS (StartAddress, 0);
  ASSERT (((StartAddress & 0xFFF) + Size) <= 0x1000);

  if (Size == 0) {
    return 0;
  }

  ASSERT (Buffer != NULL);

  //
  // Save Size for return
  //
  ReturnValue = Size;

  if ((StartAddress & BIT0) != 0) {
    //
    // Write a byte if StartAddress is byte aligned
    //
    PciSegmentWrite8 (StartAddress, *(UINT8*)Buffer);
    StartAddress += sizeof (UINT8);
    Size -= sizeof (UINT8);
    Buffer = (UINT8*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16) && (StartAddress & BIT1) != 0) {
    //
    // Write a word if StartAddress is word aligned
    //
    PciSegmentWrite16 (StartAddress, ReadUnaligned16 (Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  while (Size >= sizeof (UINT32)) {
    //
    // Write as many double words as possible
    //
    PciSegmentWrite32 (StartAddress, ReadUnaligned32 (Buffer));
    StartAddress += sizeof (UINT32);
    Size -= sizeof (UINT32);
    Buffer = (UINT32*)Buffer + 1;
  }

  if (Size >= sizeof (UINT16)) {
    //
    // Write the last remaining word if exist
    //
    PciSegmentWrite16 (StartAddress, ReadUnaligned16 (Buffer));
    StartAddress += sizeof (UINT16);
    Size -= sizeof (UINT16);
    Buffer = (UINT16*)Buffer + 1;
  }

  if (Size >= sizeof (UINT8)) {
    //
    // Write the last remaining byte if exist
    //
    PciSegmentWrite8 (StartAddress, *(UINT8*)Buffer);
  }

  return ReturnValue;
}