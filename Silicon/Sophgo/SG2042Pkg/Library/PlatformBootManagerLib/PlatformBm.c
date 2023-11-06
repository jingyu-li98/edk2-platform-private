/** @file
  Implementation for PlatformBootManagerLib library class interfaces.

  Copyright (c) 2023. SOPHGO Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Pci22.h>
#include <Library/BootLogoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PciIo.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include "PlatformBm.h"

STATIC PLATFORM_SERIAL_CONSOLE mSerialConsole = {
  //
  // VENDOR_DEVICE_PATH SerialDxe
  //
  {
    { HARDWARE_DEVICE_PATH, HW_VENDOR_DP, DP_NODE_LEN (VENDOR_DEVICE_PATH) },
    EDKII_SERIAL_PORT_LIB_VENDOR_GUID
  },

  //
  // UART_DEVICE_PATH Uart
  //
  {
    { MESSAGING_DEVICE_PATH, MSG_UART_DP, DP_NODE_LEN (UART_DEVICE_PATH) },
    0,                                      // Reserved
    115200,             // BaudRate
    8,                  // DataBits
    1,                  // Parity
    1                   // StopBits
  },

  //
  // VENDOR_DEVICE_PATH TermType
  //
  {
    {
      MESSAGING_DEVICE_PATH, MSG_VENDOR_DP,
      DP_NODE_LEN (VENDOR_DEVICE_PATH)
    }
    //
    // Guid to be filled in dynamically
    //
  },

  //
  // EFI_DEVICE_PATH_PROTOCOL End
  //
  {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
    DP_NODE_LEN (EFI_DEVICE_PATH_PROTOCOL)
  }
};

STATIC PLATFORM_USB_KEYBOARD mUsbKeyboard = {
  //
  // USB_CLASS_DEVICE_PATH Keyboard
  //
  {
    {
      MESSAGING_DEVICE_PATH, MSG_USB_CLASS_DP,
      DP_NODE_LEN (USB_CLASS_DEVICE_PATH)
    },
    0xFFFF, // VendorId: any
    0xFFFF, // ProductId: any
    3,      // DeviceClass: HID
    1,      // DeviceSubClass: boot
    1       // DeviceProtocol: keyboard
  },

  //
  // EFI_DEVICE_PATH_PROTOCOL End
  //
  {
    END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
    DP_NODE_LEN (EFI_DEVICE_PATH_PROTOCOL)
  }
};

/**
  Locate all handles that carry the specified protocol, filter them with a
  callback function, and pass each handle that passes the filter to another
  callback.

  @param[in] ProtocolGuid  The protocol to look for.

  @param[in] Filter        The filter function to pass each handle to. If this
                           parameter is NULL, then all handles are processed.

  @param[in] Process       The callback function to pass each handle to that
                           clears the filter.
**/
VOID
FilterAndProcess (
  IN EFI_GUID          *ProtocolGuid,
  IN FILTER_FUNCTION   Filter         OPTIONAL,
  IN CALLBACK_FUNCTION Process
  )
{
  EFI_STATUS Status;
  EFI_HANDLE *Handles;
  UINTN      NoHandles;
  UINTN      Idx;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  ProtocolGuid,
                  NULL /* SearchKey */,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status)) {
    //
    // This is not an error, just an informative condition.
    //
    DEBUG ((
      DEBUG_VERBOSE,
      "%a: %g: %r\n",
      __func__,
      ProtocolGuid,
      Status
      ));
    return;
  }

  ASSERT (NoHandles > 0);
  for (Idx = 0; Idx < NoHandles; ++Idx) {
    CHAR16        *DevicePathText;
    STATIC CHAR16 Fallback[] = L"<device path unavailable>";

    //
    // The ConvertDevicePathToText () function handles NULL input transparently.
    //
    DevicePathText = ConvertDevicePathToText (
                       DevicePathFromHandle (Handles[Idx]),
                       FALSE, // DisplayOnly
                       FALSE  // AllowShortcuts
                       );
    if (DevicePathText == NULL) {
      DevicePathText = Fallback;
    }

    if ((Filter == NULL)
      || (Filter (Handles[Idx], DevicePathText)))
    {
      Process (Handles[Idx], DevicePathText);
    }

    if (DevicePathText != Fallback) {
      FreePool (DevicePathText);
    }
  }
  gBS->FreePool (Handles);
}

