/* @file
** Dp.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_H_
#define    _DP_H_

#include "../PhyGopDxe.h"
typedef  float    FLOAT;

EFI_STATUS
DpInit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DP_CONFIG         *DpConfig,
  IN PHY_GOP_MODE      *Gopmode
  );

VOID
DpConfigInit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_GOP_MODE      *Gopmode
  );

UINT32
DistypeToPixelClock (
  IN UINT8  ModeNum
  );
#endif
