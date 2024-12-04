/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include "DcHw.h"
#include "DcUtil.h"
#include "../Dp/DpDrv.h"

/**
  Initialization of dc core.

  @param[in,out]  Private  Pointer to private data structure
  @param[in]      Num      the index of dcdp , 0 or 1

  @retval     NULL
**/
VOID
CoreInit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  ZeroMem (&Private->DcCore[Num], sizeof (DC_CORE));
}

/**
  Set dc clear function config.

  @param[in,out]  Private    Pointer to private data structure
  @param[in]      Num        the index of dcdp , 0 or 1
  @param[in]      isClear    clear enable , 1 - enable , 0 - disable
  @param[in]      clearvalue clear value , format is A8R8G8B8

  @retval     NULL
**/
VOID
ConfClearSet (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           IsClear,
  IN     UINT32            ClearValue
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  Private->DcCore[Num].Display.Framebuffer.ClearFB = IsClear;
  if (IsClear) {
    Private->DcCore[Num].Display.Framebuffer.ClearValue = ClearValue;
  }
  else {
    Private->DcCore[Num].Display.Framebuffer.ClearValue = 0;
  }
  Private->DcCore[Num].Display.FbDirty = PHY_TRUE;
}

/**
  Config horizontal timing parameter.

  @param[in,out]  Private        Pointer to private data structure.
  @param[in]      Num            the index of dcdp , 0 or 1.
  @param[in]      LinePixels     Visible Number of horizontal lines.
  @param[in]      TotalPixels    Total Number of horizontal lines.
  @param[in]      HsyncStart     Start of horizontal sync pulse.
  @param[in]      HsyncEnd       End of horizontal sync pulse.
  @param[in]      HsyncPolarty   Polarity of the horizontal sync pulse.1 - positive , 0 - negative.

  @retval     NULL
**/
VOID
ConfDisplaySetHorizontal (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            LinePixels,
  IN     UINT32            TotalPixels,
  IN     UINT32            HsyncStart,
  IN     UINT32            HsyncEnd,
  IN     BOOLEAN           HsyncPolarity
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  Private->DcCore[Num].Display.DpyHline   = LinePixels;
  Private->DcCore[Num].Display.DpyHtotal  = TotalPixels;
  Private->DcCore[Num].Display.DpyHsyncStart = HsyncStart;
  Private->DcCore[Num].Display.DpyHsyncEnd   = HsyncEnd;
  Private->DcCore[Num].Display.DpyHsyncPolarity = HsyncPolarity;

  Private->DcCore[Num].Display.DisplayDirty = PHY_TRUE;
}

/**
  Config vertical timing parameter.

  @param[in,out]  Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.
  @param[in]      LinePixels       Visible Number of vertical lines.
  @param[in]      TotalPixels      Total Number of vertical lines.
  @param[in]      VsyncStart       Start of vertical sync pulse.
  @param[in]      VsyncEnd         End of vertical sync pulse.
  @param[in]      VsyncPolarity    Polarity of the vertical sync pulse.1 - positive , 0 - negative.

  @retval     NULL
**/
VOID
ConfDisplaySetVertical (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            LinePixels,
  IN     UINT32            TotalPixels,
  IN     UINT32            VsyncStart,
  IN     UINT32            VsyncEnd,
  IN     BOOLEAN           VsyncPolarity
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  Private->DcCore[Num].Display.DpyVline   = LinePixels;
  Private->DcCore[Num].Display.DpyVtotal  = TotalPixels;
  Private->DcCore[Num].Display.DpyVsyncStart = VsyncStart;
  Private->DcCore[Num].Display.DpyVsyncEnd   = VsyncEnd;
  Private->DcCore[Num].Display.DpyVsyncPolarity = VsyncPolarity;

  Private->DcCore[Num].Display.DisplayDirty = PHY_TRUE;
}

/**
  Commit display configuration about timing parameter .

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1

  @retval     NULL
**/
STATIC
VOID
ConfDisplayCommit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwDisplaySetConfig (Private, Num);
  HwDisplaySetSync (Private, Num);
  Private->DcCore[Num].Display.DisplayDirty = PHY_FALSE;
}

