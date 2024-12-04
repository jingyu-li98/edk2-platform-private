/** @file
This file contains the definination for GHES.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef  PHTIUM_GHES_H_
#define  PHTIUM_GHES_H_

#include <IndustryStandard/Acpi.h>

#include <Guid/Cper.h>

#include <Library/ArmMmuLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/MmSpec.h>
#include <Library/PcdLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiLib.h>

#include <Uefi.h>

#define  READ_ACK_PRESERVE                  0xFFFFFFFFFFFFFFFE
#define  READ_ACK_WRITE                     0x1
#define  MAX_GHES                           6
#define  GENERIC_HARDWARE_ERROR_BLOCK_SIZE  0X1000
#define  HEST_TABLE_SIZE                    0x2000

//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_ARM_OEM_ID           'P','H','Y','L','T','D'
#define EFI_ACPI_ARM_OEM_TABLE_ID     SIGNATURE_64('P','H','Y','T','I','U','M','.')
#define EFI_ACPI_ARM_OEM_REVISION     0x1
#define EFI_ACPI_ARM_CREATOR_ID       SIGNATURE_32('P','H','Y','T')
#define EFI_ACPI_ARM_CREATOR_REVISION 0x1

//
// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
//
#define ARM_ACPI_HEADER(Signature, Type, Revision) {              \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    { EFI_ACPI_ARM_OEM_ID },        /* UINT8   OemId[6] */        \
    EFI_ACPI_ARM_OEM_TABLE_ID,      /* UINT64  OemTableId */      \
    EFI_ACPI_ARM_OEM_REVISION,      /* UINT32  OemRevision */     \
    EFI_ACPI_ARM_CREATOR_ID,        /* UINT32  CreatorId */       \
    EFI_ACPI_ARM_CREATOR_REVISION   /* UINT32  CreatorRevision */ \
  }

typedef struct _HEST_CONTEXT  HEST_CONTEXT;

struct _HEST_CONTEXT {
  EFI_ACPI_6_4_HARDWARE_ERROR_SOURCE_TABLE_HEADER  *HestHeader;
  UINT32                                           OccupiedMemorySize;
};

typedef struct _RAS_TIME {
  EFI_TIME  Time;
  UINT64    Utc;
  UINT64    Tick;
} RAS_TIME;

extern HEST_CONTEXT                 mHestContext;
extern RAS_TIME                     mRasTime;
extern SYNC_MEMORY_DEVICE_LOCATION  mMemDeviceLocation;
extern RAS_STRATEGY                 mRasStrategy;
/**
  Binary conversion to bcd code.

  @param[in]  Val  Binary code to convert.

  @retval     BCD code.
**/
UINT8
BinToBcd (
  IN UINT8  Val
  );

/**
  Print EFI_TIME.

  @param[in]  Time  A pointer to EFI_TIME.
**/
VOID
PrintEfiTime (
  IN EFI_TIME  *Time
  );

/**
  Error block content initialization. Specify the severity of this block.

  @param[in]  Severity  Severity of this block.
                        0 - Recoverable.
                        1 - Fatal.
                        2 - Corrected.
                        3 - Information.
  @param[in]  Block     A pointer to error block.
  @param[in]  Severity  Severity of this block.

  @retval      A pointer to error block.
**/
EFI_ACPI_6_4_GENERIC_ERROR_STATUS_STRUCTURE*
ErrorBlockInitial (
  IN  VOID   *Block,
  IN  UINT32 Severity
  );

/**
  Update the error block header content. If the current ErrorDataEntryCount is
  0, the error block is reset. If the current ErrorDataEntryCount it not 0, it
  means that the error data is the content of the same error block, and the
  error block header is not processed.

  @param[in,out]    Block     A pointer to error block.
  @param[in]        Severity  Severity of this block.

  @retval    TRUE    Update successfully.
  @retval    FALSE   Update failed.
**/
BOOLEAN
ErrorBlockUpdateStatusStructure (
  IN OUT VOID *ErrorBlock,
  IN     UINT32  Severity
  );

/**
  Add new error data content to errro block. The data to add is from CPER.

  @param[in]    ErrorBlock              A pointer to error block.
  @param[in]    MaxBlockLength          The maximum length allowed for error data.
  @param[in]    Header                  A pointer to EFI_COMMON_ERROR_RECORD_HEADER.
  @param[in]    Descriptor              A pointer to EFI_ERROR_SECTION_DESCRIPTOR.
  @param[in]    GenericErrorData        Generic error data to add.
  @param[in]    SizeOfGenericErrorData  The length of generic error data.
  @param[in]    Correctable             1 - Correctable
                                        0 - Uncorrectable

  @retval       TRUE    Success.
  @retval       FALSE   Failed.
 **/
BOOLEAN
ErrorBlockAddErrorData (
  IN VOID                            *ErrorBlock,
  IN UINT32                          MaxBlockLength,
  IN EFI_COMMON_ERROR_RECORD_HEADER  *Header,
  IN EFI_ERROR_SECTION_DESCRIPTOR    *Descriptor,
  IN VOID                            *GenericErrorData,
  IN UINT32                          SizeOfGenericErrorData,
  IN BOOLEAN                         Correctable
  );

/**
  Create HEST table header.

  @param[in, out]  Context               A pointer of HEST_CONTEXT.
  @param[in]       PreAllocatedHestSize  HEST table pre-allocate space size.

  @retval    EFI_SUCCESS             Success.
  @retval    EFI_BUFFER_TOO_SMALL    PreAllocatedHestSize is too small.
**/
EFI_STATUS
HestHeaderCreator (
  IN OUT HEST_CONTEXT  *Context,
  IN     UINT32        PreAllocatedHestSize
  );

/**
  Add error source descriptor to HEST table.

  @param[in, out]  Context                A pointer to HEST_CONTEXT.
  @param[in]       ErrorSourceDescriptor  A pointer to GHES struct.
  @param[in]       SizeOfDescriptor       Size of GHES.

  @retval          EFI_SUCCESS            Success.
  @retval          EFI_INVALID_PARAMETER  Context or ErrorSourceDescriptor is NULL.
**/
EFI_STATUS
HestAddErrorSourceDescriptor (
  IN OUT HEST_CONTEXT  *Context,
  IN VOID              *ErrorSourceDescriptor,
  IN UINT32            SizeOfDescriptor
  );

/**
  Fill all GHES tables in the HEST table.

  @param[in]  Ghes[MAX_GHES]    A point to GhesV2 tables.
  @param[in]  NumOfGhesV2       Number of GHES tables.

  @retval     EFI_SUCCESS            Success.
  @retval     EFI_INVALID_PARAMETER  GHES it NULL or
                                     NumofGhesV2 is not equal to MAX_GHES.
**/
EFI_STATUS
GhesV2ContextForHest (
  IN EFI_ACPI_6_4_GENERIC_HARDWARE_ERROR_SOURCE_VERSION_2_STRUCTURE GhesV2[MAX_GHES],
  IN UINT8                                                          NumOfGhesV2
  );

#endif
