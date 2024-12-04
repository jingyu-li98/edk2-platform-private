/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include "DcStru.h"
#include "DcHw.h"
#include "../PhyGopDxe.h"

/**
  Soft reset DC by configurating register .

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 , 1
  @param[in]  Type     Dc reset type, Core, Axi or AHB.
  @retval     NULL
**/
VOID
HwFramebufferReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DC_RESET_TYPE     Type
  )
{
  UINT32  Value1;
  UINT32  Value2;

  ASSERT (Num < DPDC_PATH_NUM);

  Value1 = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  switch (Type) {
  case DcResetCore:
    Value1 |= BIT(17 + Num);
    Value2 = Value1 & (~BIT(17 + Num));
    break;
  case DcResetAxi:
    Value1 |= BIT(16 + Num);
    Value2 = Value1 & (~BIT(16 + Num));
    break;
  case DcResetAhb:
    Value1 |= BIT(12 + Num);
    Value2 = Value1 & (~BIT(12 + Num));
    break;
  default:
    DEBUG ((DEBUG_ERROR, "Dc Reset Type Error!\n"));
    ASSERT(0);
  }
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value1);
  gBS->Stall (20 * 1000);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value2);
  gBS->Stall (20 * 1000);
}

/**
  DC all module reset.
  1.Reset Dc core, AXI bus, AHB. Delay 20ms after each module is reset.
  2.Release AHB, AXI bus, Dc core reset. Delay 20ms after each step.

  @retval    Null.
**/
VOID
DcReset (
  IN PHY_PRIVATE_DATA  *Private
  )
{
  UINT32  Value;

  //
  //Dc Core
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value |= BIT(17) | BIT(18);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
  //
  //AXI
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value |= BIT(16);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
  //
  //AHB
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value |= BIT(12);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
  //
  //Clear AHB
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value &= ~BIT(12);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
  //
  //Clear AXI
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value &= ~BIT(16);
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
  //
  //Clear Dc Core
  //
  Value = DcCtrlRegRead (Private, DC_AQ_HI_CLOCK_CONTROL);
  Value &= (~BIT(17)) & (~BIT(18));
  DcCtrlRegWrite (Private, DC_AQ_HI_CLOCK_CONTROL, Value);
  gBS->Stall (20 * 1000);
}

