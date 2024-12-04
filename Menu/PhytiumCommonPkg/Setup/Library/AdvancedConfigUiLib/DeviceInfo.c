/** @file
This file defines a device information formset include pcie, usb and hdd information.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Guid/MdeModuleHii.h>
#include <Guid/MemoryTypeInformation.h>

#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Nvme.h>
#include <IndustryStandard/Scsi.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <Library/PrintLib.h>

#include <Protocol/DiskInfo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/NvmExpressPassthru.h>
#include <Protocol/PciIo.h>
#include <Protocol/Smbios.h>
#include <Protocol/UsbIo.h>
#include <Protocol/Usb2HostController.h>

#include "AdvancedConfigFormGuid.h"
#include "AdvancedConfig.h"
#include "DeviceInfo.h"

#define MAX_DEVICE 20

typedef struct
{
    UINT16 VendorID;
    UINT16 DeviceID;
    UINT8  DeviceClass;
    UINT8  DeviceSubClass;
} USB_DEVICE_LIST;

USB_DEVICE_LIST mAllUsbDeviceList[MAX_DEVICE] = {0};


CONST  UINT16  mUsbLangId = 0x0409;  //English

STATIC CHAR8 *mPcieClassCodeList [] = {
  "Pre Pci 2.0",
  "Mass storage controller",
  "Network Controller",
  "Display Controller",
  "Multimedis Device",
  "Memory Controller",
  "Bridge Device",
  "Simple Communications Controllers",
  "Base System Peripherals",
  "Input Devices",
  "Docking Stations",
  "Processorts",
  "Serial Bus Controllers",
};

STATIC CHAR16 *mUsbDeviceClassCodeList [] = {
  L"[Interface Descriptors]",          //00
  L"[Audio]",                          //01
  L"[Communcations & CDC]",            //02
  L"[HID]",                            //03
  L"[Reserved]",                       //04
  L"[Physical]",                       //05
  L"[Image]",                          //06
  L"[Printer]",                        //07
  L"[Mass Storage]",                   //08
  L"[Hub]",                            //09
  L"[CDC-Data]",                       //0A
  L"[Smart Card]",                     //0B
  L"[Reserved]",                       //0C
  L"[Content Security]",               //0D
  L"[Video]",                          //0E
  L"[Personal Healthcare]",            //0F
  L"[Audio/Video Devices]",            //10
  L"[Billboard]",                      //11
  L"[USB Type-C]",                     //12
};

STATIC CHAR8 *mHddTypeList [] = {
  "AHCI",
  "IDE",
  "NVME",
};

CONST CHAR16 *mUsbDeviceClass3C = L"[I3C]";
CONST CHAR16 *mUsbDeviceClassDC = L"[Diagostic]";
CONST CHAR16 *mUsbDeviceClassE0 = L"[Wireless Controller]";
CONST CHAR16 *mUsbDeviceClassEF = L"[Miscellaneous]";
CONST CHAR16 *mUsbDeviceClassFE = L"[Application]";
CONST CHAR16 *mUsbDeviceClassFF = L"[Vendor Spec]";
CONST CHAR16 *mUsbDeviceClassReserved = L"[REserved]";

STATIC CHAR8 *mUsbControllerMaxSpeedList [] = {
  "UHCI",
  "EHCI",
  "XHCI",
};

/**
  Bytes swap.

  @param[in]  Data    Data to swap.
  @param[in]  Size    Size of the data to swap.
**/
VOID
SwapEntries (
  IN CHAR8    *Data,
  IN UINT16   Size
  )
{
  UINT16  Index;
  CHAR8   Temp8;

  for (Index = 0; (Index + 1) < Size; Index += 2) {
    Temp8           = Data[Index];
    Data[Index]     = Data[Index + 1];
    Data[Index + 1] = Temp8;
  }
}

/**
  Convert USB speed to protocol type emun.

  @param[in]  Speed    USB speed.

  #retval   Data     USB protocol emun.
 **/
