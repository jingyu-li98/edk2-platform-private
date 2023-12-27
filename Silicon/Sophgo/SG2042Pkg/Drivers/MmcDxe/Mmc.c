/** @file
  Main file of the MMC Dxe driver. The driver entrypoint is defined into this file.

  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
  Copyright (c) 2023, Academy of Intelligent Innovation, Shandong Universiy, China.P.R. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Protocol/DevicePath.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>

#include "Mmc.h"

EFI_BLOCK_IO_MEDIA mMmcMediaTemplate = {
  SIGNATURE_32 ('m','m','c','o'),           // MediaId
  TRUE,                                     // RemovableMedia
  FALSE,                                    // MediaPresent
  FALSE,                                    // LogicalPartition
  FALSE,                                    // ReadOnly
  FALSE,                                    // WriteCaching
  512,                                      // BlockSize
  4,                                        // IoAlign
  0,                                        // Pad
  0                                         // LastBlock
};

//
// This device structure is serviced as a header.
// Its next field points to the first root bridge device node.
//
LIST_ENTRY  mMmcHostPool;

/**
  Event triggered by the timer to check if any cards have been removed
  or if new ones have been plugged in
**/

EFI_EVENT gCheckCardsEvent;

/**
  Initialize the MMC Host Pool to support multiple MMC devices
**/
VOID
InitializeMmcHostPool (
  VOID
  )
{
  InitializeListHead (&mMmcHostPool);
}

/**
  Insert a new Mmc Host controller to the pool.

  @param  MmcHostInstance  The MMC_HOST_INSTANCE to be inserted into the pool.

**/
VOID
InsertMmcHost (
  IN MMC_HOST_INSTANCE      *MmcHostInstance
  )
{
  InsertTailList (&mMmcHostPool, &(MmcHostInstance->Link));
}

/**
  Remove a new Mmc Host controller to the pool.

  @param  MmcHostInstance  The MMC_HOST_INSTANCE to be removed from the pool.

**/
VOID
RemoveMmcHost (
  IN MMC_HOST_INSTANCE      *MmcHostInstance
  )
{
  RemoveEntryList (&(MmcHostInstance->Link));
}

/**
  This function creates a new MMC host controller instance and initializes its members.
  It allocates memory for the instance, sets the necessary fields,
  and installs the BlockIO and DevicePath protocols.

  @param   MmcHost  The EFI_MMC_HOST_PROTOCOL instance representing the MMC host.

  @return  A pointer to the created MMC_HOST_INSTANCE on success, or NULL on failure.
**/
MMC_HOST_INSTANCE*
CreateMmcHostInstance (
  IN EFI_MMC_HOST_PROTOCOL* MmcHost
  )
{
  EFI_STATUS          Status;
  MMC_HOST_INSTANCE*  MmcHostInstance;
  EFI_DEVICE_PATH_PROTOCOL    *NewDevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  MmcHostInstance = AllocateZeroPool (sizeof (MMC_HOST_INSTANCE));
  if (MmcHostInstance == NULL) {
    return NULL;
  }

  MmcHostInstance->Signature = MMC_HOST_INSTANCE_SIGNATURE;

  MmcHostInstance->State = MmcHwInitializationState;

  MmcHostInstance->BlockIo.Media = AllocateCopyPool (sizeof (EFI_BLOCK_IO_MEDIA), &mMmcMediaTemplate);
  if (MmcHostInstance->BlockIo.Media == NULL) {
    goto FREE_INSTANCE;
  }

  MmcHostInstance->BlockIo.Revision = EFI_BLOCK_IO_INTERFACE_REVISION;
  MmcHostInstance->BlockIo.Reset = MmcReset;
  MmcHostInstance->BlockIo.ReadBlocks = MmcReadBlocks;
  MmcHostInstance->BlockIo.WriteBlocks = MmcWriteBlocks;
  MmcHostInstance->BlockIo.FlushBlocks = MmcFlushBlocks;

  MmcHostInstance->MmcHost = MmcHost;

  // Create DevicePath for the new MMC Host
  Status = MmcHost->BuildDevicePath (MmcHost, &NewDevicePathNode);
  if (EFI_ERROR (Status)) {
    goto FREE_MEDIA;
  }

  DevicePath = (EFI_DEVICE_PATH_PROTOCOL*)AllocatePool (END_DEVICE_PATH_LENGTH);
  if (DevicePath == NULL) {
    goto FREE_MEDIA;
  }

  SetDevicePathEndNode (DevicePath);
  MmcHostInstance->DevicePath = AppendDevicePathNode (DevicePath, NewDevicePathNode);

  // Publish BlockIO protocol interface
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &MmcHostInstance->MmcHandle,
                  &gEfiBlockIoProtocolGuid, &MmcHostInstance->BlockIo,
                  &gEfiDevicePathProtocolGuid, MmcHostInstance->DevicePath,
                  NULL
                );
  if (EFI_ERROR (Status)) {
    goto FREE_DEVICE_PATH;
  }

  return MmcHostInstance;

