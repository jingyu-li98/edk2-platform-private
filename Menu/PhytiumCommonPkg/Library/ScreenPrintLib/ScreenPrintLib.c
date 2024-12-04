
#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#include <IndustryStandard/Bmp.h>


/**

  Init BMP file header.

  @param  Bmp             The pointer for BMP header.
  @param  Width           Width in pixel of BMP
  @param  Height          Height in pixel of BMP

  @retval None

**/
STATIC
VOID
InitBmpHeader (
  BMP_IMAGE_HEADER     *Bmp,
  UINTN                Width,
  UINTN                Height
  )
{
  UINTN                OneLineSize;

  OneLineSize = Width * 3;
  if ((OneLineSize % 4) != 0) {
    OneLineSize = OneLineSize + (4 - (OneLineSize % 4));
  }
  Bmp->CharB = 'B';
  Bmp->CharM = 'M';
  Bmp->Size = sizeof (BMP_IMAGE_HEADER) + OneLineSize * Height;
  Bmp->Reserved[0] = 0;
  Bmp->Reserved[1] = 0;
  Bmp->ImageOffset = sizeof (BMP_IMAGE_HEADER);
  Bmp->HeaderSize = sizeof (BMP_IMAGE_HEADER) - 
                    OFFSET_OF (BMP_IMAGE_HEADER, HeaderSize);
  Bmp->PixelWidth = Width;
  Bmp->PixelHeight = Height;
  Bmp->Planes = 1;
  Bmp->BitPerPixel = 24;
  Bmp->CompressionType = 0;
  Bmp->ImageSize = OneLineSize * Height;
  Bmp->XPixelsPerMeter = 0;
  Bmp->YPixelsPerMeter = 0;
  Bmp->NumberOfColors = 0;
  Bmp->ImportantColors = 0;
}
/**
   
  Check the handle is USB device or not

  @param  Handle                 The handle to be checked

  @retval TRUE                   It is USB device
  @retval FALSE                  It is not USB device

**/
STATIC
BOOLEAN
SaveBmpIsUsbDevice (
  IN EFI_HANDLE             Handle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath
                  );
  ASSERT_EFI_ERROR (Status);

  while (!IsDevicePathEnd (DevicePath)) {
    if (DevicePath->Type == MESSAGING_DEVICE_PATH &&
        DevicePath->SubType == MSG_USB_DP) {
      return TRUE;
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }

  return FALSE;
}
/**
   
  Find all simple file system protocol installed on USB devices
      
  @param  UsbFsNumber            The number of simple file system protocol installed on 
                                 USB devices.

  @retval The pointers to simple file system buffer, this buffer is allocated by this function, 
          and should be freed by its caller.

**/
STATIC
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **
SaveBmpFindUsbFs (
  OUT UINTN                            *UsbFsNumber
  )
{
  EFI_STATUS                          Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL     *FsInterface;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL     **FsBuffer;
  EFI_HANDLE                          *Handles;
  UINTN                               NoHandles;
  UINTN                               Index;
  UINTN                               Index2;

  Handles = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NoHandles,
                  &Handles
                  );
  if (EFI_ERROR (Status) || Handles == NULL || NoHandles == 0) {
    return NULL;
  }

  FsBuffer = NULL;
  *UsbFsNumber = 0;
  for (Index = 0; Index < NoHandles; Index++) {
    if (SaveBmpIsUsbDevice (Handles[Index])) {
      (*UsbFsNumber)++;
    }
  }

  if ((*UsbFsNumber) == 0) {
    goto Done;
  }

  FsBuffer = (EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **)
             AllocatePool (sizeof (VOID *) * (*UsbFsNumber));
  if (FsBuffer == NULL) {
    goto Done;
  }
  for (Index = 0, Index2 = 0; Index < NoHandles; Index++) {
    if (SaveBmpIsUsbDevice (Handles[Index])) {
      Status = gBS->HandleProtocol (
                      Handles[Index],
                      &gEfiSimpleFileSystemProtocolGuid,
                      (VOID **) &FsInterface
                      );
      ASSERT_EFI_ERROR (Status);
      DEBUG ((EFI_D_INFO, "Find Fs: 0x%08X\n", FsInterface));
      FsBuffer[Index2++] = FsInterface;
      ASSERT (Index2 <= (*UsbFsNumber));
    }
  }