/**
  This FILTER_FUNCTION checks if a handle corresponds to a PCI display device.

  @param  Handle   The handle to check
  @param  ReportText   A pointer to a string at the time of the error.

  @retval    TURE     THe  handle corresponds to a PCI display device.
  @retval    FALSE    THe  handle does not corresponds to a PCI display device.
**/
BOOLEAN
EFIAPI
IsPciDisplay (
  IN EFI_HANDLE   Handle,
  IN CONST CHAR16 *ReportText
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID**)&PciIo
                  );
  if (EFI_ERROR (Status)) {
    //
    // This is not an error worth reporting.
    //
    return FALSE;
  }

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0 /* Offset */,
                        sizeof Pci / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: %s: %r\n",
      __func__,
      ReportText,
      Status
      ));
    return FALSE;
  }

  return IS_PCI_DISPLAY (&Pci);
}

/**
  This FILTER_FUNCTION checks if a handle corresponds to a non-discoverable
  USB host controller.
**/
STATIC
BOOLEAN
EFIAPI
IsUsbHost (
  IN EFI_HANDLE    Handle,
  IN CONST CHAR16  *ReportText
  )
{
  NON_DISCOVERABLE_DEVICE  *Device;
  EFI_STATUS               Status;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEdkiiNonDiscoverableDeviceProtocolGuid,
                  (VOID **)&Device
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (CompareGuid (Device->Type, &gEdkiiNonDiscoverableUhciDeviceGuid) ||
      CompareGuid (Device->Type, &gEdkiiNonDiscoverableEhciDeviceGuid) ||
      CompareGuid (Device->Type, &gEdkiiNonDiscoverableXhciDeviceGuid))
  {
    return TRUE;
  }

  return FALSE;
}


/**
  This CALLBACK_FUNCTION attempts to connect a handle non-recursively, asking
  the matching driver to produce all first-level child handles.

  @param  Handle       The handle to connect.
  @param  ReportText   A pointer to a string at the time of the error.

  @retval  VOID
**/
VOID
EFIAPI
Connect (
  IN EFI_HANDLE   Handle,
  IN CONST CHAR16 *ReportText
  )
{
  EFI_STATUS Status;

  Status = gBS->ConnectController (
                  Handle, // ControllerHandle
                  NULL,   // DriverImageHandle
                  NULL,   // RemainingDevicePath -- produce all children
                  FALSE   // Recursive
                  );
  DEBUG ((
    EFI_ERROR (Status) ? DEBUG_ERROR : DEBUG_VERBOSE,
    "%a: %s: %r\n",
    __func__,
    ReportText,
    Status
    ));
}

/**
  This CALLBACK_FUNCTION retrieves the EFI_DEVICE_PATH_PROTOCOL from the
  handle, and adds it to ConOut and ErrOut.

  @param  Handle   The handle to retrieves.
  @param  ReportText   A pointer to a string at the time of the error.

  @retval  VOID
**/
VOID
EFIAPI
AddOutput (
  IN EFI_HANDLE    Handle,
  IN CONST CHAR16  *ReportText
  )
{
  EFI_STATUS               Status;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;

  DevicePath = DevicePathFromHandle (Handle);
  if (DevicePath == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: %s: handle %p: device path not found\n",
      __func__,
      ReportText,
      Handle
      ));
    return;
  }

  Status = EfiBootManagerUpdateConsoleVariable (ConOut, DevicePath, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: %s: adding to ConOut: %r\n",
      __func__,
      ReportText,
      Status
      ));
    return;
  }

  Status = EfiBootManagerUpdateConsoleVariable (ErrOut, DevicePath, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: %s: adding to ErrOut: %r\n",
      __func__,
      ReportText,
      Status)
      );
    return;
  }

  DEBUG ((
    DEBUG_INFO,
    "%a: %s: added to ConOut and ErrOut\n",
    __func__,
    ReportText
    ));
}
/**
  Register the boot option.

  @param  FileGuid      File Guid.
  @param  Description   Option descriptor.
  @param  Attributes    Option  Attributes.

  @retval  VOID
**/
VOID
PlatformRegisterFvBootOption (
  IN EFI_GUID     *FileGuid,
  IN CHAR16       *Description,
  IN UINT32       Attributes,
  EFI_INPUT_KEY   *Key
  )
{
  EFI_STATUS                        Status;
  INTN                              OptionIndex;
  EFI_BOOT_MANAGER_LOAD_OPTION      NewOption;
  EFI_BOOT_MANAGER_LOAD_OPTION      *BootOptions;
  UINTN                             BootOptionCount;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH FileNode;
  EFI_LOADED_IMAGE_PROTOCOL         *LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &LoadedImage
                  );
  ASSERT_EFI_ERROR (Status);

  EfiInitializeFwVolDevicepathNode (&FileNode, FileGuid);
  DevicePath = DevicePathFromHandle (LoadedImage->DeviceHandle);
  ASSERT (DevicePath != NULL);
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *) &FileNode
                 );
  ASSERT (DevicePath != NULL);

  Status = EfiBootManagerInitializeLoadOption (
             &NewOption,
             LoadOptionNumberUnassigned,
             LoadOptionTypeBoot,
             Attributes,
             Description,
             DevicePath,
             NULL,
             0
             );
  ASSERT_EFI_ERROR (Status);
  FreePool (DevicePath);

  BootOptions = EfiBootManagerGetLoadOptions (
                  &BootOptionCount,
                  LoadOptionTypeBoot
                  );

  OptionIndex = EfiBootManagerFindLoadOption (
                  &NewOption,
                  BootOptions,
                  BootOptionCount
                  );

  if (OptionIndex == -1) {
    Status = EfiBootManagerAddLoadOptionVariable (&NewOption, MAX_UINTN);
    ASSERT_EFI_ERROR (Status);
    Status = EfiBootManagerAddKeyOptionVariable (
               NULL,
               (UINT16)NewOption.OptionNumber,
               0,
               Key,
               NULL
               );
    ASSERT (Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
  }

  EfiBootManagerFreeLoadOption (&NewOption);
  EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
}

