/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_H_
#define    _DP_H_

#include "PhyGopDxe.h"

/**
  Initialization of dp hardware.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  DpConfig     Pointer to dp config description structure.
  @param[in]  GopMode      Pointer to video mode description structure

  @retval     SINK_DISCONNECT  Sink disconnected
  @retval     SINK_CONNECT     Sink connected.
**/
EFI_STATUS
DpInit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DP_CONFIG         *DpConfig,
  IN PHY_GOP_MODE      *Gopmode
  );

/**
  Initialization of dp configuration parameter.

  @param[in,out]  Private      Pointer to private data structure.
  @param[in]      Num          the index of dcdp , 0 or 1.
  @param[in]      GopMode      Pointer to video mode description structure.

  @retval     NULL
**/
VOID
DpConfigInit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_GOP_MODE      *Gopmode
  );

/**
  Convert display resolution number to pixel clock frequence in megahertz unit.

  @param[in]  modeNum    display resolution number(mode number).
                         0 - 640*480  60Hz
                         1 - 800*600p 60Hz
                         2 - 1024*768p 60Hz
                         3 - 1280*720p 60Hz
                         4 - 1366*768p 60Hz
                         5 - 1920*1080p 60Hz
                         6 - 1600*1200p 60Hz
                         7 - 1280*800p 60Hz
                         8 - 800*480p 60Hz
                         9 - 1280*1024p 60Hz

  @retval     Pixel clock frequence in KHz unit.
              25175 - 640*480  60Hz
              40000 - 800*600p 60Hz
              65000 - 1024*768p 60Hz
              74250 - 1280*720p 60Hz
              85500 - 1366*768p 60Hz
              148500 - 1920*1080p 60Hz
              162000 - 1600*1200p 60Hz
              83500 - 1280*800p 60Hz
              28232 - 800*480p 60Hz
              108000 - 1280*1024p 60Hz
**/
UINT32
DistypeToPixelClock (
  IN UINT8  ModeNum
  );
#endif
