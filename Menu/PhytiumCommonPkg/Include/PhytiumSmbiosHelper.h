/** @file
Phytium smbios helper header file.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PHYTIUM_SMBIOS_HELEPER_H_
#define PHYTIUM_SMBIOS_HELEPER_H_

#include <Uefi/UefiBaseType.h>
#include <Protocol/Smbios.h>

#define EFI_SMBIOS_TYPE_PHYTIUM_OEM_INFORMATION   SMBIOS_TYPE_PHYTIUM_OEM_INFORMATION
#define SMBIOS_TYPE_PHYTIUM_OEM_INFORMATION       128

#define MAX_MEMORY_DEVICES_COUNT 32

#pragma pack(1)
//Phytium Smbios Cpu Info
typedef struct {
  UINTN  CpuTypeId;
  CHAR8  *CpuTypeName;
  UINT8  CpuVoltage;
  UINT16 MaxSpeed;
} SmbiosCpuInfo;

typedef struct {
  UINT8    Size;         //GB
  UINT8    TotalWidth;
  UINT8    DataWidth;
  UINT8    DimmType;
  UINT8    MemoryType;
  UINT16   MaxSpeed;
  UINT16   Manufacturer;
  UINT8    DeviceLocator;
  UINT32   SerialNumber;
  CHAR8    PartNumber[31];
  UINT8    RankNumber;
  UINT16   ConfiguredMemoryClockSpeed;
  UINT8    MemoryTechnology;
  UINT16   ModuleManufacturerID;
} MEMORY_DEVICE_INFO;

typedef struct {
  UINT8               MaxDimmCount;
  UINT8               EccType;
  MEMORY_DEVICE_INFO  MemoryDevice[MAX_MEMORY_DEVICES_COUNT];
} PHYTIUM_MEMORY_SMBIOS_INFO;

typedef struct {
  UINT16  ManufacturerID;
  CHAR8  *Name;
} SPD_JEDEC_MANUFACTURER;

///
/// Phytium OEM Information (Type 128).
///
typedef struct {
  SMBIOS_STRUCTURE             Hdr;
  UINT16                       PBFMajorVersion;
  UINT16                       PBFMinorVersion;
  SMBIOS_TABLE_STRING          PBFReleaseDate;
} SMBIOS_TABLE_TYPE128;

#pragma pack()

#endif

