/** @file
  The description of UART.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Platform.h>

Scope(_SB)
{

  Device(PWRB){
    Name(_HID,"PNP0C0C")
    Name(_UID,Zero)
    Method(_STA,0x0,NotSerialized){
      Return(0xF)
    }
  }

  Device (GED0) {
    Name (_HID, "ACPI0013")
    Name (_UID, Zero)
    Name (_CRS, ResourceTemplate () {
     Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake) { 37 }
    })

    OperationRegion (GPIO, SystemMemory, 0x2000a000, 0x40)
    Field (GPIO, DWordAcc, NoLock, Preserve){
      Offset (0x0),
      PADT, 32,
      Offset (0x4),
      PADR, 32,
      Offset (0x8),
      PAIN, 32,
      Offset (0x18),
      PAIE, 32,
      Offset (0x1c),
      PAIM, 32,
      Offset (0x20),
      PAIL, 32,
      Offset (0x24),
      PAIP, 32,
      Offset (0x28),
      PAST, 32,
      Offset (0x2c),
      PARS, 32,
      Offset (0x30),
      PALS, 32,
      Offset (0x34),
      PADB, 32,
      Offset (0x38),
      PCLR, 32,
    }

    Method (_INI) {
        PAIE |= 0x100
        PAIL |= 0x100
        PAIP &= 0xfffffeff
    }

    Method (_STA, 0x0, NotSerialized){
      Return (0xF)
    }

    Method (_EVT, 0x1) {
      Switch(ToInteger(Arg0)){
        Case(37){
          Store(PAST,Local0)
          And(Local0, 0x100, Local0)
          Switch(ToInteger(Local0)){
            Case(0x100){  /*be gpio8 */
              Notify(\_SB.PWRB, 0x80)

              Store(PCLR,Local0)
              Or(Local0,0x1,Local0)//clear interrupt
              Store(Local0,PCLR)
            }
          }
          Return
        }
      }
    }
  }


}
