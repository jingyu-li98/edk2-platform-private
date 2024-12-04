/* @file
** DcHw.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "DcHw.h"
#include "DcReq.h"
#include "DcStru.h"
#include "../PhyGopDxe.h"

/**
  Soft reset DC by configurating register .

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwFramebufferReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Idle;
  UINT32  Ret;
  Idle = 0;
  PhyWriteMmio (Private, Num, DCREQ_RESET, 3);
  Ret = PhyReadMmio (Private, Num, AHB_AQ_HI_CLOCK_CONTROL);
  PhyWriteMmio (Private, Num, AHB_AQ_HI_CLOCK_CONTROL, Ret | (1 << 12));
  PhyWriteMmio (Private, Num, AHB_AQ_HI_CLOCK_CONTROL, Ret);
  do{
    PhyWriteMmio (Private, Num, AHB_AQ_HI_IDLE, 0);
    Idle = PhyReadMmio (Private, Num, AHB_AQ_HI_IDLE);
    if (Idle == 0) {
      gBS->Stall (100*1000);
    }
  } while ((Idle & (1 << 16)) == 0);
  PhyWriteMmio (Private, Num, DCREQ_RESET, 0);
}

/**
  Configurate video framebuffer parameter by configurating register .

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwFramebufferSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  UINT32  I;
  UINT32  Temp;
  DC_FRAMEBUFFER  DcFramebuffer;
  UINT32  Format;
  BOOLEAN  HasOutput;
  BOOLEAN  HasGamma;
  Config  = 0;
  ASSERT (Num < 3);
  CopyMem (&DcFramebuffer, &Private->DcCore[Num].Display.Framebuffer, sizeof(DC_FRAMEBUFFER));
  Format = Private->DcCore[Num].Display.FbFormat;
  HasOutput = Private->DcCore[Num].Display.OutputEnable;
  HasGamma = Private->DcCore[Num].Display.GammaEnable;
  //DEBUG ((DEBUG_INFO, "%a , begin ,dc[%d]\n", __FUNCTION__, Num));
  //DEBUG ((DEBUG_INFO, "Display.fb_format : %d\n", Format));
  //DEBUG ((DEBUG_INFO, "Display.output_enable : %d\n", HasOutput));
  //DEBUG ((DEBUG_INFO, "Display.gamma_enable : %d\n", HasGamma));
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    13:11) - (0 ? 13:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:11) - (0 ? 13:11) + 1))))))) << (0 ?
    13:11))) | (((UINT32) ((UINT32) (DcFramebuffer.RotAngle) & ((UINT32) ((((1 ?
    13:11) - (0 ? 13:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:11) - (0 ? 13:11) + 1))))))) << (0 ?
    13:11))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    21:17) - (0 ? 21:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:17) - (0 ? 21:17) + 1))))))) << (0 ?
    21:17))) | (((UINT32) ((UINT32) (DcFramebuffer.TileMode) & ((UINT32) ((((1 ?
    21:17) - (0 ? 21:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:17) - (0 ? 21:17) + 1))))))) << (0 ?
    21:17))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    31:26) - (0 ? 31:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:26) - (0 ? 31:26) + 1))))))) << (0 ?
    31:26))) | (((UINT32) ((UINT32) (Format) & ((UINT32) ((((1 ? 31:26) - (0 ?
    31:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:26) - (0 ? 31:26) + 1))))))) << (0 ?
    31:26))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
    22:22))) | (((UINT32) ((UINT32) (DcFramebuffer.Scale) & ((UINT32) ((((1 ?
    22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ?
    22:22))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    24:23) - (0 ? 24:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ?
    24:23))) | (((UINT32) ((UINT32) (DcFramebuffer.Swizzle) & ((UINT32) ((((1 ?
    24:23) - (0 ? 24:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ?
    24:23))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
    25:25))) | (((UINT32) ((UINT32) (DcFramebuffer.UvSwizzle) & ((UINT32) ((((1 ?
    25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
    25:25))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    10:9) - (0 ? 10:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ?
    10:9))) | (((UINT32) ((UINT32) (DcFramebuffer.Transparency) & ((UINT32) ((((1 ?
    10:9) - (0 ? 10:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ?
    10:9))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 8:8) - (0 ?
    8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
    8:8))) | (((UINT32) ((UINT32) (DcFramebuffer.ClearFB) & ((UINT32) ((((1 ?
    8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
    8:8)));

  if (HasOutput) {
  Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
    4:4))) | (((UINT32) ((UINT32) (1) & ((UINT32) ((((1 ? 4:4) - (0 ?
    4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
    4:4)));
    Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
      0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
  }
  else {
    Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
      0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
      Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
      4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
      4:4))) | (((UINT32) ((UINT32) (0) & ((UINT32) ((((1 ? 4:4) - (0 ?
      4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
      4:4)));
  }
  if (HasGamma) {
    Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
      2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
      2:2))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
  }
  else {
    Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
      2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
      2:2))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
  }
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_UPPLANARADDR0, DcFramebuffer.UAddress);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_VPPLANARADDR0, DcFramebuffer.VAddress);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_USTRIDE0, DcFramebuffer.UStride);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_VSTRIDE0, DcFramebuffer.VStride);
  PhyWriteMmio (Private, Num, AHB_GCREG_INDEXCOLORTABLE_INDEX0, 0);
  for (I = 0; I < 0; I++) {
    PhyWriteMmio (Private, Num, AHB_GCREG_INDEXCOLORTABLE_DATA0, DcFramebuffer.Lut[I]);
  }

  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (DcFramebuffer.Width) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (DcFramebuffer.Height) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_SIZE0, Temp);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_SCALEFACTORX0, DcFramebuffer.ScaleFactorX);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_SCALEFACTORY0, DcFramebuffer.ScaleFactorY);
  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 3:0) - (0 ?
    3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
    3:0))) | (((UINT32) ((UINT32) (DcFramebuffer.FilterTap) & ((UINT32) ((((1 ?
    3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
    3:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 7:4) - (0 ?
    7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
    7:4))) | (((UINT32) ((UINT32) (DcFramebuffer.HorizontalFilterTap) & ((UINT32) ((((1 ?
    7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
    7:4)));
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_SCALECONFIG0, Temp);
  if(DcFramebuffer.Scale == 0) {
    PhyWriteMmio (Private, Num, AHB_GCREG_HORIFILTERKERNEL_INDEX0, 0);
    PhyWriteMmio (Private, Num, AHB_GCREG_HORI_FILTERKERNEL0, DcFramebuffer.HorKernel[0]);
    PhyWriteMmio (Private, Num, AHB_GCREG_VERTI_FILTERKERNEL_INDEX0, 0);
    PhyWriteMmio (Private, Num, AHB_GCREG_VERTI_FILTERKERNEL0, DcFramebuffer.VerKernel[0]);
  }
  else {
    PhyWriteMmio (Private, Num, AHB_GCREG_HORIFILTERKERNEL_INDEX0, 0);
    for (I = 0; I < HOR_KERNEL_MAX; I++) {
      PhyWriteMmio (Private, Num, AHB_GCREG_HORI_FILTERKERNEL0, DcFramebuffer.HorKernel[I]);
    }
    PhyWriteMmio (Private, Num, AHB_GCREG_VERTI_FILTERKERNEL_INDEX0, 0);
    for (I = 0; I < VER_KERNEL_MAX; I++) {
      PhyWriteMmio (Private, Num, AHB_GCREG_VERTI_FILTERKERNEL0, DcFramebuffer.VerKernel[I]);
    }
  }
  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 15:0) - (0 ?
    15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
    15:0))) | (((UINT32) ((UINT32) (DcFramebuffer.InitialOffsetX) & ((UINT32) ((((1 ?
    15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
    15:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 31:16) - (0 ?
    31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
    31:16))) | (((UINT32) ((UINT32) (DcFramebuffer.InitialOffsetY) & ((UINT32) ((((1 ?
    31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
    31:16)));
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_INITOFFSET0, Temp);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_COLORKEY0, DcFramebuffer.ColorKey);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_COLORKEYHIGH0, DcFramebuffer.ColorKeyHigh);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_BGCOLOR0, DcFramebuffer.BgColor);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CLEARVALUE0, DcFramebuffer.ClearValue);
  /*
  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 0:0) - (0 ?
    0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
    0:0))) | (((UINT32) ((UINT32) (1) & ((UINT32) ((((1 ? 0:0) - (0 ?
    0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
    0:0)));*/
  Temp = 0;
  PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_ENABLE, Temp);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0, Config);
}

