/** @file
The description of CEU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/

Scope(_SB)
{
  Device(CEU0) {
    Name(_HID, "PHYT0041")
    Name(_UID, 0)
    Name(_CCA, 1)
    Name(_PXM, 0) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x3ba00000,            // Min Base Address
        0x3bbfffff,            // Max Base Address
        0x00000000,            // Translate
        0x200000,              // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 63 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 64 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 65 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 66 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 67 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 68 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 69 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 70 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 71 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 72 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 73 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 74 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 75 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 76 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 77 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 78 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 79 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 80 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 81 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 82 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 119 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 120 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU1) {
    Name(_HID, "PHYT0041")
    Name(_UID, 1)
    Name(_CCA, 1)
    Name(_PXM, 1) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x3ba00000,            // Min Base Address
        0x3bbfffff,            // Max Base Address
        0x100000000000,        // Translate
        0x200000,              // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 159 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 160 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 161 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 162 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 163 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 164 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 165 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 166 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 167 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 168 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 169 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 170 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 171 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 172 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 173 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 174 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 175 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 176 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 177 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 178 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 215 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 216 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU2) {
    Name(_HID, "PHYT0041")
    Name(_UID, 2)
    Name(_CCA, 1)
    Name(_PXM, 2) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,                // Granularity
        0x3ba00000,                // Min Base Address
        0x3bbfffff,                // Max Base Address
        0x200000000000,            // Translate
        0x200000,                  // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 255 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 256 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 257 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 258 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 259 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 260 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 261 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 262 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 263 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 264 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 265 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 266 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 267 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 268 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 269 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 270 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 271 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 272 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 273 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 274 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 311 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 312 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU3) {
    Name(_HID, "PHYT0041")
    Name(_UID, 3)
    Name(_CCA, 1)
    Name(_PXM, 3) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,                // Granularity
        0x3ba00000,                // Min Base Address
        0x3bbfffff,                // Max Base Address
        0x300000000000,            // Translate
        0x200000,                  // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 351 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 352 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 353 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 354 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 355 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 356 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 357 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 358 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 359 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 360 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 361 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 362 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 363 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 364 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 365 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 366 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 367 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 368 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 369 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 370 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 407 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 408 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }

  Device(CEU4) {
    Name(_HID, "PHYT0041")
    Name(_UID, 4)
    Name(_CCA, 1)
    Name(_PXM, 4) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,            // Granularity
        0x3ba00000,            // Min Base Address
        0x3bbfffff,            // Max Base Address
        0x400000000000,        // Translate
        0x200000,              // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 447 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 448 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 449 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 450 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 451 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 452 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 453 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 454 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 455 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 456 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 457 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 458 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 459 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 460 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 461 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 462 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 463 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 464 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 465 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 466 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 503 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 504 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU5) {
    Name(_HID, "PHYT0041")
    Name(_UID, 5)
    Name(_CCA, 1)
    Name(_PXM, 5) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,                // Granularity
        0x3ba00000,                // Min Base Address
        0x3bbfffff,                // Max Base Address
        0x500000000000,            // Translate
        0x200000,                  // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 543 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 544 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 545 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 546 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 547 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 548 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 549 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 550 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 551 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 552 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 553 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 554 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 555 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 556 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 557 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 558 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 559 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 560 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 561 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 562 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 599 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 600 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU6) {
    Name(_HID, "PHYT0041")
    Name(_UID, 6)
    Name(_CCA, 1)
    Name(_PXM, 6) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,                // Granularity
        0x3ba00000,                // Min Base Address
        0x3bbfffff,                // Max Base Address
        0x600000000000,            // Translate
        0x200000,                  // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 639 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 640 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 641 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 642 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 643 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 644 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 645 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 646 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 647 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 648 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 649 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 650 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 651 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 652 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 653 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 654 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 655 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 656 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 657 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 658 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 695 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 696 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
  Device(CEU7) {
    Name(_HID, "PHYT0041")
    Name(_UID, 7)
    Name(_CCA, 1)
    Name(_PXM, 7) //NUMA
    Name(_CRS, ResourceTemplate() {
      QWordMemory (
        ResourceProducer,
        PosDecode,
        MinFixed,
        MaxFixed,
        NonCacheable,
        ReadWrite,
        0x00000000,                // Granularity
        0x3ba00000,                // Min Base Address
        0x3bbfffff,                // Max Base Address
        0x700000000000,            // Translate
        0x200000,                  // Length
        , , BUFF
        )
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 735 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 736 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 737 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 738 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 739 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 740 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 741 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 742 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 743 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 744 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 745 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 746 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 747 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 748 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 749 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 750 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 751 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 752 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 753 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 754 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 791 }
      Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive) { 792 }
    })
    Method (_STA, 0, NotSerialized) { // _STA: Status
      Return(0xF)
    }
  }
}
