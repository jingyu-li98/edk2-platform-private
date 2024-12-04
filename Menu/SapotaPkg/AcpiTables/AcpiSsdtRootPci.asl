/**
Phytium PCIe SSDT table for S5000C.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <AcpiPlatform.h>

#define LNK_DEVICE(Unique_Id, Link_Name, irq)         \
  Device(Link_Name) {                                 \
      Name(_HID, EISAID("PNP0C0F"))                   \
      Name(_UID, Unique_Id)                           \
      Name(_PRS, ResourceTemplate() {                 \
        Interrupt(ResourceProducer, Level, ActiveHigh, Exclusive) { irq }   \
      })                                              \
      Method (_CRS, 0) { Return (_PRS) }              \
      Method (_SRS, 1) { }                            \
      Method (_DIS) { }                              \
  }

#define PRT_ENTRY(Address, Pin, Link)                  \
        Package (4) {                                                                             \
            Address,    /* uses the same format as _ADR */                                        \
            Pin,        /* The PCI pin number of the device (0-INTA, 1-INTB, 2-INTC, 3-INTD). */  \
            Link,       /* Interrupt allocated via Link device. */                             \
            Zero        /* global system interrupt number (no used) */          \
          }

#define ROOT_PRT_ENTRY(Dev, Pin, Link)   PRT_ENTRY(Dev * 0x10000 + 0xFFFF, Pin, Link)