/**
  Set video framebuffer stride by configurating register .

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwFramebufferSetStride (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Stride;
  ASSERT (Num < 3);
  Stride = Private->DcCore[Num].Display.FbStride;
  //DEBUG ((DEBUG_INFO, "Display.fb_stride : %d\n", Stride));
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFERSTRIDE0, Stride);
}

/**
  Set video framebuffer start address by configurating register .

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwFramebufferSetAddress (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT8 Addrh;
  UINT32 Addrl;
  ASSERT (Num < 3);
  Addrh = (UINT8) ((Private->DcCore[Num].Display.FbPhysAddr >> 32) & 0xFF);
  Addrl = (UINT32) Private->DcCore[Num].Display.FbPhysAddr;
  //DEBUG ((DEBUG_INFO, "Display.fb_phys_addr[0] low32 : 0x%08x\n", Addrl));
  //DEBUG ((DEBUG_INFO, "Display.fb_phys_addr[0] high8 : 0x%02x\n", Addrh));
  PhyWriteMmio (Private, Num, FRAME_ADDR_PREFIX, (UINT32) Addrh);
  PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFERADDR0, Addrl);
}

/**
  Set Cursor parameter by configurating register, include type, enable switch and hotspot.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwCursorSetConfig(
  IN PHY_PRIVATE_DATA  *Private,
  IN INT8              Num
  )
{
  UINT32  Config;
  UINT32  Type;
  UINT32  InsideX;
  UINT32  InsideY;
  BOOLEAN Enable;

  ASSERT (Num < 3);
  Type = Private->DcCore[Num].Cursor.Type;
  InsideX = Private->DcCore[Num].Cursor.HotX;
  InsideY = Private->DcCore[Num].Cursor.HotY;
  Enable = Private->DcCore[Num].Cursor.Enable;

  if (Enable) {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0))) | (((UINT32) ((UINT32) (Type) & ((UINT32) ((((1 ? 1:0) - (0 ?
      1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      20:16) - (0 ? 20:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ?
      20:16))) | (((UINT32) ((UINT32) (InsideX) & ((UINT32) ((((1 ?
      20:16) - (0 ? 20:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ?
      20:16))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
      12:8))) | (((UINT32) ((UINT32) (InsideY) & ((UINT32) ((((1 ?
      12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
      12:8)));
  }
  else {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0))) | (((UINT32) ((UINT32) (0) & ((UINT32) ((((1 ? 1:0) - (0 ?
      1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0)));
  }
  PhyWriteMmio (Private, Num, AHB_GCREG_CURSOR_CONFIG, Config);
}

/**
  Set Cursor position parameter by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwCursorSetPos (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Data;
  UINT32  X;
  UINT32  Y;

  ASSERT (Num < 3);
  X = Private->DcCore[Num].Cursor.X;
  Y = Private->DcCore[Num].Cursor.Y;
  DEBUG ((DEBUG_INFO, "Cursor.x : %d\n", X));
  DEBUG ((DEBUG_INFO, "Cursor.y : %d\n", Y));
  Data = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (X) & ((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16))) | (((UINT32) ((UINT32) (Y) & ((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16)));
  PhyWriteMmio (Private, Num, AHB_GCREG_CURSOR_LOCATION, Data);
}

/**
  Set background and foreground color of masked mode Cursor by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwCursorSetColor (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  BgColor;
  UINT32  FgColor;
  ASSERT (Num < 3);
  BgColor = Private->DcCore[Num].Cursor.BgColor;
  FgColor = Private->DcCore[Num].Cursor.FgColor;
  DEBUG ((DEBUG_INFO, "Cursor.bg_color : %d\n", BgColor));
  DEBUG ((DEBUG_INFO, "Cursor.fg_color : %d\n", FgColor));
  PhyWriteMmio (Private, Num, AHB_GCREG_CURSOR_BACKGROUND, BgColor);
  PhyWriteMmio (Private, Num, AHB_GCREG_CURSOR_FORGEGROUND, FgColor);
}

/**
  Set Cursor buffer start address by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwCursorSetAddress (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  PhysAddr;
  PhysAddr = (UINT32) Private->DcCore[Num].Cursor.PhysAddr;
  //DEBUG ((DEBUG_INFO,"Cursor.phys_addr : %d\n",PhysAddr));
  PhyWriteMmio (Private, Num, AHB_GCREG_CURSOR_ADDR, PhysAddr);
}

/**
  Set visible and total Number of horizontal pixels,visible and total Number of vertical lines by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwDisplaySetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Config;
  UINT32 HorizontalLinePixels;
  UINT32 HorizontalTotalPixels;
  UINT32 VerticalLinePixels;
  UINT32 VerticalTotalPixels;
  HorizontalLinePixels = Private->DcCore[Num].Display.DpyHline;
  HorizontalTotalPixels = Private->DcCore[Num].Display.DpyHtotal;
  VerticalLinePixels = Private->DcCore[Num].Display.DpyVline;
  VerticalTotalPixels = Private->DcCore[Num].Display.DpyVtotal;

  ASSERT (Num < 3);
  //DEBUG ((DEBUG_INFO, "Display.dpy_hline : %d\n", HorizontalLinePixels));
  //DEBUG ((DEBUG_INFO, "Display.dpy_htotal : %d\n", HorizontalTotalPixels));
  //DEBUG ((DEBUG_INFO, "Display.dpy_vline : %d\n", VerticalLinePixels));
  //DEBUG ((DEBUG_INFO, "Display.dpy_vtotal : %d\n", VerticalTotalPixels));
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (HorizontalLinePixels) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16))) | (((UINT32) ((UINT32) (HorizontalTotalPixels) & ((UINT32) ((((1 ?
    30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16)));
  PhyWriteMmio (Private, Num, AHB_GCREG_HDISPLAY0, Config);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (VerticalLinePixels) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16))) | (((UINT32) ((UINT32) (VerticalTotalPixels) & ((UINT32) ((((1 ?
    30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16)));
  PhyWriteMmio (Private, Num, AHB_GCREG_VDISPLAY0, Config);
}

/**
  Set horizontal and vertical sync counter include start , end and polarity by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwDisplaySetSync (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  UINT32  HsyncStart;
  UINT32  HsyncEnd;
  BOOLEAN HasHsyncPositive;
  UINT32  VsyncStart;
  UINT32  VsyncEnd;
  BOOLEAN HasVsyncPositive;

  ASSERT (Num < 3);
  HsyncStart = Private->DcCore[Num].Display.DpyHsyncStart;
  HsyncEnd = Private->DcCore[Num].Display.DpyHsyncEnd;
  HasHsyncPositive = Private->DcCore[Num].Display.DpyHsyncPolarity;
  VsyncStart = Private->DcCore[Num].Display.DpyVsyncStart;
  VsyncEnd = Private->DcCore[Num].Display.DpyVsyncEnd;
  HasVsyncPositive = Private->DcCore[Num].Display.DpyVsyncPolarity;

  //DEBUG ((DEBUG_INFO, "Display.dpy_hsync_start : %d\n", HsyncStart));
  //DEBUG ((DEBUG_INFO, "Display.dpy_hsync_end : %d\n", HsyncEnd));
  //DEBUG ((DEBUG_INFO, "Display.dpy_hsync_polarity : %d\n", HasHsyncPositive));
  //DEBUG ((DEBUG_INFO, "Display.dpy_vsync_start : %d\n", VsyncStart));
  //DEBUG ((DEBUG_INFO, "Display.dpy_vsync_end : %d\n", VsyncEnd));
  //DEBUG ((DEBUG_INFO, "Display.dpy_vsync_polarity : %d\n", HasVsyncPositive));

  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (HsyncStart) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (HsyncEnd) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));

  Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
    30:30))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));

  Config = HasHsyncPositive ? ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
    31:31))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) : ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
    31:31))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

  PhyWriteMmio (Private, Num, AHB_GCREG_HSYNC0, Config);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (VsyncStart) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (VsyncEnd) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));

  Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
    30:30))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));

  Config = HasVsyncPositive ? ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
    31:31))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) : ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
    31:31))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

  PhyWriteMmio (Private, Num, AHB_GCREG_VSYNC0, Config);
}

/**
  Set panel parameters include data enable polarity , data polarity and clock polarity by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwPanelSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Config;
  BOOLEAN HasDepPosivive;
  BOOLEAN HasDapPositive;
  BOOLEAN HasClockpPositive;

  ASSERT (Num < 3);
  HasDepPosivive = Private->DcCore[Num].Display.PanelDepPolarity;
  HasDapPositive = Private->DcCore[Num].Display.PanelDapPolarity;
  HasClockpPositive = Private->DcCore[Num].Display.PanelClockpPolarity;

  //DEBUG ((DEBUG_INFO, "Display.panel_dep_polarity : %d\n", HasDepPosivive));
  //DEBUG ((DEBUG_INFO, "Display.panel_dap_polarity : %d\n", HasDapPositive));
  //DEBUG ((DEBUG_INFO, "Display.panel_clockp_polarity : %d\n", HasClockpPositive));
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
    0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
    0:0))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

  Config = HasDepPosivive ? ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
    1:1))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) : ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
    1:1))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

  Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
    4:4))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

  Config = HasDapPositive ? ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
    5:5))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) : ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
    5:5))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

  Config = ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
    8:8))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)));

  Config = HasClockpPositive ? ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
    9:9))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) : ((((UINT32) (Config)) & ~(((UINT32) (((UINT32) ((((1 ?
    9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
    9:9))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ?
    ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));

  PhyWriteMmio (Private, Num, AHB_GCREG_PANEL_CONFIG0, Config);
}

/**
  Select DP output bus mode and color coding format by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwDpSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  UINT32  Format;
  ASSERT (Num < 3);
  Format = Private->DcCore[Num].Display.OutputDpFormat;
  //DEBUG ((DEBUG_INFO, "DcCore->Display.output_dp_format : %d\n", Format));
  Config = 0x08 | Format;
  PhyWriteMmio (Private, Num, AHB_GCREG_DPCONFIG0, Config);
}

/**
  Enable or disable dither function by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwDitherEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Config;
  BOOLEAN Enable;
  UINT32  Low;
  UINT32  High;

  ASSERT (Num < 3);
  Enable = Private->DcCore[Num].Display.DitherEnable;
  Low = Private->DcCore[Num].Display.DitherTableLow;
  High = Private->DcCore[Num].Display.DitherTableHigh;

  //DEBUG ((DEBUG_INFO, "Display.dither_enable : %d\n", Enable));
  //DEBUG ((DEBUG_INFO, "Display.dither_table_low : %d\n", Low));
  //DEBUG ((DEBUG_INFO, "Display.dither_table_high : %d\n", High));
  if (Enable) {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
      31:31))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_DITHERTABLELOW0, Low);
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_DITHERTABLEHIGH0, High);
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAYDITHERCONFIG0, Config);
  }
  else {
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAYDITHERCONFIG0, 0);
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_DITHERTABLELOW0, 0);
    PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_DITHERTABLEHIGH0, 0);
  }
}

/**
  Set gamma table by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwGammaSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 I;
  UINT32 Config;
  ASSERT (Num < 3);
  for (I = 0; I < GAMMA_INDEX_MAX; I++) {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
      7:0))) | (((UINT32) ((UINT32) (I) & ((UINT32) ((((1 ? 7:0) - (0 ?
      7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
      7:0)));
    PhyWriteMmio (Private, Num, AHB_GCREG_GAMMAINDEX0, Config);
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      9:0) - (0 ? 9:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
      9:0))) | (((UINT32) ((UINT32) (Private->DcCore[Num].Display.Gamma[I][2]) & ((UINT32) ((((1 ?
      9:0) - (0 ? 9:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
      9:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      19:10) - (0 ? 19:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:10) - (0 ? 19:10) + 1))))))) << (0 ?
      19:10))) | (((UINT32) ((UINT32) (Private->DcCore[Num].Display.Gamma[I][1]) & ((UINT32) ((((1 ?
      19:10) - (0 ? 19:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:10) - (0 ? 19:10) + 1))))))) << (0 ?
      19:10))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      29:20) - (0 ? 29:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:20) - (0 ? 29:20) + 1))))))) << (0 ?
      29:20))) | (((UINT32) ((UINT32) (Private->DcCore[Num].Display.Gamma[I][0]) & ((UINT32) ((((1 ?
      29:20) - (0 ? 29:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:20) - (0 ? 29:20) + 1))))))) << (0 ?
      29:20)));
    PhyWriteMmio (Private, Num, AHB_GCREG_GAMMADATA0, Config);
  }
}

/**
  Enable or disable interrupt function by configurating register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwInterruptEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  UINT32 Config;
  if (Enable) {
    Config =  ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0))) | (((UINT32) ((UINT32) (1) & ((UINT32) ((((1 ? 0:0) - (0 ?
      0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0)));
  }
  else {
    Config =  ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0))) | (((UINT32) ((UINT32) (0) & ((UINT32) ((((1 ? 0:0) - (0 ?
      0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0)));
  }
  PhyWriteMmio (Private, Num, AHB_GCREG_DISPLAY_ENABLE, Config);
}

/**
  Get DC interrupt status by reading register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
UINT32
HwInterruptGet (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Ret;
  //reg 147C
  Ret = PhyReadMmio (Private, Num, AHB_GCREG_DISPLAY_INTR);
  return ((Ret >> 0) & 0x1);
}

/**
  Get DC hardware information by reading register.

  @param[in,out]  Private  Pointer to private data structure

  @param[in]      Num      the index of dcdp , 0 , 1 or 2

  @retval         NULL
**/
VOID
HwVersionGet (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  )
{
  ASSERT (Num < 3);
  Private->DcCore[Num].Hardware.ChipId = PhyReadMmio (Private, Num, AHB_GC_CHIP_ID);
  Private->DcCore[Num].Hardware.ChipRevision = PhyReadMmio (Private, Num, AHB_GC_CHIP_REV);
  Private->DcCore[Num].Hardware.ChipPatchRevision = PhyReadMmio (Private, Num, AHB_GCREG_HI_CHIPPATCHREV);
  Private->DcCore[Num].Hardware.ProductId = PhyReadMmio (Private, Num, AHB_GC_PRODUCT_ID);
  Private->DcCore[Num].Hardware.EcoId = PhyReadMmio (Private, Num, AHB_GC_ECO_ID);
  Private->DcCore[Num].Hardware.CustomerId = PhyReadMmio (Private, Num, AHB_GC_CUSTOMER_ID);
  Private->DcCore[Num].Hardware.ProductDate = PhyReadMmio (Private, Num, AHB_GC_CHIP_DATE);
  Private->DcCore[Num].Hardware.ProductTime = PhyReadMmio (Private, Num, AHB_GC_PRODUCT_TIME);
}

