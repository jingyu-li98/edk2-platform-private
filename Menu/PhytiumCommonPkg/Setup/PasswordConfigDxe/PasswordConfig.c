/** @file
HII Config Access protocol implementation of password configuration module.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include "PasswordConfig.h"
#include "PasswordConfigFormGuid.h"

//#include <ArmPlatform.h>
//#include <OEMSvc.h>

#include <Library/HobLib.h>
//#include <Library/ParameterTable.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/EncryptionLib.h>

#define ADMIN_USER_CLEAN_TOGETHER  1

VOID                  *mStartOpCodeHandle;
VOID                  *mEndOpCodeHandle;
EFI_IFR_GUID_LABEL    *mStartLabel;
EFI_IFR_GUID_LABEL    *mEndLabel;
UINT8                 mCheckFlag;

EFI_GUID mPasswordConfigGuid = PASSWORDCONFIG_FORMSET_GUID;
extern UINT8 PasswordConfigUiBin[];
BOOLEAN  mFirstEnterPasswordConfigForm;

HII_VENDOR_DEVICE_PATH  mPasswordConfigHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    {0x89dc10fe, 0xdcf5, 0x11ed, {0x8d, 0x3e, 0x6b, 0x7f, 0x11, 0xdd, 0xb1, 0x5f}}
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

PASSWORD_CONFIG_PRIVATE_DATA gPasswordConfigPrivate = {
  PASSWORD_CONFIG_PRIVATE_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    PasswordConfigExtractConfig,
    PasswordConfigRouteConfig,
    PasswordConfigCallback
  }
};


/**
  Password configuration restore defaults.

  @param[in,out]    PasswordConfigData    Point to password configuration struct.

**/
VOID
PasswordConfigDefault (
  IN OUT PASSWORD_CONFIG_DATA  *PasswordConfigData
  )
{
  PasswordConfigData->UserPasswordEnable = 0;
  PasswordConfigData->AdminPasswordEnable = 0;
  ZeroMem (PasswordConfigData->UserPassword, sizeof (CHAR16) * PASSWD_MAXLEN);
  ZeroMem (PasswordConfigData->AdminPassword, sizeof (CHAR16) * PASSWD_MAXLEN);
}

/**
  Password configuration initialization.Get the current password configuration from variable.
  If the variable does not exist, restore the default configuration and save the variable.

  @return       EFI_SUCCESS    Initialize password configuration in the form successfully.
  @return       other          Initialize password configuration in the form failed.
**/
EFI_STATUS
PasswordConfigInit (
  VOID
  )
{
  EFI_STATUS            Status;
  PASSWORD_CONFIG_DATA  PasswordConfigData;
  UINTN                 VarSize;

  VarSize = sizeof(PASSWORD_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PASSWORD_CONFIG_NAME,
                  &gPasswordConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PasswordConfigData
                  );
  if (Status == EFI_NOT_FOUND) {
    PasswordConfigDefault (&PasswordConfigData);
    Status = gRT->SetVariable (
                    VAR_PASSWORD_CONFIG_NAME,
                    &gPasswordConfigVarGuid,
                    PLATFORM_SETUP_VARIABLE_FLAG,
                    VarSize,
                    &PasswordConfigData
                    );
    ASSERT_EFI_ERROR(Status);
  }

  return Status;
}

/**
  Get Privilege configuration data from variable and set browser data.

  @param[in]  PrivateData    A pointer to ADVANCED_CONFIG_CALLBACK_DATA.

  @retval     EFI_SUCCESS     Success.
  @retval     Other           Failed.
**/
EFI_STATUS
GetPrivData (
  IN PASSWORD_CONFIG_PRIVATE_DATA  *PrivateData
  )
{
  EFI_STATUS        Status;
  UINTN             VarSize;
  UINT8             UserPriv;

  Status = EFI_SUCCESS;
  VarSize = sizeof (UINT8);

  Status = gRT->GetVariable (
                  PASSWORD_PRIV,
                  &gPasswordPrivGuid,
                  NULL,
                  &VarSize,
                  &UserPriv
                  );

  if (Status == EFI_SUCCESS) {
    PrivateData->PasswordConfigData.UserPriv = UserPriv;
    Status = HiiSetBrowserData (
              &mPasswordConfigGuid,
              PASSWORD_CONFIG_VARIABLE,
              sizeof (PASSWORD_CONFIG_DATA),
              (UINT8 *) &PrivateData->PasswordConfigData,
              NULL
              );
  }

  return Status;
}

