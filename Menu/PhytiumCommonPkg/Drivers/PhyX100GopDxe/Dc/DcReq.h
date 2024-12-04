/* @file
** DcReq.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _DC_REQ_H_
#define  _DC_REQ_H_
#include   "../PhyGopDxe.h"

#define    DC_REQ_BASE             0x2000

#define    DCREQ_ADDR_START00       (DC_REQ_BASE + 0x00)
#define    DCREQ_ADDR_END00         (DC_REQ_BASE + 0x04)
#define    DCREQ_ADDR_START10       (DC_REQ_BASE + 0x08)
#define    DCREQ_ADDR_END10         (DC_REQ_BASE + 0x0C)
#define    DCREQ_LAYER0_CFG         (DC_REQ_BASE + 0x10)
#define    DCREQ_LAYER1_CFG         (DC_REQ_BASE + 0x14)
#define    DCREQ_CLEARCOLOR0_L      (DC_REQ_BASE + 0x18)
#define    DCREQ_CLEARCOLOR0_H      (DC_REQ_BASE + 0x1C)
#define    DCREQ_CLEARCOLOR1_L      (DC_REQ_BASE + 0x20)
#define    DCREQ_CLEARCOLOR1_H      (DC_REQ_BASE + 0x24)
#define    DCREQ_CH0123_VAL0        (DC_REQ_BASE + 0x28)
#define    DCREQ_CH0123_VAL1        (DC_REQ_BASE + 0x2C)
#define    DCREQ_YUV_VAL0           (DC_REQ_BASE + 0x30)
#define    DCREQ_YUV_VAL1           (DC_REQ_BASE + 0x34)
#define    DCREQ_PIXELCLK_CONFIG    (DC_REQ_BASE + 0x38)
#define    DCREQ_FBDC_POWER         (DC_REQ_BASE + 0x3C)
#define    DCREQ_FBDC_BP            (DC_REQ_BASE + 0x40)
#define    DCREQ_FBDC_NV            (DC_REQ_BASE + 0x44)
#define    DCREQ_FBDC_C             (DC_REQ_BASE + 0x48)
#define    DCREQ_FBDC_IP            (DC_REQ_BASE + 0x4C)
#define    FRAME_ADDR_PREFIX        (DC_REQ_BASE + 0x50)
#define    DCREQ_FILTER             (DC_REQ_BASE + 0x58)
#define    DCREQ_ENABLE             (DC_REQ_BASE + 0x5c)
#define    DCREQ_RESET              (DC_REQ_BASE + 0x68)
#define    DCREQ_DCLPI_ENABLE       (DC_REQ_BASE + 0x6c)

VOID
GetFBDCInfo (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT FBDC_CORE_INFO   *Info
  );

VOID
SetDcreqConfig (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DCREQ_CONFIG      *Config
  );

UINT32
HeaderCal (
  IN UINT32  Width,
  IN UINT32  Height,
  IN UINT32  TileMode
  );

UINT32
DcreqChangePixel (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            PixelClk
  );

EFI_STATUS
EdpPowerControl (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             PowerOn
  );

EFI_STATUS
EdpBgLightControl (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             LightOpen,
  IN UINT8             LightLevel
  );

EFI_STATUS
DcDpReset (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );
#endif
