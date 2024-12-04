/** @file
** DcReq.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "DcReq.h"
#include "../PhyGopDxe.h"
#include <Library/IoLib.h>

/**
  Set dcreq video layer 0 or 1 framebuffer start and end address by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  addr       Pointer to dcreq buffer structure include start and end address.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
SetDcreqAddr (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DCREQ_ADDR        *Addr,
  IN UINT8             LayerNum
  )
{
  //DEBUG ((DEBUG_INFO, "layer %d start : 0x%x , end : 0x%x\n", LayerNum, Addr->AddrStart, Addr->AddrEnd));
  if (LayerNum == 0) {
    PhyWriteMmio (Private, Num, DCREQ_ADDR_START00, Addr->AddrStart);
    PhyWriteMmio (Private, Num, DCREQ_ADDR_END00, Addr->AddrEnd);
  }
  else {
    PhyWriteMmio (Private, Num, DCREQ_ADDR_START10, Addr->AddrStart);
    PhyWriteMmio (Private, Num, DCREQ_ADDR_END10, Addr->AddrEnd);
  }
}

/**
  Set dcreq video layer 0 or 1 configuration include lossy state , tilemode , argbswizzle mode ,color format
  and tile tpye by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  layer      Pointer to dcreq layer structure , include lossy state , tilemode , argbswizzle mode ,color format
                         and tile tpye

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
SetLayerConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN LAYER_CONFIG      *Layer,
  IN UINT8             LayerNum
  )
{
  UINT32 Config;
  //DEBUG ((DEBUG_INFO, "layer %d config:\n", LayerNum));
  //DEBUG ((DEBUG_INFO, "isLossy : %d\n", Layer->IsLossy));
  //DEBUG ((DEBUG_INFO, "tile type : %d\n", Layer->TileType));
  //DEBUG ((DEBUG_INFO, "argb swizzle : %d\n", Layer->ArgbSwizzle));
  //DEBUG ((DEBUG_INFO, "color format : %02x\n", Layer->ColorFormat));
  //DEBUG ((DEBUG_INFO, "mode : %d\n", Layer->Mode));
  Config = 0;
  //
  //bit 0 ,lossy
  //
  Config |= Layer->IsLossy;
  //
  //bit 2:1 tile type
  //
  Config &= 0xFFFFFFF9;
  Config |= (Layer->TileType << 1);
  //
  //bit 7:4 argb swizzle
  //
  Config &= 0xFFFFFF0F;
  Config |= (Layer->ArgbSwizzle << 4);
  //
  //bit 14:8
  //
  Config &= 0xFFFF80FF;
  Config |= (Layer->ColorFormat << 8);
  //
  //bit 16
  //
  Config &= 0xFFFEFFFF;
  Config |= (Layer->Mode << 16);
  if (LayerNum == 0) {
    PhyWriteMmio (Private, Num, DCREQ_LAYER0_CFG, Config);
  }
  else{
    PhyWriteMmio (Private, Num, DCREQ_LAYER1_CFG, Config);
  }
}

/**
  Set dcreq video layer 0 or 1 clear color by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  color      Pointer to dcreq clear color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
SetClearColor (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN CLEAR_COLOR       *Color,
  IN UINT8             LayerNum
  )
{
  //DEBUG ((DEBUG_INFO, "layer %d clear color high : 0x%x, low : 0x%x\n", LayerNum, Color->ColorH, Color->ColorL));
  if (LayerNum == 0) {
    PhyWriteMmio (Private, Num, DCREQ_CLEARCOLOR0_L, Color->ColorL);
    PhyWriteMmio (Private, Num, DCREQ_CLEARCOLOR0_H, Color->ColorH);
  }
  else {
    PhyWriteMmio (Private, Num, DCREQ_CLEARCOLOR1_L, Color->ColorL);
    PhyWriteMmio (Private, Num, DCREQ_CLEARCOLOR1_H, Color->ColorH);
  }
}

/**
  Set dcreq video layer 0 or 1 ARGB constant color by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  val        Pointer to dcreq ARGB constant color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
SetCh0123Val (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN CH0123_VAL        *Val,
  IN UINT8             LayerNum
  )
{
  UINT32  Config;
  //DEBUG ((DEBUG_INFO, "layer %d ch1023 val, alpha : %d,red : %d,green : %d,blue : %d\n", LayerNum, Val->Alpha, Val->Red, Val->Green, Val->Blue));
  Config = 0;
  Config &= 0xFFFFFF00;
  Config |= (Val->Blue);
  Config &= 0xFFFF00FF;
  Config |= (Val->Green << 8) & 0x0000FF00;
  Config &= 0xFF00FFFF;
  Config |= (Val->Red << 16) & 0x00FF0000;
  Config &= 0x00FFFFFF;
  Config |= (Val->Alpha << 24) & 0xFF000000;
  if (LayerNum == 0) {
    PhyWriteMmio (Private, Num, DCREQ_CH0123_VAL0, Config);
  }
  else {
    PhyWriteMmio( Private, Num, DCREQ_CH0123_VAL1, Config);
  }
}

/**
  Set dcreq video layer 0 or 1 yuv constant color by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  val        Pointer to dcreq yuv constant color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
SetYuvVal (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN YUV_VAL           *Val,
  IN UINT8             LayerNum
  )
{
  UINT32  Config;
  //DEBUG ((DEBUG_INFO, "layer %d Yuv val ,y : 0x%x,uv : 0x%x\n", LayerNum, Val->YVal, Val->UvVal));
  Config = 0;
  //
  //y
  //
  Config &= 0xFFFFFC03;
  Config |= (Val->YVal << 2) & 0x000003FC;
  //
  //uv
  //
  Config &= 0xFC03FFFF;
  Config |= (Val->UvVal << 16) & 0x03FC0000;
  if (LayerNum == 0) {
    PhyWriteMmio (Private, Num, DCREQ_YUV_VAL0, Config);
  }
  else {
    PhyWriteMmio (Private, Num, DCREQ_YUV_VAL1, Config);
  }
}

/**
  Set dcreq enable switch by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  enable     Enable switch , 0- disable , 1 - enable.

  @retval     NULL
**/
VOID
SetDcreqEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Enable
  )
{
  if (Enable == 0) {
    PhyWriteMmio (Private, Num, DCREQ_ENABLE, 0);
  }
  else {
    PhyWriteMmio (Private, Num, DCREQ_ENABLE, 1);
  }
}