STATIC
UINT8
MaxPseedToProtocol (
  UINT8  Speed
  )
{
  UINT8  Data;

  switch (Speed) {
  case EFI_USB_SPEED_FULL:
  case EFI_USB_SPEED_LOW:
    Data = 0;
    break;
  case EFI_USB_SPEED_HIGH:
    Data = 1;
    break;
  case EFI_USB_SPEED_SUPER:
    Data = 2;
    break;
  default:
    Data = 0;
    break;
  }
  return Data;
}

/**
  Convert pcie class code to corresponding string.

  @param[in]  ClassCode       Pcie main class code.

  #retval   Deiscription    Pcie main class code description string.
 **/
CHAR16 *
UsbDeviceClassToString (
  IN UINT8  ClassCode
  )
{
  UINT16      *Description;
  UINT32      StringLen;

  switch (ClassCode) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
  case 0x0A:
  case 0x0B:
  case 0x0C:
  case 0x0D:
  case 0x0E:
  case 0x0F:
  case 0x10:
  case 0x11:
  case 0x12:
    StringLen = StrSize (mUsbDeviceClassCodeList[ClassCode]);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassCodeList[ClassCode]);
    break;
  case 0x3C:
    StringLen = StrSize (mUsbDeviceClass3C);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClass3C);
    break;
  case 0xDC:
    StringLen = StrSize (mUsbDeviceClassDC);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassDC);
    break;
  case 0xE0:
    StringLen = StrSize (mUsbDeviceClassE0);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassE0);
    break;
  case 0xEF:
    StringLen = StrSize (mUsbDeviceClassEF);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassEF);
    break;
  case 0xFE:
    StringLen = StrSize (mUsbDeviceClassFE);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassFE);
    break;
  case 0xFF:
    StringLen = StrSize (mUsbDeviceClassFF);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassFF);
    break;
  default:
    StringLen = StrSize (mUsbDeviceClassReserved);
    Description = AllocateZeroPool (StringLen);
    ASSERT (Description != NULL);
    StrCpyS (Description, StringLen / sizeof(CHAR16), mUsbDeviceClassReserved);
    break;
  }

  return Description;
}

/**
  Eliminate the extra spaces in the char8 Str to one space.

  @param[in]    Str     Input string info.
**/
VOID
EliminateExtraSpacesChar8 (
  IN CHAR8                    *Str
  )
{
  UINTN                        Index;
  UINTN                        ActualIndex;

  for (Index = 0, ActualIndex = 0; Str[Index] != '\0'; Index++) {
    if ((Str[Index] != ' ') || ((ActualIndex > 0) && (Str[ActualIndex - 1] != ' '))) {
      Str[ActualIndex++] = Str[Index];
    }
  }

  if (Str[ActualIndex - 1] == ' ') {
    ActualIndex--;
  }
  Str[ActualIndex] = '\0';
}

/**
  Eliminate the extra spaces in the char16 Str to one space.

  @param[in]    Str     Input string info.
**/
VOID
EliminateExtraSpaces (
  IN CHAR16                    *Str
  )
{
  UINTN                        Index;
  UINTN                        ActualIndex;

  for (Index = 0, ActualIndex = 0; Str[Index] != L'\0'; Index++) {
    if ((Str[Index] != L' ') || ((ActualIndex > 0) && (Str[ActualIndex - 1] != L' '))) {
      Str[ActualIndex++] = Str[Index];
    }
  }
  Str[ActualIndex] = L'\0';
}


