/** @file
The header file of advanced form guid defines.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  ADVANCEDCONFIG_FORM_GUID_H_
#define  ADVANCEDCONFIG_FORM_GUID_H_

#define ADVANCEDCONFIG_FORMSET_GUID\
  { \
  0xf25bd188, 0x0b8b, 0x11ec, {0xb2, 0x95, 0x33, 0x31, 0x22, 0x45, 0xad, 0x18} \
  }

#define  ADVANCED_CONFIG_VARIABLE            L"AdvancedConfigSetup"
#define  FORM_ADVANCEDCONFIG_ID              0x6000
#define  FORM_X100CONFIG_ID                  0x6001
#define  FORM_PCIEDEVICEINFO_ID              0x6002
#define  FORM_USBDEVICEINFO_ID               0x6003
#define  FORM_HDDINFO_ID                     0x6004

#define  FORM_ADVANCEDCONFIG_OPEN            0x6f00
#define  FORM_X100CONFIG_OPEN                0x6f01
#define  FORM_PCIEDEVICEINFO_OPEN            0x6f02
#define  FORM_USBDEVICEINFO_OPEN             0x6f03
#define  FORM_HDDINFO_OPEN                   0x6f04

#define  VARSTORE_ID_ADVANCED_CONFIG         0x6100
#define  ADVANCED_KEY_OFFSET                 0x6200

#define  LABEL_FORM_PCIEDEVICEINFO_START     0xff01
#define  LABEL_FORM_PCIEDEVICEINFO_END       0xff02
#define  LABEL_FORM_USBCONTROLLERINFO_START  0xff03
#define  LABEL_FORM_USBCONTROLLERINFO_END    0xff04
#define  LABEL_FORM_USBDEVICEINFO_START      0xff05
#define  LABEL_FORM_USBDEVICEINFO_END        0xff06
#define  LABEL_FORM_HDDDEVICEINFO_START      0xff07
#define  LABEL_FORM_HDDDEVICEINFO_END        0xff08
#define  LABEL_FORM_ADVANCEDCONFIG_START     0xff0c
#define  LABEL_FORM_ADVANCEDCONFIG_END       0xff0d
#define  LABEL_END                           0xfe12

#endif