/**
  Set dcreq fbdc function enable switch by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  enable     Enable switch of fbdc function , 0- disable , 1 - enable.

  @retval     NULL
**/
VOID
SetFBDCFilterEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Enable
  )
{
  UINT32 Config = 0;
  Config |= 0x100;
  if (Enable == 0) {
    Config &= 0xFFFFFFFE;
  }
  else {
    Config |= 0x1;
  }
  PhyWriteMmio (Private, Num, DCREQ_FILTER, Config);
}

/**
  Get fbdc hardware information by reading register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[out] info       Pointer to fbdc information structure.

  @retval     NULL
**/
VOID
GetFBDCInfo (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT FBDC_CORE_INFO   *Info
  )
{
  UINT32 Value;
  Value = PhyReadMmio (Private, Num, DCREQ_FBDC_BP);
  Info->ProductCode = (UINT16) Value;
  Info->BranchCode = (UINT16) (Value >> 16);
  Value = PhyReadMmio (Private, Num, DCREQ_FBDC_NV);
  Info->VersionCode = (UINT16) Value;
  Info->ScalableCoreCode = (UINT16) (Value >> 16);
  Value = PhyReadMmio (Private, Num, DCREQ_FBDC_C);
  Info->ConfigCode = (UINT16) Value;
  Info->IPChangelistCode = PhyReadMmio (Private, Num, DCREQ_FBDC_IP);
  //DEBUG ((DEBUG_INFO, "FBDC core info :\n"));
  //DEBUG ((DEBUG_INFO, "product code : %04x\n", Info->ProductCode));
  //DEBUG ((DEBUG_INFO, "branch code : %04x\n", Info->BranchCode));
  //DEBUG ((DEBUG_INFO, "version code : %04x\n", Info->VersionCode));
  //DEBUG ((DEBUG_INFO, "scalable core code : %04x\n", Info->ScalableCoreCode));
  //DEBUG ((DEBUG_INFO, "configuration code : %04x\n", Info->ConfigCode));
  //DEBUG ((DEBUG_INFO, "IP changelist code : %08x\n",Info->IPChangelistCode));
}