/**
  Config tilemode and color format of vedio.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      Format     color format.
  @param[in]      Tiling     tilemode.

  @retval     NULL
**/
VOID
ConfFramebufferSetConfig (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     VIDEO_FORMAT      Format,
  IN     PHY_TILING_TYPE   Tiling
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Display.FbFormat = Format;
  switch (Tiling) {
    case PhyTILED:
      Private->DcCore[Num].Display.FbTiling = PhyTILED;
      break;
    default:
      Private->DcCore[Num].Display.FbTiling = PhyLINEAR;
      break;
  }
  Private->DcCore[Num].Display.FbDirty = PHY_TRUE;
}

/**
  Config the stride of vedio framebuffer.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      Stride     Stride , Number of bytes from start of one line to next line.

  @retval     NULL
**/
VOID
ConfFramebufferSetStride (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Stride
  )
{
  UINT32 Astride;

  Astride = PHY_ALIGN (Stride, 0x80);
  Astride = Stride;
  Private->DcCore[Num].Display.FbStride = Stride;
  Private->DcCore[Num].Display.FbAstride = Astride;

  HwFramebufferSetStride (Private, Num);
}

/**
  Config  register about starting address of the frame buffer.

  @param[in,out]  Private  Pointer to private data structure.
  @param[in]      Num      the index of dcdp , 0 or 1.
  @param[in]      Addr     Starting address of the frame buffer.

  @retval     NULL
**/
VOID
ConfFramebufferSetAddress (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT64            Addr
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Display.FbPhysAddr = Addr;
  HwFramebufferSetAddress (Private, Num);
}

/**
  Soft reset DC hardware.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.

  @retval     NULL
**/
VOID
ConfFramebufferReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwFramebufferReset (Private, Num, DcResetCore);
}

/**
  Core data config about framebuffer parameter.

  @param[in,out]  Private      Pointer to private data structure.
  @param[in]      Num          the index of dcdp , 0 or 1.
  @param[in]      Framebuffer  Pointer to configuration of framebuffer parameter.

  @retval     NULL
**/
VOID
ConfFramebufferSetFramebuffer (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     DC_FRAMEBUFFER    *Framebuffer
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  CopyMem (&Private->DcCore[Num].Display.Framebuffer, Framebuffer, sizeof(DC_FRAMEBUFFER));
  Private->DcCore[Num].Display.FbDirty = PHY_TRUE;
}

/**
  Config register about vedio parameter.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     NULL
**/
STATIC
VOID
ConfFramebufferCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwFramebufferSetConfig (Private, Num);
  Private->DcCore[Num].Display.FbDirty = PHY_FALSE;
}

/**
  Config core data about dc output enable.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      enable     Enable of output , 1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
ConfOutputEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Enable != Private->DcCore[Num].Display.OutputEnable) {
    Private->DcCore[Num].Display.OutputEnable = Enable;
    Private->DcCore[Num].Display.FbDirty = PHY_TRUE;
  }
}

/**
  Config core data about dc output mode , DP mode or DPI mode.

  @param[in,out]  Private       Pointer to private data structure.
  @param[in]      Num           the index of dcdp , 0 or 1
  @param[in]      OutputType    Output format , 0 - DPI mode , 1 - DP mode.

  @retval     NULL
**/
VOID
ConfOutputSelect (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_OUTPUT_TYPE   OutputType
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Display.OutputType = OutputType;
  Private->DcCore[Num].Display.OutputDirty = PHY_TRUE;
}

/**
  Config core data about dc output formort , DP mode or DPI mode.

  @param[in,out]  Private      Pointer to private data structure.
  @param[in]      Num          the index of dcdp , 0 or 1.
  @param[in]      OutputType   DC output type , 1 - RGB666 , 2 - RGB888 , 3 - RGB101010.

  @retval     NULL
**/
VOID
ConfOutputDpformat (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     DP_FORMAT         Format
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Display.OutputDpFormat = Format;
  Private->DcCore[Num].Display.OutputDirty = PHY_TRUE;
}

/**
  Config register about dc output mode.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     NULL
**/
STATIC
VOID
ConfOutputCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < 3);

  if (Private->DcCore[Num].Display.OutputType == PhyDP) {
    HwDpSetConfig (Private, Num);
  }
  Private->DcCore[Num].Display.OutputDirty = PHY_FALSE;
}

