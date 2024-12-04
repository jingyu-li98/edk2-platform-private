/* @file
** DcType.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DC_TYPE_H_
#define    _DC_TYPE_H_

typedef enum {
  ROT_0 = 0,
  FLIP_X,
  FLIP_Y,
  FLIP_XY,
  ROT_90,
  ROT_180,
  ROT_270,
} ROT_TYPE;

typedef enum {
  DP_RGB565 = 0,
  DP_RGB666,
  DP_RGB888,
  DP_RGB101010,
} DP_FORMAT;

typedef enum {
  LINEAR = 0,
  TILED4_4,
  SUPER_TILED_XMAJOR,
  SUPER_TILED_YMAJOR,
  TiLE_MODE0,
  TiLE_MODE1,
  TiLE_MODE2,
  TiLE_MODE3,
  TiLE_MODE4,
  TiLE_MODE5,
  TiLE_MODE6,
  SUPER_TILED_XMAJOR8_4,
  SUpER_TILED_YMAJOR4_8,
  TILE_Y,
  TILE_MODE7,
  TILED_128_1,
  TIlED_256_1
} BUFFER_TILEMODE;

typedef enum {
  FORMAT_X4R4G4B4 = 0x0,
  FORMAT_A4R4G4B4,
  FORMAT_X1R5G5B5,
  FORMAT_A1R5G5B5,
  FORMAT_R5G6B5,
  FORMAT_X8R8G8B8,
  FORMAT_A8R8G8B8,
  FORMAT_YUY2,
  FORMAT_UYVY,
  FORMAT_INDEX8,
  FORMAT_MONOCHROME,
  FORMAT_YV12 = 0x0F,
  FORMAT_A8,
  FORMAT_NV12,
  FORMAT_NV16,
  FORMAT_RG16,
  FORMAT_RB,
  FORMAT_NV12_10BIT,
  FORMAT_A2R10G10B10,
  FORMAT_NV16_10BIT,
  FORMAT_INDEX1,
  FORMAT_INDEX2,
  FORMAT_INDEX4,
  FORMAT_P010
} VIDEO_FORMAT;

typedef enum {
  DCREQ_A4R4G4B4 = 0x02,
  DCREQ_A1R5G5B5 = 0x04,
  DCREQ_R5G6B5 = 0x05,
  DCREQ_ARGB2101010 = 0x0E,
  DCREQ_BGRA8888 = 0x29,
  DCREQ_YUV422_VYUY = 0x59,
  DCREQ_YUV422_YVYU = 0x5B
} DCREQ_COLOR_FORMAT;

typedef enum {
  TILETYPE_8_8 = 0x1,
  TILETYPE_16_4,
  TILETYPE_32_2
} DCREQ_TILETYPE;

//transparency mode
typedef enum {
  TRANS_OPAQUE = 0,
  TRANS_MASK,
  TRANS_KEY
} TRANSPARENCY_MODE;

/*
* Frame buffer mode.
* Used in phy_conf_framebuffer_set_config()
*/
typedef enum {
  PhyLINEAR,
  PhyTILED,
} PHY_TILING_TYPE;

typedef enum {
  PhyDPI,
  PhyDP,
} PHY_OUTPUT_TYPE;
/*
* Cursor type.
* Used in viv_conf_cursor_set_type()
*/
typedef enum {
  PhyCURSOR_DISABLE,
  PhyCURSOR_MASK,
  PhyCURSOR_ARGB,
} PHY_CURSOR_TYPE;

#define PHY_FALSE                0
#define PHY_TRUE                 1

#define PHY_MAX(a, b) ((a) > (b) ? (a) : (b))
#define PHY_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
