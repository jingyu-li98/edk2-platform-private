/** @file
The implementation of the OEM commands.

Copyright (C) 2022-2023, Phytium Technology Co., Ltd. All rights reserved<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/ArmSmcLib.h>

#include "BiosUpdate.h"
#include "UefiShellOemCommandsLib.h"

EFI_MTFTP4_TOKEN            Mtftp4Token;
EFI_NORFLASH_DRV_PROTOCOL   *mFlash = NULL;

// Path of the local file, Unicode encoded
CONST CHAR16                *mLocalFilePath;
/*
   Constant strings and definitions related to the message indicating the amount of
   progress in the dowloading of a TFTP file.
*/

// Frame for the progression slider
STATIC CONST CHAR16 mUpdateBiosProgressFrame[] = L"[                                        ]";

STATIC CONST CHAR16 mUpdateBiosProgressDelete[] = L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";


STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  {L"-i", TypeValue},
  {L"-l", TypeValue},
  {L"-r", TypeValue},
  {L"-c", TypeValue},
  {L"-t", TypeValue},
  {L"-s", TypeValue},
  {L"-w", TypeValue},
  {NULL , TypeMax}
  };



EFI_MTFTP4_CONFIG_DATA DefaultMtftp4ConfigData = {
  TRUE,                             // Use default setting
  { { 0, 0, 0, 0 } },               // StationIp         - Not relevant as UseDefaultSetting=TRUE
  { { 0, 0, 0, 0 } },               // SubnetMask        - Not relevant as UseDefaultSetting=TRUE
  0,                                // LocalPort         - Automatically assigned port number.
  { { 0, 0, 0, 0 } },               // GatewayIp         - Not relevant as UseDefaultSetting=TRUE
  { { 0, 0, 0, 0 } },               // ServerIp          - Not known yet
  69,                               // InitialServerPort - Standard TFTP server port
  6,                                // TryCount          - The number of times to transmit request packets and wait for a response.
  4                                 // TimeoutValue      - Retransmission timeout in seconds.
};


