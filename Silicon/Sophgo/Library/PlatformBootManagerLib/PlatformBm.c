/** @file
  Implementation for PlatformBootManagerLib library class interfaces.

  Copyright (c) 2023. SOPHGO Inc. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PlatformBm.h"
#include "IniParserUtil.h"
#include <Protocol/BlockIo.h>
#include <stdlib.h>
#include <string.h>
#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadFile.h>
#include <Protocol/DevicePath.h>


#define EFI_FILE_NAME L"\\conf.ini"
#define BUFFER_SIZE    1024
#define INI_HEADER  L"[sophgo-config]"
#define INI_TAIL    L"[eof]"

typedef struct {
  CHAR8 *Name;
  UINTN Addr;
} TEST_CONFIG;

TEST_CONFIG *Config;


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
    0,                  // Reserved
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

/** Boot a Fv Boot Option.

  This function is useful for booting the UEFI Shell as it is loaded
  as a non active boot option.

  @param[in] FileGuid      The File GUID.
  @param[in] Description   String describing the Boot Option.

**/
STATIC
VOID
PlatformBootFvBootOption (
  IN  CONST EFI_GUID  *FileGuid,
  IN  CHAR16          *Description
  )
{
  EFI_STATUS                         Status;
  EFI_BOOT_MANAGER_LOAD_OPTION       NewOption;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  FileNode;
  EFI_LOADED_IMAGE_PROTOCOL          *LoadedImage;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **)&LoadedImage
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // The UEFI Shell was registered in PlatformRegisterFvBootOption ()
  // previously, thus it must still be available in this FV.
  //
  EfiInitializeFwVolDevicepathNode (&FileNode, FileGuid);
  DevicePath = DevicePathFromHandle (LoadedImage->DeviceHandle);
  ASSERT (DevicePath != NULL);
  DevicePath = AppendDevicePathNode (
                 DevicePath,
                 (EFI_DEVICE_PATH_PROTOCOL *)&FileNode
                 );
  ASSERT (DevicePath != NULL);

  Status = EfiBootManagerInitializeLoadOption (
             &NewOption,
             LoadOptionNumberUnassigned,
             LoadOptionTypeBoot,
             LOAD_OPTION_ACTIVE,
             Description,
             DevicePath,
             NULL,
             0
             );
  ASSERT_EFI_ERROR (Status);
  FreePool (DevicePath);

  EfiBootManagerBoot (&NewOption);
}

