/** @file
Definition of bmc command and page initialization function.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <BmcSetup.h>
#include <SetupBmcConfig.h>

#include <Library/IpmiBaseLib.h>
#include <Library/StringBaseLib.h>

UINT8  IsBmcConfigChanged = 0;
UINT8  mUserId[MAX_USER_NUMBER + 1] = {0};
UINT16 UserNames[] = {STR_BMC_USER_NAME_VAL,    STR_BMC_USER_NAME_VAL,
                      STR_BMC_USER_NAME_VAL_2,  STR_BMC_USER_NAME_VAL_3,
                      STR_BMC_USER_NAME_VAL_4,  STR_BMC_USER_NAME_VAL_5,
                      STR_BMC_USER_NAME_VAL_6,  STR_BMC_USER_NAME_VAL_7,
                      STR_BMC_USER_NAME_VAL_8,  STR_BMC_USER_NAME_VAL_9,
                      STR_BMC_USER_NAME_VAL_10, STR_BMC_USER_NAME_VAL_11,
                      STR_BMC_USER_NAME_VAL_12, STR_BMC_USER_NAME_VAL_13,
                      STR_BMC_USER_NAME_VAL_14, STR_BMC_USER_NAME_VAL_15};
/**
  Get Chassis Status command:
  NetFunction Id is 0x00, command Id is 0x01.

  @param[out]  ChassisStatus  A pointer to BMC Chassis Status information.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
EFIAPI
IpmiGetChassisStatus (
  OUT IPMI_GET_CHASSIS_STATUS_RESPONSE  *ChassisStatus
  )
{
  EFI_STATUS                   Status;
  UINT8                        DataSize;

  DataSize = sizeof (IPMI_GET_CHASSIS_STATUS_RESPONSE);
  Status = IpmiSubmitCommand (
             IPMI_NETFN_CHASSIS,
             IPMI_CHASSIS_GET_STATUS,
             NULL,
             0,
             (VOID *)ChassisStatus,
             &DataSize
             );
  return Status;
}

/**
  Set Power Restore Policy command:
  NetFunction Id is 0x00, command Id is 0x06.

  @param[in]   PowerRestorePolictRequest  A pointer to BMC Power Restore Polict Request information.
  @param[out]  PowerRestorePolictResponse A pointer to BMC Power Restore Polict Response information.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.

**/
EFI_STATUS
EFIAPI
IpmiSetPowerPolicy (
  IN  IPMI_SET_POWER_RESTORE_POLICY_REQUEST  *PowerRestorePolictRequest,
  OUT IPMI_SET_POWER_RESTORE_POLICY_RESPONSE *PowerRestorePolictResponse
  )
{
  EFI_STATUS                  Status;
  UINT8                       DataSize;

  ZeroMem (PowerRestorePolictResponse, sizeof (IPMI_SET_POWER_RESTORE_POLICY_RESPONSE));
  DataSize = sizeof (IPMI_SET_POWER_RESTORE_POLICY_RESPONSE);
  Status = IpmiSubmitCommand (
               IPMI_NETFN_CHASSIS,
               IPMI_CHASSIS_SET_POWER_RESTORE_POLICY,
               (VOID *)PowerRestorePolictRequest,
               sizeof (IPMI_SET_POWER_RESTORE_POLICY_REQUEST),
               (VOID*)PowerRestorePolictResponse,
               &DataSize
               );
  if (PowerRestorePolictResponse->CompletionCode != 0) {
    DEBUG ((DEBUG_ERROR, "CompletionCode = %d,%d\n", PowerRestorePolictResponse->CompletionCode, __LINE__));
    DEBUG ((DEBUG_ERROR, "PowerRestorePolicySupport = %d,%d\n", PowerRestorePolictResponse->PowerRestorePolicySupport, __LINE__));
    Status = EFI_INVALID_PARAMETER;
  }
  return Status;
}

