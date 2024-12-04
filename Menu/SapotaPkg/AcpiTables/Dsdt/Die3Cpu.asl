/** @file
The description of DIE 3 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die3
  Device (CL30)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 24)
    //die 3 core 0
    Device (CP30)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x30)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

    //die 3 core 1
    Device (CP31)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x31)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }
  }

  Device (CL31)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 25)
    //die 3 core 2
    Device (CP32)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x32)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }

    //die 3 core 3
    Device (CP33)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x33)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }
  }

  Device (CL32)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 26)
    //die 3 core 4
    Device (CP34)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x34)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

    //die 3 core 5
    Device (CP35)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x35)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

  }

  Device (CL33)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 27)
    //die 3 core 6
    Device (CP36)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x36)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }

    //die 3 core 7
    Device (CP37)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x37)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }
  }

  Device (CL34)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 28)
    //die 3 core 8
    Device (CP38)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x38)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

    //die 3 core 9
    Device (CP39)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x39)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }
  }

  Device (CL35)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 29)
    //die 3 core 10
    Device (CP3A)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3A)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }

    //die 3 core 11
    Device (CP3B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3B)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }
  }

  Device (CL36)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 30)
    //die 3 core 12
    Device (CP3C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3C)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

    //die 3 core 13
    Device (CP3D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3D)
      Method (_PSD, 0)
      {
        return (\_SB.PSD6)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC6)
      }
    }

  }

  Device (CL37)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 31)
    //die 3 core 14
    Device (CP3E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3E)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }

    //die 3 core 15
    Device (CP3F)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x3F)
      Method (_PSD, 0)
      {
        return (\_SB.PSD7)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC7)
      }
    }
  }
}