/**
  Make a platform driver to create predefined boot options and related hot keys.

  @param  VOID

  @retval  VOID
**/
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

  //
  // Load platform boot options
  //
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
#if 0
/**
  Get the conf.ini file path from a media device

**/
EFI_DEVICE_PATH_PROTOCOL *
ExpandMediaDeviceFilePath (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;
  EFI_HANDLE                *BlockIoHandles;
  EFI_HANDLE                *SimpleFileSystemHandles;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  UINTN                     NumberBlockIoHandles;
  UINTN                     NumberSimpleFileSystemHandles;
  UINTN                     Index;
  UINTN                     TempSize;
  UINTN                     Size;
  VOID                      *Buffer;

  //
  // Step 1. Get the device path
  //
  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiBlockIoProtocolGuid,
                 NULL,
                 &NumberBlockIoHandles,
                 &BlockIoHandles
                 );

  if (EFI_ERROR (Status)) {
    //
    // This is not an error, just an informative condition.
    //
    DEBUG ((
      DEBUG_ERROR,
      "%a: %g: %r\n",
      __func__,
      gEfiBlockIoProtocolGuid,
      Status
      ));
    return NULL;
  }
  ASSERT (NumberBlockIoHandles > 0);

  for (Index = 0; Index < NumberBlockIoHandles; Index++) {
    //
    // Get the device path of SimpleFileSystem handle
    //
    TempDevicePath = DevicePathFromHandle (BlockIoHandles[Index]);
  }

  //
  // Step 2. Check whether the device is connected
  //
  Status = gBS->LocateDevicePath (
                 &gEfiBlockIoProtocolGuid,
                 &TempDevicePath,
                 &Handle
                 );
  ASSERT_EFI_ERROR (Status);

  gBS->ConnectController (Handle, NULL, NULL, TRUE);

  //
  // Issue a dummy read to the device to check for media change.
  // When the removable media is changed, any Block IO read/write will
  // cause the BlockIo protocol be reinstalled and EFI_MEDIA_CHANGED is
  // returned. After the Block IO protocol is reinstalled, subsequent
  // Block IO read/write will success.
  //
  Status = gBS->HandleProtocol (
                 Handle,
                 &gEfiBlockIoProtocolGuid,
                 (VOID **)&BlockIo
                 );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Buffer = AllocatePool (BlockIo->Media->BlockSize);
  if (Buffer != NULL) {
    BlockIo->ReadBlocks (
               BlockIo,
               BlockIo->Media->MediaId,
               0,
               BlockIo->Media->BlockSize,
               Buffer
               );
    FreePool (Buffer);
  }

  //
  // Step 3. Detect the ASpeedAst2600Gop.efi file from device
  //
  FilePath = NULL;
  Size = GetDevicePathSize (TempDevicePath) - END_DEVICE_PATH_LENGTH;
  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiSimpleFileSystemProtocolGuid,
         NULL,
         &NumberSimpleFileSystemHandles,
         &SimpleFileSystemHandles
         );
  for (Index = 0; Index < NumberSimpleFileSystemHandles; Index++) {
    //
    // Get the device path size of SimpleFileSystem handle
    //
    TempDevicePath = DevicePathFromHandle (SimpleFileSystemHandles[Index]);
    TempSize = GetDevicePathSize (TempDevicePath) - END_DEVICE_PATH_LENGTH;
    //
    // Check whether the device path of boot option is part of the SimpleFileSystem handle's device path
    //
    if ((Size <= TempSize) && (CompareMem (TempDevicePath, TempDevicePath, Size) == 0)) {
      FilePath = FileDevicePath (SimpleFileSystemHandles[Index], EFI_FILE_NAME);
    }
  }

  if (BlockIoHandles != NULL) {
    FreePool (BlockIoHandles);
  }

  if (SimpleFileSystemHandles != NULL) {
    FreePool (SimpleFileSystemHandles);
  }

  return FilePath;
}

/**
  load conf.ini into memory

**/
EFI_STATUS
LoadFileFromExpandMediaDevice (
 IN VOID *NewBuffer
 )
{
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL   *FilePath;
  EFI_DEVICE_PATH_PROTOCOL   *HandleFilePath;
  UINTN                      FileBufferSize;
  UINTN                      FileInfoSize;
  EFI_HANDLE                 Handle;
  // EFI_LOAD_FILE_PROTOCOL     *LoadFileProtocol;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE_HANDLE            FileHandle;
  EFI_FILE_HANDLE            File;
  EFI_FILE_INFO              *FileInfo;


  FileInfo = NULL;
  FileHandle = NULL;
  Handle = NULL;
  File = NULL;
  FileBufferSize = 0;
  //FileBuffer = NULL;

  //
  // Get the file path
  //

    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Get the file path\n",
      __func__,
      __LINE__
      ));
  FilePath = ExpandMediaDeviceFilePath ();
  if (FilePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HandleFilePath = FilePath;
#if 0
  Status = gBS->LocateDevicePath (
		 &gEfiFirmwareVolume2ProtocolGuid,
		 &HandleFilePath,
		 &Handle
		 );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Locate Load FirmwareVolume2 Protocol failed (%r)!\n",
      __func__,
      __LINE__,
      Status
      ));

    //return Status;
  }
#endif
  Status = gBS->LocateDevicePath (
		 &gEfiSimpleFileSystemProtocolGuid,
		 &HandleFilePath,
		 &Handle
		 );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Locate Load Simple File System Protocol failed (%r)!\n",
      __func__,
      __LINE__,
      Status
      ));

    return Status;
  }
