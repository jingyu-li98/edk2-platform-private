/** @file
  TPCM Security Measurement Handler implementation.

  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/HashLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Include/Tpcm.h>
#include "TpcmImageVerify.h"

#define SM3_DIGEST_SIZE  32

STATIC SOPHGO_TPCM_PROTOCOL       *mTpcmProtocol;
STATIC TPCM_STATUS                *TpcmInfo;
STATIC TPCM_IMAGE_VERIFY_PROTOCOL *mTpcmVerifyProtocol;

/**
  Verify and measure loaded image using TPCM.

  @param[in]  This             Pointer to TPCM_IMAGE_VERIFY_PROTOCOL
  @param[in]  MeasureStage     Measurement stage (LOAD/EXECUTE)
  @param[in]  ImageInfo        Pointer to image information
  @param[in]  ImageInfoSize    Size of image information
  @param[in]  NumAddrRange     Number of address ranges
  @param[in]  Ranges           Array of address ranges
  @param[out] MeasureResult    Measurement result
  @param[out] ControlResult    Control policy result

  @retval EFI_SUCCESS          Measurement completed
  @retval EFI_ACCESS_DENIED    Measurement failed
**/
EFI_STATUS
EFIAPI
TpcmVerifyImageRaw (
  IN  TPCM_IMAGE_VERIFY_PROTOCOL *This,
  IN  UINT32                     MeasureStage,
  IN  VOID                       *ImageInfo,
  IN  UINT32                     ImageInfoSize,
  IN  UINT32                     NumAddrRange,
  IN  IMAGE_ADDR_RANGE           Ranges[],
  OUT UINT32                     *MeasureResult,
  OUT UINT32                     *ControlResult
  )
{
  EFI_STATUS      Status;
  BOOLEAN         TpcmEnabled;
  UINT8           Digest[SM3_DIGEST_SIZE];
  UINTN           TotalLength = 0;
  VOID           *Sm3Context;

  //
  // Check TPCM status
  //
  Status = mTpcmProtocol->GetTpcmCtrlStatus (TpcmInfo, &TpcmEnabled);
  if (EFI_ERROR (Status) || !TpcmEnabled) {
    *ControlResult = TPCM_CTRL_DISABLED;
    return EFI_SUCCESS;
  }

  //
  // Calculate total image length
  //
  for (UINT32 Index = 0; Index < NumAddrRange; Index ++) {
    TotalLength += Ranges[Index].Length;
  }

  //
  // Allocate and initialize SM3 context
  //
  Sm3Context = AllocatePool (Sm3GetContextSize ());
  if (Sm3Context == NULL) {
    *MeasureResult = TPCM_MEASURE_FAILED;
    return EFI_OUT_OF_RESOURCES;
  }

  if (!Sm3Init (Sm3Context)) {
    *MeasureResult = TPCM_MEASURE_FAILED;
    FreePool (Sm3Context);
    return EFI_DEVICE_ERROR;
  }

  //
  // Update SM3 context with image data
  //
  for (UINT32 Index = 0; Index < NumAddrRange; Index++) {
    if (!Sm3Update (Sm3Context,
                    (VOID *)(UINTN)Ranges[Index].Start,
                    Ranges[Index].Length)) {
      *MeasureResult = TPCM_MEASURE_FAILED;
      FreePool (Sm3Context);
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // Finalize SM3 hash
  //
  if (!Sm3Final (Sm3Context, Digest)) {
    *MeasureResult = TPCM_MEASURE_FAILED;
    FreePool (Sm3Context);
    return EFI_DEVICE_ERROR;
  }

  FreePool (Sm3Context);

  //
  // Send measurement to TPCM
  //
  Status = mTpcmProtocol->Measure (Digest, SM3_DIGEST_SIZE);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "TPCM: Measurement failed %r\n",
      Status
      ));
    *MeasureResult = TPCM_MEASURE_FAILED;
    return EFI_ACCESS_DENIED;
  }

  *MeasureResult = TPCM_MEASURE_SUCCESS;
  *ControlResult = TPCM_CTRL_ENFORCED;

  return EFI_SUCCESS;
}

