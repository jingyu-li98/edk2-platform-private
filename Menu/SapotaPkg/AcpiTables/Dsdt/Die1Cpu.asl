/** @file
The description of DIE 1 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope (_SB)
{
  //die1
  Device (CL10)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 8)
    //die 1 core 0
    Device (CP10)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x10)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC2)
      }
    }

    //die 1 core 1
    Device (CP11)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x11)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPC2)
      }
    }
  }

  Device (CL11)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 9)
    //die 1 core 2
    Device (CP12)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x12)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }

    //die 1 core 3
    Device (CP13)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x13)
      Method (_PSD, 0)
      {
        return(\_SB.PSD3)
      }
      Method(_CPC,0)
      {
        return(\_SB.CPC3)
      }
    }
  }

  Device (CL12)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 10)
    //die 1 core 4
    Device (CP14)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x14)

      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC2)
      }
    }

    //die 1 core 5
    Device (CP15)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x15)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC2)
      }
    }
  }

  Device (CL13)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 11)
    //die 1 core 6
    Device (CP16)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x16)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }

    //die 1 core 7
    Device (CP17)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x17)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }
  }

  Device (CL14)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 12)
    //die 1 core 8
    Device (CP18)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x18)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPC2)
      }
    }

    //die 1 core 9
    Device (CP19)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x19)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC2)
      }
    }
  }

  Device (CL15)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 13)
    //die 1 core 10
    Device (CP1A)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1A)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }

    //die 1 core 11
    Device (CP1B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1B)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }
  }

  Device (CL16)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 14)
    //die 1 core 12
    Device (CP1C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1C)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC2)
      }
    }

    //die 1 core 13
    Device (CP1D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1D)
      Method (_PSD, 0)
      {
        return (\_SB.PSD2)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC2)
      }
    }
  }

  Device (CL17)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 15)
    //die 1 core 14
    Device (CP1E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1E)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }

    //die 1 core 15
    Device (CP1F)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x1F)
      Method (_PSD, 0)
      {
        return (\_SB.PSD3)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPC3)
      }
    }
  }
}
