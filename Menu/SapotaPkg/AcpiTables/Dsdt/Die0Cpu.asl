/** @file
The description of DIE 0 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die0
  Device (CL00)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 0)
    //die 0 core 0
    Device (CP00)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0)

      Method(_PSD, 0)
      {
        return(\_SB.PSD0)
      }
      Method(_CPC,0)
      {
        return(\_SB.CPC0)
      }
    }

    //die 0 core 1
    Device (CP01)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 1)
      Method (_PSD,0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC0)
      }
    }
  }

  Device (CL01)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 1)
    //die 0 core 2
    Device (CP02)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 2)

      Method(_PSD,0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }

    //die 0 core 3
    Device (CP03)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 3)
      Method (_PSD,0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }
  }

  Device (CL02)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 2)
    //die 0 core 4
    Device (CP04)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 4)
      Method (_PSD, 0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC0)
      }
    }

    //die 0 core 5
    Device (CP05)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 5)
      Method (_PSD, 0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC0)
      }
    }
  }

  Device (CL03)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 3)
    //die 0 core 6
    Device (CP06)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 6)
      Method (_PSD, 0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC1)
      }
    }

    //die 0 core 7
    Device (CP07)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 7)
      Method (_PSD, 0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }
  }

  Device (CL04)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 4)
    //die 0 core 8
    Device (CP08)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 8)
      Method (_PSD, 0)
      {
        return (\_SB.PSD0)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPC0)
      }
    }

    //die 0 core 9
    Device (CP09)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 9)
      Method (_PSD, 0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC0)
      }
    }
  }

  Device (CL05)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 5)
    //die 0 core 10
    Device (CP0A)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x0A)
      Method (_PSD, 0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }

    //die 0 core 11
    Device (CP0B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x0B)
      Method (_PSD, 0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC1)
      }
    }
  }

  Device (CL06)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 6)
    //die 0 core 12
    Device (CP0C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x0C)
      Method (_PSD,0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC0)
      }
    }

    //die 0 core 13
    Device (CP0D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x0D)
      Method (_PSD, 0)
      {
        return (\_SB.PSD0)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC0)
      }
    }
  }

  Device (CL07)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 7)
    //die 0 core 14
    Device (CP0E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x0E)
      Method (_PSD, 0)
      {
        return (\_SB.PSD1)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }

    //die 0 core 15
    Device (CP0F)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x0F)
      Method (_PSD,0)
      {
        return (\_SB.PSD1)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC1)
      }
    }
  }
}
