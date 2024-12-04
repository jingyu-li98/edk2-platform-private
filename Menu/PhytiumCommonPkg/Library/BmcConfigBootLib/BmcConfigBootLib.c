/** @file
Support BMC Set Boot Type Command.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright (c) 2017, Hisilicon Limited. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <string.h>
#include <Uefi.h>
#include <Guid/GlobalVariable.h>

#include <IndustryStandard/IpmiNetFnChassis.h>
#include <IpmiCommandLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BmcConfigBootLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/LoadedImage.h>

#define BBS_TYPE_MENU           0xFE

/**
  Set Boot Info Acknowledge to notify BMC that the Boot Flags has been handled by UEFI.

  @retval EFI_SUCCESS                    The command byte stream was successfully submit to the device.
  @retval other                          Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiSetBootInfoAck (
  VOID
  )
{
  EFI_STATUS                             Status;
  IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4 *ParameterData;
  IPMI_SET_BOOT_OPTIONS_REQUEST          *SetBootOptionsRequest;
  UINT32                                 SetBootOptionsRequestSize;
  UINT8                                  CompletionCode;

  CompletionCode = 0;
  SetBootOptionsRequestSize = sizeof (IPMI_SET_BOOT_OPTIONS_REQUEST) \
                              + sizeof (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4);
  SetBootOptionsRequest = AllocateZeroPool (SetBootOptionsRequestSize);
  if (SetBootOptionsRequest == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetBootOptionsRequest->ParameterValid.Bits.ParameterSelector = IPMI_BOOT_OPTIONS_PARAMETER_BOOT_INFO_ACK;
  SetBootOptionsRequest->ParameterValid.Bits.MarkParameterInvalid = 0x0;

  ParameterData = (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4 *)&SetBootOptionsRequest->ParameterData;
  ParameterData->WriteMask = BIT0;
  ParameterData->BootInitiatorAcknowledgeData = 0x0;
  Status = IpmiSetSystemBootOptions (
             SetBootOptionsRequest,
             SetBootOptionsRequestSize,
             &CompletionCode
             );
  FreePool (SetBootOptionsRequest);
  return Status;
}

/**
  Retrieve Boot Info Acknowledge from BMC.

  @param[out] BootInitiatorAcknowledgeData  Pointer to returned buffer.

  @retval EFI_SUCCESS                     The command byte stream was successfully submit to the device.
  @retval EFI_INVALID_PARAMETER           BootInitiatorAcknowledgeData was NULL.
  @retval other                           Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiGetBootInfoAck (
  OUT UINT8 *BootInitiatorAcknowledgeData
  )
{
  EFI_STATUS                              Status;
  IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4  *ParameterData;
  IPMI_GET_BOOT_OPTIONS_RESPONSE          *GetBootOptionsResponse;
  UINT8                                   GetBootOptionsResponseSize;

  if (BootInitiatorAcknowledgeData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GetBootOptionsResponseSize = sizeof (IPMI_GET_BOOT_OPTIONS_RESPONSE) \
                               + sizeof (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4);
  GetBootOptionsResponse = AllocateZeroPool (GetBootOptionsResponseSize);
  if (GetBootOptionsResponse == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ParameterData = (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_4 *)&GetBootOptionsResponse->ParameterData;

  Status = IpmiGetSystemBootOptions (
             IPMI_BOOT_OPTIONS_PARAMETER_BOOT_INFO_ACK,
             GetBootOptionsResponse,
             &GetBootOptionsResponseSize
             );
  if (!EFI_ERROR (Status)) {
    *BootInitiatorAcknowledgeData = ParameterData->BootInitiatorAcknowledgeData;
  }

  FreePool (GetBootOptionsResponse);
  return Status;
}

/**
  Send command to clear BMC Boot Flags parameter.

  @retval EFI_SUCCESS                    The command byte stream was successfully submit to the device.
  @retval other                          Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiClearBootFlags (
  VOID
  )
{
  EFI_STATUS                              Status;
  IPMI_SET_BOOT_OPTIONS_REQUEST           *SetBootOptionsRequest;
  UINT32                                  SetBootOptionsRequestSize;
  UINT8                                   CompletionCode;

  CompletionCode = 0;
  SetBootOptionsRequestSize = sizeof (IPMI_SET_BOOT_OPTIONS_REQUEST) \
                              + sizeof (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5);
  SetBootOptionsRequest = AllocateZeroPool (SetBootOptionsRequestSize);
  if (SetBootOptionsRequest == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetBootOptionsRequest->ParameterValid.Bits.ParameterSelector = IPMI_BOOT_OPTIONS_PARAMETER_BOOT_FLAGS;

  Status = IpmiSetSystemBootOptions (
             SetBootOptionsRequest,
             SetBootOptionsRequestSize,
             &CompletionCode
             );

  FreePool (SetBootOptionsRequest);
  return Status;
}

/**
  Retrieve Boot Flags parameter from BMC.

  @param[out] BootFlags                    Pointer to returned buffer.

  @retval EFI_SUCCESS                      The command byte stream was successfully submit to the device.
  @retval EFI_INVALID_PARAMETER            BootFlags was NULL.
  @retval other                            Failed to write data to the device.

**/
EFI_STATUS
EFIAPI
IpmiGetBootFlags (
  OUT IPMI_BOOT_FLAGS_INFO *BootFlags
  )
{
  EFI_STATUS                              Status;
  IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5  *ParameterData;
  IPMI_GET_BOOT_OPTIONS_RESPONSE          *GetBootOptionsResponse;
  UINT8                                   GetBootOptionsResponseSize;

  if (BootFlags == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GetBootOptionsResponseSize = sizeof (IPMI_GET_BOOT_OPTIONS_RESPONSE) \
                               + sizeof (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5);
  GetBootOptionsResponse = AllocateZeroPool (GetBootOptionsResponseSize);
  if (GetBootOptionsResponse == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ParameterData = (IPMI_BOOT_OPTIONS_RESPONSE_PARAMETER_5 *)&GetBootOptionsResponse->ParameterData;

  Status = IpmiGetSystemBootOptions (
             IPMI_BOOT_OPTIONS_PARAMETER_BOOT_FLAGS,
             GetBootOptionsResponse,
             &GetBootOptionsResponseSize
             );

  if (!EFI_ERROR (Status)) {
    BootFlags->IsPersistent = ParameterData->Data1.Bits.PersistentOptions;
    BootFlags->IsBootFlagsValid = ParameterData->Data1.Bits.BootFlagValid;
    BootFlags->DeviceSelector = ParameterData->Data2.Bits.BootDeviceSelector;
    BootFlags->InstanceSelector = ParameterData->Data5.Bits.DeviceInstanceSelector;
  }
  FreePool (GetBootOptionsResponse);

  return Status;
}

/**
  Retrieve Boot Flags parameter from BMC.

  @param[in]  UsbPathTxt                      Ponter the Usb Path.
  @param[in]  FileSysPathTxt                  Pointer to File System path.
  @param[in]  FileSysPath                     Pointer to EFI_DEVICE_PATH_PROTOCOL.

  @retval BBS_TYPE_UNKNOWN                    Unknown Device Type.
  @retval BBS_TYPE_CDROM                      CD ROM Decice Type.
  @retval other                               Refer to DeviceType definitions.

**/
UINT16
EFIAPI
GetBBSTypeFromFileSysPath (
  IN CHAR16                   *UsbPathTxt,
  IN CHAR16                   *FileSysPathTxt,
  IN EFI_DEVICE_PATH_PROTOCOL *FileSysPath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *Node;

  if (StrnCmp (UsbPathTxt, FileSysPathTxt, StrLen (UsbPathTxt)) == 0) {
    Node = FileSysPath;
    while (!IsDevicePathEnd (Node)) {
      if ((DevicePathType (Node) == MEDIA_DEVICE_PATH) &&
          (DevicePathSubType (Node) == MEDIA_CDROM_DP)) {
        return BBS_TYPE_CDROM;
      }
      Node = NextDevicePathNode (Node);
    }
  }

  return BBS_TYPE_UNKNOWN;
}

/**
  Get BBS Type form a Usb device path.

  @param[in]    UsbPath              Current node of the Usb device path.

  @retval  UINT16                    Return BBS Type.

**/
UINT16
EFIAPI
GetBBSTypeFromUsbPath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL *UsbPath
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *FileSystemHandles;
  UINTN                             NumberFileSystemHandles;
  UINTN                             Index;
  EFI_DEVICE_PATH_PROTOCOL          *FileSysPath;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL  *DevPathToText;
  CHAR16                            *UsbPathTxt;
  CHAR16                            *FileSysPathTxt;
  UINT16                            Result;

  Status = gBS->LocateProtocol (
                  &gEfiDevicePathToTextProtocolGuid,
                  NULL,
                  (VOID **) &DevPathToText);
  if (EFI_ERROR (Status)) {
    return BBS_TYPE_UNKNOWN;
  }

  Result = BBS_TYPE_UNKNOWN;
  UsbPathTxt = DevPathToText->ConvertDevicePathToText (UsbPath, TRUE, TRUE);
  if (UsbPathTxt == NULL) {
    return Result;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberFileSystemHandles,
                  &FileSystemHandles
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Locate SimpleFileSystemProtocol error(%r)\n", Status));
    FreePool (UsbPathTxt);
    return BBS_TYPE_UNKNOWN;
  }

  for (Index = 0; Index < NumberFileSystemHandles; Index++) {
    FileSysPath = DevicePathFromHandle (FileSystemHandles[Index]);
    FileSysPathTxt = DevPathToText->ConvertDevicePathToText (FileSysPath, TRUE, TRUE);

    if (FileSysPathTxt == NULL) {
      continue;
    }

    Result = GetBBSTypeFromFileSysPath (UsbPathTxt, FileSysPathTxt, FileSysPath);
    FreePool (FileSysPathTxt);

    if (Result != BBS_TYPE_UNKNOWN) {
      break;
    }
  }

  if (NumberFileSystemHandles != 0) {
    FreePool (FileSystemHandles);
  }

  FreePool (UsbPathTxt);

  return Result;
}

/**
  Get BBS Type form a messaging device path.

  @param[in]    DevicePath           Current node of the messaging device path.
  @param[in]    Node                 Current node of the messaging device path.

  @retval  UINT16                    Return BBS Type.

**/
UINT16
EFIAPI
GetBBSTypeFromMessagingDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN EFI_DEVICE_PATH_PROTOCOL  *Node
  )
{
  VENDOR_DEVICE_PATH       *Vendor;
  UINT16                   Result;

  Result = BBS_TYPE_UNKNOWN;

  switch (DevicePathSubType (Node)) {
  case MSG_MAC_ADDR_DP:
    Result = BBS_TYPE_EMBEDDED_NETWORK;
    break;

  case MSG_USB_DP:
    Result = GetBBSTypeFromUsbPath (DevicePath);
    if (Result == BBS_TYPE_UNKNOWN) {
      Result = BBS_TYPE_USB;
    }
    break;

  case MSG_SATA_DP:
  case MSG_NVME_NAMESPACE_DP:
    Result = BBS_TYPE_HARDDRIVE;
    break;

  case MSG_VENDOR_DP:
    Vendor = (VENDOR_DEVICE_PATH *) (Node);
    if (&Vendor->Guid != NULL) {
      if (CompareGuid (&Vendor->Guid, &((EFI_GUID)DEVICE_PATH_MESSAGING_SAS))) {
        Result = BBS_TYPE_HARDDRIVE;
      }
    }
    break;

  default:
    Result = BBS_TYPE_UNKNOWN;
    break;
  }

  return Result;
}

