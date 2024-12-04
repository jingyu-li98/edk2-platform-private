/** @file
Need Send Device Information Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef IPMIDEVICEINFO_H_
#define IPMIDEVICEINFO_H_

#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Nvme.h>
#include <IndustryStandard/Scsi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IpmiBaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>

#include <Protocol/DiskInfo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/NvmExpressPassthru.h>
#include <Protocol/Smbios.h>

#define EFI_PHY_NETFN_INFO                0x3a
#define EFI_SMBIOS_NETFN_INFO             0x2e
#define EFI_SOC_SETCPU_INFO               0x01
#define EFI_SET_MEM_INFO                  0x03
#define EFI_SET_BIOS_INFO                 0x20
#define EFI_IPMI_PCIE_INFO                0x09
#define EFI_IPMI_HDD_INFO                 0x16
#define EFI_IPMI_SMBIOS_INFO              0x80

#define VENDOR_IDENTIFICATION_OFFSET      3
#define VENDOR_IDENTIFICATION_LENGTH      8
#define PRODUCT_IDENTIFICATION_OFFSET     11
#define PRODUCT_IDENTIFICATION_LENGTH     16
#define SMBIOS_TYPE4_CPU_SOCKET_POPULATED BIT6

#pragma pack(1)

//
// IPMI Write Data to Blob.
//
typedef struct {
  UINT8   Blob[4];  // OEM defines
  UINT16  Crc16;
  UINT16  SsionId;
  UINT32  Offset;
  UINT8   Data[255];
} IPMI_WRITE_TYPE_DATA_BLOB;

//
// IPMI End Blob.
//
typedef struct {
  UINT8   OemNumber[3];
  UINT8   Command;
  UINT16  Crc16;
  UINT16  SsionId;
  UINT8   Data;
} IPMI_SOC_BMC_END_BLOB;


/**
  Send Ipmi Device Information.

**/
EFI_STATUS
IpmiDeviceInfoToBmc (
  VOID
  );

/**
  Eliminate the extra spaces in the Str to one space.

  @param    Str    Input string info.

**/
EFI_STATUS
BmEliminateExtraSpaces (
  IN CHAR16        *Str
  );

#pragma pack()
#endif
