/** @file
Header file of bmc command and page initialization function.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef   SETUP_BMCCONFIG_H_
#define   SETUP_BMCCONFIG_H_

#include <BmcInfo.h>
#include <IpmiPhyStandard.h>
#include <SetupData.h>

#include <IndustryStandard/Ipmi.h>

#include <Library/DebugLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>

#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>

// Channel Definitions
// #define BASE_BOARD_NIC_CHANNEL_SEL    0x02
#define BASE_BOARD_NIC_CHANNEL_SEL    0x01
#define RMM_NIC_CHANNEL_SEL           0x03

// Lan Net function
#define EFI_SM_NETFN_LAN              0x0C

// Set Lan configuration command
#define EFI_SET_LAN_CONFIG_CMD        0x01

// Get Lan configuration command
#define EFI_GET_LAN_CONFIG_CMD        0x02

// Definitions for Lan configuration Parameters
#define IP_ADDRESS_PARM               0x03
#define IP_SOURCE_PARM                0x04
#define SUBNET_MASK_PARM              0x06
#define GATEWAY_IP_PARM               0x0c
#define DHCP_NAME_PARM                0xC7

// Definitions for Lan configuration Parameters  for IPV6
//
#define IPV6_STATUS_PARM              0x33
#define IPV6_ADDRESS_PARM             0x38
#define GATEWAY_IPV6_CONFIG_PARM      0x40
#define GATEWAY_IPV6_PARM             0x41
#define DHCP_IPV6_NAME_PARM           0xC7
//User configuration Commands
#define GET_USER_NAME_CMD             0x46
#define GET_USER_ACCESS_CMD           0x44
#define SET_USER_NAME_CMD             0x45
#define SET_USER_ACCESS_CMD           0x43
#define SET_USER_PASSWORD_CMD         0x47

/**
  Get Chassis Status command:
  NetFunction Id is 0x00, command Id is 0x01.

  @param[out]  ChassisStatus  A pointer to BMC Chassis Status information.

  @retval      EFI_SUCCESS       Command completed successfully.
               EFI_DEVICE_ERROR  IPMI command failed.
               EFI_UNSUPPORTED   Command is not supported by BMC.
               EFI_BUFFER_TOO_SMALL   Response buffer is too small.
               EFI_INVALID_PARAMETER  RecvData or RecvLength is NULL.
**/
EFI_STATUS
EFIAPI
IpmiGetChassisStatus (
  OUT IPMI_GET_CHASSIS_STATUS_RESPONSE  *ChassisStatus
  );

/**
  Validate user strings entered.

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
  );

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
);

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
ProcessPasswordSet (
  IN  BMC_INFO_CALLBACK_DATA  *Private,
  IN  EFI_QUESTION_ID         QuestionId,
  IN  EFI_IFR_TYPE_VALUE      *Value
  );

#endif
