/* @file
** PhyGraphicsOutput.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/GraphicsOutput.h>

#include "Dc/DcCore.h"
#include "Dc/DcReq.h"
#include "Dc/DcUtil.h"
#include "Dp/Dp.h"
#include "Dp/DpDrv.h"
#include "Dc/DcHw.h"
#include "Dp/DpPhy.h"
#include "PhyGopDxe.h"

EFI_GUID  mEdpLightValueVar = EDP_LIGHT_VARIABLE_GUID;

DC_SYNC  mDcSyncTable[PHY_GOP_MAX_MODENUM] = {
  {800 , 640 , 656 , 752 , 0 , 525 , 480 , 490 , 492 , 0 },           //640 * 480  60Hz
  {1056 , 800 , 840 , 968 , 1 , 628 , 600 , 601 , 605 , 1},           //800*600p 60Hz
  {1344 , 1024 , 1048 , 1184 , 0 , 806 , 768 , 771 , 777 , 0},        //1024*768p 60Hz
  {1650 , 1280 , 1390 , 1430 , 1 , 750 , 720 , 725 , 730 , 1 },       //1280*720p 60Hz
  {1792 , 1366 , 1436 , 1579 , 1 , 798 , 768 , 771 , 774 , 1},        //1366*768p_60Hz
  {2200 , 1920 , 2008 , 2052 , 1 , 1125 , 1080 , 1084 , 1089 , 1 },   //1920*1080p 60Hz
  {3488 , 2560 , 2744 , 3024 , 0 , 1490 , 1440 , 1441 , 1444 , 1 },   //2560*1440p_60Hz
  {3504 , 2560 , 2752 , 3032 , 0 , 1658 , 1600 , 1603 , 1609 , 1 },   //2560*1600p_60Hz
  {2160, 1600, 1664, 1856, 1, 1250, 1200, 1201, 1204, 1 },            //1600*1200 60Hz
  {2592, 1920, 2056, 2256, 0, 1245, 1200, 1203, 1209, 1 },            //1290x1200 60Hz
  {1680, 1280, 1352, 1480, 0, 831, 800, 803, 809, 1 },                //1280x800 60Hz
  {928, 800, 840, 888, 1, 525, 490, 493, 496, 1 },                    //800x480 60Hz
  {1688, 1280, 1328, 1440, 1, 1066, 1024, 1025, 1028, 1},              //1280x1024 60Hz
  { 4400 , 3840 , 4016 , 4104 , 1 , 2250 , 2160 , 2168 , 2178 , 1 }    //3840*2160p_60Hz
};

DP_SYNC mDpSyncTable[PHY_GOP_MAX_MODENUM] = {
  {800 , 525 , 1 , 1 , 96 , 2 , 640 , 480 , 144 , 35 , 0 , 0 },       //640*480 60Hz
  {1056 , 628 , 0 , 0 , 128 , 4 , 800 , 600 , 216 , 27 , 1 , 1 },     //800*600p 60Hz
  {1344 , 806 , 1 , 1 , 136 , 6 , 1024 , 768 , 296 , 35 , 0 , 0 },    //1024*768p 60Hz
  {1650 , 750 , 0 , 0 , 40 , 5 , 1280 , 720 , 260 , 25 , 1 , 1 },     //1280*720 60Hz
  {1792 , 798 , 0 , 0 , 143 , 3 , 1366 , 768 , 356 , 27 , 1 , 1 },    //1366*768p_60Hz
  {2200 , 1125 , 0 , 0 , 44 , 5 , 1920 , 1080 , 192 , 41 , 1 , 1},    //1920*1080p 60Hz
  {3488 , 1490 , 1 , 0 , 280 , 3 , 2560 , 1440 , 184 , 1 , 0 , 1 },   //2560*1440p_60Hz
  {3504 , 1490 , 1 , 0 , 280 , 6 , 2560 , 1600 , 192 , 3 , 0 , 1 },   //2560*1600p_60Hz
  {2160, 1250, 0, 0, 192, 3, 1600, 1200, 496, 49, 1, 1},              //1600x1200 60Hz
  {2592, 1245, 1, 0, 200, 6, 1920, 1200, 536, 42, 0, 1},              //1920x1200 60Hz
  {1680, 831, 1, 0, 128, 6, 1280, 800, 328, 28, 0, 1 },               //1280x800 60Hz
  {928, 528, 0, 0, 48, 3, 800, 480, 88, 32, 1, 1},                    //800x480 60Hz
  {1688, 1066, 0, 0, 112, 3, 1280, 1024, 360, 41, 1, 1},               //1280x1024 60Hz
  { 4400 , 2250 , 0 , 0 , 88 , 10 , 3840 , 2160 , 176 , 8 , 1 , 1 }    //3840*2160p_60Hz
};

PHY_GOP_MODE mPhyGopMode[PHY_GOP_MAX_MODENUM] = {
  {640, 480, 32, 60, PHY_GOP_640_480},
  {800, 600, 32, 60, PHY_GOP_800_600},
  {1024, 768, 32, 60, PHY_GOP_1024_768},
  {1280, 720, 32, 60, PHY_GOP_1280_720},
  {1366, 768, 32, 60, PHY_GOP_1366_768},
  {1920, 1080, 32, 60, PHY_GOP_1920_1080},
  {2560, 1440, 32, 60, PHY_GOP_2560_1440},
  {2560, 1600, 32, 60, PHY_GOP_2560_1600},
  {1600, 1200, 32, 60, PHY_GOP_1600_1200},
  {1920, 1200, 32, 60, PHY_GOP_1920_1200},
  {1280, 800, 32, 60, PHY_GOP_1280_800},
  {800, 480, 32, 60, PHY_GOP_800_480},
  {1280, 1024, 32, 60, PHY_GOP_1280_1024},
  {3840, 2160, 32, 60, PHY_GOP_3840_2160}
};

/**
  Get mode Number by width and height

  @param[in]  width   Width of resolution.

  @param[in]  height  Height of resolution.

  @retval     0       PHY_GOP_640_480

  @retval     1       PHY_GOP_800_600

  @retval     2       PHY_GOP_1024_768

  @retval     3       PHY_GOP_1280_720

  @retval     4       PHY_GOP_1366_768

  @retval     5       PHY_GOP_1920_1080

  @retval     6       PHY_GOP_2560_1440

  @retval     7       PHY_GOP_2560_1600

  @retval     8       PHY_GOP_3840_2160
**/
UINT8
ResToModeNum (
  IN UINT32  Width,
  IN UINT32  Height
  )
{
  UINT8  Value;
  Value = PHY_GOP_640_480;
  if ((Width == 640) && (Height == 480)) {
    Value = PHY_GOP_640_480;
  }
  else if ((Width == 800) && (Height == 600)) {
    Value = PHY_GOP_800_600;
  }
  else if ((Width == 1024) && (Height == 768)) {
    Value = PHY_GOP_1024_768;
  }
  else if ((Width == 1280) && (Height == 720)) {
    Value = PHY_GOP_1280_720;
  }
  else if ((Width == 1366) && (Height == 768)) {
    Value = PHY_GOP_1366_768;
  }
  else if ((Width == 1920) && (Height == 1080)) {
    Value = PHY_GOP_1920_1080;
  }
  else if ((Width == 2560) && (Height == 1440)) {
    Value = PHY_GOP_2560_1440;
  }
  else if ((Width == 2560) && (Height == 1600)) {
    Value = PHY_GOP_2560_1600;
  }
  else if ((Width == 3840) && (Height == 2160)) {
    return PHY_GOP_3840_2160;
  }
  else if ((Width == 1600) && (Height == 1200)) {
    return PHY_GOP_1600_1200;
  }
  else if ((Width == 1920) && (Height == 1200)) {
    return PHY_GOP_1920_1200;
  }
  else if ((Width == 1280) && (Height == 800)) {
    return PHY_GOP_1280_800;
  }
  else if ((Width == 1280) && (Height == 1024)) {
    return PHY_GOP_1280_1024;
  }
  else if ((Width == 800) && (Height == 480)) {
    return PHY_GOP_800_480;
  }
  return Value;
}