/**
  Function for 'MemCp' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).

  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).

  @retval EFI_SUCCESS       The entry point is executed successfully.

  @retval SHELL_INVALID_PARAMETER   There is an error with the parameter.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunMemCp (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status;
  SHELL_STATUS      ShellStatus;
  LIST_ENTRY        *Package;
  CHAR16            *ProblemParam;
  CONST CHAR16      *Param1;
  CONST CHAR16      *Param2;
  CONST CHAR16      *Param3;
  UINTN Destination;
  UINTN Source;
  UINTN Length;

  Status = ShellCommandLineParse (EmptyParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellOemHiiHandle, L"MemCp", ProblemParam);
      FreePool(ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  }

  //
  // check for "-?"
  //
  if (ShellCommandLineGetFlag(Package, L"-?")) {
    ASSERT(FALSE);
  } else if (ShellCommandLineGetRawValue(Package, 4) != NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), gShellOemHiiHandle, L"MemCp");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else if (ShellCommandLineGetRawValue(Package, 3) == NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW), gShellOemHiiHandle, L"MemCp");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else {
    //
    Param1 = ShellCommandLineGetRawValue (Package, 1);
    Param2 = ShellCommandLineGetRawValue (Package, 2);
    Param3 = ShellCommandLineGetRawValue (Package, 3);


    ShellConvertStringToUint64(Param1, &Destination, FALSE, FALSE);
    ShellConvertStringToUint64(Param2, &Source, FALSE, FALSE);
    ShellConvertStringToUint64(Param3, &Length, FALSE, FALSE);

    ShellPrintEx(-1, -1, L"Copy from [%x] to 0x[%x], Length:%d \r\n", Source, Destination, Length);
    gBS->CopyMem((VOID *)Destination, (VOID *)Source, Length);

    ShellPrintEx(-1, -1, L"Copy ok\r\n");

  }

  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  //
  // return the status
  //
  return (ShellStatus);

}

/**
  Function for 'Bootefi' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).

  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).

  @retval EFI_SUCCESS       The entry point is executed successfully.

  @retval SHELL_INVALID_PARAMETER   There is an error with the parameter.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunBootefi (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  CONST CHAR16      *BootArgs;
  UINTN             BootArgsSize;
  CHAR16            *ClibParam;
  EFI_DEVICE_PATH   *DevicePath;
  EFI_SHELL_PARAMETERS_PROTOCOL EfiShellParametersProtocol;
  UINTN             Index;
  UINT64            ImageAddress;
  CONST CHAR16      *ImageSize;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  EFI_HANDLE        MyImageHandle;
  LIST_ENTRY        *Package;
  CONST CHAR16      *Param1;
  CONST CHAR16      *ParamClib;
  UINTN             ParamNum;
  CHAR16            *ProblemParam;
  UINTN             Size;
  EFI_STATUS        Status;
  SHELL_STATUS      ShellStatus;

  BootArgsSize  = 0;
  ClibParam  = NULL;
  DevicePath    = NULL;
  ParamNum        = 0;
  ProblemParam  = NULL;
  MyImageHandle = NULL;
  ShellStatus   = SHELL_SUCCESS;

  ZeroMem (&EfiShellParametersProtocol, sizeof(EFI_SHELL_PARAMETERS_PROTOCOL));

  //
  // parse the command line
  //
  Status = ShellCommandLineParse (EmptyParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PROBLEM),
        gShellOemHiiHandle,
        L"Bootefi",
        ProblemParam
        );
      FreePool (ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  }

  //
  // check for "-?"
  //
  if (ShellCommandLineGetFlag (Package, L"-?")) {
    ASSERT (FALSE);
  } else if (ShellCommandLineGetRawValue (Package, 1) == NULL){
    ShellPrintHiiEx (
      -1,
      -1,
      NULL,
      STRING_TOKEN (STR_GEN_TOO_FEW),
      gShellOemHiiHandle,
      L"Bootefi"
      );
    ShellPrintEx(-1,
      -1,
      L"bootefi <address> (<clib> <(param1 ...paramx)>\r\n"
      );
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else {
    ParamNum = ShellCommandLineGetCount (Package);
    ShellPrintEx (-1, -1, L"ParamNum= %d\r\n", ParamNum);
    Param1 = ShellCommandLineGetRawValue (Package, 1);
    Status = ShellConvertStringToUint64 (Param1, &ImageAddress, FALSE, FALSE);
    if (EFI_ERROR(Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1
        );
      return SHELL_INVALID_PARAMETER;
    }

    ImageSize = ShellGetEnvironmentVariable (L"image_size");
    if (ImageSize == NULL) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_ENV_NOT_FOUND),
        gShellOemHiiHandle,
        L"Bootefi"
        );
      return SHELL_INVALID_PARAMETER;
    }

    Status = ShellConvertStringToUint64 (ImageSize, &Size, FALSE, FALSE);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1);
      return SHELL_INVALID_PARAMETER;
    }

    Status = gBS->LoadImage (
                    FALSE,
                    gImageHandle,
                    DevicePath,
                    (VOID*)ImageAddress,
                    Size,
                    &MyImageHandle
                    );
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1,
        -1,
        NULL,
        STRING_TOKEN (STR_GEN_LOAD_FILE_FAIL),
        gShellOemHiiHandle,
        L"Bootefi",
        Param1
        );
      return SHELL_LOAD_ERROR;
    }

    Status = gBS->HandleProtocol (
             MyImageHandle,
             &gEfiLoadedImageProtocolGuid,
             (VOID **)&LoadedImage
           );
    ASSERT_EFI_ERROR(Status);

    if (ParamNum < 3) {
      BootArgs = ShellGetEnvironmentVariable (L"bootargs");
      if (BootArgs != NULL)  {
        ShellPrintEx (-1, -1, L"BootArgs= %x\r\n", BootArgs);
        BootArgsSize = StrSize (BootArgs);
        LoadedImage->LoadOptions = (VOID *)BootArgs;
        LoadedImage->LoadOptionsSize = BootArgsSize;
        gBS->ReinstallProtocolInterface(
               MyImageHandle,
               &gEfiLoadedImageProtocolGuid,
               (VOID *)LoadedImage,
               (VOID *)LoadedImage
               );
      }
    }else if (ParamNum >= 3) {
      ParamClib = ShellCommandLineGetRawValue (Package, 2);
      ShellPrintEx (-1, -1, L"%s \n", ParamClib);
      if (StrCmp (ParamClib, L"clib") == 0) {
        ShellPrintEx(-1, -1, L"This is a Clib App.\n");

        Status = gBS->OpenProtocol (
                        gImageHandle,
                        &gEfiShellParametersProtocolGuid,
                        (VOID **)&EfiShellParametersProtocol,
                        gImageHandle,
                        NULL,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
        if (EFI_ERROR (Status)){
          ShellPrintEx (-1, -1, L"EfiShellParametersProtocol open failed.\r\n");
          return Status;
        }

        EfiShellParametersProtocol.Argc = (ParamNum - 2);
        EfiShellParametersProtocol.Argv = AllocatePool (
                                            sizeof (CHAR16 *) * 24
                                            );
        if (EfiShellParametersProtocol.Argv == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto UnloadImage;
        }

        ShellPrintEx (
          -1,
          -1,
          L"EfiShellParametersProtocol.Argc=%d\n",
          EfiShellParametersProtocol.Argc
          );

        for (Index = 0; Index < EfiShellParametersProtocol.Argc; Index++) {
          ClibParam = (CHAR16 *)ShellCommandLineGetRawValue (Package, Index + 2);
          EfiShellParametersProtocol.Argv[Index] = ClibParam;
        }

        Status = gBS->InstallProtocolInterface (
                        &MyImageHandle,
                        &gEfiShellParametersProtocolGuid,
                        EFI_NATIVE_INTERFACE,
                        &EfiShellParametersProtocol
                        );
        if (EFI_ERROR (Status)){
          ShellPrintEx (
            -1,
            -1,
            L"EfiShellParametersProtocol install failed\r\n"
            );
          return Status;
        }

        Status = gBS->StartImage (MyImageHandle, NULL, NULL);
        if (EFI_ERROR(Status)) {
          ShellPrintEx (-1, -1, L"StartImage failed.\r\n");
          return SHELL_INVALID_PARAMETER;
        }

        Status = gBS->UninstallProtocolInterface (
                    MyImageHandle,
                    &gEfiShellParametersProtocolGuid,
                    &EfiShellParametersProtocol
                    );
        if (EFI_ERROR(Status)) {
          ShellPrintEx (
            -1,
            -1,
            L"EfiShellParametersProtocol Uninstall failed.\r\n"
            );
          return SHELL_INVALID_PARAMETER;
        }

        // Free Argv.
        if (EfiShellParametersProtocol.Argv != NULL) {
          FreePool (EfiShellParametersProtocol.Argv);
        }
        goto Return;
      }else {
        ShellPrintEx (-1, -1, L"Clib parameter error.\r\n");
        goto Return;
      }
    }
    Status = gBS->StartImage (MyImageHandle, 0, NULL);
    if (EFI_ERROR (Status)) {
      ShellPrintEx (-1, -1, L"StartImage failed Status =%x.\r\n", Status);
      goto Return;
    }
  }

UnloadImage:
  gBS->UnloadImage (MyImageHandle);

Return:
  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  //
  // return the status
  //
  return (ShellStatus);
}



/**
  Create a child for the service identified by its service binding protocol GUID
  and get from the child the interface of the protocol identified by its GUID.

  @param[in]   ControllerHandle            Controller handle.
  @param[in]   ServiceBindingProtocolGuid  Service binding protocol GUID of the
                                           service to be created.
  @param[in]   ProtocolGuid                GUID of the protocol to be open.
  @param[out]  ChildHandle                 Address where the handler of the
                                           created child is returned. NULL is
                                           returned in case of error.
  @param[out]  Interface                   Address where a pointer to the
                                           protocol interface is returned in
                                           case of success.

  @return  EFI_SUCCESS  The child was created and the protocol opened.
  @return  Others       Either the creation of the child or the opening
                        of the protocol failed.
**/
STATIC
EFI_STATUS
CreateServiceChildAndOpenProtocol (
  IN   EFI_HANDLE  ControllerHandle,
  IN   EFI_GUID    *ServiceBindingProtocolGuid,
  IN   EFI_GUID    *ProtocolGuid,
  OUT  EFI_HANDLE  *ChildHandle,
  OUT  VOID        **Interface
  )
{
  EFI_STATUS  Status;

  *ChildHandle = NULL;
  Status = NetLibCreateServiceChild (
             ControllerHandle,
             gImageHandle,
             ServiceBindingProtocolGuid,
             ChildHandle
             );
  if (!EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    *ChildHandle,
                    ProtocolGuid,
                    Interface,
                    gImageHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      NetLibDestroyServiceChild (
        ControllerHandle,
        gImageHandle,
        ServiceBindingProtocolGuid,
        *ChildHandle
        );
      *ChildHandle = NULL;
    }
  }

  return Status;
}

