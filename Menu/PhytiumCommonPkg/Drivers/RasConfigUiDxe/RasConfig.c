/** @file
HII Config Access protocol implementation of ras configuration module.

Copyright (c) 2024, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "RasConfig.h"
#include "RasConfigFormGuid.h"

#include <Library/HobLib.h>
#include <Library/ParameterTable.h>
#include <Library/UefiHiiServicesLib.h>

VOID                  *mStartOpCodeHandle;
VOID                  *mEndOpCodeHandle;
EFI_IFR_GUID_LABEL    *mStartLabel;
EFI_IFR_GUID_LABEL    *mEndLabel;

EFI_GUID mRasConfigGuid = RAS_FORMSET_GUID;
extern UINT8 RasConfigUiBin[];

HII_VENDOR_DEVICE_PATH  mRasConfigHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {0xc9a0daa0, 0x4af8, 0x11ee, {0x8f, 0x2f, 0xdf, 0xad, 0x8e, 0x68, 0x24, 0x93}}
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

RAS_CONFIG_CALLBACK_DATA gRasConfigPrivate = {
  RAS_CONFIG_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    RasConfigExtractConfig,
    RasConfigRouteConfig,
    RasConfigCallback
  }
};

/**
  RasConfig UI configuration restore defaults.

  @param[in,out]    RasConfigData    Point to ras configuration struct.

**/
VOID
RasConfigDefault (
  IN OUT RAS_CONFIG  *RasConfigData
  )
{
  RasConfigData->UserPriv = 0;
  ZeroMem (&RasConfigData->RasStrategy, sizeof (RAS_STRATEGY));
  RasConfigData->RasStrategy.BmcCperEnable = FixedPcdGetBool (PcdRasBmcCperEnable);
  RasConfigData->RasStrategy.FatalErrorRecord = FixedPcdGet8 (PcdRasFatalErrorRecord);
  RasConfigData->RasStrategy.MemoryIsoRecord = FixedPcdGet8 (PcdRasMemoryIsoRecord);
  RasConfigData->RasStrategy.MemFatalUceIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemFatalUceIsoStrategyEnable);
  RasConfigData->RasStrategy.MemFatalUceIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemFatalUceIsoStrategyResetClean);
  RasConfigData->RasStrategy.MemFatalUceIsoStrategy.Method = FixedPcdGet8 (PcdRasMemFatalUceIsoStrategyMethod);
  RasConfigData->RasStrategy.MemFatalUceIsoStrategy.Count = FixedPcdGet32 (PcdRasMemFatalUceIsoStrategyCount);
  RasConfigData->RasStrategy.MemNonFatalUceIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemNonFatalUceIsoStrategyEnable);
  RasConfigData->RasStrategy.MemNonFatalUceIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemNonFatalUceIsoStrategyResetClean);
  RasConfigData->RasStrategy.MemNonFatalUceIsoStrategy.Method = FixedPcdGet8 (PcdRasMemNonFatalUceIsoStrategyMethod);
  RasConfigData->RasStrategy.MemNonFatalUceIsoStrategy.Count = FixedPcdGet32 (PcdRasMemNonFatalUceIsoStrategyCount);
  RasConfigData->RasStrategy.MemCeIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemCeIsoStrategyEnable);
  RasConfigData->RasStrategy.MemCeIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemCeIsoStrategyResetClean);
  RasConfigData->RasStrategy.MemCeIsoStrategy.Method = FixedPcdGet8 (PcdRasMemCeIsoStrategyMethod);
  RasConfigData->RasStrategy.MemCeIsoStrategy.Count = FixedPcdGet32 (PcdRasMemCeIsoStrategyCount);
  RasConfigData->RasStrategy.CpuFatalStrategy.Enable = FixedPcdGetBool (PcdRasCpuFatalIsoStrategyEnable);
  RasConfigData->RasStrategy.CpuFatalStrategy.ResetClean = FixedPcdGetBool (PcdRasCpuFatalIsoStrategyResetClean);
  RasConfigData->RasStrategy.CpuFatalStrategy.Method = FixedPcdGet8 (PcdRasCpuFatalIsoStrategyMethod);
  RasConfigData->RasStrategy.CpuFatalStrategy.Record = FixedPcdGet8 (PcdRasCpuFatalIsoStrategyRecord);
  RasConfigData->RasStrategy.MemCeReportStrategy.Enable = FixedPcdGetBool (PcdRasMemCeReportStrategyEnable);
  RasConfigData->RasStrategy.MemCeReportStrategy.Count = FixedPcdGet32 (PcdRasMemCeReportStrategyCount);
  RasConfigData->RasStrategy.CpuCeReportStrategy.Enable = FixedPcdGetBool (PcdRasCpuCeReportStrategyEnable);
  RasConfigData->RasStrategy.CpuCeReportStrategy.Count = FixedPcdGet32 (PcdRasCpuCeReportStrategyCount);
  RasConfigData->RasStrategy.PcieCeReportStrategy.Enable = FixedPcdGetBool (PcdRasPcieCeReportStrategyEnable);
  RasConfigData->RasStrategy.PcieCeReportStrategy.Count = FixedPcdGet32 (PcdRasPcieCeReportStrategyCount);
  RasConfigData->RasStrategy.MemAddrIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemoryAddressIsolationStrategyEnable);
  RasConfigData->RasStrategy.MemAddrIsoStrategy.Type = FixedPcdGet8 (PcdRasMemoryAddressIsolationStrategyType);
}