/**
  Fill a specified pixel value to rectangular area.

  @param[in]  Private  Pointer to private data structure

  @param[in]  width   Width of rectangular area.

  @param[in]  height  Height of rectangular area.

  @param[in]  blt     The pixel to be filled

  @retval     NULL
**/
VOID
VideoFillFramebuffer (
  IN PHY_PRIVATE_DATA             *Private,
  IN UINT32                        Width,
  IN UINT32                        Height,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL Blt
  )
{
  UINT32  Stride;
  UINT32  I;
  UINT32  J;
  Stride = WidthToStride (Width);
  //DEBUG((DEBUG_INFO,"fill width : %d , height : %d\n",width,height));
  for (I = 0; I < Height; I++) {
    for (J = 0; J < Stride / 4; J++) {
      PhyFramebufferWrite (Private, I * Stride + J * 4, 1, &Blt);
    }
  }
}

/**
 *
**/
VOID
EdpLightGetFromVariable (
  OUT UINT8  *Light
  )
{
  EFI_STATUS  Status;
  UINT8       Value;
  UINTN       VarSize;

  Value = EDP_LIGHT_DEFAULT;
  VarSize = sizeof (UINT8);
  Status = EFI_NOT_FOUND;
  Status = gRT->GetVariable (
                  EDP_LIGHT_VARIABLE,
                  &mEdpLightValueVar,
                  NULL,
                  &VarSize,
                  &Value
                  );
  DEBUG ((DEBUG_INFO, "Edp Light Get : %r\n", Status));
  if (Status != EFI_SUCCESS) {
    Value = EDP_LIGHT_DEFAULT;
  }
  DEBUG ((DEBUG_INFO, "Edp Light : %d\n", Value));
  *Light = Value;
}

