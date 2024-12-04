/** @file
  POST Code interface.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef POST_CODE_LIB_H_
#define POST_CODE_LIB_H_

#define POST_CODE_KCS_DEBUG     0x1
#define POST_CODE_SERIAL_DEBUG  0x2

#define POST_CODE_POWER_ON                     0x1
#define POST_CODE_PLL_INIT_START               0x2
#define POST_CODE_PLL_INIT_COMPLETE            0x3
#define POST_CODE_PLL_INIT_FAIL                0x20

#define POST_CODE_MEM_INIT_SPD_READ            0x30
#define POST_CODE_MEM_INIT_DETECTION           0x31
#define POST_CODE_MEM_INIT_PARA_T_GET          0x32
#define POST_CODE_MEM_INIT_TRIAN_START         0x33
#define POST_CODE_MEM_INIT_TRIAN_COMPLETE      0x34
#define POST_CODE_MEM_INIT_SPD_READ_FAILED     0x40
#define POST_CODE_MEM_INIT_DETECTION_NONE      0x41
#define POST_CODE_MEM_INIT_PARA_T_GET_FAILED   0x42
#define POST_CODE_MEM_INIT_TRIAN_FAILED        0x43

#define POST_CODE_PCIE_INIT_PARA_T_GET         0x50
#define POST_CODE_PCIE_INIT_START              0x51
#define POST_CODE_PCIE_INIT_COMPLETE           0x52
#define POST_CODE_PCIE_INIT_PARA_T_GET_FAILED  0x60
#define POST_CODE_PCIE_INIT_FAILED             0x61

#define POST_CODE_C2C_INIT_PARA_T_GET          0x70
#define POST_CODE_C2C_INIT_START               0x71
#define POST_CODE_C2C_INIT_COMPLETE            0x72
#define POST_CODE_C2C_INIT_PARA_T_GET_FAILED   0x80
#define POST_CODE_C2C_INIT_FAILED              0x81

#define POST_CODE_CRYPTO_VERIFY_START          0x90
#define POST_CODE_CRYPTO_VERIFY_COMPLETE       0x91
#define POST_CODE_CRYPTO_VERIFY_SUCCESS        0x92
#define POST_CODE_CRYPTO_HASH_START            0x93
#define POST_CODE_CRYPTO_HASH_COMPLETE         0x94
#define POST_CODE_CRYPTO_HASH_COMPLETE_SUCCESS 0x95
#define POST_CODE_CRYPTO_VERIFY_FAILED         0xa0
#define POST_CODE_CRYPTO_HASH_COMPARE_FAILED   0xa1

#define POST_CODE_SEC_START                    0xb0
#define POST_CODE_PET_START                    0xb1
#define POST_CODE_DXE_START                    0xb2
#define POST_CODE_BDS_START                    0xb3
#define POST_CODE_EXITBS                       0xb4

typedef struct {
  UINT8  PostCode;
  CHAR8 *Description;
} POST_CODE_MESSAGE;

VOID
PostCode(
  UINT8  PoseCode,
  UINT8  InternalNumber,
  UINT8  SocketNumber
  );


#endif /* POST_CODE_LIB_H_ */
