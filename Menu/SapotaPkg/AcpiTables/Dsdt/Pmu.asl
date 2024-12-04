/** @file
  The description of PMU.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

Scope(_SB)
{
  //DDR Die 0
  Device(MC00) {
    Name(_HID, "PHYT0043")
    Name(_UID, 0)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 0: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC01) {
    Name(_HID, "PHYT0043")
    Name(_UID, 1)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 0: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC02) {
    Name(_HID, "PHYT0043")
    Name(_UID, 2)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 0: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC03) {
    Name(_HID, "PHYT0043")
    Name(_UID, 3)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 0: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 1
  Device(MC10) {
    Name(_HID, "PHYT0043")
    Name(_UID, 4)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 1: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC11) {
    Name(_HID, "PHYT0043")
    Name(_UID, 5)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 1: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC12) {
    Name(_HID, "PHYT0043")
    Name(_UID, 6)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 1: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC13) {
    Name(_HID, "PHYT0043")
    Name(_UID, 7)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 1: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 2
  Device(MC20) {
    Name(_HID, "PHYT0043")
    Name(_UID, 8)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 2: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC21) {
    Name(_HID, "PHYT0043")
    Name(_UID, 9)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 2: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC22) {
    Name(_HID, "PHYT0043")
    Name(_UID, 10)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 2: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC23) {
    Name(_HID, "PHYT0043")
    Name(_UID, 11)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 2: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 3
  Device(MC30) {
    Name(_HID, "PHYT0043")
    Name(_UID, 12)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 3: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC31) {
    Name(_HID, "PHYT0043")
    Name(_UID, 13)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 3: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC32) {
    Name(_HID, "PHYT0043")
    Name(_UID, 14)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 3: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC33) {
    Name(_HID, "PHYT0043")
    Name(_UID, 15)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 3: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 4
  Device(MC40) {
    Name(_HID, "PHYT0043")
    Name(_UID, 16)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 4: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC41) {
    Name(_HID, "PHYT0043")
    Name(_UID, 17)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 4: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC42) {
    Name(_HID, "PHYT0043")
    Name(_UID, 18)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 4: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC43) {
    Name(_HID, "PHYT0043")
    Name(_UID, 19)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 4: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 5
  Device(MC50) {
    Name(_HID, "PHYT0043")
    Name(_UID, 20)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 5: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC51) {
    Name(_HID, "PHYT0043")
    Name(_UID, 21)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 5: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC52) {
    Name(_HID, "PHYT0043")
    Name(_UID, 22)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 5: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC53) {
    Name(_HID, "PHYT0043")
    Name(_UID, 23)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 5: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 6
  Device(MC60) {
    Name(_HID, "PHYT0043")
    Name(_UID, 24)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 6: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC61) {
    Name(_HID, "PHYT0043")
    Name(_UID, 25)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 6: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC62) {
    Name(_HID, "PHYT0043")
    Name(_UID, 26)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 6: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC63) {
    Name(_HID, "PHYT0043")
    Name(_UID, 27)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 6: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  //DDR Die 7
  Device(MC70) {
    Name(_HID, "PHYT0043")
    Name(_UID, 28)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 7: MCU0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c032000,   // AddressMinimum - MIN
        0x000000003c032fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC71) {
    Name(_HID, "PHYT0043")
    Name(_UID, 29)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 7: MCU1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c033000,   // AddressMinimum - MIN
        0x000000003c033fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,ddr-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(MC72) {
    Name(_HID, "PHYT0043")
    Name(_UID, 30)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 7: MCU2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c432000,   // AddressMinimum - MIN
        0x000000003c432fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(MC73) {
    Name(_HID, "PHYT0043")
    Name(_UID, 31)
    Name(_CCA, ONE)
    Name(_STR, Unicode("DIE 7: MCU3"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003c433000,   // AddressMinimum - MIN
        0x000000003c433fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000024,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000008    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,ddr-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }

  //PCIe
  //PCIe Die 0
  Device(PC00) {
    Name(_HID, "PHYT0044")
    Name(_UID, 0)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 0: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC01) {
    Name(_HID, "PHYT0044")
    Name(_UID, 1)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 0: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC02) {
    Name(_HID, "PHYT0044")
    Name(_UID, 2)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 0: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 62 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",0},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 1
  Device(PC10) {
    Name(_HID, "PHYT0044")
    Name(_UID, 3)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 1: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC11) {
    Name(_HID, "PHYT0044")
    Name(_UID, 4)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 1: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC12) {
    Name(_HID, "PHYT0044")
    Name(_UID, 5)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 1: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 158 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",1},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 2
  Device(PC20) {
    Name(_HID, "PHYT0044")
    Name(_UID, 6)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 2: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC21) {
    Name(_HID, "PHYT0044")
    Name(_UID, 7)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 2: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC22) {
    Name(_HID, "PHYT0044")
    Name(_UID, 8)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 2: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 254 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",2},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 3
  Device(PC30) {
    Name(_HID, "PHYT0044")
    Name(_UID, 9)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 3: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC31) {
    Name(_HID, "PHYT0044")
    Name(_UID, 10)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 3: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC32) {
    Name(_HID, "PHYT0044")
    Name(_UID, 11)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 3: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 350 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",3},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 4
  Device(PC40) {
    Name(_HID, "PHYT0044")
    Name(_UID, 12)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 4: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC41) {
    Name(_HID, "PHYT0044")
    Name(_UID, 13)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 4: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC42) {
    Name(_HID, "PHYT0044")
    Name(_UID, 14)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 4: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000400000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",4},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 5
  Device(PC50) {
    Name(_HID, "PHYT0044")
    Name(_UID, 15)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 5: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC51) {
    Name(_HID, "PHYT0044")
    Name(_UID, 16)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 5: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC52) {
    Name(_HID, "PHYT0044")
    Name(_UID, 17)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 5: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000500000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 542 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",5},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 6
  Device(PC60) {
    Name(_HID, "PHYT0044")
    Name(_UID, 18)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 6: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC61) {
    Name(_HID, "PHYT0044")
    Name(_UID, 19)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 6: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC62) {
    Name(_HID, "PHYT0044")
    Name(_UID, 20)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 6: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000600000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 638 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",6},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
  //PCIe Die 7
  Device(PC70) {
    Name(_HID, "PHYT0044")
    Name(_UID, 21)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 7: PCIe PMU 0"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b420000,   // AddressMinimum - MIN
        0x000000003b420fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,pmu-id",0},
      }
    })
  }
  Device(PC71) {
    Name(_HID, "PHYT0044")
    Name(_UID, 22)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 7: PCIe PMU 1"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b421000,   // AddressMinimum - MIN
        0x000000003b421fff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b400280,   // AddressMinimum - MIN
        0x000000003b40028f,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000010    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,pmu-id",1},
      }
    })
  }
  Device(PC72) {
    Name(_HID, "PHYT0044")
    Name(_UID, 23)
    Name(_CCA, 1)
    Name(_STR, Unicode("DIE 7: PCIe PMU 2"))
    Method(_STA, 0, NotSerialized) {
      Return (0x0f)
    }
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b690000,   // AddressMinimum - MIN
        0x000000003b69ffff,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000010000    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000003b68001c,   // AddressMinimum - MIN
        0x000000003b68001f,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x000000001b000028,   // AddressMinimum - MIN
        0x000000001b00002b,   // AddressMaximum - MAX
        0x0000700000000000,   // AddressTranslation - TRA
        0x0000000000000004    // RangeLength - LEN
      )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 734 }
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"phytium,die-id",7},
        Package () {"phytium,pmu-id",2},
      }
    })
  }
}