/**
  Get user name command:
  NetFunction Id is 0x06, command Id is 0x46.

  @param[out]  TempBmcConfig  A pointer to configuration.
  @param[in]   Handle   hii handle.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
EFIAPI
CollectAllBmcUserDetails (
  OUT BMC_CONFIG         *TempBmcConfig,
  IN  EFI_HII_HANDLE     Handle
)
{
  EFI_STATUS                  Status;
  IPMI_GET_USER_NAME_RESPONSE GetUserNameResponse;
  IPMI_GET_USER_NAME_REQUEST  GetUserNameRequest;
  UINT8                       DataSize;
  UINT8                       Index;

  TempBmcConfig->UserTotalNum = 0;
  for (Index = 1; Index <= MAX_USER_NUMBER; Index++) {
    ZeroMem (&GetUserNameResponse, sizeof (IPMI_GET_USER_NAME_RESPONSE));
    ZeroMem (&GetUserNameRequest, sizeof (IPMI_GET_USER_NAME_REQUEST));
    GetUserNameRequest.UserId.Uint8 = Index;
    DataSize = sizeof (IPMI_GET_USER_NAME_RESPONSE);

    Status = IpmiSubmitCommand (
                IPMI_NETFN_APP,
                IPMI_APP_GET_USER_NAME,
                (UINT8 *)&GetUserNameRequest,
                sizeof (IPMI_GET_USER_NAME_REQUEST),
                (UINT8 *)&GetUserNameResponse,
                &DataSize
                );

    if (AsciiStrLen ((const CHAR8 *)GetUserNameResponse.UserName) > 0) {
      TempBmcConfig->UserTotalNum++;
      mUserId[TempBmcConfig->UserTotalNum] = Index;
      // User Name
      BmcInfoSetHiiString (
        Handle,
        STRING_TOKEN (UserNames[TempBmcConfig->UserTotalNum]),
        L"%a",
        GetUserNameResponse.UserName
      );
    }
  }
  return Status;
}

/**
  Set user password command:
  NetFunction Id is 0x06, command Id is 0x47.

  @param[in]   UserId        BMC User Id.
  @param[in]   PasswordSize  Password Size .
  @param[in]   Operation     Tells what operation to perform.
  @param[in]   UserPassword  Password.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
EFIAPI
SetUserPassword (
  IN UINT8 UserId,
  IN UINT8 PasswordSize,
  IN UINT8 Operation,
  IN CHAR8 *UserPassword
 )
{
  IPMI_SET_USER_PASSWORD_CMD      SetPassword;
  UINT8                           CompeleCode;
  EFI_STATUS                      Status;
  UINT8                           DataSize;
  EFI_INPUT_KEY                   Key;
  CHAR16                                  StringBuffer1[32];
  CHAR16                                  StringBuffer2[64];
  CHAR16                                  StringBuffer3[32];

  ZeroMem (&SetPassword, sizeof (IPMI_SET_USER_PASSWORD_CMD));

  SetPassword.UserId = UserId | (PasswordSize << 7);
  SetPassword.Operation = Operation;
  CompeleCode = 0;
  DataSize = sizeof (UINT8);
  CopyMem (SetPassword.PasswordData, UserPassword, AsciiStrLen (UserPassword));
  DEBUG((DEBUG_INFO,"%a(),%d, data %a , UserId %x over\n",__FUNCTION__,__LINE__, SetPassword.PasswordData, SetPassword.UserId));
  Status = IpmiSubmitCommand (
            IPMI_NETFN_APP,
            IPMI_APP_SET_USER_PASSWORD,
            (VOID *)&SetPassword,
            sizeof (SetPassword),
            &CompeleCode,
            &DataSize
            );
  if (CompeleCode == 0x80 || CompeleCode == 0x81) {
    do {
      StrCpyS (StringBuffer1, StrLen (L"Invalid password.") + 1, L"Invalid password.");
      StrCpyS (StringBuffer2, StrLen (L"Config password in 6-20 characters with high complexity") + 1, L"Config password in 6-20 characters with high complexity");
      StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
      CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
    } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
  }
  return Status;
}

/**
  Get/Set IP address,subnet mask, and Gateway IP of BMC channels.

  @param[in]       Channel       Channel to set.
  @param[in]       LanParameter  BMC Command Parameter to set/get IP,Subnet mask or Gateway IP.
  @param[in,out]   IpAddress     IPv4 format address to Set/Get.
  @param[in]       CommandGet    TRUE - Read Lan parameter, False = Set Lan parameter.

  @retval          EFI_SUCCESS       Command completed successfully.
  @retval          EFI_DEVICE_ERROR  IPMI command failed.
  @retval          EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval          EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval          EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
GetSetIpPram (
  IN     UINT8          Channel,
  IN     UINT8          LanParameter,
  IN OUT UINT16         *IpAddress,
  IN     BOOLEAN        CommandGet
  )
{
  UINT8       Commanddata[20];
  UINT8       Commanddatasize;
  UINT8       Response[10];
  UINT8       Responsesize;
  UINT8       Ipadd[4];
  EFI_STATUS  Status;

//
//Count number of times command has been sent to BMC.
//

  if (CommandGet == 1) {
    Commanddata[0]  = Channel;
    Commanddata[1]  = LanParameter;
    Commanddata[2]  = 0;
    Commanddata[3]  = 0;
    Commanddatasize = 4;
    Responsesize = 10;

    Status = IpmiSubmitCommand (
              EFI_SM_NETFN_LAN,
              EFI_GET_LAN_CONFIG_CMD,
              (UINT8 *) &Commanddata[0],
              Commanddatasize,
              (UINT8 *) &Response,
              (UINT8 *) &Responsesize
              );
    if (Status == EFI_SUCCESS) {
      if (Response[0] == 0x80) {
        return EFI_INVALID_PARAMETER;
      }
      IpAddtoStr (IpAddress, &Response[2]);
    }
  } else {
    Commanddata[0]  = Channel;
    Commanddata[1]  = LanParameter;
    StrStringToIp4Adr ((EFI_IPv4_ADDRESS *) &Ipadd[0], IpAddress);
    CopyMem (&Commanddata[2], &Ipadd[0], 4);
    Commanddatasize = 6;
    Responsesize = 10;

    Status = IpmiSubmitCommand (
              EFI_SM_NETFN_LAN,
              EFI_SET_LAN_CONFIG_CMD,
              (UINT8 *) &Commanddata[0],
              Commanddatasize,
              (UINT8 *) &Response,
              (UINT8 *) &Responsesize
              );
  }
  return Status;
}

/**
  Fill LAN configuartion for given Channel.

  @param[in]   Laninfo  Ponter to LAN configuration.
  @param[in]   Channel  Lan Channel.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
BmcGetLanConfig (
  IN BMC_CONFIG       *Laninfo,
  IN UINT8            Channel
  )
{
  UINT8       Commanddata[20];
  UINT8       Commanddatasize;
  UINT8       Response[20];
  UINT8       Responsesize;
  UINTN       Index;
  EFI_STATUS  Status;

  ZeroMem (Commanddata, 20);
  ZeroMem (Response, 20);

  //
  // Get IP Source (DHCP/ STATIC)
  //
  Commanddata[0] = Channel;         // Channel number
  Commanddata[1] = IP_SOURCE_PARM;  // Parameter selector
  Commanddata[2] = 0;               // Set Selector.
  Commanddata[3] = 0;               // Block Selector
  Commanddatasize = 4;
  Responsesize = 20;

  Status = IpmiSubmitCommand (
            EFI_SM_NETFN_LAN,           // NetFunction
            EFI_GET_LAN_CONFIG_CMD,     // Command
            (UINT8 *) &Commanddata[0],  // *CommandData
            Commanddatasize,            // CommandDataSize
            (UINT8 *) &Response,        // *ResponseData
            (UINT8 *) &Responsesize     // *ResponseDataSize
            );

  DEBUG ((DEBUG_INFO, "BmcGetLanConfig  -%r response:\n",Status));
  for(Index = 0 ;Index < 20;Index++){
    DEBUG ((DEBUG_INFO, "%02x ",*(Response+Index)));
  }
  DEBUG((DEBUG_INFO,"\n"));

  if (Status == EFI_SUCCESS) {
    if (Response[0] == 0x80) {
      return EFI_INVALID_PARAMETER;
    }
    Laninfo->NicIpSource = Response[2]; // Response1 = IP Source
  } else {
    Laninfo->NicIpSource = 2;
  }
  //
  // Get Ip Address,Subnet Mask and Gateway Ip
  //
  GetSetIpPram (Channel, IP_ADDRESS_PARM, &Laninfo->IpAddress[0], TRUE);
  GetSetIpPram (Channel, SUBNET_MASK_PARM, &Laninfo->SubnetMask[0], TRUE);
  GetSetIpPram (Channel, GATEWAY_IP_PARM, &Laninfo->GatewayIp[0], TRUE);
  return Status;
}

/**
  Send LAN configuration to BMC.

  @param[in]   Laninfo  Ponter to LAN configuration.
  @param[in]   Channel  Lan Channel.

  @retval      EFI_SUCCESS       Command completed successfully.
  @retval      EFI_DEVICE_ERROR  IPMI command failed.
  @retval      EFI_UNSUPPORTED   Command is not supported by BMC.
  @retval      EFI_BUFFER_TOO_SMALL   Response buffer is too small.
  @retval      EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
BmcSetLanConfig (
  IN BMC_CONFIG       *Laninfo,
  IN UINT8            Channel
  )
{
  UINT8       Commanddata[20];
  UINT8       Commanddatasize;
  UINT8       Response[20];
  UINT8       Responsesize;
  EFI_STATUS  Status;

  ZeroMem (Commanddata, 20);
  ZeroMem (Response, 20);
  //
  // Set IP Source
  //
  Commanddata[0] = Channel;               // Channel number
  Commanddata[1] = IP_SOURCE_PARM;        // Parameter selector
  Commanddata[2] = Laninfo->NicIpSource;  // IP Source (STATIC/ DYANAMIC)
  Commanddatasize = 3;
  Responsesize    = 10;

  //
  //If BMC LAN configuration IP source is not changed, then issue command once.
  //
  Status = IpmiSubmitCommand (
            EFI_SM_NETFN_LAN,           // NetFunction
            EFI_SET_LAN_CONFIG_CMD,     // Command
            (UINT8 *) &Commanddata[0],  // *CommandData
            Commanddatasize,            // CommandDataSize
            (UINT8 *) &Response,        // *ResponseData
            (UINT8 *) &Responsesize     // *ResponseDataSize
            );
  DEBUG((DEBUG_INFO," Setup Laninfo->NicIpSource = %x - %r\n",Laninfo->NicIpSource,Status));

  if (Laninfo->NicIpSource != 2) {
    //
    // IP Source is Static.Set Ip Address,Subnet Mask and Gateway Ip
    //
    GetSetIpPram (Channel, IP_ADDRESS_PARM, &Laninfo->IpAddress[0], FALSE);
    GetSetIpPram (Channel, SUBNET_MASK_PARM, &Laninfo->SubnetMask[0], FALSE);
    GetSetIpPram (Channel, GATEWAY_IP_PARM, &Laninfo->GatewayIp[0], FALSE);
  }

  return Status;
}

/**
  Record password from a HII input string.

  @param[in]  Private             The pointer to the global private data structure.
  @param[in]  StringId            The QuestionId received from HII input.
  @param[in]  StringBuffer        The unicode string buffer to store password.
  @param[in]  StringBufferLen     The len of unicode string buffer.

  @retval     EFI_INVALID_PARAMETER   Any input parameter is invalid.
  @retval     EFI_NOT_FOUND           The password string is not found or invalid.
  @retval     EFI_SUCCESS             The operation is completed successfully.
**/
EFI_STATUS
RecordPassword (
  IN   BMC_INFO_CALLBACK_DATA        *Private,
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
  Get Password configuration data and set bmc user password.

  @param[in]  Private       A pointer to ADVANCED_CONFIG_CALLBACK_DATA.
  @param[in]  QuestionId    A unique value which is sent to the original exporting driver
                            so that it can identify the type of data to expect.
  @param[in]  Value         A pointer to the data being sent to the original exporting driver.

  @retval     EFI_SUCCESS                   Success.
  @retval     EFI_ALREADY_STARTED           Old password exist.
  @retval     EFI_NOT_READY                 Typed in old password incorrect.
**/
EFI_STATUS
ProcessPasswordSet (
  IN  BMC_INFO_CALLBACK_DATA        *Private,
  IN  EFI_QUESTION_ID               QuestionId,
  IN  EFI_IFR_TYPE_VALUE            *Value
  )
{
  EFI_STATUS                     Status;
  CHAR8                          *UserPassword;
  CHAR16                         *TempPassword;
  UINT8                          Offset;

  UserPassword = AllocateZeroPool (sizeof (CHAR8) * MAX_BMC_USER_LENGTH);
  TempPassword = AllocateZeroPool (sizeof (CHAR16) * MAX_BMC_USER_LENGTH);
  Offset = QuestionId - KEY_PASSWORD_BASE;

  Status = RecordPassword (Private, Value->string, TempPassword, MAX_BMC_USER_LENGTH);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Failed to input password!"));
  } else {
    if (StrLen (TempPassword) < MIN_BMC_USER_LENGTH) {
      Status = EFI_SUCCESS;
    } else if (StrLen (TempPassword) < MAX_BMC_USER_LENGTH) {
      Status = EFI_SUCCESS;
      UnicodeStrToAsciiStrS (TempPassword, UserPassword, StrLen (TempPassword) + 1);
      Status = SetUserPassword (mUserId[Offset], IPMI_SET_USER_PASSWORD_PASSWORD_SIZE_20, IPMI_SET_USER_PASSWORD_OPERATION_TYPE_SET_PASSWORD, UserPassword);
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "\n[IPMI] BMC does not respond (status: %r)!\n\n", Status));
      }
    }
  }
  FreePool (UserPassword);
  FreePool (TempPassword);
  return Status;
}