/**
  Get BBS Type of a boot option from its option number.

  @param[in]  DevicePath           Current node of the messaging device path.

  @retval    UINT16                Return BBS Type.

**/
UINT16
EFIAPI
GetBBSTypeByDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL *Node;
  UINT16                   Result;

  Result = BBS_TYPE_UNKNOWN;
  if (DevicePath == NULL) {
    return Result;
  }
  Node = DevicePath;
  while (!IsDevicePathEnd (Node)) {
    switch (DevicePathType (Node)) {
    case MEDIA_DEVICE_PATH:
      if (DevicePathSubType (Node) == MEDIA_CDROM_DP) {
        Result = BBS_TYPE_CDROM;
      }else if(DevicePathSubType (Node) == MEDIA_PIWG_FW_VOL_DP){
        Result = BBS_TYPE_MENU;
      }
      break;

    case MESSAGING_DEVICE_PATH:
      Result = GetBBSTypeFromMessagingDevicePath (DevicePath, Node);
      break;

    default:
      Result = BBS_TYPE_UNKNOWN;
      break;
    }

    if (Result != BBS_TYPE_UNKNOWN) {
      break;
    }

    Node = NextDevicePathNode (Node);
  }

  return Result;
}

/**
  Looking for the backup of previous BootOrder,the restore it with respect to current BootOrder.

**/
VOID
EFIAPI
RestoreBootOrder (
  VOID
  )
{
  EFI_STATUS                Status;
  UINT16                    *BootOrder;
  UINTN                     BootOrderSize;

  GetVariable2 (
    L"BootOrderBackup",
    &gOemBootVariableGuid,
    (VOID **) &BootOrder,
    &BootOrderSize
    );
  if (BootOrder == NULL) {
    return;
  }

  Print (L"\nRestore BootOrder(%d).\n", BootOrderSize / sizeof (UINT16));

  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
                  | EFI_VARIABLE_NON_VOLATILE,
                  BootOrderSize,
                  BootOrder
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SetVariable BootOrder %r!\n", Status));
  }

  Status = gRT->SetVariable (
                  L"BootOrderBackup",
                  &gOemBootVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  0,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SetVariable BootOrderBackup %r!\n", Status));
  }

  FreePool (BootOrder);
}

