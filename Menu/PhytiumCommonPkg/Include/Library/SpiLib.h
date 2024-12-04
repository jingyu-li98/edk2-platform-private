/** @file

  This file contains interface functions for SpiLib of PhytiumCommonPkg

  Copyright (c) 2023, ARM Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef SPI_LIB_H_
#define SPI_LIB_H_

#include <Protocol/SpiMaster.h>

VOID
EnableController (
  UINT64  SpiControllerAddress
  );


VOID
DisableController (
  UINT64  SpiControllerAddress
  );


EFI_STATUS
EFIAPI
SpiBaseTransfer (
  IN  SPI_MASTER  *SpiMaster,
  IN  SPI_DEVICE  *Slave,
  IN  UINTN       Flag
  );



#endif
