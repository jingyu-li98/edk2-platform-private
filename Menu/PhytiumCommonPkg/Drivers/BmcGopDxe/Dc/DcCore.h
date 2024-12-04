/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_CORE_H_
#define    _DC_CORE_H_

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
  );

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
  );

/**
  Config horizontal timing parameter.

  @param[in,out]  Private       Pointer to private data structure.
  @param[in]      Num           the index of dcdp , 0 or 1.
  @param[in]      LinePixels    Visible Number of horizontal lines.
  @param[in]      TotalPixels   Total Number of horizontal lines.
  @param[in]      HsyncStart    Start of horizontal sync pulse.
  @param[in]      HsyncEnd      End of horizontal sync pulse.
  @param[in]      HsyncPolarty  Polarity of the horizontal sync pulse.1 - positive , 0 - negative.

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
  );

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
  );

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
  IN OUT PHY_PRIVATE_DATA  Private,
  IN     UINT8             Num,
  IN     VIDEO_FORMAT      Format,
  IN     PHY_TILING_TYPE   Tiling
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  IN UINT8                 Num,
  IN BOOLEAN               Enable
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Initialization of dc configuration parameter.

  @param[in,out]  Private    Pointer to private data structure.
  @param[in]      Num        the index of dcdp , 0 or 1.

  @retval     EFI_SUCCESS    Dc configuration initializes success
**/
UINT32
DcConfigInit (
  IN OUT PHY_PRIVATE_DATA   *Private,
  IN     UINT8              Num,
  IN     PHY_GOP_MODE       *GopMode
  );
#endif