/**
  Looking for the backup of previous BootOrder,the restore it with respect to current BootOrder.

  @param[in]  EFI_EVENT      The Event this notify function registered to.
  @param[in]  Context        Pointer to the context data registered to the Event.

**/
VOID
EFIAPI
RestoreBootOrderOnReadyToBoot (
  IN EFI_EVENT         Event,
  IN VOID             *Context
  )
{
  // restore BootOrder variable in normal condition.
  //RestoreBootOrder ();
}

/**
  Update Boot Order.

  @param[in]  NewOrder         Update New BootOrder.
  @param[in]  BootOrder        Pointer to the array of an existed BootOrder.
  @param[in]  BootOrderSize    Size of the existed BootOrder.

**/
VOID
EFIAPI
UpdateBootOrder (
  IN UINT16  *NewOrder,
  IN UINT16  *BootOrder,
  IN UINTN   BootOrderSize
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  Status = gRT->SetVariable (
                  L"BootOrderBackup",
                  &gOemBootVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  BootOrderSize,
                  BootOrder
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Set BootOrderBackup Variable:%r!\n", Status));
    return;
  }

  Status = gRT->SetVariable (
                  L"BootOrder",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
                  | EFI_VARIABLE_NON_VOLATILE,
                  BootOrderSize,
                  NewOrder
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Set BootOrder Variable:%r!\n", Status));
    return;
  }

  //
  // Register notify function to restore BootOrder variable on ReadyToBoot Event.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  RestoreBootOrderOnReadyToBoot,
                  NULL,
                  &gEfiEventReadyToBootGuid,
                  &Event
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Create ready to boot event %r!\n", Status));
  }
}