/**
  RasConfig UI configuration initialization.Get the current ras configuration from variable.
  If the variable does not exist, restore the default configuration and save the variable.

  @return       EFI_SUCCESS    Initialize ras configuration in the form successfully.
  @return       other          Initialize ras configuration in the form failed.
**/
EFI_STATUS
RasConfigInit (
  VOID
  )
{
  EFI_STATUS            Status;
  RAS_CONFIG            RasConfigData;
  UINTN                 VarSize;

  VarSize = sizeof(RAS_CONFIG);
  Status = gRT->GetVariable (
                  RAS_CONFIG_VAR,
                  &gRasConfigVarGuid,
                  NULL,
                  &VarSize,
                  &RasConfigData
                  );
  if (Status == EFI_NOT_FOUND) {
    RasConfigDefault (&RasConfigData);
    Status = gRT->SetVariable (
                    RAS_CONFIG_VAR,
                    &gRasConfigVarGuid,
                    PLATFORM_SETUP_VARIABLE_FLAG,
                    VarSize,
                    &RasConfigData
                    );
    ASSERT_EFI_ERROR(Status);
  }

  return Status;
}


/**
  Get ras configuration from variable and update uncommitted data in the form.

  @param[in]    PrivateData    Point to RAS_CONFIG_CALLBACK_DATA.

  @return       EFI_SUCCESS    Update ras configuration in the form successfully.
  @return       other          Update ras configuration in the form failed.
**/
EFI_STATUS
GetRasConfigData (
  IN RAS_CONFIG_CALLBACK_DATA  *PrivateData
  )
{
  EFI_STATUS            Status;
  RAS_CONFIG            RasConfigData;
  UINTN                 VarSize;

  Status = EFI_SUCCESS;

  VarSize = sizeof (RAS_CONFIG);
  Status = gRT->GetVariable (
                  RAS_CONFIG_VAR,
                  &gRasConfigVarGuid,
                  NULL,
                  &VarSize,
                  &RasConfigData
                  );

  if (Status == EFI_SUCCESS) {
    CopyMem (&PrivateData->RasConfigData, &RasConfigData, VarSize);
    Status = HiiSetBrowserData (
               &mRasConfigGuid,
               RAS_CONFIG_STORE,
               sizeof (RAS_CONFIG),
               (UINT8 *) &PrivateData->RasConfigData,
               NULL
               );
  }
  return Status;
}