/**
  Get dcreq video layer 0 or 1 configuration include lossy state , tilemode , argbswizzle mode ,color format
  and tile tpye by reading register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[out] layer      Pointer to layer configuration structure include lossy state , tilemode , argbswizzle
                           mode ,color format and tile tpye.

  @param[in]  lyaerNum     the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
GetLayerConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT LAYER_CONFIG     *Layer,
  IN UINT8             LayerNum
  )
{
  UINT32  Config = 0;
  if (LayerNum == 0) {
    Config = PhyReadMmio (Private, Num, DCREQ_LAYER0_CFG);
  }
  else {
    Config = PhyReadMmio (Private, Num, DCREQ_LAYER1_CFG);
  }
  Layer->IsLossy = (UINT8) (Config & 0x1);
  Layer->TileType = (UINT8) ((Config >> 1) & 0x3);
  Layer->ArgbSwizzle = (UINT8) ((Config >> 4) & 0xF);
  Layer->ColorFormat = (UINT8) ((Config >> 8) & 0x7F);
  Layer->Mode = (UINT8) ((Config >> 16) & 0x1);
  //DEBUG ((DEBUG_INFO, "layer %d config:\n", LayerNum));
  //DEBUG ((DEBUG_INFO, "isLossy : %d\n", Layer->IsLossy));
  //DEBUG ((DEBUG_INFO, "tile type : %d\n", Layer->TileType));
  //DEBUG ((DEBUG_INFO, "argb swizzle : %d\n", Layer->ArgbSwizzle));
  //DEBUG ((DEBUG_INFO, "color format : %02x\n", Layer->ColorFormat));
  //DEBUG ((DEBUG_INFO, "mode : %d\n", Layer->Mode));
}

/**
  Get dcreq video layer 0 or 1 clear color by reading register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[out] color      Pointer to dcreq clear color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
GetClearColor (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT CLEAR_COLOR       *Color,
  IN  UINT8             LayerNum
)
{
  if (LayerNum == 0) {
    Color->ColorL = PhyReadMmio (Private, Num, DCREQ_CLEARCOLOR0_L);
    Color->ColorH = PhyReadMmio (Private, Num, DCREQ_CLEARCOLOR0_H);
  }
  else {
    Color->ColorL = PhyReadMmio (Private, Num, DCREQ_CLEARCOLOR1_L);
    Color->ColorH = PhyReadMmio (Private, Num, DCREQ_CLEARCOLOR1_H);
  }
  //DEBUG ((DEBUG_INFO, "layer %d clear color high : 0x%x, low : 0x%x\n", LayerNum, Color->ColorH, Color->ColorL));
}

/**
  Get dcreq video layer 0 or 1 ARGB constant color by reading register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[out] val        Pointer to dcreq ARGB constant color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
GetCh0123Val (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT CH0123_VAL        *Val,
  IN  UINT8             LayerNum
  )
{
  UINT32  Config;
  if (LayerNum == 0) {
    Config = PhyReadMmio (Private, Num, DCREQ_CH0123_VAL0);
  }
  else {
    Config = PhyReadMmio (Private, Num, DCREQ_CH0123_VAL0);
  }
  Val->Blue = (UINT8) (Config & 0xFF);
  Val->Green = (UINT8) ((Config >> 8) & 0xFF);
  Val->Red = (UINT8) ((Config >> 16) & 0xFF);
  Val->Alpha = (UINT8) ((Config >> 24) & 0xFF);
  //DEBUG ((DEBUG_INFO, "layer %d ch1023 val, alpha : %d,red : %d,green : %d,blue : %d\n", LayerNum, Val->Alpha, Val->Red, Val->Green, Val->Blue));
}

/**
  Get dcreq video layer 0 or 1 yuv constant color by reading register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[out] val        Pointer to dcreq yuv constant color structure.

  @param[in]  layerNum   the index of layer , 0 or 1.

  @retval     NULL
**/
VOID
GetYuvVal (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT YUV_VAL           *Val,
  IN  UINT8             LayerNum
  )
{
  UINT32 Config;
  if (LayerNum == 0) {
    Config = PhyReadMmio (Private, Num, DCREQ_YUV_VAL0);
  }
  else {
    Config = PhyReadMmio (Private, Num, DCREQ_YUV_VAL1);
  }
  Val->YVal = (UINT8) ((Config >> 2) & 0xFF);
  Val->UvVal = (UINT8) ((Config >> 18) & 0xFF);
  //DEBUG ((DEBUG_INFO, "layer %d Yuv val ,y : 0x%x,uv : 0x%x\n", LayerNum, Val->YVal, Val->UvVal));
}