/**
  Get password configuration from variable and update uncommitted data in the form.

  @param[in]    PrivateData    Point to PASSWORD_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Update password configuration in the form successfully.
  @return       other          Update password configuration in the form failed.
**/
EFI_STATUS
GetPasswordConfigData (
  IN PASSWORD_CONFIG_PRIVATE_DATA  *PrivateData
  )
{
  EFI_STATUS            Status;
  PASSWORD_CONFIG_DATA  PasswordConfigData;
  UINTN                 VarSize;

  Status = EFI_SUCCESS;
  VarSize = sizeof (UINT8);

  VarSize = sizeof (PASSWORD_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PASSWORD_CONFIG_NAME,
                  &gPasswordConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PasswordConfigData
                  );

  if (Status == EFI_SUCCESS) {
    PasswordDecode(&PasswordConfigData);
    CopyMem (&PrivateData->PasswordConfigData, &PasswordConfigData, VarSize);
    Status = HiiSetBrowserData (
               &mPasswordConfigGuid,
               PASSWORD_CONFIG_VARIABLE,
               sizeof (PASSWORD_CONFIG_DATA),
               (UINT8 *) &PrivateData->PasswordConfigData,
               NULL
               );
  }
  return Status;
}

/**
  Get password configuration from variable.

  @param[in]    PrivateData    Point to PASSWORD_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Get password configuration from variable successfully.
  @return       other          Get password configuration from variable failed.
**/
EFI_STATUS
UpdatePasswordConfig (
  IN PASSWORD_CONFIG_PRIVATE_DATA  *PrivateData
)
{
  EFI_STATUS            Status;
  PASSWORD_CONFIG_DATA  PasswordConfigData;
  UINTN                 VarSize;

  Status = EFI_SUCCESS;
  VarSize = sizeof (PASSWORD_CONFIG_DATA);
  Status = gRT->GetVariable (
                  VAR_PASSWORD_CONFIG_NAME,
                  &gPasswordConfigVarGuid,
                  NULL,
                  &VarSize,
                  &PasswordConfigData
                  );
  if (Status == EFI_SUCCESS) {
    PasswordDecode(&PasswordConfigData);
    CopyMem (&PrivateData->PasswordConfigData, &PasswordConfigData, VarSize);
  }
  return Status;
}