/**
  Close the protocol identified by its GUID on the child handle of the service
  identified by its service binding protocol GUID, then destroy the child
  handle.

  @param[in]  ControllerHandle            Controller handle.
  @param[in]  ServiceBindingProtocolGuid  Service binding protocol GUID of the
                                          service to be destroyed.
  @param[in]  ProtocolGuid                GUID of the protocol to be closed.
  @param[in]  ChildHandle                 Handle of the child to be destroyed.

**/
STATIC
VOID
CloseProtocolAndDestroyServiceChild (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_GUID    *ServiceBindingProtocolGuid,
  IN  EFI_GUID    *ProtocolGuid,
  IN  EFI_HANDLE  ChildHandle
  )
{
  gBS->CloseProtocol (
         ChildHandle,
         ProtocolGuid,
         gImageHandle,
         ControllerHandle
         );

  NetLibDestroyServiceChild (
    ControllerHandle,
    gImageHandle,
    ServiceBindingProtocolGuid,
    ChildHandle
    );
}

/**
  Check and convert the UINT16 option values of the 'tftp' command

  @param[in]  ValueStr  Value as an Unicode encoded string
  @param[out] Value     UINT16 value

  @return     TRUE      The value was returned.
  @return     FALSE     A parsing error occured.
**/
STATIC
BOOLEAN
StringToUint16 (
  IN   CONST CHAR16  *ValueStr,
  OUT  UINT16        *Value
  )
{
  UINTN  Val;

  Val = ShellStrToUintn (ValueStr);
  if (Val > MAX_UINT16) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
      gShellOemHiiHandle, L"updatebios", ValueStr
    );
    return FALSE;
  }

  *Value = (UINT16)Val;
  return TRUE;
}

/**
  Get the name of the NIC.

  @param[in]   ControllerHandle  The network physical device handle.
  @param[in]   NicNumber         The network physical device number.
  @param[out]  NicName           Address where to store the NIC name.
                                 The memory area has to be at least
                                 IP4_CONFIG2_INTERFACE_INFO_NAME_LENGTH
                                 double byte wide.

  @return  EFI_SUCCESS  The name of the NIC was returned.
  @return  Others       The creation of the child for the Managed
                        Network Service failed or the opening of
                        the Managed Network Protocol failed or
                        the operational parameters for the
                        Managed Network Protocol could not be
                        read.
**/
STATIC
EFI_STATUS
GetNicName (
  IN   EFI_HANDLE  ControllerHandle,
  IN   UINTN       NicNumber,
  OUT  CHAR16      *NicName
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    MnpHandle;
  EFI_MANAGED_NETWORK_PROTOCOL  *Mnp;
  EFI_SIMPLE_NETWORK_MODE       SnpMode;

  Status = CreateServiceChildAndOpenProtocol (
             ControllerHandle,
             &gEfiManagedNetworkServiceBindingProtocolGuid,
             &gEfiManagedNetworkProtocolGuid,
             &MnpHandle,
             (VOID**)&Mnp
             );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Status = Mnp->GetModeData (Mnp, NULL, &SnpMode);
  if (EFI_ERROR (Status) && (Status != EFI_NOT_STARTED)) {
    goto Error;
  }

  UnicodeSPrint (
    NicName,
    IP4_CONFIG2_INTERFACE_INFO_NAME_LENGTH,
    SnpMode.IfType == NET_IFTYPE_ETHERNET ?
    L"eth%d" :
    L"unk%d" ,
    NicNumber
    );

  Status = EFI_SUCCESS;

Error:

  if (MnpHandle != NULL) {
    CloseProtocolAndDestroyServiceChild (
      ControllerHandle,
      &gEfiManagedNetworkServiceBindingProtocolGuid,
      &gEfiManagedNetworkProtocolGuid,
      MnpHandle
      );
  }

  return Status;
}

