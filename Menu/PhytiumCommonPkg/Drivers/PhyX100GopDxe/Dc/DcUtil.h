/* @file
** DcUtil.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_UTIL_H_
#define    _DC_UTIL_H_

#include "DcType.h"
#include "DcStru.h"

typedef struct {
  UINT8  KernelSize;
  UINT32 ScaleFactor;
  UINT32 *KernelStates;
} FILTER_BLIT_ARRAY;

typedef struct {
  UINT32  X;
  UINT32  Y;
  UINT32  W;
  UINT32  H;
} SCALE_RECT;

VOID
CalBufferSize (
  IN  UINT32 Width,
  IN  UINT32 Height,
  OUT UINT32 *Size,
  OUT UINT32 *Stride
  );

UINT32
WidthToStride (
  IN UINT32  Width
  );
#endif
