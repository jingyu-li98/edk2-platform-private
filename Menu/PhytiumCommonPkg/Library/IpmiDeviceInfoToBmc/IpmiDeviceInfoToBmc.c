/** @file
Device Information Library class interfaces.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <ArmPlatform.h>
#include <IndustryStandard/Pci22.h>
#include <IpmiCommandLib.h>
#include <IpmiPhyStandard.h>
#include <Library/IpmiDeviceInfoToBmc.h>
#include <Library/ArmLib.h>
#include <Library/CrcLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <malloc.h>

#include <Protocol/Cpu.h>
#include <Protocol/IpmiInteractiveProtocol.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Uefi.h>


/**
  Generate Smbios String Length.

  @param[in] Smbios  Pointer the Smbios Type.

  @retval  NumLen  The Length of Smibos String Length.

**/
UINT16
GetSmbiosStringLength (
  IN  UINT8  *Smbios
  )
{
  UINT8  NumLen;
  UINT8  *String;

  NumLen = 2;
  String = Smbios;
  for( ; *String != 0 || (*(String+1) != 0); String++) {
    NumLen++;
  }
  return NumLen;
}

/**
  Send Smbios Information.

  @retval  EFI_SUCCESS   The data is sending completed.
  @retval  Other         Failure.

**/
EFI_STATUS
IpmiSubmitSmbiosInfo (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINT8                                 ResponseData[50];
  UINT8                                 ResponseSize;
  UINT8                                 OpenResponseData[10];
  UINT8                                 DataSize;
  UINT16                                SessionId;
  UINT8                                 *Tsmbios;
  UINT16                                StringLength;
  UINT8                                 *BlobType;
  UINT16                                NoCrcLen;
  UINT16                                TotalLen;
  EFI_SMBIOS_HANDLE                     SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER               *Record;
  EFI_SMBIOS_PROTOCOL                   *Smbios;
  IPMI_SOC_BMC_END_BLOB                 BmcEndBlob;
  IPMI_WRITE_TYPE_DATA_BLOB             WriteBlobTypeTx;

  UINT8  Blob[] = {0xcf,0xc2,0,0};
  TotalLen = 0;
  ResponseSize = 0;
  Status = EFI_SUCCESS;
  //
  //  Data {0xcf,0xc2,0} is BMC OEM Blob Command.Next Data {2} represent Open Blob subcommands.
  //  And Data {0xcb,0xa8} is Crc16 value.Data {2,0} is Bmc Blob open flags.
  //  Data {47,115,109,98,105,111,115,0} represent '/smbios' strings.
  //
  UINT8  BmcOpenBlob[] = {0xcf,0xc2,0,2,0xcb,0xa8,2,0,47,115,109,98,105,111,115,0};

  //
  // Step 1  Enumer Blob.
  //
  DataSize = sizeof(Blob);
  Status = IpmiSubmitCommand (
             EFI_SMBIOS_NETFN_INFO,
             EFI_IPMI_SMBIOS_INFO,
             (UINT8 *)&Blob,
             DataSize,
             ResponseData,
             &ResponseSize
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Step 2  Open Blob.
  //
  DataSize = sizeof(BmcOpenBlob);
  Status = IpmiSubmitCommand (
             EFI_SMBIOS_NETFN_INFO,
             EFI_IPMI_SMBIOS_INFO,
             (UINT8 *)&BmcOpenBlob,
             DataSize,
             OpenResponseData,
             &ResponseSize
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Session Id.
  //
  SessionId = (OpenResponseData[7] << 8 ) + OpenResponseData[6];

  //
  // Step 3  Write Blob.
  //
  WriteBlobTypeTx.Blob[0] = 0xcf;
  WriteBlobTypeTx.Blob[1] = 0xc2;
  WriteBlobTypeTx.Blob[2] = 0;
  WriteBlobTypeTx.Blob[3] = 4;         //Write Blob Subcommands.

  WriteBlobTypeTx.Crc16 = 0;
  WriteBlobTypeTx.SsionId = SessionId; //open result SessionId.
  WriteBlobTypeTx.Offset = 0;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);
  ASSERT_EFI_ERROR (Status);

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (!EFI_ERROR(Status)) {
    Tsmbios = (UINT8 *)Record + Record->Length;
    StringLength = GetSmbiosStringLength (Tsmbios);

    TotalLen = Record->Length + StringLength;
    CopyMem (WriteBlobTypeTx.Data, (VOID*)Record, TotalLen);
    NoCrcLen = TotalLen + 6;         // Blob + Crc16.
    WriteBlobTypeTx.Crc16 = GenerateCrc16 ((VOID*)&WriteBlobTypeTx.SsionId, NoCrcLen, 0xffff);

    DataSize = TotalLen + 12;
    Status = IpmiSubmitCommand (
               EFI_SMBIOS_NETFN_INFO,
               EFI_IPMI_SMBIOS_INFO,
               (UINT8 *)&WriteBlobTypeTx,
               DataSize,
               ResponseData,
               &ResponseSize
               );
    if (EFI_ERROR(Status)) {
      return Status;
    }
    WriteBlobTypeTx.Offset += TotalLen;
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
  }

  //
  // Step 4  Commit Blob.
  //
  BmcEndBlob.OemNumber[0] = 0xcf;
  BmcEndBlob.OemNumber[1] = 0xc2;
  BmcEndBlob.OemNumber[2] = 0;
  BmcEndBlob.Command = 5;    //Commit Blob Subcommands.
  BmcEndBlob.SsionId = SessionId;
  BmcEndBlob.Data = 0;

  BlobType = (UINT8 *)&BmcEndBlob + 6;
  // TotalLen = 3;
  BmcEndBlob.Crc16 = GenerateCrc16 (BlobType, 3, 0xffff);
  DataSize = sizeof (IPMI_SOC_BMC_END_BLOB);
  Status = IpmiSubmitCommand (
             EFI_SMBIOS_NETFN_INFO,
             EFI_IPMI_SMBIOS_INFO,
             (UINT8 *)&BmcEndBlob,
             DataSize,
             ResponseData,
             &ResponseSize
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Step 5  Close Blob.
  //
  BmcEndBlob.Command = 6;    //Close Blob Subcommands.
  DataSize = sizeof(IPMI_SOC_BMC_END_BLOB);
  Status = IpmiSubmitCommand (
             EFI_SMBIOS_NETFN_INFO,
             EFI_IPMI_SMBIOS_INFO,
             (UINT8 *)&BmcEndBlob,
             DataSize,
             ResponseData,
             &ResponseSize
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


/**
  Send PCIe Device Information.

  @retval  EFI_SUCCESS   The data is sending completed.
  @retval  Other         Failure.

**/
EFI_STATUS
IpmiSubmitPcieDeviceInfo (
  VOID
  )
{
  EFI_STATUS            Status;
  UINT8                 Index;
  UINTN                 HandleCount;
  UINTN                 Segment;
  UINTN                 Bus;
  UINTN                 Device;
  UINTN                 Function;
  UINT16                VendorId;
  UINT8                 ClassCode;
  UINT16                DeviceId;
  UINT16                SubsystemVendorID;
  UINT16                SubsystemID;
  UINT8                 SubClass;
  UINT8                 BaseClass;
  UINT8                 ProgrammingIF;
  UINT8                 ResponseData;
  UINT8                 ResponseSize;
  UINT8                 DataSize;
  UINT8                 Count;
  UINT8                 MacAddr[6];
  EFI_HANDLE            *HandleBuffer;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  IPMI_SET_PCIE_DEVICE_INFO  PcidevInfo;

  Status = EFI_SUCCESS;
  ZeroMem (&MacAddr, sizeof(MacAddr));
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_VENDOR_ID_OFFSET, 1, &VendorId);

    DataSize = sizeof (IPMI_SET_PCIE_DEVICE_INFO);
    ZeroMem (&PcidevInfo, DataSize);
    PcidevInfo.Segment = Segment;
    PcidevInfo.BusNumber = Bus;
    PcidevInfo.DeviceNumber = Device;
    PcidevInfo.FunctionNumber = Function;

    PcidevInfo.VendorID = VendorId;

    //
    // The PCIE Slot Index information is customized by the actual machine used,
    // and currently all PCIE device information will be provided and the PCIE
    // Slot Index as 0xFF,indicating unknow.
    //

   PcidevInfo.PCIeSlotIndex = 0xFF;
   PcidevInfo.PCIeDeviePresent = 0x1;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x0b, 1, &ClassCode);
   if (ClassCode == PCI_CLASS_NETWORK) {
     PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_VENDOR_ID_OFFSET, 1, &VendorId);
     if (VendorId == 0x8086) {
       Status = PciIo->Attributes (PciIo, EfiPciIoAttributeOperationEnable, EFI_PCI_IO_ATTRIBUTE_MEMORY, NULL);
       PciIo->Mem.Read (PciIo, EfiPciIoWidthUint8, 0, 0x40, 6, &MacAddr);
       for (Count = 0; Count < 6; Count++) {
         PcidevInfo.MACAddress[Count] = MacAddr[Count];
       }
     }
   }
   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x09, 1, &ProgrammingIF);
   PcidevInfo.ProgrammingIF = ProgrammingIF;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x0a, 1, &SubClass);
   PcidevInfo.SubClass = SubClass;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x0b, 1, &BaseClass);
   PcidevInfo.BaseClass = BaseClass;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &DeviceId);
   PcidevInfo.DeviceID = DeviceId;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, 1, &SubsystemVendorID);
   PcidevInfo.SubVendorID = SubsystemVendorID;

   PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_SUBSYSTEM_ID_OFFSET, 1, &SubsystemID);
   PcidevInfo.SubDeviceID = SubsystemID;

   Status = IpmiSubmitCommand (
              EFI_PHY_NETFN_INFO,
              EFI_IPMI_PCIE_INFO,
              (UINT8 *)&PcidevInfo,
              DataSize,
              &ResponseData,
              &ResponseSize
              );
   if (EFI_ERROR(Status)) {
     return Status;
   }
  }

  if (HandleBuffer!=NULL) {
    FreePool(HandleBuffer);
  }

  return EFI_SUCCESS;
}

