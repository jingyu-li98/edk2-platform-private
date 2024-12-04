/* @file
** DpFun.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_FUN_H_
#define    _DP_FUN_H_

UINT8
SetMainStreamMisc0 (
  IN UINT8  BitDepth,
  IN UINT8  CFormat,
  IN UINT8  ClockMode
  );

UINT32
SetMVID (
  IN UINT32  PixelClock
  );

UINT32
SetTransUintSRC0 (
  IN UINT32 PixelClock,
  IN UINT32 LinkRate,
  IN UINT8  BitDepth,
  IN UINT32 LaneCount,
  IN UINT8  TuSize
  );

UINT32
SetNVID (
  IN UINT32  LinkRate
  );

UINT32
SetUserDataCount (
  IN UINT32  Hres,
  IN UINT8   BitDepth,
  IN UINT32  LaneCount
  );

#endif
