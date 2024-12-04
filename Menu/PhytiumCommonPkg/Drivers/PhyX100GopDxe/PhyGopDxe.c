/* @file
** PhyGopDxe.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
*/

#include "PhyGopDxe.h"
//#include "PhyGraphicsOutput.h"

/**
  Read the register of MMD(dc,dcreq,dp,vram).
  Register address mapping in every group:
    dc     : 0x0000 - 0x1fff
    dcreq  : 0x2000 - 0x2fff
    dp     : 0x3000 - 0x3fff
    vram   : 0x4000 - 0x4fff
  There are three group:
    group 0 : 0x0000 - 0x7fff
    group 1 : 0x8000 - 0xffff
    group 2 : 0x100000 - 0x17fff
  Actual register address = num * 0x8000 + Offset

  @param[in]  Private      Pointer to private data structure.

  @param[in]  num          the index of dcdp , 0 , 1 or 2.

  @param[in]  Offset       Register address offset.

  @retval     The value of register.
**/
UINT32
PhyReadMmio (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINTN             Offset
  )
{
  UINT32 Data;
  EFI_STATUS Status;
  Status = Private->PciIo->Mem.Read (
                                 Private->PciIo,
                                 EfiPciIoWidthUint32,
                                 PCI_BAR_MMIO,        //Point to Bar 0(MMIO base address),
                                 Num * 0x8000 + Offset,
                                 1,
                                 &Data
                                 );
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO,"read reg : %08x, data : %08x\n", Num * 0x8000 + Offset, Data));
#endif
 ASSERT_EFI_ERROR (Status);
 return Data;
}

/**
  Set the register of MMD(dc,dcreq,dp,vram).
  Register address mapping in every group:
    dc     : 0x0000 - 0x1fff
    dcreq  : 0x2000 - 0x2fff
    dp     : 0x3000 - 0x3fff
    vram   : 0x4000 - 0x4fff
  There are three group:
    group 0 : 0x0000 - 0x7fff
    group 1 : 0x8000 - 0xffff
    group 2 : 0x100000 - 0x17fff
  Actual register address = num * 0x8000 + Offset

  @param[in]  Private      Pointer to private data structure.

  @param[in]  num          the index of dcdp , 0 , 1 or 2.

  @param[in]  Offset       Register address offset.

  @param[in]  Data         The value that you want to write to the register in a specified group.

  @retval     NULL
**/
VOID
PhyWriteMmio (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINTN            Offset,
  IN UINT32           Data
  )
{
  EFI_STATUS Status;
  Status = Private->PciIo->Mem.Write (
                                 Private->PciIo,
                                 EfiPciIoWidthUint32,
                                 PCI_BAR_MMIO,      //Point to Bar 0(MMIO base address)
                                 Num * 0x8000 + Offset,
                                 1,
                                 &Data
                                 );
#ifdef REG_DEBUG
  DEBUG ((DEBUG_INFO, "write reg : %08x, data : %08x\n", Num * 0x8000 + Offset, Data));
#endif
  gBS->Stall (10);
 ASSERT_EFI_ERROR (Status);
}

/**
  Write video buffer data to video memory.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the selected BAR to start video memory.

  @param[in]  len          The number of data to write in byte unit.

  @param[in]  buffer       Pointer to the source buffer to write data from.

  @retval     NULL
**/
VOID
PhyFramebufferWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT64            Offset,
  IN UINTN             Len,
  IN VOID              *Buffer
  )
{
#ifndef MAIN_RAM_USE
  EFI_STATUS Status;
  UINT32 Index;
  Status = Private->PciIo->Mem.Write (
                                 Private->PciIo,
                                 EfiPciIoWidthUint32,
                                 PCI_BAR_FRAMEBUFFER,     //Point to Bar 1(FRAMEBUFFER base address)
                                 Offset,
                                 Len,
                                 Buffer
                                 );
  ASSERT_EFI_ERROR (Status);
  for (Index = 0; Index < Len; Index++) {
    MmioWrite32 (Private->ShadowBuffer + Offset + Index * 4, *((UINT32 *) (Buffer + Index * 4)));
  }
#else
  UINT32 Index;
  for (Index = 0; Index < Len; Index++) {
    MmioWrite32 (Private->BufferAddr + Offset + Index * 4, *((UINT32 *) (Buffer + Index * 4)));
  }
#endif
}