/**
  Config core data about panel.

  @param[in,out]  Private         Pointer to private data structure.
  @param[in]      Num             the index of dcdp , 0 or 1.
  @param[in]      Deppolarity     Data Enable polarity , 1 - positive , 0 - negative.
  @param[in]      Dappolarity     Data polarity , 1 - positive , 0 - negative.
  @param[in]      ClockpPolarity  Clock polarity , 1 - positive , 0 - negative.

  @retval     NULL
**/
VOID
ConfPanelSetConfig (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           DepPolarity,
  IN     BOOLEAN           DapPolarity,
  IN     BOOLEAN           ClockpPolarity
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Display.PanelDepPolarity = DepPolarity;
  Private->DcCore[Num].Display.PanelDapPolarity = DapPolarity;
  Private->DcCore[Num].Display.PanelClockpPolarity = ClockpPolarity;

  Private->DcCore[Num].Display.PanelDirty = PHY_TRUE;
}

/**
  Config register about panel.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.

  @retval     NULL
**/
STATIC
VOID
ConfPanelCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwPanelSetConfig (Private, Num);
  Private->DcCore[Num].Display.PanelDirty = PHY_FALSE;
}

/**
  Config core data about dither enable.

  @param[in,out]  Private      Pointer to private data structure.
  @param[in]      Num          the index of dcdp , 0 or 1.
  @param[in]      Enable       Dither enable , 1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
ConfDitherEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  )
{
  ASSERT (Num < 3);

  if (Enable != Private->DcCore[Num].Display.DitherEnable) {
    Private->DcCore[Num].Display.DitherEnable = Enable;
    Private->DcCore[Num].Display.DitherDirty = PHY_TRUE;
  }
}

/**
  Config core data about gamma.

  @param[in,out]  Private  Pointer to private data structure.
  @param[in]      Num      the index of dcdp , 0 or 1.
  @param[in]      Enable   Gamma enable , 1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
ConfGammaEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Enable != Private->DcCore[Num].Display.GammaEnable) {
    Private->DcCore[Num].Display.GammaEnable = Enable;
    Private->DcCore[Num].Display.FbDirty = PHY_TRUE;
  }
}

/**
  Config core data about gamma table.

  @param[in,out]  Private      Pointer to private data structure.
  @param[in]      Num          the index of dcdp , 0 or 1.
  @param[in]      Index        Index into gamma table.
  @param[in]      Red          Red translation value for the gamma table.
  @param[in]      Green        Green translation value for the gamma table.
  @param[in]      Blue         Blue translation value for the gamma table.

  @retval     NULL
**/
VOID
ConfGammaSetConfig (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Index,
  IN     UINT32            Red,
  IN     UINT32            Green,
  IN     UINT32            Blue
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Index > GAMMA_INDEX_MAX) {
    return;
  }

  Private->DcCore[Num].Display.Gamma[Index][0] = Red;
  Private->DcCore[Num].Display.Gamma[Index][1] = Green;
  Private->DcCore[Num].Display.Gamma[Index][2] = Blue;

  Private->DcCore[Num].Display.GammaDirty = PHY_TRUE;
}

//
//Not nead to support hard cursor, config cursor mode to disable.
//
/**
  Config core data about Cursor.

  @param[in,out]  Private     Pointer to private data structure.
  @param[in]      Num         the index of dcdp , 0 or 1.
  @param[in]      Enable      Cursor enable , 1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
ConfCursorEnable (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     BOOLEAN           Enable
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Private->DcCore[Num].Cursor.Enable != Enable) {
      Private->DcCore[Num].Cursor.Enable = Enable;
      Private->DcCore[Num].Cursor.Dirty = PHY_TRUE;
  }
}

/**
  Config core data about Cursor type.

  @param[in,out]  Private   Pointer to private data structure.
  @param[in]      Num       the index of dcdp , 0 or 1.
  @param[in]      Type      Cursor type , 0 - disable , 1 - mask mode , 2 - argb mode.

  @retval     NULL
**/
VOID
ConfCursorSetType (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_CURSOR_TYPE   Type
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Cursor.Type = Type;
  Private->DcCore[Num].Cursor.Dirty = PHY_TRUE;
}

