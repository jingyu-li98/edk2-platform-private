/** @file

  Copyright (c) 2011 - 2019, Intel Corporaton. All rights reserved.
  Copyright (c) 2020, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include "DwMac4SnpDxe.h"

#include <Library/DmaLib.h>
#include <Library/NetLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

STATIC
EFI_STATUS
EFIAPI
DriverSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
DriverStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
DriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  );

STATIC
EFI_DRIVER_BINDING_PROTOCOL mDriverBinding = {
  DriverSupported,
  DriverStart,
  DriverStop,
  0xa,
  NULL,
  NULL
};

STATIC
SOPHGO_SIMPLE_NETWORK_DEVICE_PATH PathTemplate = {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_MAC_ADDR_DP,
      {
        (UINT8)(sizeof (MAC_ADDR_DEVICE_PATH)),
        (UINT8)((sizeof (MAC_ADDR_DEVICE_PATH)) >> 8)
      }
    },
    {
      {
        0
      }
    },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

STATIC
EFI_STATUS
EFIAPI
DriverSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  NON_DISCOVERABLE_DEVICE    *Dev;
  EFI_STATUS                 Status;

  //
  // Connect to the non-discoverable device
  //
  Status = gBS->OpenProtocol (Controller,
                              &gEdkiiNonDiscoverableDeviceProtocolGuid,
                              (VOID **)&Dev,
                              This->DriverBindingHandle,
                              Controller,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Clean up.
  //
  gBS->CloseProtocol (Controller,
                      &gEdkiiNonDiscoverableDeviceProtocolGuid,
                      This->DriverBindingHandle,
                      Controller);

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
DriverStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  EFI_STATUS                        Status;
  SOPHGO_SIMPLE_NETWORK_DRIVER      *Snp;
  EFI_SIMPLE_NETWORK_MODE           *SnpMode;
  SOPHGO_SIMPLE_NETWORK_DEVICE_PATH *DevicePath;
  UINT64                            DefaultMacAddress;
  EFI_MAC_ADDRESS                   *SwapMacAddressPtr;
  UINTN                             DescriptorSize;
  UINTN                             BufferSize;
  UINTN                             *RxBufferAddr;
  EFI_PHYSICAL_ADDRESS              RxBufferAddrMap;
  UINT32                            Index;

  //
  // Allocate Resources
  //
  Snp = AllocatePages (EFI_SIZE_TO_PAGES (sizeof (SOPHGO_SIMPLE_NETWORK_DRIVER)));
  if (Snp == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->OpenProtocol (Controller,
                              &gEdkiiNonDiscoverableDeviceProtocolGuid,
                              (VOID **)&Snp->Dev,
                              This->DriverBindingHandle,
                              Controller,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);

  DEBUG ((DEBUG_INFO, "%a[%d]: Enter success, Status=%r\n", __func__, __LINE__, Status));
  //
  // Size for descriptor
  //
  DescriptorSize = EFI_PAGES_TO_SIZE (sizeof (DMA_DESCRIPTOR));
  DEBUG ((DEBUG_INFO, "%a[%d]: DescriptorSize=0x%x\n", __func__, __LINE__, Status));

  //
  // Size for transmit and receive buffer
  //
  BufferSize = ETH_BUFFER_SIZE;

  for (Index = 0; Index < TX_DESC_NUM; Index++) {
    //
    // DMA TxDescRing allocate buffer and map
    //
    Status = DmaAllocateBuffer (EfiBootServicesData,
                                EFI_SIZE_TO_PAGES (sizeof (DMA_DESCRIPTOR)),
	                        (VOID *)&Snp->MacDriver.TxDescRing[Index]
				);
    if (EFI_ERROR (Status)) {
      DEBUG ((
	DEBUG_ERROR,
        "%a () for TxDescRing: %r\n",
	__func__,
	Status
	));
      return Status;
    }

    DEBUG ((DEBUG_INFO, "%a[%d]: DmaTxDescRing-Allocate: Status=%r\n", __func__, __LINE__, Status));
    Status = DmaMap (MapOperationBusMasterCommonBuffer,
		     Snp->MacDriver.TxDescRing[Index],
                     &DescriptorSize,
	             &Snp->MacDriver.TxDescRingMap[Index].AddrMap,
	             &Snp->MacDriver.TxDescRingMap[Index].Mapping
	             );
    if (EFI_ERROR (Status)) {
      DEBUG ((
	DEBUG_ERROR,
        "%a () for TxDescRing: %r\n",
	__func__,
	Status
	));
      return Status;
    }
    DEBUG ((DEBUG_INFO, "%a[%d]: DmaTxDescRing-Map: Status=%r\n", __func__, __LINE__, Status));

    //
    // DMA RxDescRing allocte buffer and map
    //
    Status = DmaAllocateBuffer (EfiBootServicesData,
                                EFI_SIZE_TO_PAGES (sizeof (DMA_DESCRIPTOR)),
				(VOID *)&Snp->MacDriver.RxDescRing[Index]
				);
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
	"%a () for RxDescRing: %r\n",
	__func__,
	Status
	));
      return Status;
    }
    DEBUG ((DEBUG_INFO, "%a[%d]: DmaRxDescRing-Allocate: Status=%r\n", __func__, __LINE__, Status));

    Status = DmaMap (MapOperationBusMasterCommonBuffer,
		     Snp->MacDriver.RxDescRing[Index],
                     &DescriptorSize,
		     &Snp->MacDriver.RxDescRingMap[Index].AddrMap,
		     &Snp->MacDriver.RxDescRingMap[Index].Mapping
		     );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
	"%a () for RxDescRing: %r\n",
	 __func__,
	 Status
	 ));
      return Status;
    }

    DEBUG ((DEBUG_INFO, "%a[%d]: DmaRxDescRing-Map: Status=%r\n", __func__, __LINE__, Status));
    //
    // DMA mapping for receive buffer
    //
    RxBufferAddr = (UINTN*)((UINTN)Snp->MacDriver.RxBuffer + (Index * BufferSize));
    Status = DmaMap (MapOperationBusMasterWrite,
		     (VOID *) RxBufferAddr,
                     &BufferSize,
		     &RxBufferAddrMap,
		     &Snp->MacDriver.RxBufNum[Index].Mapping
		     );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a () for Rxbuffer: %r\n",
	__func__,
	Status
	));
      return Status;
    }
    Snp->MacDriver.RxBufNum[Index].AddrMap= RxBufferAddrMap;
  }
    DEBUG ((DEBUG_INFO, "%a[%d]: DmaRxDescRing-Map: Status=%r\n", __func__, __LINE__, Status));

  DevicePath = (SOPHGO_SIMPLE_NETWORK_DEVICE_PATH*)AllocateCopyPool (sizeof (SOPHGO_SIMPLE_NETWORK_DEVICE_PATH), &PathTemplate);
  if (DevicePath == NULL) {

    DEBUG ((
      DEBUG_ERROR,
      "%a () for DeivcePath is NULL!\n",
      __func__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialized signature (used by INSTANCE_FROM_SNP_THIS macro)
  //
  Snp->Signature = SNP_DRIVER_SIGNATURE;

  EfiInitializeLock (&Snp->Lock, TPL_CALLBACK);

  //
  // Initialize pointers
  //
  SnpMode = &Snp->SnpMode;
  Snp->Snp.Mode = SnpMode;

  //
  // Get MAC controller base address
  //
  Snp->MacBase = (UINTN)Snp->Dev->Resources[0].AddrRangeMin;

  //
  // Assign fields and func pointers
  //
  Snp->Snp.Revision       = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Snp->Snp.WaitForPacket  = NULL;
  Snp->Snp.Initialize     = SnpInitialize;
  Snp->Snp.Start          = SnpStart;
  Snp->Snp.Stop           = SnpStop;
  Snp->Snp.Reset          = SnpReset;
  Snp->Snp.Shutdown       = SnpShutdown;
  Snp->Snp.ReceiveFilters = SnpReceiveFilters;
  Snp->Snp.StationAddress = SnpStationAddress;
  Snp->Snp.Statistics     = SnpStatistics;
  Snp->Snp.MCastIpToMac   = SnpMcastIptoMac;
  Snp->Snp.NvData         = SnpNvData;
  Snp->Snp.GetStatus      = SnpGetStatus;
  Snp->Snp.Transmit       = SnpTransmit;
  Snp->Snp.Receive        = SnpReceive;

  Snp->RecycledTxBuf = AllocatePool (sizeof (UINT64) * SNP_TX_BUFFER_INCREASE);
  if (Snp->RecycledTxBuf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  }

  Snp->MaxRecycledTxBuf = SNP_TX_BUFFER_INCREASE;
  Snp->RecycledTxBufCount = 0;

  //
  // Start completing simple network mode structure
  //
  SnpMode->State = EfiSimpleNetworkStopped;
  SnpMode->HwAddressSize = NET_ETHER_ADDR_LEN;    // HW address is 6 bytes
  SnpMode->MediaHeaderSize = sizeof (ETHER_HEAD);
  SnpMode->MaxPacketSize = EFI_PAGE_SIZE;         // Preamble + SOF + Ether Frame (with VLAN tag +4bytes)
  SnpMode->NvRamSize = 0;                         // No NVRAM with this device
  SnpMode->NvRamAccessSize = 0;                   // No NVRAM with this device

  //
  // Update network mode information
  //
  SnpMode->ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST     |
                               EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST   |
                               EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST   |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  //
  // We do not intend to receive anything for the time being.
  //
  SnpMode->ReceiveFilterSetting = 0;

  //
  // EMAC has 64bit hash table, can filter 64 MCast MAC Addresses
  //
  SnpMode->MaxMCastFilterCount = MAX_MCAST_FILTER_CNT;
  SnpMode->MCastFilterCount = 0;
  ZeroMem (&SnpMode->MCastFilter, MAX_MCAST_FILTER_CNT * sizeof (EFI_MAC_ADDRESS));

  //
  // Set the interface type (1: Ethernet or 6: IEEE 802 Networks)
  //
  SnpMode->IfType = NET_IFTYPE_ETHERNET;

  //
  // Mac address is changeable as it is loaded from erasable memory
  //
  SnpMode->MacAddressChangeable = TRUE;

  //
  // Can only transmit one packet at a time
  //
  SnpMode->MultipleTxSupported = FALSE;

  //
  // MediaPresent checks for cable connection and partner link
  //
  SnpMode->MediaPresentSupported = TRUE;
  SnpMode->MediaPresent = FALSE;

  //
  // Set broadcast address
  //
  SetMem (&SnpMode->BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  //
  // Set current address
  //
  DefaultMacAddress = Snp->Dev->Resources[1].AddrRangeMin;

  //
  // Swap PCD human readable form to correct endianess
  //
  SwapMacAddressPtr = (EFI_MAC_ADDRESS *) &DefaultMacAddress;
  SnpMode->CurrentAddress.Addr[0] = SwapMacAddressPtr->Addr[5];
  SnpMode->CurrentAddress.Addr[1] = SwapMacAddressPtr->Addr[4];
  SnpMode->CurrentAddress.Addr[2] = SwapMacAddressPtr->Addr[3];
  SnpMode->CurrentAddress.Addr[3] = SwapMacAddressPtr->Addr[2];
  SnpMode->CurrentAddress.Addr[4] = SwapMacAddressPtr->Addr[1];
  SnpMode->CurrentAddress.Addr[5] = SwapMacAddressPtr->Addr[0];

  //
  // Assign fields for device path
  //
  CopyMem (&DevicePath->MacAddrDP.MacAddress, &Snp->Snp.Mode->CurrentAddress, NET_ETHER_ADDR_LEN);
  DevicePath->MacAddrDP.IfType = Snp->Snp.Mode->IfType;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleNetworkProtocolGuid, &(Snp->Snp),
                  &gEfiDevicePathProtocolGuid, DevicePath,
                  NULL
                  );

  if (EFI_ERROR(Status)) {
    gBS->CloseProtocol (Controller,
                        &gEdkiiNonDiscoverableDeviceProtocolGuid,
                        This->DriverBindingHandle,
                        Controller);

    FreePages (Snp, EFI_SIZE_TO_PAGES (sizeof (SOPHGO_SIMPLE_NETWORK_DRIVER)));
  } else {
    Snp->ControllerHandle = Controller;
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
DriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  )
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_NETWORK_PROTOCOL  *SnpProtocol;
  SOPHGO_SIMPLE_NETWORK_DRIVER *Snp;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID **)&SnpProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a (): HandleProtocol: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  Snp = INSTANCE_FROM_SNP_THIS(SnpProtocol);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &Snp->Snp,
                  NULL
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a (): UninstallMultipleProtocolInterfaces: %r\n",
      __func__,
      Status
      ));
    return Status;
  }

  FreePool (Snp->RecycledTxBuf);
  FreePages (Snp, EFI_SIZE_TO_PAGES (sizeof (SOPHGO_SIMPLE_NETWORK_DRIVER)));

  return Status;
}


/**
   UEFI Driver Entry Point API
   @param  ImageHandle       EFI_HANDLE.
   @param  SystemTable       EFI_SYSTEM_TABLE.
   @return EFI_SUCCESS       Success.
   EFI_DEVICE_ERROR  Fail.
**/
EFI_STATUS
EFIAPI
DwMac4SnpDxeEntry (
  IN  EFI_HANDLE       ImageHandle,
  IN  EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &mDriverBinding,
             ImageHandle,
             &gSnpComponentName,
             &gSnpComponentName2
             );

  DEBUG ((DEBUG_INFO, "Enter %a success, Status=%r\n", __func__, Status));
  return Status;
}