FREE_DEVICE_PATH:
  FreePool (DevicePath);

FREE_MEDIA:
  FreePool (MmcHostInstance->BlockIo.Media);

FREE_INSTANCE:
  FreePool (MmcHostInstance);

  return NULL;
}

/**
  This function uninstalls the BlockIO and DevicePath protocols from the MMC host controller instance,
  and frees the memory allocated for the instance and its associated resources.

  @param   MmcHostInstance  The MMC_HOST_INSTANCE to be destroyed.

  @retval  EFI_SUCCESS      The instance is successfully destroyed.
  @retval  Other            The instance cannot be destroyed.

**/
EFI_STATUS
DestroyMmcHostInstance (
  IN MMC_HOST_INSTANCE* MmcHostInstance
  )
{
  EFI_STATUS Status;

  // Uninstall Protocol Interfaces
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  MmcHostInstance->MmcHandle,
                  &gEfiBlockIoProtocolGuid, &(MmcHostInstance->BlockIo),
                  &gEfiDevicePathProtocolGuid, MmcHostInstance->DevicePath,
                  NULL
                );
  ASSERT_EFI_ERROR (Status);

  // Free Memory allocated for the instance
  if (MmcHostInstance->BlockIo.Media) {
    FreePool (MmcHostInstance->BlockIo.Media);
  }
  if (MmcHostInstance->CardInfo.ECSDData) {
    FreePages (MmcHostInstance->CardInfo.ECSDData, EFI_SIZE_TO_PAGES (sizeof (ECSD)));
  }
  FreePool (MmcHostInstance);

  return Status;
}

/**
  This function checks if the controller implement the Mmc Host and the Device Path Protocols.

  @param  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller           The handle of the controller to check for support.
  @param  RemainingDevicePath  A pointer to the remaining portion of the device path.

  @retval EFI_SUCCESS          The controller is supported.
  @retval EFI_UNSUPPORTED      The controller is unsupported.
**/
EFI_STATUS
EFIAPI
MmcDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                      Status;
  //EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_MMC_HOST_PROTOCOL           *MmcHost;
  EFI_DEV_PATH_PTR                Node;

  //
  // Check RemainingDevicePath validation
  //
  if (RemainingDevicePath != NULL) {
    //
    // Check if RemainingDevicePath is the End of Device Path Node,
    // if yes, go on checking other conditions
    //
    if (!IsDevicePathEnd (RemainingDevicePath)) {
      //
      // If RemainingDevicePath isn't the End of Device Path Node,
      // check its validation
      //
      Node.DevPath = RemainingDevicePath;
      if (Node.DevPath->Type != HARDWARE_DEVICE_PATH ||
          Node.DevPath->SubType != HW_VENDOR_DP ||
          DevicePathNodeLength (Node.DevPath) != sizeof (VENDOR_DEVICE_PATH)) {
        return EFI_UNSUPPORTED;
      }
    }
  }

  //
  // Check if Mmc Host protocol is installed by platform
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gSophgoMmcHostProtocolGuid,
                  (VOID**)&MmcHost,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the Mmc Host used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,
         &gSophgoMmcHostProtocolGuid,
         This->DriverBindingHandle,
         Controller
       );

  return EFI_SUCCESS;
}

/**
  This function opens the Mmc Host Protocol, creates an MMC_HOST_INSTANCE, and adds it to the MMC host pool.

  @param  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller           The handle of the controller to start the driver on.
  @param  RemainingDevicePath  A pointer to the remaining portion of the device path.

  @retval  EFI_SUCCESS         The driver is successfully started.
  @retval  Other               The driver failed to start.

**/
EFI_STATUS
EFIAPI
MmcDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS              Status;
  MMC_HOST_INSTANCE       *MmcHostInstance;
  EFI_MMC_HOST_PROTOCOL   *MmcHost;

  //
  // Check RemainingDevicePath validation
  //
  if (RemainingDevicePath != NULL) {
    //
    // Check if RemainingDevicePath is the End of Device Path Node,
    // if yes, return EFI_SUCCESS
    //
    if (IsDevicePathEnd (RemainingDevicePath)) {
      return EFI_SUCCESS;
    }
  }

  //
  // Get the Mmc Host protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gSophgoMmcHostProtocolGuid,
                  (VOID**)&MmcHost,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_ALREADY_STARTED) {
      return EFI_SUCCESS;
    }
    return Status;
  }

  MmcHostInstance = CreateMmcHostInstance (MmcHost);

  if (MmcHostInstance != NULL) {
    // Add the handle to the pool
    InsertMmcHost (MmcHostInstance);

    MmcHostInstance->Initialized = FALSE;

    // Detect card presence now
    CheckCardsCallback (NULL, NULL);
  }

  return EFI_SUCCESS;
}

