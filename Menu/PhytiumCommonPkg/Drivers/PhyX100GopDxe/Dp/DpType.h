/* @file
** DpType.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_TYPE_H_
#define    _DP_TYPE_H_

typedef enum {
  BIT_DEPTH_6 = 0,
  BIT_DEPTH_8,
  BIT_DEPTH_10,
  BIT_DEPTH_12,
  BIT_DEPTH_16,
  BIT_DEPTH_RESERVE
} DP_BIT_DEPTH;

typedef struct {
  UINT16  HTotal;
  UINT16  VTotal;
  BOOLEAN HPolarity;        //0 - active high , 1 - active low
  BOOLEAN VPolarity;        //0 - active high , 1 - active low
  UINT16  HsWidth;
  UINT16  VsWidth;
  UINT16  HRes;
  UINT16  VRes;
  UINT16  HStart;
  UINT16  VStart;
  BOOLEAN HUserPolarity;    //0 - active high , 1 - active low
  BOOLEAN VUserPolarity;    //0 - active high , 1 - active low
} DP_SYNC;

typedef struct {
  UINT32  LaneCount;
  UINT32   LinkRate;
  UINT8   BitDepth;
  UINT8   ComponentFormat;
  UINT8   ClockMode;      //0-async     1-sync
  UINT32   PixelClock;
  UINT8   TuSize;
  UINT8   DisType;
  UINT8   HpdStatus;      //0-off       1-on
} DP_CONFIG;

typedef enum {
  D0_MODE = 1,
  D3_MODE = 2,
} PHY_PWR_MODE;

typedef enum {
  TRAINING_OFF = 0,
  TRAINING_PATTERN_1,
  TRAINING_PATTERN_2,
  TRAINING_PATTERN_3
} TRAINING_PATTERN;
#endif
