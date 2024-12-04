/** @file
  The description of SPI Ctroller

  Copyright (c) 2024, Phytium Technology Co., Ltd. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  Device (SPI0) {
    Name (_HID, "PHYT000E")  // _HID: Hardware ID
    Name (_UID, Zero)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
        QWordMemory (
          ResourceProducer,     // ResourceUsage
          PosDecode,            // Decode
          MinFixed,             // IsMinFixed
          MaxFixed,             // IsMaxFixed
          NonCacheable,         // Cacheable
          ReadWrite,            // ReadAndWrite
          0x0000000000000000,   // AddressGranularity - GRA
          0x0000000020006000,   // AddressMinimum - MIN
          0x0000000020006fff,   // AddressMaximum - MAX
          0x0000000000000000,   // AddressTranslation - TRA
          0x0000000000001000    // RangeLength - LEN
        )
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {38}
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
        Package (0x02)
        {
            Package (0x02)
            {
                "num-cs",
                0x04
            },

            Package (0x02)
            {
                "global-cs",
                0x01
            }
        }
    })

    Device (TPM)
    {
        Name (_ADR, Zero)  // _ADR: Address
        Name (_HID, "SMO0768")  // _HID: Hardware ID
        Name (_UID, Zero)  // _UID: Unique ID
        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Name (RBUF, ResourceTemplate ()
            {
              SpiSerialBusV2 (
                0x0000,
                PolarityLow,
                FourWireMode,
                0x08,
                ControllerInitiated,
                0x000F4240,
                ClockPolarityLow,
                ClockPhaseFirst,
                "\\_SB.SPI0",
                0x00,
                ResourceConsumer, , Exclusive,
                )
            })
            Return (RBUF) /* \_SB_.SPI0.TPM_._CRS.RBUF */
        }
    }
  }
}
