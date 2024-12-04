/** @file
The description of DIE 5 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die5
  Device (CL50)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 40)
    //die 5 core 0
    Device (CP50)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x50)

      Method (_PSD, 0)
      {
        return (\_SB.PSDA)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCA)
      }
    }

    //die 5 core 1
    Device (CP51)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x51)
      Method (_PSD,0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCA)
      }
    }
  }

  Device (CL51)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 41)
    //die 5 core 2
    Device (CP52)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x52)

      Method(_PSD,0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }

    //die 5 core 3
    Device (CP53)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x53)
      Method (_PSD,0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }
  }

  Device (CL52)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 42)
    //die 5 core 4
    Device (CP54)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x54)
      Method (_PSD, 0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCA)
      }
    }

    //die 5 core 5
    Device (CP55)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x55)
      Method (_PSD, 0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCA)
      }
    }
  }

  Device (CL53)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 43)
    //die 5 core 6
    Device (CP56)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x56)
      Method (_PSD, 0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCB)
      }
    }

    //die 5 core 7
    Device (CP57)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x57)
      Method (_PSD, 0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }
  }

  Device (CL54)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 44)
    //die 5 core 8
    Device (CP58)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x58)
      Method (_PSD, 0)
      {
        return(\_SB.PSDA)
      }
      Method(_CPC,0)
      {
        return(\_SB.CPCA)
      }
    }

    //die 5 core 9
    Device (CP59)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x59)
      Method (_PSD, 0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCA)
      }
    }
  }

  Device (CL55)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 45)
    //die 5 core 10
    Device (CP5A)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x5a)
      Method (_PSD, 0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }

    //die 5 core 11
    Device (CP5B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x5b)
      Method (_PSD, 0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCB)
      }
    }
  }

  Device (CL56)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 46)
    //die 5 core 12
    Device (CP5C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x5c)
      Method (_PSD,0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCA)
      }
    }

    //die 5 core 13
    Device (CP5D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x5d)
      Method (_PSD, 0)
      {
        return (\_SB.PSDA)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCA)
      }
    }
  }

  Device (CL57)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 47)
    //die 5 core 14
    Device (CP5E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x5e)
      Method (_PSD, 0)
      {
        return (\_SB.PSDB)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }

    //die 5 core 15
    Device (CP5F)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x5f)
      Method (_PSD,0)
      {
        return (\_SB.PSDB)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCB)
      }
    }
  }
}
