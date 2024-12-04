/** @file
The guid define header file of HII Config Access protocol implementation of ras configuration module.

Copyright (c) 2024, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  RASCONFIG_FORM_GUID_H_
#define  RASCONFIG_FORM_GUID_H_


#define  RAS_CONFIG_STORE             L"RasConfigSetup"

#define  FORM_RAS_ID                  0x6001

#define  QUESTION_ID_MEM_FATAL_UCE_IOS_COUNT      0x6002
#define  QUESTION_ID_MEM_NONFATAL_UCE_IOS_COUNT   0x6003
#define  QUESTION_ID_MEM_CE_IOS_COUNT             0x6004
#define  QUESTION_ID_MEM_CE_REPORT_COUNT          0x6005
#define  QUESTION_ID_PCIE_CE_REPORT_COUNT         0x6006
#define  QUESTION_ID_CPU_CE_REPORT_COUNT          0x6007

#define  TRIGGER_ID                   0x6101

#define  FORM_USER_PASSWD_OPEN        0x6f05
#define  FORM_ADMIN_PASSWD_OPEN       0x6f06

#define  LABEL_END                    0xffff

#endif