/**
  Set overlay parameter by reading register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
DcHwSetOverlay (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  J;
  UINT32  Config;
  DC_OVERLAY  Overlay;

  ASSERT (Num < 3);
  CopyMem (&Overlay, &Private->DcCore[Num].Display.Overlay, sizeof(DC_OVERLAY));

  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 21:16) - (0 ?
    21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
    21:16))) | (((UINT32) ((UINT32) (Overlay.Format) & ((UINT32) ((((1 ?
    21:16) - (0 ? 21:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
    21:16))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 24:24) - (0 ?
    24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
    24:24))) | (((UINT32) ((UINT32) (Overlay.Enable) & ((UINT32) ((((1 ?
    24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
    24:24))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 4:2) - (0 ?
    4:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:2) - (0 ? 4:2) + 1))))))) << (0 ?
    4:2))) | (((UINT32) ((UINT32) (Overlay.RotAngle) & ((UINT32) ((((1 ?
    4:2) - (0 ? 4:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:2) - (0 ? 4:2) + 1))))))) << (0 ?
    4:2))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:13) - (0 ?
    14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
    14:13))) | (((UINT32) ((UINT32) (Overlay.Swizzle) & ((UINT32) ((((1 ?
    14:13) - (0 ? 14:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ?
    14:13))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 15:15) - (0 ?
    15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
    15:15))) | (((UINT32) ((UINT32) (Overlay.UvSwizzle) & ((UINT32) ((((1 ?
    15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
    15:15))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 1:0) - (0 ?
    1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
    1:0))) | (((UINT32) ((UINT32) (Overlay.Transparency) & ((UINT32) ((((1 ?
    1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
    1:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 25:25) - (0 ?
    25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
    25:25))) | (((UINT32) ((UINT32) (Overlay.ClearOverlay) & ((UINT32) ((((1 ?
    25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
    25:25))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 12:8) - (0 ?
    12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
    12:8))) | (((UINT32) ((UINT32) (Overlay.TileMode) & ((UINT32) ((((1 ?
    12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
    12:8)));
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_CONFIG0, Config);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_STRIDE0, Overlay.Stride[0]);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_USTRIDE0, Overlay.Stride[1]);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_VSTRIDE0, Overlay.Stride[2]);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (Overlay.TlX) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (Overlay.TlY) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_TL0, Config);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (Overlay.BrX) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (Overlay.BrY) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_BR0, Config);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 7:5) - (0 ?
    7:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:5) - (0 ? 7:5) + 1))))))) << (0 ?
    7:5))) | (((UINT32) ((UINT32) (0x2) & ((UINT32) ((((1 ? 7:5) - (0 ?
    7:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:5) - (0 ? 7:5) + 1))))))) << (0 ?
    7:5))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 8:8) - (0 ?
    8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
    8:8))) | (((UINT32) ((UINT32) (0x0) & ((UINT32) ((((1 ? 8:8) - (0 ?
    8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
    8:8))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 0:0) - (0 ?
    0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
    0:0))) | (((UINT32) ((UINT32) (Overlay.SrcAlphaMode) & ((UINT32) ((((1 ?
    0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
    0:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 4:3) - (0 ?
    4:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ?
    4:3))) | (((UINT32) ((UINT32) (Overlay.SrcGlobalAlphaMode) & ((UINT32) ((((1 ?
    4:3) - (0 ? 4:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ?
    4:3))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:12) - (0 ?
    14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
    14:12))) | (((UINT32) ((UINT32) (0x2) & ((UINT32) ((((1 ? 14:12) - (0 ?
    14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
    14:12))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 15:15) - (0 ?
    15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
    15:15))) | (((UINT32) ((UINT32) (0x0) & ((UINT32) ((((1 ? 15:15) - (0 ?
    15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
    15:15))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 9:9) - (0 ?
    9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
    9:9))) | (((UINT32) ((UINT32) (Overlay.DstAlphaMode) & ((UINT32) ((((1 ?
    9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
    9:9))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 11:10) - (0 ?
    11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
    11:10))) | (((UINT32) ((UINT32) (Overlay.DstGlobalAlphaMode) & ((UINT32) ((((1 ?
    11:10) - (0 ? 11:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ?
    11:10)));
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_ALPHABLENDCONFIG0, Config);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_SRCGLOBALCOLOR0, Overlay.SrcGlobalAlphaValue);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_DSTGLOBALCOLOR0, Overlay.DstGlobalAlphaValue);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_CLEARVALUE0, Overlay.ClearValue);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (Overlay.Width) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (Overlay.Height) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_SIZE0, Config);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_COLORKEY0, Overlay.ColorKey);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_COLORKEYHIGH0, Overlay.ColorKeyHigh);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_ADDR0, Overlay.Address[0]);
  PhyWriteMmio (Private, Num, AHB_GCREG_OVERLAY_UPLANARADDR0, Overlay.Address[1]);
  if (Overlay.Scale) {
    PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_YSCALEFACTORX0, Overlay.ScaleFactorX);
    PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_YSCALEFACTORY0, Overlay.ScaleFactorY);
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
      8:8))) | (((UINT32) ((UINT32) (Overlay.Scale) & ((UINT32) ((((1 ?
      8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
      8:8))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
      3:0))) | (((UINT32) ((UINT32) (Overlay.FilterTap) & ((UINT32) ((((1 ?
      3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
      3:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
      7:4))) | (((UINT32) ((UINT32) (Overlay.HorizontalFilterTap) & ((UINT32) ((((1 ?
      7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
      7:4)));
    PhyWriteMmio(Private,Num,AHB_GCREG_OVERLA_SCALECONFIG0,Config);
    PhyWriteMmio(Private,Num,AHB_GCREG_OVERLA_HFILKERNEL_INDEX0, 0);
    for (J = 0; J < 128; J++) {
      PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_HFILKERNEL0, Overlay.HorKernel[J]);
    }
    PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_VFILKERNEL_INDEX0, 0);
    for (J = 0; J < 128; J++) {
      PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_VFILKERNEL0, Overlay.VerKernel[J]);
    }
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
      15:0))) | (((UINT32) ((UINT32) (Overlay.InitialOffsetX) & ((UINT32) ((((1 ?
      15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
      15:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
      31:16))) | (((UINT32) ((UINT32) (Overlay.InitialOffsetY) & ((UINT32) ((((1 ?
      31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ?
      31:16)));
    PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_INITOFFSET0, Config);
  }
  else{
    PhyWriteMmio (Private, Num, AHB_GCREG_OVERLA_SCALECONFIG0, 0);
  }
}

/**
  Begin OR stop to copy a new set of registers at the next VBLANK by reading register.

  @param[in]  Private  Pointer to private data structure

  @param[in]  Num      the index of dcdp , 0 , 1 or 2

  @retval     NULL
**/
VOID
HwShadowRegisterPendingEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Enable
  )
{
  UINT32 Config;
  UINT32 Value;
  if (Enable) {
    Value = PhyReadMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0);
    Config = ((((UINT32) (Value)) & ~(((UINT32) (((UINT32) ((((1 ?
      3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
      3:3))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
    PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0, Config);
  }
  else {
    Value = PhyReadMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0);
    Config = ((((UINT32) (Value)) & ~(((UINT32) (((UINT32) ((((1 ?
      3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
      3:3))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
    PhyWriteMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0, Config);
  }
}

/**
  Judge whether dc flip is in progress by reading register.

  @param[in,out]  Private  Pointer to private data structure

  @param[in]      Num      the index of dcdp , 0 , 1 or 2

  @retval         0        flip not in progress

  @retval         other    flip in progress
**/
UINT32
HwFlipInProgressValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Value;
  //reg 1518
  Value = PhyReadMmio (Private, Num, AHB_GCREG_FRAMEBUFFER_CONFIG0);
  return (Value & (1 << 6));
}
