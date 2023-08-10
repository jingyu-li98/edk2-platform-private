/** @file

 Copyright (c) 2023, SOPHGO Inc. All rights reserved.

 SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Protocol/PciPlatform.h>
#include <Protocol/PciIo.h>
#include <Protocol/FirmwareVolume2.h>
#include <IndustryStandard/Pci22.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/MemoryAllocationLib.h>

//
// Global variables for Option ROMs
//
#define NULL_ROM_FILE_GUID \
  { \
    0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} \
  }

#define SAS_OPTION_ROM_FILE_GUID \
{ 0xb47533c7, 0xcc78, 0x4e5e, {0x94, 0x33, 0xf2, 0x8b, 0x16, 0xcd, 0x66, 0xdb}}

#define SAS3108_OPTION_ROM_FILE_GUID \
{ 0xb47533c8, 0xcc78, 0x4e5e, {0x94, 0x33, 0xf2, 0x8b, 0x16, 0xcd, 0x66, 0xd8}}

#define INVALID                     0xBD


typedef struct {
  EFI_HANDLE                      PciPlatformHandle;
  EFI_PCI_PLATFORM_PROTOCOL       PciPlatform;
} PCI_PLATFORM_PRIVATE_DATA;


#define MAX_ROM_NUMBER  2


typedef struct {
  EFI_GUID  FileName;
  UINTN     Segment;
  UINTN     Bus;
  UINTN     Device;
  UINTN     Function;
  UINT16    VendorId;
  UINT16    DeviceId;
} PCI_OPTION_ROM_TABLE;

typedef struct {
  UINTN                   RomSize;
  VOID                    *RomBase;
} OPTION_ROM_INFO;

PCI_PLATFORM_PRIVATE_DATA *mPciPrivateData = NULL;

PCI_OPTION_ROM_TABLE      mPciOptionRomTable[] = {
  {
    SAS_OPTION_ROM_FILE_GUID,
    0,
    2,
    0,
    0,
    0x1000,
    0x0097
  },
  {
    SAS3108_OPTION_ROM_FILE_GUID,
    0,
    1,
    0,
    0,
    0x1000,
    0x005D
  },

    //
    // End of OptionROM Entries
    //
  {
    NULL_ROM_FILE_GUID, // Guid
    0,                  // Segment
    0,                  // Bus Number
    0,                  // Device Number
    0,                  // Function Number
    0xffff,             // Vendor ID
    0xffff              // Device ID
  }
};

/**

   Perform Platform initialization first in PciPlatform.

   Arguments:

   Returns:

    VOID.

**/
VOID
EFIAPI
PciInitPlatform (
  VOID
  );

/**

   Perform Platform initialization by the phase indicated.

   @param HostBridge    The associated PCI host bridge handle.
   @param Phase         The phase of the PCI controller enumeration.
   @param ChipsetPhase  Defines the execution phase of the PCI chipset driver.

   Returns:

**/
VOID
EFIAPI
PhaseNotifyPlatform (
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  );

/**

  Set the PciPolicy as EFI_RESERVE_ISA_IO_NO_ALIAS | EFI_RESERVE_VGA_IO_NO_ALIAS.

  @param This       The pointer to the Protocol itself.
  @param PciPolicy  The returned Policy.

  @retval EFI_UNSUPPORTED        Function not supported.
  @retval EFI_INVALID_PARAMETER  Invalid PciPolicy value.

**/
EFI_STATUS
EFIAPI
GetPlatformPolicy (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL    *This,
  OUT EFI_PCI_PLATFORM_POLICY           *PciPolicy
  )
{
  if (PciPolicy == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_UNSUPPORTED;
}

/**
  Get an indicated image in raw sections.

  @param NameGuid  NameGuid of the image to get
  @param Buffer    Buffer to store the image get
  @param Size      Size of the image get

  @retval EFI_NOT_FOUND   Could not find the image
  @retval EFI_LOAD_ERROR  Error occurred during image loading
  @retval EFI_SUCCESS     Image has been successfully loaded

**/
EFI_STATUS
GetRawImage (
  IN EFI_GUID   *NameGuid,
  IN OUT VOID   **Buffer,
  IN OUT UINTN  *Size
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  UINT32                        AuthenticationStatus;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }

  //
  // Find desired image in all Fvs
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **)&Fv
                    );:

    if (EFI_ERROR (Status)) {
      return EFI_LOAD_ERROR;
    }

    //
    // Try a raw file
    //
    *Buffer = NULL;
    *Size   = 0;
    Status = Fv->ReadSection (
                  Fv,
                  NameGuid,
                  EFI_SECTION_RAW,
                  0,
                  Buffer,
                  Size,
                  &AuthenticationStatus
                  );

    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  if (Index >= HandleCount) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
  GetPciRom from platform specific location for sepcific PCI device

  @param This       Protocol instance pointer
  @param PciHandle  Identify the specific PCI device
  @param RomImage   the ROM Image memory location
  @param RomSize    Rom Image size in bytes

  @retval EFI_SUCCESS    RomImage is valid
  @retval EFI_NOT_FOUND  No RomImage