/**
  Set dcreq  parameter by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  config     Pointer to dcreq parameter structure.

  @retval     NULL
**/
VOID
SetDcreqConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DCREQ_CONFIG      *Config
  )
{
  UINT32 I;
  ASSERT (Num < 3);
  if (Private->DcCore[Num].DcreqConfig.DcreqEnable == 0) {
    //DEBUG ((DEBUG_INFO, "disable dcreq\n"));
    SetDcreqEnable (Private, Num, 0);
  }
  else {
    SetDcreqEnable (Private, Num, 1);
    for (I = 0; I < 2; I++) {
      if (Config->LayerConfig[I].Mode == 0) {
        SetLayerConfig (Private, Num, &Private->DcCore[Num].DcreqConfig.LayerConfig[I],I);
      }
      else {
        SetLayerConfig (Private, Num, &Private->DcCore[Num].DcreqConfig.LayerConfig[I], I);
        SetDcreqAddr (Private, Num, &Private->DcCore[Num].DcreqConfig.DcreqAddr[I], I);
        SetClearColor (Private, Num, &Private->DcCore[Num].DcreqConfig.ClearColor[I], I);
        SetCh0123Val (Private, Num, &Private->DcCore[Num].DcreqConfig.Ch0123Val[I], I);
      }
    }
  }
}

/**
  Convert dcreq color format to dc color format.

  @param[in]  Format  dcreq color format ,define by DCREQ_COLOR_FORMAT eNum.
                      0xE - ARGB2101010
                      0x29 - BGRA8888
                      0x02 - A4R4G4B4
                      0x04 - A1R5G5B5
                      0x5 - A5G6B5
                      0x59 - YUV422-VYUY
                      0x5B - YUV422-YVYU

  @retval     0x01    FORMAT_A4R4G4B4

  @retval     0x03    FORMAT_A1R5G5B5

  @retval     0x04    FORMAT_R5G6B5

  @retval     0x06    FORMAT_A8R8G8B8

  @retval     0x16    FORMAT_A2R10G10B10

  @retval     0x07    FORMAT_YUY2

  @retval     0x08    FORMAT_UYVY
**/
UINT8
DcreqformatToDc (
  IN UINT8  Format
  )
{
  UINT8 Dcformat;
  Dcformat = 0;
  switch (Format) {
  case DCREQ_A4R4G4B4:
    Dcformat = FORMAT_A4R4G4B4;
    break;
  case DCREQ_A1R5G5B5:
    Dcformat = FORMAT_A1R5G5B5;
    break;
  case DCREQ_R5G6B5:
    Dcformat = FORMAT_R5G6B5;
    break;
  case DCREQ_ARGB2101010:
    Dcformat = FORMAT_A2R10G10B10;
    break;
  case DCREQ_BGRA8888:
    Dcformat = FORMAT_A8R8G8B8;
    break;
  case DCREQ_YUV422_VYUY:
    Dcformat = FORMAT_YUY2;
    break;
  case DCREQ_YUV422_YVYU:
    Dcformat = FORMAT_UYVY;
    break;
  default:
    break;
  }
  return  Dcformat;
}