/**
  Set Boot Order.

  @param[in]  BootType      Pointer to the array of an existed BootOrder.

**/
VOID
EFIAPI
SetBootOrder (
  IN UINT16 BootType
  )
{
  EFI_STATUS                   Status;
  UINT16                       *NewOrder;
  UINT16                       *RemainBoots;
  UINT16                       *BootOrder;
  UINTN                        BootOrderSize;
  EFI_BOOT_MANAGER_LOAD_OPTION Option;
  CHAR16                       OptionName[sizeof ("Boot####")];
  UINTN                        Index;
  UINTN                        SelectCnt;
  UINTN                        RemainCnt;

  GetEfiGlobalVariable2 (L"BootOrder", (VOID **)&BootOrder, &BootOrderSize);
  if (BootOrder == NULL) {
    return ;
  }

  NewOrder = AllocatePool (BootOrderSize);
  RemainBoots = AllocatePool (BootOrderSize);
  if ((NewOrder == NULL) || (RemainBoots == NULL)) {
    DEBUG ((DEBUG_ERROR, "Out of resources."));
    goto Exit;
  }

  SelectCnt = 0;
  RemainCnt = 0;

  for (Index = 0; Index < BootOrderSize / sizeof (UINT16); Index++) {
    UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", BootOrder[Index]);
    
    Status = EfiBootManagerVariableToLoadOption (OptionName, &Option);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Boot%04x is invalid option!\n", BootOrder[Index]));
      continue;
    }
    
   if(BootType == BBS_TYPE_SHELL)
   {
        if (StrCmp (Option.Description, L"UEFI Shell") == 0)
	    NewOrder[SelectCnt++] = BootOrder[Index];
        else
	    RemainBoots[RemainCnt++] = BootOrder[Index];
    }
    else if(BootType == BBS_TYPE_MENU)
    {
        if (StrCmp (Option.Description, L"Enter Setup") == 0)
	    NewOrder[SelectCnt++] = BootOrder[Index];
        else
	    RemainBoots[RemainCnt++] = BootOrder[Index];
    }
    else
    {
	if (GetBBSTypeByDevicePath (Option.FilePath) == BootType )
	    NewOrder[SelectCnt++] = BootOrder[Index];
         else
	    RemainBoots[RemainCnt++] = BootOrder[Index];
    }
  }

  if (SelectCnt != 0) {
    // append RemainBoots to NewOrder
    for (Index = 0; Index < RemainCnt; Index++) {
      NewOrder[SelectCnt + Index] = RemainBoots[Index];
    }

    if (CompareMem (NewOrder, BootOrder, BootOrderSize) != 0) {
      UpdateBootOrder (NewOrder, BootOrder, BootOrderSize);
    }
  }