#if 0
  Status = gBS->LocateDevicePath (
		 &gEfiLoadFileProtocolGuid,
		 &HandleFilePath,
		 &Handle
		 );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Locate Load File Protocol failed (%r)!\n",
      __func__,
      __LINE__,
      Status
      ));

    return Status;
  }

  //
  // Attempt to access the file via LoadFile interface
  //
  Status = gBS->HandleProtocol (
		  Handle,
		  &gEfiLoadFileProtocolGuid,
		  (VOID **)&LoadFileProtocol
		  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Get Load File Protocol failed (%r)!\n",
      __func__,
      __LINE__,
      Status
      ));

    return Status;
  }


    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: load file first\n",
      __func__,
      __LINE__
      ));
    FileBuffer = AllocatePages (FileBufferSize);
    if (FileBuffer == NULL) {
      DEBUG ((
        DEBUG_ERROR,
        "%a[%d]: Allocate Pages failed!\n",
        __func__,
        __LINE__
        ));
    }
  Status = LoadFileProtocol->LoadFile (
		 LoadFileProtocol,
		 FilePath,
		 TRUE,
		 &FileBufferSize,
		 FileBuffer
		 );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileBuffer = AllocatePages (FileBufferSize);
    if (FileBuffer == NULL) {
      DEBUG ((
        DEBUG_ERROR,
        "%a[%d]: Allocate Pages failed!\n",
        __func__,
        __LINE__
        ));

      return EFI_OUT_OF_RESOURCES;
    } else {
      Status = LoadFileProtocol->LoadFile (
		   LoadFileProtocol,
		   FilePath,
		   TRUE,
		   &FileBufferSize,
		   FileBuffer
		   );
      if (EFI_ERROR (Status)) {
        DEBUG ((
          DEBUG_ERROR,
          "%a[%d]: Load File failed!\n",
          __func__,
          __LINE__
          ));

        return Status;
      }
    }
  }
#endif
// 1. 获取 Simple File System Protocol
    Status = gBS->HandleProtocol (
		   Handle,
		   &gEfiSimpleFileSystemProtocolGuid,
		   (VOID **)&SimpleFileSystem
		   );
    if (EFI_ERROR (Status)) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to get Simple File System Protocol: %r\n",
	  Status
	  ));
        return Status;
    }

    //
    // 2. Open the Volume to get the File System handle
    //
    Status = SimpleFileSystem->OpenVolume (SimpleFileSystem, &FileHandle);
    if (EFI_ERROR (Status)) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to open volume: %r\n",
	  Status
	  ));
        return Status;
    }

    //
    // 3. 打开文件
    //
    Status = FileHandle->Open (
		    FileHandle,
		    &File,
		    ((FILEPATH_DEVICE_PATH *)HandleFilePath)->PathName,
		    EFI_FILE_MODE_READ,
		    0
		    );
    if (EFI_ERROR (Status)) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to open file: %r\n",
	  Status
	  ));
        FileHandle->Close (FileHandle);
        return Status;
    }

    //
    // 4. We have found the file. Now we need to read it. Before we can read the
    // file we need to figure out how big the file is.
    //
    //
    Status = File->GetInfo (
		    File,
		    &gEfiFileInfoGuid,
		    &FileInfoSize,
		    FileInfo
		    );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      FileInfo = AllocatePool (FileInfoSize);
      if (FileInfo == NULL) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to allocate memory for file buffer\n"
	  ));
        return EFI_OUT_OF_RESOURCES;
      } else {
	Status = File->GetInfo (
			File,
			&gEfiFileInfoGuid,
			&FileInfoSize,
			FileInfo
			);

      }
    }

    //
    // Allocate space for the file
    //
    NewBuffer = AllocatePool ((UINTN)FileInfo->FileSize);
    if (NewBuffer == NULL) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to allocate memory for file buffer\n"
	  ));
        return EFI_OUT_OF_RESOURCES;
    } else {
      FileBufferSize = (UINTN)FileInfo->FileSize;
      Status = File->Read (
		      File,
		      &FileBufferSize,
		      NewBuffer
		      );
      if (EFI_ERROR (Status)) {
        DEBUG ((
	  DEBUG_ERROR,
	  "Failed to read file: %r\n",
	  Status
	  ));
        FreePool (NewBuffer);
        File->Close(File);
        RootDir->Close(RootDir);
        return Status;
    }

    DEBUG((DEBUG_INFO, "Read %d bytes from file.\n", FileBufferSize));

    FreePool (NewBuffer);
    File->Close (File);
    FileHandle->Close (FileHandle);

    return EFI_SUCCESS;
}
#endif
#if 0
STATIC
CHAR8 *
IniStrDup (
  IN CONST CHAR8  *String
  )
{
  return AllocateCopyPool (AsciiStrSize (String), String);
}
STATIC
INT32
TestHandler (
  IN       VOID  *User,
  IN CONST CHAR8 *Section,
  IN CONST CHAR8 *Name,
  IN CONST CHAR8 *Value
  )
{
  Config = (TEST_CONFIG *)User;

  #define MATCH(S, N) (AsciiStrCmp (Section, S) == 0 && AsciiStrCmp (Name, N) == 0)

  if (MATCH("devicetree", "name")) {
    Config->Name = IniStrDup (Value);
  } else if (MATCH("devicetree", "addr")) {
    Config->Addr = strtoul (Value, NULL, 16);
  } else {
    return 0;
  }

  return -1;
}
#endif

