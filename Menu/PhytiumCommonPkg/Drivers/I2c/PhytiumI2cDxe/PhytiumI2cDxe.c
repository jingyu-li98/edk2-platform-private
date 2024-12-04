/** @file
  Phytium I2c Drivers.

  This driver implement Enumerate and  StartRequest function.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PhytiumI2cDxe.h"

STATIC CONST EFI_GUID  gDevGuid = I2C_GUID;

STATIC I2C_DEVICE_PATH  I2cDevicePathProtocol = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
  (UINT8) (sizeof(VENDOR_DEVICE_PATH)),
  (UINT8) (sizeof(VENDOR_DEVICE_PATH) >> 8),
      },
    },
    EFI_CALLER_ID_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof(EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

/**
  The user entry point for the PhytiumI2cDxe module. The user code starts with
  this function.

  Get the base address of the bus from PCD and initialize the controller of each bus.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PhytiumI2cInitialise (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  I2C_BUS_INFO  *I2C_BUS_INFO;
  EFI_STATUS    Status;
  UINT64        I2cBusAddress;
  UINTN         Index;
  UINTN         Count;

  I2C_BUS_INFO = PcdGetPtr (PcdI2cControllerInformation);
  Count = I2C_BUS_INFO->I2cControllerNumber;

/* Initialize enabled chips */
  for (Index = 0; Index < Count; Index++) {
    I2cBusAddress = I2C_BUS_INFO->I2cControllerAddress[Index];
    Status = PhytiumI2cInitialiseController (
      ImageHandle,
      SystemTable,
      I2cBusAddress
    );
    if (EFI_ERROR(Status))
      return Status;
  }

  return Status;
}


/**
  Initiaize the bus controller.

  Creat a device path for each bus and alloc memory for MasterContext,
  install the protocols on the bus controller handle.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.
  @param[in] BaseAddress  The base addres of bus to be initialized.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PhytiumI2cInitialiseController (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable,
  IN EFI_PHYSICAL_ADDRESS  BaseAddress
  )
{
  EFI_STATUS           Status;
  I2C_INFO             I2cInfo;
  I2C_DEVICE_PATH     *DevicePath;
  I2C_MASTER_CONTEXT  *I2cMasterContext;
  STATIC INTN Bus = 0;

  DevicePath = AllocateCopyPool (sizeof(I2cDevicePathProtocol), &I2cDevicePathProtocol);
  if (DevicePath == NULL) {
    DEBUG ((DEBUG_ERROR, "I2cDxe: I2C device path allocation failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  DevicePath->Vendor.Guid.Data4[7] = Bus;

  I2cMasterContext = AllocateZeroPool (sizeof (I2C_MASTER_CONTEXT));
  if (I2cMasterContext == NULL) {
    DEBUG((DEBUG_ERROR, "I2cDxe: I2C master context allocation failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  I2cMasterContext->Signature = I2C_MASTER_SIGNATURE;
  I2cMasterContext->I2cMaster.Reset = PhytiumI2cReset;
  I2cMasterContext->I2cMaster.StartRequest = PhytiumI2cStartRequest;
  I2cMasterContext->I2cEnumerate.Enumerate = PhytiumI2cEnumerate;
  I2cMasterContext->I2cBusConf.EnableI2cBusConfiguration = PhytiumI2cEnableConf;
  I2cMasterContext->TclkFrequency = PcdGet32 (PcdI2cBusSpeed);
  I2cMasterContext ->BaseAddress = BaseAddress;
  I2cMasterContext->Bus = Bus;
  EfiInitializeLock(&I2cMasterContext->Lock, TPL_NOTIFY);

  ZeroMem (&I2cInfo, sizeof (I2C_INFO));
  I2cInfo.BusAddress = I2cMasterContext->BaseAddress;
  I2cInfo.Speed = PcdGet32 (PcdI2cBusSpeed);
  I2cInit(&I2cInfo);

  Status = gBS->InstallMultipleProtocolInterfaces (
                &I2cMasterContext->Controller,
                &gEfiI2cMasterProtocolGuid,
                &I2cMasterContext->I2cMaster,
                &gEfiI2cEnumerateProtocolGuid,
                &I2cMasterContext->I2cEnumerate,
                &gEfiI2cBusConfigurationManagementProtocolGuid,
                &I2cMasterContext->I2cBusConf,
                &gEfiDevicePathProtocolGuid,
                (EFI_DEVICE_PATH_PROTOCOL *) DevicePath,
                NULL);

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "I2cDxe: Installing protocol interfaces failed!\n"));
    goto fail;
  }
  DEBUG((DEBUG_ERROR, "Succesfully installed controller %d at 0x%llx\n", Bus,I2cMasterContext));

  Bus++;

  return Status;

fail:
  FreePool (I2cMasterContext);
  return Status;
}

/**
  Reset the I2C controller and configure it for use

  This routine must be called at or below TPL_NOTIFY.

  The I2C controller is reset.  The caller must call SetBusFrequench() after
  calling Reset().

  @param[in] This              Pointer to an EFI_I2C_MASTER_PROTOCOL structure.

  @retval EFI_SUCCESS          The reset completed successfully.
  @retval EFI_ALREADY_STARTED  The controller is busy with another transaction.
  @retval EFI_DEVICE_ERROR     The reset operation failed.

**/
EFI_STATUS
EFIAPI
PhytiumI2cReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}


