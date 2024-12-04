/* @file
** DcHw.h
** some define about dc module
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_HW_H_
#define    _DC_HW_H_
#include   "../PhyGopDxe.h"

#define    DC_REG_BASE        0

//AHB Byte Address
//HI
#define    AHB_AQ_HI_CLOCK_CONTROL                   (DC_REG_BASE + 0x0000)
#define    AHB_AQ_HI_IDLE                            (DC_REG_BASE + 0x0004)
#define    AHB_AQ_AXI_CONFIG                         (DC_REG_BASE + 0x00A4)
#define    AHB_AQ_AXI_STATUS                         (DC_REG_BASE + 0x000C)
#define    AHB_AQ_INTR_ENBL                          (DC_REG_BASE + 0x0014)
#define    AHB_AQ_INTR_ACKNOWLEDGE                   (DC_REG_BASE + 0x0010)
#define    AHB_GC_CHIP_ID                            (DC_REG_BASE + 0x0020)
#define    AHB_GC_CHIP_REV                           (DC_REG_BASE + 0x0024)
#define    AHB_GC_CHIP_DATE                          (DC_REG_BASE + 0x0028)
#define    AHB_GC_PRODUCT_TIME                       (DC_REG_BASE + 0x002C)
#define    AHB_GC_CUSTOMER_ID                        (DC_REG_BASE + 0x0030)
#define    AHB_GC_TOTAL_READS                        (DC_REG_BASE + 0x0040)
#define    AHB_GC_TOTAL_CYCLES                       (DC_REG_BASE + 0x0078)
#define    AHB_GCREG_HI_CHIPPATCHREV                 (DC_REG_BASE + 0x0098)
#define    AHB_GC_PRODUCT_ID                         (DC_REG_BASE + 0x00A8)
#define    AHB_GC_ECO_ID                             (DC_REG_BASE + 0x00E8)
//MMU
#define    AHB_GCREG_MMU_SAFEADDR                    (DC_REG_BASE + 0x0180)
#define    AHB_GCREG_MMU_CONFIGRATION                (DC_REG_BASE + 0x0184)
#define    AHB_GCREG_MMU_CONTROL                     (DC_REG_BASE + 0x018C)
//DEC
#define    AHB_GCREG_DEC_READCONFIG0                 (DC_REG_BASE + 0x0800)
#define    AHB_GCREG_DEC_READBUFFERBASE0             (DC_REG_BASE + 0x0900)
#define    AHB_GCREG_DEC_READCACHEBASE0              (DC_REG_BASE + 0x0980)
#define    AHB_GCREG_DEC_CONTROL                     (DC_REG_BASE + 0x0B00)
#define    AHB_GCREG_DEC_INTRACKNOWLEDGE             (DC_REG_BASE + 0x0B04)
#define    AHB_GCREG_DEC_INTRENBL                    (DC_REG_BASE + 0x0B08)
#define    AHB_GCREG_DEC_TILESTATUSDEBUG             (DC_REG_BASE + 0x0B0C)
#define    AHB_GCREG_DEC_DECODERDEBUG                (DC_REG_BASE + 0x0B14)
#define    AHB_GCREG_DEC_TOTALREAD_SIN               (DC_REG_BASE + 0x0B18)
#define    AHB_GCREG_DEC_TOTALREAD_SREQIN            (DC_REG_BASE + 0x0B28)
#define    AHB_GCREG_DEC_TOTALREAD_BURSTSIN          (DC_REG_BASE + 0x0B20)
#define    AHB_GCREG_DEC_TOTALREAD_LASTSIN           (DC_REG_BASE + 0x0B30)
#define    AHB_GCREG_DEC_TOTALREAD_SOUT              (DC_REG_BASE + 0x0B38)
#define    AHB_GCREG_DEC_TOTALREAD_BURSTSOUT         (DC_REG_BASE + 0x0B40)
#define    AHB_GCREG_DEC_TOTALREAD_SREQOUT           (DC_REG_BASE + 0x0B48)
#define    AHB_GCREG_DEC_TOTALREAD_LASTSOUT          (DC_REG_BASE + 0x0B50)
#define    AHB_GCREG_DEC_DEBUG1                      (DC_REG_BASE + 0x0B5C)
#define    AHB_GCREG_DEC_DEBUG0                      (DC_REG_BASE + 0x0B58)
#define    AHB_GCREG_DEC_DEBUG2                      (DC_REG_BASE + 0x0B60)
#define    AHB_GCREG_DEC_DEBUG3                      (DC_REG_BASE + 0x0B64)
#define    AHB_GCREG_DEC_CONTROLEX                   (DC_REG_BASE + 0x0B68)
#define    AHB_GCREG_DEC_CLEAR0                      (DC_REG_BASE + 0x0B80)
#define    AHB_GCREG_DEC_READEX_CONFIG0              (DC_REG_BASE + 0x0C00)
#define    AHB_GCREG_DEC_READ_STRIDE0                (DC_REG_BASE + 0x0C80)
#define    AHB_GCREG_DEC_READ_BUFFEREND0             (DC_REG_BASE + 0x0E00)
//DC
#define    AHB_GCREG_FRAMEBUFFERADDR0                (DC_REG_BASE + 0x1400)
#define    AHB_GCREG_FRAMEBUFFERADDR1                (DC_REG_BASE + 0x1404)
#define    AHB_GCREG_FRAMEBUFFERSTRIDE0              (DC_REG_BASE + 0x1408)
#define    AHB_GCREG_FRAMEBUFFERSTRIDE1              (DC_REG_BASE + 0x140C)
#define    AHB_GCREG_DISPLAYDITHERCONFIG0            (DC_REG_BASE + 0x1410)
#define    AHB_GCREG_DISPLAYDITHERCONFIG1            (DC_REG_BASE + 0x1414)
#define    AHB_GCREG_PANEL_CONFIG1                   (DC_REG_BASE + 0x141C)
#define    AHB_GCREG_PANEL_CONFIG0                   (DC_REG_BASE + 0x1418)
#define    AHB_GCREG_DISPLAY_DITHERTABLELOW0         (DC_REG_BASE + 0x1420)
#define    AHB_GCREG_DISPLAY_DITHERTABLELOW1         (DC_REG_BASE + 0x1424)
#define    AHB_GCREG_DISPLAY_DITHERTABLEHIGH0        (DC_REG_BASE + 0x1428)
#define    AHB_GCREG_DISPLAY_DITHERTABLEHIGH1        (DC_REG_BASE + 0x142C)
#define    AHB_GCREG_HDISPLAY0                       (DC_REG_BASE + 0x1430)
#define    AHB_GCREG_HDISPLAY1                       (DC_REG_BASE + 0x1434)
#define    AHB_GCREG_HSYNC0                          (DC_REG_BASE + 0x1438)
#define    AHB_GCREG_HSYNC1                          (DC_REG_BASE + 0x143C)
#define    AHB_GCREG_VDISPLAY0                       (DC_REG_BASE + 0x1440)
#define    AHB_GCREG_VDISPLAY1                       (DC_REG_BASE + 0x1444)
#define    AHB_GCREG_VSYNC0                          (DC_REG_BASE + 0x1448)
#define    AHB_GCREG_VSYNC1                          (DC_REG_BASE + 0x144C)
#define    AHB_GCREG_DISPLAY_CURLOCATION0            (DC_REG_BASE + 0x1450)
#define    AHB_GCREG_DISPLAY_CURLOCATION1            (DC_REG_BASE + 0x1454)
#define    AHB_GCREG_GAMMAINDEX0                     (DC_REG_BASE + 0x1458)
#define    AHB_GCREG_GAMMAINDEX1                     (DC_REG_BASE + 0x145C)
#define    AHB_GCREG_GAMMADATA0                      (DC_REG_BASE + 0x1460)
#define    AHB_GCREG_GAMMADATA1                      (DC_REG_BASE + 0x1464)
#define    AHB_GCREG_CURSOR_CONFIG                   (DC_REG_BASE + 0x1468)
#define    AHB_GCREG_CURSOR_ADDR                     (DC_REG_BASE + 0x146C)
#define    AHB_GCREG_CURSOR_LOCATION                 (DC_REG_BASE + 0x1470)
#define    AHB_GCREG_CURSOR_BACKGROUND               (DC_REG_BASE + 0x1474)
#define    AHB_GCREG_CURSOR_FORGEGROUND              (DC_REG_BASE + 0x1478)
#define    AHB_GCREG_DISPLAY_INTR                    (DC_REG_BASE + 0x147C)
#define    AHB_GCREG_DISPLAY_ENABLE                  (DC_REG_BASE + 0x1480)
#define    AHB_GCREG_CURSOR_MODCLKGATECONTROL        (DC_REG_BASE + 0x1484)
#define    AHB_GCREG_DBI_CONTROL0                    (DC_REG_BASE + 0x1488)
#define    AHB_GCREG_DBI_CONTROL1                    (DC_REG_BASE + 0x148C)
#define    AHB_GCREG_DBI_IFRESET0                    (DC_REG_BASE + 0x1490)
#define    AHB_GCREG_DBI_IFRESET1                    (DC_REG_BASE + 0x1494)
#define    AHB_GCREG_DBI_WRITECHAR10                 (DC_REG_BASE + 0x1498)
#define    AHB_GCREG_DBI_WRITECHAR11                 (DC_REG_BASE + 0x149C)
#define    AHB_GCREG_DBI_WRITECHAR20                 (DC_REG_BASE + 0x14A0)
#define    AHB_GCREG_DBI_WRITECHAR21                 (DC_REG_BASE + 0x14A4)
#define    AHB_GCREG_DBI_CMD0                        (DC_REG_BASE + 0x14A8)
#define    AHB_GCREG_DBI_CMD1                        (DC_REG_BASE + 0x14AC)
#define    AHB_GCREG_GENERAL_CONFIG0                 (DC_REG_BASE + 0x14B0)
#define    AHB_GCREG_GENERAL_CONFIG1                 (DC_REG_BASE + 0x14B4)
#define    AHB_GCREG_DPI_CONFIG0                     (DC_REG_BASE + 0x14B8)
#define    AHB_GCREG_DPI_CONFIG1                     (DC_REG_BASE + 0x14BC)
#define    AHB_GCREG_DBI_TYPECCONFIG0                (DC_REG_BASE + 0x14C0)
#define    AHB_GCREG_DPI_TYPECCONFIG1                (DC_REG_BASE + 0x14C4)
#define    AHB_GCREG_DC_STATUS0                      (DC_REG_BASE + 0x14C8)
#define    AHB_GCREG_DC_STATUS1                      (DC_REG_BASE + 0x14CC)
#define    AHB_GCREG_DEBUGCOUNTER_SELECT0            (DC_REG_BASE + 0x14D0)
#define    AHB_GCREG_DEBUGCOUNTER_SELECT1            (DC_REG_BASE + 0x14D4)
#define    AHB_GCREG_DEBUGCOUNTER_VALUE0             (DC_REG_BASE + 0x14D8)
#define    AHB_GCREG_DEBUGCOUNTER_VALUE1             (DC_REG_BASE + 0x14DC)
#define    AHB_GCREG_FRAMEBUFFER_COLORKEY0           (DC_REG_BASE + 0x1508)
#define    AHB_GCREG_FRAMEBUFFER_COLORKEY1           (DC_REG_BASE + 0x150C)
#define    AHB_GCREG_FRAMEBUFFER_COLORKEYHIGH0       (DC_REG_BASE + 0x1510)
#define    AHB_GCREG_FRAMEBUFFER_COLORKEYHIGH1       (DC_REG_BASE + 0x1514)
#define    AHB_GCREG_FRAMEBUFFER_CONFIG0             (DC_REG_BASE + 0x1518)
#define    AHB_GCREG_FRAMEBUFFER_CONFIG1             (DC_REG_BASE + 0x151C)
#define    AHB_GCREG_FRAMEBUFFER_SCALECONFIG0        (DC_REG_BASE + 0x1520)
#define    AHB_GCREG_FRAMEBUFFER_SCALECONFIG1        (DC_REG_BASE + 0x1524)
#define    AHB_GCREG_FRAMEBUFFER_BGCOLOR0            (DC_REG_BASE + 0x1528)
#define    AHB_GCREG_FRAMEBUFFER_BGCOLOR1            (DC_REG_BASE + 0x152C)
#define    AHB_GCREG_FRAMEBUFFER_UPPLANARADDR0       (DC_REG_BASE + 0x1530)
#define    AHB_GCREG_FRAMEBUFFER_UPPLANARADDR1       (DC_REG_BASE + 0x1534)
#define    AHB_GCREG_FRAMEBUFFER_VPPLANARADDR0       (DC_REG_BASE + 0x1538)
#define    AHB_GCREG_FRAMEBUFFER_VPPLANARADDR1       (DC_REG_BASE + 0x153C)
#define    AHB_GCREG_OVERLAY_CONFIG0                 (DC_REG_BASE + 0x1540)
#define    AHB_GCREG_OVERLAY_CONFIG1                 (DC_REG_BASE + 0x1544)
#define    AHB_GCREG_OVERLAY_ALPHABLENDCONFIG0       (DC_REG_BASE + 0x1580)
#define    AHB_GCREG_OVERLAY_ALPHABLENDCONFIG1       (DC_REG_BASE + 0x1584)
#define    AHB_GCREG_OVERLAY_ADDR0                   (DC_REG_BASE + 0x15C0)
#define    AHB_GCREG_OVERLAY_ADDR1                   (DC_REG_BASE + 0x15C4)
#define    AHB_GCREG_OVERLAY_STRIDE0                 (DC_REG_BASE + 0x1600)
#define    AHB_GCREG_OVERLAY_STRIDE1                 (DC_REG_BASE + 0x1604)
#define    AHB_GCREG_OVERLAY_TL0                     (DC_REG_BASE + 0x1640)
#define    AHB_GCREG_OVERLAY_TL1                     (DC_REG_BASE + 0x1644)
#define    AHB_GCREG_OVERLAY_BR0                     (DC_REG_BASE + 0x1680)
#define    AHB_GCREG_OVERLAY_SRCGLOBALCOLOR0         (DC_REG_BASE + 0x16C0)
#define    AHB_GCREG_OVERLAY_BR1                     (DC_REG_BASE + 0x1684)
#define    AHB_GCREG_OVERLAY_SRCGLOBALCOLOR1         (DC_REG_BASE + 0x16C4)
#define    AHB_GCREG_OVERLAY_DSTGLOBALCOLOR0         (DC_REG_BASE + 0x1700)
#define    AHB_GCREG_OVERLAY_DSTGLOBALCOLOR1         (DC_REG_BASE + 0x1704)
#define    AHB_GCREG_OVERLAY_COLORKEY0               (DC_REG_BASE + 0x1740)
#define    AHB_GCREG_OVERLAY_COLORKEY1               (DC_REG_BASE + 0x1744)
#define    AHB_GCREG_OVERLAY_COLORKEYHIGH0           (DC_REG_BASE + 0x1780)
#define    AHB_GCREG_OVERLAY_COLORKEYHIGH1           (DC_REG_BASE + 0x1784)
#define    AHB_GCREG_OVERLAY_SIZE0                   (DC_REG_BASE + 0x17C0)
#define    AHB_GCREG_OVERLAY_SIZE1                   (DC_REG_BASE + 0x17C4)
#define    AHB_GCREG_FRAMEBUFFER_USTRIDE0            (DC_REG_BASE + 0x1800)
#define    AHB_GCREG_FRAMEBUFFER_USTRIDE1            (DC_REG_BASE + 0x1804)
#define    AHB_GCREG_FRAMEBUFFER_VSTRIDE0            (DC_REG_BASE + 0x1808)
#define    AHB_GCREG_FRAMEBUFFER_VSTRIDE1            (DC_REG_BASE + 0x180C)
#define    AHB_GCREG_FRAMEBUFFER_SIZE0               (DC_REG_BASE + 0x1810)
#define    AHB_GCREG_FRAMEBUFFER_SIZE1               (DC_REG_BASE + 0x1814)
#define    AHB_GCREG_INDEXCOLORTABLE_INDEX0          (DC_REG_BASE + 0x1818)
#define    AHB_GCREG_INDEXCOLORTABLE_INDEX1          (DC_REG_BASE + 0x181C)
#define    AHB_GCREG_INDEXCOLORTABLE_DATA0           (DC_REG_BASE + 0x1820)
#define    AHB_GCREG_INDEXCOLORTABLE_DATA1           (DC_REG_BASE + 0x1824)
#define    AHB_GCREG_FRAMEBUFFER_SCALEFACTORX0       (DC_REG_BASE + 0x1828)
#define    AHB_GCREG_FRAMEBUFFER_SCALEFACTORX1       (DC_REG_BASE + 0x182C)
#define    AHB_GCREG_FRAMEBUFFER_SCALEFACTORY0       (DC_REG_BASE + 0x1830)
#define    AHB_GCREG_FRAMEBUFFER_SCALEFACTORY1       (DC_REG_BASE + 0x1834)
#define    AHB_GCREG_HORIFILTERKERNEL_INDEX0         (DC_REG_BASE + 0x1838)
#define    AHB_GCREG_HORIFILTERKERNEL_INDEX1         (DC_REG_BASE + 0x183C)
#define    AHB_GCREG_OVERLAY_UPLANARADDR0            (DC_REG_BASE + 0x1840)
#define    AHB_GCREG_OVERLAY_UPLANARADDR1            (DC_REG_BASE + 0x1844)
#define    AHB_GCREG_OVERLAY_VPLANARADDR0            (DC_REG_BASE + 0x1880)
#define    AHB_GCREG_OVERLAY_VPLANARADDR1            (DC_REG_BASE + 0x1884)
#define    AHB_GCREG_OVERLAY_USTRIDE0                (DC_REG_BASE + 0x18C0)
#define    AHB_GCREG_OVERLAY_USTRIDE1                (DC_REG_BASE + 0x18C4)
#define    AHB_GCREG_OVERLAY_VSTRIDE0                (DC_REG_BASE + 0x1900)
#define    AHB_GCREG_OVERLAY_VSTRIDE1                (DC_REG_BASE + 0x1904)
#define    AHB_GCREG_OVERLAY_CLEARVALUE0             (DC_REG_BASE + 0x1940)
#define    AHB_GCREG_OVERLAY_CLEARVALUE1             (DC_REG_BASE + 0x1944)
#define    AHB_GCREG_OVERLAY_INDEXCOLORTABLE_INDEX0  (DC_REG_BASE + 0x1980)
#define    AHB_GCREG_OVERLAY_INDEXCOLORTABLE_INDEX1  (DC_REG_BASE + 0x1984)
#define    AHB_GCREG_OVERLAY_INDEXCOLORTABLE_DATA0   (DC_REG_BASE + 0x19C0)
#define    AHB_GCREG_OVERLAY_INDEXCOLORTABLE_DATA1   (DC_REG_BASE + 0x19C4)
#define    AHB_GCREG_HORI_FILTERKERNEL0              (DC_REG_BASE + 0x1A00)
#define    AHB_GCREG_HORI_FILTERKERNEL1              (DC_REG_BASE + 0x1A04)
#define    AHB_GCREG_VERTI_FILTERKERNEL_INDEX0       (DC_REG_BASE + 0x1A08)
#define    AHB_GCREG_VERTI_FILTERKERNEL0             (DC_REG_BASE + 0x1A10)
#define    AHB_GCREG_VERTI_FILTERKERNEL_INDEX1       (DC_REG_BASE + 0x1A0C)
#define    AHB_GCREG_VERTI_FILTERKERNEL1             (DC_REG_BASE + 0x1A14)
#define    AHB_GCREG_FRAMEBUFFER_CLEARVALUE0         (DC_REG_BASE + 0x1A18)
#define    AHB_GCREG_FRAMEBUFFER_CLEARVALUE1         (DC_REG_BASE + 0x1A1C)
#define    AHB_GCREG_FRAMEBUFFER_INITOFFSET0         (DC_REG_BASE + 0x1A20)
#define    AHB_GCREG_FRAMEBUFFER_INITOFFSET1         (DC_REG_BASE + 0x1A24)
#define    AHB_GCREG_MODCLKGATE_CONTROL0             (DC_REG_BASE + 0x1A28)
#define    AHB_GCREG_MODCLKGATE_CONTROL1             (DC_REG_BASE + 0x1A2C)
#define    AHB_GCREG_OVERLA_YSCALEFACTORX0           (DC_REG_BASE + 0x1A40)
#define    AHB_GCREG_OVERLA_YSCALEFACTORX1           (DC_REG_BASE + 0x1A44)
#define    AHB_GCREG_OVERLA_YSCALEFACTORY0           (DC_REG_BASE + 0x1A80)
#define    AHB_GCREG_OVERLA_YSCALEFACTORY1           (DC_REG_BASE + 0x1A84)
#define    AHB_GCREG_OVERLA_HFILKERNEL_INDEX0        (DC_REG_BASE + 0x1AC0)
#define    AHB_GCREG_OVERLA_HFILKERNEL_INDEX1        (DC_REG_BASE + 0x1AC4)
#define    AHB_GCREG_OVERLA_HFILKERNEL0              (DC_REG_BASE + 0x1B00)
#define    AHB_GCREG_OVERLA_HFILKERNEL1              (DC_REG_BASE + 0x1B04)
#define    AHB_GCREG_OVERLA_VFILKERNEL_INDEX0        (DC_REG_BASE + 0x1B40)
#define    AHB_GCREG_OVERLA_VFILKERNEL_INDEX1        (DC_REG_BASE + 0x1B44)
#define    AHB_GCREG_OVERLA_VFILKERNEL0              (DC_REG_BASE + 0x1B80)
#define    AHB_GCREG_OVERLA_VFILKERNEL1              (DC_REG_BASE + 0x1B84)
#define    AHB_GCREG_OVERLA_INITOFFSET0              (DC_REG_BASE + 0x1BC0)
#define    AHB_GCREG_OVERLA_INITOFFSET1              (DC_REG_BASE + 0x1BC4)
#define    AHB_GCREG_OVERLA_SCALECONFIG0             (DC_REG_BASE + 0x1C00)
#define    AHB_GCREG_OVERLA_SCALECONFIG1             (DC_REG_BASE + 0x1C04)
#define    AHB_GCREG_DPCONFIG0                       (DC_REG_BASE + 0x1CD0)

#define PHY_ALIGN(Data, Offset)             ((Data + Offset - 1) & ~(Offset - 1))

VOID
HwFramebufferReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwFramebufferSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwFramebufferSetStride (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwFramebufferSetAddress (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwCursorSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN INT8              Num
  );

VOID
HwCursorSetPos (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwCursorSetColor (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwCursorSetAddress (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwDisplaySetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwDisplaySetSync (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwPanelSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwDpSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwDitherEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwGammaSetConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwInterruptEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           enable
  );

UINT32
HwInterruptGet (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwVersionGet (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  );

VOID
DcHwSetOverlay(
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

VOID
HwShadowRegisterPendingEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Enable
  );

UINT32
HwFlipInProgressValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );
#endif
