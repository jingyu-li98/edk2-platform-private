/** @file
  Common definitions for APEI implementation.

  Copyright (c) 2025, Sophgo Technologies Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef COMMON_H_
#define COMMON_H_

//
// Default ACPI table information
//
#define ACPI_OEM_ID           {'S','O','P','H','G','O'}
#define ACPI_OEM_TABLE_ID     SIGNATURE_64('S','G','2','0','4','4','0','0')
#define ACPI_OEM_REVISION     0x00000001
#define ACPI_CREATOR_ID       SIGNATURE_32('S','P','H','G')
#define ACPI_CREATOR_REVISION 0x00000001

#endif // COMMON_H_ 