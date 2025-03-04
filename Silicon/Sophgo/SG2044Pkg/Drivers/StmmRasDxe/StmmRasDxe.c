/** @file
  SOPHGO STMM RAS Handler implementation.

  This file implements the RAS (Reliability, Availability and Serviceability)
  handler for SOPHGO STMM, supporting PCIe AER and DDR inline ECC error handling.

  Copyright (C) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/StandaloneMmLib.h>
#include "Ras.h"

//
// Global variables
//
STATIC EFI_HANDLE  mRasHandle = NULL;
STATIC RAS_STRATEGY mRasStrategy;

/**
  STMM RAS handler entry point.

  This function handles RAS events in STMM environment, including:
  - PCIe AER error handling
  - DDR inline ECC error handling
  - Error record generation and management

  @param[in]     DispatchHandle  The unique handle assigned to this handler by MmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                be conveyed from a non-MM environment into an MM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS           The interrupt was handled successfully.
  @retval EFI_INVALID_PARAMETER CommBuffer or CommBufferSize is null.
**/
EFI_STATUS
EFIAPI
SophgoStmmRasHandler (
  IN     EFI_HANDLE  DispatchHandle,
  IN     CONST VOID  *Context,
  IN OUT VOID        *CommBuffer,
  IN OUT UINTN       *CommBufferSize
  )
{
  UINT32                  Status;
  UINT8                   *ContentBuffer;
  MM_COMM_CONTENT_HEADER  Header;

  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "%a begin\n", __func__));
  Status = PHYTIUM_STMM_HANDLER_SUCCESS;
  if (*CommBufferSize < 8) {
    Status = PHYTIUM_STMM_HANDLER_HEADER_ERROR;
    goto ProcExit;
  }
  CopyMem (&Header, (UINT8*) CommBuffer, sizeof (MM_COMM_CONTENT_HEADER));
  Status = gMmst->MmAllocatePool (
                    EfiRuntimeServicesData,
                    Header.Length,
                    (VOID **)&ContentBuffer
                    );
  ASSERT_EFI_ERROR (Status);
  CopyMem (ContentBuffer, CommBuffer + 8, Header.Length);
  switch (Header.MainId) {
  case RasFunction:
    Status = RasMainFunctionHandle (Header.SubId, ContentBuffer, &Header.Length);
    break;
  default:
    Status = PHYTIUM_STMM_HANDLER_FUNC_ID_UNSUPPORTED;
    break;
  }
  CopyMem ((UINT8 *)CommBuffer, &Header, sizeof (MM_COMM_CONTENT_HEADER));
  CopyMem ((UINT8 *)CommBuffer + 8, ContentBuffer, Header.Length);
  *CommBufferSize = Header.Length + 8;

ProcExit:
  if (ContentBuffer != NULL) {
    gMmst->MmFreePool (ContentBuffer);
  }
  DEBUG ((DEBUG_INFO, "%a, end Status : %r\n", __func__, Status));
  return Status;
}