Done:
  if (Handles != NULL) {
    FreePool (Handles);
  }
  return FsBuffer;
}
/**

  Open file and get file size by "gEfiSimpleFileSystemProtocol".

  @param  RootIo          The pointer for Root file.
  @param  DevicePath2     The unicode string for file path.
  @param  FileIo          The pointer for file has been opened
  &param  FileSize        File size in bytes

  @retval EFI_SUCCESS     Open success.
  @retval Ohters          Open failed.

**/
STATIC
EFI_STATUS
SaveBmpOpenFile (
  IN  EFI_FILE           *RootIo,
  IN  CHAR16             *FilePath,
  OUT EFI_FILE           **FileIo,
  IN  UINTN              FileSize
  )
{
  EFI_STATUS      Status;
  EFI_FILE_INFO   *FileInfo = NULL;
  UINTN           FileInfoSize = 0;

  Status = RootIo->Open (
                     RootIo, 
                     FileIo,
                     FilePath,
                     EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | 
                     EFI_FILE_MODE_READ, 
                     0
                     );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Can't open \"%s\"\n", FilePath));
    return Status;
  }
  Status = (*FileIo)->GetInfo (
                        (*FileIo), 
                        &gEfiFileInfoGuid, 
                        &FileInfoSize, 
                        FileInfo
                        );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileInfo = AllocatePool (FileInfoSize);
    if (FileInfo == NULL) {
      (*FileIo)->Close ((*FileIo));
      DEBUG ((EFI_D_ERROR, "Allocate file info error for \"%s\"\n", FilePath));
      return EFI_OUT_OF_RESOURCES;
    }
    Status = (*FileIo)->GetInfo (
                          (*FileIo), 
                          &gEfiFileInfoGuid, 
                          &FileInfoSize, 
                          FileInfo
                          );
  }
  if (!EFI_ERROR (Status)) {
    FileInfo->FileSize = FileSize;
    Status = (*FileIo)->SetInfo (
                          (*FileIo), 
                          &gEfiFileInfoGuid, 
                          FileInfoSize, 
                          FileInfo
                          );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Set file info error for \"%s\"\n", FilePath));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Get file info error for \"%s\"\n", FilePath));
  }

  if (EFI_ERROR (Status)) {
    (*FileIo)->Close ((*FileIo));
  }
  if (FileInfo != NULL) {
    FreePool (FileInfo);
  }
  return Status;
}
/**

  Save BMP file to USB file system

  @param  Bmp               The pointer for BMP file.

  @retval EFI_SUCCESS       Save success.
  @retval EFI_NO_MEDIA      No USB file system.
  @retval EFI_DEVICE_ERROR  Save file to USB file system error.

**/
STATIC
EFI_STATUS
SaveBmpFile (
  BMP_IMAGE_HEADER     *Bmp
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **UsbFs = NULL;
  UINTN                           UsbFsNumber = 0;
  EFI_FILE                        *RootIo;
  EFI_FILE                        *FileIo; 
  CHAR16                          FileDir[] = L"\\BIOS xxxx-xx-xx xx-xx-xx.bmp";
  UINTN                           FileSize = Bmp->Size;
  EFI_TIME                        SystemTime;
  UINTN                           Index;

  Status = gRT->GetTime (&SystemTime, NULL);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  UnicodeSPrint (
    FileDir,
    sizeof (FileDir),
    L"\\BIOS %04d-%02d-%02d %02d-%02d-%02d.bmp",
    SystemTime.Year,
    SystemTime.Month,
    SystemTime.Day,
    SystemTime.Hour,
    SystemTime.Minute,
    SystemTime.Second
    );
  DEBUG ((EFI_D_INFO, "BMP file path: \"%s\"\n", FileDir));

  UsbFs = SaveBmpFindUsbFs (&UsbFsNumber);
  DEBUG ((EFI_D_INFO, "Find %d USB file system in SaveBmpFile\n", UsbFsNumber));
  if (UsbFs == NULL || UsbFsNumber == 0) {
    return EFI_NO_MEDIA;
  }

  for (Index = 0; Index < UsbFsNumber; Index++) {
    DEBUG ((EFI_D_INFO, "UsbFs[%d] = 0x%08X\n", Index, UsbFs[Index]));
    //
    // Open RootIo
    //
    Status = UsbFs[Index]->OpenVolume (UsbFs[Index], &RootIo);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Open Volume error, Status = %r\n", Status));
      continue;
    }
    //
    // Open File
    //
    Status = SaveBmpOpenFile (RootIo, FileDir, &FileIo, FileSize);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Open File error, Status = %r\n", Status));
      continue;
    }
    //
    // Write file
    //
    Status = FileIo->Write (FileIo, &FileSize, Bmp);
    FileIo->Close (FileIo);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Write File error, Status = %r\n", Status));
    } else {
      break;
    }
  }

  if (EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;
  }
  if (UsbFs != NULL) {
    FreePool (UsbFs);
  }
  DEBUG ((EFI_D_INFO, "SaveBmpFile return, Status = %r\n", Status));
  return Status;
}