/**
  Worker function that gets the size in numbers of bytes of a file from a TFTP
  server before to download the file.

  @param[in]   Mtftp4    MTFTP4 protocol interface
  @param[in]   FilePath  Path of the file, ASCII encoded
  @param[out]  FileSize  Address where to store the file size in number of
                         bytes.

  @retval  EFI_SUCCESS      The size of the file was returned.
  @retval  EFI_UNSUPPORTED  The server does not support the "tsize" option.
  @retval  Others           Error when retrieving the information from the server
                            (see EFI_MTFTP4_PROTOCOL.GetInfo() status codes)
                            or error when parsing the response of the server.
**/
STATIC
EFI_STATUS
GetFileSize (
  IN   EFI_MTFTP4_PROTOCOL  *Mtftp4,
  IN   CONST CHAR8          *FilePath,
  OUT  UINTN                *FileSize
  )
{
  EFI_STATUS         Status;
  EFI_MTFTP4_OPTION  ReqOpt[1];
  EFI_MTFTP4_PACKET  *Packet;
  UINT32             PktLen;
  EFI_MTFTP4_OPTION  *TableOfOptions;
  EFI_MTFTP4_OPTION  *Option;
  UINT32             OptCnt;
  UINT8              OptBuf[128];

  ReqOpt[0].OptionStr = (UINT8*)"tsize";
  OptBuf[0] = '0';
  OptBuf[1] = 0;
  ReqOpt[0].ValueStr = OptBuf;

  Status = Mtftp4->GetInfo (
             Mtftp4,
             NULL,
             (UINT8*)FilePath,
             NULL,
             1,
             ReqOpt,
             &PktLen,
             &Packet
             );

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Status = Mtftp4->ParseOptions (
                     Mtftp4,
                     PktLen,
                     Packet,
                     (UINT32 *) &OptCnt,
                     &TableOfOptions
                     );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Option = TableOfOptions;
  while (OptCnt != 0) {
    if (AsciiStrnCmp ((CHAR8 *)Option->OptionStr, "tsize", 5) == 0) {
      *FileSize = AsciiStrDecimalToUintn ((CHAR8 *)Option->ValueStr);
      break;
    }
    OptCnt--;
    Option++;
  }
  FreePool (TableOfOptions);

  if (OptCnt == 0) {
    Status = EFI_UNSUPPORTED;
  }

Error :

  return Status;
}


/**
  Update the progress of a file download
  This procedure is called each time a new TFTP packet is received.

  @param[in]  This       MTFTP4 protocol interface
  @param[in]  Token      Parameters for the download of the file
  @param[in]  PacketLen  Length of the packet
  @param[in]  Packet     Address of the packet

  @retval  EFI_SUCCESS  All packets are accepted.

**/
STATIC
EFI_STATUS
EFIAPI
CheckPacket (
  IN EFI_MTFTP4_PROTOCOL  *This,
  IN EFI_MTFTP4_TOKEN     *Token,
  IN UINT16               PacketLen,
  IN EFI_MTFTP4_PACKET    *Packet
  )
{
  DOWNLOAD_CONTEXT  *Context;
  CHAR16            Progress[UPDATEBIOS_PROGRESS_MESSAGE_SIZE];
  UINTN             NbOfKb;
  UINTN             Index;
  UINTN             LastStep;
  UINTN             Step;
  UINTN             DownloadLen;
  EFI_STATUS        Status;

  if ((NTOHS (Packet->OpCode)) != EFI_MTFTP4_OPCODE_DATA) {
    return EFI_SUCCESS;
  }

  Context = (DOWNLOAD_CONTEXT*)Token->Context;

  //
  // The data in the packet are prepended with two UINT16 :
  // . OpCode = EFI_MTFTP4_OPCODE_DATA
  // . Block  = the number of this block of data
  //
  DownloadLen = (UINTN)PacketLen - sizeof (Packet->OpCode) - sizeof (Packet->Data.Block);


  if (Context->DownloadedNbOfBytes == 0) {
    ShellPrintEx (-1, -1, L"%s       0 Kb", mUpdateBiosProgressFrame);
  }

  Context->DownloadedNbOfBytes += DownloadLen;
  NbOfKb = Context->DownloadedNbOfBytes / 1024;

  Progress[0] = L'\0';
  LastStep  = (Context->LastReportedNbOfBytes * UPDATEBIOS_PROGRESS_SLIDER_STEPS) / Context->FileSize;
  Step      = (Context->DownloadedNbOfBytes * UPDATEBIOS_PROGRESS_SLIDER_STEPS) / Context->FileSize;

  if (Step <= LastStep) {
    return EFI_SUCCESS;
  }

  ShellPrintEx (-1, -1, L"%s", mUpdateBiosProgressDelete);

  Status = StrCpyS (Progress, UPDATEBIOS_PROGRESS_MESSAGE_SIZE, mUpdateBiosProgressFrame);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  for (Index = 1; Index < Step; Index++) {
    Progress[Index] = L'=';
  }
  Progress[Step] = L'>';

  UnicodeSPrint (
    Progress + (sizeof (mUpdateBiosProgressFrame) / sizeof (CHAR16)) - 1,
    sizeof (Progress) - sizeof (mUpdateBiosProgressFrame),
    L" %7d Kb",
    NbOfKb
    );
  Context->LastReportedNbOfBytes = Context->DownloadedNbOfBytes;

  ShellPrintEx (-1, -1, L"%s", Progress);

  return EFI_SUCCESS;
}