/**
  Check if TPCM verification is enabled.

  @param[in]  This             Pointer to TPCM_IMAGE_VERIFY_PROTOCOL

  @retval TRUE                 TPCM verification is enabled
  @retval FALSE                TPCM verification is disabled
**/
BOOLEAN
EFIAPI
TpcmIsVerifyEnabled (
  IN TPCM_IMAGE_VERIFY_PROTOCOL *This
  )
{
  BOOLEAN         Enabled;
  
  if (EFI_ERROR (mTpcmProtocol->GetTpcmCtrlStatus (TpcmInfo, &Enabled))) {
    return FALSE;
  }

  return Enabled;
}

/**
  Security measurement handler for EFI images.
**/
EFI_STATUS
EFIAPI
TpcmSecurity2Handler (
  IN  UINT32                           AuthenticationStatus,
  IN  CONST EFI_DEVICE_PATH_PROTOCOL   *File,
  IN  VOID                             *FileBuffer,
  IN  UINTN                            FileSize,
  IN  BOOLEAN                          BootPolicy
  )
{
  EFI_STATUS                  Status;
  UINT32                      MeasureResult;
  UINT32                      ControlResult;
  IMAGE_ADDR_RANGE            Range;

  //
  // Setup address range
  //
  Range.Start = (UINT64)(UINTN) FileBuffer;
  Range.Length = FileSize;

  Status = mTpcmVerifyProtocol->TpcmVerifyRaw (
                  mTpcmVerifyProtocol,
                  TPCM_MEASURE_STAGE_LOAD,
                  NULL,
                  0,
                  1,
                  &Range,
                  &MeasureResult,
                  &ControlResult
                  );

  if (EFI_ERROR (Status) || 
      (MeasureResult != TPCM_MEASURE_SUCCESS) ||
      (ControlResult == TPCM_CTRL_BLOCK)) {
    DEBUG ((
      DEBUG_ERROR,
      "TPCM: Image verification failed! Status=%r\n",
      Status
      ));
    return EFI_ACCESS_DENIED;
  }

  return EFI_SUCCESS;
}

/**
  Initialize TPCM verification protocol and register handler.
**/
EFI_STATUS
EFIAPI
InitializeTpcmVerification (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                 Status;
  TPCM_IMAGE_VERIFY_PROTOCOL *TpcmVerify;

  //
  // Allocate protocol structure
  //
  TpcmVerify = AllocateZeroPool (sizeof (TPCM_IMAGE_VERIFY_PROTOCOL));
  if (TpcmVerify == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  TpcmInfo = AllocateZeroPool (sizeof (TPCM_STATUS));
  if (TpcmInfo == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Allocate TPCM INFO failed!\n",
      __func__
      ));
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Locate TPCM protocol
  //
  Status = gBS->LocateProtocol (
		  &gSophgoTpcmProtocolGuid,
		  NULL,
		  (VOID **)&mTpcmProtocol
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Cannot locate TPCM protocol\n",
      __func__
      ));
    goto Exit;
  }

  //
  // Init TPCM
  //
  Status = mTpcmProtocol->InitTpcm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Init TPCM failed!\n",
      __func__
      ));

    goto Exit;
  }

  //
  // Get TPCM Information
  //
  Status = mTpcmProtocol->GetTpcmStatus (TpcmInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Get TPCM status failed!\n",
      __func__
      ));
    goto Exit;
  }

  TpcmVerify->TpcmVerifyRaw = TpcmVerifyImageRaw;
  TpcmVerify->IsTpcmEnabled = TpcmIsVerifyEnabled;

  //
  // Install TPCM verification protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gTpcmImageVerifyProtocolGuid,
                  TpcmVerify,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Install TPCM verification protocol failed!\n",
      __func__
      ));

    goto Exit;
  }

  mTpcmVerifyProtocol = TpcmVerify;

  //
  // Register security handler
  //
  Status = RegisterSecurity2Handler (
             TpcmSecurity2Handler,
             EFI_AUTH_OPERATION_MEASURE_IMAGE
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Register security handler failed!\n",
      __func__
      ));

    goto Exit;
  }

  Status = EFI_SUCCESS;
  
Exit:
  if (TpcmVerify) {
    FreePool (TpcmVerify);
  }

  if (TpcmInfo) {
    FreePool (TpcmInfo);
  }

  return Status;
}