/**
  Get the pcie device information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval     EFI_SUCCESS  Success.
  @retval     Other        Failed.
**/
EFI_STATUS
GetPcieDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  EFI_STATUS                  Status;
  UINT8                       Index;
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartLabel;
  EFI_IFR_GUID_LABEL          *EndLabel;
  UINTN                       HandleCount;
  EFI_HANDLE                  *HandleBuffer;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINTN                       Segment;
  UINTN                       Bus;
  UINTN                       Device;
  UINTN                       Function;
  UINT16                      VenderId;
  UINT16                      DeviceId;
  UINT8                       BaseClass;
  STRING_REF                  StrRef;
  STRING_REF                  StrRef1;

  StrRef = 0;
  StrRef1 = 0;
  HandleBuffer = NULL;
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_FORM_PCIEDEVICEINFO_START;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_FORM_PCIEDEVICEINFO_END;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  StrRef = SetHiiString (Handle, 0, L"Seg Bus Dev Fun");
  StrRef1 = SetHiiString (Handle, 0, L"Device Vendor Class");
  HiiCreateTextOpCode (StartOpCodeHandle, StrRef, STRING_TOKEN (STR_NONE), StrRef1);
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    ASSERT_EFI_ERROR (Status);
    PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 0, 1, &VenderId);
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 2, 1, &DeviceId);
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 11, 1, &BaseClass);

    if (BaseClass <= PCIE_CLASSCODE_VALID) {
      StrRef1 = SetHiiString(
         Handle,
         0,
         L" %04x   %04x   %02x-%a",
         VenderId,
         DeviceId,
         BaseClass,
         mPcieClassCodeList[BaseClass]
         );
    }
    else if (BaseClass <= PCIE_CLASSCODE_RESERVED) {
      StrRef1 = SetHiiString(
         Handle,
         0,
         L" %04x   %04x   %02x-Reserved",
         VenderId,
         DeviceId,
         BaseClass
         );
    }
    else {
      StrRef1 = SetHiiString(
         Handle,
         0,
         L" %04x   %04x   %02x-Misc",
         VenderId,
         DeviceId,
         BaseClass
         );
    }

    StrRef = SetHiiString(
       Handle,
       0,
       L"%02x  %02x  %02x  %02x",
       Segment, Bus, Device, Function
       );

    HiiCreateTextOpCode (
      StartOpCodeHandle,
      StrRef,
      STRING_TOKEN (STR_NONE),
      StrRef1
      );
  }

  Status = HiiUpdateForm (Handle, NULL, FORM_PCIEDEVICEINFO_ID, StartOpCodeHandle, EndOpCodeHandle);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  if (HandleBuffer != NULL){
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

/**
  Get the usb controller information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval   EFI_SUCCESS  Success.
  @retval   Other        Failed.
**/
EFI_STATUS
GetUsbControllerInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  UINT32                      Status;
  UINTN                       NumOfPciHandles;
  EFI_HANDLE                  *HandleBuffer;
  EFI_USB2_HC_PROTOCOL        *Usb2Hc;
  UINT32                      Index;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  UINT8                       MaxSpeed;
  UINT8                       PortNum;
  UINT8                       Is64Bit;
  CHAR16                      *DevicePathStr;
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartLabel;
  EFI_IFR_GUID_LABEL          *EndLabel;
  STRING_REF                  StrRef;
  STRING_REF                  StrRef1;
  UINT32                      HcIndex;

  Status = 0;
  NumOfPciHandles = 0;
  HandleBuffer = NULL;

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_FORM_USBCONTROLLERINFO_START;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_FORM_USBCONTROLLERINFO_END;

  StrRef = SetHiiString (Handle, 0, L"Index Capability");
  StrRef1 = SetHiiString (Handle, 0, L"DevicePath");
  HiiCreateTextOpCode (StartOpCodeHandle, StrRef, STRING_TOKEN (STR_NONE), StrRef1);
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiUsb2HcProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  DEBUG ((DEBUG_INFO, "Usb 2Hc handles number is %d\n", NumOfPciHandles));

  HcIndex = 0;
  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiUsb2HcProtocolGuid,
                    (VOID **)&Usb2Hc
                    );
    if (!EFI_ERROR (Status)) {
      Usb2Hc->GetCapability (Usb2Hc, &MaxSpeed, &PortNum, &Is64Bit);
      Status = gBS->HandleProtocol (
                          HandleBuffer[Index],
                          &gEfiDevicePathProtocolGuid,
                          (VOID **)&DevicePath
                          );
      if (!EFI_ERROR(Status)) {
        HcIndex++;
        DevicePathStr = ConvertDevicePathToText (DevicePath, 1, 1);
        StrRef = SetHiiString(
           Handle,
           0,
           L" %02d   Max Port:%d [%a]",
           Index,
           PortNum,
           mUsbControllerMaxSpeedList[MaxPseedToProtocol (MaxSpeed)]
           );

        StrRef1 = SetHiiString(Handle, 0, DevicePathStr);
        HiiCreateTextOpCode (
          StartOpCodeHandle,
          StrRef,
          STRING_TOKEN (STR_NONE),
          StrRef1
          );

        if (DevicePathStr != NULL) {
          FreePool (DevicePathStr);
        }
      }
    }
  }

  Status = HiiUpdateForm (Handle, NULL, FORM_USBDEVICEINFO_ID, StartOpCodeHandle, EndOpCodeHandle);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  if (HandleBuffer != NULL){
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

/*
  USB Device List just list differrent USB device 
 */

UINT8 
DoUsbDeviceInList (
  IN UINT16  UsbVendor,
  IN UINT16  UsbDevice,
  IN UINT8   UsbClass,
  IN UINT8   UsbSubClass
  )
{
  UINT8 Index = 0;
  for(Index=0; Index<MAX_DEVICE;Index++)
  {
    if((mAllUsbDeviceList[Index].VendorID == 0)&&(mAllUsbDeviceList[Index].DeviceID == 0))
    {
       mAllUsbDeviceList[Index].VendorID = UsbVendor;
       mAllUsbDeviceList[Index].DeviceID = UsbDevice;
       mAllUsbDeviceList[Index].DeviceClass = UsbClass;
       mAllUsbDeviceList[Index].DeviceSubClass = UsbSubClass;
       return 1;
    }
    else if((mAllUsbDeviceList[Index].VendorID == UsbVendor)&&(mAllUsbDeviceList[Index].DeviceID == UsbDevice))
    {
	//E2000 USB Port used as USB Hub,it have more than one device
        if((UsbVendor != 0x1D6B)&&(UsbDevice !=0x0104))
	  return 0;
	else
	{
           if((mAllUsbDeviceList[Index].DeviceClass == UsbClass)&&(mAllUsbDeviceList[Index].DeviceSubClass == UsbSubClass))
	     return 0;
	}
    }
  }
  return 0;
}



/**
  Get the usb device information and put to formset.

  @param[in]  Handle       HII Handle.

  @retval   EFI_SUCCESS  Success.
  @retval   Other        Failed.
**/
EFI_STATUS
GetUsbDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  EFI_STATUS                    Status;
  UINT8                         Index;
  VOID                          *StartOpCodeHandle;
  VOID                          *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL            *StartLabel;
  EFI_IFR_GUID_LABEL            *EndLabel;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  STRING_REF                    StrRef;
  STRING_REF                    StrRef1;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_DEVICE_DESCRIPTOR     DevDesc;
  CHAR16                        NullChar;
  CHAR16                        *Manufacturer;
  CHAR16                        *Product;
  CHAR16                        *SerialNumber;
  CHAR16                        *ClassCode;
  CHAR16                        *Description;
  UINTN                         DescMaxSize;
  UINT32                        DeviceIndex;
  UINT32                        SerialNumLen;
  UINT8                         DeviceClass;
  UINT8                         DeviceSubClass;
  UINT8                         DeviceProtocol;
  EFI_USB_INTERFACE_DESCRIPTOR  IfDesc;

  StrRef = 0;
  StrRef1 = 0;
  NullChar = L'\0';
  HandleCount = 0;
  HandleBuffer = NULL;

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  ZeroMem (mAllUsbDeviceList, sizeof(USB_DEVICE_LIST)*MAX_DEVICE);

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_FORM_USBDEVICEINFO_START;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_FORM_USBDEVICEINFO_END;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiUsbIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  StrRef = SetHiiString (Handle, 0, L"Index Vendor Product Class Protocol");
  StrRef1 = SetHiiString (Handle, 0, L"Description");
  HiiCreateTextOpCode (StartOpCodeHandle, StrRef, STRING_TOKEN (STR_NONE), StrRef1);

  DeviceIndex = 0;
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiUsbIoProtocolGuid,
                    (VOID **)&UsbIo
                    );
    ASSERT_EFI_ERROR (Status);

    Status = UsbIo->UsbGetDeviceDescriptor (UsbIo, &DevDesc);
    if (EFI_ERROR (Status)) {
      continue;
    }
    DeviceClass = DevDesc.DeviceClass;
    DeviceSubClass = DevDesc.DeviceSubClass;
    DeviceProtocol = DevDesc.DeviceProtocol;

    if (DeviceClass == 0) {
      //
      // If Class in Device Descriptor is set to 0, use the Class, SubClass and
      // Protocol in Interface Descriptor instead.
      //
      Status = UsbIo->UsbGetInterfaceDescriptor (UsbIo, &IfDesc);
      if (EFI_ERROR (Status)) {
        continue;
      }

      DeviceClass    = IfDesc.InterfaceClass;
      DeviceSubClass = IfDesc.InterfaceSubClass;
      DeviceProtocol = IfDesc.InterfaceProtocol;
    }

    Status = UsbIo->UsbGetStringDescriptor (
                      UsbIo,
                      mUsbLangId,
                      DevDesc.StrManufacturer,
                      &Manufacturer
                      );
    if (EFI_ERROR (Status)) {
      Manufacturer = &NullChar;
    }

    Status = UsbIo->UsbGetStringDescriptor (
                      UsbIo,
                      mUsbLangId,
                      DevDesc.StrProduct,
                      &Product
                      );
    if (EFI_ERROR (Status)) {
      Product = &NullChar;
    }

    Status = UsbIo->UsbGetStringDescriptor (
                      UsbIo,
                      mUsbLangId,
                      DevDesc.StrSerialNumber,
                      &SerialNumber
                      );
    if (EFI_ERROR (Status)) {
      SerialNumber = &NullChar;
    }

    if ((Manufacturer == &NullChar) &&
        (Product == &NullChar) &&
        (SerialNumber == &NullChar)
        ) {
      continue;
    }

    ClassCode = UsbDeviceClassToString (DeviceClass);

    DeviceIndex++;
    SerialNumLen = StrSize (SerialNumber) >= 34 ? 34 : StrSize (SerialNumber);
    DescMaxSize = StrSize (ClassCode) + StrSize (Manufacturer) +
                    StrSize (Product) + SerialNumLen;
    Description = AllocateZeroPool (DescMaxSize);
    ASSERT (Description != NULL);
    StrCatS (Description, DescMaxSize/sizeof(CHAR16), ClassCode);
    StrCatS (Description, DescMaxSize/sizeof(CHAR16), L" ");

    StrCatS (Description, DescMaxSize/sizeof(CHAR16), Manufacturer);
    StrCatS (Description, DescMaxSize/sizeof(CHAR16), L" ");

    StrCatS (Description, DescMaxSize/sizeof(CHAR16), Product);
    StrCatS (Description, DescMaxSize/sizeof(CHAR16), L" ");

    StrnCatS (Description, DescMaxSize/sizeof(CHAR16),  SerialNumber, (SerialNumLen - 2) / 2);

    if (Manufacturer != &NullChar) {
      FreePool (Manufacturer);
    }
    if (Product != &NullChar) {
      FreePool (Product);
    }
    if (SerialNumber != &NullChar) {
      FreePool (SerialNumber);
    }
    if (ClassCode != &NullChar) {
      FreePool (ClassCode);
    }

    EliminateExtraSpaces (Description);

    if(!DoUsbDeviceInList(DevDesc.IdVendor, DevDesc.IdProduct, DeviceClass, DeviceSubClass))
    {
      if (Description != &NullChar) {
         FreePool (Description);
      }
      continue;
    }

    StrRef = SetHiiString(
       Handle,
       0,
       L" %02d   %04x   %04x    %02x-%02x   %02x",
       DeviceIndex,
       DevDesc.IdVendor,
       DevDesc.IdProduct,
       DeviceClass,
       DeviceSubClass,
       DeviceProtocol
       );

    StrRef1 = SetHiiString(
       Handle,
       0,
       Description
       );

    HiiCreateTextOpCode (
      StartOpCodeHandle,
      StrRef,
      STRING_TOKEN (STR_NONE),
      StrRef1
      );
    if (Description != &NullChar) {
      FreePool (Description);
    }
  }

  Status = HiiUpdateForm (Handle, NULL, FORM_USBDEVICEINFO_ID, StartOpCodeHandle, EndOpCodeHandle);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  if (HandleBuffer != NULL){
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

/**
  Get a hdd device information and put information to formset.

  @param[in]    Handle         HII Handle.
  @param[in]    StartOpHandle  The opcode handle to save the new opcode.
  @param[in]    Info           Hdd information struct.
  @param[in]    Num            Current device index.
**/
VOID
AtaInfoToForm (
  IN EFI_HII_HANDLE  Handle,
  IN VOID            *StartOpHandle,
  IN HDD_INFO        *Info,
  IN UINT8           Num
  )
{
  STRING_REF                  StrRef;
  CHAR16                      *DevicePathStr;

  //
  //title
  //
  StrRef = SetHiiString(Handle, 0, L"[%d]%a", Num, Info->ModuleName);
  HiiCreateSubTitleOpCode (StartOpHandle, StrRef, 0, 0, 0);
  //
  //type
  //
  StrRef = SetHiiString(Handle, 0, L"%a", mHddTypeList[Info->Type]);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_TYPE),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  //
  //Size GB
  /* sunrui - HDD capacity
  StrRef = SetHiiString(Handle, 0, L"%d GB", Info->SizeGB);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_SIZEGB),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  */
  //Serial No
  //
  StrRef = SetHiiString(Handle, 0, L"%a", Info->SerialNo);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_SERIALNO),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  //
  //Module Name
  //
  StrRef = SetHiiString(Handle, 0, L"%a", Info->ModuleName);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_MODULENAME),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  //
  //Firmware Version
  //
  StrRef = SetHiiString(Handle, 0, L"%a", Info->FirmwareVer);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_FWVERSION),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  //
  //DevicePath
  //
  DevicePathStr = ConvertDevicePathToText (Info->DevicePath, 1, 1);
  StrRef = SetHiiString (Handle, 0, DevicePathStr);
  HiiCreateTextOpCode (
    StartOpHandle,
    STRING_TOKEN (STR_HDD_DEVICEPATH),
    STRING_TOKEN (STR_NONE),
    StrRef
    );
  if (DevicePathStr != NULL) {
    FreePool (DevicePathStr);
  }
}

