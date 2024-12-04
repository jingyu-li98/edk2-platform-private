/** @file
Generic MM IPMI stack driver.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "MmIpmiBmc.h"

IPMI_MM_ATTRIBUTE_DATA             *mIpmiInstance;
EFI_HANDLE                         mImageHandle;

/**
  Get Bmc Id information.

  @param[out]  BmcContxt  A pointer to Bmc Information.

  @retval  EFI_SUCCESS            Return Successly.
  @retval  Other                  Failure.

**/
EFI_STATUS
GetBmcDeviceId (
  OUT  IPMI_BMC_INFO    *BmcContxt
  )
{
  EFI_STATUS    Status;
  //
  // Get the device ID information for the BMC.
  //
  Status = IpmiGetDeviceId (BmcContxt);

  DEBUG ((DEBUG_INFO, "[IPMI MM] BMC Device ID: 0x%02X, firmware version: %d.%02X \n",
          BmcContxt->DeviceId, BmcContxt->MajorFmRevision, BmcContxt->MinorFmRevision));
  return Status;
}

/**
  The entry point of the MM Phase Ipmi Interface Init.

  @param[in]  ImageHandle      Handle of this driver image.
  @param[in]  MmSystemTable    Table containing standard EFI services.

  @retval EFI_SUCCESS          Successful driver initialization.
  @retval  Other               Failure.

**/
EFI_STATUS
EFIAPI
InitializeMmGenericIpmi (
  IN  EFI_HANDLE              ImageHandle,
  IN  EFI_MM_SYSTEM_TABLE     *MmSystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      Handle;
  IPMI_MM_ATTRIBUTE_DATA          *mIpmiInstance;
  IPMI_BMC_INFO                   BmcInfo;

  DEBUG ((DEBUG_INFO,"MmInitializeIpmiKcsPhysicalLayer entry \n"));
  //check bmc in place or not
  if (CheckBmcInPlace () != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Bmc is not in place, it is not need to install IPMI protocol!\n"));
    return EFI_SUCCESS;
  }

  mImageHandle = ImageHandle;
  mIpmiInstance = AllocateZeroPool (sizeof (IPMI_MM_ATTRIBUTE_DATA));

  if (mIpmiInstance == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR!! Null Pointer returned by AllocateZeroPool ()\n"));
    return EFI_OUT_OF_RESOURCES;
  } else {

    //
    // Initialize the KCS transaction timeout. Assume delay unit is 1000 us.
    //
    mIpmiInstance->TotalTimeTicks = (BMC_KCS_TIMEOUT * 1000*1000) / KCS_DELAY_UNIT_MM;

    //
    // Initialize IPMI IO Base.
    //
    mIpmiInstance->IpmiBaseAddress                  = FixedPcdGet16(PcdIpmiMmBaseAddress) + FixedPcdGet64 (PcdKcsBaseAddress);
    mIpmiInstance->Signature                        = IPMI_MM_ATTRIBUTE_DATA_SIGNATURE;
    mIpmiInstance->BmcStatus                        = BMC_NOTREADY;
    mIpmiInstance->mIpmiProtocol.IpmiPhySubmitCommand  = MmIpmiCommandExecuteToBmc;
    mIpmiInstance->mIpmiProtocol.GetBmcStatus          = MmIpmiGetBmcStatus;

    DEBUG ((DEBUG_INFO,"IPMI: Waiting for Getting BMC ID in MM \n"));

  //
  // Get the Device ID.
  //
  Status = GetBmcDeviceId (&BmcInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"IPMI Peim:Get BMC Device Id Failed. Status=%r\n",Status));
  }

  Handle = NULL;
  Status = gMmst->MmInstallProtocolInterface (
                      &Handle,
                      &gMmIpmiTransportProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      &mIpmiInstance->mIpmiProtocol
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"Install MM IpmiTransportocol Failed Status=%r\n",Status));
  }
  DEBUG ((DEBUG_INFO,"MmInitializeIpmiKcsPhysicalLayer exit \n"));

  return EFI_SUCCESS;
  }
}

