/* @file
** DcStru.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _DC_STRU_H_
#define  _DC_STRU_H_

typedef  struct {
  UINT16 HTotal;
  UINT16 HDisEnd;
  UINT16 HStart;
  UINT16 HEnd;
  BOOLEAN HPolarity;  //0:negative    1:positive
  UINT16 VTotal;
  UINT16 VDisEnd;
  UINT16 VStart;
  UINT16 VEnd;
  BOOLEAN VPolarity;  //0:negative    1:positive
} DC_SYNC;

typedef struct {
  UINT32  YAddress;
  UINT32  UAddress;
  UINT32  VAddress;
  UINT32  UStride;
  UINT32  VStride;
  UINT32  RotAngle;
  UINT32  AlphaMode;
  UINT32  AlphaValue;
#define  LUT_MAX  256
  UINT32  Lut[LUT_MAX];
  /* Original size in pixel before rotation and scale. */
  UINT32  Width;
  UINT32  Height;
  UINT32  TileMode;
  UINT32  Scale;
  UINT32  ScaleFactorX;
  UINT32  ScaleFactorY;
  UINT32  FilterTap;
  UINT32  HorizontalFilterTap;
#define  HOR_KERNEL_MAX  128
  UINT32  HorKernel[128];
#define  VER_KERNEL_MAX  128
  UINT32  VerKernel[128];
  UINT32  Swizzle;
  UINT32  UvSwizzle;
  UINT32  ColorKey;
  UINT32  ColorKeyHigh;
  UINT32  BgColor;
  UINT32  Transparency;
  UINT32  ClearFB;
  UINT32  ClearValue;
  UINT32  InitialOffsetX;
  UINT32  InitialOffsetY;
  UINT32  Compressed;
} DC_FRAMEBUFFER;

typedef struct {
  INT32   Mode;
  INT32   SrcAlphaValue;
  UINT32  SrcAlphaMode;
  UINT32  SrcGlobalAlphaValue;
  UINT32  SrcGlobalAlphaMode;
  UINT32  Enable;
  UINT32  Format;

  UINT32  Logical[3];
  UINT64  Address[3];
  UINT32  TileStatusAddress[3];
  UINT32  Stride[3];
  UINT32  TlX;
  UINT32  TlY;
  UINT32  BrX;
  UINT32  BrY;

  UINT32  RotAngle;
  UINT32  Width;
  UINT32  Height;
  UINT32  AlignedWidth[3];
  UINT32  AlignedHeight[3];

  UINT32  Swizzle;
  UINT32  UvSwizzle;

  UINT32  ColorKey;
  UINT32  ColorKeyHigh;
  UINT32  Transparency;

  UINT32  DstAlphaMode;
  UINT32  DstAlphaValue;

  UINT32  DstGlobalAlphaMode;
  UINT32  DstGlobalAlphaValue;

  UINT32  ClearOverlay;
  UINT32  ClearValue;

  UINT32  TileMode;

  UINT32  Lut[256];

  UINT32  Compressed;

  UINT32  Scale;
  UINT32  ScaleFactorX;
  UINT32  ScaleFactorY;
  UINT32  FilterTap;
  UINT32  HorizontalFilterTap;
  UINT32  HorKernel[128];
  UINT32  VerKernel[128];

  UINT32  InitialOffsetX;
  UINT32  InitialOffsetY;
} DC_OVERLAY;

typedef struct {
  UINT32  ChipId;
  UINT32  ChipInfo;
  UINT32  ChipRevision;
  UINT32  ChipPatchRevision;
  UINT32  ProductId;
  UINT32  EcoId;
  UINT32  CustomerId;
  UINT32  ProductDate;
  UINT32  ProductTime;
} DC_HARDWARE;

typedef struct {
  BOOLEAN Enable;
  UINT64  PhysAddr;
  UINT32  Type;
  UINT32  X;
  UINT32  Y;
  UINT32  HotX;
  UINT32  HotY;
  UINT32  BgColor;
  UINT32  FgColor;
  BOOLEAN Dirty;
} DC_CURSOR;

