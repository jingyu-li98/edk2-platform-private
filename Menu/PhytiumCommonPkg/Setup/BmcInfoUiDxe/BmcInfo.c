/** @file
HII Config Access protocol implementation of bmc configuration module.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <BmcInfo.h>
#include <IpmiCommandLib.h>
#include <IpmiPhyStandard.h>
#include <SetupBmcConfig.h>

#include <Library/IpmiBaseLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/StringBaseLib.h>

EFI_GUID                         mBmcInfoGuid = BMC_INFO_FORMSET_GUID;
EFI_GUID                         mBmcSetupGuid = BMC_SETUP_VARIABLE_GUID;
IPMI_BMC_INFO                    mBmcInfo;
IPMI_GET_SELF_TEST_RESULT        mSelfTestResult;
UINT8                            mFirstEnterPage;

HII_VENDOR_DEVICE_PATH  mBmcInfoHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
      (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
      (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    //
    // {0x8236697e, 0xf0f6, 0x405f, {0x99, 0x13, 0xac, 0xbc, 0x50, 0xaa, 0x45, 0xd1}}
    //
    SETUP_BMC_CFG_GUID
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

BMC_INFO_CALLBACK_DATA  gBmcInfoPrivate = {
  BMC_INFO_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    BmcInfoExtractConfig,
    BmcInfoRouteConfig,
    BmcInfoCallback
  }
};

/**
  Get Bmc status selftest information and display it.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS       Command completed successfully.
  @retval     EFI_DEVICE_ERROR  IPMI command failed.
  @retval     EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval     EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval     EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
GetBmcInfo(
  IN  EFI_HII_HANDLE            Handle
)
{
  EFI_STATUS                   Status;

  if (mFirstEnterPage == 1) {
    mFirstEnterPage = 0;

    Status = IpmiGetDeviceId (&mBmcInfo);

    if (!EFI_ERROR (Status)) {
      //
      // Get the SELF TEST Results.
      //
      Status = IpmiGetSelfTestResult (&mSelfTestResult);
      if (EFI_ERROR (Status)) {
        DEBUG((DEBUG_ERROR, "\n[IPMI] BMC does not respond (status: %r)!\n\n", Status));
        return Status;
      }

    } else if (EFI_ERROR (Status)) {
        DEBUG((DEBUG_ERROR,"Faild Get Bmc ID\n"));
        return Status;
    }

    DEBUG((DEBUG_INFO, "Bmc info get success\n"));
    DEBUG((DEBUG_INFO, "devicde id:%d\n", mBmcInfo.DeviceId));
    DEBUG((DEBUG_INFO, "device version:%0x\n", mBmcInfo.DeviceRevision));
    DEBUG((DEBUG_INFO, "device check state : %0x %0x\n", mSelfTestResult.Result, mSelfTestResult.Param));
    DEBUG((DEBUG_INFO, "bmc fw version:%0x %0x\n", mBmcInfo.MajorFmRevision, mBmcInfo.MinorFmRevision));
    DEBUG((DEBUG_INFO, "bmc manufacturer id:%0x %0x %0x\n", mBmcInfo.ManufacturerId[0], mBmcInfo.ManufacturerId[1],mBmcInfo.ManufacturerId[2]));
    DEBUG((DEBUG_INFO, "ipmi version:%02x %02x\n", mBmcInfo.IpmiVersionMostSigBits, mBmcInfo.IpmiVersionLeastSigBits));
    //device id
    BmcInfoSetHiiString (
      Handle,
      STRING_TOKEN (STR_BMC_ID_VALUE),
      L"%d",
      mBmcInfo.DeviceId
    );
    //device version
    mBmcInfo.DeviceRevision &= 0x0F;
    BmcInfoSetHiiString (
      Handle,
      STRING_TOKEN (STR_BMC_VER_VALUE),
      L"%x",
      mBmcInfo.DeviceRevision
    );
    //device check state
    if(mSelfTestResult.Result == 0x55) {
      BmcInfoSetHiiString (
        Handle,
        STRING_TOKEN (STR_BMC_CHECK_VALUE),
        L"PASS"
      );
    } else {
      BmcInfoSetHiiString (
        Handle,
        STRING_TOKEN (STR_BMC_CHECK_VALUE),
        L"ERROR %02x:%02x",
        mSelfTestResult.Result,
        mSelfTestResult.Param
      );
    }
    //bmc fw version
    BmcInfoSetHiiString (
      Handle,
      STRING_TOKEN (STR_BMC_FM_VER_VALUE),
      L"%d.%02d",
      mBmcInfo.MajorFmRevision&0x7F,
      mBmcInfo.MinorFmRevision
    );
    //bmc manufacturer id
    BmcInfoSetHiiString (
      Handle,
      STRING_TOKEN (STR_BMC_FM_VENDER_VALUE),
      L"%02x%02x%02x",
      mBmcInfo.ManufacturerId[2],
      mBmcInfo.ManufacturerId[1],
      mBmcInfo.ManufacturerId[0]
    );
    //ipmi version
    BmcInfoSetHiiString (
      Handle,
      STRING_TOKEN (STR_BMC_IPMI_VER_VALUE),
      L"%x.%x",
      mBmcInfo.IpmiVersionMostSigBits,
      mBmcInfo.IpmiVersionLeastSigBits
    );
  }

  return EFI_SUCCESS;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param[in]  This       Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Request    A null-terminated Unicode string in <ConfigRequest> format.
  @param[out] Progress   On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param[out] Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BmcInfoExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                 Status;
  UINTN                      BufferSize;
  EFI_STRING                 ConfigRequestHdr;
  EFI_STRING                 ConfigRequest;
  BOOLEAN                    AllocatedRequest;
  UINTN                      Size;
  BMC_INFO_CALLBACK_DATA     *Private;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mBmcInfoGuid, BMC_CONFIG_STORE)) {
    DEBUG((DEBUG_INFO,"%a(),%d,HiiIsConfigHdrMatch not match\n",__FUNCTION__,__LINE__));
    return EFI_NOT_FOUND;
  }
  ConfigRequestHdr = NULL;
  ConfigRequest   = NULL;
  AllocatedRequest = FALSE;
  Size       = 0;

  Private = BMC_INFO_CALLBACK_DATA_FROM_THIS (This);
  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  BufferSize = sizeof (BMC_CONFIG);
  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&mBmcInfoGuid, BMC_CONFIG_STORE, Private->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
  }

  Status = gHiiConfigRouting->BlockToConfig (
                  gHiiConfigRouting,
                  ConfigRequest,
                  (UINT8 *) &Private->BmcConfig,
                  BufferSize,
                  Results,
                  Progress
                  );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
    ConfigRequest = NULL;
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

  @param[in]  This        Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param[out] Progress      A pointer to a string filled in with the offset of the most
               recent '&' before the first failing name/value pair (or the
               beginning of the string if the failure is in the first
               name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS      The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND      Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BmcInfoRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL  *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{

  EFI_STATUS                       Status;
  UINTN                            buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  BMC_INFO_CALLBACK_DATA           *Private;

  Private = BMC_INFO_CALLBACK_DATA_FROM_THIS(This);
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
          &gEfiHiiConfigRoutingProtocolGuid,
          NULL,
          (VOID **)&ConfigRouting
          );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((DEBUG_ERROR, "BMC Cfg route config\n"));
  if (!HiiIsConfigHdrMatch (Configuration, &mBmcInfoGuid, BMC_CONFIG_STORE)) {
    return EFI_NOT_FOUND;
  }
  //config bmc ip
  Status = ValidateSetupConfig(This);
  buffsize = sizeof(BMC_CONFIG);
  Status = ConfigRouting->ConfigToBlock (
                        ConfigRouting,
                        Configuration,
                        (UINT8 *) &Private->BmcConfig,
                        &buffsize,
                        Progress
                        );
  return Status;
}

/**
  This call back function is registered with Bmc infomation formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.

  @param[in]  This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Action          Specifies the type of action taken by the browser.
  @param[in]  QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param[in]  Type            The type of value for the question.
  @param[in]  Value           A pointer to the data being sent to the original exporting driver.
  @param[out] ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
BmcInfoCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                             Status;
  BMC_INFO_CALLBACK_DATA                *Private;

  Status = EFI_SUCCESS;
  DEBUG((DEBUG_INFO,"%a(),%d,QuestionId:%04x,begin\n",__FUNCTION__,__LINE__,QuestionId));
  Private = BMC_INFO_CALLBACK_DATA_FROM_THIS(This);
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
    //
    //Means enter the bmc info form.
    //Update the bmc info page,because the bmc ip may changed.
    //
    if (QuestionId == KEY_MAIN) {
      Status = GetBmcInfo (Private->HiiHandle);
    } else if (QuestionId == KEY_IP) {
      Status = ValidateSetupInput (This, Action, QuestionId, Type, Value, ActionRequest);
    }
  } else if (Action == EFI_BROWSER_ACTION_CHANGING) {
    if (QuestionId >= KEY_PASSWORD_1 && QuestionId <= KEY_PASSWORD_15) {
      Status = ProcessPasswordSet (Private, QuestionId, Value);
    }
  }
  return Status;
}

/**
  Install bmc information Menu driver.

  @param[in] PrivateData    Points to the BMC_INFO_CALLBACK_DATA.

  @retval  EFI_SUCEESS  Install Boot manager menu success.
**/
EFI_STATUS
EFIAPI
InstallBmcInfoForm (
  IN BMC_INFO_CALLBACK_DATA   *PrivateData
)
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;

  mFirstEnterPage = 1;
  ZeroMem (&mBmcInfo, sizeof (mBmcInfo));
  ZeroMem (&mSelfTestResult, sizeof (mSelfTestResult));
  //
  // Initialize COM layer Library as we will use IPMI COM Library functions.
  //
  InitializeIpmiBase ();
  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //

  DriverHandle = NULL;
  ConfigAccess = &PrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
          &DriverHandle,
          &gEfiDevicePathProtocolGuid,
          &mBmcInfoHiiVendorDevicePath,
          &gEfiHiiConfigAccessProtocolGuid,
          ConfigAccess,
          NULL
          );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PrivateData->DriverHandle = DriverHandle;

  //
  // Publish the HII package list
  //
  HiiHandle = HiiAddPackages (
                  &mBmcInfoGuid,
                  DriverHandle,
                  BmcInfoUiDxeBin,
                  BmcInfoUiDxeStrings,
                  NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
            DriverHandle,
            &gEfiDevicePathProtocolGuid,
            &mBmcInfoHiiVendorDevicePath,
            &gEfiHiiConfigAccessProtocolGuid,
            ConfigAccess,
            NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->HiiHandle = HiiHandle;

  Status = GetBmcInfo (HiiHandle);

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  PrivateData     Points to the BMC_INFO_CALLBACK_DATA.
**/
VOID
UninstallBmcInfoForm (
  IN BMC_INFO_CALLBACK_DATA   *PrivateData
  )
{
  //
  // Uninstall HII package list
  //
  if (PrivateData->HiiHandle != NULL) {
    HiiRemovePackages (PrivateData->HiiHandle);
    PrivateData->HiiHandle = NULL;
  }

  //
  // Uninstall HII Config Access Protocol
  //
  if (PrivateData->DriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
            PrivateData->DriverHandle,
            &gEfiDevicePathProtocolGuid,
            &mBmcInfoHiiVendorDevicePath,
            &gEfiHiiConfigAccessProtocolGuid,
            &PrivateData->ConfigAccess,
            NULL
           );
    PrivateData->DriverHandle = NULL;
  }

  FreePool (PrivateData);
}