/**
  Get password configuration from the form and set the configuration to the variable.

  @param[in]    This           Point to EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]    PrivateData    Point to PASSWORD_CONFIG_PRIVATE_DATA.

  @return       EFI_SUCCESS    Set password configuration in the form successfully.
  @return       other          Set password configuration in the form failed.
**/
EFI_STATUS
PasswordConfigSetupConfig (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN PASSWORD_CONFIG_PRIVATE_DATA           *PrivateData
  )
{
  EFI_STATUS                Status;
  PASSWORD_CONFIG_DATA      PasswordData;
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mPasswordConfigGuid, PASSWORD_CONFIG_VARIABLE, sizeof (PASSWORD_CONFIG_DATA), (UINT8 *) &PasswordData)) {
    return EFI_NOT_FOUND;
  }

  CopyMem (&PrivateData->PasswordConfigData, &PasswordData, sizeof(PASSWORD_CONFIG_DATA));
  Status = EFI_SUCCESS;

  PasswordEncryption(&PrivateData->PasswordConfigData);

  Status = gRT->SetVariable (
                  VAR_PASSWORD_CONFIG_NAME,
                  &gPasswordConfigVarGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  sizeof (PASSWORD_CONFIG_DATA),
                  &PrivateData->PasswordConfigData
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
PasswordConfigExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                     Status;
  PASSWORD_CONFIG_PRIVATE_DATA   *Private;
  EFI_STRING                     ConfigRequestHdr;
  EFI_STRING                     ConfigRequest;
  BOOLEAN                        AllocatedRequest;
  UINTN                          Size;
  UINTN                          BufferSize;

  Status = EFI_SUCCESS;
  BufferSize = sizeof (PASSWORD_CONFIG_DATA);

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Private = PASSWORD_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  UpdatePasswordConfig (Private);
  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &mPasswordConfigGuid, PASSWORD_CONFIG_VARIABLE)) {
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
    ConfigRequestHdr = HiiConstructConfigHdr (&mPasswordConfigGuid, PASSWORD_CONFIG_VARIABLE, Private->DriverHandle);
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
                                (UINT8 *) &Private->PasswordConfigData,
                                sizeof (PASSWORD_CONFIG_DATA),
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
PasswordConfigRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS                       Status;
  UINTN                            Buffsize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *ConfigRouting;
  PASSWORD_CONFIG_PRIVATE_DATA     *Private;

  if (Configuration == NULL || Progress == NULL) {
      return EFI_INVALID_PARAMETER;
  }
  Private = PASSWORD_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  *Progress = Configuration;
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **)&ConfigRouting
                  );
  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (!HiiIsConfigHdrMatch (Configuration, &mPasswordConfigGuid, PASSWORD_CONFIG_VARIABLE)) {
      return EFI_NOT_FOUND;
  }
  PasswordConfigSetupConfig (This, Private);
  Buffsize = sizeof (PASSWORD_CONFIG_DATA);
  Status = ConfigRouting->ConfigToBlock (
                            ConfigRouting,
                            Configuration,
                            (UINT8 *) &Private->PasswordConfigData,
                            &Buffsize,
                            Progress
                            );

  return Status;
}


/**
  Record password from a HII input string.

  @param[in]  Private             The pointer to the global private data structure.
  @param[in]  StringId            The QuestionId received from HII input.
  @param[in]  StringBuffer        The unicode string buffer to store password.
  @param[in]  StringBufferLen     The len of unicode string buffer.

  @retval EFI_INVALID_PARAMETER   Any input parameter is invalid.
  @retval EFI_NOT_FOUND           The password string is not found or invalid.
  @retval EFI_SUCCESS             The operation is completed successfully.

**/
EFI_STATUS
RecordPassword (
  IN   PASSWORD_CONFIG_PRIVATE_DATA  *Private,
  IN   EFI_STRING_ID                 StringId,
  IN   CHAR16                        *StringBuffer,
  IN   UINTN                         StringBufferLen
  )
{
  CHAR16  *Password;

  if ((StringId == 0) || (StringBuffer == NULL) || (StringBufferLen <= 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Password = HiiGetString (Private->HiiHandle, StringId, NULL);
  if (Password == NULL) {
    return EFI_NOT_FOUND;
  }

  if (StrLen (Password) > StringBufferLen) {
    FreePool (Password);
    return EFI_NOT_FOUND;
  }

  StrnCpyS (StringBuffer, StringBufferLen, Password, StrLen (Password));
  ZeroMem (Password, (StrLen (Password) + 1) * sizeof (CHAR16));
  FreePool (Password);

  //
  // Clean password in string package
  //
  HiiSetString (Private->HiiHandle, StringId, L"", NULL);
  return EFI_SUCCESS;
}



/**
  Clean User Password,need admin right

  @param[in]  Private       A pointer to ADVANCED_CONFIG_CALLBACK_DATA.
  @param[in]  QuestionId    A unique value which is sent to the original exporting driver
                            so that it can identify the type of data to expect.
  @param[in]  Value         A pointer to the data being sent to the original exporting driver.
EFI_NOT_READY
  @retval     EFI_SUCCESS                   Success.
  @retval     EFI_ALREADY_STARTED           Old password exist.
  @retval     EFI_NOT_READY                 Typed in old password incorrect.
**/
EFI_STATUS
CleanUserPasswordConfigData (
  IN  PASSWORD_CONFIG_PRIVATE_DATA  *Private,
  IN  EFI_QUESTION_ID               QuestionId,
  IN  EFI_IFR_TYPE_VALUE            *Value
  )
{
  EFI_STATUS                     Status;
  UINTN                          Length;
  EFI_INPUT_KEY                  Key;
  CHAR16                         *ConfirmCleanPasswdString;

  Length = sizeof (CHAR16) * PASSWD_MAXLEN;

  if(Private->PasswordConfigData.UserPriv == 1)
  {
    return EFI_NOT_READY;
  }

    do {
        ConfirmCleanPasswdString = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_CONFIRM_CLEAN_PASSWD), NULL);
        CreatePopUp (EFI_WHITE | EFI_BACKGROUND_BLUE, &Key, L" ", ConfirmCleanPasswdString, L" ",NULL);
    } while ((Key.ScanCode != SCAN_ESC) && (Key.UnicodeChar != CHAR_CARRIAGE_RETURN));

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN){
        //DEBUG ((DEBUG_ERROR, "Private->PasswordConfigData.UserPassword is before encry %s",Private->PasswordConfigData.UserPassword));
        SetMem (&Private->PasswordConfigData.UserPassword, Length, 0x00);
	Private->PasswordConfigData.UserPasswordEnable = 0;  
	PasswordEncryption(&Private->PasswordConfigData);
        Status = gRT->SetVariable (
            VAR_PASSWORD_CONFIG_NAME,
            &gPasswordConfigVarGuid,
            PLATFORM_SETUP_VARIABLE_FLAG,
            sizeof (PASSWORD_CONFIG_DATA),
            &Private->PasswordConfigData
            );
        GetPasswordConfigData (Private);
    }

    return EFI_SUCCESS;
}



