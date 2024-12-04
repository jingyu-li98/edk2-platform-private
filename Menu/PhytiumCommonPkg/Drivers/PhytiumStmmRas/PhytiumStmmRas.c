/** @file
Provides Phytium stmm ras handle service.

Copyright (C) 2022 - 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/MmSpec.h>

#include "Ras.h"

APEI_TABLE_LIST    mApeiList;

/**
  Ras event handle. The source buffer data comes from the content of the MM
  communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
  Phytium MM Interface Specification.

  @param[in]     DispatchHandle  The unique handle assigned to this handler by
                                 MmiHandlerRegister().
  @param[in]     RegisterContext Points to an optional handler context which was
                                 specified when the handler was registered.
  @param[in, out] CommBuffer     A pointer to a collection of data in memory that
                                 will be conveyed from a non-MM environment into
                                 an MM environment.
  @param[in, out] CommBufferSize The size of the CommBuffer.

  @retval       EFI_SUCCESS    Success.
  @retval       Other          Failed.
**/
EFI_STATUS
EFIAPI
PhytiumStmmRasHandler (
  IN     EFI_HANDLE   DispatchHandle,
  IN     CONST VOID   *RegisterContext,
  IN OUT VOID         *CommBuffer,
  IN OUT UINTN        *CommBufferSize
  )
{
  UINT32                  Status;
  UINT8                   *ContentBuffer;
  MM_COMM_CONTENT_HEADER  Header;

  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "%a begin\n", __FUNCTION__));
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
  DEBUG ((DEBUG_INFO, "%a, end Status : %r\n", __FUNCTION__, Status));
  return Status;
}

/**
  Ras strategy initialization.

  @param[in, out]  Strategy    A poniter to RAS_STRATEGY.
**/
STATIC
VOID
RasStrategyInit (
  IN OUT RAS_STRATEGY  *Strategy
  )
{
  ZeroMem (Strategy, sizeof (RAS_STRATEGY));
  Strategy->BmcCperEnable = FixedPcdGetBool (PcdRasBmcCperEnable);
  Strategy->FatalErrorRecord = FixedPcdGet8 (PcdRasFatalErrorRecord);
  Strategy->MemoryIsoRecord = FixedPcdGet8 (PcdRasMemoryIsoRecord);
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
  Clear rat total including memory CE count, CPU CE count and PCIe CE count.

  @param[in, out]    Total    A poniter to RAS_TOTAL_COUNT.
**/
VOID
ClearRasTotal (
  IN OUT RAS_TOTAL_COUNT  *Total
  )
{
  Total->MemCeCount = 0;
  Total->CpuCeCount = 0;
  Total->PcieCeCount = 0;
}

/**
  Phytium service MM driver entry point.

  @param[in] ImageHandle    A handle for the image that is initializing this
                            driver
  @param[in] MmSystemTable  A pointer to the MM system table

  @retval EFI_SUCCESS       Phytium stmm service successfully initialized.
**/
EFI_STATUS
EFIAPI
PhytiumStmmRasHandleInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_MM_SYSTEM_TABLE  *MmSystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  PhytiumStmmHandle;

  //
  // Register Phytium Stmm Ras Event handler
  //
  ZeroMem (&mRasTime, sizeof (RAS_TIME));
  ApeiTableInit (&mApeiList);
  ClearRasTotal (&mRasTotalCount);
  RasStrategyInit (&mRasStrategy);
  Status = gMmst->MmLocateProtocol (
                    &gSpiNorFlashProtocolGuid,
                    NULL,
                    (VOID*) &mFlash
                    );
  DEBUG ((DEBUG_INFO, "mFlash Locate Protocol : %r\n", Status));
  mFlash->Initialization ();
  Status = InitHestTable ();
  ASSERT_EFI_ERROR (Status);
  Status = InitEinjTable ();
  ASSERT_EFI_ERROR (Status);
  Status = InitBertTable ();
  ASSERT_EFI_ERROR (Status);
  PhytiumStmmHandle = NULL;
  Status = gMmst->MmiHandlerRegister (
                    PhytiumStmmRasHandler,
                    &gEfiPhytiumStmmRasProtocolGuid,
                    &PhytiumStmmHandle
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
