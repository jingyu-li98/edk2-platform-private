/** @file
  POST Code operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/BaseLib.h>
#include <Library/BmcBaseLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PhytiumPostCodeLib.h>

POST_CODE_MESSAGE mPostCodeMessage[] = {
  /*CPU Progress Codes*/
  {0x1, "Power on"},
  {0x2, "PLL Init Start"},
  {0x3, "PLL Init Complete"},

  /*CPU Error Codes*/
  {0x20, "PLL Init Fail"},

  /*Memory Progress Codes*/
  {0x30, "Memory initialization: Serial Presence Detect (SPD) data reading"},
  {0x31, "Memory initialization: Memory presence detection"},
  {0x32, "Memory initialization: Get memory parameter table"},
  {0x33, "Memory initialization: Start Training"},
  {0x34, "Memory initialization: Complete Training"},

  /*Memory Error Codes*/
  {0x40, "Memory initialization error: SPD reading has failed"},
  {0x41, "Memory initialization error: No usable memory detected"},
  {0x42, "Memory initialization error: Not Find memory parameter table"},
  {0x43, "Memory initialization error: Train Fail"},

  /*PCIE Progress Codes*/
  {0x50, "PCIE Initialization: Get Parameter table"},
  {0x51, "PCIE Initialization: Start Init"},
  {0x52, "PCIE Initialization: Complete Init"},

  /*PCIE Error Codes*/
  {0x60, "PCIE Initialization Error: Not Find Parameter table"},
  {0x61, "PCIE Initialization Error: Init Fail."},

  /*C2C Interconnection Progress Codes*/
  {0x70, "C2C Initialization: Get Parameter table"},
  {0x71, "C2C Initialization: Start Init"},
  {0x72, "C2C Initialization: Complete Init"},

  /*C2C Interconnection Error Codes*/
  {0x80, "C2C Initialization Error: Get Parameter table Fail"},
  {0x81, "C2C Initialization Error: Init Fail"},

  /*Secure Progress Codes*/
  {0x90, "Cryptography Verify Start"},
  {0x91, "Cryptography Verify Complete"},
  {0x92, "Cryptography Verify Successful"},
  {0x93, "Cryptography Hash Start"},
  {0x94, "Cryptography Hash Complete"},
  {0x95, "Cryptography Hash Compare Successful"},

  /*Secure Error Codes*/
  {0xa0, "Cryptography Verify Fail"},
  {0xa1, "Cryptography Hash Compare Fail"},

  /*Software Progress Codes*/
  {0xb0, "SEC is Started"},
  {0xb1, "PEI Core is Started"},
  {0xb2, "DXE is Started"},
  {0xb3, "BDS is Started"},
  {0xb4, "ExitBootServices"},

  /*Software Error Codes*/

};


/**
  Pass the post code information to the BMC by KCS.

  @param[in]  PostCode      Code of power on self test information.
  @param[in]  LocationCode  Code used to locate the phase in which
                            the error occurred.

**/
VOID
PostCodeKcs(
  IN UINT8  PostCode,
  IN UINT8  LocationCode
  )
{
  UINT64  Port80;
  UINT64  Port81;

  //
  //Get the output port address
  //
  Port80 = PcdGet64 (PcdKcsBaseAddress) + 0x80;
  Port81 = PcdGet64 (PcdKcsBaseAddress) + 0x81;

  //
  //Pass the Post and Location Code by KCS
  //
  MmioWrite8(Port81, PostCode);
  MmioWrite8(Port80, LocationCode);
}

/**
  Debug the post code information to the by serial port.

  @param[in]  PoseCode        Code of power on self test information.
  @param[in]  SocketNumer    Socket number of error happened
  @param[in]  InternalNumer  Internal number of error happened, such as DIMM ,PCIE.

**/
VOID
PostCodeSerialPort(
  IN UINT8  PoseCode,
  IN UINT8  SocketNumer,
  IN UINT8  InternalNumer
  )
{
  UINT8  Index;
  UINTN  CharCount;
  UINT8  Description[100];
  UINT8  DescriptionDebug[100];
  CHAR8  AsciiInternalNumer[3];
  CHAR8  AsciiSocketNumer[2];

  AsciiInternalNumer[0] = (CHAR8)(InternalNumer / 10 % 10 + 0x30);
  AsciiInternalNumer[1] = (CHAR8)(InternalNumer % 10 + 0x30);
  AsciiInternalNumer[2] = '\0';
  AsciiSocketNumer[0] = (CHAR8)(SocketNumer + 0x30);
  AsciiSocketNumer[1] = '\0';

  ZeroMem(Description, 100);
  ZeroMem(DescriptionDebug, 100);

  //
  //Select the appropriate prompt based on the PoseCode
  //
  for (Index = 0; Index < sizeof(mPostCodeMessage)/sizeof(POST_CODE_MESSAGE); Index++) {
    if (mPostCodeMessage[Index].PostCode == PoseCode) {
      CopyMem (Description, mPostCodeMessage[Index].Description, AsciiStrLen (mPostCodeMessage[Index].Description));
      break;
    }
  }

  //
  //PostCode DEBUG
  //
  CharCount = AsciiSPrint ((CHAR8 *) DescriptionDebug, sizeof (Description), "POST CODE %a ", Description);
  SerialPortWrite (DescriptionDebug, CharCount);

  //
  //LocationCode DEBUG
  //
  CharCount = AsciiSPrint ((CHAR8 *) DescriptionDebug, sizeof (Description), "SocketNumer : %a InternalNumer : %a\n\r", AsciiSocketNumer, AsciiInternalNumer);
  SerialPortWrite (DescriptionDebug, CharCount);
}

/**
  Debug the post code information.

  @param[in]  PoseCode        Code of power on self test information.
  @param[in]  SocketNumber    Socket number of error happened
  @param[in]  InternalNumber  Internal number of error happened, such as DIMM ,PCIE.

**/
VOID
PostCode(
  IN UINT8  PoseCode,
  IN UINT8  SocketNumber,
  IN UINT8  InternalNumber
  )
{
  UINT8  PostCodePort;
  UINT8  LocationCode;

  PostCodePort = PcdGet8 (PcdPostCodeOutputPort);
  LocationCode = (SocketNumber << 5) | InternalNumber;

  //
  //Choose the debug port according to the pcd
  //
  if (PostCodePort & POST_CODE_KCS_DEBUG) {
    if (CheckBmcInPlace () == EFI_SUCCESS) {
      PostCodeKcs (PoseCode, LocationCode);
    }
  }
  if (PostCodePort & POST_CODE_SERIAL_DEBUG) {
     PostCodeSerialPort (PoseCode, SocketNumber, InternalNumber);
  }

}