/**
  Process callback function PasswordCheck().
  Get Password configuration data from variable and set browser data.

  @param[in]  Private       A pointer to ADVANCED_CONFIG_CALLBACK_DATA.
  @param[in]  QuestionId    A unique value which is sent to the original exporting driver
                            so that it can identify the type of data to expect.
  @param[in]  Value         A pointer to the data being sent to the original exporting driver.

  @retval     EFI_SUCCESS                   Success.
  @retval     EFI_ALREADY_STARTED           Old password exist.
  @retval     EFI_NOT_READY                 Typed in old password incorrect.
**/
EFI_STATUS
ProcessPasswordConfigData (
  IN  PASSWORD_CONFIG_PRIVATE_DATA  *Private,
  IN  EFI_QUESTION_ID               QuestionId,
  IN  EFI_IFR_TYPE_VALUE            *Value
  )
{
  CHAR16                         *TempPassword;
  CHAR16                         *OldPassword;
  EFI_STATUS                     Status;
  UINTN                          VarSize;
  UINTN                          Length;
  UINT8                          PasswordExist;
  PASSWORD_CONFIG_DATA           PasswordConfigData;
  EFI_INPUT_KEY                  Key;

  CHAR16                      *ConfirmPasswdString;

  #if ADMIN_USER_CLEAN_TOGETHER 
  CHAR16                      *CleanUserPasswdWarning;
  #endif

  CHAR16                      *UserAndAdminPasswdSameString;
  CHAR16                      *AdminAndUserPasswdSameString;
  CHAR16                      *ReInputPasswdString;

  VarSize = sizeof (PASSWORD_CONFIG_DATA);
  Length = sizeof (CHAR16) * PASSWD_MAXLEN;
  TempPassword = AllocateZeroPool (Length);
  OldPassword = AllocateZeroPool (Length);

  PasswordExist = 0;
  // Use a NULL password to test whether old password is required
  Status = RecordPassword (Private, Value->string, TempPassword, PASSWD_MAXLEN);
  if (EFI_ERROR (Status)) {
    mCheckFlag = 0;
    DEBUG ((DEBUG_ERROR, "Error: Failed to input password!"));
    return Status;
  }
  //1st,check Old password exist
  Status = gRT->GetVariable (
    VAR_PASSWORD_CONFIG_NAME,
    &gPasswordConfigVarGuid,
    NULL,
    &VarSize,
    &PasswordConfigData
    );

  PasswordDecode(&PasswordConfigData);

  if (QuestionId == FORM_USER_PASSWD_OPEN && StrLen (PasswordConfigData.UserPassword) > 0) {
    PasswordExist = 1;
    CopyMem (OldPassword, PasswordConfigData.UserPassword, Length);
  } else if (QuestionId == FORM_ADMIN_PASSWD_OPEN && StrLen (PasswordConfigData.AdminPassword) > 0) {
    PasswordExist = 1;
    CopyMem (OldPassword, PasswordConfigData.AdminPassword, Length);
  }

  if (TempPassword == NULL) {
    if (PasswordExist == 1) {
      //Old password exist
      mCheckFlag = 0;
      Status = EFI_ALREADY_STARTED;
    } else {
      //Old password not exist
      mCheckFlag = 1;
      Status = EFI_SUCCESS;
    }
  } else if (TempPassword != NULL && mCheckFlag == 0) {
    //2nd Check user input old password
    if (StrCmp (OldPassword, TempPassword) == 0) {
      // Typed in old password correct
      mCheckFlag = 1;
      Status = EFI_SUCCESS;
    } else {
      // Typed in old password incorrect
      Status = EFI_NOT_READY;
    }
  } else if (TempPassword != NULL && mCheckFlag == 1) {
    //3rd,Save new password
    if (QuestionId == FORM_USER_PASSWD_OPEN) {
      if ((StrCmp (TempPassword, Private->PasswordConfigData.AdminPassword) == 0) && StrLen (TempPassword) > 0) {
        do {
          UserAndAdminPasswdSameString = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_USER_ADMIN_SAME), NULL);
          ReInputPasswdString          = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_REINPUT_PASSWD), NULL);
          CreatePopUp (EFI_WHITE | EFI_BACKGROUND_BLUE, &Key, L" ", UserAndAdminPasswdSameString, ReInputPasswdString,NULL);
        } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        mCheckFlag = 0;
        goto ProcExit;
      }
      CopyMem (Private->PasswordConfigData.UserPassword, TempPassword, Length);
      if (StrLen (TempPassword) > 0) {
        Private->PasswordConfigData.UserPasswordEnable = 1;
      } else {
        Private->PasswordConfigData.UserPasswordEnable = 0;
      }
    } else if (QuestionId == FORM_ADMIN_PASSWD_OPEN) {
      if (StrCmp (TempPassword, Private->PasswordConfigData.UserPassword) == 0 && StrLen (TempPassword) > 0) {
        do {
          AdminAndUserPasswdSameString = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_ADMIN_USER_SAME), NULL);
          ReInputPasswdString          = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_REINPUT_PASSWD), NULL);
          CreatePopUp (EFI_WHITE | EFI_BACKGROUND_BLUE, &Key, L" ", AdminAndUserPasswdSameString, ReInputPasswdString,NULL);
        } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        mCheckFlag = 0;
        goto ProcExit;
      }
      CopyMem (Private->PasswordConfigData.AdminPassword, TempPassword, Length);
      if (StrLen (TempPassword) > 0) {
        Private->PasswordConfigData.AdminPasswordEnable = 1;
      } else {
        Private->PasswordConfigData.AdminPasswordEnable = 0;
      }
    }
    do {
	#if ADMIN_USER_CLEAN_TOGETHER
	if(Private->PasswordConfigData.AdminPasswordEnable == 0)
	{
	   ConfirmPasswdString     = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_CONFIRM_PASSWD), NULL);
	   CleanUserPasswdWarning  = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_CLEAN_USER_PASSWORD_WARNING), NULL);
           CreatePopUp (EFI_WHITE | EFI_BACKGROUND_BLUE, &Key, CleanUserPasswdWarning, ConfirmPasswdString, L" ",NULL);
	}
	else
	#endif
	{
           ConfirmPasswdString = HiiGetString (Private->HiiHandle, STRING_TOKEN (STR_CONFIRM_PASSWD), NULL);
           CreatePopUp (EFI_WHITE | EFI_BACKGROUND_BLUE, &Key, L" ", ConfirmPasswdString, L" ",NULL);
	}
    } while ((Key.ScanCode != SCAN_ESC) && (Key.UnicodeChar != CHAR_CARRIAGE_RETURN));
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN){
	#if ADMIN_USER_CLEAN_TOGETHER
	if(Private->PasswordConfigData.AdminPasswordEnable == 0)
	{
            SetMem (&Private->PasswordConfigData.UserPassword, (sizeof (CHAR16) * PASSWD_MAXLEN), 0x00);
            Private->PasswordConfigData.UserPasswordEnable = 0;
	}
	#endif
        PasswordEncryption(&Private->PasswordConfigData);
        Status = gRT->SetVariable (
            VAR_PASSWORD_CONFIG_NAME,
            &gPasswordConfigVarGuid,
            PLATFORM_SETUP_VARIABLE_FLAG,
            sizeof (PASSWORD_CONFIG_DATA),
            &Private->PasswordConfigData
            );
        GetPasswordConfigData (Private);
    }
    Status = EFI_SUCCESS;
    mCheckFlag = 0;
  }
