/** @file
Update Bios by net work headerfile.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef BIOS_UPDATE_H_
#define BIOS_UPDATE_H_
#include <Guid/GlobalVariable.h>
#include <Library/ArmPlatformLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/NetLib.h>
#include <Library/PcdLib.h>
#include <Library/PhytiumPowerControlLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/Cpu.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/Mtftp4.h>
#include <Protocol/ServiceBinding.h>
#include <Protocol/SpiNorFlashProtocol.h>
#include <Uefi.h>

#define IP4_CONFIG2_INTERFACE_INFO_NAME_LENGTH   32
#define UPDATEBIOS_PROGRESS_SLIDER_STEPS         ((sizeof (mUpdateBiosProgressFrame) / sizeof (CHAR16)) - 3)
#define UPDATEBIOS_PROGRESS_MESSAGE_SIZE         ((sizeof (mUpdateBiosProgressFrame) / sizeof (CHAR16)) + 12)

#define MTFTP_DEFAULT_BLKSIZE                    512
#define MTFTP_MIN_BLKSIZE                        8
#define MTFTP_MAX_BLKSIZE                        65464
#define MTFTP_DEFAULT_WINDOWSIZE                 1
#define MTFTP_MIN_WINDOWSIZE                     1
#define MTFTP_MAX_WINDOWSIZE                     64
#define EFI_BIOS_HEAD_SIGNATURE                  SIGNATURE_32 ('B', 'I', 'O', 'S')

typedef struct {
  UINTN  FileSize;
  UINTN  DownloadedNbOfBytes;
  UINTN  LastReportedNbOfBytes;
} DOWNLOAD_CONTEXT;


#endif