Exit:
  FreePool (BootOrder);
  if (NewOrder != NULL) {
    FreePool (NewOrder);
  }
  if (RemainBoots != NULL) {
    FreePool (RemainBoots);
  }
}

/**
  Get IPMI device Boot Type.

**/
VOID
EFIAPI
IpmiBmcBootType (
  VOID
  )
{
  EFI_STATUS                              Status;
  IPMI_BOOT_FLAGS_INFO                    BootFlags;
  UINT16                                  BootType;

  //
  // Get Boot Flags parameter from BMC.
  //

  Status = IpmiGetBootFlags (&BootFlags);

  if (EFI_ERROR (Status)) {
    return;
  }
  
  switch (BootFlags.DeviceSelector) {
  case ForceShell:
    BootType = BBS_TYPE_SHELL;
    break;

  case ForcePxe:
    BootType = BBS_TYPE_EMBEDDED_NETWORK;
    break;

  case ForcePrimaryRemoveableMedia:
    BootType = BBS_TYPE_USB;
    break;

  case ForceDefaultHardDisk:
    BootType = BBS_TYPE_HARDDRIVE;
    break;

  case ForceDefaultCD:
    BootType = BBS_TYPE_CDROM;
    break;

  case ForceBiosSetup:
    BootType = BBS_TYPE_MENU;
    break;

  default:
    return;
  }

  //
  // Set Boot Order.
  //
  SetBootOrder (BootType);

  //
  // Set Boot Info Acknowledge to BMC.
  //
  Status = IpmiSetBootInfoAck ();
  if (!EFI_ERROR (Status)) {
    if (BootFlags.IsPersistent == 0) {
      IpmiClearBootFlags ();
      return;
    } else {
      DEBUG ((DEBUG_INFO,"IpmiSetBootInfoAck to BMC \n"));
    }
  } else if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"IpmiSetBootInfoAck %r \n",Status));
    return;
  }
}

