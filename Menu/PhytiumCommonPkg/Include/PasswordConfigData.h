
/** @file
The header file of Password configuration Data.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PASSWORD_CONFIG_DATA_H_
#define  PASSWORD_CONFIG_DATA_H

#define VAR_PASSWORD_CONFIG_NAME                 L"PasswordConfigVar"
#define PLATFORM_SETUP_VARIABLE_FLAG             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#define VAR_USR_PASSWD                           L"UsrPasswd"
#define VAR_ADMIN_PASSWD                         L"AdminPasswd"
//PASSWD_MAXLEN is AES length 64*2 =128,do not change this
#define PASSWD_MAXLEN                            64
//PASSWD_MAXLEN_INPUT is the dialog input length
#define PASSWD_MAXLEN_INPUT                      30
//PASSWD_MAXLEN_ALLOW is the allowed password max length
#define PASSWD_MAXLEN_ALLOW                      20
#define PASSWD_MINLEN                            0
#define PASSWORDCONFIG_FORMSET_GUID\
  {0x30803760, 0xdcf5, 0x11ed, {0xac, 0x93, 0xdf, 0x69, 0x7b, 0x48, 0x85, 0x6f}}
#define VARSTORE_ID_PASSWORD_CONFIG              0x9100
#define PASSWORD_PRIV                            L"PasswordPriv"


typedef struct _PASSWORD_CONFIG_DATA {
  UINT8         UserPasswordEnable;
  UINT8         AdminPasswordEnable;
  CHAR16        UserPassword[PASSWD_MAXLEN];
  CHAR16        AdminPassword[PASSWD_MAXLEN];
  UINT8         UserPriv;
} PASSWORD_CONFIG_DATA;

#endif
