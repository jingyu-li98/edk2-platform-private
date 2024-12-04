/** @file
The description of DIE 4 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die4
  Device (CL40)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 32)
    //die 4 core 0
    Device (CP40)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x40)

      Method(_PSD, 0)
      {
        return(\_SB.PSD8)
      }
      Method(_CPC,0)
      {
        return(\_SB.CPC8)
      }
    }

    //die 4 core 1
    Device (CP41)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x41)
      Method (_PSD,0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC8)
      }
    }
  }

  Device (CL41)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 33)
    //die 4 core 2
    Device (CP42)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x42)

      Method(_PSD,0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }

    //die 4 core 3
    Device (CP43)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x43)
      Method (_PSD,0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }
  }

  Device (CL42)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 34)
    //die 4 core 4
    Device (CP44)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x44)
      Method (_PSD, 0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC8)
      }
    }

    //die 4 core 5
    Device (CP45)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x45)
      Method (_PSD, 0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC8)
      }
    }
  }

  Device (CL43)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 35)
    //die 4 core 6
    Device (CP46)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x46)
      Method (_PSD, 0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC9)
      }
    }

    //die 4 core 7
    Device (CP47)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x47)
      Method (_PSD, 0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }
  }

  Device (CL44)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 36)
    //die 4 core 8
    Device (CP48)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x48)
      Method (_PSD, 0)
      {
        return (\_SB.PSD8)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPC8)
      }
    }

    //die 4 core 9
    Device (CP49)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x49)
      Method (_PSD, 0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC8)
      }
    }
  }

  Device (CL45)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 37)
    //die 4 core 10
    Device (CP4A)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x4a)
      Method (_PSD, 0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }

    //die 4 core 11
    Device (CP4B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x4b)
      Method (_PSD, 0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC9)
      }
    }
  }

  Device (CL46)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 38)
    //die 4 core 12
    Device (CP4C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x4c)
      Method (_PSD,0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC8)
      }
    }

    //die 4 core 13
    Device (CP4D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x4d)
      Method (_PSD, 0)
      {
        return (\_SB.PSD8)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC8)
      }
    }
  }

  Device (CL47)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 39)
    //die 4 core 14
    Device (CP4E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x4e)
      Method (_PSD, 0)
      {
        return (\_SB.PSD9)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }

    //die 4 core 15
    Device (CP4F)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x4f)
      Method (_PSD,0)
      {
        return (\_SB.PSD9)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC9)
      }
    }
  }
}