#define PCI_OSC_SUPPORT() \
  Name (SUPP, Zero) /* PCI _OSC Support Field value */ \
  Name (CTRL, Zero) /* PCI _OSC Control Field value */ \
  Method (_OSC,4) { \
    If (LEqual (Arg0, ToUUID ("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) { \
      /* Create DWord-adressable fields from the Capabilities Buffer */ \
      CreateDWordField (Arg3, 0, CDW1) \
      CreateDWordField (Arg3, 4, CDW2) \
      CreateDWordField (Arg3, 8, CDW3) \
      /* Save Capabilities DWord2 & 3 */ \
      Store (CDW2, SUPP) \
      Store (CDW3, CTRL) \
      /* Only allow native hot plug control if OS supports: */ \
      /* ASPM */ \
      /* Clock PM */ \
      /* MSI/MSI-X */ \
      If (LNotEqual (And (SUPP, 0x16), 0x16)) { \
        And (CTRL,0x1E,CTRL) \
      }\
      \
      /* allow native Hotplug, native PME, AER */ \
      /* Never allow SHPC (no SHPC controller in this system)*/ \
      And (CTRL,0x1D, CTRL) \
      If (LNotEqual (Arg1, One)) { /* Unknown revision */ \
        Or (CDW1,0x08, CDW1) \
      } \
      \
      If (LNotEqual (CDW3, CTRL)) { /* Capabilities bits were masked */ \
        Or (CDW1, 0x10, CDW1) \
      } \
      \
      /* Update DWORD3 in the buffer */ \
      Store (CTRL, CDW3) \
      Return (Arg3) \
    } Else { \
      Or (CDW1, 4, CDW1) /* Unrecognized UUID */ \
      Return (Arg3) \
    } \
  } // End _OSC

DefinitionBlock("Ssdt.aml", "SSDT", 2, "PHYLTD", "PHYTSSDT", EFI_ACPI_PHYTIUM_OEM_REVISION) {
  Scope(_SB) {
    External (\_SB.DBGC, MethodObj)
    /*
     * PCI Root Complex
     */
    LNK_DEVICE(1, LNKA, 86)
    LNK_DEVICE(2, LNKB, 85)
    LNK_DEVICE(3, LNKC, 84)
    LNK_DEVICE(4, LNKD, 83)

    LNK_DEVICE(5, LNKE, 182)
    LNK_DEVICE(6, LNKF, 181)
    LNK_DEVICE(7, LNKG, 180)
    LNK_DEVICE(8, LNKH, 179)

    LNK_DEVICE(9, LNKI, 278)
    LNK_DEVICE(10, LNKJ, 277)
    LNK_DEVICE(11, LNKK, 276)
    LNK_DEVICE(12, LNKL, 275)

    LNK_DEVICE(13, LNKM, 374)
    LNK_DEVICE(14, LNKN, 373)
    LNK_DEVICE(15, LNKO, 372)
    LNK_DEVICE(16, LNKP, 371)

    LNK_DEVICE(17, LNKQ, 470)
    LNK_DEVICE(18, LNKR, 469)
    LNK_DEVICE(19, LNKS, 468)
    LNK_DEVICE(20, LNKT, 467)

    LNK_DEVICE(21, LNKU, 566)
    LNK_DEVICE(22, LNKV, 565)
    LNK_DEVICE(23, LNKW, 564)
    LNK_DEVICE(24, LNKX, 563)

    LNK_DEVICE(25, LNLA, 662)
    LNK_DEVICE(26, LNLB, 661)
    LNK_DEVICE(27, LNLC, 660)
    LNK_DEVICE(28, LNLD, 659)

    LNK_DEVICE(29, LNLE, 758)
    LNK_DEVICE(30, LNLF, 757)
    LNK_DEVICE(31, LNLG, 756)
    LNK_DEVICE(32, LNLH, 755)

    // reserve ECAM memory range
    Device (RES0)
    {
      Name (_HID, EISAID ("PNP0C02"))
      Name (_UID, 0)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,        // Granularity
           0x40000000, // Range Minimum
           0x4FFFFFFF, // Range Maximum
           0,          // Translation Offset
           0x10000000, // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device (RES1)
    {
      Name (_HID, EISAID("PNP0C02"))
      Name (_UID, 1)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,            // Granularity
           0x100040000000, // Range Minimum
           0x10004FFFFFFF, // Range Maximum
           0x0,            // Translation Offset
           0x10000000,     // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device (RES2)
    {
      Name(_HID, EISAID("PNP0C02"))
      Name(_UID, 2)
      Name(_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,            // Granularity
           0x200040000000, // Range Minimum
           0x20004FFFFFFF, // Range Maximum
           0x0,            // Translation Offset
           0x10000000,     // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device(RES3)
    {
      Name (_HID, EISAID ("PNP0C02"))
      Name (_UID, 3)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,             // Granularity
           0x300040000000,  // Range Minimum
           0x30004FFFFFFF,  // Range Maximum
           0x0,             // Translation Offset
           0x10000000,      // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device(RES4)
    {
      Name(_HID, EISAID("PNP0C02"))
      Name(_UID, 4)
      Name(_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,             // Granularity
           0x400040000000,  // Range Minimum
           0x40004FFFFFFF,  // Range Maximum
           0x0,             // Translation Offset
           0x10000000,      // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device (RES5)
    {
      Name (_HID, EISAID("PNP0C02"))
      Name (_UID, 5)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,             // Granularity
           0x500040000000,  // Range Minimum
           0x50004FFFFFFF,  // Range Maximum
           0x0,             // Translation Offset
           0x10000000,      // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device(RES6)
    {
      Name (_HID, EISAID ("PNP0C02"))
      Name (_UID, 6)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,             // Granularity
           0x600040000000,  // Range Minimum
           0x60004FFFFFFF,  // Range Maximum
           0x0,             // Translation Offset
           0x10000000,      // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    // reserve ECAM memory range
    Device(RES7)
    {
      Name (_HID, EISAID ("PNP0C02"))
      Name (_UID, 7)
      Name (_CRS, ResourceTemplate() {

      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
           0x0,            // Granularity
           0x700040000000, // Range Minimum
           0x70004FFFFFFF, // Range Maximum
           0x0,            // Translation Offset
           0x10000000,     // Length
           ,,)
      })

      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
    }
    Device(PCI0) {
      Name(_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name(_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name(_SEG, 0)                   /* PCI Segment Group number */
      Name(_BBN, 0x0)                 /* PCI Base Bus Number */
      Name(_CCA, 1)                   /* Cache Coherency Attribute */
      Name(_PXM, 0)
      /* PCI Routing Table */
      Name(_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKA),
        ROOT_PRT_ENTRY(0, 1, LNKB),
        ROOT_PRT_ENTRY(0, 2, LNKC),
        ROOT_PRT_ENTRY(0, 3, LNKD),

        ROOT_PRT_ENTRY(1, 0, LNKA),
        ROOT_PRT_ENTRY(1, 1, LNKB),
        ROOT_PRT_ENTRY(1, 2, LNKC),
        ROOT_PRT_ENTRY(1, 3, LNKD),

        ROOT_PRT_ENTRY(2, 0, LNKA),
        ROOT_PRT_ENTRY(2, 1, LNKB),
        ROOT_PRT_ENTRY(2, 2, LNKC),
        ROOT_PRT_ENTRY(2, 3, LNKD),

        ROOT_PRT_ENTRY(3, 0, LNKA),
        ROOT_PRT_ENTRY(3, 1, LNKB),
        ROOT_PRT_ENTRY(3, 2, LNKC),
        ROOT_PRT_ENTRY(3, 3, LNKD),

        ROOT_PRT_ENTRY(4, 0, LNKA),
        ROOT_PRT_ENTRY(4, 1, LNKB),
        ROOT_PRT_ENTRY(4, 2, LNKC),
        ROOT_PRT_ENTRY(4, 3, LNKD),

        ROOT_PRT_ENTRY(5, 0, LNKA),
        ROOT_PRT_ENTRY(5, 1, LNKB),
        ROOT_PRT_ENTRY(5, 2, LNKC),
        ROOT_PRT_ENTRY(5, 3, LNKD),

        ROOT_PRT_ENTRY(6, 0, LNKA),
        ROOT_PRT_ENTRY(6, 1, LNKB),
        ROOT_PRT_ENTRY(6, 2, LNKC),
        ROOT_PRT_ENTRY(6, 3, LNKD),

        ROOT_PRT_ENTRY(7, 0, LNKA),
        ROOT_PRT_ENTRY(7, 1, LNKB),
        ROOT_PRT_ENTRY(7, 2, LNKC),
        ROOT_PRT_ENTRY(7, 3, LNKD),

        ROOT_PRT_ENTRY(8, 0, LNKA),
        ROOT_PRT_ENTRY(8, 1, LNKB),
        ROOT_PRT_ENTRY(8, 2, LNKC),
        ROOT_PRT_ENTRY(8, 3, LNKD),

        ROOT_PRT_ENTRY(9, 0, LNKA),
        ROOT_PRT_ENTRY(9, 1, LNKB),
        ROOT_PRT_ENTRY(9, 2, LNKC),
        ROOT_PRT_ENTRY(9, 3, LNKD),

        ROOT_PRT_ENTRY(10, 0, LNKA),
        ROOT_PRT_ENTRY(10, 1, LNKB),
        ROOT_PRT_ENTRY(10, 2, LNKC),
        ROOT_PRT_ENTRY(10, 3, LNKD),

        ROOT_PRT_ENTRY(11, 0, LNKA),
        ROOT_PRT_ENTRY(11, 1, LNKB),
        ROOT_PRT_ENTRY(11, 2, LNKC),
        ROOT_PRT_ENTRY(11, 3, LNKD),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x00000000,                   /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x00000000000,                /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                      /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                 /* Granularity */
            0x00000000,                 /* Min Base Address */
            0x0000ffff,                 /* Max Base Address */
            0x50000000,                 /* Translate */
            0x00010000,                 /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI0 */

    Device(PCI1) {
      Name (_HID, EISAID ("PNP0A08"))   /* PCI Express Root Bridge */
      Name (_CID, EISAID ("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name (_SEG, 1)                   /* PCI Segment Group number */
      Name (_BBN, 0x0)                 /* PCI Base Bus Number */
      Name (_CCA, 1)                   /* Cache Coherency Attribute */
      Name (_PXM, 1)
      /* PCI Routing Table */
      Name(_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKE),
        ROOT_PRT_ENTRY(0, 1, LNKF),
        ROOT_PRT_ENTRY(0, 2, LNKG),
        ROOT_PRT_ENTRY(0, 3, LNKH),


        ROOT_PRT_ENTRY(1, 0, LNKE),
        ROOT_PRT_ENTRY(1, 1, LNKF),
        ROOT_PRT_ENTRY(1, 2, LNKG),
        ROOT_PRT_ENTRY(1, 3, LNKH),

        ROOT_PRT_ENTRY(2, 0, LNKE),
        ROOT_PRT_ENTRY(2, 1, LNKF),
        ROOT_PRT_ENTRY(2, 2, LNKG),
        ROOT_PRT_ENTRY(2, 3, LNKH),

        ROOT_PRT_ENTRY(3, 0, LNKE),
        ROOT_PRT_ENTRY(3, 1, LNKF),
        ROOT_PRT_ENTRY(3, 2, LNKG),
        ROOT_PRT_ENTRY(3, 3, LNKH),

        ROOT_PRT_ENTRY(4, 0, LNKE),
        ROOT_PRT_ENTRY(4, 1, LNKF),
        ROOT_PRT_ENTRY(4, 2, LNKG),
        ROOT_PRT_ENTRY(4, 3, LNKH),

        ROOT_PRT_ENTRY(5, 0, LNKE),
        ROOT_PRT_ENTRY(5, 1, LNKF),
        ROOT_PRT_ENTRY(5, 2, LNKG),
        ROOT_PRT_ENTRY(5, 3, LNKH),

        ROOT_PRT_ENTRY(6, 0, LNKE),
        ROOT_PRT_ENTRY(6, 1, LNKF),
        ROOT_PRT_ENTRY(6, 2, LNKG),
        ROOT_PRT_ENTRY(6, 3, LNKH),

        ROOT_PRT_ENTRY(7, 0, LNKE),
        ROOT_PRT_ENTRY(7, 1, LNKF),
        ROOT_PRT_ENTRY(7, 2, LNKG),
        ROOT_PRT_ENTRY(7, 3, LNKH),

        ROOT_PRT_ENTRY(8, 0, LNKE),
        ROOT_PRT_ENTRY(8, 1, LNKF),
        ROOT_PRT_ENTRY(8, 2, LNKG),
        ROOT_PRT_ENTRY(8, 3, LNKH),

        ROOT_PRT_ENTRY(9, 0, LNKE),
        ROOT_PRT_ENTRY(9, 1, LNKF),
        ROOT_PRT_ENTRY(9, 2, LNKG),
        ROOT_PRT_ENTRY(9, 3, LNKH),

        ROOT_PRT_ENTRY(10, 0, LNKE),
        ROOT_PRT_ENTRY(10, 1, LNKF),
        ROOT_PRT_ENTRY(10, 2, LNKG),
        ROOT_PRT_ENTRY(10, 3, LNKH),

        ROOT_PRT_ENTRY(11, 0, LNKE),
        ROOT_PRT_ENTRY(11, 1, LNKF),
        ROOT_PRT_ENTRY(11, 2, LNKG),
        ROOT_PRT_ENTRY(11, 3, LNKH),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x100000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x100000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x100050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI1 */
    Device (PCI2) {
      Name (_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name (_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name (_SEG, 2)                   /* PCI Segment Group number */
      Name (_BBN, 0x0)                 /* PCI Base Bus Number */
      Name (_CCA, 1)                   /* Cache Coherency Attribute */
      Name (_PXM, 2)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKI),
        ROOT_PRT_ENTRY(0, 1, LNKJ),
        ROOT_PRT_ENTRY(0, 2, LNKK),
        ROOT_PRT_ENTRY(0, 3, LNKL),

        ROOT_PRT_ENTRY(1, 0, LNKI),
        ROOT_PRT_ENTRY(1, 1, LNKJ),
        ROOT_PRT_ENTRY(1, 2, LNKK),
        ROOT_PRT_ENTRY(1, 3, LNKL),

        ROOT_PRT_ENTRY(2, 0, LNKI),
        ROOT_PRT_ENTRY(2, 1, LNKJ),
        ROOT_PRT_ENTRY(2, 2, LNKK),
        ROOT_PRT_ENTRY(2, 3, LNKL),

        ROOT_PRT_ENTRY(3, 0, LNKI),
        ROOT_PRT_ENTRY(3, 1, LNKJ),
        ROOT_PRT_ENTRY(3, 2, LNKK),
        ROOT_PRT_ENTRY(3, 3, LNKL),

        ROOT_PRT_ENTRY(4, 0, LNKI),
        ROOT_PRT_ENTRY(4, 1, LNKJ),
        ROOT_PRT_ENTRY(4, 2, LNKK),
        ROOT_PRT_ENTRY(4, 3, LNKL),

        ROOT_PRT_ENTRY(5, 0, LNKI),
        ROOT_PRT_ENTRY(5, 1, LNKJ),
        ROOT_PRT_ENTRY(5, 2, LNKK),
        ROOT_PRT_ENTRY(5, 3, LNKL),

        ROOT_PRT_ENTRY(6, 0, LNKI),
        ROOT_PRT_ENTRY(6, 1, LNKJ),
        ROOT_PRT_ENTRY(6, 2, LNKK),
        ROOT_PRT_ENTRY(6, 3, LNKL),

        ROOT_PRT_ENTRY(7, 0, LNKI),
        ROOT_PRT_ENTRY(7, 1, LNKJ),
        ROOT_PRT_ENTRY(7, 2, LNKK),
        ROOT_PRT_ENTRY(7, 3, LNKL),

        ROOT_PRT_ENTRY(8, 0, LNKI),
        ROOT_PRT_ENTRY(8, 1, LNKJ),
        ROOT_PRT_ENTRY(8, 2, LNKK),
        ROOT_PRT_ENTRY(8, 3, LNKL),

        ROOT_PRT_ENTRY(9, 0, LNKI),
        ROOT_PRT_ENTRY(9, 1, LNKJ),
        ROOT_PRT_ENTRY(9, 2, LNKK),
        ROOT_PRT_ENTRY(9, 3, LNKL),

        ROOT_PRT_ENTRY(10, 0, LNKI),
        ROOT_PRT_ENTRY(10, 1, LNKJ),
        ROOT_PRT_ENTRY(10, 2, LNKK),
        ROOT_PRT_ENTRY(10, 3, LNKL),

        ROOT_PRT_ENTRY(11, 0, LNKI),
        ROOT_PRT_ENTRY(11, 1, LNKJ),
        ROOT_PRT_ENTRY(11, 2, LNKK),
        ROOT_PRT_ENTRY(11, 3, LNKL),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x200000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x200000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x200050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI2 */

    Device (PCI3) {
      Name (_HID, EISAID ("PNP0A08"))   /* PCI Express Root Bridge */
      Name (_CID, EISAID ("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name (_SEG, 3)                   /* PCI Segment Group number */
      Name (_BBN, 0x0)                 /* PCI Base Bus Number */
      Name (_CCA, 1)                   /* Cache Coherency Attribute */
      Name (_PXM, 3)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKM),
        ROOT_PRT_ENTRY(0, 1, LNKN),
        ROOT_PRT_ENTRY(0, 2, LNKO),
        ROOT_PRT_ENTRY(0, 3, LNKP),

        ROOT_PRT_ENTRY(1, 0, LNKM),
        ROOT_PRT_ENTRY(1, 1, LNKN),
        ROOT_PRT_ENTRY(1, 2, LNKO),
        ROOT_PRT_ENTRY(1, 3, LNKP),

        ROOT_PRT_ENTRY(2, 0, LNKM),
        ROOT_PRT_ENTRY(2, 1, LNKN),
        ROOT_PRT_ENTRY(2, 2, LNKO),
        ROOT_PRT_ENTRY(2, 3, LNKP),

        ROOT_PRT_ENTRY(3, 0, LNKM),
        ROOT_PRT_ENTRY(3, 1, LNKN),
        ROOT_PRT_ENTRY(3, 2, LNKO),
        ROOT_PRT_ENTRY(3, 3, LNKP),

        ROOT_PRT_ENTRY(4, 0, LNKM),
        ROOT_PRT_ENTRY(4, 1, LNKN),
        ROOT_PRT_ENTRY(4, 2, LNKO),
        ROOT_PRT_ENTRY(4, 3, LNKP),

        ROOT_PRT_ENTRY(5, 0, LNKM),
        ROOT_PRT_ENTRY(5, 1, LNKN),
        ROOT_PRT_ENTRY(5, 2, LNKO),
        ROOT_PRT_ENTRY(5, 3, LNKP),

        ROOT_PRT_ENTRY(6, 0, LNKM),
        ROOT_PRT_ENTRY(6, 1, LNKN),
        ROOT_PRT_ENTRY(6, 2, LNKO),
        ROOT_PRT_ENTRY(6, 3, LNKP),

        ROOT_PRT_ENTRY(7, 0, LNKM),
        ROOT_PRT_ENTRY(7, 1, LNKN),
        ROOT_PRT_ENTRY(7, 2, LNKO),
        ROOT_PRT_ENTRY(7, 3, LNKP),

        ROOT_PRT_ENTRY(8, 0, LNKM),
        ROOT_PRT_ENTRY(8, 1, LNKN),
        ROOT_PRT_ENTRY(8, 2, LNKO),
        ROOT_PRT_ENTRY(8, 3, LNKP),

        ROOT_PRT_ENTRY(9, 0, LNKM),
        ROOT_PRT_ENTRY(9, 1, LNKN),
        ROOT_PRT_ENTRY(9, 2, LNKO),
        ROOT_PRT_ENTRY(9, 3, LNKP),

        ROOT_PRT_ENTRY(10, 0, LNKM),
        ROOT_PRT_ENTRY(10, 1, LNKN),
        ROOT_PRT_ENTRY(10, 2, LNKO),
        ROOT_PRT_ENTRY(10, 3, LNKP),

        ROOT_PRT_ENTRY(11, 0, LNKM),
        ROOT_PRT_ENTRY(11, 1, LNKN),
        ROOT_PRT_ENTRY(11, 2, LNKO),
        ROOT_PRT_ENTRY(11, 3, LNKP),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x300000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x300000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x300050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI3 */

    Device (PCI4) {
      Name(_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name(_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name(_SEG, 4)                   /* PCI Segment Group number */
      Name(_BBN, 0x0)                 /* PCI Base Bus Number */
      Name(_CCA, 1)                   /* Cache Coherency Attribute */
      Name(_PXM, 4)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKQ),
        ROOT_PRT_ENTRY(0, 1, LNKR),
        ROOT_PRT_ENTRY(0, 2, LNKS),
        ROOT_PRT_ENTRY(0, 3, LNKT),

        ROOT_PRT_ENTRY(1, 0, LNKQ),
        ROOT_PRT_ENTRY(1, 1, LNKR),
        ROOT_PRT_ENTRY(1, 2, LNKS),
        ROOT_PRT_ENTRY(1, 3, LNKT),

        ROOT_PRT_ENTRY(2, 0, LNKQ),
        ROOT_PRT_ENTRY(2, 1, LNKR),
        ROOT_PRT_ENTRY(2, 2, LNKS),
        ROOT_PRT_ENTRY(2, 3, LNKT),

        ROOT_PRT_ENTRY(3, 0, LNKQ),
        ROOT_PRT_ENTRY(3, 1, LNKR),
        ROOT_PRT_ENTRY(3, 2, LNKS),
        ROOT_PRT_ENTRY(3, 3, LNKT),

        ROOT_PRT_ENTRY(4, 0, LNKQ),
        ROOT_PRT_ENTRY(4, 1, LNKR),
        ROOT_PRT_ENTRY(4, 2, LNKS),
        ROOT_PRT_ENTRY(4, 3, LNKT),

        ROOT_PRT_ENTRY(5, 0, LNKQ),
        ROOT_PRT_ENTRY(5, 1, LNKR),
        ROOT_PRT_ENTRY(5, 2, LNKS),
        ROOT_PRT_ENTRY(5, 3, LNKT),

        ROOT_PRT_ENTRY(6, 0, LNKQ),
        ROOT_PRT_ENTRY(6, 1, LNKR),
        ROOT_PRT_ENTRY(6, 2, LNKS),
        ROOT_PRT_ENTRY(6, 3, LNKT),

        ROOT_PRT_ENTRY(7, 0, LNKQ),
        ROOT_PRT_ENTRY(7, 1, LNKR),
        ROOT_PRT_ENTRY(7, 2, LNKS),
        ROOT_PRT_ENTRY(7, 3, LNKT),

        ROOT_PRT_ENTRY(8, 0, LNKQ),
        ROOT_PRT_ENTRY(8, 1, LNKR),
        ROOT_PRT_ENTRY(8, 2, LNKS),
        ROOT_PRT_ENTRY(8, 3, LNKT),

        ROOT_PRT_ENTRY(9, 0, LNKQ),
        ROOT_PRT_ENTRY(9, 1, LNKR),
        ROOT_PRT_ENTRY(9, 2, LNKS),
        ROOT_PRT_ENTRY(9, 3, LNKT),

        ROOT_PRT_ENTRY(10, 0, LNKQ),
        ROOT_PRT_ENTRY(10, 1, LNKR),
        ROOT_PRT_ENTRY(10, 2, LNKS),
        ROOT_PRT_ENTRY(10, 3, LNKT),

        ROOT_PRT_ENTRY(11, 0, LNKQ),
        ROOT_PRT_ENTRY(11, 1, LNKR),
        ROOT_PRT_ENTRY(11, 2, LNKS),
        ROOT_PRT_ENTRY(11, 3, LNKT),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x400000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x400000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x400050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI4 */
    Device(PCI5) {
      Name(_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name(_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name(_SEG, 5)                   /* PCI Segment Group number */
      Name(_BBN, 0x0)                 /* PCI Base Bus Number */
      Name(_CCA, 1)                   /* Cache Coherency Attribute */
      Name(_PXM, 5)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNKU),
        ROOT_PRT_ENTRY(0, 1, LNKV),
        ROOT_PRT_ENTRY(0, 2, LNKW),
        ROOT_PRT_ENTRY(0, 3, LNKX),

        ROOT_PRT_ENTRY(1, 0, LNKU),
        ROOT_PRT_ENTRY(1, 1, LNKV),
        ROOT_PRT_ENTRY(1, 2, LNKW),
        ROOT_PRT_ENTRY(1, 3, LNKX),

        ROOT_PRT_ENTRY(2, 0, LNKU),
        ROOT_PRT_ENTRY(2, 1, LNKV),
        ROOT_PRT_ENTRY(2, 2, LNKW),
        ROOT_PRT_ENTRY(2, 3, LNKX),

        ROOT_PRT_ENTRY(3, 0, LNKU),
        ROOT_PRT_ENTRY(3, 1, LNKV),
        ROOT_PRT_ENTRY(3, 2, LNKW),
        ROOT_PRT_ENTRY(3, 3, LNKX),

        ROOT_PRT_ENTRY(4, 0, LNKU),
        ROOT_PRT_ENTRY(4, 1, LNKV),
        ROOT_PRT_ENTRY(4, 2, LNKW),
        ROOT_PRT_ENTRY(4, 3, LNKX),

        ROOT_PRT_ENTRY(5, 0, LNKU),
        ROOT_PRT_ENTRY(5, 1, LNKV),
        ROOT_PRT_ENTRY(5, 2, LNKW),
        ROOT_PRT_ENTRY(5, 3, LNKX),

        ROOT_PRT_ENTRY(6, 0, LNKU),
        ROOT_PRT_ENTRY(6, 1, LNKV),
        ROOT_PRT_ENTRY(6, 2, LNKW),
        ROOT_PRT_ENTRY(6, 3, LNKX),

        ROOT_PRT_ENTRY(7, 0, LNKU),
        ROOT_PRT_ENTRY(7, 1, LNKV),
        ROOT_PRT_ENTRY(7, 2, LNKW),
        ROOT_PRT_ENTRY(7, 3, LNKX),

        ROOT_PRT_ENTRY(8, 0, LNKU),
        ROOT_PRT_ENTRY(8, 1, LNKV),
        ROOT_PRT_ENTRY(8, 2, LNKW),
        ROOT_PRT_ENTRY(8, 3, LNKX),

        ROOT_PRT_ENTRY(9, 0, LNKU),
        ROOT_PRT_ENTRY(9, 1, LNKV),
        ROOT_PRT_ENTRY(9, 2, LNKW),
        ROOT_PRT_ENTRY(9, 3, LNKX),

        ROOT_PRT_ENTRY(10, 0, LNKU),
        ROOT_PRT_ENTRY(10, 1, LNKV),
        ROOT_PRT_ENTRY(10, 2, LNKW),
        ROOT_PRT_ENTRY(10, 3, LNKX),

        ROOT_PRT_ENTRY(11, 0, LNKU),
        ROOT_PRT_ENTRY(11, 1, LNKV),
        ROOT_PRT_ENTRY(11, 2, LNKW),
        ROOT_PRT_ENTRY(11, 3, LNKX),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x500000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x500000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x500050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI5 */
    Device (PCI6) {
      Name (_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name (_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name (_SEG, 6)                   /* PCI Segment Group number */
      Name (_BBN, 0x0)                 /* PCI Base Bus Number */
      Name (_CCA, 1)                   /* Cache Coherency Attribute */
      Name (_PXM, 6)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNLA),
        ROOT_PRT_ENTRY(0, 1, LNLB),
        ROOT_PRT_ENTRY(0, 2, LNLC),
        ROOT_PRT_ENTRY(0, 3, LNLD),

        ROOT_PRT_ENTRY(1, 0, LNLA),
        ROOT_PRT_ENTRY(1, 1, LNLB),
        ROOT_PRT_ENTRY(1, 2, LNLC),
        ROOT_PRT_ENTRY(1, 3, LNLD),

        ROOT_PRT_ENTRY(2, 0, LNLA),
        ROOT_PRT_ENTRY(2, 1, LNLB),
        ROOT_PRT_ENTRY(2, 2, LNLC),
        ROOT_PRT_ENTRY(2, 3, LNLD),

        ROOT_PRT_ENTRY(3, 0, LNLA),
        ROOT_PRT_ENTRY(3, 1, LNLB),
        ROOT_PRT_ENTRY(3, 2, LNLC),
        ROOT_PRT_ENTRY(3, 3, LNLD),

        ROOT_PRT_ENTRY(4, 0, LNLA),
        ROOT_PRT_ENTRY(4, 1, LNLB),
        ROOT_PRT_ENTRY(4, 2, LNLC),
        ROOT_PRT_ENTRY(4, 3, LNLD),

        ROOT_PRT_ENTRY(5, 0, LNLA),
        ROOT_PRT_ENTRY(5, 1, LNLB),
        ROOT_PRT_ENTRY(5, 2, LNLC),
        ROOT_PRT_ENTRY(5, 3, LNLD),

        ROOT_PRT_ENTRY(6, 0, LNLA),
        ROOT_PRT_ENTRY(6, 1, LNLB),
        ROOT_PRT_ENTRY(6, 2, LNLC),
        ROOT_PRT_ENTRY(6, 3, LNLD),

        ROOT_PRT_ENTRY(7, 0, LNLA),
        ROOT_PRT_ENTRY(7, 1, LNLB),
        ROOT_PRT_ENTRY(7, 2, LNLC),
        ROOT_PRT_ENTRY(7, 3, LNLD),

        ROOT_PRT_ENTRY(8, 0, LNLA),
        ROOT_PRT_ENTRY(8, 1, LNLB),
        ROOT_PRT_ENTRY(8, 2, LNLC),
        ROOT_PRT_ENTRY(8, 3, LNLD),

        ROOT_PRT_ENTRY(9, 0, LNLA),
        ROOT_PRT_ENTRY(9, 1, LNLB),
        ROOT_PRT_ENTRY(9, 2, LNLC),
        ROOT_PRT_ENTRY(9, 3, LNLD),

        ROOT_PRT_ENTRY(10, 0, LNLA),
        ROOT_PRT_ENTRY(10, 1, LNLB),
        ROOT_PRT_ENTRY(10, 2, LNLC),
        ROOT_PRT_ENTRY(10, 3, LNLD),

        ROOT_PRT_ENTRY(11, 0, LNLA),
        ROOT_PRT_ENTRY(11, 1, LNLB),
        ROOT_PRT_ENTRY(11, 2, LNLC),
        ROOT_PRT_ENTRY(11, 3, LNLD),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x600000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x600000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x600050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI6 */
    Device (PCI7) {
      Name (_HID, EISAID("PNP0A08"))   /* PCI Express Root Bridge */
      Name (_CID, EISAID("PNP0A03"))   /* Compatible PCI Root Bridge */
      Name (_SEG, 7)                   /* PCI Segment Group number */
      Name (_BBN, 0x0)                 /* PCI Base Bus Number */
      Name (_CCA, 1)                   /* Cache Coherency Attribute */
      Name (_PXM, 7)
      /* PCI Routing Table */
      Name (_PRT, Package() {
        ROOT_PRT_ENTRY(0, 0, LNLE),
        ROOT_PRT_ENTRY(0, 1, LNLF),
        ROOT_PRT_ENTRY(0, 2, LNLG),
        ROOT_PRT_ENTRY(0, 3, LNLH),

        ROOT_PRT_ENTRY(1, 0, LNLE),
        ROOT_PRT_ENTRY(1, 1, LNLF),
        ROOT_PRT_ENTRY(1, 2, LNLG),
        ROOT_PRT_ENTRY(1, 3, LNLH),

        ROOT_PRT_ENTRY(2, 0, LNLE),
        ROOT_PRT_ENTRY(2, 1, LNLF),
        ROOT_PRT_ENTRY(2, 2, LNLG),
        ROOT_PRT_ENTRY(2, 3, LNLH),

        ROOT_PRT_ENTRY(3, 0, LNLE),
        ROOT_PRT_ENTRY(3, 1, LNLF),
        ROOT_PRT_ENTRY(3, 2, LNLG),
        ROOT_PRT_ENTRY(3, 3, LNLH),

        ROOT_PRT_ENTRY(4, 0, LNLE),
        ROOT_PRT_ENTRY(4, 1, LNLF),
        ROOT_PRT_ENTRY(4, 2, LNLG),
        ROOT_PRT_ENTRY(4, 3, LNLH),

        ROOT_PRT_ENTRY(5, 0, LNLE),
        ROOT_PRT_ENTRY(5, 1, LNLF),
        ROOT_PRT_ENTRY(5, 2, LNLG),
        ROOT_PRT_ENTRY(5, 3, LNLH),

        ROOT_PRT_ENTRY(6, 0, LNLE),
        ROOT_PRT_ENTRY(6, 1, LNLF),
        ROOT_PRT_ENTRY(6, 2, LNLG),
        ROOT_PRT_ENTRY(6, 3, LNLH),

        ROOT_PRT_ENTRY(7, 0, LNLE),
        ROOT_PRT_ENTRY(7, 1, LNLF),
        ROOT_PRT_ENTRY(7, 2, LNLG),
        ROOT_PRT_ENTRY(7, 3, LNLH),

        ROOT_PRT_ENTRY(8, 0, LNLE),
        ROOT_PRT_ENTRY(8, 1, LNLF),
        ROOT_PRT_ENTRY(8, 2, LNLG),
        ROOT_PRT_ENTRY(8, 3, LNLH),

        ROOT_PRT_ENTRY(9, 0, LNLE),
        ROOT_PRT_ENTRY(9, 1, LNLF),
        ROOT_PRT_ENTRY(9, 2, LNLG),
        ROOT_PRT_ENTRY(9, 3, LNLH),

        ROOT_PRT_ENTRY(10, 0, LNLE),
        ROOT_PRT_ENTRY(10, 1, LNLF),
        ROOT_PRT_ENTRY(10, 2, LNLG),
        ROOT_PRT_ENTRY(10, 3, LNLH),

        ROOT_PRT_ENTRY(11, 0, LNLE),
        ROOT_PRT_ENTRY(11, 1, LNLF),
        ROOT_PRT_ENTRY(11, 2, LNLG),
        ROOT_PRT_ENTRY(11, 3, LNLH),
      })
      Method (_STA, 0, NotSerialized) { // _STA: Status
        Return(0xF)
      }
      /* Root complex resources */
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber (                 /* Bus numbers assigned to this root */
            ResourceProducer,
            MinFixed, MaxFixed, PosDecode,
            0,                            /* AddressGranularity */
            0x0,                          /* AddressMinimum - Minimum Bus Number */
            0xff,                         /* AddressMaximum - Maximum Bus Number */
            0,                            /* AddressTranslation - Set to 0 */
            0x100                         /* RangeLength - Number of Busses */
          )

          QWordMemory (                   /* 32-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x58000000,                   /* Min Base Address */
            0x7fffffff,                   /* Max Base Address */
            0x700000000000,               /* Translate */
            0x28000000                    /* Length */
          )

          QWordMemory (                   /* 64-bit BAR Windows */
            ResourceProducer, PosDecode,
            MinFixed, MaxFixed,
            Cacheable, ReadWrite,
            0x00000000,                   /* Granularity */
            0x40000000000,                /* Min Base Address */
            0x4ffffffffff,                /* Max Base Address */
            0x700000000000,               /* Translate */
            0x10000000000                 /* Length */
          )

          QWordIo (                       /* IO window */
            ResourceProducer,
            MinFixed,
            MaxFixed,
            PosDecode,
            EntireRange,
            0x00000000,                   /* Granularity */
            0x00000000,                   /* Min Base Address */
            0x0000ffff,                   /* Max Base Address */
            0x700050000000,               /* Translate */
            0x00010000,                   /* Length */
            ,,,TypeTranslation
          )
        }) /* Name(RBUF)*/
        Return (RBUF)
      } /* Method(_CRS) */
      PCI_OSC_SUPPORT()
    } /* PCI7 */
  }
}
