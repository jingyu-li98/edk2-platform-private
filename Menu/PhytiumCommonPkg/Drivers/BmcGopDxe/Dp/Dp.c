/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include "DpType.h"
#include "PhyGopDxe.h"
#include "DpDrv.h"
#include "../Dc/DcHw.h"

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
  )
{
  UINT32  PixelClock;
  switch (ModeNum){
  case 0:
    PixelClock = 25175;
    break;
  case 1:
    PixelClock = 40000;
    break;
  case 2:
    PixelClock = 65000;
    break;
  case 3:
    PixelClock = 74250;
    break;
  case 4:
    PixelClock = 85500;
    break;
  case 5:
    PixelClock = 148500;
    break;
  case 6:
    PixelClock = 162000;
    break;
  case 7:
    PixelClock = 83500;
    break;
  case 8:
    PixelClock = 28232;
    break;
  case 9:
    PixelClock = 108000;
    break;
  default:
    PixelClock = 25175;
    break;
  }
  return PixelClock;
}

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
  IN PHY_GOP_MODE      *GopMode
  )
{
  EFI_STATUS  Status;
  UINT8       EdidBuffer[256];
  DP_SYNC     DpSyncInfo;

  ASSERT (Num < 3);
  Status = SINK_DISCONNECT;
  if(Private->DpIsTrained[Num] == 0){
    InitAux (Private, Num);
    ZeroMem (EdidBuffer, sizeof (EdidBuffer));
#ifndef PLD_TEST
    //GetEdid (Private, Num, EdidBuffer);
#endif
    ZeroMem (&Private->DtdList[Num], sizeof (DTD_LIST));
#ifdef EDID_USE
    ParseEdidDtdList (EdidBuffer, &Private->DtdList[Num]);
#endif
#ifndef PLD_TEST
    WakeUpSink (Private, Num);
    gBS->Stall (1 * 1000);
    Status = DpConnect (Private, Num);
#else
    Status = SINK_CONNECT;
#endif
    DptxSetTraningPattern (Private, Num, TRAINING_OFF);
    DptxDisableScrambling (Private, Num, 0);
    DptxScramblingRest (Private, Num);
  }
  ZeroMem (&DpSyncInfo, sizeof (DP_SYNC));
  if (EFI_SUCCESS == FindDpSyncFromDtd (&Private->DtdList[Num], GopMode, &DpSyncInfo, &Private->DpConfig[Num].PixelClock)) {
    DEBUG ((DEBUG_INFO, "Dp Sync Edid Dtd\n"));
  } else {
    DEBUG ((DEBUG_INFO, "Dp Sync VESA Default\n"));
    Private->DpConfig[Num].PixelClock = DistypeToPixelClock (GopMode->ModeName);
    CopyMem (&DpSyncInfo, &Private->DpSync[Private->DpConfig[Num].DisType], sizeof (DP_SYNC));
  }
  //
  //pld test
  //
#ifdef PLD_TEST
  ConfigDptx (Private, Num, Private->DpConfig[Num].LinkRate, Private->DpConfig[Num].LaneCount);
#endif
  DcreqChangePixel (Private, Num, Private->DpConfig[Num].PixelClock);
  //gBS->Stall (2 * 1000 * 1000);
  ConfigMainStreamAttr (Private, Num, &Private->DpConfig[Num], &DpSyncInfo);
  return Status;
}

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
  )
{
  Private->DpConfig[Num].BitDepth = Gopmode->ColorDepth/4;
  Private->DpConfig[Num].ClockMode = 1;
  Private->DpConfig[Num].ComponentFormat = 0;
  Private->DpConfig[Num].PixelClock = DistypeToPixelClock (Gopmode->ModeName);
  Private->DpConfig[Num].TuSize = 64;
  Private->DpConfig[Num].DisType = Gopmode->ModeName;
#ifdef PLD_TEST
  Private->DpConfig[Num].LinkRate = 270;
  Private->DpConfig[Num].LaneCount = 1;
#endif
}