/**
  Read video buffer data from video memory.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the selected BAR to start video memory.

  @param[in]  Len          The number of data to read in byte unit.

  @param[in]  Buffer       Pointer to the destination buffer to store the results.

  @retval     NULL
**/
VOID
PhyFramebufferRead (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT64            Offset,
  IN  UINTN             Len,
  OUT VOID              *Buffer
  )
{
#ifndef MAIN_RAM_USE
  //EFI_STATUS Status;
  //Status = Private->PciIo->Mem.Read (
  //                               Private->PciIo,
  //                               EfiPciIoWidthUint32,
  //                               PCI_BAR_FRAMEBUFFER,     //Point to Bar 2(FRAMEBUFFER base address)
  //                               Offset,
  //                               Len,
  //                               Buffer
  //                               );
  //ASSERT_EFI_ERROR(Status);
  UINT32 Index;
  for (Index = 0; Index < Len; Index++) {
    *((UINT32 *) (Buffer + Index * 4)) = MmioRead32 (Private->ShadowBuffer + Offset + Index * 4);
  }
#else
  UINT32 Index;
  for (Index = 0; Index < Len; Index++) {
    *((UINT32 *) (Buffer + Index * 4)) = MmioRead32 (Private->BufferAddr + Offset + Index * 4);
  }
#endif
}

/**
  Read PCI controller registers in PCI configuration space.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the PCI configuration space for the PCI controller.

  @retval     The result that get from PCI configuration space
**/
UINT32
PhyPciRead (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32            Offset
  )
{
  UINT32    Pci;
  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Private->PciIo->Pci.Read (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        Offset,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  return Pci;
}

/**
  Write PCI controller registers in PCI configuration space.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Offset       The offset within the PCI configuration space for the PCI controller.

  @param[in]  Pci          The data to write.

  @retval EFI_SUCCESS           Write successfully.

  @retval EFI_UNSUPPORTED       The address range specified by Offset, Width, and Count is not
                                valid for the PCI configuration header of the PCI controller.

  @retval EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack of resources.

  @retval EFI_INVALID_PARAMETER Buffer is NULL or Width is invalid.
**/
EFI_STATUS
PhyPciWrite (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32            Offset,
  IN UINT32            Pci
  )
{
  EFI_STATUS Status;
  //
  // Write the PCI Configuration Header from the PCI Device
  //
  Status = Private->PciIo->Pci.Write (
                                 Private->PciIo,
                                 EfiPciIoWidthUint32,
                                 Offset,
                                 sizeof (Pci) / sizeof (UINT32),
                                 &Pci
);
  return Status;
}

EFI_DRIVER_BINDING_PROTOCOL gPhyDriverBinding = {
  PhyControllerDriverSupported,
  PhyControllerDriverStart,
  PhyControllerDriverStop,
  0x10,     //Driver version
  NULL,
  NULL
};

/**
  Check if this device is supported.

  @param  This                   The driver binding protocol.

  @param  Controller             The controller handle to check.

  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.

  @retval EFI_UNSUPPORTED        This device isn't supported.
**/
EFI_STATUS
EFIAPI
PhyControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;

  //DEBUG ((DEBUG_INFO, "PhyControllerDriverSupported enter -\n"));
  //
  // Open the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    //DEBUG ((DEBUG_ERROR, "OpenProtocol gEfiPciIoProtocolGuid Error- %r\n", Status));
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //DEBUG ((DEBUG_INFO, "VendorId : %04x,DeviceId : %04x,ClassCode 0 : %02x,ClassCode 1 : %02x,ClassCode 2 : %02x\n",
    //Pci.Hdr.VendorId,Pci.Hdr.DeviceId,Pci.Hdr.ClassCode[0],Pci.Hdr.ClassCode[1],Pci.Hdr.ClassCode[2]));
  Status = EFI_UNSUPPORTED;
  //
  // If there was a need to check for PCI I/O Enable, this is the
  // place to do so.  We'll assume the  is the only card in the
  // system.
  //
  // See if this is a gp PCI controller
  //
  if ((Pci.Hdr.VendorId == VENDOR_ID_PHY) && (Pci.Hdr.DeviceId == DEVICE_ID_PHY) && (Pci.Hdr.ClassCode[2] == 0x03)) {
    Status = EFI_SUCCESS;
  }
  else {
    Status = EFI_UNSUPPORTED;
  }
  Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  return Status;
}

EFI_STATUS
EFIAPI
PhyControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  EFI_STATUS                    Status;
  PHY_PRIVATE_DATA              *Private;


  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the Graphics Output Protocol interface does not exist the driver is not started
    //
    return Status;
  }

  //
  // Get our private context information
  //
  Private = PHY_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);

  //
  // Remove the Graphics Output Protocol interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UninstallMultipleProtocolInterfaces gEfiGraphicsOutputProtocolGuid,%r\n", Status));
    return Status;
  }

  //
  // Shutdown the hardware
  //
  PhyGraphicsOutputDestructor (Private);

  Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationDisable,
                    EFI_PCI_DEVICE_ENABLE,
                    NULL
                    );

  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  //
  // Free our instance data
  //
  gBS->FreePool (Private);
  return EFI_SUCCESS;
}

