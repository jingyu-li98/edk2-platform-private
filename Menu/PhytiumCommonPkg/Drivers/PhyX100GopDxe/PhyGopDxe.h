/* @file
** PhyGopDxe.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef  _PHY_GOP_DXE_H_
#define  _PHY_GOP_DXE_H_

#include <IndustryStandard/Pci.h>
#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/PciIo.h>

#include "Dc/DcStru.h"
#include "Dc/DcType.h"
#include "Dp/DpType.h"
#include "PhyGraphicsOutput.h"

#define PCI_BAR_FRAMEBUFFER           2
#define PCI_BAR_MMIO                  0

#define VENDOR_ID_PHY                 0x1DB7
#define DEVICE_ID_PHY                 0xDC22
#define PHY_GRAPHICS_OUTPUT_MODE_COUNT    9
#define PHY_DEFAULT_MODE                  0

#define EDID_USE

//#define  REG_DEBUG
//#define  DPCD_DEBUG

#define  DPSINK_HOTPLUG_CHECK_INTERVAL       (2000*10000)      //1s
//
// PHY Private Data Structure
//
#define PHY_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('P', 'G', 'O', 'P')

extern EFI_DRIVER_BINDING_PROTOCOL gPhyDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gPhyComponentName;

//
// BIOS video child handle private data Structure
//
#define BIOS_VIDEO_DEV_SIGNATURE    SIGNATURE_32 ('P', 'G', 'O', 'P')

typedef struct _DTD_TABLE {
  UINT32  PixelClock;
  UINT32  HorPixel;
  UINT32  VerPixel;
  DC_SYNC DcSync;
  DP_SYNC DpSync;
} DTD_TABLE;

typedef struct _DTD_LIST {
  UINT8     DtdListNum;
#define  DTD_LIST_MAX_NUM  4
  DTD_TABLE DtdTable[DTD_LIST_MAX_NUM];
} DTD_LIST;

typedef struct _PHY_PRIVATE_DATA {
  UINT64                          Signature;
  EFI_HANDLE                      Handle;
  EFI_PCI_IO_PROTOCOL             *PciIo;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    GraphicsOutput;
  //
  // GOP Private Data
  //
  EFI_DEVICE_PATH_PROTOCOL        *GopDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *UgaDevicePath;
  UINT64                          OriginalPciAttributes;
  EFI_EVENT                       DpSinkHotPlugEvent;
  BOOLEAN                         HardwareNeedsStarting;
  PHY_GOP_MODE                    ModeData[PHY_GOP_MAX_MODENUM];
  DC_SYNC                         DcSync[PHY_GOP_MAX_MODENUM];
  DP_SYNC                         DpSync[PHY_GOP_MAX_MODENUM];
  VOID                            *LineBuffer;
  UINT64                          BufferAddr;
  UINT64                          ShadowBuffer;
  UINT8                           MaxMode;
  UINT8                           ModeNum;
  UINT32                          DpIsHpdOn[3];
  UINT32                          DpIsTrained[3];
  UINT32                          DpConnect[3];
  UINT32                          DpIsUsed[3];
  UINT8                           IsEdp[3];
  UINT8                           EdpBgLightOn[3];
  UINT8                           EdpLight;
  UINT8                           EdpNeedInit[3];
  DP_CONFIG                       DpConfig[3];
  DC_CORE                         DcCore[3];
  DTD_LIST                        DtdList[3];
  UINT8                           DownSpreadEnable[3];
} PHY_PRIVATE_DATA;

#define PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS(a) CR(a, PHY_PRIVATE_DATA, GraphicsOutput, PHY_PRIVATE_DATA_SIGNATURE)


UINT32
PhyReadMmio (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINTN             Offset
  );

VOID
PhyWriteMmio (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINTN            Offset,
  IN UINT32           Data
  );

UINT32
PhyPciRead (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32            Offset
  );

EFI_STATUS
PhyPciWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32            Offset,
  IN UINT32            Pci
  );

VOID
PhyFramebufferWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT64            Offset,
  IN UINTN             Len,
  IN VOID              *Buffer
  );

VOID
PhyFramebufferRead (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT64            Offset,
  IN  UINTN             Len,
  OUT VOID              *Buffer
  );


EFI_STATUS
PhyGraphicsOutputConstructor (
  IN OUT PHY_PRIVATE_DATA *Private
  );

EFI_STATUS
PhyGraphicsOutputDestructor (
  IN OUT PHY_PRIVATE_DATA *Private
  );

EFI_STATUS
EFIAPI
PhyControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PhyControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_STATUS
EFIAPI
PhyControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );
#endif