/**
  Enumerate the I2C devices

  This function enables the caller to traverse the set of I2C devices
  on an I2C bus.

  @param[in] This                The platform data for the next device on
                                 the I2C bus was returned successfully.
  @param[in, out] Device         Pointer to a buffer containing an
                                 EFI_I2C_DEVICE structure.  Enumeration is
                                 started by setting the initial EFI_I2C_DEVICE
                                 structure pointer to NULL.  The buffer
                                 receives an EFI_I2C_DEVICE structure pointer
                                 to the next I2C device.

  @retval EFI_SUCCESS            The platform data for the next device on
                                 the I2C bus was returned successfully.
  @retval EFI_INVALID_PARAMETER  Device is NULL
  @retval EFI_NO_MAPPING         *Device does not point to a valid
                                 EFI_I2C_DEVICE structure returned in a
                                 previous call Enumerate().

**/
EFI_STATUS
EFIAPI
PhytiumI2cEnumerate (
  IN CONST EFI_I2C_ENUMERATE_PROTOCOL  *This,
  IN OUT CONST EFI_I2C_DEVICE          **Device
  )
{
  I2C_MASTER_CONTEXT  *I2cMasterContext;
  UINT8               *DevicesPcd;
  UINT8               *DeviceBusPcd;
  UINTN               Index;
  UINTN               NextIndex;
  UINTN               DevCount;
  UINT8               NextDeviceAddress;

  I2cMasterContext = I2C_SC_FROM_ENUMERATE(This);
  DevCount = PcdGetSize (PcdI2cSlaveAddress);
  DevicesPcd = PcdGetPtr (PcdI2cSlaveAddress);
  DeviceBusPcd = PcdGetPtr (PcdI2cSlaveBusesNumber);

  if ( * Device == NULL) {
    for (Index = 0; Index < DevCount ; Index++) {
      if (DeviceBusPcd[Index] != I2cMasterContext->Bus)
        continue;
      if (Index < DevCount)
        PhytiumI2cAllocDevice (DevicesPcd[Index], I2cMasterContext->Bus, Device);
      return EFI_SUCCESS;
    }
  } else {
 /* Device is not NULL, so something was already allocated */
    for (Index = 0; Index < DevCount; Index++) {
      if (DeviceBusPcd[Index] != I2cMasterContext->Bus)
        continue;
      if (DevicesPcd[Index] == I2C_DEVICE_ADDRESS((*Device)->DeviceIndex)) {
        for (NextIndex = Index + 1; NextIndex < DevCount; NextIndex++) {
          if (DeviceBusPcd[NextIndex] != I2cMasterContext->Bus)
            continue;
          NextDeviceAddress = DevicesPcd[NextIndex];
          if (NextIndex < DevCount)
            PhytiumI2cAllocDevice(NextDeviceAddress, I2cMasterContext->Bus, Device);
          return EFI_SUCCESS;
        }
      }
    }
    *Device = NULL;
     return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

/**
  Start an I2C transaction on the host controller.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  This function initiates an I2C transaction on the controller.  To
  enable proper error handling by the I2C protocol stack, the I2C
  master protocol does not support queuing but instead only manages
  one I2C transaction at a time.  This API requires that the I2C bus
  is in the correct configuration for the I2C transaction.

  The transaction is performed by sending a start-bit and selecting the
  I2C device with the specified I2C slave address and then performing
  the specified I2C operations.  When multiple operations are requested
  they are separated with a repeated start bit and the slave address.
  The transaction is terminated with a stop bit.

  When Event is NULL, StartRequest operates synchronously and returns
  the I2C completion status as its return value.

  When Event is not NULL, StartRequest synchronously returns EFI_SUCCESS
  indicating that the I2C transaction was started asynchronously.  The
  transaction status value is returned in the buffer pointed to by
  I2cStatus upon the completion of the I2C transaction when I2cStatus
  is not NULL.  After the transaction status is returned the Event is
  signaled.

  Note: The typical consumer of this API is the I2C host protocol.
  Extreme care must be taken by other consumers of this API to prevent
  confusing the third party I2C drivers due to a state change at the
  I2C device which the third party I2C drivers did not initiate.  I2C
  platform specific code may use this API within these guidelines.

  @param[in] This               Pointer to an EFI_I2C_MASTER_PROTOCOL structure.
  @param[in] SlaveAddress       Address of the device on the I2C bus.  Set the
                                I2C_ADDRESSING_10_BIT when using 10-bit addresses,
                                clear this bit for 7-bit addressing.  Bits 0-6
                                are used for 7-bit I2C slave addresses and bits
                                0-9 are used for 10-bit I2C slave addresses.
  @param[in] RequestPacket      Pointer to an EFI_I2C_REQUEST_PACKET
                                structure describing the I2C transaction.
  @param[in] Event              Event to signal for asynchronous transactions,
                                NULL for asynchronous transactions
  @param[out] I2cStatus         Optional buffer to receive the I2C transaction
                                completion status

  @retval EFI_SUCCESS           The asynchronous transaction was successfully
                                started when Event is not NULL.
  @retval EFI_SUCCESS           The transaction completed successfully when
                                Event is NULL.
  @retval EFI_ALREADY_STARTED   The controller is busy with another transaction.
  @retval EFI_BAD_BUFFER_SIZE   The RequestPacket->LengthInBytes value is too
                                large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the
                                transaction.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NOT_FOUND         Reserved bit set in the SlaveAddress parameter
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave
                                address.  EFI_DEVICE_ERROR will be returned if
                                the controller cannot distinguish when the NACK
                                occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C transaction
  @retval EFI_UNSUPPORTED       The controller does not support the requested
                                transaction.

**/
EFI_STATUS
EFIAPI
PhytiumI2cStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This,
  IN UINTN                          SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET         *RequestPacket,
  IN EFI_EVENT                      Event     OPTIONAL,
  OUT EFI_STATUS                    *I2cStatus OPTIONAL
  )
{
  I2C_MASTER_CONTEXT  *I2cMasterContext;
  EFI_I2C_OPERATION   *Operation;
  I2C_INFO            I2cInfo;
  STATIC UINT8        Reg;
  UINTN               Count;
  UINTN               ReadMode;
  INT32               Alen;

  I2cMasterContext= I2C_SC_FROM_MASTER(This);
  Alen = 1;

  ZeroMem (&I2cInfo, sizeof (I2C_INFO));
  I2cInfo.BusAddress = I2cMasterContext->BaseAddress;
  I2cInfo.Speed = PcdGet32 (PcdI2cBusSpeed);
  I2cInfo.SlaveAddress = SlaveAddress;

  ASSERT (RequestPacket != NULL);
  ASSERT (I2cMasterContext != NULL);
  for (Count = 0; Count < RequestPacket->OperationCount; Count++) {
    Operation = &RequestPacket->Operation[Count];
    ReadMode = Operation->Flags & I2C_FLAG_READ;
    if(Count == 0 ){
     Reg = *(Operation->Buffer);
    }
    if ( Count >= 1){
       if( ReadMode ) {
        I2cRead (
          &I2cInfo,
          Reg,
          Alen,
          Operation->Buffer,
          Operation->LengthInBytes
          );
        } else {
        I2cWrite (
          &I2cInfo,
          Reg,
          Alen,
          Operation->Buffer,
          Operation->LengthInBytes
          );
        }
    }
  }
  if (I2cStatus != NULL)
    *I2cStatus = EFI_SUCCESS;
  if (Event != NULL)
    gBS->SignalEvent(Event);
  return EFI_SUCCESS;
}

/**
  Allocate for the I2C devices

  @param[in] SlaveAddress        I2c slave device address.
  @param[in] BusNumer            Numer of the bus where slave is located.
  @param[in, out] Device         Pointer to a buffer containing an
                                 EFI_I2C_DEVICE structure.  Enumeration is
                                 started by setting the initial EFI_I2C_DEVICE
                                 structure pointer to NULL.  The buffer
                                 receives an EFI_I2C_DEVICE structure pointer
                                 to the next I2C device.

  @retval EFI_SUCCESS            The platform data for the next device on
                                 the I2C bus was returned successfully.
  @retval EFI_INVALID_PARAMETER  Device is NULL
  @retval EFI_NO_MAPPING         *Device does not point to a valid
                                 EFI_I2C_DEVICE structure returned in a
                                 previous call Enumerate().

**/
EFI_STATUS
PhytiumI2cAllocDevice (
  IN UINT32                    SlaveAddress,
  IN UINT8                     BusNumer,
  IN OUT CONST EFI_I2C_DEVICE  **Device
  )
{
  EFI_I2C_DEVICE  *Dev;
  EFI_STATUS       Status;
  EFI_GUID        *TmpGuidP;
  UINT32          *TmpSlaveArray;

  Status = gBS->AllocatePool ( EfiBootServicesData,
            sizeof(EFI_I2C_DEVICE),
            (VOID **) &Dev );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "I2cDxe: I2C device memory allocation failed\n"));
    return Status;
  }

  *Device = Dev;
  Dev->DeviceIndex = I2C_DEVICE_INDEX (BusNumer, SlaveAddress);
  Dev->SlaveAddressCount =  1 ;
  Dev->I2cBusConfiguration = 0;

  Status = gBS->AllocatePool ( EfiBootServicesData,
    sizeof (UINT32),
    (VOID **) &TmpSlaveArray);
  if (EFI_ERROR(Status)) {
    goto fail1;
  }
  TmpSlaveArray[0] = SlaveAddress;
  Dev->SlaveAddressArray = TmpSlaveArray;

  Status = gBS->AllocatePool ( EfiBootServicesData,
    sizeof (EFI_GUID),
    (VOID **) &TmpGuidP);
  if (EFI_ERROR(Status)) {
    goto fail2;
  }
  *TmpGuidP = gDevGuid;
  Dev->DeviceGuid = TmpGuidP;

  DEBUG ((DEBUG_INFO, " i2c device with address 0x%x\n", (UINTN)SlaveAddress));
  return EFI_SUCCESS;

