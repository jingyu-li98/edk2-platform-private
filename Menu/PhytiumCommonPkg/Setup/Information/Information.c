/** @file

Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
All intellectual property rights(Copyright, Patent and Trademark) reserved.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Information.h"
#include "SetupFormInit.h"

EFI_HANDLE                  DriverHandle;
EFI_HII_HANDLE              gStringPackHandle;
EFI_EVENT                   gEvent;
INFORMATION_PRIVATE_DATA    *mPrivateData = NULL;


HII_VENDOR_DEVICE_PATH  mInformHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    INFORMATION_SET_FORMSET_GUID
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
  EFI_STATUS  Status;

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
  Status = EFI_SUCCESS;
  return Status;
}




/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
InformInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;
  CHAR16                          *NewString;
  //VOID                            *Registration;
  //LS_MEM_INFO_HOB                 MemInfoHobData;
 // LS_CPU_INFO_HOB                 CpuInfoHobData;
 // UINTN                           HtFreq __attribute__ ((unused));;
  //UINTN                           DdrFreq;
  //UINT64                          DimmMaxNum;
  //DDR4_SMBIOS                     *Ddr4Smbios;

  NewString        = NULL;
  //Registration     = NULL;

  //HtFreq           = 0;
  mPrivateData = AllocateZeroPool (sizeof (INFORMATION_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = SET_INFORMATION_PRIVATE_SIGNATURE;

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
                  &mInformHiiVendorDevicePath,
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
                   &gInformationFormSetGuid,
                   DriverHandle,
                   InformationStrings,
                   InformationVfrBin,
                   NULL
                   );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle = HiiHandle;

  //
  // Update the device path string.
  //
  NewString = ConvertDevicePathToText((EFI_DEVICE_PATH_PROTOCOL*)&mInformHiiVendorDevicePath, FALSE, FALSE);
  if (HiiSetString (HiiHandle, STRING_TOKEN (STR_DEVICE_PATH), NewString, NULL) == 0) {
    InformUnload (ImageHandle);
    return EFI_OUT_OF_RESOURCES;
  }
  if (NewString != NULL) {
    FreePool (NewString);
  }


  MainFormInit(HiiHandle);
#if 0
  //
  //Update the Main Menu String Read form Smbios With Event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  //(EFI_EVENT_NOTIFY) UpdateString,
                  UpdateString,
                  NULL,
                  &gEvent
                  );
  ASSERT (!EFI_ERROR (Status));

  Status = gBS->RegisterProtocolNotify (
                  &gEfiSmbiosTableCompleteProtocolGuid,
                  gEvent,
                  &Registration
                  );
  ASSERT (!EFI_ERROR (Status));
  gBS->SignalEvent (gEvent);

#endif

#if 0
  //
  //Update the Core Num String
  //
  Status = GetCpuInfoHob (&CpuInfoHobData);
  ASSERT_EFI_ERROR (Status);

  ConvertCpuNumToString (CORES_PER_NODE * TOT_NODE_NUM, &NewString);
  HiiSetString (HiiHandle, STRING_TOKEN (STR_FRONT_PAGE_CPU_NUM), NewString, NULL);
  FreePool (NewString);

  //
  //Update the Mem Freq String
  //
  Status = GetMemInfoHob (&MemInfoHobData);
  ASSERT_EFI_ERROR (Status);

  DdrFreq = MemInfoHobData.MemInfo.Freq * 4;

  ConvertFreqToString (DdrFreq, &NewString);
  HiiSetString (HiiHandle, STRING_TOKEN (STR_FRONT_PAGE_MEMORY_FREQ), NewString, NULL);
  FreePool (NewString);


  //
  //Update the Dimm Size String
  //
  DimmMaxNum = TOT_NODE_NUM * MCMAX * DIMMMAX;
  Ddr4Smbios = (DDR4_SMBIOS *)MemInfoHobData.MemInfo.Ddr4Smbios;
  CopyMem (Ddr4Smbios, (VOID *)SMBIOS_INFO_IN_SDRAM_OFFS, sizeof (DDR4_SMBIOS) * DimmMaxNum);
  Status = LsUpdateDimmInfo (DimmMaxNum, Ddr4Smbios);


#endif

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
InformUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  if (DriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
            DriverHandle,
            &gEfiDevicePathProtocolGuid,
            &mInformHiiVendorDevicePath,
            &gEfiHiiConfigAccessProtocolGuid,
            &mPrivateData->ConfigAccess,
            NULL
           );
    DriverHandle = NULL;
  }
  if (mPrivateData->HiiHandle != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle);
  }

  FreePool (mPrivateData);
  mPrivateData = NULL;

  return EFI_SUCCESS;
}