/**
  Initialize RAS strategy configuration.

  This function initializes the RAS strategy structure with configuration values
  from PCDs, including:
  - Error handling policies for PCIe AER
  - Error handling policies for DDR ECC
  - Error reporting thresholds
  - Isolation strategies

  @param[in,out]  Strategy    Pointer to RAS_STRATEGY structure to initialize.
**/
STATIC
VOID
RasStrategyInit (
  IN OUT RAS_STRATEGY  *Strategy
  )
{
  if (Strategy == NULL) {
    return;
  }

  ZeroMem (Strategy, sizeof (RAS_STRATEGY));
  
  //
  // Initialize BMC CPER settings
  //
  Strategy->BmcCperEnable = FixedPcdGetBool (PcdRasBmcCperEnable);
  Strategy->FatalErrorRecord = FixedPcdGet8 (PcdRasFatalErrorRecord);
  Strategy->MemoryIsoRecord = FixedPcdGet8 (PcdRasMemoryIsoRecord);

  //
  // Initialize Memory Fatal UCE isolation strategy
  //
  Strategy->MemFatalUceIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemFatalUceIsoStrategyEnable);
  Strategy->MemFatalUceIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemFatalUceIsoStrategyResetClean);
  Strategy->MemFatalUceIsoStrategy.Method = FixedPcdGet8 (PcdRasMemFatalUceIsoStrategyMethod);
  Strategy->MemFatalUceIsoStrategy.Count = FixedPcdGet32 (PcdRasMemFatalUceIsoStrategyCount);

  Strategy->MemNonFatalUceIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemNonFatalUceIsoStrategyEnable);
  Strategy->MemNonFatalUceIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemNonFatalUceIsoStrategyResetClean);
  Strategy->MemNonFatalUceIsoStrategy.Method = FixedPcdGet8 (PcdRasMemNonFatalUceIsoStrategyMethod);
  Strategy->MemNonFatalUceIsoStrategy.Count = FixedPcdGet32 (PcdRasMemNonFatalUceIsoStrategyCount);
  Strategy->MemCeIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemCeIsoStrategyEnable);
  Strategy->MemCeIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemCeIsoStrategyResetClean);
  Strategy->MemCeIsoStrategy.Method = FixedPcdGet8 (PcdRasMemCeIsoStrategyMethod);
  Strategy->MemCeIsoStrategy.Count = FixedPcdGet32 (PcdRasMemCeIsoStrategyCount);
  Strategy->CpuFatalStrategy.Enable = FixedPcdGetBool (PcdRasCpuFatalIsoStrategyEnable);
  Strategy->CpuFatalStrategy.ResetClean = FixedPcdGetBool (PcdRasCpuFatalIsoStrategyResetClean);
  Strategy->CpuFatalStrategy.Method = FixedPcdGet8 (PcdRasCpuFatalIsoStrategyMethod);
  Strategy->CpuFatalStrategy.Record = FixedPcdGet8 (PcdRasCpuFatalIsoStrategyRecord);
  Strategy->MemCeReportStrategy.Enable = FixedPcdGetBool (PcdRasMemCeReportStrategyEnable);
  Strategy->MemCeReportStrategy.Count = FixedPcdGet32 (PcdRasMemCeReportStrategyCount);
  Strategy->CpuCeReportStrategy.Enable = FixedPcdGetBool (PcdRasCpuCeReportStrategyEnable);
  Strategy->CpuCeReportStrategy.Count = FixedPcdGet32 (PcdRasCpuCeReportStrategyCount);
  Strategy->PcieCeReportStrategy.Enable = FixedPcdGetBool (PcdRasPcieCeReportStrategyEnable);
  Strategy->PcieCeReportStrategy.Count = FixedPcdGet32 (PcdRasPcieCeReportStrategyCount);
  Strategy->MemAddrIsoStrategy.Enable = FixedPcdGetBool (PcdRasMemoryAddressIsolationStrategyEnable);
  Strategy->MemAddrIsoStrategy.ResetClean = FixedPcdGetBool (PcdRasMemoryAddressIsolationStrategyResetClean);
  Strategy->MemAddrIsoStrategy.Type = FixedPcdGet8 (PcdRasMemoryAddressIsolationStrategyType);
}

/**
  Clear RAS total count statistics.

  This function resets all RAS error counters to zero, including:
  - Memory error counts (CE/UCE)
  - PCIe error counts
  - CPU error counts

  @param[in,out]  Total    Pointer to RAS_TOTAL_COUNT structure to clear.
**/
VOID
ClearRasTotal (
  IN OUT RAS_TOTAL_COUNT  *Total
  )
{
  if (Total == NULL) {
    return;
  }

  ZeroMem (Total, sizeof (RAS_TOTAL_COUNT));
}

/**
  Initialize STMM RAS handler.

  This function performs the following initialization:
  - Registers RAS event handler
  - Initializes RAS strategy configuration
  - Initializes error counters
  - Sets up error handling tables (HEST/BERT)

  @param[in]  ImageHandle   Image handle of this driver
  @param[in]  MmSystemTable MM system table

  @retval EFI_SUCCESS           Initialization completed successfully
  @retval EFI_ALREADY_STARTED  The protocol has already been installed
  @retval Others               Initialization failed
**/
EFI_STATUS
EFIAPI
SophgoStmmRasHandleInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_MM_SYSTEM_TABLE  *MmSystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  StmmHandle;

  DEBUG ((DEBUG_INFO, "SophgoStmmRasHandleInitialize Entry.\n"));

  //
  // Initialize RAS strategy
  //
  RasStrategyInit (&mRasStrategy);

  //
  // Clear error counters
  //
  ClearRasTotal (&mRasTotalCount);

  //
  // Initialize APEI tables
  //
  Status = InitHestTable ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "InitHestTable failed - %r\n", Status));
    return Status;
  }

  //
  // Register RAS handler
  //
  StmmHandle = NULL;
  Status = gMmst->MmiHandlerRegister (
                    SophgoStmmRasHandler,
                    &gEfiSophgoStmmRasProtocolGuid,
                    &StmmHandle
                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "MmiHandlerRegister failed - %r\n", Status));
    return Status;
  }

  mRasHandle = StmmHandle;
  DEBUG ((DEBUG_INFO, "SophgoStmmRasHandleInitialize Exit - %r\n", Status));

  return Status;
}
