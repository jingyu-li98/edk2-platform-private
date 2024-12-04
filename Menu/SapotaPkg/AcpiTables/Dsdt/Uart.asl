/** @file
  The description of UART.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Platform.h>

Scope(_SB)
{
  // Die 0 UART 0
  Device(D0U0) {
    Name(_HID, "ARMH0011")
    Name(_UID, 0)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x20000000,            // Min Base Address
        0x20000fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 48 }
    })
  }

  // Die 0 UART 1
  Device(D0U1) {
    Name(_HID, "ARMH0011")
    Name(_UID, 1)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x20001000,            // Min Base Address
        0x20001fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 47 }
    })
  }
/*
// Die 0 UART 0
  Device(D0U0) {
    Name(_HID, "ARMH0011")
    Name(_UID, 0)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x20000000,            // Min Base Address
        0x20000fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 48 }
    })
  }

*/
  // Die 0 UART 2
  Device(D0U2) {
    Name(_HID, "ARMH0011")
    Name(_UID, 2)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x20002000,            // Min Base Address
        0x20002fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 46 }
    })
  }

  // Die 1 UART 0
  Device(D1U0) {
    Name(_HID, "ARMH0011")
    Name(_UID, 3)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x100020000000,            // Min Base Address
        0x100020000fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 144 }
    })
  }

  // Die 1 UART 1
  Device(D1U1) {
    Name(_HID, "ARMH0011")
    Name(_UID, 4)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x100020001000,            // Min Base Address
        0x100020001fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 143 }
    })
  }

  // Die 1 UART 2
  Device(D1U2) {
    Name(_HID, "ARMH0011")
    Name(_UID, 5)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x100020002000,            // Min Base Address
        0x100020002fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 142 }
    })
  }

  // Die 2 UART 0
  Device(D2U0) {
    Name(_HID, "ARMH0011")
    Name(_UID, 6)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x200020000000,            // Min Base Address
        0x200020000fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 240 }
    })
  }

  // Die 2 UART 1
  Device(D2U1) {
    Name(_HID, "ARMH0011")
    Name(_UID, 7)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x200020001000,            // Min Base Address
        0x200020001fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 239 }
    })
  }

  // Die 2 UART 2
  Device(D2U2) {
    Name(_HID, "ARMH0011")
    Name(_UID, 8)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x200020002000,        // Min Base Address
        0x200020002fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 238 }
    })
  }

  // Die 3 UART 0
  Device(D3U0) {
    Name(_HID, "ARMH0011")
    Name(_UID, 9)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x300020000000,            // Min Base Address
        0x300020000fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 336 }
    })
  }

  // Die 3 UART 1
  Device(D3U1) {
    Name(_HID, "ARMH0011")
    Name(_UID, 10)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x300020001000,            // Min Base Address
        0x300020001fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 335 }
    })
  }

  // Die 3 UART 2
  Device(D3U2) {
    Name(_HID, "ARMH0011")
    Name(_UID, 11)
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,      // Resource Usage
        PosDecode,             // Decode
        MinFixed,              // IsMinFixed
        MaxFixed,              // IsMaxFixed
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x300020002000,            // Min Base Address
        0x300020002fff,        // Max Base Address
        0x00000000,            // Translate
        0x00001000,            // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 334 }
    })
  }

#if 0
  Method(TCOM, 1, Serialized) {
    OperationRegion(
      COM0,
      SystemMemory,
      UART1_BASE,
      0x1000
      )
    Field(COM0, DWordAcc, NoLock, Preserve) {
      DAT8,
      8,
      Offset(0x05),
      ,
      5,
      TRDY,
      1,
    }

  Add(SizeOf(Arg0), One, Local0)
  Name(BUF0, Buffer(Local0){})
  Store(Arg0, BUF0)
  store(0, Local1)
  Decrement(Local0)
  While(LNotEqual(Local1, Local0)){
    while(LEqual(TRDY, ONE)){}
      Store(DerefOf(Index(BUF0, Local1)), DAT8)
      Increment(Local1)
    }
  }

  Method(DBGC, 3, Serialized) {
    Name(CRLF, Buffer(2){0x0D, 0x0A})
    TCOM(Arg1)
    if(LEqual(Arg0, 2)){
      TCOM(ToHexString(Arg2))
    }
    TCOM(CRLF)
  }
#endif
}
