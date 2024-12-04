/** @file
The description of DIE 6 CPU.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
Scope(_SB)
{
  //die6
  Device (CL60)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 48)
    //die 6 core 0
    Device (CP60)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x60)

      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCC)
      }
    }

    //die 6 core 1
    Device (CP61)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x61)
      Method (_PSD,0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCC)
      }
    }
  }

  Device (CL61)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 49)
    //die 6 core 2
    Device (CP62)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x62)

      Method(_PSD,0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }

    //die 6 core 3
    Device (CP63)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x63)
      Method (_PSD,0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }
  }

  Device (CL62)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 50)
    //die 6 core 4
    Device (CP64)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x64)
      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCC)
      }
    }

    //die 6 core 5
    Device (CP65)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x65)
      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCC)
      }
    }
  }

  Device (CL63)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 51)
    //die 6 core 6
    Device (CP66)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x66)
      Method (_PSD, 0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCD)
      }
    }

    //die 6 core 7
    Device (CP67)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x67)
      Method (_PSD, 0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }
  }

  Device (CL64)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 52)
    //die 6 core 8
    Device (CP68)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x68)
      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCC)
      }
    }

    //die 6 core 9
    Device (CP69)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x69)
      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCC)
      }
    }
  }

  Device (CL65)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 53)
    //die 6 core 10
    Device (CP6A)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x6a)
      Method (_PSD, 0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }

    //die 6 core 11
    Device (CP6B)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x6b)
      Method (_PSD, 0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCD)
      }
    }
  }

  Device (CL66)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 54)
    //die 6 core 12
    Device (CP6C)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x6c)
      Method (_PSD,0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCC)
      }
    }

    //die 6 core 13
    Device (CP6D)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x6d)
      Method (_PSD, 0)
      {
        return (\_SB.PSDC)
      }
      Method (_CPC, 0)
      {
        return (\_SB.CPCC)
      }
    }
  }

  Device (CL67)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 55)
    //die 6 core 14
    Device (CP6E)
    {
      Name (_HID, "ACPI0007")
      Name (_UID, 0x6e)
      Method (_PSD, 0)
      {
        return (\_SB.PSDD)
      }
      Method(_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }

    //die 6 core 15
    Device (CP6F)
    {
      Name (_HID, "ACPI0007" )
      Name (_UID, 0x6f)
      Method (_PSD,0)
      {
        return (\_SB.PSDD)
      }
      Method (_CPC,0)
      {
        return (\_SB.CPCD)
      }
    }
  }
}