/**
  Validate user inputs entered.

  @param[in]  This               Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param[in]  Action             Specifies the type of action taken by the browser.
  @param[in]  QuestionId         A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect. The format of the data tends to
                                 vary based on the opcode that enerated the callback.
  @param[in]  Type               The type of value for the question.
  @param[in]  Value              A pointer to the data being sent to the original
                                 exporting driver.
  @param[out] ActionRequest     On return, points to the action requested by the
                                 callback function.

  @retval     EFI_SUCCESS            The callback successfully handled the action.
  @retval     EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                     variable and its data.
  @retval     EFI_DEVICE_ERROR       The variable could not be saved.
  @retval     EFI_UNSUPPORTED        The specified Action is not supported by the
                                     callback.Currently not implemented.
  @retval     EFI_INVALID_PARAMETERS Passing in wrong parameter.
  @retval     Others                 Other errors as indicated.
**/
EFI_STATUS
ValidateSetupInput (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                         Status;
  EFI_IPv4_ADDRESS                   IpAdd;
  EFI_IPv4_ADDRESS                   SubnetMask;
  EFI_IPv4_ADDRESS                   GatewayIp;
  EFI_INPUT_KEY                      Key;
  BMC_CONFIG                         TempBmcConfig;
  IPMI_GET_CHASSIS_STATUS_RESPONSE   ChassisStatus;
  WATCHDOG_CONFIG                    WatchdogConfig;
  UINTN                              VarSize;
  BMC_INFO_CALLBACK_DATA             *Private;
  CHAR16                             StringBuffer1[32];
  CHAR16                             StringBuffer2[64];
  CHAR16                             StringBuffer3[32];

  ZeroMem (&TempBmcConfig, sizeof(BMC_CONFIG));
  ZeroMem (StringBuffer1, sizeof(StringBuffer1));
  ZeroMem (StringBuffer2, sizeof(StringBuffer2));
  ZeroMem (StringBuffer3, sizeof(StringBuffer3));
  VarSize = sizeof (WATCHDOG_CONFIG);

  Private = BMC_INFO_CALLBACK_DATA_FROM_THIS(This);
  DEBUG ((DEBUG_INFO, "BMC Cfg callback QuestionID:%d  Action:%d\n",QuestionId,Action));
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mBmcInfoGuid, BMC_CONFIG_STORE, sizeof (BMC_CONFIG), (UINT8 *) &TempBmcConfig)) {
    return EFI_NOT_FOUND;
  }
  Status = EFI_SUCCESS;

  Status = CollectAllBmcUserDetails (&TempBmcConfig, Private->HiiHandle);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "\n[IPMI] BMC does not respond (status: %r)!\n\n", Status));
    return Status;
  }
  //
  // Check Lan parameters IP address , subnet Mask ,GateWay IP format
  //
  if ((QuestionId >= KEY_IP) && (QuestionId <= KEY_SUBNETMASK)) {
    BmcGetLanConfig (&TempBmcConfig, BASE_BOARD_NIC_CHANNEL_SEL);
    switch (QuestionId) {
    case KEY_IP:
      //get ipv4
      Status = StrStringToIp4Adr (&IpAdd, TempBmcConfig.IpAddress);
      if (Status == EFI_INVALID_PARAMETER) {
        StrCpyS (TempBmcConfig.IpAddress, StrLen (Private->BmcConfig.IpAddress) + 1, Private->BmcConfig.IpAddress);
        do {
            StrCpyS (StringBuffer1, StrLen (L"Invalid IP Address.") + 1, L"Invalid IP Address.");
            StrCpyS (StringBuffer2, StrLen (L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx");
            StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
            CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
          } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        } else {
          StrCpyS (Private->BmcConfig.IpAddress, StrLen (TempBmcConfig.IpAddress) + 1,TempBmcConfig.IpAddress);
        }
        break;
      case KEY_GW:
      //get gw
      Status = StrStringToIp4Adr (&GatewayIp, TempBmcConfig.GatewayIp);
      if (Status == EFI_INVALID_PARAMETER) {
        StrCpyS (TempBmcConfig.GatewayIp, StrLen (Private->BmcConfig.GatewayIp) + 1, Private->BmcConfig.GatewayIp);
        do {
            StrCpyS (StringBuffer1, StrLen (L"Invalid GatewayIp.") + 1, L"Invalid GatewayIp.");
            StrCpyS (StringBuffer2, StrLen (L"Please Config Correct GatewayIp in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx");
            StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
            CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
          } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        } else {
          StrCpyS (Private->BmcConfig.GatewayIp, StrLen (TempBmcConfig.GatewayIp) + 1,TempBmcConfig.GatewayIp);
        }
      break;
      case KEY_SUBNETMASK:
      //get SubnetMask
      Status = StrStringToIp4Adr (&SubnetMask, TempBmcConfig.SubnetMask);
      if (Status == EFI_INVALID_PARAMETER) {
        StrCpyS (TempBmcConfig.SubnetMask, StrLen (Private->BmcConfig.SubnetMask) + 1, Private->BmcConfig.SubnetMask);
        do {
            StrCpyS (StringBuffer1, StrLen (L"Invalid SubnetMask.") + 1, L"Invalid SubnetMask.");
            StrCpyS (StringBuffer2, StrLen (L"Please Config Correct SubnetMask in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx");
            StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
            CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
          } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
        } else {
          StrCpyS (Private->BmcConfig.SubnetMask, StrLen (TempBmcConfig.SubnetMask) + 1, TempBmcConfig.SubnetMask);
        }
      break;
    default:
      break;
    }
  }

  Status = IpmiGetChassisStatus (&ChassisStatus);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "\n[IPMI] BMC does not respond (status: %r)!\n\n", Status));
    return Status;
  } else {
    if (((ChassisStatus.CurrentPowerState >> 5) & 3) == 0) {
      TempBmcConfig.RestorePolicy = POWER_OFF;
    } else if (((ChassisStatus.CurrentPowerState >> 5) & 3) == 1) {
      TempBmcConfig.RestorePolicy = POWER_RESTORE;
    } else if (((ChassisStatus.CurrentPowerState >> 5) & 3) == 2) {
      TempBmcConfig.RestorePolicy = POWER_UP;
    } else if (((ChassisStatus.CurrentPowerState >> 5) & 3) == 3) {
      TempBmcConfig.RestorePolicy = POWER_UNKNOWN;
    }
  }

  Status = gRT->GetVariable (
  WATCHDOG_CONFIG_VAR,
  &gWatchdogGuid,
  NULL,
  &VarSize,
  &WatchdogConfig
  );

  if (Status == EFI_SUCCESS) {
    TempBmcConfig.WatchdogPost = WatchdogConfig.WatchdogPost;
    TempBmcConfig.WatchdogTimeoutPost = WatchdogConfig.WatchdogTimeoutPost;
    TempBmcConfig.WatchdogTimeoutActionPost = WatchdogConfig.WatchdogTimeoutActionPost;
    TempBmcConfig.WatchdogOS = WatchdogConfig.WatchdogOS;
    TempBmcConfig.WatchdogTimeoutOS = WatchdogConfig.WatchdogTimeoutOS;
    TempBmcConfig.WatchdogTimeoutActionOS = WatchdogConfig.WatchdogTimeoutActionOS;
  } else {
    TempBmcConfig.WatchdogPost = 2;
    TempBmcConfig.WatchdogTimeoutPost = 600;
    TempBmcConfig.WatchdogTimeoutActionPost = 1;
    TempBmcConfig.WatchdogOS = 2;
    TempBmcConfig.WatchdogTimeoutOS = 300;
    TempBmcConfig.WatchdogTimeoutActionOS = 1;
  }

  //
  // Update uncommitted data of Browser
  //
  HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
  return Status;
}