/**
  Worker function that download the data of a file from a TFTP server given
  the path of the file and its size.

  @param[in]   Mtftp4         MTFTP4 protocol interface
  @param[in]   FilePath       Path of the file, Unicode encoded
  @param[in]   AsciiFilePath  Path of the file, ASCII encoded
  @param[in]   FileSize       Size of the file in number of bytes
  @param[in]   BlockSize      Value of the TFTP blksize option
  @param[in]   WindowSize     Value of the TFTP window size option

  @retval  EFI_SUCCESS           The file was downloaded.
  @retval  EFI_OUT_OF_RESOURCES  A memory allocation failed.
  @retval  Others                The downloading of the file from the server failed
                                 (see EFI_MTFTP4_PROTOCOL.ReadFile() status codes).

**/
STATIC
EFI_STATUS
DownloadFile (
  IN   EFI_MTFTP4_PROTOCOL  *Mtftp4,
  IN   CONST CHAR16         *FilePath,
  IN   CONST CHAR8          *AsciiFilePath,
  IN   UINTN                FileSize,
  IN   UINT16               BlockSize,
  IN   UINT16               WindowSize
  )
{
  EFI_STATUS            Status;
  DOWNLOAD_CONTEXT      *TftpContext;
  UINT8                 BlksizeBuf[10];
  UINT8                 WindowsizeBuf[10];


  TftpContext = AllocatePool (sizeof (DOWNLOAD_CONTEXT));
  if (TftpContext == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  TftpContext->FileSize = FileSize;
  TftpContext->DownloadedNbOfBytes   = 0;
  TftpContext->LastReportedNbOfBytes = 0;

  Mtftp4Token.Filename    = (UINT8*)AsciiFilePath;
  Mtftp4Token.CheckPacket = CheckPacket;
  Mtftp4Token.Context     = (VOID*)TftpContext;
  Mtftp4Token.OptionCount = 0;
  Mtftp4Token.OptionList  = AllocatePool (sizeof (EFI_MTFTP4_OPTION) * 2);
  Mtftp4Token.BufferSize  = FileSize;
  if (Mtftp4Token.OptionList == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  if (BlockSize != MTFTP_DEFAULT_BLKSIZE) {
    Mtftp4Token.OptionList[Mtftp4Token.OptionCount].OptionStr = (UINT8 *) "blksize";
    AsciiSPrint ((CHAR8 *) BlksizeBuf, sizeof (BlksizeBuf), "%d", BlockSize);
    Mtftp4Token.OptionList[Mtftp4Token.OptionCount].ValueStr  = BlksizeBuf;
    Mtftp4Token.OptionCount ++;
  }

  if (WindowSize != MTFTP_DEFAULT_WINDOWSIZE) {
    Mtftp4Token.OptionList[Mtftp4Token.OptionCount].OptionStr = (UINT8 *) "windowsize";
    AsciiSPrint ((CHAR8 *) WindowsizeBuf, sizeof (WindowsizeBuf), "%d", WindowSize);
    Mtftp4Token.OptionList[Mtftp4Token.OptionCount].ValueStr  = WindowsizeBuf;
    Mtftp4Token.OptionCount ++;
  }

  ShellPrintHiiEx (
    -1, -1, NULL, STRING_TOKEN (STR_TFTP_DOWNLOADING),
    gShellOemHiiHandle, FilePath
    );
  //Print(L"Mtftp4Token.Buffer address %p\n",Mtftp4Token.Buffer);

  Status = Mtftp4->ReadFile (Mtftp4, &Mtftp4Token);
  ShellPrintHiiEx (
    -1, -1, NULL, STRING_TOKEN (STR_GEN_CRLF),
    gShellOemHiiHandle
    );
  Print(L"Mtftp4Token.Buffer address %p\n",Mtftp4Token.Buffer);



Error :
  if (TftpContext != NULL) {
    FreePool (TftpContext);
  }

  if (Mtftp4Token.OptionList != NULL) {
    FreePool (Mtftp4Token.OptionList);
  }

  return Status;
}

/**
  Find and create information in uni file.

  @param[in]    StringToken1    String token 1.
  @param[in]    StringToken2    String token 2.

**/
VOID
InvalidInformation (
  IN UINT16  StringToken1,
  IN UINT16  StringToken2
  )
{
  CHAR16         Str1[64];
  CHAR16         Str2[64];
  CHAR16         *HiiFormatString;
  EFI_INPUT_KEY  InputKey;
  UINTN          EventIndex;

  HiiFormatString = HiiGetString (gShellOemHiiHandle, StringToken1, NULL);
  UnicodeSPrint (Str1, sizeof (Str1), L"%s", HiiFormatString);
  if (HiiFormatString != NULL) {
    FreePool (HiiFormatString);
  }
  HiiFormatString = HiiGetString (gShellOemHiiHandle, StringToken2, NULL);
  UnicodeSPrint (Str2, sizeof (Str2), L"%s", HiiFormatString);
  if (HiiFormatString != NULL) {
    FreePool (HiiFormatString);
  }

  CreatePopUp (
    EFI_LIGHTGRAY | EFI_BACKGROUND_RED,
    NULL,
    Str1,
    Str2,
    NULL
    );

  while (1) {
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);
    if (InputKey.ScanCode == SCAN_NULL && InputKey.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    }
  }
}

/**
  Check the header of bios image.

  @param[in]    Buffer    A pointer to update bios data.
  @param[in]    FileSize  Size of bios.

  @retval    EFI_SUCCESS    Success.
             Other          Failed.
**/
EFI_STATUS
CheckBiosImage (
  IN UINT8        *Buffer,
  IN UINTN        FileSize
  )
{
  EFI_STATUS    Status;
  UINT64        SignOffset;
  UINT32        Signature;
  UINTN         Md5ContextSize;
  VOID          *Md5Ctx;
  UINT8         HashValue[16];
  UINT8         MD5Value[16];
  //
  // check BIOS.Signature.
  // BIOS header Signature offset is 0
  //
  Status = EFI_SUCCESS;
  SignOffset = 0;
  Signature  = *(UINT32*)(Buffer + SignOffset);
  SignOffset = 0x12;

  CopyMem(MD5Value, (Buffer + SignOffset), 16);
  //DEBUG((DEBUG_INFO,"the MD5Value0 1  is %x%x",MD5Value[0],MD5Value[1]));
  if (Signature != EFI_BIOS_HEAD_SIGNATURE) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((DEBUG_ERROR , "Bios Signature isn't right ,please check the bios if correct or not !\n"));
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_SIGNATURE_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

  Md5ContextSize = Md5GetContextSize ();
  Md5Ctx = AllocatePool (Md5ContextSize);
  if (Md5Ctx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  if (!Md5Init (Md5Ctx)) {
    goto ProExit;
  }
  if (!Md5Update (Md5Ctx,(Buffer+0x30), FileSize - 0x30)) {
    goto ProExit;
  }
  if (Md5Final (Md5Ctx, HashValue)) {
    Status = EFI_SUCCESS;
  }
  //DEBUG((DEBUG_INFO,"the HashValue 1  is %x%x",HashValue[0],HashValue[1]));

  if (CompareMem(HashValue, MD5Value, 16) == 0) {
    //DEBUG((DEBUG_INFO,"the HashValue is identical\n"));
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((DEBUG_ERROR,"the HashValue is not identical\n"));
    Status = EFI_INVALID_PARAMETER;
    InvalidInformation (STRING_TOKEN(STR_CHECK_BIOS_MD5_ERROR), STRING_TOKEN(STR_PRESS_ENTER_CONTINUE));
    goto ProExit;
  }

ProExit:
  return Status;
}

/**
  Update bios through qspi flash.

  @param[in]    Address    Address to update.
  @param[in]    Buffer     A pointer to update bios data.
  @param[in]    Size       Size of update bios to update.
  @param[in]    String     Start info to print.

  @retval       EFI_SUCCESS    Success.
                Other          Failed.
**/
EFI_STATUS
UpdateSpi (
  IN UINTN                     Address,
  IN UINT8                     *Buffer,
  IN UINTN                     Size,
  IN CHAR16                    *String
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       Count;
  VOID        *TempBuffer;

  TempBuffer = NULL;
  TempBuffer = AllocatePool (SIZE_64KB);
  Status = EFI_SUCCESS;
  Count  = (Size / SIZE_64KB);
  //
  // make sure Address & Size is 64k align
  //
  if (((Address % SIZE_64KB) >=0) || ((Size % SIZE_64KB) >=0) ) {
    Count ++;
    Address = Address & (~0xFFFF);
  }

  Print (L"%s%02d%%%", String, 1);
  for (Index = 0; Index < Count; Index ++) {
    if (TempBuffer != NULL) {
      mFlash->Read (Address + Index * SIZE_64KB, TempBuffer, SIZE_64KB);
      if (CompareMem (TempBuffer, Buffer + Index * SIZE_64KB, SIZE_64KB) == 0) {
        Print (L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
        continue;
      }
    }
    Status = mFlash->Erase (Address + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Status = mFlash->Write (Address + Index * SIZE_64KB, Buffer + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
  }
  Print(L"\r%s Success!\n",String);

ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }
  return Status;
}


/**
  Run bios update by network.

  @param[in] ImageHandle     The image handle.
  @param[in] SystemTable     The system table.

  @retval  SHELL_SUCCESS  Run bios update success.
  @retval  Other          Return error status.
**/
SHELL_STATUS
EFIAPI
ShellCommandRunBiosUpdate (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SHELL_STATUS            ShellStatus;
  EFI_STATUS              Status;
  LIST_ENTRY              *CheckPackage;
  CHAR16                  *ProblemParam;
  UINTN                   ParamCount;
  CONST CHAR16            *UserNicName;
  BOOLEAN                 NicFound;
  CONST CHAR16            *ValueStr;
  CONST CHAR16            *RemoteFilePath;
  CHAR8                   *AsciiRemoteFilePath;
  UINTN                   FilePathSize;
  CONST CHAR16            *Walker;
  EFI_MTFTP4_CONFIG_DATA  Mtftp4ConfigData;
  EFI_HANDLE              *Handles;
  UINTN                   HandleCount;
  UINTN                   NicNumber;
  CHAR16                  NicName[IP4_CONFIG2_INTERFACE_INFO_NAME_LENGTH];
  EFI_HANDLE              ControllerHandle;
  EFI_HANDLE              Mtftp4ChildHandle;
  EFI_MTFTP4_PROTOCOL     *Mtftp4;
  UINTN                   FileSize;
  UINT16                  BlockSize;
  UINT16                  WindowSize;
  UINTN                   VariableBase;
  UINT32                  VariableSize;
  UINT8                   *Biosaddress;


  ShellStatus         = SHELL_INVALID_PARAMETER;
  ProblemParam        = NULL;
  NicFound            = FALSE;
  AsciiRemoteFilePath = NULL;
  Handles             = NULL;
  FileSize            = 0;
  BlockSize           = MTFTP_DEFAULT_BLKSIZE;
  WindowSize          = MTFTP_DEFAULT_WINDOWSIZE;

  //
  // Initialize the Shell library (we must be in non-auto-init...)
  //
  Status = ShellInitialize ();
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return SHELL_ABORTED;
  }

  Status = gBS->LocateProtocol (&gSpiNorFlashProtocolGuid, NULL, (VOID*) &mFlash);
  if (EFI_ERROR(Status)) {
    Print(L"  Locate SpiFlashProtocol failed!\n");
    return SHELL_ABORTED;
  }
  //
  // Parse the command line.
  //
  Status = ShellCommandLineParse (ParamList, &CheckPackage, &ProblemParam, TRUE);
  if (EFI_ERROR (Status)) {
    if ((Status == EFI_VOLUME_CORRUPTED) &&
        (ProblemParam != NULL) ) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellOemHiiHandle,
        L"updatebios", ProblemParam
        );
      FreePool (ProblemParam);
    } else {
      ASSERT (FALSE);
    }
    goto Error;
  }

  //
  // Check the number of parameters
  //
  ParamCount = ShellCommandLineGetCount (CheckPackage);
  if (ParamCount > 4) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY),
      gShellOemHiiHandle, L"updatebios"
      );
    goto Error;
  }
  if (ParamCount < 3) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW),
      gShellOemHiiHandle, L"updatebios"
      );
    goto Error;
  }

  CopyMem (&Mtftp4ConfigData, &DefaultMtftp4ConfigData, sizeof (EFI_MTFTP4_CONFIG_DATA));

  //
  // Check the host IPv4 address
  //
  ValueStr = ShellCommandLineGetRawValue (CheckPackage, 1);
  Status = NetLibStrToIp4 (ValueStr, &Mtftp4ConfigData.ServerIp);
  if (EFI_ERROR (Status)) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
      gShellOemHiiHandle, L"updatebios", ValueStr
    );
    goto Error;
  }

  RemoteFilePath = ShellCommandLineGetRawValue (CheckPackage, 2);
  ASSERT(RemoteFilePath != NULL);
  FilePathSize = StrLen (RemoteFilePath) + 1;
  AsciiRemoteFilePath = AllocatePool (FilePathSize);
  if (AsciiRemoteFilePath == NULL) {
    ShellStatus = SHELL_OUT_OF_RESOURCES;
    goto Error;
  }
  UnicodeStrToAsciiStrS (RemoteFilePath, AsciiRemoteFilePath, FilePathSize);

  if (ParamCount == 4) {
    mLocalFilePath = ShellCommandLineGetRawValue (CheckPackage, 3);
  } else {
    Walker = RemoteFilePath + StrLen (RemoteFilePath);
    while ((--Walker) >= RemoteFilePath) {
      if ((*Walker == L'\\') ||
          (*Walker == L'/' )    ) {
        break;
      }
    }
    mLocalFilePath = Walker + 1;
  }

  //
  // Get the name of the Network Interface Card to be used if any.
  //
  UserNicName = ShellCommandLineGetValue (CheckPackage, L"-i");

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-l");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &Mtftp4ConfigData.LocalPort)) {
      goto Error;
    }
  }

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-r");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &Mtftp4ConfigData.InitialServerPort)) {
      goto Error;
    }
  }

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-c");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &Mtftp4ConfigData.TryCount)) {
      goto Error;
    }

    if (Mtftp4ConfigData.TryCount == 0) {
      Mtftp4ConfigData.TryCount = 6;
    }
  }

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-t");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &Mtftp4ConfigData.TimeoutValue)) {
      goto Error;
    }
    if (Mtftp4ConfigData.TimeoutValue == 0) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle, L"updatebios", ValueStr
      );
      goto Error;
    }
  }

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-s");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &BlockSize)) {
      goto Error;
    }
    if (BlockSize < MTFTP_MIN_BLKSIZE || BlockSize > MTFTP_MAX_BLKSIZE) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle, L"updatebios", ValueStr
      );
      goto Error;
    }
  }

  ValueStr = ShellCommandLineGetValue (CheckPackage, L"-w");
  if (ValueStr != NULL) {
    if (!StringToUint16 (ValueStr, &WindowSize)) {
      goto Error;
    }
    if (WindowSize < MTFTP_MIN_WINDOWSIZE || WindowSize > MTFTP_MAX_WINDOWSIZE) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV),
        gShellOemHiiHandle, L"updatebios", ValueStr
      );
      goto Error;
    }
  }

  //
  // Locate all MTFTP4 Service Binding protocols
  //
  ShellStatus = SHELL_NOT_FOUND;
  Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiManagedNetworkServiceBindingProtocolGuid,
                 NULL,
                 &HandleCount,
                 &Handles
                 );
  if (EFI_ERROR (Status) || (HandleCount == 0)) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_NO_NIC),
      gShellOemHiiHandle
    );
    goto Error;
  }

  for (NicNumber = 0;
       (NicNumber < HandleCount) && (ShellStatus != SHELL_SUCCESS);
       NicNumber++) {
    ControllerHandle = Handles[NicNumber];

    Status = GetNicName (ControllerHandle, NicNumber, NicName);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_NIC_NAME),
        gShellOemHiiHandle, NicNumber, Status
      );
      continue;
    }

    if (UserNicName != NULL) {
      if (StrCmp (NicName, UserNicName) != 0) {
        continue;
      }
      NicFound = TRUE;
    }

    Status = CreateServiceChildAndOpenProtocol (
               ControllerHandle,
               &gEfiMtftp4ServiceBindingProtocolGuid,
               &gEfiMtftp4ProtocolGuid,
               &Mtftp4ChildHandle,
               (VOID**)&Mtftp4
               );
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_OPEN_PROTOCOL),
        gShellOemHiiHandle, NicName, Status
      );
      continue;
    }

    Status = Mtftp4->Configure (Mtftp4, &Mtftp4ConfigData);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_CONFIGURE),
        gShellOemHiiHandle, NicName, Status
      );
      goto NextHandle;
    }

    Status = GetFileSize (Mtftp4, AsciiRemoteFilePath, &FileSize);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_FILE_SIZE),
        gShellOemHiiHandle, RemoteFilePath, NicName, Status
      );
      goto NextHandle;
    }

    Mtftp4Token.Buffer  = AllocatePool(FileSize);

    Print (L"Mtftp4Token.Buffer Allocate : %p , FileSize : %d\n",Mtftp4Token.Buffer, FileSize);

    Status = DownloadFile (Mtftp4, RemoteFilePath, AsciiRemoteFilePath, FileSize, BlockSize, WindowSize);
    if (EFI_ERROR (Status)) {
      ShellPrintHiiEx (
        -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_DOWNLOAD),
        gShellOemHiiHandle, RemoteFilePath, NicName, Status
      );
      goto NextHandle;
    }

  //check the bios head
  //
  Status = CheckBiosImage (Mtftp4Token.Buffer, FileSize);
  if (Status == EFI_SUCCESS) {
    Biosaddress = Mtftp4Token.Buffer + 0x30;
    FileSize    = FileSize - 0x30;
    Status = UpdateSpi (
                 0,
                 Biosaddress,
                 FileSize,
                 L"  Updating bios."
                 );
    VariableBase = PcdGet64(PcdFlashNvStorageVariableBase64);
    VariableSize = PcdGet32(PcdFlashNvStorageVariableSize);
    Status = mFlash->Erase (
                  VariableBase,
                  VariableSize
                  );
    SystemReboot();
    ShellStatus = SHELL_SUCCESS;
  }

    NextHandle:

    CloseProtocolAndDestroyServiceChild (
      ControllerHandle,
      &gEfiMtftp4ServiceBindingProtocolGuid,
      &gEfiMtftp4ProtocolGuid,
      Mtftp4ChildHandle
      );
  }

  if ((UserNicName != NULL) && (!NicFound)) {
    ShellPrintHiiEx (
      -1, -1, NULL, STRING_TOKEN (STR_TFTP_ERR_NIC_NOT_FOUND),
      gShellOemHiiHandle, UserNicName
    );
  }

  Error:

  ShellCommandLineFreeVarList (CheckPackage);
  if (AsciiRemoteFilePath != NULL) {
    FreePool (AsciiRemoteFilePath);
  }
  if (Handles != NULL) {
    FreePool (Handles);
  }

  if ((ShellStatus != SHELL_SUCCESS) && (EFI_ERROR(Status))) {
    ShellStatus = Status & ~MAX_BIT;
  }

  return ShellStatus;
}

