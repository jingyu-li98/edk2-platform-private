/** @file
  Master header file for SecCore.

  Copyright (c) 2022, Ventana Micro Systems Inc. All rights reserved.<BR>
  Copyright (c) 2023, Academy of Intelligent Innovation, Shandong Universiy, China.P.R. All rights reserved.<BR>
  Copyright (c) 2025, SOPHGO Technologies Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef PEI_H_
#define PEI_H_

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/ExtractGuidedSectionLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffExtraActionLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PeCoffLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseRiscVSbiLib.h>
#include <Library/PrePiLib.h>
#include <Library/PrePiHobListPointerLib.h>

/**
  Perform Platform PEIM initialization.

  @return EFI_SUCCESS     The platform initialized successfully.
  @retval  Others        - As the error code indicates

**/
EFI_STATUS
EFIAPI
PlatformPeimInitialization (
  VOID
  );

/**
  Perform Memory PEIM initialization.

  @param  DeviceTreeAddress  Pointer to FDT.
  @return EFI_SUCCESS        The platform initialized successfully.
  @retval  Others          - As the error code indicates

**/
EFI_STATUS
EFIAPI
MemoryPeimInitialization (
  VOID	
  );

/**
  Perform CPU PEIM initialization.

  @return EFI_SUCCESS     The platform initialized successfully.
  @retval  Others        - As the error code indicates

**/
EFI_STATUS
EFIAPI
CpuPeimInitialization (
  VOID
  );

#endif
