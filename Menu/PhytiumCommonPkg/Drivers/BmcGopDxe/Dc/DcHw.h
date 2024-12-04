/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_HW_H_
#define    _DC_HW_H_
#include   "../PhyGopDxe.h"

//AHB Byte Address
//DC Ctrl Register
#define    DC_AQ_HI_CLOCK_CONTROL                   0x0000
#define    DC_AQ_HI_IDLE                            0x0004
#define    DC_AQ_AXI_STATUS                         0x000C
#define    DC_AQ_INTR_ENBL                          0x0014
#define    DC_AQ_INTR_ACKNOWLEDGE                   0x0010
#define    DC_GC_TOTAL_READS                        0x0040
#define    DC_GC_TOTAL_CYCLES                       0x0078
#define    DC_CTRL_CH0_PIXEL_CLOCK                  0x00F0
#define    DC_CTRL_CH1_PIXEL_CLOCK                  0x00F4

//Each Dc Core Register
#define    DC_GCREG_FRAMEBUFFERADDR_LOW             0x400
#define    DC_GCREG_FRAMEBUFFERADDR_HIGH            0x404
#define    DC_GCREG_FRAMEBUFFERSTRIDE0              0x408
#define    DC_GCREG_DISPLAYDITHERCONFIG0            0x410
#define    DC_GCREG_PANEL_CONFIG0                   0x418
#define    DC_GCREG_DISPLAY_DITHERTABLELOW0         0x420
#define    DC_GCREG_DISPLAY_DITHERTABLEHIGH0        0x428
#define    DC_GCREG_HDISPLAY0                       0x430
#define    DC_GCREG_HSYNC0                          0x438
#define    DC_GCREG_VDISPLAY0                       0x440
#define    DC_GCREG_VSYNC0                          0x448
#define    DC_GCREG_DISPLAY_CURLOCATION0            0x450
#define    DC_GCREG_GAMMAINDEX0                     0x458
#define    DC_GCREG_GAMMADATA0                      0x460
#define    DC_GCREG_CURSOR_CONFIG                   0x468
#define    DC_GCREG_CURSOR_ADDR_LOW                 0x46C
#define    DC_GCREG_CURSOR_ADDR_HIGH                0x490
#define    DC_GCREG_CURSOR_LOCATION                 0x470
#define    DC_GCREG_CURSOR_BACKGROUND               0x474
#define    DC_GCREG_CURSOR_FORGEGROUND              0x478
#define    DC_GCREG_DISPLAY_INTR                    0x47C
#define    DC_GCREG_DISPLAY_INTR_ENABLE             0x480
#define    DC_GCREG_CURSOR_MODCLKGATECONTROL        0x484
#define    DC_GCREG_DEBUGCOUNTER_SELECT0            0x4D0
#define    DC_GCREG_DEBUGCOUNTER_VALUE0             0x4D8
#define    DC_GCREG_FRAMEBUFFER_COLORKEY0           0x508
#define    DC_GCREG_FRAMEBUFFER_COLORKEYHIGH0       0x510
#define    DC_GCREG_FRAMEBUFFER_CONFIG0             0x518
#define    DC_GCREG_FRAMEBUFFER_SCALECONFIG0        0x520
#define    DC_GCREG_FRAMEBUFFER_BGCOLOR0            0x528
#define    DC_GCREG_FRAMEBUFFER_UPPLANARADDR_LOW    0x530
#define    DC_GCREG_FRAMEBUFFER_UPPLANARADDR_HIGH   0x534
#define    DC_GCREG_FRAMEBUFFER_VPPLANARADDR_LOW    0x538
#define    DC_GCREG_FRAMEBUFFER_VPPLANARADDR_HIGH   0x53C
#define    DC_GCREG_FRAMEBUFFER_USTRIDE0            0x800
#define    DC_GCREG_FRAMEBUFFER_VSTRIDE0            0x808
#define    DC_GCREG_FRAMEBUFFER_SIZE0               0x810
#define    DC_GCREG_FRAMEBUFFER_SCALEFACTORX0       0x828
#define    DC_GCREG_FRAMEBUFFER_SCALEFACTORY0       0x830
#define    DC_GCREG_HORIFILTERKERNEL_INDEX0         0x838
#define    DC_GCREG_HORI_FILTERKERNEL0              0xA00
#define    DC_GCREG_VERTI_FILTERKERNEL_INDEX0       0xA08
#define    DC_GCREG_VERTI_FILTERKERNEL0             0xA10
#define    DC_GCREG_FRAMEBUFFER_CLEARVALUE0         0xA18
#define    DC_GCREG_FRAMEBUFFER_INITOFFSET0         0xA20
#define    DC_GCREG_MODCLKGATE_CONTROL0             0xA28
#define    DC_GCREG_READ_OT                         0xCC8
#define    DC_GCREG_DPCONFIG0                       0xCD0

#define PHY_ALIGN(Data, Offset)             ((Data + Offset - 1) & ~(Offset - 1))

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
  );

/**
  DC all module reset.
  1.Reset Dc core, AXI bus, AHB. Delay 20ms after each module is reset.
  2.Release AHB, AXI bus, Dc core reset. Delay 20ms after each step.

  @retval    Null.
**/
VOID
DcReset (
  IN PHY_PRIVATE_DATA  *Private
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  IN BOOLEAN           enable
  );

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
  );

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
  );

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
  );

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
  );
#endif
