/** @file
I2c description. Take the i2c1 of die0-3 as an example.

Copyright (c) 2024, Phytium Limited. All rights reserved.
**/
Scope(_SB)
{
  Device (CLK2) {
    Name (_HID, "PHYT8002")
    Name (_CID, "FTCK0002") /* compatible with v1.0 */
    Name (_UID, 0x01)
    Name (_DSD, Package() {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package () {
          Package(2) {"clock-frequency", 48000000}
        }
      })

    Method (FREQ, 0x0, NotSerialized) {
      Return (48000000)
    }
  }

  //Die0 I2C1
  Device (I2C0) {
    Name (_HID, "PHYT0038")
    Name (_UID, 0)
    Name (_CRS, ResourceTemplate () {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x0000000020004000,   // AddressMinimum - MIN
        0x0000000020004fff,   // AddressMaximum - MAX
        0x0000000000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {44}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0xEF,
          0xEF,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x35,
          0x35,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x8,
          0x8,
          0
      })
    }
  }

  //Die1 I2C1
  Device (I2C1) {
    Name (_HID, "PHYT0038")
    Name (_UID, 1)
    Name (_CRS, ResourceTemplate () {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x0000000020004000,   // AddressMinimum - MIN
        0x0000000020004fff,   // AddressMaximum - MAX
        0x0000100000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {140}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0xEF,
          0xEF,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x35,
          0x35,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x8,
          0x8,
          0
      })
    }
  }
  //Die2 I2C1
  Device (I2C2) {
    Name (_HID, "PHYT0038")
    Name (_UID, 2)
    Name (_CRS, ResourceTemplate () {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x0000000020004000,   // AddressMinimum - MIN
        0x0000000020004fff,   // AddressMaximum - MAX
        0x0000200000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {236}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0xEF,
          0xEF,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x35,
          0x35,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x8,
          0x8,
          0
      })
    }
  }
  //Die3 I2C1
  Device (I2C3) {
    Name (_HID, "PHYT0038")
    Name (_UID, 3)
    Name (_CRS, ResourceTemplate () {
      QWordMemory (
        ResourceProducer,     // ResourceUsage
        PosDecode,            // Decode
        MinFixed,             // IsMinFixed
        MaxFixed,             // IsMaxFixed
        NonCacheable,         // Cacheable
        ReadWrite,            // ReadAndWrite
        0x0000000000000000,   // AddressGranularity - GRA
        0x0000000020004000,   // AddressMinimum - MIN
        0x0000000020004fff,   // AddressMaximum - MAX
        0x0000300000000000,   // AddressTranslation - TRA
        0x0000000000001000    // RangeLength - LEN
      )
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {332}
    })
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"clock-frequency", 400000},
        Package () {"clocks", Package () {"\\_SB.CLK2"}}
      }
    })

    //100k
    Method (SSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0xEF,
          0xEF,
          0
      })
    }

    //400K
    Method (FMCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x35,
          0x35,
          0
      })
    }

    //1M
    Method (FPCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          100,
          100,
          0
      })
    }

    //3.4M
    Method (HSCN, 0, NotSerialized)
    {
      Return (Package (0x03)
      {
          0x8,
          0x8,
          0
      })
    }
  }
}
