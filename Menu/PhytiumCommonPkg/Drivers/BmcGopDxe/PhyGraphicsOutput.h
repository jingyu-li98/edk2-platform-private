/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _PHY_GRAPHICS_OUTPUT_H_
#define  _PHY_GRAPHICS_OUTPUT_H_

#define  PHY_GOP_640_480        0
#define  PHY_GOP_800_600        1
#define  PHY_GOP_1024_768       2
#define  PHY_GOP_1280_720       3
#define  PHY_GOP_1366_768       4
#define  PHY_GOP_1920_1080      5
#define  PHY_GOP_1600_1200      6
#define  PHY_GOP_1280_800       7
#define  PHY_GOP_800_480        8
#define  PHY_GOP_1280_1024      9

#define  PHY_GOP_MAX_MODENUM    10
#define  DPDC_PATH_NUM          1
//#define  MAIN_RAM_USE           1

#define  MRAM_SIZE              (3840 * 2160 * 4)
#define  MRAM_PAGES             ((MRAM_SIZE / 4096 ) + 1)

#define  INIT_WIDTH             1024
#define  INIT_HEIGHT            768
#define  INIT_RES_INIT

#define  MAIN_MEMORY_START      0x80000000
#define  MAIN_MEMORY_END        0xFFFFFFFF
#define  MAIN_MEMORY_SIZE       (MAIN_MEMORY_END - MAIN_MEMORY_START + 1)

#define  DCDP_CONFIG_VAR        L"DcDpConfigVar"
#define  DCDP_CONFIG_VAR_GUID   \
    {0x6d9da7de, 0xed1a, 0x11ec, {0x8e, 0x95, 0x97, 0x7f, 0x3c, 0x38, 0x22, 0x8c}}
#define  DCDP_CONFIG_VAR_FLAG   \
    (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)

typedef struct _PHY_GOP_MODE {
  UINT32  Width;
  UINT32  Height;
  UINT32  ColorDepth;
  UINT32  RefreshRate;
  UINT32  ModeName;
} PHY_GOP_MODE;

#endif
