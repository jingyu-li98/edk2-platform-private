/** @file
  Phytium I2c Device Driver.

  This driver provides the Transfer function to call the QueueRequest() of
  IoProtocol to read or write the i2c slave device.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Protocol/DriverBinding.h>
#include <Protocol/I2cIo.h>
#include <Protocol/I2cDevicesIo.h>
#include <Protocol/PhytiumI2c.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Pi/PiI2c.h>

#include "PhytiumI2cDevicesIo.h"

STATIC CONST EFI_GUID I2cGuid = I2C_GUID;

EFI_DRIVER_BINDING_PROTOCOL gDriverBindingProtocol = {
  PhytiumI2cDevicesIoSupported,
  PhytiumI2cDevicesIoStart,
  PhytiumI2cDevicesIoStop,
};

//
// Driver name table
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE mPhytiumI2cDevicesIoDriverNameTable[] = {
  { "eng;en", L"Phytium I2c Devices Io Driver" },
  { NULL , NULL }
};

//
// EFI Component Name Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL gPhytiumI2cDevicesIoComponentName = {
  (EFI_COMPONENT_NAME_GET_DRIVER_NAME) PhytiumI2cDevicesIoComponentNameGetDriverName,
  (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME) PhytiumI2cDevicesIoComponentNameGetControllerName,
  "eng"
};

//
// EFI Component Name 2 Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME2_PROTOCOL gPhytiumI2cDevicesIoComponentName2 = {
  PhytiumI2cDevicesIoComponentNameGetDriverName,
  PhytiumI2cDevicesIoComponentNameGetControllerName,
  "en"
};

/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 4646 or ISO 639-2 language code format.

  @param  DriverName[out]       A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  )
{
  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mPhytiumI2cDevicesIoDriverNameTable,
           DriverName,
           (BOOLEAN)(This != &gPhytiumI2cDevicesIoComponentName2)
           );
}

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle[in]  The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.

  @param  ChildHandle[in]       The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 4646 or ISO 639-2 language code format.

  @param  ControllerName[out]   A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle        OPTIONAL,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Tests to see if this driver supports a given controller. If a child device is provided,
  it further tests to see if this driver supports creating a handle for the specified child device.

  This function checks to see if the driver specified by This supports the device specified by
  ControllerHandle. Drivers will typically use the device path attached to
  ControllerHandle and/or the services from the bus I/O abstraction attached to
  ControllerHandle to determine if the driver supports ControllerHandle. This function
  may be called many times during platform initialization. In order to reduce boot times, the tests
  performed by this function must be very small, and take as little time as possible to execute. This
  function must not change the state of any hardware devices, and this function must be aware that the
  device specified by ControllerHandle may already be managed by the same driver or a
  different driver. This function must match its calls to AllocatePages() with FreePages(),
  AllocatePool() with FreePool(), and OpenProtocol() with CloseProtocol().
  Because ControllerHandle may have been previously started by the same driver, if a protocol is
  already in the opened state, then it must not be closed with CloseProtocol(). This is required
  to guarantee the state of ControllerHandle is not modified by this function.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to test. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For bus drivers, if this parameter is not NULL, then
                                   the bus driver must determine if the bus controller specified
                                   by ControllerHandle and the child controller specified
                                   by RemainingDevicePath are both supported by this
                                   bus driver.

  @retval EFI_SUCCESS              The device specified by ControllerHandle and
                                   RemainingDevicePath is supported by the driver specified by This.
  @retval EFI_ALREADY_STARTED      The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by the driver
                                   specified by This.
  @retval EFI_ACCESS_DENIED        The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by a different
                                   driver or an application that requires exclusive access.
                                   Currently not implemented.
  @retval EFI_UNSUPPORTED          The device specified by ControllerHandle and
                                   RemainingDevicePath is not supported by the driver specified by This.
**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS           Status;
  EFI_I2C_IO_PROTOCOL  *I2cIoProtocol;
  UINT8                *I2cDeviceAddresses;
  UINT8                *I2cDeviceBuses;
  UINTN                Index;

  Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiI2cIoProtocolGuid,
      (VOID **) &I2cIoProtocol,
      gImageHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_BY_DRIVER
      );
  if (EFI_ERROR(Status)) {
    goto Fail;
  }

  /* get devices' addresses from PCD */
  I2cDeviceAddresses = PcdGetPtr (PcdDevicesAddress);
  I2cDeviceBuses = PcdGetPtr (PcdI2cDevicesBusesNumber);
  if (I2cDeviceAddresses == NULL) {
    Status = EFI_UNSUPPORTED;
    DEBUG((DEBUG_INFO, "I2CDeviceSupported: I2C device found failed \n"));
    goto Fail;
  }

  for (Index = 0; I2cDeviceAddresses[Index] != '\0'; Index++) {
    /* I2C guid must fit and valid DeviceIndex must be provided */
    if (CompareGuid(I2cIoProtocol->DeviceGuid, &I2cGuid) &&
        I2cIoProtocol->DeviceIndex == I2C_DEVICE_INDEX(I2cDeviceBuses[Index],
          I2cDeviceAddresses[Index])) {
      DEBUG((DEBUG_INFO, "I2CDeviceSupported: attached to i2c device\n"));
      Status = EFI_SUCCESS;
      break;
    }
  }