ProcExit:
  FreePool (TempPassword);
  FreePool (OldPassword);
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
PasswordConfigCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  PASSWORD_CONFIG_PRIVATE_DATA  *Private;
  EFI_STATUS                    Status;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_ERROR, "%a,%d,QuestionId = 0x%lx, Action = %d\n", __func__, __LINE__, QuestionId, Action));
  Private = PASSWORD_CONFIG_PRIVATE_DATA_FROM_THIS (This);
  if (Action == EFI_BROWSER_ACTION_FORM_OPEN) {
    if (QuestionId == TRIGGER_ID) {
      Status = GetPasswordConfigData (Private);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Update password config failed!\n"));
      }
      Status = GetPrivData(Private);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Get Privilege configuration data failed!\n"));
      }
    }
  } else if (Action == EFI_BROWSER_ACTION_CHANGING) {
    if (QuestionId == FORM_USER_PASSWD_OPEN || QuestionId == FORM_ADMIN_PASSWD_OPEN) {
      Status = ProcessPasswordConfigData (Private, QuestionId, Value);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Process Password Config Data failed!\n"));
      }
    }
    if (QuestionId == FORM_CLEAN_USER_PASSWORD) {
      Status = CleanUserPasswordConfigData (Private, QuestionId, Value);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Process Clean user password failed!\n"));
      }
    }
  }

  return Status;
}

