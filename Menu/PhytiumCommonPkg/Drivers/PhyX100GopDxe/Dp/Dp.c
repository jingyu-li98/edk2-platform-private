/* @file
** Dp.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include "DpDrv.h"
#include "DpType.h"
#include "DpPhy.h"
#include "PhyGopDxe.h"
#include "../Dc/DcReq.h"
/**
  Convert display resolution number to pixel clock frequence in megahertz unit.

  @param[in]  modeNum    display resolution number(mode number).
                         0 - 640*480  60Hz
                         1 - 800*600p 60Hz
                         2 - 1024*768p 60Hz
                         3 - 1280*720p 60Hz
                         4 - 1366*768p_60Hz
                         5 - 1920*1080p 60Hz
                         6 - 2560*1440p 60Hz
                         7 - 2560*1600p 60Hz
                         8 - 3840*2160p 60Hz

  @retval     Pixel clock frequence in megahertz unit.
              25.2 - 640*480  60Hz
              40 - 800*600p 60Hz
              65 - 1024*768p 60Hz
              74.25 - 1280*720p 60Hz
              85.5 - 1366*768p_60Hz
              148.5  - 1920*1080p 60Hz
              311.75 - 2560*1440p 60Hz
              384.5 - 2560*1600p 60Hz
              594 - 3840*2160p 60Hz
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
    PixelClock = 311750;
    break;
  case 7:
    PixelClock = 384500;
    break;
  case 8:
    //PixelClock = 594000;
    PixelClock = 162000;
    break;
  case 9:
    PixelClock = 193250;
    break;
  case 10:
    PixelClock = 83500;
    break;
  case 11:
    PixelClock = 28232;
    break;
  case 12:
    PixelClock = 108000;
    break;
  case 13:
    PixelClock = 594000;
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

  @param[in]  num          the index of dcdp , 0 , 1 or 2.

  @param[in]  DpConfig     Pointer to dp config description structure.

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
  EFI_STATUS  Status = SINK_DISCONNECT;
  UINT8  EdidBuffer[256];
  DP_SYNC  DpSyncInfo;
  ASSERT (Num < 3);
  //if(CheckHpdStatus(Private,num) == DP_HPD_OFF){
  //  DEBUG((DEBUG_INFO,"hpd is off\n"));
  //  return DP_HPD_OFF;
  //}
  //DEBUG((DEBUG_INFO,"hpd is on\n"));
  if(Private->DpIsTrained[Num] == 0){
    InitAux (Private, Num);
    ZeroMem (EdidBuffer, sizeof(EdidBuffer));
    GetEdid (Private, Num, EdidBuffer);
    ZeroMem (&Private->DtdList[Num], sizeof (DTD_LIST));
#ifdef EDID_USE
    ParseEdidDtdList (EdidBuffer, &Private->DtdList[Num]);
#endif
    WakeUpSink (Private, Num);
    gBS->Stall (10*1000);
    Status = DpConnect (Private, Num);
    if(Status == SINK_DISCONNECT){
      //return SINK_DISCONNECT;
    }
    DptxSetTraningPattern (Private, Num, TRAINING_OFF);
    DptxDisableScrambling (Private, Num, 0);
    DptxScramblingRest (Private, Num);
  }
  ZeroMem (&DpSyncInfo, sizeof (DP_SYNC));
  if (EFI_SUCCESS == FindDpSyncFromDtd (&Private->DtdList[Num], GopMode, &DpSyncInfo, &Private->DpConfig[Num].PixelClock)) {
    DEBUG ((DEBUG_INFO, "Dp Sync Edid Dtd\n"));
  }
  else {
    DEBUG ((DEBUG_INFO, "Dp Sync VESA Default\n"));
    Private->DpConfig[Num].PixelClock = DistypeToPixelClock (GopMode->ModeName);
    CopyMem (&DpSyncInfo, &Private->DpSync[Private->DpConfig[Num].DisType], sizeof (DP_SYNC));
  }
  DcreqChangePixel (Private, Num, Private->DpConfig[Num].PixelClock);
  ConfigMainStreamAttr (Private, Num, &Private->DpConfig[Num], &DpSyncInfo);
  return Status;
}

/**
  Initialization of dp configuration parameter.

  @param[in,out]  Private      Pointer to private data structure.

  @param[in]  num          the index of dcdp , 0 , 1 or 2.

  @param[in]  gopMode      Pointer to video mode description structure.

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
  //Private->DpConfig[Num].LinkRate = 2.7;
  //Private->DpConfig[Num].LaneCount = 1;
}
