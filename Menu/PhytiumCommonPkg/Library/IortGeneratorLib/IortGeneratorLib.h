/** @file
Iort table common libary header file.

Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef IORT_COMMON_LIB_H_
#define IORT_COMMON_LIB_H_

#include <Protocol/ConfigurationManagerProtocol.h>
#include <Uefi/UefiBaseType.h>

#define EFI_ACPI_IORT_OEM_TABLE_ID     SIGNATURE_64('P','H','Y','T','I','O','R','T') // OEM table id 8 bytes long
/** A structure that describes the Node indexer
    used for indexing the IORT nodes.
*/
typedef struct IortNodeIndexer {
  /// Index token for the Node
  CM_OBJECT_TOKEN    Token;
  /// Pointer to the node
  VOID               *Object;
  /// Node offset from the start of the IORT table
  UINT32             Offset;
} IORT_NODE_INDEXER;


/**
  Return GitItsIdentifierArray Info.

  @param [in]      Token               A unique token for identifying the requested
                                       CM_ARM_ITS_IDENTIFIER object.
  @param [in, out] ItsIdentifier       A pointer to a pointer to the CM_ARM_ITS_IDENTIFIER object.
  @param [in, out] ItsIdentifierCount  The Count of object.

  @retval EFI_SUCCESS                  Success.
**/
EFI_STATUS
EFIAPI
GetEArmObjGicItsIdentifierArray (
  IN     CM_OBJECT_TOKEN        Token,
  IN OUT CM_ARM_ITS_IDENTIFIER  **ItsIdentifier,
  IN OUT UINT32                 *ItsIdentifierCount
  );
/**
  Return GitItsGroup Info.

  @param [in, out] ItsGroupNodeList    A pointer to a pointer to the CM_ARM_ITS_GROUP_NODE object.
  @param [in, out] ItsGroupNodeCount   The Count of object.

  @retval EFI_SUCCESS                  Success.
**/
EFI_STATUS
EFIAPI
GetEArmObjItsGroup (
  IN OUT  CM_ARM_ITS_GROUP_NODE  **ItsGroupNodeList,
  IN OUT  UINT32                 *ItsGroupNodeCount
  );
/**
  Return NamedComponent Info.

  @param [in, out] NamedComponentNodeList   A pointer to a pointer to the CM_ARM_NAMED_COMPONENT_NODE object.
  @param [in, out] NamedComponentNodeCount  The Count of object.

  @retval EFI_SUCCESS                  Success.
  @retval EFI_NOT_FOUND                The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjNamedComponent (
  IN OUT  CM_ARM_NAMED_COMPONENT_NODE  **NamedComponentNodeList,
  IN OUT  UINT32                       *NamedComponentNodeCount
  );

/**
  Return RootComplex Info.

  @param [in, out] RootComplexNodeList   A pointer to a pointer to the CM_ARM_ROOT_COMPLEX_NODE object.
  @param [in, out] RootComplexNodeCount  The Count of object.

  @retval EFI_SUCCESS                    Success.
**/
EFI_STATUS
EFIAPI
GetEArmObjRootComplex (
  IN OUT CM_ARM_ROOT_COMPLEX_NODE  **RootComplexNodeList,
  IN OUT UINT32                    *RootComplexNodeCount
  );
/**
  Return SmmuV1/SmmuV2 Info.

  @param [in, out] SmmuV1V2NodeList      A pointer to a pointer to the CM_ARM_SMMUV1_SMMUV2_NODE object.
  @param [in, out] SmmuV1V2NodeCount     The Count of object.

  @retval EFI_SUCCESS                    Success.
  @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjSmmuV1SmmuV2 (
  IN OUT CM_ARM_SMMUV1_SMMUV2_NODE **SmmuV1V2NodeList,
  IN OUT UINT32                    *SmmuV1V2NodeCount
  );
/**
  Return Smmu Interrupt Info.

  @param [in]      Token                 A unique token for identifying the requested
                                         CM_ARM_SMMU_INTERRUPT object.
  @param [in, out] SmmuInterrupt         A pointer to a pointer to the CM_ARM_SMMU_INTERRUPT object.
  @param [in, out] SmmuInterruptCount    The Count of object.

  @retval EFI_SUCCESS                    Success.
  @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjSmmuInterruptArray (
  IN     CM_OBJECT_TOKEN        InterruptToken,
  IN OUT CM_ARM_SMMU_INTERRUPT  **SmmuInterrupt,
  IN OUT UINT32                 *SmmuInterruptCount
  );
/**
  Return SmmuV3 Info.

  @param [in, out] SmmuV1V2NodeList      A pointer to a pointer to the CM_ARM_SMMUV3_NODE object.
  @param [in, out] SmmuV1V2NodeCount     The Count of object.

  @retval EFI_SUCCESS                    Success.
  @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjSmmuV3 (
  IN OUT CM_ARM_SMMUV3_NODE  **SmmuV3NodeList,
  IN OUT UINT32              *SmmuV3NodeCount
  );

/**
  Return Pmgc Info.

  @param [in, out] PmcgNodeList   A pointer to a pointer to the CM_ARM_PMCG_NODE object.
  @param [in, out] PmcgNodeCount  The Count of object.

  @retval EFI_SUCCESS                    Success.
  @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjPmcg (
  IN OUT CM_ARM_PMCG_NODE    **PmcgNodeList,
  IN OUT UINT32              *PmcgNodeCount
  );

/**
  Return IdMapping Info.

  @param [in]      Token                 A unique token for identifying the requested
                                         CM_ARM_ID_MAPPING object.
  @param [in, out] SmmuInterrupt         A pointer to a pointer to the CM_ARM_ID_MAPPING object.
  @param [in, out] SmmuInterruptCount    The Count of object.

  @retval EFI_SUCCESS                    Success.
  @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetEArmObjIdMappingArray (
  IN     CM_OBJECT_TOKEN    IdMappingToken,
  IN OUT CM_ARM_ID_MAPPING  **IdMappings,
  IN OUT UINT32             *IdMappingCount
  );
/**
  Initialize the platform info.
**/
VOID
InitPlatformInfo (
  VOID
  );
#endif

