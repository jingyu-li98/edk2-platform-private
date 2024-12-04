/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>

#include <Pi/PiFirmwareFile.h>
#include <Uefi/UefiBaseType.h>

#include "DcUtil.h"
/**
  Data conversion according to 128 pixels alignment.The height is constant.Width conversion according to 128 pixels alignment.

  @param[in]   width   Raw data width.
  @param[in]   height  Raw data height.
  @param[out]  size    Buffer size after data alignment calculation.
  @param[out]  srtide  Buffer stride after data alignment calculation.

  @retval      NULL
**/
VOID
CalBufferSize (
  IN  UINT32 Width,
  IN  UINT32 Height,
  OUT UINT32 *Size,
  OUT UINT32 *Stride
  )
{
  UINT32  TWidth = 0;
  if ((Width % 128) != 0) {
    TWidth = ((Width / 128) + 1) * 128;
  }
  else{
    TWidth = Width;
  }
  *Size = TWidth * Height;
  *Stride = TWidth * 4;
}

/**
  Buffer Width conversion to stride according to 128 pixels alignment.

  @param[in]  width    Raw data width.

  @retval     srtide  Buffer stride after data alignment calculation.
**/
UINT32
WidthToStride (
  IN UINT32  Width
  )
{
  UINT32  Stride;
  if ((Width % 128) == 0) {
    Stride = Width * 4;
  } else {
    Stride = ((Width / 128) * 128 + 128) * 4;
  }
  return Stride;
}
