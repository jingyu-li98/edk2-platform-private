/* @file
** PhyGraphicsOutput.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _PHY_GRAPHICS_OUTPUT_H_
#define  _PHY_GRAPHICS_OUTPUT_H_

#define    PHY_GOP_640_480        0
#define    PHY_GOP_800_600        1
#define    PHY_GOP_1024_768       2
#define    PHY_GOP_1280_720       3
#define    PHY_GOP_1366_768       4
#define    PHY_GOP_1920_1080      5
#define    PHY_GOP_2560_1440      6
#define    PHY_GOP_2560_1600      7
#define    PHY_GOP_1600_1200      8
#define    PHY_GOP_1920_1200      9
#define    PHY_GOP_1280_800       10
#define    PHY_GOP_800_480        11
#define    PHY_GOP_1280_1024      12
#define    PHY_GOP_3840_2160      13

#define    PHY_GOP_MAX_MODENUM    14
#define    DPDC_PATH_NUM          3
//#define    MAIN_RAM_USE           1

#define    VRAM_SIZE              ( 512 * 1024 * 1024 )
#define    MRAM_SIZE              (3840 * 2160 * 4)
#define    MRAM_PAGES             ((MRAM_SIZE / 4096 ) + 1)

#define    INIT_WIDTH             1024
#define    INIT_HEIGHT            768
#define    INIT_RES_INIT

#define    EDP_LIGHT_VARIABLE    L"X100EdpLightVar"
#define    EDP_LIGHT_VARIABLE_GUID \
  {0xc7d004ae, 0xba39, 0x11ec, {0xba, 0x2c, 0x97, 0xe4, 0x29, 0xdc, 0xca, 0x13}}

#define    EDP_LIGHT_VARIABLE_FLAG  (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#define    EDP_LIGHT_DEFAULT     50

typedef struct _PHY_GOP_MODE {
  UINT32  Width;
  UINT32  Height;
  UINT32  ColorDepth;
  UINT32  RefreshRate;
  UINT32  ModeName;
} PHY_GOP_MODE;

#endif