/**

  Print screen and save as BMP file to USB file system

  @param  None.

  @retval EFI_SUCCESS           Save success.
  @retval EFI_NOT_READY         GOP is not ready.
  @retval EFI_NO_MEDIA          No USB file system.
  @retval EFI_DEVICE_ERROR      Save file or print screen error.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to allocate buffer.

**/
EFI_STATUS
EFIAPI
ScreenPrintToBmp (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *Gop;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *ModeInfo;
  UINTN                                 ModeInfoSize;
  BMP_IMAGE_HEADER                      *BmpHeader = NULL;
  UINTN                                 BmpSize;
  UINTN                                 BmpOneLineSize;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer = NULL;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Blt;
  UINTN                                 HorizontalResolution;
  UINTN                                 VerticalResolution;
  UINTN                                 HeightIndex;
  UINTN                                 WidthIndex;
  UINTN                                 ImageIndex;
  UINT8                                 *ImageBase;
  UINT8                                 *Image;

  //
  // Get GOP Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **) &Gop
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Open GOP protocol error\n"));
    Status = EFI_NOT_READY;
    goto Done;
  }
  DEBUG ((EFI_D_INFO, "GOP Mode number: %d\n", Gop->Mode->Mode));

  //
  // Get GOP Mode information
  //
  Status = Gop->QueryMode (
                  Gop,
                  Gop->Mode->Mode,
                  &ModeInfoSize,
                  &ModeInfo
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;
    goto Done;
  }
  HorizontalResolution = ModeInfo->HorizontalResolution;
  VerticalResolution = ModeInfo->VerticalResolution;
  DEBUG ((
    EFI_D_INFO, 
    "Mode Resolution: %d * %d\n", 
    HorizontalResolution,
    VerticalResolution
    ));

  //
  // Allocate buffer for Blt data and BMP file
  //
  BltBuffer = AllocatePool (
                HorizontalResolution * VerticalResolution *
                sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                );
  if (BltBuffer == NULL) {
    DEBUG ((EFI_D_ERROR, "Out of memory when allocating BltBuffer\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  BmpOneLineSize = HorizontalResolution * 3;
  if ((BmpOneLineSize % 4) != 0) {
    BmpOneLineSize = BmpOneLineSize + (4 - (BmpOneLineSize % 4));
  }
  BmpSize = sizeof (BMP_IMAGE_HEADER) + BmpOneLineSize * VerticalResolution;
  BmpHeader = AllocateZeroPool (BmpSize);
  if (BmpHeader == NULL) {
    DEBUG ((EFI_D_ERROR, "Out of memory when allocating Bmp\n"));
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Init BMP file header
  //
  InitBmpHeader (BmpHeader, HorizontalResolution, VerticalResolution);

  //
  // Get Blt data from GOP
  //
  Status = Gop->Blt (
                  Gop,
                  BltBuffer,
                  EfiBltVideoToBltBuffer,
                  0,
                  0,
                  0,
                  0,
                  HorizontalResolution,
                  VerticalResolution,
                  HorizontalResolution * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;
    goto Done;
  }

  //
  // Convert Blt data to BMP image
  //
  ImageBase = ((UINT8 *) BmpHeader) + sizeof (BMP_IMAGE_HEADER);
  Image = ImageBase;
  for (HeightIndex = 0; HeightIndex < VerticalResolution; HeightIndex++) {
    Blt = &BltBuffer[
             (VerticalResolution - HeightIndex - 1) * HorizontalResolution
             ];
    for (
      WidthIndex = 0; 
      WidthIndex < HorizontalResolution; 
      WidthIndex++, Image++, Blt++) {
      *Image++ = Blt->Blue;
      *Image++ = Blt->Green;
      *Image = Blt->Red;
    }
    ImageIndex = (UINTN) (Image - ImageBase);
    if ((ImageIndex % 4) != 0) {
      Image = Image + (4 - (ImageIndex % 4));
    }
  }

  //
  // Save BMP file to USB file system
  //
  Status = SaveBmpFile (BmpHeader);

Done :
  if (BltBuffer != NULL) {
    FreePool (BltBuffer);
  }
  if (BmpHeader != NULL) {
    FreePool (BmpHeader);
  }

  return Status;
}

