/** @file
Ipmi interface initialize driver.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <DxeIpmiInit.h>
#include <IpmiCommandLib.h>

#include <Library/BmcBaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/IpmiInteractiveProtocol.h>

/**
  Get Bmc Id information.

  @param[out]  BmcInfo  A pointer to Bmc Information.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
EFIAPI
GetDeviceId (
  OUT IPMI_BMC_INFO  *BmcInfo
  )
{
  EFI_STATUS  Status;
  UINT32      Retries;

  Retries = RETRY_TICKS;
  //
  // Get the device ID information for the BMC.
  //
  do {
    Status = IpmiGetDeviceId (BmcInfo);
    if (!EFI_ERROR(Status)) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay(1000);
  } while (Retries--);

  return Status;
}

/**
  Fill Ipmi Context Structure Information.

  @param[out]  IpmiInfo  A pointer to Ipmi Information.

**/
VOID
InitialIpmiStructure (
  OUT  IPMI_ATTRIBUTE_DATA  *IpmiInfo
  )
{

  IpmiInfo->BmcFirmwareVersion = 0xFFFF;
  IpmiInfo->IpmiVersion = IPMI_VERSION_DEFAULT;
  IpmiInfo->BmcStatus = BmcStatusUnknow;
  IpmiInfo->ProtocolHandle = NULL;
  IpmiInfo->IpmiBaseAddress = FixedPcdGet16 (PcdIpmiIoBaseAddress) + FixedPcdGet64 (PcdKcsBaseAddress);
  IpmiInfo->IpmiBaseAddressOffset = FixedPcdGet16 (PcdIpmiIoRegOffset);

  IpmiInfo->mIpmiProtocol.IpmiPhySubmitCommand = IpmiCommandExecuteToBmc;
  IpmiInfo->mIpmiProtocol.IpmiInterfaceType = IpmiGetInterfaceType;
}

/**
  The entry point of the Ipmi Interface Init.

  @param[in]  ImageHandle      Handle of this driver image.
  @param[in]  SystemTable      Table containing standard EFI services.

  @retval EFI_SUCCESS          Always success is returned even if KCS does not function.
  @retval Other                Failure.

**/
EFI_STATUS
EFIAPI
IpmiInterfaceInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                 Status;
  IPMI_ATTRIBUTE_DATA        *IpmiInfo;
  IPMI_BMC_INFO              BmcContent;

  //check bmc in place or not
  if (CheckBmcInPlace () != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Bmc is not in place, it is not need to install IPMI protocol!\n"));
    return EFI_SUCCESS;
  }

  IpmiInfo = (IPMI_ATTRIBUTE_DATA*)AllocateZeroPool (sizeof(IPMI_ATTRIBUTE_DATA));
  if (IpmiInfo == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Initalize the KCS transaction timeout.
  //
  IpmiInfo->TotalTimeTicks = (FixedPcdGet8 (PcdIpmiExecuteTime) * 1000 *1000) / KCS_DELAY_UNIT;

  InitialIpmiStructure(IpmiInfo);

  Status = gBS->InstallProtocolInterface(
                 &IpmiInfo->ProtocolHandle,
                 &gIpmiTransportProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &IpmiInfo->mIpmiProtocol
                 );
  if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Install Protocol InterfaceFailed\n", Status));
      return Status;
  }

  //
  // Get the Device ID and Check SELF TEST Results.
  //
  Status = GetDeviceId (&BmcContent);
  if (!EFI_ERROR (Status)) {
    DEBUG((DEBUG_INFO,"Get Bmc ID 0x%x \n",BmcContent.DeviceId));
  } else if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR,"Faild Get Bmc ID\n"));
  }

  return EFI_SUCCESS;
}