/**
  This function publish the Phytium password configuration Form.

  @param[in, out]  PrivateData   Points to password configuration private data.

  @retval EFI_SUCCESS            HII Form is installed successfully.
  @retval EFI_OUT_OF_RESOURCES   Not enough resource for HII Form installation.
  @retval Others                 Other errors as indicated.

**/
EFI_STATUS
InstallPasswordConfigForm (
  IN OUT PASSWORD_CONFIG_PRIVATE_DATA  *PrivateData
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL  *ConfigAccess;

  DriverHandle = NULL;
  mCheckFlag = 0;
  mFirstEnterPasswordConfigForm = FALSE;
  ConfigAccess = &PrivateData->ConfigAccess;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mPasswordConfigHiiVendorDevicePath,
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
                &gPasswordConfigFormSetGuid,
                DriverHandle,
                PasswordConfigUiDxeStrings,
                PasswordConfigUiBin,
                NULL
                );
  if (HiiHandle == NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           DriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mPasswordConfigHiiVendorDevicePath,
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
    UninstallPasswordConfigForm (PrivateData);
    return EFI_OUT_OF_RESOURCES;
  }

  mEndOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (mEndOpCodeHandle == NULL) {
    UninstallPasswordConfigForm (PrivateData);
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
  //mStartLabel->Number       = LABEL_FORM_PASSWORDCONFIG_START;

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
  //mEndLabel->Number       = LABEL_FORM_PASSWORDCONFIG_END;

  PasswordConfigInit ();

  return EFI_SUCCESS;
}

/**
  This function removes phytium password configuration Form.

  @param[in, out]  PrivateData   Points to password configuration private data.

**/
VOID
UninstallPasswordConfigForm (
  IN OUT PASSWORD_CONFIG_PRIVATE_DATA    *PrivateData
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
           &mPasswordConfigHiiVendorDevicePath,
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
