/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _DC_STRU_H_
#define  _DC_STRU_H_

typedef  struct {
  UINT16  HTotal;
  UINT16  HDisEnd;
  UINT16  HStart;
  UINT16  HEnd;
  BOOLEAN HPolarity;  //0:negative    1:positive
  UINT16  VTotal;
  UINT16  VDisEnd;
  UINT16  VStart;
  UINT16  VEnd;
  BOOLEAN VPolarity;  //0:negative    1:positive
} DC_SYNC;

typedef struct {
  UINT64  YAddress;
  UINT64  UAddress;
  UINT64  VAddress;
  UINT32  UStride;
  UINT32  VStride;
  UINT32  RotAngle;
  UINT32  AlphaMode;
  UINT32  AlphaValue;
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

} DC_DISPLAY;

typedef struct {
  UINT8         DisType;
  UINT8         DcNum;
  //
  //cursor
  //
  DC_CURSOR     Cursor;
  //
  //DISPLAY
  //
  DC_DISPLAY    Display;
} DC_CORE;
#endif