/**
  Get ras configuration from variable.

  @param[in]    PrivateData    Point to RAS_CONFIG_CALLBACK_DATA.

  @return       EFI_SUCCESS    Get ras configuration from variable successfully.
  @return       other          Get ras configuration from variable failed.
**/
EFI_STATUS
UpdateRasConfig (
  IN RAS_CONFIG_CALLBACK_DATA  *PrivateData
)
{
  EFI_STATUS            Status;
  RAS_CONFIG            RasConfigData;
  UINTN                 VarSize;

  Status = EFI_SUCCESS;
  VarSize = sizeof (RAS_CONFIG);
  Status = gRT->GetVariable (
                  RAS_CONFIG_VAR,
                  &gRasConfigVarGuid,
                  NULL,
                  &VarSize,
                  &RasConfigData
                  );
  if (Status == EFI_SUCCESS) {
    CopyMem (&PrivateData->RasConfigData, &RasConfigData, VarSize);
  }
  return Status;
}

/**
  Get ras configuration from the form and set the configuration to the variable.

  @param[in]    This           Point to EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]    PrivateData    Point to RAS_CONFIG_CALLBACK_DATA.

  @return       EFI_SUCCESS    Set ras configuration in the form successfully.
  @return       other          Set ras configuration in the form failed.
**/
EFI_STATUS
RasConfigSetupConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN RAS_CONFIG_CALLBACK_DATA               *PrivateData
  )
{
  EFI_STATUS                Status;
  RAS_CONFIG                RasConfigData;
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mRasConfigGuid, RAS_CONFIG_STORE, sizeof (RAS_CONFIG), (UINT8 *) &RasConfigData)) {
    return EFI_NOT_FOUND;
  }

  CopyMem (&PrivateData->RasConfigData, &RasConfigData, sizeof(RAS_CONFIG));
  Status = EFI_SUCCESS;
  Status = gRT->SetVariable (
                  RAS_CONFIG_VAR,
                  &gRasConfigVarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (RAS_CONFIG),
                  &PrivateData->RasConfigData
                  );
  return Status;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param[in]  This           Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Request        A null-terminated Unicode string in <ConfigRequest> format.
  @param[out] Progress       On return, points to a character in the Request string.
                             Points to the string's null terminator if request was successful.
                             Points to the most recent '&' before the first failing name/value
                             pair (or the beginning of the string if the failure is in the
                             first name/value pair) if the request was not successful.
  @param[out] Results        A null-terminated Unicode string in <ConfigAltResp> format which
                             has all values filled in for the names in the Request string.
                             String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
RasConfigExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                     Status;
  RAS_CONFIG_CALLBACK_DATA       *Private;
  EFI_STRING                     ConfigRequestHdr;
  EFI_STRING                     ConfigRequest;
  BOOLEAN                        AllocatedRequest;
  UINTN                          Size;
  UINTN                          BufferSize;

  Status = EFI_SUCCESS;
  BufferSize = sizeof (RAS_CONFIG);

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = RAS_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  Status = UpdateRasConfig (Private);
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mRasConfigGuid, RAS_CONFIG_STORE)) {
    DEBUG((DEBUG_INFO,"%a(),%d,HiiIsConfigHdrMatch not match\n",__FUNCTION__,__LINE__));
    return EFI_NOT_FOUND;
  }
  ConfigRequestHdr = NULL;
  ConfigRequest  = NULL;
  AllocatedRequest = FALSE;

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&mRasConfigGuid, RAS_CONFIG_STORE, Private->DriverHandle);
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
                                (UINT8 *) &Private->RasConfigData,
                                sizeof (RAS_CONFIG),
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
  }
  else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param[in] This                 Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in] Configuration        A null-terminated Unicode string in <ConfigResp> format.
  @param[out] Progress            A pointer to a string filled in with the offset of the most
                                  recent '&' before the first failing name/value pair (or the
                                  beginning of the string if the failure is in the first
                                  name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
RasConfigRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            Buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  RAS_CONFIG_CALLBACK_DATA         *Private;

  if (Configuration == NULL || Progress == NULL) {
      return EFI_INVALID_PARAMETER;
  }
  Private = RAS_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&ConfigRouting
                  );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (!HiiIsConfigHdrMatch (Configuration, &mRasConfigGuid, RAS_CONFIG_STORE)) {
      return EFI_NOT_FOUND;
  }
  Status = RasConfigSetupConfig (This, Private);
  Buffsize = sizeof (RAS_CONFIG);
  Status = ConfigRouting->ConfigToBlock (
                            ConfigRouting,
                            Configuration,
                            (UINT8 *) &Private->RasConfigData,
                            &Buffsize,
                            Progress
                            );

  return Status;
}

