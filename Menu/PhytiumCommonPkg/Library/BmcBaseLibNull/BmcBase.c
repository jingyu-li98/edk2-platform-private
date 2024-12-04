/** @file
This Library is mainly used to some function about bmc.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/ParameterTable.h>


/**
  Check BMC in place or not.

  @retval  EFI_NOT_FOUND  BMC is not in place.

**/
EFI_STATUS
EFIAPI
CheckBmcInPlace (
  VOID
  )
{
  return EFI_NOT_FOUND;
}

/**
  Initialized the Kcs Interface.

**/
VOID
KcsInit (
  VOID
  )
{

}