typedef struct {
  /* display */
  UINT32  DpyHline;
  UINT32  DpyHtotal;
  UINT32  DpyVline;
  UINT32  DpyVtotal;
  UINT32  DpyHsyncStart;
  UINT32  DpyHsyncEnd;
  UINT32  DpyVsyncStart;
  UINT32  DpyVsyncEnd;
  BOOLEAN DpyHsyncPolarity;
  BOOLEAN DpyVsyncPolarity;
  BOOLEAN DisplayDirty;

  /* framebuffer */
  UINT64  FbPhysAddr;
  UINT32  FbFormat;
  UINT32  FbTiling;
  UINT32  FbYuvType;
  UINT32  FbStride;
  UINT32  FbAstride;
  BOOLEAN FbDirty;

  /* panel */
  BOOLEAN PanelDepPolarity;
  BOOLEAN PanelDapPolarity;
  BOOLEAN PanelClockpPolarity;
  BOOLEAN PanelDirty;

/* gamma correction */
#define GAMMA_INDEX_MAX 256
  BOOLEAN GammaEnable;
  UINT32  Gamma[GAMMA_INDEX_MAX][3];
  BOOLEAN GammaDirty;

  /* dither */
  BOOLEAN DitherEnable;
  UINT32  DitherRedChannel;
  UINT32  DitherGreenChannel;
  UINT32  DitherBlueChannel;
  UINT32  DitherTableLow;
  UINT32  DitherTableHigh;
  BOOLEAN DitherDirty;

  /* output */
  BOOLEAN OutputEnable;
  UINT32  OutputType;
  UINT32  OutputDpFormat;
  UINT32  OutputDbiType;
  UINT32  OutputDbiFormat;
  UINT32  OutputDbiActime;
  UINT32  OutputDbiPeriod[2];
  UINT32  OutputDbiEorAssert[2];
  UINT32  OutputDbiCsAssert[2];
  BOOLEAN OutputDbiPolarity;
  UINT32  OutputDpiFormat;
  BOOLEAN OutputDirty;

  /*video framebuffer*/
  DC_FRAMEBUFFER Framebuffer;

  /* Overlay. */
  DC_OVERLAY Overlay;
  BOOLEAN OverlayDirty;
} DC_DISPLAY;

typedef struct {
  UINT32  ColorL;
  UINT32  ColorH;
} CLEAR_COLOR;

typedef struct {
  UINT8  YVal;
  UINT8  UvVal;
} YUV_VAL;

typedef struct {
  UINT16  ProductCode;
  UINT16  BranchCode;
  UINT16  VersionCode;
  UINT16  ScalableCoreCode;
  UINT16  ConfigCode;
  UINT32  IPChangelistCode;
} FBDC_CORE_INFO;

typedef struct {
  UINT8  Blue;
  UINT8  Green;
  UINT8  Red;
  UINT8  Alpha;
} CH0123_VAL;

typedef struct {
  UINT32  AddrStart;
  UINT32  AddrEnd;
} DCREQ_ADDR;

typedef struct {
  UINT8  IsLossy;               //0-no lossy, 1-lossy
  UINT8  TileType;              //0-reserved, 1-8*8, 2-16*4, 3-32*2
  UINT8  ArgbSwizzle;           //0x0000-ARGB
                                //0x0001-ARBG
                                //0x0010-AGRB
                                //0x0011-AGBR
                                //0x0100-ABGR
                                //0x0110:0x0111-reserved
                                //0x1000-RGBA
                                //0x1001-RBGA
                                //0x1010-GRBA
                                //0x1011-GBRA
                                //0x1100-BGRA
                                //0x1101-BRGA
                                //0x1110:0x1111-reserved
  UINT8  ColorFormat;           //0x0E-ARGB2101010
                                //0x29-BGRA8888
                                //0x02-A4R4G4B4
                                //0x04-A1R5G5B5
                                //0x05-A5G6B5
                                //0x59-YUV422-VYUY
                                //0x5B-YUV422-YVYU
  UINT8  Mode;                  //1-tile,0-linear
} LAYER_CONFIG;

typedef struct {
  DCREQ_ADDR      DcreqAddr[2];
  LAYER_CONFIG    LayerConfig[2];
  CLEAR_COLOR     ClearColor[2];
  CH0123_VAL      Ch0123Val[2];
  YUV_VAL         YuvVal[2];
  UINT8           FreqSel;
  FBDC_CORE_INFO  FbdcCoreInfo;
  UINT8           DcreqEnable;
} DCREQ_CONFIG;


typedef struct {
  UINT8         DisType;
  UINT8         DcNum;
  /* hardware */
  DC_HARDWARE   Hardware;
  /* cursor */
  DC_CURSOR     Cursor;
  //DISPLAY
  DC_DISPLAY    Display;
  DCREQ_CONFIG  DcreqConfig;
} DC_CORE;
#endif