/**
  This call back function is registered with Device Info infomation formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.


  @param[in] This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in] Action          Specifies the type of action taken by the browser.
  @param[in] QuestionId      A unique value which is sent to the original exporting driver
                             so that it can identify the type of data to expect.
  @param[in] Type            The type of value for the question.
  @param[in] Value           A pointer to the data being sent to the original exporting driver.
  @param[out] ActionRequest  On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS       The callback successfully handled the action.
  @retval  Others            Other errors as indicated.

**/
EFI_STATUS
EFIAPI
RasConfigCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  RAS_CONFIG_CALLBACK_DATA      *Private;
  EFI_STATUS                    Status;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_ERROR, "%a,%d,QuestionId = 0x%lx, Action = %d\n", __func__, __LINE__, QuestionId, Action));
  Private = RAS_CONFIG_CALLBACK_DATA_FROM_THIS (This);
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
    if (QuestionId == TRIGGER_ID) {
      Status = GetRasConfigData (Private);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Update ras config failed!\n"));
      }
    }
  }

  return Status;
}

/**
  This function publish the Phytium ras configuration Form.

  @param[in, out]  PrivateData   Points to ras configuration private data.

  @retval EFI_SUCCESS            HII Form is installed successfully.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource for HII Form installation.
  @retval Others                 Other errors as indicated.

**/
EFI_STATUS
InstallRasConfigForm (
  IN OUT RAS_CONFIG_CALLBACK_DATA  *PrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;

  DriverHandle = NULL;

  ConfigAccess = &PrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mRasConfigHiiVendorDevicePath,
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
                &mRasConfigGuid,
                DriverHandle,
                RasConfigStrings,
                RasConfigUiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mRasConfigHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           ConfigAccess,
           NULL
           );
    return EFI_OUT_OF_RESOURCES;
  }

  PrivateData->HiiHandle = HiiHandle;

  //
  // Init OpCode Handle and Allocate space for creation of Buffer
  //
  mStartOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (mStartOpCodeHandle == NULL) {
    UninstallRasConfigForm (PrivateData);
    return EFI_OUT_OF_RESOURCES;
  }

  mEndOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (mEndOpCodeHandle == NULL) {
    UninstallRasConfigForm (PrivateData);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  mStartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                         mStartOpCodeHandle,
                                         &gEfiIfrTianoGuid,
                                         NULL,
                                         sizeof (EFI_IFR_GUID_LABEL)
                                         );
  mStartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  mEndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
                                       mEndOpCodeHandle,
                                       &gEfiIfrTianoGuid,
                                       NULL,
                                       sizeof (EFI_IFR_GUID_LABEL)
                                       );
  mEndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  mEndLabel->Number       = LABEL_END;

  Status = RasConfigInit ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Init ras config failed!\n"));
  }

  return EFI_SUCCESS;
}

/**
  This function removes phytium ras configuration Form.

  @param[in, out]  PrivateData   Points to ras configuration private data.

**/
VOID
UninstallRasConfigForm (
  IN OUT RAS_CONFIG_CALLBACK_DATA    *PrivateData
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
           &mRasConfigHiiVendorDevicePath,
           &gEfiHiiConfigAccessProtocolGuid,
           &PrivateData->ConfigAccess,
           NULL
           );
    PrivateData->DriverHandle = NULL;
  }

  FreePool (PrivateData);

  if (mStartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (mStartOpCodeHandle);
  }

  if (mEndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (mEndOpCodeHandle);
  }
}
