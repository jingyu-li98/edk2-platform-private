/** @file

Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
All intellectual property rights(Copyright, Patent and Trademark) reserved.

Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _SECURITY_NV_H_
#define _SECURITY_NV_H_

#define SECURITY_PAGE_FORM_ID          0x1000
#define FORM_SECURITY_OPEN             0x1f00
#define LABEL_SECURITYPAGE_INFORMATION 0x1001
#define LABEL_END                      0xffff

#define SECURITY_PAGE_FORMSET_GUID { \
   0x4d600d05, 0xb147, 0x93a1, { 0xc6, 0x47, 0xca, 0x7f, 0xac, 0x9e, 0x94, 0xc3 } \
}

#endif