/**
  Get the Hdd information and put to formset.

  @param[in]  Handle         HII Handle.
  @param[in]  StartOpHandle  The opcode handle to save the new opcode.
  @param[in]  Num            Current device index.

  @retval   Num            Current total device num.
**/
UINT32
GetAtaDeviceInfo (
  IN EFI_HII_HANDLE  Handle,
  IN VOID            *StartOpHandle,
  IN UINT32          Num
  )
{
  UINTN                                    Index;
  EFI_STATUS                               Status;
  EFI_DISK_INFO_PROTOCOL                   *DiskInfo;
  UINT32                                   BufferSize;
  EFI_ATA_IDENTIFY_DATA                    IdentifyData;
  UINTN                                    HandleCount;
  EFI_HANDLE                               *HandleBuffer;
  HDD_INFO                                 HddInfo;
  UINT32                                   DriveSizeInGB;
  UINT64                                   DriveSizeInBytes;
  UINT64                                   RemainderInBytes;
  UINT32                                   IdeChannel;
  UINT32                                   SataPortIndex;
  UINT64                                   NumSectors;
  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL       *NvmePassthru;
  EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET CommandPacket;
  EFI_NVM_EXPRESS_COMMAND                  Command;
  EFI_NVM_EXPRESS_COMPLETION               Completion;
  NVME_ADMIN_CONTROLLER_DATA               ControllerData;

  NumSectors = 0;
  DriveSizeInGB = 0;
  HandleCount = 0;
  HandleBuffer = NULL;
  DriveSizeInBytes = 0;
  RemainderInBytes = 0;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDiskInfoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Num;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDiskInfoProtocolGuid,
                    (VOID **) &DiskInfo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid) ||
        CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid)) {
      Status = DiskInfo->WhichIde (
                 DiskInfo,
                 &IdeChannel,
                 &SataPortIndex
                 );
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "No Disk!\n"));
        continue;
      }
      BufferSize   = sizeof (EFI_ATAPI_IDENTIFY_DATA);
      Status = DiskInfo->Identify (
                           DiskInfo,
                           &IdentifyData,
                           &BufferSize
                           );
      if (EFI_ERROR (Status)) {
        continue;
      }
      ZeroMem (&HddInfo, sizeof (HDD_INFO));
      //
      //firmware version, model name and serial number
      //
      CopyMem (HddInfo.FirmwareVer, IdentifyData.FirmwareVer, 8);
      SwapEntries (HddInfo.FirmwareVer, 8);
      EliminateExtraSpacesChar8 (HddInfo.FirmwareVer);
      CopyMem (HddInfo.ModuleName, IdentifyData.ModelName, 40);
      SwapEntries (HddInfo.ModuleName, 40);
      EliminateExtraSpacesChar8 (HddInfo.ModuleName);
      CopyMem (HddInfo.SerialNo, IdentifyData.SerialNo, 20);
      SwapEntries (HddInfo.SerialNo, 20);
      EliminateExtraSpacesChar8 (HddInfo.SerialNo);
      //
      //Size GB
      //
      if ((!(IdentifyData.config & ATAPI_DEVICE)) || (IdentifyData.config == 0x848A)) {
        if (IdentifyData.command_set_supported_83 & _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED) {
          NumSectors = *(UINT64 *)&IdentifyData.maximum_lba_for_48bit_addressing;
        }
        else {
          NumSectors = IdentifyData.user_addressable_sectors_lo + (IdentifyData.user_addressable_sectors_hi << 16) ;
        }
        DriveSizeInBytes = MultU64x32 (NumSectors, 512);
        //
        // DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Binary GB = 2^30 bytes)
        // DriveSizeInGB = (UINT32) Div64(DriveSizeInBytes, (1 << 30), &RemainderInBytes);
        // Convert the Remainder, which is in bytes, to number of tenths of a Binary GB.
        // NumTenthsOfGB = GetNumTenthsOfGB(RemainderInBytes);
        // DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Decimal GB = 10^9 bytes)
        //
        DriveSizeInGB = (UINT32) DivU64x64Remainder (DriveSizeInBytes, 1000000000, &RemainderInBytes);
      }
      else {
        DriveSizeInGB = 0;
      }
      HddInfo.SizeGB = DriveSizeInGB;
      //
      //Type AHCI or IDE
      //
      if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid)) {
        HddInfo.Type = TYPE_AHCI;
      } else {
        HddInfo.Type = TYPE_IDE;
      }
      //
      //devicepath
      //
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      (VOID **)&HddInfo.DevicePath
                      );
      AtaInfoToForm (Handle, StartOpHandle, &HddInfo, Num);
      Num++;
    } else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoScsiInterfaceGuid)) {
    //
    //todo
    //
    }
    else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid)) {
      Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID **) &HddInfo.DevicePath);
      if (EFI_ERROR (Status)) {
        continue;
      }
      Status = gBS->LocateDevicePath (&gEfiNvmExpressPassThruProtocolGuid, &HddInfo.DevicePath, &HandleBuffer[Index]);
      if (EFI_ERROR (Status) ||  (DevicePathType (HddInfo.DevicePath) != MESSAGING_DEVICE_PATH) ||
           (DevicePathSubType (HddInfo.DevicePath) != MSG_NVME_NAMESPACE_DP)) {
      //
      // Do not return description when the Handle is not a child of NVME controller.
      //
        continue;
      }
      //
      // Send ADMIN_IDENTIFY command to NVME controller to get the model and serial number.
      //
      Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiNvmExpressPassThruProtocolGuid, (VOID **) &NvmePassthru);
      ASSERT_EFI_ERROR (Status);
      ZeroMem (&CommandPacket, sizeof(EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
      ZeroMem (&Command, sizeof(EFI_NVM_EXPRESS_COMMAND));
      ZeroMem (&Completion, sizeof(EFI_NVM_EXPRESS_COMPLETION));
      Command.Cdw0.Opcode = NVME_ADMIN_IDENTIFY_CMD;
      //
      // According to Nvm Express 1.1 spec Figure 38, When not used, the field shall be cleared to 0h.
      // For the Identify command, the Namespace Identifier is only used for the Namespace data structure.
      //
      Command.Nsid = 0;
      CommandPacket.NvmeCmd = &Command;
      CommandPacket.NvmeCompletion = &Completion;
      CommandPacket.TransferBuffer = &ControllerData;
      CommandPacket.TransferLength = sizeof (ControllerData);
      CommandPacket.CommandTimeout = EFI_TIMER_PERIOD_SECONDS (5);
      CommandPacket.QueueType  = NVME_ADMIN_QUEUE;
      //
      // Set bit 0 (Cns bit) to 1 to identify a controller
      //
      Command.Cdw10  = 1;
      Command.Flags  = CDW10_VALID;

      Status = NvmePassthru->PassThru (
                               NvmePassthru,
                               0,
                               &CommandPacket,
                               NULL
                               );
      if (EFI_ERROR (Status)) {
        continue;
      }

      ZeroMem (&HddInfo, sizeof (HDD_INFO));

      CopyMem (HddInfo.ModuleName, ControllerData.Mn, 40);
      EliminateExtraSpacesChar8 (HddInfo.ModuleName);
      CopyMem (HddInfo.SerialNo, ControllerData.Sn, 20);
      EliminateExtraSpacesChar8 (HddInfo.SerialNo);
      CopyMem (HddInfo.FirmwareVer, ControllerData.Fr, 8);
      EliminateExtraSpacesChar8 (HddInfo.FirmwareVer);
      DriveSizeInBytes = *((UINT64*)ControllerData.Tnvmcap);
      DriveSizeInGB = DivU64x64Remainder (DriveSizeInBytes, 1000000000, &RemainderInBytes);
      HddInfo.SizeGB = DriveSizeInGB;
      HddInfo.Type = TYPE_NVME;
      //
      //devicepath
      //
      Status = gBS->HandleProtocol (
                          HandleBuffer[Index],
                          &gEfiDevicePathProtocolGuid,
                          (VOID **)&HddInfo.DevicePath
                          );
      AtaInfoToForm (Handle, StartOpHandle, &HddInfo, Num);
      Num++;
    }
  }

  return Num;
}

/**
  Get hdd device list and put to formset.

  @param[in]    Handle         HII Handle.

  @retval   EFI_SUCCESS    Success.
  @retval   Other          Failed.
**/
EFI_STATUS
GetHddDeviceInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  UINT32                      Status;
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartLabel;
  EFI_IFR_GUID_LABEL          *EndLabel;
  UINT32                      HddIndex;

  Status = 0;
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_FORM_HDDDEVICEINFO_START;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_FORM_HDDDEVICEINFO_END;

  HddIndex = 0;
  //
  //ATA/ATAPI and NVME Device
  //
  HddIndex = GetAtaDeviceInfo (Handle, StartOpCodeHandle, HddIndex);
  Status = HiiUpdateForm (Handle, NULL, FORM_HDDINFO_ID, StartOpCodeHandle, EndOpCodeHandle);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  return Status;
}