/**
  Remove all MemoryMapped (...)/FvFile (...) and Fv (...)/FvFile (...) boot options
  whose device paths do not resolve exactly to an FvFile in the system.

  This removes any boot options that point to binaries built into the firmware
  and have become stale due to any of the following:
  - FvMain's base address or size changed (historical),
  - FvMain's FvNameGuid changed,
  - the FILE_GUID of the pointed-to binary changed,
  - the referenced binary is no longer built into the firmware.

  EfiBootManagerFindLoadOption () used in PlatformRegisterFvBootOption () only
  avoids exact duplicates.
**/
// VOID
// RemoveStaleFvFileOptions (
//   VOID
//   )
// {
//   EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
//   UINTN                        BootOptionCount;
//   UINTN                        Index;

//   BootOptions = EfiBootManagerGetLoadOptions (&BootOptionCount,
//                   LoadOptionTypeBoot);

//   for (Index = 0; Index < BootOptionCount; ++Index) {
//     EFI_DEVICE_PATH_PROTOCOL *Node1, *Node2, *SearchNode;
//     EFI_STATUS               Status;
//     EFI_HANDLE               FvHandle;

//     //
//     // If the device path starts with neither MemoryMapped (...) nor Fv (...),
//     // then keep the boot option.
//     //
//     Node1 = BootOptions[Index].FilePath;
//     if (!(DevicePathType (Node1) == HARDWARE_DEVICE_PATH
//       && DevicePathSubType (Node1) == HW_MEMMAP_DP)
//       && !(DevicePathType (Node1) == MEDIA_DEVICE_PATH
//       && DevicePathSubType (Node1) == MEDIA_PIWG_FW_VOL_DP))
//     {
//       continue;
//     }

//     //
//     // If the second device path node is not FvFile (...), then keep the boot
//     // option.
//     //
//     Node2 = NextDevicePathNode (Node1);
//     if ((DevicePathType (Node2) != MEDIA_DEVICE_PATH)
//       || (DevicePathSubType (Node2) != MEDIA_PIWG_FW_FILE_DP))
//     {
//       continue;
//     }

//     //
//     // Locate the Firmware Volume2 protocol instance that is denoted by the
//     // boot option. If this lookup fails (i.e., the boot option references a
//     // firmware volume that doesn't exist), then we'll proceed to delete the
//     // boot option.
//     //
//     SearchNode = Node1;
//     Status = gBS->LocateDevicePath (&gEfiFirmwareVolume2ProtocolGuid,
//                     &SearchNode, &FvHandle);