/**
  Match dcreq swizzle by dcreq color format.Two categories : BGRA8888 and other.

  @param[in]  format    dcreq color format ,define by DCREQ_COLOR_FORMAT eNum.
                        0xE - ARGB2101010
                        0x29 - BGRA8888
                        0x02 - A4R4G4B4
                        0x04 - A1R5G5B5
                        0x5 - A5G6B5
                        0x59 - YUV422-VYUY
                        0x5B - YUV422-YVYU

  @retval     0x0     Swizzle ARGB

  @retval     0x0C    Swizzle BGRA
**/
UINT8
DcreqformatToSwizzle (
  IN UINT8  Format
  )
{
  UINT8 Swizzle;
  if (Format == DCREQ_BGRA8888) {
    Swizzle = 0x0C;
  }
  else {
    Swizzle = 0;
  }
  return Swizzle;
}

/**
  Match dcreq tile type  by dc tile mode.

  @param[in]  format    dc tile mode ,define by DCREQ_COLOR_FORMAT eNum.
                        0x4 - tilemode0
                        0x7 - tilemode3

  @retval     0x1       8x8

  @retval     0x2       16x4
**/
UINT8
DcreqFormatToTiletype (
  IN UINT8  Tilemode
  )
{
  UINT8  TileType;
  TileType = 0;
  if (Tilemode == TiLE_MODE3) {
    TileType = TILETYPE_16_4;
  }
  else if (Tilemode == TiLE_MODE0) {
    TileType = TILETYPE_8_8;
  }
  return TileType;
}

/**
  Calculate buffer end address offset from start address by tile mode.It is special that The buffer end address
  of tile mode 3 is 8 lines aligned.

  @param[in]  tilemode  dc tile mode ,define by DCREQ_COLOR_FORMAT eNum.
                        0x4 - tilemode0
                        0x7 - tilemode3

  @param[in]  offsetAddr  Buffer size

  @param[in]  width     Buffer width

  @param[in]  height    Buffer height

  @retval     buffer end address offset from start address
**/
UINT32
DcreqFormatToEndaddr (
  IN UINT8  Tilemode,
  IN UINT32 OffsetAddr,
  IN UINT32 Width,
  IN UINT32 Height
  )
{
   UINT32  Value;
   Value = OffsetAddr;
   if (((Height / 4) % 2) != 0) {
     if (Tilemode == TiLE_MODE3) {
       Value = OffsetAddr + Width * 4;
     }
   }
   //DEBUG ((DEBUG_INFO, "offset : %d\n", Value));
   return Value;
}

/**
  Set pixel clock in kilohertz unit by configurating register.

  @param[in]  Private    Pointer to private data structure.

  @param[in]  Num        the index of dc , 0 ,1 or 2.

  @param[in]  pixelClk   pixel clock in kilohertz unit

  @retval     0         Success

  @retval     1         Timeout
**/
UINT32
DcreqChangePixel (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            PixelClk
  )
{
  UINT32 Status;    //0-success
  UINT32 Value;
  UINT32  CheckTimes;
  UINT32  CheckTimes1;
  Status = 1;
  Value = 0;
  DEBUG ((DEBUG_INFO, "dcreq change pixel : %d Hz\n", PixelClk));
#if 0
  Status = 0;
  Value = pixelClk;
  //Value = 25175;
  PhyWriteMmio(Private,Num,DCREQ_PIXELCLK_CONFIG,Value);
  gBS->Stall(100*1000);
#endif
#if 1
  CheckTimes = 1000;
  Value &= 0x3FFFFFFF;
  Value |= PixelClk;
  Value |= 0x40000000;
  PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
  while (CheckTimes!=0) {
    Value = PhyReadMmio (Private, Num, DCREQ_PIXELCLK_CONFIG);
    if (( Value & 0x80000000) != 0) {
      //DEBUG ((DEBUG_INFO, "pixel clock change success\n"));
      //clear bit 30
      Value &= 0xBFFFFFFF;
      Status = 1;
      PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
      for (CheckTimes1 = 0; CheckTimes1 < 500; CheckTimes1++) {
        gBS->Stall (1 * 1000);
        Value = PhyReadMmio (Private, Num, DCREQ_PIXELCLK_CONFIG);
        if ((Value & 0x80000000) == 0) {
          Status = 0;
          break;
        }
      }
      break;
    }
    gBS->Stall (1 * 1000);
    CheckTimes--;
    if (CheckTimes == 0) {
      Value &= 0xBFFFFFFF;
      PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
      DEBUG ((DEBUG_INFO, "pixel clock change timeout\n"));
      Status = 1;
    }
  }
#endif
  return Status;
}

