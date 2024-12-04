/** @file
The description of DIE 2 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope (_SB)
{
  //die2
  Device (CL20)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 16)
    //die 2 core 0
    Device (CP20)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x20)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }

    //die 2 core 1
    Device (CP21)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x21)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }
  }

  Device (CL21)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 17)
    //die 2 core 2
    Device (CP22)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x22)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }

    //die 2 core 3
    Device (CP23)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x23)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }
  }

  Device (CL22)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 18)
    //die 2 core 4
    Device (CP24)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x24)

      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }

    }

    //die 2 core 5
    Device (CP25)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x25)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }
  }

  Device (CL23)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 19)
    //die 2 core 6
    Device (CP26)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x26)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }

    //die 2 core 7
    Device (CP27)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x27)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }

  }

  Device (CL24)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 20)
    //die 2 core 8
    Device (CP28)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x28)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }

    //die 2 core 9
    Device (CP29)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x29)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }
  }

  Device (CL25)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 21)
    //die 2 core 10
    Device (CP2A)
    {
      Name (_HID, "ACPI0007" )  // _HID: Hardware ID
      Name (_UID, 0x2A)  // _UID: Unique ID
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }

    //die 2 core 11
    Device (CP2B)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x2B)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }
  }

  Device (CL26)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 22)
    //die 2 core 12
    Device (CP2C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x2C)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }

    //die 2 core 13
    Device (CP2D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x2D)
      Method (_PSD, 0)
      {
        return (\_SB.PSD4)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC4)
      }
    }
  }

  Device (CL27)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 23)
    //die 2 core 14
    Device (CP2E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x2E)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }

    //die 2 core 15
    Device (CP2F)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x2F)
      Method (_PSD, 0)
      {
        return (\_SB.PSD5)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC5)
      }
    }
  }
}