/**
  Exchagnge data space.

  @param[in]  Data    The pointer to Data.
  @param[in]  Size    The Data of size.

**/
VOID
SwapEntries (
  IN CHAR8     *Data,
  IN UINT16    Size
  )
{
  UINT16     Index;
  CHAR8      Temp8;

  for (Index = 0; (Index + 1) < Size; Index += 2) {
    Temp8 = Data[Index];
    Data[Index] = Data[Index+1];
    Data[Index + 1] = Temp8;
  }
}

/**
  Eliminate extra spaces.

  @param[in]  Str    The pointer to Data.

**/
VOID
EliminateExtraSpaces (
  IN CHAR8 *Str
  )
{
  UINTN    Index;
  UINTN    ActualIndex;

  for (Index = 0,ActualIndex = 0; Str[Index] != '\0'; Index++) {
    if ((Str[Index] !='\0') || ((ActualIndex >0) && (Str[ActualIndex -1] != '\0'))) {
      Str[ActualIndex++] = Str[Index];
    }
  }
  if (Str[ActualIndex -1] == '\0') {
    ActualIndex--;
  }
  Str[ActualIndex] = '\0';
}

/**
  Send Hard Disk Device Information.

  @retval  EFI_SUCCESS   The data is sending completed.
  @retval  Other         Failure.

**/
EFI_STATUS
IpmiSubmitHDDeviceInfo (
  VOID
  )
{
  EFI_STATUS                               Status;
  UINTN                                    Index;
  UINTN                                    HandleCount;
  EFI_HANDLE                               *HandleBuffer;
  UINT32                                   SataPortIndex;
  UINT32                                   IdeChannel;
  CHAR8                                    *ModelNumber;
  UINT64                                   NumSectors;
  UINT64                                   RemainderInBytes;
  UINT32                                   DriveSizeInGB;
  UINT64                                   NameSpaceSizeInBytes;
  UINT64                                   DriveSizeInBytes;
  UINT32                                   BufferSize;
  UINT8                                    ResponseData;
  UINT8                                    ResponseSize;
  UINT16                                   DataSize;
  EFI_HANDLE                               Handle;
  CHAR16                                   *Description;
  CHAR16                                   *Char;
  UINTN                                    Idx;
  ATA_IDENTIFY_DATA                        IdentifyData;
  IPMI_SET_HDD_DEVICE_INFO                 HddInfo;
  EFI_DISK_INFO_PROTOCOL                   *DiskInfo;
  EFI_DEV_PATH_PTR                         DevicePath;
  EFI_NVM_EXPRESS_COMMAND                  Command;
  EFI_NVM_EXPRESS_COMPLETION               Completion;
  NVME_ADMIN_CONTROLLER_DATA               ControllerData;
  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL       *NvmePassthru;
  EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET CommandPacket;

  ResponseSize  = 0;
  DriveSizeInGB = 0;
  HandleBuffer = NULL;
  NameSpaceSizeInBytes = 0;
  Status = EFI_SUCCESS;

  DataSize = sizeof (IPMI_SET_HDD_DEVICE_INFO);
  ZeroMem (&HddInfo, DataSize);

  ModelNumber = AllocateZeroPool(42);
  ASSERT (ModelNumber != NULL);
  ZeroMem (ModelNumber, 42);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDiskInfoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  for (Idx = 0; Idx < HandleCount; Idx++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Idx],
                    &gEfiDiskInfoProtocolGuid,
                    (VOID **)&DiskInfo
                    );
    ASSERT_EFI_ERROR(Status);

    if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid) ||
      CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid)) {
      Status = DiskInfo->WhichIde (
                           DiskInfo,
                           &IdeChannel,
                           &SataPortIndex
                           );
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR,"NO Disk!\n"));
        continue;
      }
      ASSERT_EFI_ERROR(Status);
      BufferSize = sizeof (ATA_IDENTIFY_DATA);
      Status = DiskInfo->Identify(
                           DiskInfo,
                           &IdentifyData,
                           &BufferSize
                           );
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR,"Identify failed!\n"));
        continue;
      }

      if (!EFI_ERROR(Status)) {
        CopyMem (ModelNumber, IdentifyData.ModelName, 40);
        SwapEntries (ModelNumber, 40);
        EliminateExtraSpaces (ModelNumber);
        if ((!(IdentifyData.config & 0x8000)) || (IdentifyData.config == 0x848A)) {
          if (IdentifyData.command_set_supported_83 & 0x0400) {
            NumSectors = *(UINT64 *)&IdentifyData.maximum_lba_for_48bit_addressing;
          } else {
            NumSectors = IdentifyData.user_addressable_sectors_lo + (IdentifyData.user_addressable_sectors_hi << 16);
          }
          DriveSizeInBytes = MultU64x32 (NumSectors, 512);
          DriveSizeInGB = (UINT32)DivU64x64Remainder (DriveSizeInBytes, 1000000000, &RemainderInBytes);
        } else {
          DriveSizeInGB = 0;
        }
        HddInfo.HddNumber = Idx;
        if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid) == 1) {
          HddInfo.HddType = 0x06;
        } else {
          HddInfo.HddType = 0x01;
        }
        CopyMem (HddInfo.HddSN, IdentifyData.SerialNo, 20);
        CopyMem (HddInfo.HddFWVersion, IdentifyData.FirmwareVer, 8);
        CopyMem (HddInfo.HddModelName, IdentifyData.ModelName, 40);
        HddInfo.HddSize = (UINT16)DriveSizeInGB;

        Status = IpmiSubmitCommand (
                   EFI_PHY_NETFN_INFO,
                   EFI_IPMI_HDD_INFO,
                   (UINT8 *)&HddInfo,
                   DataSize,
                   &ResponseData,
                   &ResponseSize
                   );
        if (EFI_ERROR(Status)) {
          return Status;
        }
      }
    } else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoScsiInterfaceGuid)) {
      //
      // To do
      //
    } else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid)) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Idx],
                      &gEfiDevicePathProtocolGuid,
                      (VOID **) &DevicePath.DevPath
                      );
      if (EFI_ERROR (Status)) {
        return EFI_LOAD_ERROR;
      }
      Status = gBS->LocateDevicePath (
                      &gEfiNvmExpressPassThruProtocolGuid,
                      &DevicePath.DevPath,
                      &Handle
                      );
      if (EFI_ERROR (Status) ||
        (DevicePathType (DevicePath.DevPath) != MESSAGING_DEVICE_PATH) ||
        (DevicePathSubType (DevicePath.DevPath) != MSG_NVME_NAMESPACE_DP)) {
        //
        // Do not return description when the Handle is not a child of NVME controller.
        //
        return Status;
      }

      //
      // Send ADMIN_IDENTIFY command to NVME controller to get the model and serial number.
      //
      Status = gBS->HandleProtocol (
                      Handle,
                      &gEfiNvmExpressPassThruProtocolGuid,
                      (VOID **) &NvmePassthru
                      );
      ASSERT_EFI_ERROR (Status);
      ZeroMem (&CommandPacket, sizeof(EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
      ZeroMem (&Command, sizeof(EFI_NVM_EXPRESS_COMMAND));
      ZeroMem (&Completion, sizeof(EFI_NVM_EXPRESS_COMPLETION));
      Command.Cdw0.Opcode = NVME_ADMIN_IDENTIFY_CMD;

      //
      // According to Nvm Express 1.1 spec Figure 38, When not used, the field shall be cleared to 0h.
      // For the Identify command, the Namespace Identifier is only used for the Namespace data structure.
      //
      Command.Nsid = 0;
      CommandPacket.NvmeCmd = &Command;
      CommandPacket.NvmeCompletion = &Completion;
      CommandPacket.TransferBuffer = &ControllerData;
      CommandPacket.TransferLength = sizeof (ControllerData);
      CommandPacket.CommandTimeout = EFI_TIMER_PERIOD_SECONDS (5);
      CommandPacket.QueueType = NVME_ADMIN_QUEUE;

      //
      // Set bit 0 (Cns bit) to 1 to identify a controller
      //
      Command.Cdw10 = 1;
      Command.Flags = CDW10_VALID;
      Status = NvmePassthru->PassThru (
                               NvmePassthru,
                               0,
                               &CommandPacket,
                               NULL
                               );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Description = AllocateZeroPool (
                      (ARRAY_SIZE (ControllerData.Mn) + 1
                      + ARRAY_SIZE (ControllerData.Sn) + 1
                      + MAXIMUM_VALUE_CHARACTERS + 1
                      ) * sizeof (CHAR16));
      if (Description != NULL) {
        Char = Description;
        for (Index = 0; Index < ARRAY_SIZE (ControllerData.Mn); Index++) {
          *(Char++) = (CHAR16)ControllerData.Mn[Index];
        }
          *(Char++) = L' ';
        for (Index = 0; Index < ARRAY_SIZE (ControllerData.Sn); Index++) {
          *(Char++) = (CHAR16)ControllerData.Sn[Index];
        }
          *(Char++) = L' ';
        UnicodeValueToStringS (
          Char, sizeof (CHAR16) * (MAXIMUM_VALUE_CHARACTERS + 1),
          0, DevicePath.NvmeNamespace->NamespaceId, 0
          );
        BmEliminateExtraSpaces (Description);
      }

      HddInfo.HddNumber = Idx;
      CopyMem (HddInfo.HddModelName, ControllerData.Mn, 40);
      CopyMem (HddInfo.HddSN, ControllerData.Sn, 20);
      CopyMem (HddInfo.HddFWVersion, ControllerData.Fr, 8);

      NameSpaceSizeInBytes = *((UINT64*)ControllerData.Tnvmcap);
      DriveSizeInGB = (UINT32)DivU64x64Remainder (NameSpaceSizeInBytes, 1000000000, &RemainderInBytes);
      HddInfo.HddSize = (UINT16)DriveSizeInGB;

      Status = IpmiSubmitCommand (
                 EFI_PHY_NETFN_INFO,
                 EFI_IPMI_HDD_INFO,
                 (UINT8 *)&HddInfo,
                 DataSize,
                 &ResponseData,
                 &ResponseSize
                 );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
  }

  gBS->FreePool (ModelNumber);

  return EFI_SUCCESS;
}

/**
  Send Ipmi Device Information.

  @retval  EFI_SUCCESS   The data is sending completed.
  @retval  Other         Failure.

**/
EFI_STATUS
IpmiDeviceInfoToBmc (
  VOID
  )
{
  EFI_STATUS            Status;
  IPMI_PHY_PROTOCOL     *mIpmiProtocol = NULL;

  Status = EFI_SUCCESS;
  Status = gBS->LocateProtocol (
                  &gIpmiTransportProtocolGuid,
                  NULL,
                  (VOID **) &mIpmiProtocol
                  );
  if (EFI_ERROR(Status)) {
     return Status;
  }

  Status = IpmiSubmitSmbiosInfo ();
  DEBUG((DEBUG_INFO,"IpmiSubmitSmbiosInfo : %r\n",Status));

  Status = IpmiSubmitPcieDeviceInfo ();
  DEBUG((DEBUG_INFO,"IpmiSubmitPcieDeviceInfo : %r\n",Status));

  Status = IpmiSubmitHDDeviceInfo ();
  DEBUG((DEBUG_INFO,"IpmiSubmitHDDeviceInfo : %r\n",Status));

  return Status;
}