#define  EDP_LIGHTTURNON_CMD           101
#define  EDP_LIGHTTURNOFF_CMD          102
#define  EDP_OPENPWR_CMD               103
#define  EDP_POWEROFF_CMD              104
#define  LIGHT_ADJUST_CMD              0x1
#define  RESET_CMD                     0x3
#define  CMD_STATE_OFFSET              28
#define  LIGHT_VALUE_OFFSET            21

/**
  Send eDP light adjust command.

  @param[in]  Private        Pointer to private data structure.

  @param[in]  Num            the index of dc , 0 ,1 or 2.

  @param[in]  Cmd            Command or light value.
                             0x101 : Turn on backlight.
                             0x102 : Turn off backlight.
                             0x103 : Open pwr.
                             0x104 : Power off.
                             0 - 100 : light level.

  @retval     EFI_SECCUSS    Success.

  @retval     EFI_TIMEOUT    Timeout.
**/
EFI_STATUS
SendLightAdjustCmd (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            Cmd
  )
{
  EFI_STATUS  Status;
  UINT32      Value;
  UINT32      CheckTimes;
  UINT32      CheckTimes1;
  CheckTimes = 500;
  Value = 0;
  Value &= 0x3FFFFFFF;
  Value |= (Cmd << LIGHT_VALUE_OFFSET);
  Value |= (LIGHT_ADJUST_CMD << CMD_STATE_OFFSET);
  Value |= 0x40000000;
  DEBUG ((DEBUG_INFO, "cmd Value : %x\n", Value));
  PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
  while (CheckTimes != 0) {
    Value = PhyReadMmio (Private, Num, DCREQ_PIXELCLK_CONFIG);
    if (( Value & 0x80000000) != 0) {
      //clear bit 30
      Status = EFI_TIMEOUT;
      Value &= 0xBFFFFFFF;
      PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
      for (CheckTimes1 = 0; CheckTimes1 < 500; CheckTimes1++) {
        gBS->Stall (1 * 1000);
        Value = PhyReadMmio (Private, Num, DCREQ_PIXELCLK_CONFIG);
        if ((Value & 0x80000000) == 0) {
          Status = EFI_SUCCESS;
          gBS->Stall (5 * 1000);
          break;
        }
      }
      break;
    }
    gBS->Stall (1 * 1000);
    CheckTimes--;
    if (CheckTimes == 0) {
      Value &= 0xBFFFFFFF;
      PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
      DEBUG ((DEBUG_INFO, "eDP backlight adjust timeout, cmd : %d\n", Cmd));
      Status = EFI_TIMEOUT;
    }
  }
  return Status;
}
/**
  Control the eDP power.

  @param[in]  Private        Pointer to private data structure.

  @param[in]  Num            the index of dc , 0 ,1 or 2.

  @param[in]  PowerOn        1 - power on, 0 - power off.

  @retval     EFI_SECCUSS    Success.

  @retval     EFI_TIMEOUT    Timeout.
**/
EFI_STATUS
EdpPowerControl (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             PowerOn
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (1 == PowerOn) {
    //
    //open pwr
    //
    Status = SendLightAdjustCmd (Private, Num, EDP_OPENPWR_CMD);
    //gBS->Stall (10 * 1000);
  }
  else {
    //
    //power off
    //
    Status = SendLightAdjustCmd (Private, Num, EDP_POWEROFF_CMD);
    //gBS->Stall (10 * 1000);
  }
  return Status;
}
/**
  Control the eDP backlight. When the backlight is turned on, the brightness
  range is 1-100.

  @param[in]  Private        Pointer to private data structure.

  @param[in]  Num            the index of dc , 0 ,1 or 2.

  @param[in]  LightOpen      1 - turn on backlight, 0 - turn off backlight.

  @param[in]  LightLevel     the range is 1 - 100.

  @retval     EFI_SECCUSS    Success.

  @retval     EFI_TIMEOUT    Timeout.
**/
EFI_STATUS
EdpBgLightControl (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             LightOpen,
  IN UINT8             LightLevel
  )
{
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;

  //
  //Open backlight
  //
  if (1 == LightOpen) {
    //
    //set light value 50
    //
    Status = SendLightAdjustCmd (Private, Num, LightLevel);
    //gBS->Stall (10 * 1000);
    if (EFI_SUCCESS != Status) {
      return Status;
    }
    //
    //turn on backlight
    //
    Status = SendLightAdjustCmd (Private, Num, EDP_LIGHTTURNON_CMD);
    //gBS->Stall (10 * 1000);
    if (EFI_SUCCESS != Status) {
      return Status;
    }
  }
  else if (0 == LightOpen) {
    Status = SendLightAdjustCmd (Private, Num, EDP_LIGHTTURNOFF_CMD);
    //gBS->Stall (10 * 1000);
    if (EFI_SUCCESS != Status) {
      return Status;
    }
  }

  return Status;
}