EFI_STATUS
IniParser (
  VOID
  )
{
  UINTN Address = PcdGet64 (PcdIniFileRamAddress);
  UINT8 *Buffer = (UINT8 *)Address;  // 将内存地址转换为指向 UINT8 的指针

//  UINTN Length = BUFFER_SIZE;

  if (Buffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "Memory address is invalid.\n"
      ));
    return EFI_INVALID_PARAMETER;
   }

  CHAR16 *MemoryData = (CHAR16 *)Buffer;
  if (StrnCmp(MemoryData, INI_HEADER, StrLen(INI_HEADER))) {
    DEBUG ((
      DEBUG_ERROR,
      "conf.ini should start with \"%s\"\n",
      INI_HEADER
      ));
			   ;
    return EFI_INVALID_PARAMETER;
  } else {
     DEBUG ((
      DEBUG_INFO,
      "Found string: %s\n",
      INI_HEADER
      ));
     return EFI_SUCCESS;
  }
#if 0

  //
  // We have a limitation for flash DMMR mode. It can only read out data
  // below 16MB. But it is enough for a configuration file.
  //
  if (mango_load_conf_sd(read_buf, sizeof(read_buf))) {
    if (mango_load_conf_spi(read_buf, sizeof(read_buf)))
                        return -1;
        }


        if (ini_parse_string((const char*)read_buf, handler, &config) < 0
            || config.name == NULL)
                return -1;
#endif
#if 0
  if (AsciiStrnCmp(Header, (CONST CHAR8 *)FileBuffer, AsciiStrLen(Header))) {
    DEBUG ((
      DEBUG_ERROR,
      "conf.ini should start with \"%s\"\n",
      Header
      ));
			   ;
    return EFI_INVALID_PARAMETER;
  }

  Eof = AsciiStrStr((CONST CHAR8 *)FileBuffer, Tail);

  if (!Eof) {
    DEBUG ((
      DEBUG_ERROR,
      "conf.ini should terminated by \"%s\"\n",
      Tail
      ));
			   ;
    return EFI_INVALID_PARAMETER;
  }

  if (IniParseString((CONST CHAR8 *)FileBuffer, TestHandler, &Config) < 0
            || Config->Name == NULL) {
                return EFI_INVALID_PARAMETER;

  }

  return EFI_SUCCESS;