fail2:
  FreePool(TmpSlaveArray);
fail1:
  FreePool(Dev);

  return Status;
}

/**
  Enable access to an I2C bus configuration.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  Reconfigure the switches and multiplexers in the I2C bus to enable
  access to a specific I2C bus configuration.  Also select the maximum
  clock frequency for this I2C bus configuration.

  This routine uses the I2C Master protocol to perform I2C transactions
  on the local bus.  This eliminates any recursion in the I2C stack for
  configuration transactions on the same I2C bus.  This works because the
  local I2C bus is idle while the I2C bus configuration is being enabled.

  If I2C transactions must be performed on other I2C busses, then the
  EFI_I2C_HOST_PROTOCOL, the EFI_I2C_IO_PROTCOL, or a third party I2C
  driver interface for a specific device must be used.  This requirement
  is because the I2C host protocol controls the flow of requests to the
  I2C controller.  Use the EFI_I2C_HOST_PROTOCOL when the I2C device is
  not enumerated by the EFI_I2C_ENUMERATE_PROTOCOL.  Use a protocol
  produced by a third party driver when it is available or the
  EFI_I2C_IO_PROTOCOL when the third party driver is not available but
  the device is enumerated with the EFI_I2C_ENUMERATE_PROTOCOL.

  When Event is NULL, EnableI2cBusConfiguration operates synchronously
  and returns the I2C completion status as its return value.

  @param[in]  This            Pointer to an EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL
                              structure.
  @param[in]  I2cBusConfiguration Index of an I2C bus configuration.  All
                                  values in the range of zero to N-1 are
                                  valid where N is the total number of I2C
                                  bus configurations for an I2C bus.
  @param[in]  Event           Event to signal when the transaction is complete
  @param[in]  I2cStatus       Buffer to receive the transaction status.

  @return  When Event is NULL, EnableI2cBusConfiguration operates synchrouously
  and returns the I2C completion status as its return value.  In this case it is
  recommended to use NULL for I2cStatus.  The values returned from
  EnableI2cBusConfiguration are:

  @retval EFI_SUCCESS           The asynchronous bus configuration request
                                was successfully started when Event is not
                                NULL.
  @retval EFI_SUCCESS           The bus configuration request completed
                                successfully when Event is NULL.
  @retval EFI_DEVICE_ERROR      The bus configuration failed.
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value

**/
EFI_STATUS
EFIAPI
PhytiumI2cEnableConf (
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL  *This,
  IN UINTN                                                I2cBusConfiguration,
  IN EFI_EVENT                                            Event     OPTIONAL,
  IN EFI_STATUS                                           *I2cStatus OPTIONAL
  )
{
  //
  //Not yet realized
  //
  if (I2cStatus != NULL)
    I2cStatus = EFI_SUCCESS;
  if (Event != NULL)
    gBS->SignalEvent(Event);
  return EFI_SUCCESS;
}