/**
  Start to process the controller.

  @param  This                   The USB bus driver binding instance.

  @param  Controller             The controller to check.

  @param  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.

  @retval EFI_ALREADY_STARTED    The controller is already controlled by the usb
                                 bus.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.
**/
EFI_STATUS
EFIAPI
PhyControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  PHY_PRIVATE_DATA            *Private;
  BOOLEAN                     PciAttributesSaved;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
  ACPI_ADR_DEVICE_PATH        AcpiDeviceNode;

  PciAttributesSaved = FALSE;

  //
  // Allocate Private context data for GOP inteface.
  //
  Private = NULL;
  Private = AllocateZeroPool(sizeof (PHY_PRIVATE_DATA));
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG((DEBUG_ERROR, "Allocate poll failed -\n"));
    goto Error;
  }

  //
  // Set up context record
  //
  Private->Signature  = PHY_PRIVATE_DATA_SIGNATURE;
  Private->Handle     = NULL;

  //
  // Open PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &Private->PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "OpenProtocol gEfiPciIoProtocolGuid failed -\n"));
    goto Error;
  }
  //
  // Save original PCI attributes
  //
  Status = Private->PciIo->Attributes (
                             Private->PciIo,
                             EfiPciIoAttributeOperationGet,
                             0,
                             &Private->OriginalPciAttributes
                             );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "pciio Attributes EfiPciIoAttributeOperationGet failed -\n"));
    goto Error;
  }
  PciAttributesSaved = TRUE;
  Status = Private->PciIo->Attributes (
                             Private->PciIo,
                             EfiPciIoAttributeOperationEnable,
                             EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                             NULL
                             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "pciio Attributes EfiPciIoAttributeOperationEnable failed -\n"));
    goto Error;
  }
  //
  // Get ParentDevicePath
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HandleProtocol gEfiDevicePathProtocolGuid failed\n"));
    goto Error;
  }
  //
  // Set Gop Device Path
  //
  if (RemainingDevicePath == NULL) {
    ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
    AcpiDeviceNode.Header.Type = ACPI_DEVICE_PATH;
    AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
    AcpiDeviceNode.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
    SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));

    Private->GopDevicePath = AppendDevicePathNode (
                               ParentDevicePath,
                               (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDeviceNode
                               );
  }
  else if (!IsDevicePathEnd (RemainingDevicePath)) {
    //
    // If RemainingDevicePath isn't the End of Device Path Node,
    // only scan the specified device by RemainingDevicePath
    //
    Private->GopDevicePath = AppendDevicePathNode (ParentDevicePath, RemainingDevicePath);
  }
  else {
    //
    // If RemainingDevicePath is the End of Device Path Node,
    // don't create child device and return EFI_SUCCESS
    //
    Private->GopDevicePath = NULL;
  }
  if (Private->GopDevicePath != NULL) {
    //
    // Creat child handle and device path protocol firstly
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Private->Handle,
                    &gEfiDevicePathProtocolGuid,
                    Private->GopDevicePath,
                    NULL
                    );
    //
    // Start the Graphics Output Protocol software stack.
    //
    Status = PhyGraphicsOutputConstructor (Private);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "PhyControllerDriverStart Status %r\n", Status));
      goto Error;
    }

    //
    // Publish the Graphics Output Protocol interface to the world
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Private->Handle,
                    &gEfiGraphicsOutputProtocolGuid,
                    &Private->GraphicsOutput,
                    NULL
                    );
    //
    // Open the Parent Handle for the child
    //
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &Private->PciIo,
                    This->DriverBindingHandle,
                    Private->Handle,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "OpenProtocol gEfiPciIoProtocolGuid,%r\n",Status));
      goto Error;
    }
  } else {
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );
    Status = EFI_SUCCESS;
  }
Error:
  if (EFI_ERROR (Status)) {
    if (Private) {
      if (Private->PciIo) {
        if (PciAttributesSaved == TRUE) {
          //
          // Restore original PCI attributes
          //
          Private->PciIo->Attributes (
                            Private->PciIo,
                            EfiPciIoAttributeOperationSet,
                            Private->OriginalPciAttributes,
                            NULL
                            );
        }
        //
        // Close the PCI I/O Protocol
        //
        gBS->CloseProtocol (
               Private->Handle,
               &gEfiPciIoProtocolGuid,
               This->DriverBindingHandle,
               Private->Handle
               );
      }
      gBS->FreePool (Private);
    }
  }
  return Status;
}

/**
  The entry point for PhyGop driver which installs the driver binding and component
  name protocol on its ImageHandle.

  @param[in]  ImageHandle  The image handle of the driver.

  @param[in]  SystemTable  The system table.

  @retval EFI_SUCCES       The driver binding and component name protocols are
                           successfully installed.
  @retval Others           Other errors as indicated.
**/
EFI_STATUS
EFIAPI
PhyGraphicsOutputDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS      Status;
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gPhyDriverBinding,
             ImageHandle,
             &gPhyComponentName,
             NULL
             );
  ASSERT_EFI_ERROR (Status);
  return Status;
}
