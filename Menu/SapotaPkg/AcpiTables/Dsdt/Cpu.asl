/** @file
  The description of CPU.

  Copyright (C) 2022, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

Scope(_SB)
{
  Device (CLU0) { // Cluster0 state
    Name (_HID, "ACPI0010" /* Processor Container Device */) // _HID: Hardware ID
    Name (_UID, 0)  // _UID: Unique ID

    Device (CPU0)
    {
      Name (_HID, "ACPI0007")  // _HID: Hardware ID
      Name (_UID, 0)           // _UID: Unique ID
    }
  }
}
