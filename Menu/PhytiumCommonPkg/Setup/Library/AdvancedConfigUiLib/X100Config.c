/** @file
Some functions about X100.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/PciIo.h>
#include <Protocol/Smbios.h>

#include "AdvancedConfig.h"
#include "AdvancedConfigFormGuid.h"

/**
  Check whether X100 exists.

  @retval   1     Existed.
  @retval   0     Not existed.
**/
UINT32
IsX100Existed (
  VOID
  )
{
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;
  UINT32                      Status;
  UINT32                      Exist;

  NumOfPciHandles = 0;
  HandleBuffer = NULL;
  Exist = 0;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return 0;
  }
  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      if ((DeviceID == 0xdc3a) && (VendorID == 0x1db7)) {
        Exist = 1;
        break;
      }
    }
  }
  if(HandleBuffer != NULL){
    FreePool(HandleBuffer);
  }

  return Exist;
}

/**
  Get X100 firmware version and build time.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetX100BaseInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  EFI_STATUS                  Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT32                      Index;
  UINT16                      DeviceID;
  UINT16                      VendorID;
  UINT8                       SeVersion;
  UINT8                       ThirdVersion;
  UINT32                      Utc;
  EFI_TIME                    BuildTime;

  NumOfPciHandles = 0;
  SeVersion = 0;
  HandleBuffer = NULL;
  Status = EFI_SUCCESS;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  //get x100 se soft version
  //
  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      if ((DeviceID == 0xdc20) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x2f,
                     1,
                     &SeVersion
                     );
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x2e,
                     1,
                     &ThirdVersion
                     );
        SetHiiString (
           Handle,
           STRING_TOKEN(STR_X100_SE_VERSION_VALUE),
           L"V%d.%d.%d",
           (SeVersion >> 4) & 0xF,
           (SeVersion >> 0) & 0xF,
           ThirdVersion
           );

      }
      if ((DeviceID == 0xdc35) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint32,
                     0x2c,
                     1,
                     &Utc
                     );
        Utc += 8 * 60 *60;
        EpochToEfiTime ((UINTN)Utc, &BuildTime);
        SetHiiString (
          Handle,
          STRING_TOKEN (STR_X100_SE_BUILDTIME_VALUE),
          L"%02d/%02d/%04d %02d:%02d:%02d",
          BuildTime.Month,
          BuildTime.Day,
          BuildTime.Year,
          BuildTime.Hour,
          BuildTime.Minute,
          BuildTime.Second
          );
        DEBUG ((DEBUG_INFO, "SE Build Time : Year : %d\nMonth : %d\nDay : %d\nHour : %d\nMinute : %d\nSecond : %d\n",
                    BuildTime.Year, BuildTime.Month, BuildTime.Day, BuildTime.Hour, BuildTime.Minute, BuildTime.Second));
      }
    }
  }
  if(HandleBuffer != NULL){
    FreePool(HandleBuffer);
  }
  return Status;
}


/**
  Create X100 Config menu in the page.

  @param[in]    HiiHandle           The hii handle for the Uiapp driver.
  @param[in]    StartOpCodeHandle   The opcode handle to save the new opcode.
**/
VOID
CreateX100ConfigMenu (
  IN EFI_HII_HANDLE              HiiHandle,
  IN VOID                        *StartOpCodeHandle
  )
{
  HiiCreateGotoOpCode (
    StartOpCodeHandle,
    FORM_X100CONFIG_ID,
    STRING_TOKEN (STR_FORM_X100CONFIG_TITLE),
    STRING_TOKEN (STR_FORM_X100CONFIG_HELP),
    EFI_IFR_FLAG_CALLBACK,
    FORM_X100CONFIG_ID
    );
}