/**
  Hardware initialization and modeset code.At last, fill black to full screen.

  @param[in]  Private  Pointer to private data structure.

  @param[in]  gopMode  Pointer to mode description structure.

  @retval     EFI_SUCCESS  Initialize successfully.

  @retval     other        Initialize failed.
**/
EFI_STATUS
InitializeGraphicsMode(
  IN PHY_PRIVATE_DATA  *Private,
  IN PHY_GOP_MODE      *GopMode
  )
{
  DEBUG ((DEBUG_INFO, "%a(),%d\n", __FUNCTION__, __LINE__));
  //EFI_STATUS  Status;
  UINT32  DpNum;
  //UINT32   PixelClock;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  Blt;
  DpNum = 0;
  //clear to black
  Blt.Blue = 0;
  Blt.Green = 0;
  Blt.Red = 0x0;
  Blt.Reserved = 0;
  VideoFillFramebuffer (Private, GopMode->Width, GopMode->Height, Blt);
  for (DpNum = 0; DpNum < DPDC_PATH_NUM; DpNum++) {
    if (Private->DpIsUsed[DpNum] == 0){
      continue;
    }
    if ((DpNum == 2) && (IsPhy2Dp (Private) == 0)){
      continue;
    }
    //
    //if Edp, control backlight
    //
    //if (1 == Private->IsEdp[DpNum]) {
    //  if (1 == Private->EdpNeedInit[DpNum]) {
    //    if (EFI_SUCCESS != EdpBgLightControl (Private, DpNum, 1, 50)) {
    //      Private->EdpNeedInit[DpNum] = 0;
    //    }
    //  }
    //}
    //
    //If edp back light is on ,must turn off.it.
    //
    if (1 == Private->IsEdp[DpNum]) {
      if (Private->EdpBgLightOn[DpNum] == 1) {
        if (EFI_SUCCESS == EdpBgLightControl (Private, DpNum, 0, 0)) {
          Private->EdpBgLightOn[DpNum] = 0;
          DEBUG ((DEBUG_INFO, "Edp Light Off\n"));
        }
      }
    }
    HwFramebufferReset (Private, DpNum);
    ZeroMem (&Blt, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    //PixelClock = DistypeToPixelClock (GopMode->ModeName);
    //DcreqChangePixel (Private, DpNum, PixelClock);
    DpConfigInit (Private, DpNum, GopMode);
    if (CheckHpdStatus (Private, DpNum) == DP_HPD_OFF) {
      DEBUG((DEBUG_INFO,"hpd is off\n"));
      Private->DpIsHpdOn[DpNum] = DP_HPD_OFF;
    }
    else{
      DEBUG((DEBUG_INFO,"hpd is on\n"));
      Private->DpIsHpdOn[DpNum] = DP_HPD_ON;
      Private->DpConnect[DpNum] = DpInit (Private, DpNum, &Private->DpConfig[DpNum], GopMode);
    }
    DcConfigInit (Private, DpNum, GopMode);
    //Private->DpConnect[DpNum] = DpInit (Private, DpNum, &Private->DpConfig[DpNum]);
    //DcreqChangePixel (Private, DpNum, PixelClock);
    CoreCommit (Private, DpNum);
    if (1 == Private->IsEdp[DpNum]) {
      if (Private->EdpBgLightOn[DpNum] == 0) {
        if (EFI_SUCCESS == EdpBgLightControl (Private, DpNum, 1, Private->EdpLight)) {
          Private->EdpBgLightOn[DpNum] = 1;
          DEBUG ((DEBUG_INFO, "Edp Light on\n"));
        }
      }
    }
    //gBS->Stall (100*1000);
  }
  return EFI_SUCCESS;
}

/**
  Performs a UEFI Graphics Output Protocol Blt Video to Buffer operation
  with extended parameters.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @param[out] BltBuffer     Output buffer for pixel color data

  @param[in]  SourceX       X location within video

  @param[in]  SourceY       Y location within video

  @param[in]  DestinationX  X location within BltBuffer

  @param[in]  DestinationY  Y location within BltBuffer

  @param[in]  Width         Width (in pixels)

  @param[in]  Height        Height

  @param[in]  Delta         Number of bytes in a row of BltBuffer

  @retval  EFI_DEVICE_ERROR A hardware error occured

  @retval  EFI_INVALID_PARAMETER  Invalid parameter passed in

  @retval  EFI_SUCCESS  Success.
**/
EFI_STATUS
BltVideoToBltBuffer (
  IN  PHY_PRIVATE_DATA                      *Private,
  IN  UINT8                                 Num,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height,
  IN  UINTN                                 Delta
  )
{
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;
  UINT32  ResWidth;
  UINT32  ResHeight;
  UINT32  PixelWidth;
  UINT32  Stride;
  UINT32  SrcY;
  UINT32  DstY;
  UINTN   Offset;
  ASSERT (Num < 3);
  ResWidth = Private->ModeData[Private->ModeNum].Width;
  ResHeight = Private->ModeData[Private->ModeNum].Height;
  Stride = WidthToStride (ResWidth);
  PixelWidth = Private->ModeData[Private->ModeNum].ColorDepth / 8;
  //DEBUG((DEBUG_INFO,"Private->ModeNum : %d\n",Private->ModeNum));
  //DEBUG((DEBUG_INFO,"resWidth : %d , resHeight : %d,SourceX : %d,SourceY : %d,DestinationX:%d,DestinationY:%d\n",
    //ResWidth,ResHeight,SourceX,SourceY,DestinationX,DestinationY));
  if (DestinationY + Height > ResHeight) {
    DEBUG ((DEBUG_ERROR, "VideotoBuffer: Past screen (Y)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (DestinationX + Width > ResWidth) {
    DEBUG ((DEBUG_ERROR, "VideotoBuffer: Past screen (X)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((Width == 0) || (Height == 0)) {
    DEBUG ((DEBUG_ERROR, "VideotoBuffer: Width or Height is 0\n"));
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the Number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the Number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  for (SrcY = SourceY, DstY = DestinationY; DstY < (Height + DestinationY); SrcY++, DstY++) {
      Offset = SrcY * Stride + SourceX * PixelWidth;
      PhyFramebufferRead (Private, Offset, Width, (UINT8 *) (BltBuffer) + Delta * DstY + DestinationX * PixelWidth);
  }
  return Status;
}

/**
  Performs a UEFI Graphics Output Protocol Blt Video to Video operation

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @param[in]  SourceX       X location within video

  @param[in]  SourceY       Y location within video

  @param[in]  DestinationX  X location within video

  @param[in]  DestinationY  Y location within video

  @param[in]  Width         Width (in pixels)

  @param[in]  Height        Height

  @retval  EFI_DEVICE_ERROR  A hardware error occured

  @retval  EFI_INVALID_PARAMETER  Invalid parameter passed in

  @retval  EFI_SUCCESS  Success.
**/
EFI_STATUS
BltVideoToVideo (
  IN  PHY_PRIVATE_DATA                      *Private,
  IN  UINT8                                 Num,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height
  )
{
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;
  UINT32  ResWidth;
  UINT32  ResHeight;
  UINT32  PixelWidth;
  UINT32  Stride;
  UINT32  SrcY;
  UINT32  DstY;
  UINTN   Offset;
 // DEBUG((DEBUG_INFO,"Width:%d,Height:%d,SourceX : %d,SourceY : %d,DestinationX:%d,DestinationY:%d\n",Width,Height,SourceX,SourceY,DestinationX,DestinationY));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Blt;
  ASSERT (Num < 3);
  ResWidth = Private->ModeData[Private->ModeNum].Width;
  ResHeight = Private->ModeData[Private->ModeNum].Height;
  Stride = WidthToStride (ResWidth);
  PixelWidth = Private->ModeData[Private->ModeNum].ColorDepth / 8;
  if (DestinationY + Height > ResHeight) {
    DEBUG ((DEBUG_ERROR, "VideotoVedeo: Past screen (Y)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (DestinationX + Width > ResWidth) {
    DEBUG ((DEBUG_ERROR, "VideotoVedeo: Past screen (X)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    DEBUG ((DEBUG_ERROR, "VideotoBuffer: Width or Height is 0\n"));
    return EFI_INVALID_PARAMETER;
  }
  Blt = AllocatePool(Width * Height * PixelWidth);
  if(Blt == NULL){
    DEBUG((DEBUG_INFO,"VideotoVedeo blt alloc failed\n"));
    return EFI_BUFFER_TOO_SMALL;
  }
  //
  //video to buffer
  //
  for (SrcY = SourceY, DstY = 0; SrcY < (Height + SourceY); SrcY++, DstY++) {
    Offset = SrcY * Stride + SourceX * PixelWidth;
    //DEBUG((DEBUG_INFO,"SrcY:%d,Offset:%d,Width:%d\n",SrcY,Offset,Width));
    PhyFramebufferRead (Private, Offset, Width, (UINT8 *) (Blt) + Width * DstY * PixelWidth);
  }
  //
  //buffer to video
  //
  for (SrcY = 0, DstY = DestinationY; SrcY < (Height); SrcY++, DstY++) {
    Offset = DstY * Stride + DestinationX * PixelWidth;
    //DEBUG((DEBUG_INFO,"SrcY:%d,Offset:%d,Width:%d\n",SrcY,Offset,Width));
    PhyFramebufferWrite (Private, Offset, Width, (UINT8 *) (Blt) + Width * SrcY * PixelWidth);
  }
  FreePool(Blt);
  return Status;
}

/**
  Performs a UEFI Graphics Output Protocol Blt Video Fill.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @param[in]  Color         Color to fill the region with

  @param[in]  DestinationX  X location to start fill operation

  @param[in]  DestinationY  Y location to start fill operation

  @param[in]  Width         Width (in pixels) to fill

  @param[in]  Height        Height to fill

  @retval  EFI_DEVICE_ERROR  A hardware error occured

  @retval  EFI_INVALID_PARAMETER  Invalid parameter passed in

  @retval  EFI_SUCCESS  Success.
**/
EFI_STATUS
BltVideoToFill (
  IN PHY_PRIVATE_DATA                      *Private,
  IN UINT8                                 Num,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Color,
  IN UINTN                                 DestinationX,
  IN UINTN                                 DestinationY,
  IN UINTN                                 Width,
  IN UINTN                                 Height
  )
{
  UINT32  ResWidth;
  UINT32  ResHeight;
  UINT32  Stride;
  UINT32  I;
  UINT32  J;
  UINT32  Blt;
  ASSERT (Num < 3);
  ResWidth = Private->ModeData[Private->ModeNum].Width;
  ResHeight = Private->ModeData[Private->ModeNum].Height;
  Stride = WidthToStride (ResWidth);
  CopyMem (&Blt, Color, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  //DEBUG((DEBUG_INFO,"desy : %d,height:%d,resheight:%d\n",DestinationY,Height,resHeight));
  if (DestinationY + Height > ResHeight) {
    DEBUG ((DEBUG_ERROR, "VideoFill: Past screen (Y)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (DestinationX + Width > ResWidth) {
    DEBUG ((DEBUG_ERROR, "VideoFill: Past screen (X)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    DEBUG ((DEBUG_ERROR, "VideoFill: Width or Height is 0\n"));
    return EFI_INVALID_PARAMETER;
  }
  for (I = DestinationY; I < (DestinationY + Height); I++) {
    for (J = DestinationX; J < (DestinationX + Width); J++) {
      PhyFramebufferWrite (Private, I * Stride + J * 4, 1, &Blt);
    }
  }
  return EFI_SUCCESS;
}

/**
  Performs a UEFI Graphics Output Protocol Blt Buffer to Video operation
  with extended parameters.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @param[in]  BltBuffer     Output buffer for pixel color data

  @param[in]  SourceX       X location within BltBuffer

  @param[in]  SourceY       Y location within BltBuffer

  @param[in]  DestinationX  X location within video

  @param[in]  DestinationY  Y location within video

  @param[in]  Width         Width (in pixels)

  @param[in]  Height        Height

  @param[in]  Delta         Number of bytes in a row of BltBuffer

  @retval  EFI_DEVICE_ERROR  A hardware error occured

  @retval  EFI_INVALID_PARAMETER  Invalid parameter passed in

  @retval  EFI_SUCCESS  The sizes were returned
**/
EFI_STATUS
BltBufferToVideo (
  IN PHY_PRIVATE_DATA                       *Private,
  IN UINT8                                   Num,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height,
  IN  UINTN                                 Delta
  )
{
  UINT32 Status;
  ASSERT (Num < 3);
  UINT32  ResWidth;
  UINT32  ResHeight;
  UINT32  Stride;
  UINT32  SrcY;
  UINT32  DstY;
  UINT32  Offset;
  UINT32  PixelWidth;
  Status = EFI_SUCCESS;
  ASSERT (Num < 3);
  ResWidth = Private->ModeData[Private->ModeNum].Width;
  ResHeight = Private->ModeData[Private->ModeNum].Height;
  PixelWidth = Private->ModeData[Private->ModeNum].ColorDepth / 8;
  Stride = WidthToStride (ResWidth);
  //DEBUG((DEBUG_INFO,"Private->ModeNum : %d\n",Private->ModeNum));
  //DEBUG((DEBUG_INFO,"resWidth : %d , resHeight : %d, stride : %d,SourceX : %d,SourceY : %d,DestinationX:%d,DestinationY:%d\n",
    //resWidth,resHeight,stride,SourceX,SourceY,DestinationX,DestinationY));
  if (DestinationY + Height > ResHeight) {
    DEBUG ((DEBUG_ERROR, "BufferToVideo: Past screen (Y)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (DestinationX + Width > ResWidth) {
    DEBUG ((DEBUG_ERROR, "BufferToVideo: Past screen (X)\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    DEBUG ((DEBUG_ERROR, "BufferToVideo: Width or Height is 0\n"));
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the Number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the Number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  for (SrcY = SourceY, DstY = DestinationY; SrcY < (Height + SourceY); SrcY++, DstY++) {
      Offset = DstY * Stride + DestinationX * PixelWidth;
      PhyFramebufferWrite (Private, Offset, Width, (UINT8 *) (BltBuffer) + Delta * SrcY + SourceX * PixelWidth);
  }
  return Status;
}
/**
  Graphics Output protocol interface to query video mode

  @param[in]  This        Protocol instance pointer.

  @param[in]  ModeNumber  The mode Number to return information on.

  @param[in]  Info        Caller allocated buffer that returns information about ModeNumber.

  @param[in]  SizeOfInfo  A pointer to the size, in bytes, of the Info buffer.

  @retval     EFI_SUCCESS           Success.

  @retval     EFI_BUFFER_TOO_SMALL  The Info buffer was too small.

  @retval     EFI_DEVICE_ERROR      A hardware error occurred trying to retrieve the video mode.

  @retval     EFI_NOT_STARTED       Video display is not initialized. Call SetMode ()

  @retval     EFI_INVALID_PARAMETER One of the input args was NULL.
**/
EFI_STATUS
EFIAPI
PhyGraphicsOutputQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
{
  PHY_PRIVATE_DATA  *Private;

  Private = PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);

  if (Private->HardwareNeedsStarting) {
    return EFI_NOT_STARTED;
  }

  if (Info == NULL || SizeOfInfo == NULL || ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  *Info = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

  if (*Info == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  (*Info)->Version = 1000;
  (*Info)->HorizontalResolution = Private->ModeData[ModeNumber].Width;
  (*Info)->VerticalResolution   = Private->ModeData[ModeNumber].Height;
  //(*Info)->PixelFormat = PixelBltOnly;
  (*Info)->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  (*Info)->PixelsPerScanLine = (*Info)->HorizontalResolution;
  return EFI_SUCCESS;
}

/**
  Graphics Output protocol interface to set video mode

  @param[in]  This        Protocol instance pointer.

  @param[in]  ModeNumber  The mode Number to be set.

  @retval  EFI_SUCCESS        Graphics mode was changed.

  @retval  EFI_DEVICE_ERROR   The device had an error and could not complete the request.

  @retval  EFI_UNSUPPORTED    ModeNumber is not supported by this device.
**/
EFI_STATUS
EFIAPI
PhyGraphicsOutputSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
{
  PHY_PRIVATE_DATA  *Private;
  PHY_GOP_MODE       *ModeData;
  DEBUG((DEBUG_INFO,"SetMode --- %d\n",ModeNumber));
  DEBUG ((DEBUG_INFO, "%a(),%d\n", __FUNCTION__, __LINE__));
  Private = PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  //if (ModeNumber == This->Mode->Mode) {
  //   return EFI_SUCCESS;
  //}

  ModeData = &Private->ModeData[ModeNumber];
  if (Private->LineBuffer) {
    gBS->FreePool (Private->LineBuffer);
  }

  Private->LineBuffer = NULL;
  Private->LineBuffer = AllocatePool (Private->ModeData[ModeNumber].Width);
  if (Private->LineBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeGraphicsMode (Private, &mPhyGopMode[ModeNumber]);
  This->Mode->Mode = ModeNumber;
  This->Mode->Info->HorizontalResolution = ModeData->Width;
  This->Mode->Info->VerticalResolution  = ModeData->Height;
  //This->Mode->Info->PixelFormat         = PixelBltOnly;
  This->Mode->Info->PixelFormat         = PixelBlueGreenRedReserved8BitPerColor;
  This->Mode->Info->PixelsPerScanLine   = ModeData->Width;
  This->Mode->SizeOfInfo                = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  Private->ModeNum = ModeNumber;

  This->Mode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS)Private->BufferAddr;
  This->Mode->FrameBufferSize = ModeData->Width * ModeData->Height * 4;
  Private->HardwareNeedsStarting  = FALSE;
  return EFI_SUCCESS;
}

/**
  Graphics Output protocol instance to block transfer for CirrusLogic device

  @param[in]  This          Pointer to Graphics Output protocol instance

  @param[in]  BltBuffer     The data to transfer to screen

  @param[in]  BltOperation  The operation to perform

  @param[in]  SourceX       The X coordinate of the source for BltOperation

  @param[in]  SourceY       The Y coordinate of the source for BltOperation

  @param[in]  DestinationX  The X coordinate of the destination for BltOperation

  @param[in]  DestinationY  The Y coordinate of the destination for BltOperation

  @param[in]  Width         The width of a rectangle in the blt rectangle in pixels

  @param[in]  Height        The height of a rectangle in the blt rectangle in pixels

  @param[in]  Delta         Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                            If a Delta of 0 is used, the entire BltBuffer will be operated on.
                            If a subrectangle of the BltBuffer is used, then Delta represents
                            the Number of bytes in a row of the BltBuffer.

  @retval     EFI_INVALID_PARAMETER  Invalid parameter passed in

  @retval     EFI_SUCCESS            Blt operation success
**/
EFI_STATUS
EFIAPI
PhyGraphicsOutputBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION     BltOperation,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height,
  IN  UINTN                                 Delta
  )
{

  PHY_PRIVATE_DATA                  *Private;
  EFI_TPL                           OriginalTPL;
  EFI_STATUS                        Status;
  Private = PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (This);
  //DEBUG((DEBUG_INFO,"BltBuffer : %p\n",BltBuffer));
  if ((BltOperation < 0) || (BltOperation >= EfiGraphicsOutputBltOperationMax)) {
      return EFI_INVALID_PARAMETER;
  }
  if (Width == 0 || Height == 0) {
      return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the Number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the Number of bytes in each row can be computed.
  //
  if (Delta == 0) {
      Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }
  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  //DEBUG((DEBUG_INFO,"Private->ModeNum : %d\n",Private->ModeNum));
  //DEBUG((DEBUG_INFO,"Width:%d,Height:%d,SourceX : %d,SourceY : %d,DestinationX:%d,DestinationY:%d\n",Width,Height,SourceX,SourceY,DestinationX,DestinationY));
  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);
  switch (BltOperation) {
  case EfiBltVideoToBltBuffer:
    Status = BltVideoToBltBuffer (Private, 0, BltBuffer, SourceX, SourceY, DestinationX, DestinationY, Width, Height, Delta);
    break;
  case EfiBltVideoToVideo:
    Status = BltVideoToVideo (Private, 0, SourceX, SourceY, DestinationX, DestinationY, Width, Height);
    break;
  case EfiBltVideoFill:
    Status = BltVideoToFill (Private, 0, BltBuffer, DestinationX, DestinationY, Width, Height);
    break;
  case EfiBltBufferToVideo:
    Status = BltBufferToVideo (Private, 0, BltBuffer, SourceX, SourceY, DestinationX, DestinationY, Width, Height, Delta);
    break;
  default:
    Status = EFI_INVALID_PARAMETER ;
    break;
  }
  gBS->RestoreTPL (OriginalTPL);
  return Status;
}

VOID
EFIAPI
DpSinkHotPlugCheck (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  )
{
  PHY_PRIVATE_DATA  *This;
  UINT32            Index;
  UINT32            HpdStatus;
  //UINT32            Status;
  UINT32            Irq;
  This = (PHY_PRIVATE_DATA *)Context;
  for (Index = 0; Index < DPDC_PATH_NUM; Index++){
    //DEBUG ((DEBUG_INFO, "hot plug %d\n", Index));
    if (This->DpIsUsed[Index] == 0){
      continue;
    }
    if ((Index == 2) && (IsPhy2Dp (This) == 0)){
      continue;
    }
    HpdStatus = GetHpdStatusOnce (This, Index);
    if (HpdStatus == DP_HPD_ON) {
        if (This->DpIsHpdOn[Index] == DP_HPD_OFF) {
          if (1 == This->IsEdp[Index]) {
            if (This->EdpBgLightOn[Index] == 1) {
              if (EFI_SUCCESS == EdpBgLightControl (This, Index, 0, 0)) {
                This->EdpBgLightOn[Index] = 0;
                DEBUG ((DEBUG_INFO, "Edp Light Off\n"));
              }
            }
          }
          This->DpIsHpdOn[Index] = DP_HPD_ON;
          HwFramebufferReset (This, Index);
          This->DpIsTrained[Index] = 0;
          DpConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
          This->DpConnect[Index] = DpInit (This, Index, &This->DpConfig[Index], &mPhyGopMode[This->ModeNum]);
          DcConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
          CoreCommit (This, Index);
#if 0
          InitAux (This,Index);
          ZeroMem (EdidBuffer, sizeof(EdidBuffer));
          GetEdid (Private, Num, EdidBuffer);
          DpConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
          Status = DpConnect(This,Index);
          DptxSetTraningPattern (This,Index,TRAINING_OFF);
          SinkDpcdWrite (This,Index,0x102,0x0);
          DptxDisableScrambling (This,Index,0);
          DptxScramblingRest (This,Index);
          ConfigMainStreamAttr (This,Index,&This->DpConfig[Index],&This->DpSync[This->ModeNum]);
#endif
          if(This->DpConnect[Index] == SINK_CONNECT) {
            if (1 == This->IsEdp[Index]) {
              if (This->EdpBgLightOn[Index] == 0) {
                if (EFI_SUCCESS == EdpBgLightControl (This, Index, 1, This->EdpLight)) {
                  This->EdpBgLightOn[Index] = 1;
                  DEBUG ((DEBUG_INFO, "Edp Light on\n"));
                }
              }
            }
            DEBUG((DEBUG_INFO,"hot plug connect\n"));
          }
          else {
            DEBUG((DEBUG_INFO,"hot plug disconnect\n"));
          }
        }
        else {
          This->DpIsHpdOn[Index] = DP_HPD_ON;
        }
    }
    else {
      This->DpIsHpdOn[Index] = DP_HPD_OFF;
    }
    Irq = PhyReadMmio (This, Index, 0x3130);
    //PhyReadMmio (This, Index, 0x3140);
    if (((Irq >> 1)&0x1) == 0x1) {
      //
      //clear int state
      //
      PhyReadMmio (This, Index, 0x3140);
      DEBUG((DEBUG_INFO, "IRQ_HPD !!!!!  Index : %d\n",Index));
      if (SinkNeedReTrain (This, Index) == 1) {
        if (1 == This->IsEdp[Index]) {
          if (This->EdpBgLightOn[Index] == 1) {
            if (EFI_SUCCESS == EdpBgLightControl (This, Index, 0, 0)) {
              This->EdpBgLightOn[Index] = 0;
              DEBUG ((DEBUG_INFO, "Edp Light Off\n"));
            }
          }
        }
        HwFramebufferReset (This, Index);
        This->DpIsTrained[Index] = 0;
        DpConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
        This->DpConnect[Index] = DpInit (This, Index, &This->DpConfig[Index], &mPhyGopMode[This->ModeNum]);
        DcConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
        CoreCommit (This, Index);
#if 0
        InitAux (This,Index);
        DpConfigInit (This, Index, &mPhyGopMode[This->ModeNum]);
        WakeUpSink (This, Index);
        gBS->Stall (10*1000);
        Status = DpConnect(This,Index);
        DptxSetTraningPattern (This,Index,TRAINING_OFF);
        SinkDpcdWrite (This,Index,0x102,0x0);
        DptxDisableScrambling (This,Index,0);
        DptxScramblingRest (This,Index);
        ConfigMainStreamAttr (This,Index,&This->DpConfig[Index],&This->DpSync[This->ModeNum]);
#endif
        if(This->DpConnect[Index] == SINK_CONNECT) {
          if (1 == This->IsEdp[Index]) {
            if (This->EdpBgLightOn[Index] == 0) {
              if (EFI_SUCCESS == EdpBgLightControl (This, Index, 1, This->EdpLight)) {
                This->EdpBgLightOn[Index] = 1;
                DEBUG ((DEBUG_INFO, "Edp Light on\n"));
              }
            }
          }
          DEBUG((DEBUG_INFO,"hot plug connect\n"));
          //gBS->Stall (5*1000*1000);
        }
        else{
          DEBUG((DEBUG_INFO,"hot plug disconnect\n"));
        }
      }
    }
  }
}


/**
  Constructor for the Graphics Output Protocol,initialize,specific variables, information.

  @param[in,out]  This          Pointer to Graphics Output protocol instance

  @retval         EFI_SUCCESS   Initialization completes without error.
**/
EFI_STATUS
PhyGraphicsOutputConstructor (
  IN OUT PHY_PRIVATE_DATA  *Private
  )
{
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  UINT32                       Index;
  UINT32                       Width;
  UINT32                       Height;
  UINT8                        DpUseStatus;
  UINT8                        EdpStatus;
  UINT32                       DownSpreadStatus;

  //UINT32                       Addrl,Addrh;
  Width = PcdGet32(PcdVideoHorizontalResolution);
  Height = PcdGet32(PcdVideoVerticalResolution);
#ifdef INIT_RES_INIT
  Width = INIT_WIDTH;
  Height = INIT_HEIGHT;
#endif
  DEBUG((DEBUG_INFO,"init height : %d , width : %d\n",Height,Width));
  GraphicsOutput            = &Private->GraphicsOutput;
  GraphicsOutput->QueryMode = PhyGraphicsOutputQueryMode;
  GraphicsOutput->SetMode   = PhyGraphicsOutputSetMode;
  GraphicsOutput->Blt       = PhyGraphicsOutputBlt;

  //
  // Initialize the private data
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
                  (VOID **) &Private->GraphicsOutput.Mode
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  (VOID **) &Private->GraphicsOutput.Mode->Info
                );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (Private->GraphicsOutput.Mode);
    return Status;
  }
  Private->GraphicsOutput.Mode->MaxMode = PHY_GOP_MAX_MODENUM;
  Private->GraphicsOutput.Mode->Mode    = ResToModeNum (Width, Height);
  Private->HardwareNeedsStarting        = TRUE;
  Private->LineBuffer                   = NULL;
  Private->ModeNum =   Private->GraphicsOutput.Mode->Mode ;
  for (Index = 0; Index < Private->GraphicsOutput.Mode->MaxMode; Index++) {
    Private->ModeData[Index].Width = mPhyGopMode[Index].Width;
    Private->ModeData[Index].Height = mPhyGopMode[Index].Height;
    Private->ModeData[Index].ColorDepth = mPhyGopMode[Index].ColorDepth;
    Private->ModeData[Index].RefreshRate = mPhyGopMode[Index].RefreshRate;
    Private->ModeData[Index].ModeName = mPhyGopMode[Index].ModeName;
    CopyMem (&Private->DpSync[Index], &mDpSyncTable[Index], sizeof(DP_SYNC));
    CopyMem (&Private->DcSync[Index], &mDcSyncTable[Index], sizeof(DC_SYNC));
  }
  //
  //Get Edp Light Value form Variable
  //
  EdpLightGetFromVariable (&Private->EdpLight);
  DEBUG ((DEBUG_INFO, "Get Edp Light From Variable : %d\n", Private->EdpLight));
  //init dp trained status
  Private->DpIsTrained[0] = 0;
  Private->DpIsTrained[1] = 0;
  Private->DpIsTrained[2] = 0;
  Private->DpIsHpdOn[0] = DP_HPD_OFF;
  Private->DpIsHpdOn[1] = DP_HPD_OFF;
  Private->DpIsHpdOn[2] = DP_HPD_OFF;
  DpUseStatus = (UINT8) ((PhyPciRead (Private, 0x2c) >> 16));
  DEBUG ((DEBUG_INFO, "Dp Used Status : 0x%02x\n", DpUseStatus));
  for (Index = 0; Index < 3; Index++) {
    Private->DpIsUsed[Index] = (UINT32) ((DpUseStatus >> Index) & 0x1);
    DEBUG ((DEBUG_INFO, "Dp[%d] is Used : %d\n", Index, Private->DpIsUsed[Index]));
  }
  Private->IsEdp[0] = 0;
  Private->IsEdp[1] = 0;
  Private->IsEdp[2] = 0;
  Private->EdpNeedInit[0] = 0;
  Private->EdpNeedInit[1] = 0;
  Private->EdpNeedInit[2] = 0;
  EdpStatus = (UINT8) ((PhyPciRead (Private, 0x2c) >> 16));
  EdpStatus = (EdpStatus >> 3) & 0x7;
  DEBUG ((DEBUG_INFO, "eDp status : %x\n", EdpStatus));
  for (Index = 0; Index < 3; Index++) {
    Private->IsEdp[Index] = (EdpStatus >> Index) & 0x1;
    DEBUG ((DEBUG_INFO, "Dp[%d] is eDp : %d\n",Index, Private->IsEdp[Index]));
    if (1 == Private->IsEdp[Index]) {
      //
      //edp power on
      //
      EdpPowerControl (Private, Index, 1);
      DEBUG ((DEBUG_INFO, "Edp power on!\n"));
      Private->EdpNeedInit[Index] = 1;
      if (EFI_SUCCESS == EdpBgLightControl(Private, Index, 0, 0)) {
        Private->EdpBgLightOn[Index] = 0;
        DEBUG ((DEBUG_INFO, "Edp Light Off\n"));
      }
    }
  }
  //
  //Downspread ctrl
  //
  DownSpreadStatus = (PhyReadMmio (Private, 0, 0x4048) >> 3) & 0x7;
  DEBUG ((DEBUG_INFO, "DownSpread Status : 0x%x\n", DownSpreadStatus));
  for (Index = 0; Index < 3; Index++) {
    Private->DownSpreadEnable[Index] = (DownSpreadStatus >> Index) & 0x1;
    DEBUG ((DEBUG_INFO, "Dp[%d] downspread ctrl : %d\n", Index, Private->DownSpreadEnable[Index]));
  }
  gBS->Stall (10 * 1000);
  //
  //dc dp reset
  //
  for (Index = 0; Index < 3; Index ++) {
    if (Private->DpIsUsed[Index] == 1) {
      if (EFI_SUCCESS == DcDpReset (Private, Index)) {
        DEBUG ((DEBUG_INFO, "Dc Dp [%d] reset success\n", Index));
      }
      else {
        DEBUG ((DEBUG_INFO, "Dc Dp [%d] reset timeout\n", Index));
      }
    }
  }
#ifndef MAIN_RAM_USE
  UINT32  Addrl,Addrh;
  UINT32  Value;
  Addrl = PhyPciRead (Private, 0x18) & 0xFFFFFFF0;
  Addrh = PhyPciRead (Private, 0x1c);
  Private->BufferAddr = ((UINT64) Addrh << 32) + Addrl;
  PhyWriteMmio (Private, 0, 0x4024, (UINT32) ((Private->BufferAddr & 0xFFFFFFFFFF)>> 22));
  PhyWriteMmio (Private, 0, 0x4028, (VRAM_SIZE >> 22) | 0x80000000);
  Value = PhyReadMmio (Private, 0, 0x402c);
  PhyWriteMmio (Private, 0, 0x402c, Value);
  DEBUG ((DEBUG_INFO, "Buffer Addr : %llx\n", Private->BufferAddr));
  Private->ShadowBuffer = SIZE_4GB - 1;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  MRAM_PAGES,
                  &Private->ShadowBuffer
                  );
  ASSERT (!EFI_ERROR(Status));
  DEBUG ((DEBUG_INFO, "Shadow Buffer Addr : %llx\n", Private->ShadowBuffer));
#else
  DEBUG((DEBUG_INFO,"mram pages : %d\n",MRAM_PAGES));
  Private->BufferAddr = SIZE_4GB - 1;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  MRAM_PAGES,
                  &Private->BufferAddr
                  );
  ASSERT (!EFI_ERROR(Status));
  DEBUG ((DEBUG_INFO, "Buffer Addr : %llx\n", Private->BufferAddr));
#endif
  //
  // Initialize the hardware
  //
  //init phy
  LinkPhyAllInit (Private);
  GraphicsOutput->SetMode (GraphicsOutput, Private->GraphicsOutput.Mode->Mode );
  //Create hot plug check event
#if 1
  Status = gBS->CreateEvent (
              EVT_NOTIFY_SIGNAL | EVT_TIMER,
              TPL_CALLBACK,
              DpSinkHotPlugCheck,
              Private,
              &Private->DpSinkHotPlugEvent
              );
  Status = gBS->SetTimer (
                  Private->DpSinkHotPlugEvent,
                  TimerPeriodic,
                  DPSINK_HOTPLUG_CHECK_INTERVAL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG (
      (DEBUG_ERROR,
      "PhyGraphicsOutputConstructor, gBS->SetTimer for DpSinkHotPlugEvent %r.\n",
      Status)
      );
    gBS->CloseEvent (Private->DpSinkHotPlugEvent);
  }
#endif
  return EFI_SUCCESS;
}

/**
  Clean up after closing the Graphics Output Protocol.

  @param[in,out]  This          Pointer to Graphics Output protocol instance

  @retval         EFI_SUCCESS   Success.
**/
EFI_STATUS
PhyGraphicsOutputDestructor (
  IN OUT PHY_PRIVATE_DATA  *Private
  )
{
  gBS->CloseEvent (Private->DpSinkHotPlugEvent);

  if (Private->GraphicsOutput.Mode != NULL) {
    if (Private->GraphicsOutput.Mode->Info != NULL) {
      gBS->FreePool (Private->GraphicsOutput.Mode->Info);
    }
    gBS->FreePool (Private->GraphicsOutput.Mode);
  }
  return EFI_SUCCESS;
}