Fail:
  gBS->CloseProtocol (
      ControllerHandle,
      &gEfiI2cIoProtocolGuid,
      gImageHandle,
      ControllerHandle
      );

  return Status;
}

/**
  I2C Device Transfer Function.

  @param[in] This          Pointer to an PHYTIUM_I2C_DEVICES_IO_PROTOCOL structure.
  @param[in] Address       Slave device address to operate on.
  @param[in] Length        Length to operate
  @param[in] Buffer        Buffer for reading or writing.
  @param[in] Operation     flag of read or write

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoTransfer (
  IN CONST PHYTIUM_I2C_DEVICES_IO_PROTOCOL  *This,
  IN UINT16                                 Address,
  IN UINT32                                 Length,
  IN UINT8                                  *Buffer,
  IN UINT8                                  Operation
  )
{
  I2C_SLAVE_DEVICE_CONTEXT  *I2cDeviceContext;
  EFI_I2C_REQUEST_PACKET    *RequestPacket;
  EFI_STATUS                Status;
  UINTN                     RequestPacketSize;
  UINT32                    BufferLength;
  UINT32                    CurrentAddress;

  I2cDeviceContext = I2CDEVICE_SC_FROM_I2CDEVICE(This);
  CurrentAddress = Address;

  ASSERT(I2cDeviceContext != NULL);
  ASSERT(I2cDeviceContext->I2cIo != NULL);

  RequestPacketSize = sizeof(UINTN) + sizeof (EFI_I2C_OPERATION) * 2;
  RequestPacket = AllocateZeroPool (RequestPacketSize);
  if (RequestPacket == NULL){
    Status = EFI_OUT_OF_RESOURCES;
    goto Out;
  }

  RequestPacket->OperationCount = 2;
  RequestPacket->Operation[0].LengthInBytes = 1;
  RequestPacket->Operation[0].Buffer = AllocateZeroPool ( RequestPacket->Operation[0].LengthInBytes );
  if (RequestPacket->Operation[0].Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Out;
  }

  RequestPacket->Operation[1].Flags = Operation;
  BufferLength = (Length <= MAX_BUFFER_LENGTH ? Length : MAX_BUFFER_LENGTH);
  RequestPacket->Operation[0].Buffer[0] = CurrentAddress;
  RequestPacket->Operation[1].LengthInBytes = BufferLength;
  RequestPacket->Operation[1].Buffer = Buffer;
  Status = I2cDeviceContext->I2cIo->QueueRequest(I2cDeviceContext->I2cIo, 0, NULL, RequestPacket, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I2cDeviceTransfer: error %r during transmission\n", Status));
  }

Out:
  FreePool(RequestPacket->Operation[0].Buffer);
  FreePool(RequestPacket);

  return Status;
}

/**
  Starts a device controller or a bus controller.

  The Start() function is designed to be invoked from the EFI boot service ConnectController().
  As a result, much of the error checking on the parameters to Start() has been moved into this
  common boot service. It is legal to call Start() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE.
  2. If RemainingDevicePath is not NULL, then it must be a pointer to a naturally aligned
     EFI_DEVICE_PATH_PROTOCOL.
  3. Prior to calling Start(), the Supported() function for the driver specified by This must
     have been called with the same calling parameters, and Supported() must have returned EFI_SUCCESS.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to start. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For a bus driver, if this parameter is NULL, then handles
                                   for all the children of Controller are created by this driver.
                                   If this parameter is not NULL and the first Device Path Node is
                                   not the End of Device Path Node, then only the handle for the
                                   child device specified by the first Device Path Node of
                                   RemainingDevicePath is created by this driver.
                                   If the first Device Path Node of RemainingDevicePath is
                                   the End of Device Path Node, no child handle is created by this
                                   driver.

  @retval EFI_SUCCESS              The device was started.
  @retval EFI_DEVICE_ERROR         The device could not be started due to a device error.Currently not implemented.
  @retval EFI_OUT_OF_RESOURCES     The request could not be completed due to a lack of resources.
  @retval Others                   The driver failded to start the device.

**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
{
  I2C_SLAVE_DEVICE_CONTEXT  *I2cDeviceContext;
  EFI_STATUS                Status;

  I2cDeviceContext = AllocateZeroPool (sizeof(I2C_SLAVE_DEVICE_CONTEXT));
  if (I2cDeviceContext == NULL) {
    DEBUG((DEBUG_ERROR, "I2cDevice: allocation I2cDeviceContext fail\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto Fail;
  }

  I2cDeviceContext->ControllerHandle = ControllerHandle;
  I2cDeviceContext->Signature = I2C_SLAVE_DEVICE_SIGNATURE;
  I2cDeviceContext->I2cDeviceProtocol.Transfer = PhytiumI2cDevicesIoTransfer;

  Status = gBS->OpenProtocol (
      ControllerHandle,
      &gEfiI2cIoProtocolGuid,
      (VOID **) &I2cDeviceContext->I2cIo,
      gImageHandle,
      ControllerHandle,
      EFI_OPEN_PROTOCOL_BY_DRIVER
      );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to open I2cIoProtocol\n"));
    goto Fail;
  }

  I2cDeviceContext->I2cDeviceProtocol.Identifier = I2cDeviceContext->I2cIo->DeviceIndex;
  Status = gBS->InstallMultipleProtocolInterfaces (
      &ControllerHandle,
      &gPhytiumI2cDevicesIoProtocolGuid,
      &I2cDeviceContext->I2cDeviceProtocol,
      NULL
      );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I2cDevice: failed to install I2c Slave Device protocol\n"));
    goto Fail;
  }

  return Status;

Fail:
  FreePool(I2cDeviceContext);
  gBS->CloseProtocol (
      ControllerHandle,
      &gEfiI2cIoProtocolGuid,
      gImageHandle,
      ControllerHandle
      );

  return Status;
}

/**
  Stops a device controller or a bus controller.

  The Stop() function is designed to be invoked from the EFI boot service DisconnectController().
  As a result, much of the error checking on the parameters to Stop() has been moved
  into this common boot service. It is legal to call Stop() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE that was used on a previous call to this
     same driver's Start() function.
  2. The first NumberOfChildren handles of ChildHandleBuffer must all be a valid
     EFI_HANDLE. In addition, all of these handles must have been created in this driver's
     Start() function, and the Start() function must have called OpenProtocol() on
     ControllerHandle with an Attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.

  @param[in]  This              A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle  A handle to the device being stopped. The handle must
                                support a bus specific I/O protocol for the driver
                                to use to stop the device.
  @param[in]  NumberOfChildren  The number of child device handles in ChildHandleBuffer.
  @param[in]  ChildHandleBuffer An array of child handles to be freed. May be NULL
                                if NumberOfChildren is 0.

  @retval EFI_SUCCESS           The device was stopped.
  @retval EFI_DEVICE_ERROR      The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                  ControllerHandle,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer OPTIONAL
  )
{
  PHYTIUM_I2C_DEVICES_IO_PROTOCOL  *I2cDeviceProtocol;
  I2C_SLAVE_DEVICE_CONTEXT         *I2cDeviceContext;
  EFI_STATUS                       Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gPhytiumI2cDevicesIoProtocolGuid,
                  (VOID **) &I2cDeviceProtocol,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  I2cDeviceContext = I2CDEVICE_SC_FROM_I2CDEVICE(I2cDeviceProtocol);

  gBS->UninstallMultipleProtocolInterfaces (
         &ControllerHandle,
         &gPhytiumI2cDevicesIoProtocolGuid,
         &I2cDeviceContext->I2cDeviceProtocol,
         &gEfiDriverBindingProtocolGuid,
         &gDriverBindingProtocol,
         NULL
         );

  gBS->CloseProtocol (
        ControllerHandle,
        &gEfiI2cIoProtocolGuid,
        gImageHandle,
        ControllerHandle
        );

  FreePool(I2cDeviceContext);
  return Status;
}

/**
  The user entry point for the I2CDeviceDrive module. The user code starts with
  this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PhytiumI2cDevicesIoInitialise (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = EfiLibInstallDriverBindingComponentName2 (
      ImageHandle,
      SystemTable,
      &gDriverBindingProtocol,
      ImageHandle,
      &gPhytiumI2cDevicesIoComponentName,
      &gPhytiumI2cDevicesIoComponentName2
      );

  ASSERT_EFI_ERROR (Status);
  return Status;
}