**/
EFI_STATUS
EFIAPI
GetPciRom (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL    *This,
  IN  EFI_HANDLE                        PciHandle,
  OUT VOID                              **RomImage,
  OUT UINTN                             *RomSize
  )
{
  EFI_STATUS                        Status;
  EFI_PCI_IO_PROTOCOL               *PciIo;
  UINTN                             Segment;
  UINTN                             Bus;
  UINTN                             Device;
  UINTN                             Function;
  UINT16                            VendorId;
  UINT16                            DeviceId;
  UINT16                            DeviceClass;
  UINTN                             TableIndex;
  UINTN                             RomImageNumber;
  OPTION_ROM_INFO                   OptionRominfo[MAX_ROM_NUMBER];

  Status = gBS->HandleProtocol (
                  PciHandle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  (VOID)PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_VENDOR_ID_OFFSET, 1, &VendorId);

  (VOID)PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &DeviceId);

  (VOID)PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, PCI_SUBCLASSCODE_OFFSET, 1, &DeviceClass);

  (VOID)PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);

  //
  // Loop through table of video option rom descriptions
  //
  RomImageNumber = 0;
  for (TableIndex = 0; mPciOptionRomTable[TableIndex].VendorId != 0xffff; TableIndex++) {
    //
    // See if the PCI device specified by PciHandle matches at device in mPciOptionRomTable
    //
    if (VendorId != mPciOptionRomTable[TableIndex].VendorId || 
		DeviceId != mPciOptionRomTable[TableIndex].DeviceId) {
        continue;
    }

    Status = GetRawImage (
              &mPciOptionRomTable[TableIndex].FileName,
              &(OptionRominfo[RomImageNumber].RomBase),
              &(OptionRominfo[RomImageNumber].RomSize)
              );

    if (EFI_ERROR (Status)) {
      continue;
    } else {
      RomImageNumber++;
      if (RomImageNumber == MAX_ROM_NUMBER) {
        break;
      }
    }
  }

  if (RomImageNumber == 0) {
    return EFI_NOT_FOUND;
  } else {
    *RomImage = OptionRominfo[RomImageNumber - 1].RomBase;
    *RomSize  = OptionRominfo[RomImageNumber - 1].RomSize;

    if (RomImageNumber > 1) {
      //
      // More than one OPROM have been found!
      //

    }

    return EFI_SUCCESS;
  }
}

/**

  The PlatformPrepController() function can be used to notify the platform driver so that
  it can perform platform-specific actions. No specific actions are required.
  Several notification points are defined at this time. More synchronization points may be
  added as required in the future. The PCI bus driver calls the platform driver twice for
  every PCI controller-once before the PCI Host Bridge Resource Allocation Protocol driver
  is notified, and once after the PCI Host Bridge Resource Allocation Protocol driver has
  been notified.
  This member function may not perform any error checking on the input parameters. It also
  does not return any error codes. If this member function detects any error condition, it
  needs to handle those errors on its own because there is no way to surface any errors to
  the caller.

  @param This          Pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param HostBridge    The associated PCI host bridge handle.
  @param RootBridge    The associated PCI root bridge handle.
  @param PciAddress    The address of the PCI device on the PCI bus.
  @param Phase         The phase of the PCI controller enumeration.
  @param ChipsetPhase  Defines the execution phase of the PCI chipset driver.

  @retval EFI_SUCCESS     The function completed successfully.
  @retval EFI_UNSUPPORTED Not supported.

**/
EFI_STATUS
EFIAPI
PlatformPrepController (
  IN  EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_HANDLE                                     RootBridge,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  )
{

  return EFI_SUCCESS;
}

/**

  Perform initialization by the phase indicated.

  @param This          Pointer to the EFI_PCI_PLATFORM_PROTOCOL instance.
  @param HostBridge    The associated PCI host bridge handle.
  @param Phase         The phase of the PCI controller enumeration.
  @param ChipsetPhase  Defines the execution phase of the PCI chipset driver.

  @retval EFI_SUCCESS  Must return with success.

**/
EFI_STATUS
EFIAPI
PhaseNotify (
  IN EFI_PCI_PLATFORM_PROTOCOL                       *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  )
{
  PhaseNotifyPlatform (HostBridge, Phase, ChipsetPhase);

  return EFI_SUCCESS;
}

/**

  Main Entry point of the Pci Platform Driver.

  @param ImageHandle  Handle to the image.
  @param SystemTable  Handle to System Table.

  @retval EFI_STATUS  Status of the function calling.

**/
EFI_STATUS
PciPlatformDriverEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  PCI_PLATFORM_PRIVATE_DATA *PciPrivateData;

  PciInitPlatform ();

  PciPrivateData = AllocateZeroPool (sizeof (PCI_PLATFORM_PRIVATE_DATA));
  mPciPrivateData = PciPrivateData;

  mPciPrivateData->PciPlatform.PlatformNotify             = PhaseNotify;
  mPciPrivateData->PciPlatform.PlatformPrepController     = PlatformPrepController;
  mPciPrivateData->PciPlatform.GetPlatformPolicy          = GetPlatformPolicy;
  mPciPrivateData->PciPlatform.GetPciRom                  = GetPciRom;

  //
  // Install on a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPciPrivateData->PciPlatformHandle,
                  &gEfiPciPlatformProtocolGuid,
                  &mPciPrivateData->PciPlatform,
                  NULL
                  );

  return Status;
}