//     if (!EFI_ERROR (Status)) {
//       //
//       // The firmware volume was found; now let's see if it contains the FvFile
//       // identified by GUID.
//       //
//       EFI_FIRMWARE_VOLUME2_PROTOCOL     *FvProtocol;
//       MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFileNode;
//       UINTN                             BufferSize;
//       EFI_FV_FILETYPE                   FoundType;
//       EFI_FV_FILE_ATTRIBUTES            FileAttributes;
//       UINT32                            AuthenticationStatus;

//       Status = gBS->HandleProtocol (FvHandle, &gEfiFirmwareVolume2ProtocolGuid,
//                       (VOID **)&FvProtocol);
//       ASSERT_EFI_ERROR (Status);

//       FvFileNode = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)Node2;
//       //
//       // Buffer==NULL means we request metadata only: BufferSize, FoundType,
//       // FileAttributes.
//       //
//       Status = FvProtocol->ReadFile (
//                              FvProtocol,
//                              &FvFileNode->FvFileName, // NameGuid
//                              NULL,                    // Buffer
//                              &BufferSize,
//                              &FoundType,
//                              &FileAttributes,
//                              &AuthenticationStatus
//                              );
//       if (!EFI_ERROR (Status)) {
//         //
//         // The FvFile was found. Keep the boot option.
//         //
//         continue;
//       }
//     }

//     //
//     // Delete the boot option.
//     //
//     Status = EfiBootManagerDeleteLoadOptionVariable (
//                BootOptions[Index].OptionNumber, LoadOptionTypeBoot);
//     DEBUG_CODE (
//       CHAR16 *DevicePathString;

//       DevicePathString = ConvertDevicePathToText (BootOptions[Index].FilePath,
//                            FALSE, FALSE);
//       DEBUG ((
//         EFI_ERROR (Status) ? EFI_D_WARN : DEBUG_VERBOSE,
//         "%a: removing stale Boot#%04x %s: %r\n",
//         __func__,
//         (UINT32)BootOptions[Index].OptionNumber,
//         DevicePathString == NULL ? L"<unavailable>" : DevicePathString,
//         Status
//         ));
//       if (DevicePathString != NULL) {
//         FreePool (DevicePathString);
//       }
//       );
//   }

//   EfiBootManagerFreeLoadOptions (BootOptions, BootOptionCount);
// }

STATIC
VOID
GetPlatformOptions (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_BOOT_MANAGER_LOAD_OPTION    *CurrentBootOptions;
  EFI_BOOT_MANAGER_LOAD_OPTION    *BootOptions;
  EFI_INPUT_KEY                   *BootKeys;
  PLATFORM_BOOT_MANAGER_PROTOCOL  *PlatformBootManager;
  UINTN                           CurrentBootOptionCount;
  UINTN                           Index;
  UINTN                           BootCount;

  Status = gBS->LocateProtocol (
                  &gPlatformBootManagerProtocolGuid,
                  NULL,
                  (VOID **)&PlatformBootManager
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = PlatformBootManager->GetPlatformBootOptionsAndKeys (
                                  &BootCount,
                                  &BootOptions,
                                  &BootKeys
                                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Fetch the existent boot options. If there are none, CurrentBootCount
  // will be zeroed.
  //
  CurrentBootOptions = EfiBootManagerGetLoadOptions (
                         &CurrentBootOptionCount,
                         LoadOptionTypeBoot
                         );
  //
  // Process the platform boot options.
  //
  for (Index = 0; Index < BootCount; Index++) {
    INTN   Match;
    UINTN  BootOptionNumber;

    //
    // If there are any preexistent boot options, and the subject platform boot
    // option is already among them, then don't try to add it. Just get its
    // assigned boot option number so we can associate a hotkey with it. Note
    // that EfiBootManagerFindLoadOption() deals fine with (CurrentBootOptions
    // == NULL) if (CurrentBootCount == 0).
    //
    Match = EfiBootManagerFindLoadOption (
              &BootOptions[Index],
              CurrentBootOptions,
              CurrentBootOptionCount
              );
    if (Match >= 0) {
      BootOptionNumber = CurrentBootOptions[Match].OptionNumber;
    } else {
      //
      // Add the platform boot options as a new one, at the end of the boot
      // order. Note that if the platform provided this boot option with an
      // unassigned option number, then the below function call will assign a
      // number.
      //
      Status = EfiBootManagerAddLoadOptionVariable (
                 &BootOptions[Index],
                 MAX_UINTN
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a: failed to register \"%s\": %r\n",
          __func__,
          BootOptions[Index].Description,
          Status
          ));
        continue;
      }

      BootOptionNumber = BootOptions[Index].OptionNumber;
    }

    //
    // Register a hotkey with the boot option, if requested.
    //
    if (BootKeys[Index].UnicodeChar == L'\0') {
      continue;
    }

    Status = EfiBootManagerAddKeyOptionVariable (
               NULL,
               BootOptionNumber,
               0,
               &BootKeys[Index],
               NULL
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "%a: failed to register hotkey for \"%s\": %r\n",
        __func__,
        BootOptions[Index].Description,
        Status
        ));
    }
  }

  EfiBootManagerFreeLoadOptions (CurrentBootOptions, CurrentBootOptionCount);
  EfiBootManagerFreeLoadOptions (BootOptions, BootCount);
  FreePool (BootKeys);
}

