/** @file
  PCI Host Bridge Library instance for Sophgo SG2042

  Copyright (c) 2023, SOPHGO Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
// #include <Library/PcdLib.h>
#include <Library/PciHostBridgeLib.h>
// #include <Library/PlatformPciLib.h>

#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciRootBridgeIo.h>

GLOBAL_REMOVE_IF_UNREFERENCED
CHAR16 *mPciHostBridgeLibAcpiAddressSpaceTypeStr[] = {
  L"Mem", L"I/O", L"Bus"
};

#pragma pack(1)
typedef struct {
  ACPI_HID_DEVICE_PATH     AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

// STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath[] = {
//  // host bridge 0
//  {
//    {
//      {
//        ACPI_DEVICE_PATH,
//        ACPI_DP,
//        {
//          (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
//          (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
//        }
//      },
//      EISA_PNP_ID(0x0A08), // PCI Express
//      0 // AcpiDevicePath.UID
//    }, {
//      END_DEVICE_PATH_TYPE,
//      END_ENTIRE_DEVICE_PATH_SUBTYPE,
//      {
//        END_DEVICE_PATH_LENGTH,
//        0
//      }
//    }
//  },

//  // Host Bridge 1
//  {
//    {
//      {
//        ACPI_DEVICE_PATH,
//        ACPI_DP,
//        {
//          (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
//          (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
//        }
//      },
//      EISA_PNP_ID(0x0A08), // PCI Express
//      1 // AcpiDevicePath.UID
//    }, {
//      END_DEVICE_PATH_TYPE,
//      END_ENTIRE_DEVICE_PATH_SUBTYPE,
//      {
//        END_DEVICE_PATH_LENGTH,
//        0
//      }
//    }
//  },
STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath[] = {
 // Host Bridge 2
 {
   {
     {
       ACPI_DEVICE_PATH,
       ACPI_DP,
       {
         (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
         (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
       }
     },
     EISA_PNP_ID(0x0A08), // PCI Express
     0 // AcpiDevicePath.UID
   }, {
     END_DEVICE_PATH_TYPE,
     END_ENTIRE_DEVICE_PATH_SUBTYPE,
     {
       END_DEVICE_PATH_LENGTH,
       0
     }
   }
 }
};

//
// See description in MdeModulePkg/Include/Library/PciHostBridgeLib.h
// TODO: retrieve the PCI Express Base Address via a PCD entry
//
// STATIC PCI_ROOT_BRIDGE mRootBridges[] = {
//  // Host Bridge 0
//  {
//    0, // Segment
//    0, // Supports
//    0, // Attributes
//    FALSE, // DmaAbove4G
//    FALSE, // NoExtendedConfigSpace
//    FALSE, // ResourceAssigned
//    EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |
//    EFI_PCI_HOST_BRIDGE_MEM64_DECODE, // AllocationAttributes
//    { 0x00, 0x3f, 0 }, // Bus: Base, Limit, Translation
//    { 0, 0, 0 }, // IO
//    { MAX_UINT64, 0, 0 }, // Mem   < 0x1 0000 0000
//    { MAX_UINT64, 0, 0 }, // MemAbove4G
//    { MAX_UINT64, 0, 0
//     }, // PMem  < 0x1 0000 0000
//    { MAX_UINT64, 0, 0 }, // PMemAbove4G
//    (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath[0] // *DevicePath
//  },
//  // Host Bridge 1
//  {
//    0, // Segment
//    0, // Supports
//    0, // Attributes
//    FALSE, // DmaAbove4G
//    FALSE, // NoExtendedConfigSpace
//    FALSE, // ResourceAssigned
//    EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |
//    EFI_PCI_HOST_BRIDGE_MEM64_DECODE, // AllocationAttributes
//    { 0x40, 0x7f, 0 }, // Bus: Base, Limit, Translation
//    { 0, 0, 0 }, // IO
//    { MAX_UINT64, 0, 0 }, // Mem   < 0x1 0000 0000
//    { MAX_UINT64, 0, 0 }, // MemAbove4G
//    { MAX_UINT64, 0, 0 }, // PMem  < 0x1 0000 0000
//    { MAX_UINT64, 0, 0 }, // PMemAbove4G
//    (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath[1] // *DevicePath
//  },
//  // Host Bridge 2
STATIC PCI_ROOT_BRIDGE mRootBridges[] = {
 {
   0,
   0,
   0,
   TRUE,  // DmaAbove4G
   FALSE,
   FALSE,
   EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM | // combine memory
   EFI_PCI_HOST_BRIDGE_MEM64_DECODE, // support 64-bit memory window
   { 0x80, 0xff, 0 },
   { 0x800000, 0xffffff, 0 },
   { 0xe0000000, 0xefffffff, 0 },
  //  { 0x800000, 0xffffff, -0x4810000000 }, // IO
  //  { 0xe0000000, 0xefffffff, 0xffffffb800000000 }, // 256 MB
   { 0x4900000000, 0x4affffffff, 0 }, // MemAbove4G
   { MAX_UINT64, 0, 0 }, // PMem < 0x1 0000 0000
   { MAX_UINT64, 0, 0 }, // PMemAbove4G
   (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath
 }
};

/**
  Return all the root bridge instances in an array.

  @param Count  Return the count of root bridge instances.

  @return All the root bridge instances in an array.
          The array should be passed into PciHostBridgeFreeRootBridges()
          when it's not used.
**/
PCI_ROOT_BRIDGE *
EFIAPI
PciHostBridgeGetRootBridges (
  UINTN *Count
  )
{
  *Count = ARRAY_SIZE (mRootBridges);
  return mRootBridges;
}