/**
  This function closes the Mmc Host Protocol, removes the MMC_HOST_INSTANCE from the pool, and destroys the instance.

  @param  This               A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param  Controller         The handle of the controller to stop the driver on.
  @param  NumberOfChildren   The number of children handles.
  @param  ChildHandleBuffer  An array of child handles.

  @retval  EFI_SUCCESS       The driver is successfully stopped.
  @retval  Other             The driver failed to stop.

**/
EFI_STATUS
EFIAPI
MmcDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
{
  EFI_STATUS          Status = EFI_SUCCESS;
  LIST_ENTRY          *CurrentLink;
  MMC_HOST_INSTANCE   *MmcHostInstance;

  MMC_TRACE ("MmcDriverBindingStop()");

  // For each MMC instance
  CurrentLink = mMmcHostPool.ForwardLink;
  while (CurrentLink != NULL && CurrentLink != &mMmcHostPool && (Status == EFI_SUCCESS)) {
    MmcHostInstance = MMC_HOST_INSTANCE_FROM_LINK (CurrentLink);
    ASSERT (MmcHostInstance != NULL);

    // Close gSophgoMmcHostProtocolGuid
    Status = gBS->CloseProtocol (
                    Controller,
                    &gSophgoMmcHostProtocolGuid,
                    (VOID**)&MmcHostInstance->MmcHost,
                    This->DriverBindingHandle
                  );

    // Remove MMC Host Instance from the pool
    RemoveMmcHost (MmcHostInstance);

    // Destroy MmcHostInstance
    DestroyMmcHostInstance (MmcHostInstance);
  }

  return Status;
}

/**
  Callback function to check MMC cards.

  @param[in] Event    The event that is being triggered
  @param[in] Context  The context passed to the event

**/
VOID
EFIAPI
CheckCardsCallback (
  IN  EFI_EVENT   Event,
  IN  VOID        *Context
  )
{
  LIST_ENTRY          *CurrentLink;
  MMC_HOST_INSTANCE   *MmcHostInstance;
  EFI_STATUS          Status;

  CurrentLink = mMmcHostPool.ForwardLink;
  while (CurrentLink != NULL && CurrentLink != &mMmcHostPool) {
    MmcHostInstance = MMC_HOST_INSTANCE_FROM_LINK (CurrentLink);
    ASSERT (MmcHostInstance != NULL);

    if (MmcHostInstance->MmcHost->IsCardPresent (MmcHostInstance->MmcHost) == !MmcHostInstance->Initialized) {
      MmcHostInstance->State = MmcHwInitializationState;
      MmcHostInstance->BlockIo.Media->MediaPresent = !MmcHostInstance->Initialized;
      MmcHostInstance->Initialized = !MmcHostInstance->Initialized;

      if (MmcHostInstance->BlockIo.Media->MediaPresent) {
        InitializeMmcDevice (MmcHostInstance);
        // if (EFI_ERROR (Status)) {
        //   DEBUG ((DEBUG_ERROR, "CheckCardsCallback: Error InitializeMmcDevice, Status=%r.\n", Status));
        //   MmcHostInstance->Initialized = !MmcHostInstance->Initialized;
        //   DEBUG ((DEBUG_ERROR, "%a[%d] MmcHostInstance->Initialized=%r\n", __func__, __LINE__, Status));
        //   return Status;
        // }
      }

      Status = gBS->ReinstallProtocolInterface (
                      (MmcHostInstance->MmcHandle),
                      &gEfiBlockIoProtocolGuid,
                      &(MmcHostInstance->BlockIo),
                      &(MmcHostInstance->BlockIo)
                    );

      if (EFI_ERROR (Status)) {
        Print (L"MMC Card: Error reinstalling BlockIo interface\n");
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }
}


EFI_DRIVER_BINDING_PROTOCOL gMmcDriverBinding = {
  MmcDriverBindingSupported,
  MmcDriverBindingStart,
  MmcDriverBindingStop,
  0xa,
  NULL,
  NULL
};

/**
  This function is the entry point of the MMC DXE driver.
  It initializes the MMC host pool, installs driver model protocols,
  driver diagnostics, and sets up a timer for card detection.

  @param  ImageHandle  The image handle of the driver.
  @param  SystemTable  A pointer to the EFI system table.

  @retval  EFI_SUCCESS       The driver is successfully initialized.
  @retval  Other             The driver failed to initialize.

**/
EFI_STATUS
EFIAPI
MmcDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Initializes MMC Host pool
  //
  InitializeMmcHostPool ();

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gMmcDriverBinding,
             ImageHandle,
             &gMmcComponentName,
             &gMmcComponentName2
           );
  ASSERT_EFI_ERROR (Status);

  // Install driver diagnostics
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiDriverDiagnostics2ProtocolGuid,
                  &gMmcDriverDiagnostics2,
                  NULL
                );
  ASSERT_EFI_ERROR (Status);

  // Use a timer to detect if a card has been plugged in or removed
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL | EVT_TIMER,
                  TPL_CALLBACK,
                  CheckCardsCallback,
                  NULL,
                  &gCheckCardsEvent
                );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->SetTimer (gCheckCardsEvent,
                  TimerPeriodic,
                  (UINT64)(10 * 1000 * 200)); // 200 ms
  ASSERT_EFI_ERROR (Status);

  return Status;
}