/**
  Reset dc/dp. Send reset command to se and wait response.

  @param[in]  Private        Pointer to private data structure.

  @param[in]  Num            the index of dc , 0 ,1 or 2.

  @retval     EFI_SECCUSS    Success.

  @retval     EFI_TIMEOUT    Timeout.
**/
EFI_STATUS
DcDpReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  EFI_STATUS  Status;
  UINT32      Value;

  Status = EFI_TIMEOUT;
  Value = 0;
  Value &= 0x3FFFFFFF;
  Value |= (RESET_CMD << CMD_STATE_OFFSET);
  Value |= 0x40000000;
  PhyWriteMmio (Private, Num, DCREQ_PIXELCLK_CONFIG, Value);
  while (1) {
    gBS->Stall (1 * 1000);
    Value = PhyReadMmio (Private, Num, 0x4048);
    if (((Value >> Num) & 0x1) == 0x1) {
      Status = EFI_SUCCESS;
      DEBUG ((DEBUG_INFO, "reset Value : %x\n", Value));
      Value &= (~ (0x1 << Num ));
      DEBUG ((DEBUG_INFO, "reset Value : %x\n", Value));
      PhyWriteMmio (Private, Num, 0x4048, Value);
      gBS->Stall (20 * 1000);
      break;
    }
  }
  return Status;
}

/**
  Calculate buffer header size in bytes unit by width , height and tile mode.

  @param[in]  Tilemode    dc tile mode ,define by DCREQ_COLOR_FORMAT enum.
                          0x4 - tilemode0
                          0x7 - tilemode3

  @param[in]  Width       Buffer width

  @param[in]  Height      Buffer height

  @retval     buffer end address offset from start address
**/
UINT32
HeaderCal (
  IN UINT32  Width,
  IN UINT32  Height,
  IN UINT32  TileMode
  )
{
  UINT32 Tbyte;
  UINT32 Temp;
  UINT32 Header;
  if (TileMode == TiLE_MODE3) {
    Tbyte = 4;
  }
  else {
    Tbyte = 2;
  }
  Temp = Width * Height * Tbyte / 256 % 256;
  if (Temp == 0)
    Header = Width * Height * Tbyte / 256;
  else
    Header = (Width * Height * Tbyte / 256 / 256 + 1) * 256;
  return Header;
}