/**
  Register the boot option And Keys.

  @param  VOID

  @retval  VOID
**/
VOID
PlatformRegisterOptionsAndKeys (
  VOID
  )
{
  EFI_STATUS                   Status;
  EFI_INPUT_KEY                Enter;
  EFI_INPUT_KEY                F2;
  EFI_INPUT_KEY                Esc;
  EFI_BOOT_MANAGER_LOAD_OPTION BootOption;

  GetPlatformOptions ();

  //
  // Register ENTER as CONTINUE key
  //
  Enter.ScanCode    = SCAN_NULL;
  Enter.UnicodeChar = CHAR_CARRIAGE_RETURN;
  Status = EfiBootManagerRegisterContinueKeyOption (0, &Enter, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // Map F2 and ESC to Boot Manager Menu
  //
  F2.ScanCode     = SCAN_F2;
  F2.UnicodeChar  = CHAR_NULL;
  Esc.ScanCode    = SCAN_ESC;
  Esc.UnicodeChar = CHAR_NULL;
  Status = EfiBootManagerGetBootManagerMenu (&BootOption);
  ASSERT_EFI_ERROR (Status);
  Status = EfiBootManagerAddKeyOptionVariable (
             NULL,
             (UINT16) BootOption.OptionNumber,
             0,
             &F2,
             NULL
             );
  ASSERT (Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
  Status = EfiBootManagerAddKeyOptionVariable (
             NULL,
             (UINT16) BootOption.OptionNumber,
             0,
             &Esc,
             NULL
             );
  ASSERT (Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED);
}

//
// BDS Platform Functions
//
/**
  Do the platform init, can be customized by OEM/IBV
  Possible things that can be done in PlatformBootManagerBeforeConsole:
  > Update console variable: 1. include hot-plug devices;
  >                          2. Clear ConIn and add SOL for AMT
  > Register new Driver#### or Boot####
  > Register new Key####: e.g.: F12
  > Signal ReadyToLock event
  > Authentication action: 1. connect Auth devices;
  >                        2. Identify auto logon user.
**/
VOID
EFIAPI
PlatformBootManagerBeforeConsole (
  VOID
  )
{
  //
  // Signal EndOfDxe PI Event
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Signal EndOfDxe PI Event !!!\n\n", __func__, __LINE__));
  EfiEventGroupSignal (&gEfiEndOfDxeEventGroupGuid);

  //
  // Dispatch deferred images after EndOfDxe event.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Dispatch deferred images after EndOfDxe event. !!!\n\n", __func__, __LINE__));
  EfiBootManagerDispatchDeferredImages ();

  //
  // Locate the PCI root bridges and make the PCI bus driver connect each,
  // non-recursively. This will produce a number of child handles with PciIo on
  // them.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Locate the PCI root bridges and make the PCI bus driver connect each. !!!\n\n", __func__, __LINE__));
  FilterAndProcess (&gEfiPciRootBridgeIoProtocolGuid, NULL, Connect);

  //
  // Find all display class PCI devices (using the handles from the previous
  // step), and connect them non-recursively. This should produce a number of
  // child handles with GOPs on them.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Find all display class PCI devices. !!!\n\n", __func__, __LINE__));
  FilterAndProcess (&gEfiPciIoProtocolGuid, IsPciDisplay, Connect);

  //
  // Now add the device path of all handles with GOP on them to ConOut and
  // ErrOut.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] add the device path of all handles with GOP. !!!\n\n", __func__, __LINE__));
  FilterAndProcess (&gEfiGraphicsOutputProtocolGuid, NULL, AddOutput);

  //
  // The core BDS code connects short-form USB device paths by explicitly
  // looking for handles with PCI I/O installed, and checking the PCI class
  // code whether it matches the one for a USB host controller. This means
  // non-discoverable USB host controllers need to have the non-discoverable
  // PCI driver attached first.
  //
  FilterAndProcess (&gEdkiiNonDiscoverableDeviceProtocolGuid, IsUsbHost, Connect);

  //
  // Add the hardcoded short-form USB keyboard device path to ConIn.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Add the hardcoded short-form USB keyboard device path to ConIn. !!!\n\n", __func__, __LINE__));
  EfiBootManagerUpdateConsoleVariable (
    ConIn,
    (EFI_DEVICE_PATH_PROTOCOL *)&mUsbKeyboard,
    NULL
    );

  // //
  // // Add the hardcoded serial console device path to ConIn, ConOut, ErrOut.
  // //
  // STATIC_ASSERT (
  //   FixedPcdGet8 (PcdDefaultTerminalType) == 4,
  //   "PcdDefaultTerminalType must be TTYTERM"
  //   );
  // STATIC_ASSERT (
  //   FixedPcdGet8 (PcdUartDefaultParity) != 0,
  //   "PcdUartDefaultParity must be set to an actual value, not 'default'"
  //   );
  // STATIC_ASSERT (
  //   FixedPcdGet8 (PcdUartDefaultStopBits) != 0,
  //   "PcdUartDefaultStopBits must be set to an actual value, not 'default'"
  //   );
  //
  // Add the hardcoded serial console device path to ConIn, ConOut, ErrOut.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Add the hardcoded serial console device path to ConIn, ConOut, ErrOut. !!!\n\n", __func__, __LINE__));
  CopyGuid (&mSerialConsole.TermType.Guid, &gEfiTtyTermGuid);

  EfiBootManagerUpdateConsoleVariable (
    ConIn,
    (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole,
    NULL
    );
  EfiBootManagerUpdateConsoleVariable (
    ConOut,
    (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole,
    NULL
    );
  EfiBootManagerUpdateConsoleVariable (
    ErrOut,
    (EFI_DEVICE_PATH_PROTOCOL *)&mSerialConsole,
    NULL
    );

  //
  // Register platform-specific boot options and keyboard shortcuts.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Register platform-specific boot options and keyboard shortcuts.. !!!\n\n", __func__, __LINE__));
  PlatformRegisterOptionsAndKeys ();
}

/**
  Do the platform specific action after the console is ready
  Possible things that can be done in PlatformBootManagerAfterConsole:
  > Console post action:
    > Dynamically switch output mode from 100x31 to 80x25 for certain senarino
    > Signal console ready platform customized event
  > Run diagnostics like memory testing
  > Connect certain devices
  > Dispatch aditional option roms
  > Special boot: e.g.: USB boot, enter UI
**/
VOID
EFIAPI
PlatformBootManagerAfterConsole (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  UINTN                         FirmwareVerLength;
  UINTN                         PosX;
  UINTN                         PosY;
  EFI_INPUT_KEY                 Key;

  FirmwareVerLength = StrLen (PcdGetPtr (PcdFirmwareVersionString));
  //
  // Show the splash screen.
  //
  // BootLogoEnableLogo ();
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Show the splah screen !!!\n\n", __func__, __LINE__));
  Status = BootLogoEnableLogo ();
  if (EFI_ERROR (Status)) {
    if (FirmwareVerLength > 0) {
      Print (
        VERSION_STRING_PREFIX L"%s\n",
        PcdGetPtr (PcdFirmwareVersionString)
        );
    }

    Print (L"Press ESCAPE for boot options ");
  } else if (FirmwareVerLength > 0) {
    Status = gBS->HandleProtocol (
                    gST->ConsoleOutHandle,
                    &gEfiGraphicsOutputProtocolGuid,
                    (VOID **)&GraphicsOutput
                    );
    if (!EFI_ERROR (Status)) {
      PosX = (GraphicsOutput->Mode->Info->HorizontalResolution -
              (StrLen (VERSION_STRING_PREFIX) + FirmwareVerLength) *
              EFI_GLYPH_WIDTH) / 2;
      PosY = 0;

      PrintXY (
        PosX,
        PosY,
        NULL,
        NULL,
        VERSION_STRING_PREFIX L"%s",
        PcdGetPtr (PcdFirmwareVersionString)
        );
    }
  }

  //
  // Connect the rest of the devices.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Connect the rest of the devices. !!!\n\n", __func__, __LINE__));
  EfiBootManagerConnectAll ();

  //
  // Enumerate all possible boot options, then filter and reorder them based on
  // the QEMU configuration.
  //
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Enumerate all possible boot options. !!!\n\n", __func__, __LINE__));
  EfiBootManagerRefreshAllBootOption ();

  // //
  // // Register UEFI Shell
  // //
  // PlatformRegisterFvBootOption (
  //   &gUefiShellFileGuid,
  //   L"EFI Internal Shell",
  //   LOAD_OPTION_ACTIVE
  //   );
  DEBUG ((DEBUG_WARN, "!!! %a[%d] Register UEFI Shell. !!!\n\n", __func__, __LINE__));
  Key.ScanCode    = SCAN_NULL;
  Key.UnicodeChar = L's';
  PlatformRegisterFvBootOption (
    &gUefiShellFileGuid, 
    L"UEFI Shell",
    LOAD_OPTION_ACTIVE,
    &Key);
    //
  // Boot the 'UEFI Shell'. If the Pcd is not set, the UEFI Shell is not
  // an active boot option and must be manually selected through UiApp
  // (at least during the fist boot).
  //
  // if (FixedPcdGetBool (PcdUefiShellDefaultBootEnable)) {
  //   PlatformBootFvBootOption (
  //     &gUefiShellFileGuid,
  //     L"UEFI Shell (default)"
  //     );
  // }

  // RemoveStaleFvFileOptions ();
}

/**
  This function is called each second during the boot manager waits the
  timeout.

  @param TimeoutRemain  The remaining timeout.
**/
VOID
EFIAPI
PlatformBootManagerWaitCallback (
  IN UINT16          TimeoutRemain
  )
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION  Black;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION  White;
  UINT16                               Timeout;
  EFI_STATUS                           Status;

  Timeout = PcdGet16 (PcdPlatformBootTimeOut);

  Black.Raw = 0x00000000;
  White.Raw = 0x00FFFFFF;

  Status = BootLogoUpdateProgress (
             White.Pixel,
             Black.Pixel,
             L"Press ESCAPE for boot options",
             White.Pixel,
             (Timeout - TimeoutRemain) * 100 / Timeout,
             0
             );
  if (EFI_ERROR (Status)) {
    Print (L".");
  }
}

/**
  The function is called when no boot option could be launched,
  including platform recovery options and options pointing to applications
  built into firmware volumes.

  If this function returns, BDS attempts to enter an infinite loop.
**/
VOID
EFIAPI
PlatformBootManagerUnableToBoot (
  VOID
  )
{
  // EFI_STATUS                   Status;
  // EFI_INPUT_KEY                Key;
  // EFI_BOOT_MANAGER_LOAD_OPTION BootManagerMenu;
  // UINTN                        Index;

  // //
  // // BootManagerMenu doesn't contain the correct information when return status
  // // is EFI_NOT_FOUND.
  // //
  // Status = EfiBootManagerGetBootManagerMenu (&BootManagerMenu);
  // if (EFI_ERROR (Status)) {
  //   return;
  // }
  // //
  // // Normally BdsDxe does not print anything to the system console, but this is
  // // a last resort -- the end-user will likely not see any DEBUG messages
  // // logged in this situation.
  // //
  // // AsciiPrint () will NULL-check gST->ConOut internally. We check gST->ConIn
  // // here to see if it makes sense to request and wait for a keypress.
  // //
  // if (gST->ConIn != NULL) {
  //   AsciiPrint (
  //     "%a: No bootable option or device was found.\n"
  //     "%a: Press any key to enter the Boot Manager Menu.\n",
  //     gEfiCallerBaseName,
  //     gEfiCallerBaseName
  //     );
  //   Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
  //   ASSERT_EFI_ERROR (Status);
  //   ASSERT (Index == 0);

  //   //
  //   // Drain any queued keys.
  //   //
  //   while (!EFI_ERROR (gST->ConIn->ReadKeyStroke (gST->ConIn, &Key))) {
  //     //
  //     // just throw away Key
  //     //
  //   }
  // }

  // for (;;) {
  //   EfiBootManagerBoot (&BootManagerMenu);
  // }

  return;
}