/**
  Config core data about cursor hotspot.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      X          Vertical offset to cursor hotspot.
  @param[in]      Y          Horizontal offset to cursor hotspot.

  @retval     NULL
**/
VOID
ConfCursorSetHotspot (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            X,
  IN     UINT32            Y
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Cursor.HotX = X;
  Private->DcCore[Num].Cursor.HotY = Y;
  Private->DcCore[Num].Cursor.Dirty = PHY_TRUE;
}

/**
  Config core data about cursor background color and foreground color for mask mode.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      Bg         The background color for Masked cursors . Format is ARGB8888.
  @param[in]      Fg         The foreground color for Masked cursors . Format is ARGB8888.

  @retval     NULL
**/
VOID
ConfCursorSetColor (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            Bg,
  IN     UINT32            Fg
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Cursor.BgColor = Bg;
  Private->DcCore[Num].Cursor.FgColor = Fg;
  Private->DcCore[Num].Cursor.Dirty = PHY_TRUE;
}

/**
  Config core data about address of the cursor shape.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.
  @param[in]      Addr       Address of the cursor shape.

  @retval     NULL
**/
VOID
ConfCursorSetAddress (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT64            Addr
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Addr) {
    Private->DcCore[Num].Cursor.PhysAddr = Addr;
    Private->DcCore[Num].Cursor.Dirty = PHY_TRUE;
  }
}

/**
  Config core data about location of the cursor on the owning display.

  @param[in,out]  Private  Pointer to private data structure.
  @param[in]      Num      the index of dcdp , 0 or 1.
  @param[in]      X        X location of cursor's hotspot.
  @param[in]      Y        Y location of cursor's hotspot.

  @retval     NULL
**/
VOID
ConfCursorSetPos (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     UINT32            X,
  IN     UINT32            Y
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  Private->DcCore[Num].Cursor.X = X;
  Private->DcCore[Num].Cursor.Y = Y;

  HwCursorSetPos (Private, Num);
}

/**
  Config register about cursor parameter.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     NULL
**/
STATIC
VOID
ConfCursorCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
    )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwCursorSetConfig (Private, Num);
  HwCursorSetColor (Private, Num);
  HwCursorSetAddress (Private, Num);
  Private->DcCore[Num].Cursor.Dirty = PHY_FALSE;
}

/**
  Config register about all dc parameter.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     NULL
**/
VOID
ConfCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  if (Private->DcCore[Num].Display.DisplayDirty) {
    ConfDisplayCommit (Private, Num);
  }
  if (Private->DcCore[Num].Display.OutputDirty) {
    ConfOutputCommit (Private, Num);
  }
  if (Private->DcCore[Num].Display.PanelDirty) {
    ConfPanelCommit (Private, Num);
  }
  if (Private->DcCore[Num].Cursor.Dirty) {
    ConfCursorCommit (Private, Num);
  }
  if (Private->DcCore[Num].Display.FbDirty) {
    ConfFramebufferCommit(Private,Num);
  }
}

/**
  Config register about all dc display interrupt .

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.
  @param[in]  Enable     Enable display interrupy , 1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
ConfInterruptEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwInterruptEnable (Private, Num, Enable);
}

/**
  Get display interrupt status.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     display interrupt status
**/
UINT32
ConfInterruptGet (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  return HwInterruptGet (Private, Num);
}

/**
  Config register about dc valid field .

  The valid field defines whether we can copy a new set of registers at the next VBLANK or not. This ensures a frame
  will always start with a valid working set if this register is programmed last,which reduces the need for SW to wait
  for the start of a VBLANK signal in order to ensure all states are loaded before the next VBLANK.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.
  @param[in]  Enable     Valid feild , 1 - pending , 0 - working.

  @retval     NULL
**/
VOID
ConfShadowRegisterPendingEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  HwShadowRegisterPendingEnable (Private, Num, Enable);
}

/**
  Judge whether dc flip is in progress

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     0            Flip not in progress
  @retval     other        Flip in progress
**/
UINT32
ConfFlipInProgressValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  return HwFlipInProgressValue (Private, Num);
}