#endif
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
  EfiEventGroupSignal (&gEfiEndOfDxeEventGroupGuid);

  DEBUG ((DEBUG_INFO,"\n\n-------INI Parser----\n"));
  //IniParser ();
  //
  // Dispatch deferred images after EndOfDxe event.
  //
  EfiBootManagerDispatchDeferredImages ();

  //
  // Locate the PCI root bridges and make the PCI bus driver connect each,
  // non-recursively. This will produce a number of child handles with PciIo on
  // them.
  //
  FilterAndProcess (&gEfiPciRootBridgeIoProtocolGuid, NULL, Connect);

  //
  // Ensure that USB is initialized by connecting the PCI root bridge so
  // that the xHCI PCI controller gets enumerated.
  //
  FilterAndProcess (&gEfiUsb2HcProtocolGuid, NULL, Connect);

  //
  // Find all display class PCI devices (using the handles from the previous
  // step), and connect them non-recursively. This should produce a number of
  // child handles with GOPs on them.
  //
  FilterAndProcess (&gEfiPciIoProtocolGuid, IsPciDisplay, Connect);

  //
  // Now add the device path of all handles with GOP on them to ConOut and
  // ErrOut.
  //
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
  EfiBootManagerUpdateConsoleVariable (
    ConIn,
    (EFI_DEVICE_PATH_PROTOCOL *)&mUsbKeyboard,
    NULL
    );

  //
  // Add the hardcoded serial console device path to ConIn, ConOut, ErrOut.
  //
  // ASSERT (FixedPcdGet8 (PcdDefaultTerminalType) == 4);
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
  UINTN                         FirmwareVerLength;
  EFI_INPUT_KEY                 Key;

  FirmwareVerLength = StrLen (PcdGetPtr (PcdFirmwareVersionString));
  //
  // Show the splash screen.
  //
  Status = BootLogoEnableLogo ();

  //
  // Connect the rest of the devices.
  //
  EfiBootManagerConnectAll ();

  //
  // Enumerate all possible boot options, then filter and reorder them.
  //
  EfiBootManagerRefreshAllBootOption ();

  //
  // Register UEFI Shell
  //
  Key.ScanCode    = SCAN_NULL;
  Key.UnicodeChar = L's';
  PlatformRegisterFvBootOption (
    &gUefiShellFileGuid,
    L"UEFI Shell",
    LOAD_OPTION_ACTIVE,
    &Key);
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
  EFI_STATUS                    Status;
  EFI_BOOT_MANAGER_LOAD_OPTION  BootManagerMenu;
  EFI_BOOT_MANAGER_LOAD_OPTION  *BootOptions;
  UINTN                         OldBootOptionCount;
  UINTN                         NewBootOptionCount;

  //
  // Record the total number of boot configured boot options
  //
  BootOptions = EfiBootManagerGetLoadOptions (
                  &OldBootOptionCount,
                  LoadOptionTypeBoot
                  );
  EfiBootManagerFreeLoadOptions (BootOptions, OldBootOptionCount);

  //
  // Connect all devices, and regenerate all boot options
  //
  EfiBootManagerConnectAll ();
  EfiBootManagerRefreshAllBootOption ();

  //
  // Boot the 'UEFI Shell' by default.
  //
  PlatformBootFvBootOption (
    &gUefiShellFileGuid,
    L"UEFI Shell (default)"
    );

  //
  // Record the updated number of boot configured boot options
  //
  BootOptions = EfiBootManagerGetLoadOptions (
                  &NewBootOptionCount,
                  LoadOptionTypeBoot
                  );
  EfiBootManagerFreeLoadOptions (BootOptions, NewBootOptionCount);

  //
  // If the number of configured boot options has changed, reboot
  // the system so the new boot options will be taken into account
  // while executing the ordinary BDS bootflow sequence.
  // *Unless* persistent varstore is being emulated, since we would
  // then end up in an endless reboot loop.
  //
  if (!PcdGetBool (PcdEmuVariableNvModeEnable)) {
    if (NewBootOptionCount != OldBootOptionCount) {
      DEBUG ((
        DEBUG_WARN,
        "%a: rebooting after refreshing all boot options\n",
        __func__
        ));
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
  }

  Status = EfiBootManagerGetBootManagerMenu (&BootManagerMenu);
  if (EFI_ERROR (Status)) {
    return;
  }

  for ( ; ;) {
    EfiBootManagerBoot (&BootManagerMenu);
  }
}