/**
  This function processes the results of changes in configuration.

  @param[in] This                 Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.

  @retval    EFI_SUCCESS            The Results is processed successfully.
  @retval    EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval    EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.
**/
EFI_STATUS
ValidateSetupConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This
  )
{
  EFI_STATUS                              Status;
  EFI_IPv4_ADDRESS                        IpAdd;
  EFI_IPv4_ADDRESS                        SubnetMask;
  EFI_IPv4_ADDRESS                        GatewayIp;
  EFI_INPUT_KEY                           Key;
  BMC_CONFIG                              TempBmcConfig;
  IPMI_SET_POWER_RESTORE_POLICY_REQUEST   RestorePolicy;
  WATCHDOG_CONFIG                         WatchdogConfig;
  IPMI_SET_POWER_RESTORE_POLICY_RESPONSE  PowerRestorePolictResponse;
  BMC_INFO_CALLBACK_DATA                  *Private;
  CHAR16                                  StringBuffer1[32];
  CHAR16                                  StringBuffer2[64];
  CHAR16                                  StringBuffer3[32];

  ZeroMem (&TempBmcConfig, sizeof(BMC_CONFIG));
  ZeroMem (StringBuffer1, sizeof(StringBuffer1));
  ZeroMem (StringBuffer2, sizeof(StringBuffer2));
  ZeroMem (StringBuffer3, sizeof(StringBuffer3));
  Private = BMC_INFO_CALLBACK_DATA_FROM_THIS(This);
  DEBUG ((DEBUG_INFO, "BMC Cfg route config begin\n"));
  //
  // Retrive uncommitted data from Browser
  //
  if (!HiiGetBrowserData (&mBmcInfoGuid, BMC_CONFIG_STORE, sizeof (BMC_CONFIG), (UINT8 *) &TempBmcConfig)) {
    return EFI_NOT_FOUND;
  }
  Status = EFI_SUCCESS;
  Status = StrStringToIp4Adr (&IpAdd, TempBmcConfig.IpAddress);
  //ip invalid ,default setup
  if (Status == EFI_INVALID_PARAMETER) {
    do {
        StrCpyS (StringBuffer1, StrLen (L"Invalid IP Address.") + 1, L"Invalid IP Address.");
        StrCpyS (StringBuffer2, StrLen (L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct IP address in form of xxx.xxx.xxx.xxx");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     StrCpyS (TempBmcConfig.IpAddress, StrLen (Private->BmcConfig.IpAddress) + 1,Private->BmcConfig.IpAddress);
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
  } else {
    StrCpyS (Private->BmcConfig.IpAddress, StrLen (TempBmcConfig.IpAddress) + 1,TempBmcConfig.IpAddress);
    Status = BmcSetLanConfig (&TempBmcConfig, BASE_BOARD_NIC_CHANNEL_SEL);
  }

  Status = StrStringToIp4Adr (&SubnetMask, TempBmcConfig.SubnetMask);
  //SubnetMask invalid ,default setup
  if (Status == EFI_INVALID_PARAMETER) {
    do {
        StrCpyS (StringBuffer1, StrLen (L"Invalid SubnetMask.") + 1, L"Invalid SubnetMask.");
        StrCpyS (StringBuffer2, StrLen (L"Please Config Correct SubnetMask in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct SubnetMask address in form of xxx.xxx.xxx.xxx");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     StrCpyS (TempBmcConfig.SubnetMask, StrLen (Private->BmcConfig.SubnetMask) + 1,Private->BmcConfig.SubnetMask);
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
  } else {
    StrCpyS (Private->BmcConfig.SubnetMask, StrLen (TempBmcConfig.SubnetMask) + 1,TempBmcConfig.SubnetMask);
    Status = BmcSetLanConfig (&TempBmcConfig, BASE_BOARD_NIC_CHANNEL_SEL);
  }

  Status = StrStringToIp4Adr (&GatewayIp, TempBmcConfig.GatewayIp);
  //GatewayIp invalid ,default setup
  if (Status == EFI_INVALID_PARAMETER) {
    do {
        StrCpyS (StringBuffer1, StrLen (L"Invalid GatewayIp.") + 1, L"Invalid GatewayIp.");
        StrCpyS (StringBuffer2, StrLen (L"Please Config Correct GatewayIp in form of xxx.xxx.xxx.xxx") + 1, L"Please Config Correct GatewayIp address in form of xxx.xxx.xxx.xxx");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     StrCpyS (TempBmcConfig.GatewayIp, StrLen (Private->BmcConfig.GatewayIp) + 1,Private->BmcConfig.GatewayIp);
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
  } else {
    StrCpyS (Private->BmcConfig.GatewayIp, StrLen (TempBmcConfig.GatewayIp) + 1,TempBmcConfig.GatewayIp);
    Status = BmcSetLanConfig (&TempBmcConfig, BASE_BOARD_NIC_CHANNEL_SEL);
  }

  if (TempBmcConfig.RestorePolicy == POWER_UP) {
    RestorePolicy.PowerRestorePolicy.Uint8 = 2;
    Status = IpmiSetPowerPolicy (&RestorePolicy, &PowerRestorePolictResponse);
    if (EFI_ERROR(Status)) {
      do {
        StrCpyS (StringBuffer1, StrLen (L"ERR:BMC do not support.") + 1, L"ERR:BMC do not support.");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     TempBmcConfig.RestorePolicy = POWER_UP;
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
    }
  } else if (TempBmcConfig.RestorePolicy == POWER_RESTORE) {
    RestorePolicy.PowerRestorePolicy.Uint8 = 1;
    Status = IpmiSetPowerPolicy (&RestorePolicy, &PowerRestorePolictResponse);
    if (EFI_ERROR(Status)) {
      do {
        StrCpyS (StringBuffer1, StrLen (L"ERR:BMC do not support.") + 1, L"ERR:BMC do not support.");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     TempBmcConfig.RestorePolicy = POWER_RESTORE;
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
    }
  } else if (TempBmcConfig.RestorePolicy == POWER_OFF) {
    RestorePolicy.PowerRestorePolicy.Uint8 = 0;
    Status = IpmiSetPowerPolicy (&RestorePolicy, &PowerRestorePolictResponse);
    if (EFI_ERROR(Status)) {
      do {
        StrCpyS (StringBuffer1, StrLen (L"ERR:BMC do not support.") + 1, L"ERR:BMC do not support.");
        StrCpyS (StringBuffer3, StrLen (L"Press ENTER to continue") + 1, L"Press ENTER to continue");
        CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, StringBuffer1, StringBuffer2, StringBuffer3, NULL);
     } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
     TempBmcConfig.RestorePolicy = POWER_OFF;
     HiiSetBrowserData (
      &mBmcInfoGuid,
      BMC_CONFIG_STORE,
      sizeof (BMC_CONFIG),
      (UINT8 *) &TempBmcConfig,
      NULL
      );
    }
  }

  WatchdogConfig.WatchdogPost = TempBmcConfig.WatchdogPost;
  WatchdogConfig.WatchdogTimeoutPost = TempBmcConfig.WatchdogTimeoutPost;
  WatchdogConfig.WatchdogTimeoutActionPost = TempBmcConfig.WatchdogTimeoutActionPost;
  WatchdogConfig.WatchdogOS = TempBmcConfig.WatchdogOS;
  WatchdogConfig.WatchdogTimeoutOS = TempBmcConfig.WatchdogTimeoutOS;
  WatchdogConfig.WatchdogTimeoutActionOS = TempBmcConfig.WatchdogTimeoutActionOS;

  Status = gRT->SetVariable (
    WATCHDOG_CONFIG_VAR,
    &gWatchdogGuid,
    PLATFORM_SETUP_VARIABLE_FLAG,
    sizeof (WATCHDOG_CONFIG),
    &WatchdogConfig
    );

  return Status;
}

