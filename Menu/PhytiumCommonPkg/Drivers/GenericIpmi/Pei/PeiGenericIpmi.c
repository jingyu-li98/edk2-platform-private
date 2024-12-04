/** @file
Generic IPMI stack during PEI phase.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Ipmi.h>
#include <IpmiCommandLib.h>
#include <IpmiPhyStandard.h>

#include <Library/BmcBaseLib.h>
#include <Library/ReportStatusCodeLib.h>

#include <Ppi/IpmiTransportPpi.h>
#include <Protocol/IpmiInteractiveProtocol.h>

#include "PeiGenericIpmi.h"

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
  DEBUG ((DEBUG_INFO, "[IPMI PEI] BMC Device ID: 0x%02X, firmware version: %d.%02X \n",
          BmcContxt->DeviceId, BmcContxt->MajorFmRevision, BmcContxt->MinorFmRevision));
  return Status;
}

/**
  The entry point of the Ipmi PEIM. Instals Ipmi PPI interface.

  @param[in]  FileHandle  Handle of the file being invoked.
  @param[in]  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS   Indicates that Ipmi initialization completed successfully.

**/
EFI_STATUS
PeimIpmiInterfaceInit (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                  Status;
  IPMI_BMC_INFO               BmcInfo;
  PEI_IPMI_BMC_INSTANCE_DATA  *mIpmiInstance;
  mIpmiInstance = NULL;

  //check bmc in place or not
  if (CheckBmcInPlace () != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Bmc is not in place, it is not need to install IPMI protocol!\n"));
    return EFI_SUCCESS;
  }

  mIpmiInstance = AllocateZeroPool (sizeof (PEI_IPMI_BMC_INSTANCE_DATA));
  if (mIpmiInstance == NULL) {
    DEBUG ((DEBUG_ERROR,"IPMI Peim:EFI_OUT_OF_RESOURCES of memory allocation\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  mIpmiInstance->KcsTimeoutPeriod = (BMC_KCS_TIMEOUT_PEI *1000*1000) / KCS_DELAY_UNIT_PEI;
  DEBUG ((DEBUG_INFO,"IPMI Peim:KcsTimeoutPeriod = 0x%x\n", mIpmiInstance->KcsTimeoutPeriod));

  //
  // Initialize IPMI IO Base.
  //
  mIpmiInstance->IpmiIoBase                         = PcdGet16 (PcdIpmiIoBaseAddress) + FixedPcdGet64 (PcdKcsBaseAddress);
  DEBUG ((DEBUG_INFO,"IPMI Peim:IpmiIoBase=0x%x\n",mIpmiInstance->IpmiIoBase));
  mIpmiInstance->Signature                          = SM_IPMI_BMC_SIGNATURE;
  mIpmiInstance->BmcStatus                          = BMC_NOTREADY;
  mIpmiInstance->IpmiTransportPpi.IpmiSubmitCommand = PeiIpmiCommandExecuteToBmc;
  mIpmiInstance->IpmiTransportPpi.GetBmcStatus      = PeiGetIpmiBmcStatus;

  mIpmiInstance->PeiIpmiBmcDataDesc.Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  mIpmiInstance->PeiIpmiBmcDataDesc.Guid  = &gPeiIpmiTransportPpiGuid;
  mIpmiInstance->PeiIpmiBmcDataDesc.Ppi   = &mIpmiInstance->IpmiTransportPpi;

  //
  // Just produce PPI
  //
  Status = PeiServicesInstallPpi (&mIpmiInstance->PeiIpmiBmcDataDesc);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get the Device ID.
  //
  Status = GetBmcDeviceId (&BmcInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"IPMI Peim:Get BMC Device Id Failed. Status=%r\n",Status));
  }

  if (mIpmiInstance->BmcStatus == BMC_UPDATE_IN_PROGRESS || mIpmiInstance->BmcStatus == BMC_HARDFAIL) {
    return EFI_UNSUPPORTED;
  }
    if (BmcInfo.DeviceId != 0) {
        mIpmiInstance->BmcStatus = BMC_OK;
      return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}


/**
  Updates the  Bmc Status.

  @param[in]   This         Pointer to IPMI protocol instance.
  @param[out]  BmcStatus    BMC status.

  @retval  EFI_SUCCESS      Return Successly.
  @retval  Other            Failure.

**/
EFI_STATUS
PeiGetIpmiBmcStatus (
  IN      PEI_IPMI_TRANSPORT_PPI  *This,
  OUT     BMC_STATUS              *BmcStatus
  )
{
  PEI_IPMI_BMC_INSTANCE_DATA  *IpmiInstance;

  IpmiInstance = INSTANCE_FROM_PEI_SM_IPMI_BMC_THIS (This);

  if ((IpmiInstance->BmcStatus == BMC_SOFTFAIL) && (IpmiInstance->SoftErrorCount >= MAX_SOFT_COUNT)) {
    IpmiInstance->BmcStatus = BMC_HARDFAIL;
  }

  *BmcStatus = IpmiInstance->BmcStatus;
  return EFI_SUCCESS;
}