/**
  Config register about all dc parameter include video framebuffer  address and stride.

  @param[in]  Private    Pointer to private data structure.
  @param[in]  Num        the index of dcdp , 0 or 1.

  @retval     NULL
**/
UINT32
CoreCommit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < DPDC_PATH_NUM);

  ConfFramebufferSetAddress (Private,Num,Private->DcCore[Num].Display.FbPhysAddr);
  ConfFramebufferSetStride (Private,Num,Private->DcCore[Num].Display.FbStride);
  ConfCommit (Private, Num);

  return EFI_SUCCESS;
}

/**
  Initialization of dc configuration parameter.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.

  @retval     EFI_SUCCESS    Dc configuration initializes success
**/
UINT32
DcConfigInit (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num,
  IN     PHY_GOP_MODE      *GopMode
  )
{
  DC_SYNC  DcSyncInfo;
  DC_FRAMEBUFFER  DcFramebuffer;

  ASSERT (Num < DPDC_PATH_NUM);

  ZeroMem (&DcSyncInfo, sizeof (DC_SYNC));
  ZeroMem (&Private->DcCore[Num],sizeof(DC_CORE));
  ZeroMem (&DcFramebuffer, sizeof(DC_FRAMEBUFFER));
  //
  //set format type and tiling type
  //
  ConfPanelSetConfig (Private, Num, 1, 1, 1);
  Private->DcCore[Num].Display.OutputEnable = 1;
  //
  //Init resolution panel information through edid list and default table
  //
  if (EFI_SUCCESS == FindDcSyncFromDtd (&Private->DtdList[Num], GopMode, &DcSyncInfo)) {
    DEBUG ((DEBUG_INFO, "Dc Sync Edid Dtd\n"));
  }
  else {
    DEBUG ((DEBUG_INFO, "Dc Sync VESA Default\n"));
    CopyMem (&DcSyncInfo, &Private->DcSync[GopMode->ModeName], sizeof (DC_SYNC));
  }
  //
  //horizontal
  //
  ConfDisplaySetHorizontal (
    Private,
    Num,
    (UINT32) DcSyncInfo.HDisEnd,
    (UINT32) DcSyncInfo.HTotal,
    (UINT32) DcSyncInfo.HStart,
    (UINT32) DcSyncInfo.HEnd,
    (BOOLEAN) DcSyncInfo.HPolarity
  );
  //
  //vertical
  //
  ConfDisplaySetVertical (
    Private,
    Num,
    (UINT32) DcSyncInfo.VDisEnd,
    (UINT32) DcSyncInfo.VTotal,
    (UINT32) DcSyncInfo.VStart,
    (UINT32) DcSyncInfo.VEnd,
    (BOOLEAN) DcSyncInfo.VPolarity
  );
  ConfOutputSelect (Private, Num, PhyDP);
  ConfOutputDpformat (Private, Num, DP_RGB888);
  //
  //set format type and tiling type
  //
  ConfFramebufferSetConfig (Private, Num, FORMAT_A8R8G8B8, PhyLINEAR);
  DcFramebuffer.Width = GopMode->Width;
  DcFramebuffer.Height = GopMode->Height;
  DcFramebuffer.Transparency = 0;
  DcFramebuffer.RotAngle = ROT_0;
  DcFramebuffer.ClearFB = 0;
  //
  //frameBuffer set
  //
  ConfFramebufferSetFramebuffer (Private, Num, &DcFramebuffer);
  //
  //disable dither
  //
  ConfDitherEnable (Private, Num, 0);
  //
  //diable gamma
  //
  ConfGammaEnable (Private, Num, 0);
  Private->DcCore[Num].Display.FbPhysAddr = Private->BufferAddr;
  Private->DcCore[Num].Display.FbStride = WidthToStride (DcFramebuffer.Width);
  //
  //no dcreq in e2k
  //
#if 0
  DCREQ_CONFIG DcreqInfo;
  ZeroMem (&DcreqInfo, sizeof(DCREQ_CONFIG));
  DcreqInfo.DcreqEnable = 1;
  DcreqInfo.LayerConfig[0].Mode = 0;
  DcreqInfo.LayerConfig[1].Mode = 0;
  CopyMem (&Private->DcCore[Num].DcreqConfig, &DcreqInfo, sizeof(DCREQ_CONFIG));
#endif
  return EFI_SUCCESS;
}
