/** @file
The description of DIE 7 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die7
  Device (CL70)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 48)
    //die 7 core 0
    Device (CP70)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x70)

      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCE)
      }
    }

    //die 7 core 1
    Device (CP71)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x71)
      Method (_PSD,0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCE)
      }
    }
  }

  Device (CL71)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 49)
    //die 7 core 2
    Device (CP72)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x72)

      Method (_PSD,0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }

    //die 7 core 3
    Device (CP73)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x73)
      Method (_PSD,0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }
  }

  Device (CL72)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 50)
    //die 7 core 4
    Device (CP74)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x74)
      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCE)
      }
    }

    //die 7 core 5
    Device (CP75)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x75)
      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCE)
      }
    }
  }

  Device (CL73)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 51)
    //die 7 core 6
    Device (CP76)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x76)
      Method (_PSD, 0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCF)
      }
    }

    //die 7 core 7
    Device (CP77)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x77)
      Method (_PSD, 0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }
  }

  Device (CL74)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 52)
    //die 7 core 8
    Device (CP78)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x78)
      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCE)
      }
    }

    //die 7 core 9
    Device (CP79)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x79)
      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCE)
      }
    }
  }

  Device (CL75)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 53)
    //die 7 core 10
    Device (CP7A)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x7a)
      Method (_PSD, 0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }

    //die 7 core 11
    Device (CP7B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x7b)
      Method (_PSD, 0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCF)
      }
    }
  }

  Device (CL76)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 54)
    //die 7 core 12
    Device (CP7C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x7c)
      Method (_PSD,0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCE)
      }
    }

    //die 7 core 13
    Device (CP7D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x7d)
      Method (_PSD, 0)
      {
        return (\_SB.PSDE)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCE)
      }
    }
  }

  Device (CL77)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 55)
    //die 7 core 14
    Device (CP7E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x7e)
      Method (_PSD, 0)
      {
        return (\_SB.PSDF)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }

    //die 7 core 15
    Device (CP7F)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x7f)
      Method (_PSD,0)
      {
        return (\_SB.PSDF)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCF)
      }
    }
  }
}