/**
  Configurate video framebuffer parameter by configurating register .

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 , 1

  @retval     NULL
**/
VOID
HwFramebufferSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32          Config;
  UINT32          I;
  UINT32          Temp;
  DC_FRAMEBUFFER  DcFramebuffer;
  UINT32          Format;
  BOOLEAN         HasOutput;
  BOOLEAN         HasGamma;

  Config  = 0;
  ASSERT (Num < DPDC_PATH_NUM);

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
  } else {
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
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_UPPLANARADDR_LOW, DcChOperate, (UINT32) DcFramebuffer.UAddress);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_VPPLANARADDR_LOW, DcChOperate, (UINT32) DcFramebuffer.VAddress);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_UPPLANARADDR_HIGH, DcChOperate, (UINT32) (DcFramebuffer.UAddress >> 32));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_VPPLANARADDR_HIGH, DcChOperate, (UINT32) (DcFramebuffer.VAddress >> 32));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_USTRIDE0, DcChOperate, DcFramebuffer.UStride);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_VSTRIDE0, DcChOperate, DcFramebuffer.VStride);

  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (DcFramebuffer.Width) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 29:15) - (0 ?
    29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15))) | (((UINT32) ((UINT32) (DcFramebuffer.Height) & ((UINT32) ((((1 ?
    29:15) - (0 ? 29:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:15) - (0 ? 29:15) + 1))))))) << (0 ?
    29:15)));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_SIZE0, DcChOperate, Temp);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_SCALEFACTORX0, DcChOperate, DcFramebuffer.ScaleFactorX);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_SCALEFACTORY0, DcChOperate, DcFramebuffer.ScaleFactorY);
  Temp = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 3:0) - (0 ?
    3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
    3:0))) | (((UINT32) ((UINT32) (DcFramebuffer.FilterTap) & ((UINT32) ((((1 ?
    3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
    3:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 7:4) - (0 ?
    7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
    7:4))) | (((UINT32) ((UINT32) (DcFramebuffer.HorizontalFilterTap) & ((UINT32) ((((1 ?
    7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
    7:4)));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_SCALECONFIG0, DcChOperate, Temp);
  if(DcFramebuffer.Scale == 0) {
    ChannelRegWrite (Private, Num, DC_GCREG_HORIFILTERKERNEL_INDEX0, DcChOperate, 0);
    ChannelRegWrite (Private, Num, DC_GCREG_HORI_FILTERKERNEL0, DcChOperate, DcFramebuffer.HorKernel[0]);
    ChannelRegWrite (Private, Num, DC_GCREG_VERTI_FILTERKERNEL_INDEX0, DcChOperate, 0);
    ChannelRegWrite (Private, Num, DC_GCREG_VERTI_FILTERKERNEL0, DcChOperate, DcFramebuffer.VerKernel[0]);
  }
  else {
    ChannelRegWrite (Private, Num, DC_GCREG_HORIFILTERKERNEL_INDEX0, DcChOperate, 0);
    for (I = 0; I < HOR_KERNEL_MAX; I++) {
      ChannelRegWrite (Private, Num, DC_GCREG_HORI_FILTERKERNEL0, DcChOperate, DcFramebuffer.HorKernel[I]);
    }
    ChannelRegWrite (Private, Num, DC_GCREG_VERTI_FILTERKERNEL_INDEX0, DcChOperate, 0);
    for (I = 0; I < VER_KERNEL_MAX; I++) {
      ChannelRegWrite (Private, Num, DC_GCREG_VERTI_FILTERKERNEL0, DcChOperate, DcFramebuffer.VerKernel[I]);
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
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_INITOFFSET0, DcChOperate, Temp);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_COLORKEY0, DcChOperate, DcFramebuffer.ColorKey);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_COLORKEYHIGH0, DcChOperate, DcFramebuffer.ColorKeyHigh);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_BGCOLOR0, DcChOperate, DcFramebuffer.BgColor);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_CLEARVALUE0, DcChOperate, DcFramebuffer.ClearValue);
  Temp = 0;
  ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_INTR_ENABLE, DcChOperate, Temp);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate, Config);
}

/**
  Set video framebuffer stride by configurating register .

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
HwFramebufferSetStride (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Stride;

  ASSERT (Num < DPDC_PATH_NUM);
  Stride = Private->DcCore[Num].Display.FbStride;
  //DEBUG ((DEBUG_INFO, "Display.fb_stride : %d\n", Stride));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFERSTRIDE0, DcChOperate, Stride);
}

/**
  Set video framebuffer start address by configurating register .

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
  Addrh = (UINT8) ((Private->DcCore[Num].Display.FbPhysAddr >> 32) & 0xFF);
  Addrl = (UINT32) Private->DcCore[Num].Display.FbPhysAddr;
  //DEBUG ((DEBUG_INFO, "Display.fb_phys_addr[0] low32 : 0x%08x\n", Addrl));
  //DEBUG ((DEBUG_INFO, "Display.fb_phys_addr[0] high8 : 0x%02x\n", Addrh));
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFERADDR_HIGH, DcChOperate, (UINT32) Addrh);
  ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFERADDR_LOW, DcChOperate, Addrl);
}

/**
  Set Cursor parameter by configurating register, include type, enable switch and hotspot.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
HwCursorSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  UINT32  Type;
  UINT32  InsideX;
  UINT32  InsideY;
  BOOLEAN Enable;

  ASSERT (Num < DPDC_PATH_NUM);
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
  } else {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0))) | (((UINT32) ((UINT32) (0) & ((UINT32) ((((1 ? 1:0) - (0 ?
      1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
      1:0)));
  }
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_CONFIG, DcChOperate, Config);
}

/**
  Set Cursor position parameter by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
  X = Private->DcCore[Num].Cursor.X;
  Y = Private->DcCore[Num].Cursor.Y;
  //DEBUG ((DEBUG_INFO, "Cursor.x : %d\n", X));
  //DEBUG ((DEBUG_INFO, "Cursor.y : %d\n", Y));
  Data = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (X) & ((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16))) | (((UINT32) ((UINT32) (Y) & ((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16)));
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_LOCATION, DcChOperate, Data);
}

/**
  Set background and foreground color of masked mode Cursor by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
  BgColor = Private->DcCore[Num].Cursor.BgColor;
  FgColor = Private->DcCore[Num].Cursor.FgColor;
  //DEBUG ((DEBUG_INFO, "Cursor.bg_color : %d\n", BgColor));
  //DEBUG ((DEBUG_INFO, "Cursor.fg_color : %d\n", FgColor));
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_BACKGROUND, DcChOperate, BgColor);
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_FORGEGROUND, DcChOperate, FgColor);
}

/**
  Set Cursor buffer start address by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
HwCursorSetAddress (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  PhysAddrH;
  UINT32  PhysAddrL;

  PhysAddrL = (UINT32) Private->DcCore[Num].Cursor.PhysAddr;
  PhysAddrH = (UINT32) (Private->DcCore[Num].Cursor.PhysAddr >> 32);
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_ADDR_LOW, DcChOperate, PhysAddrL);
  ChannelRegWrite (Private, Num, DC_GCREG_CURSOR_ADDR_HIGH, DcChOperate, PhysAddrH);
}

/**
  Set visible and total Number of horizontal pixels,visible and total Number of vertical lines by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
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
  ChannelRegWrite (Private, Num, DC_GCREG_HDISPLAY0, DcChOperate, Config);
  Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 14:0) - (0 ?
    14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | (((UINT32) ((UINT32) (VerticalLinePixels) & ((UINT32) ((((1 ?
    14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
    14:0))) | ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ? 30:16) - (0 ?
    30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16))) | (((UINT32) ((UINT32) (VerticalTotalPixels) & ((UINT32) ((((1 ?
    30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
    30:16)));
  ChannelRegWrite (Private, Num, DC_GCREG_VDISPLAY0, DcChOperate, Config);
}

/**
  Set horizontal and vertical sync counter include start , end and polarity by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
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

  ChannelRegWrite (Private, Num, DC_GCREG_HSYNC0, DcChOperate, Config);
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

  ChannelRegWrite (Private, Num, DC_GCREG_VSYNC0, DcChOperate, Config);
}

/**
  Set panel parameters include data enable polarity , data polarity and clock polarity by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
HwPanelSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  BOOLEAN HasDepPosivive;
  BOOLEAN HasDapPositive;
  BOOLEAN HasClockpPositive;

  ASSERT (Num < DPDC_PATH_NUM);
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

  ChannelRegWrite (Private, Num, DC_GCREG_PANEL_CONFIG0, DcChOperate, Config);
}

/**
  Select DP output bus mode and color coding format by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
  Format = Private->DcCore[Num].Display.OutputDpFormat;
  //DEBUG ((DEBUG_INFO, "DcCore->Display.output_dp_format : %d\n", Format));
  Config = 0x08 | Format;
  ChannelRegWrite (Private, Num, DC_GCREG_DPCONFIG0, DcChOperate, Config);
}

/**
  Enable or disable dither function by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
HwDitherEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32  Config;
  BOOLEAN Enable;
  UINT32  Low;
  UINT32  High;

  ASSERT (Num < DPDC_PATH_NUM);
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
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_DITHERTABLELOW0, DcChOperate, Low);
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_DITHERTABLEHIGH0, DcChOperate, High);
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAYDITHERCONFIG0, DcChOperate, Config);
  } else {
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAYDITHERCONFIG0, DcChOperate, 0);
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_DITHERTABLELOW0, DcChOperate, 0);
    ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_DITHERTABLEHIGH0, DcChOperate, 0);
  }
}

/**
  Set gamma table by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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

  ASSERT (Num < DPDC_PATH_NUM);
  for (I = 0; I < GAMMA_INDEX_MAX; I++) {
    Config = ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
      7:0))) | (((UINT32) ((UINT32) (I) & ((UINT32) ((((1 ? 7:0) - (0 ?
      7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
      7:0)));
    ChannelRegWrite (Private, Num, DC_GCREG_GAMMAINDEX0, DcChOperate, Config);
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
    ChannelRegWrite (Private, Num, DC_GCREG_GAMMADATA0, DcChOperate, Config);
  }
}

/**
  Enable or disable interrupt function by configurating register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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
  } else {
    Config =  ((((UINT32) (0)) & ~(((UINT32) (((UINT32) ((((1 ?
      0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0))) | (((UINT32) ((UINT32) (0) & ((UINT32) ((((1 ? 0:0) - (0 ?
      0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
      0:0)));
  }
  ChannelRegWrite (Private, Num, DC_GCREG_DISPLAY_INTR_ENABLE, DcChOperate, Config);
}

/**
  Get DC interrupt status by reading register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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
  Ret = ChannelRegRead (Private, Num, DC_GCREG_DISPLAY_INTR, DcChOperate);
  return ((Ret >> 0) & 0x1);
}

/**
  Begin OR stop to copy a new set of registers at the next VBLANK by reading register.

  @param[in]  Private  Pointer to private data structure
  @param[in]  Num      the index of dcdp , 0 or 1

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
    Value = ChannelRegRead (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate);
    Config = ((((UINT32) (Value)) & ~(((UINT32) (((UINT32) ((((1 ?
      3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
      3:3))) | (((UINT32) (0x1 & ((UINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
    ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate, Config);
  } else {
    Value = ChannelRegRead (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate);
    Config = ((((UINT32) (Value)) & ~(((UINT32) (((UINT32) ((((1 ?
      3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
      3:3))) | (((UINT32) (0x0 & ((UINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ?
      ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
    ChannelRegWrite (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate, Config);
  }
}

/**
  Judge whether dc flip is in progress by reading register.

  @param[in,out]  Private  Pointer to private data structure
  @param[in]      Num      the index of dcdp , 0 or 1

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
  Value = ChannelRegRead (Private, Num, DC_GCREG_FRAMEBUFFER_CONFIG0, DcChOperate);
  return (Value & (1 << 6));
}

#ifdef PLD_TEST
UINT32
DcreqChangePixel (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            PixelClk
  )
{
  UINT32  Value;
  UINT64  Reg;

  Value = 0;
  DEBUG ((DEBUG_INFO, "dcreq change pixel : %d Hz\n", PixelClk));

  switch (Num) {
  case 0:
    Reg = DC_CTRL_CH0_PIXEL_CLOCK;
    break;
  case 1:
    Reg = DC_CTRL_CH1_PIXEL_CLOCK;
    break;
  default:
    ASSERT (0);
    break;
  }
  DcCtrlRegWrite (Private, Reg, PixelClk);

  return EFI_SUCCESS;
}

#else
/**
  Set pixel clock in kilohertz unit by configurating register.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dc , 0 or 1.
  @param[in]  PixelClk   pixel clock in kilohertz unit

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
  UINT32  Status;    //0-success
  UINT32  Value;
  UINT32  CheckTimes;
  UINT32  CheckTimes1;
  UINT64  Reg;

  Status = 1;
  Value = 0;
  DEBUG ((DEBUG_INFO, "dcreq change pixel : %d Hz\n", PixelClk));

  switch (Num) {
  case 0:
    Reg = DC_CTRL_CH0_PIXEL_CLOCK;
    break;
  case 1:
    Reg = DC_CTRL_CH1_PIXEL_CLOCK;
    break;
  default:
    ASSERT (0);
    break;
  }

  CheckTimes = 10;
  Value &= 0x3FFFFFFF;
  Value |= PixelClk;
  Value |= 0x40000000;
  DcCtrlRegWrite (Private, Reg, Value);
  while (CheckTimes!=0) {
    gBS->Stall (100 * 1000);
    Value = DcCtrlRegRead (Private, Reg);
    if (( Value & 0x80000000) != 0) {
      DEBUG ((DEBUG_INFO, "pixel clock change success\n"));
      //clear bit 30
      Value &= 0xBFFFFFFF;
      Status = 1;
      DcCtrlRegWrite (Private, Reg, Value);
      for (CheckTimes1 = 0; CheckTimes1 < 500; CheckTimes1++) {
        gBS->Stall (1 * 1000);
        Value = DcCtrlRegRead (Private, Reg);
        if ((Value & 0x80000000) == 0) {
          Status = 0;
          break;
        }
      }
      break;
    }
    CheckTimes--;
    if (CheckTimes == 0) {
      Value &= 0xBFFFFFFF;
      DcCtrlRegWrite (Private, Reg, Value);
      DEBUG ((DEBUG_ERROR, "pixel clock change timeout\n"));
      Status = 1;
    }
  }

  return Status;
}
#endif
