/* @file
  ComponentName.c
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include  <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "PhyGopDxe.h"

//
// EFI Component Name Functions
//
EFI_STATUS
EFIAPI
PhyComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

EFI_STATUS
EFIAPI
PhyComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  );

//
// EFI Component Name Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL     gPhyComponentName = {
  (EFI_COMPONENT_NAME_GET_DRIVER_NAME) PhyComponentNameGetDriverName,
  (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME) PhyComponentNameGetControllerName,
  "eng"
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mPhyDriverNameTable[] = {
  { "eng", L"Phytium E2000 BMC GOP Driver v1.4 20231209" },
  { NULL , NULL }
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mPhyControllerNameTable[] = {
  { "eng", L"Phytium E2000 BMC display v1.4 20231209" },
  { NULL , NULL }
};

/**
  Retrieves a Unicode string that is the user readable name of the EFI Driver.

  @param[in]  This       A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.

  @param[in]  Language   A pointer to a three character ISO 639-2 language identifier.
                         This is the language of the driver name that that the caller
                         is requesting, and it must match one of the languages specified
                         in SupportedLanguages.  The number of languages supported by a
                         driver is up to the driver writer.

  @param[out] DriverName A pointer to the Unicode string to return.  This Unicode string
                         is the name of the driver specified by This in the language
                         specified by Language.

  @retval     EFI_SUCCESS             The Unicode string for the Driver specified by This
                                      and the language specified by Language was returned
                                      in DriverName.

  @retval     EFI_INVALID_PARAMETER   Language is NULL.

  @retval     EFI_INVALID_PARAMETER   DriverName is NULL.

  @retval     EFI_UNSUPPORTED         The driver specified by This does not support the
                                      language specified by Language.
**/
EFI_STATUS
EFIAPI
PhyComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mPhyDriverNameTable,
           DriverName,
           (BOOLEAN)(This == &gPhyComponentName)
           );
}

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

  @param[in]  This              A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.

  @param[in]  ControllerHandle  The handle of a controller that the driver specified by
                                This is managing. This handle specifies the controller
                                whose name is to be returned.

  @param[in]  ChildHandle       The handle of the child controller to retrieve the name
                                of. This is an optional parameter that may be NULL.  It
                                will be NULL for device drivers.  It will also be NULL
                                for a bus drivers that wish to retrieve the name of the
                                bus controller. It will not be NULL for a bus driver
                                that wishes to retrieve the name of a child controller.

  @param[in]  Language          A pointer to a three character ISO 639-2 language
                                identifier. This is the language of the controller name
                                that that the caller is requesting, and it must match one
                                of the languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up to the
                                driver writer.

  @param[out] ControllerName    A pointer to the Unicode string to return.  This Unicode
                                string is the name of the controller specified by
                                ControllerHandle and ChildHandle in the language specified
                                by Language from the point of view of the driver specified
                                by This.

  @retval     EFI_SUCCESS             The Unicode string for the user readable name in the
                                      language specified by Language for the driver
                                      specified by This was returned in DriverName.

  @retval     EFI_INVALID_PARAMETER   ControllerHandle is not a valid EFI_HANDLE.

  @retval     EFI_INVALID_PARAMETER   ChildHandle is not NULL and it is not a valid EFI_HANDLE.

  @retval     EFI_INVALID_PARAMETER   Language is NULL.

  @retval     EFI_INVALID_PARAMETER   ControllerName is NULL.

  @retval     EFI_UNSUPPORTED         The driver specified by This is not currently managing
                                      the controller specified by ControllerHandle and
                                      ChildHandle.

  @retval     EFI_UNSUPPORTED         The driver specified by This does not support the
                                      language specified by Language.
**/
EFI_STATUS
EFIAPI
PhyComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
  EFI_STATUS                      Status;
  NON_DISCOVERABLE_DEVICE         *Dev;

  //
  // This is a device driver, so ChildHandle must be NULL.
  //
  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Check Controller's handle
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  (VOID **) &Dev,
                  gPhyDriverBinding.DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (!EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          ControllerHandle,
          &gEdkiiNonDiscoverableDeviceProtocolGuid,
          gPhyDriverBinding.DriverBindingHandle,
          ControllerHandle
          );

    return EFI_UNSUPPORTED;
  }

  if (Status != EFI_ALREADY_STARTED) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get the Graphics Output Protocol on Controller
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  gPhyDriverBinding.DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get the GMI 's Device structure
  //
 // Private = GMI_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);

  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mPhyControllerNameTable,
           ControllerName,
           (BOOLEAN)(This == &gPhyComponentName)
           );
}