/**
  Run every pxe option available.
**/
VOID
EFIAPI
Bootpxe (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINTN                           BootOrderSize;
  EFI_BOOT_MANAGER_LOAD_OPTION    Option;
  CHAR16                          OptionName[sizeof ("Boot####")];
  UINTN                           Index;
  UINT16                          *BootOrder;

  GetEfiGlobalVariable2 (L"BootOrder", (VOID **)&BootOrder, &BootOrderSize);
  if (BootOrder == NULL) {
    return;
  }

  for (Index = 0; Index < BootOrderSize / sizeof (UINT16); Index++) {
    UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", BootOrder[Index]);
    Status = EfiBootManagerVariableToLoadOption (OptionName, &Option);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Boot%04x is invalid option!\n", BootOrder[Index]));
      continue;
    }
    if (StrStr(Option.Description, L"PXE") != NULL || StrStr(Option.Description, L"MAC") != NULL) {
      EfiBootManagerBoot(&Option);
    }
  }
  DEBUG ((DEBUG_ERROR, "Boot PXE Error, please check\n"));
}


/**
  Function for 'boot' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).

  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).

  @retval EFI_SUCCESS       The entry point is executed successfully.

  @retval SHELL_INVALID_PARAMETER   There is an error with the parameter.

**/
SHELL_STATUS
EFIAPI
ShellCommandRunBoot (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status;
  SHELL_STATUS      ShellStatus;
  LIST_ENTRY        *Package;
  CHAR16            *ProblemParam;
  CONST CHAR16      *Param1;

  Status = ShellCommandLineParse (EmptyParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellOemHiiHandle, L"Boot", ProblemParam);
      FreePool(ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  }

  //
  // check for "-?"
  //
  if (ShellCommandLineGetFlag(Package, L"-?")) {
    ASSERT(FALSE);
  } else if (ShellCommandLineGetRawValue(Package, 2) != NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), gShellOemHiiHandle, L"Boot");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else if (ShellCommandLineGetRawValue(Package, 1) == NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW), gShellOemHiiHandle, L"Boot");
    ShellStatus = SHELL_INVALID_PARAMETER;
  } else {
    Param1 = ShellCommandLineGetRawValue (Package, 1);
    if (StrCmp (Param1, L"PXE") == 0 || StrCmp (Param1, L"pxe") == 0) {
      Bootpxe();
    } else {
      Print(L"Invaild boot parameter!\n");
    }
  }

  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  //
  // return the status
  //
  return (ShellStatus);

}