/**
  Free the root bridge instances array returned from PciHostBridgeGetRootBridges().

  @param Bridges The root bridge instances array.
  @param Count   The count of the array.
**/
VOID
EFIAPI
PciHostBridgeFreeRootBridges (
  PCI_ROOT_BRIDGE *Bridges,
  UINTN           Count
  )
{
}

/**
  Inform the platform that the resource conflict happens.

  @param HostBridgeHandle Handle of the Host Bridge.
  @param Configuration    Pointer to PCI I/O and PCI memory resource
                          descriptors. The Configuration contains the resources
                          for all the root bridges. The resource for each root
                          bridge is terminated with END descriptor and an
                          additional END is appended indicating the end of the
                          entire resources. The resource descriptor field
                          values follow the description in
                          EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL
                          .SubmitResources().
**/
VOID
EFIAPI
PciHostBridgeResourceConflict (
  EFI_HANDLE                        HostBridgeHandle,
  VOID                              *Configuration
  )
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
  UINTN                             RootBridgeIndex;
  DEBUG ((DEBUG_ERROR, "PciHostBridge: Resource conflict happens!\n"));

  RootBridgeIndex = 0;
  Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Configuration;
  while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    DEBUG ((DEBUG_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));
    for (; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) {
      ASSERT (Descriptor->ResType <
              ARRAY_SIZE (mPciHostBridgeLibAcpiAddressSpaceTypeStr)
              );
      DEBUG ((DEBUG_ERROR, " %s: Length/Alignment = 0x%lx / 0x%lx\n",
              mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType],
              Descriptor->AddrLen, Descriptor->AddrRangeMax
              ));
      if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        DEBUG ((DEBUG_ERROR, "     Granularity/SpecificFlag = %ld / %02x%s\n",
                Descriptor->AddrSpaceGranularity, Descriptor->SpecificFlag,
                ((Descriptor->SpecificFlag &
                  EFI_ACPI_MEMORY_RESOURCE_SPECIFIC_FLAG_CACHEABLE_PREFETCHABLE
                  ) != 0) ? L" (Prefetchable)" : L""
                ));
      }
    }
    //
    // Skip the END descriptor for root bridge
    //
    ASSERT (Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)(
                   (EFI_ACPI_END_TAG_DESCRIPTOR *)Descriptor + 1
                   );
  }
}
