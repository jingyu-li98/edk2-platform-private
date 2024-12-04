/** @file

Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
All intellectual property rights(Copyright, Patent and Trademark) reserved.

Copyright (c) 2004 - 2017, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Security.h"

EFI_HANDLE                      DriverHandle = NULL;
SECURITY_PAGE_PRIVATE_DATA      *mPrivateData = NULL;
SECURITY_HII_INSTANCE           *gHiiDriverList;
extern EFI_HII_HANDLE           gStringPackHandle;

HII_VENDOR_DEVICE_PATH  mSecurityPageHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    SECURITY_PAGE_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};


/**

  Update the menus in the security page.

**/
VOID
UpdateSecurityPageForm (
  VOID 
  );

/**
  Create empty line menu in the security page.

  @param    HiiHandle           The hii handle for the Uiapp driver.
  @param    StartOpCodeHandle   The opcode handle to save the new opcode.

**/
VOID
UiCreateEmptyLine (
  IN EFI_HII_HANDLE              HiiHandle,
  IN VOID                        *StartOpCodeHandle
  )
{
  HiiCreateSubTitleOpCode (StartOpCodeHandle, STRING_TOKEN (STR_NULL_STRING), 0, 0, 0);
}

/**
  Extract device path for given HII handle and class guid.

  @param Handle          The HII handle.

  @retval  NULL          Fail to get the device path string.
  @return  PathString    Get the device path string.

**/
CHAR16 *
ExtractDevicePathFromHiiHandle (
  IN      EFI_HII_HANDLE      Handle
  )
{
  EFI_STATUS                       Status;
  EFI_HANDLE                       DriverHandle;

  ASSERT (Handle != NULL);
  if (Handle == NULL) {
    return NULL;
  }

  Status = gHiiDatabase->GetPackageListHandle (gHiiDatabase, Handle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return ConvertDevicePathToText(DevicePathFromHandle (DriverHandle), FALSE, FALSE);
}

/**
  Check whether this driver need to be shown in the security page.

  @param    HiiHandle           The hii handle for the driver.
  @param    Guid                The special guid for the driver which is the target.
  @param    PromptId            Return the prompt string id.
  @param    HelpId              Return the help string id.
  @param    FormsetGuid         Return the formset guid info.

  @retval   EFI_SUCCESS         Search the driver success

**/
BOOLEAN
RequiredDriver (
  IN  EFI_HII_HANDLE              HiiHandle,
  IN  EFI_GUID                    *Guid,
  OUT EFI_STRING_ID               *PromptId,
  OUT EFI_STRING_ID               *HelpId,
  OUT VOID                        *FormsetGuid
  )
{
  EFI_STATUS                  Status;
  UINT8                       ClassGuidNum;
  EFI_GUID                    *ClassGuid;
  EFI_IFR_FORM_SET            *Buffer;
  UINTN                       BufferSize;
  UINT8                       *Ptr;
  UINTN                       TempSize;
  BOOLEAN                     RetVal;

  Status = HiiGetFormSetFromHiiHandle(HiiHandle, &Buffer, &BufferSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  RetVal = FALSE;
  TempSize = 0;
  Ptr = (UINT8 *) Buffer;
  while (TempSize < BufferSize)  {
    TempSize += ((EFI_IFR_OP_HEADER *) Ptr)->Length;

    if (((EFI_IFR_OP_HEADER *) Ptr)->Length <= OFFSET_OF (EFI_IFR_FORM_SET, Flags)){
      Ptr += ((EFI_IFR_OP_HEADER *) Ptr)->Length;
      continue;
    }

    ClassGuidNum = (UINT8) (((EFI_IFR_FORM_SET *)Ptr)->Flags & 0x3);
    ClassGuid = (EFI_GUID *) (VOID *)(Ptr + sizeof (EFI_IFR_FORM_SET));
    while (ClassGuidNum-- > 0) {
      if (!CompareGuid (Guid, ClassGuid)){
        ClassGuid ++;
        continue;
      }

      *PromptId = ((EFI_IFR_FORM_SET *)Ptr)->FormSetTitle;
      *HelpId = ((EFI_IFR_FORM_SET *)Ptr)->Help;
      CopyMem (FormsetGuid, &((EFI_IFR_FORM_SET *) Ptr)->Guid, sizeof (EFI_GUID));
      RetVal = TRUE;
    }
  }

  FreePool (Buffer);

  return RetVal;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                        Status;
  EFI_STRING                        ConfigRequest;
  UINTN                             Size;
  CHAR16                            *StrPointer;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Initialize the local variables.
  //
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;

  if (Request != NULL) {
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
    //
    // Check whether Request includes Request Element.
    //
    if (StrStr (Request, L"OFFSET") == NULL) {
      //
      // Check Request Element does exist in Reques String
      //
      StrPointer = StrStr (Request, L"PATH");
      if (StrPointer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (StrStr (StrPointer, L"&") == NULL) {
        Size = (StrLen (Request) + 32 + 1) * sizeof (CHAR16);
        ConfigRequest    = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
      }
    }
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}


/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
FakeRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_SUCCESS;

  return Status;
}


/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.

**/
EFI_STATUS
EFIAPI
FakeDriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                      Status;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //Solve when langguage change, the SecurityListThirdPartyDrivers form title do not change
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) 
  {
    if (QuestionId == FORM_SECURITY_OPEN) 
    {
       UpdateSecurityPageForm ();
    }
  }
  Status = EFI_SUCCESS;
  return Status;
}


/**
  Search the drivers in the system which need to show in the front page
  and insert the menu to the security page.

  @param    HiiHandle           The hii handle for the Uiapp driver.
  @param    ClassGuid           The class guid for the driver which is the target.
  @param    SpecialHandlerFn    The pointer to the specail handler function, if any.
  @param    StartOpCodeHandle   The opcode handle to save the new opcode.

  @retval   EFI_SUCCESS         Search the driver success

**/
EFI_STATUS
SecurityListThirdPartyDrivers (
  IN EFI_HII_HANDLE              HiiHandle,
  IN EFI_GUID                    *ClassGuid,
  IN DRIVER_SPECIAL_HANDLER      SpecialHandlerFn,
  IN VOID                        *StartOpCodeHandle
  )
{
  UINTN                       Index;
  EFI_STRING                  String;
  EFI_STRING_ID               Token;
  EFI_STRING_ID               TokenHelp;
  EFI_HII_HANDLE              *HiiHandles;
  CHAR16                      *DevicePathStr;
  UINTN                       Count;
  UINTN                       CurrentSize;
  SECURITY_HII_INSTANCE       *DriverListPtr;
  EFI_STRING                  NewName;
  BOOLEAN                     EmptyLineAfter;

  if (gHiiDriverList != NULL) {
    FreePool (gHiiDriverList);
  }

  HiiHandles = HiiGetHiiHandles (NULL);
  ASSERT (HiiHandles != NULL);

  gHiiDriverList = AllocateZeroPool (SECURITY_HII_DRIVER_LIST_SIZE * sizeof (SECURITY_HII_INSTANCE));
  ASSERT (gHiiDriverList != NULL);
  DriverListPtr = gHiiDriverList;
  CurrentSize = SECURITY_HII_DRIVER_LIST_SIZE;

  for (Index = 0, Count = 0; HiiHandles[Index] != NULL; Index++) {
    if (!RequiredDriver (HiiHandles[Index], ClassGuid, &Token, &TokenHelp, &gHiiDriverList[Count].FormSetGuid)) {
      continue;
    }

    String = HiiGetString (HiiHandles[Index], Token, NULL);
    if (String == NULL) {
      String = HiiGetString (gStringPackHandle, STRING_TOKEN (STR_MISSING_STRING), NULL);
      ASSERT (String != NULL);
    } else if (SpecialHandlerFn != NULL) {
      //
      // Check whether need to rename the driver name.
      //
      EmptyLineAfter = FALSE;
      if (SpecialHandlerFn (String, &NewName, &EmptyLineAfter)) {
        FreePool (String);
        String = NewName;
        DriverListPtr[Count].EmptyLineAfter = EmptyLineAfter;
      }
    }
    DriverListPtr[Count].PromptId = HiiSetString (HiiHandle, 0, String, NULL);
    FreePool (String);

    String = HiiGetString (HiiHandles[Index], TokenHelp, NULL);
    if (String == NULL) {
      String = HiiGetString (gStringPackHandle, STRING_TOKEN (STR_MISSING_STRING), NULL);
      ASSERT (String != NULL);
    }
    DriverListPtr[Count].HelpId = HiiSetString (HiiHandle, 0, String, NULL);
    FreePool (String);

    DevicePathStr = ExtractDevicePathFromHiiHandle(HiiHandles[Index]);
    if (DevicePathStr != NULL){
      DriverListPtr[Count].DevicePathId = HiiSetString (HiiHandle, 0, DevicePathStr, NULL);
      FreePool (DevicePathStr);
    } else {
      DriverListPtr[Count].DevicePathId = 0;
    }

    Count++;
    if (Count >= CurrentSize) {
      DriverListPtr = ReallocatePool (
                        CurrentSize * sizeof (SECURITY_HII_INSTANCE),
                        (Count + SECURITY_HII_DRIVER_LIST_SIZE)
                          * sizeof (SECURITY_HII_INSTANCE),
                        gHiiDriverList
                        );
      ASSERT (DriverListPtr != NULL);
      gHiiDriverList = DriverListPtr;
      CurrentSize += SECURITY_HII_DRIVER_LIST_SIZE;
    }
  }

  FreePool (HiiHandles);

  Index = 0;
  while (gHiiDriverList[Index].PromptId != 0) {
    HiiCreateGotoExOpCode (
      StartOpCodeHandle,
      0,
      gHiiDriverList[Index].PromptId,
      gHiiDriverList[Index].HelpId,
      0,
      (EFI_QUESTION_ID) (Index + SECURITY_PAGE_KEY_DRIVER),
      0,
      &gHiiDriverList[Index].FormSetGuid,
      gHiiDriverList[Index].DevicePathId
    );

    if (gHiiDriverList[Index].EmptyLineAfter) {
      UiCreateEmptyLine (HiiHandle, StartOpCodeHandle);
    }

    Index ++;
  }

  return EFI_SUCCESS;
}
/**
  Customize menus in the page.

  @param[in]  HiiHandle             The HII Handle of the form to update.
  @param[in]  StartOpCodeHandle     The context used to insert opcode.
  @param[in]  CustomizePageType     The page type need to be customized.

**/
VOID
UiCustomizeSecurityPage (
  IN EFI_HII_HANDLE  HiiHandle,
  IN VOID            *StartOpCodeHandle
  )
{
  //
  // Create empty line.
  //
  UiCreateEmptyLine(HiiHandle, StartOpCodeHandle);
  //
  // Find third party drivers which need to be shown in the Security page.
  //
  SecurityListThirdPartyDrivers (HiiHandle, &gEfiIfrSecurityPageGuid, NULL, StartOpCodeHandle);
  //
  // Create empty line.
  //
  UiCreateEmptyLine(HiiHandle, StartOpCodeHandle);

}

/**

  Update the menus in the security page.

**/
VOID
UpdateSecurityPageForm (
  VOID
  )
{
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartGuidLabel;
  EFI_IFR_GUID_LABEL          *EndGuidLabel;

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);
  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartGuidLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartGuidLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartGuidLabel->Number       = LABEL_SECURITYPAGE_INFORMATION;
  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndGuidLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndGuidLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndGuidLabel->Number       = LABEL_END;

  //
  //Updata Front Page form
  //
  UiCustomizeSecurityPage (
    mPrivateData->HiiHandle,
    StartOpCodeHandle
    );

  HiiUpdateForm (
    mPrivateData->HiiHandle,
    &gSecurityPageFormSetGuid,
    SECURITY_PAGE_FORM_ID,
    StartOpCodeHandle,
    EndOpCodeHandle
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

}

/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
SecurityPageInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;

  mPrivateData = AllocateZeroPool (sizeof (SECURITY_PAGE_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = SECURITY_PAGE_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = FakeRouteConfig;
  mPrivateData->ConfigAccess.Callback = FakeDriverCallback;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mSecurityPageHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle = DriverHandle;
  //
  // Publish our HII data
  //
  HiiHandle = HiiAddPackages (
                   &gSecurityPageFormSetGuid,
                   DriverHandle,
                   SecurityPageStrings,
                   SecurityVfrBin,
                   NULL
                   );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle = HiiHandle;
  UpdateSecurityPageForm ();

  return EFI_SUCCESS;
}
