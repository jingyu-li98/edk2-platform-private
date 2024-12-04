/** @file
Provides Phytium stmm management handle service.

Copyright (C)  2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmServicesTableLib.h>
#include <Library/MmSpec.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimeBaseLib.h>

typedef struct  _MONTH_DESCRIPTION {
  CONST CHAR8* MonthStr;
  UINT32       MonthInt;
} MONTH_DESCRIPTION;

MONTH_DESCRIPTION gMonthDescription[] = {
  { "Jan", 1 },
  { "Feb", 2 },
  { "Mar", 3 },
  { "Apr", 4 },
  { "May", 5 },
  { "Jun", 6 },
  { "Jul", 7 },
  { "Aug", 8 },
  { "Sep", 9 },
  { "Oct", 10 },
  { "Nov", 11 },
  { "Dec", 12 },
  { "???", 1 },  // Use 1 as default month
};

/**
  Get the time of right now.

  @param[out] Time  The Time sturct to be stored.

**/
VOID
GetReleaseTime (
  OUT EFI_TIME *Time
  )
{
  UINTN        Index;
  CONST CHAR8  *ReleaseDate = __DATE__;
  CONST CHAR8  *ReleaseTime = __TIME__;

  for (Index = 0; Index < 12; Index++) {
    if (0 == AsciiStrnCmp (ReleaseDate, gMonthDescription[Index].MonthStr, 3)) {
      break;
    }
  }
  Time->Month = gMonthDescription[Index].MonthInt;
  Time->Day = AsciiStrDecimalToUintn (ReleaseDate + 4);
  Time->Year = AsciiStrDecimalToUintn (ReleaseDate + 7);
  Time->Hour = AsciiStrDecimalToUintn (ReleaseTime);
  Time->Minute = AsciiStrDecimalToUintn (ReleaseTime + 3);
  Time->Second = AsciiStrDecimalToUintn (ReleaseTime + 6);

  return;
}

/**
  Generate stmm version information to communication buffer.

  @param [in]    Buffer    A pointer to communication buffer.

  @retral    0    Success
**/
UINT32
GenerateVersionInfo (
  IN  OUT  UINT8     *Buffer
  )
{
  GET_VERSION_INFO_CONTENT_RESP  Resp;
  EFI_TIME                       Time;
  CHAR8                          Version[64];
  UINTN                          CharCount;

  CONST CHAR16 *ReleaseString = (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVersionString);

  ZeroMem (&Resp, sizeof (GET_VERSION_INFO_CONTENT_RESP));
  GetReleaseTime (&Time);
  Resp.Result = StmmHandlerSuccess;
  //
  //Main version and sub version is a test parameter.
  //
  Resp.MainVersion = 1;
  Resp.SubVersion = 0;
  Resp.BuildTime = (UINT32) EfiTimeToEpoch (&Time);
  CharCount = AsciiSPrint (
                       Version,
                       sizeof (Version),
                       "%s %t",
                       ReleaseString,
                       &Time
                       );
  DEBUG ((DEBUG_INFO, "CharCount : %d\n", CharCount));
  DEBUG ((DEBUG_INFO, "Version : %a\n", Version));
  DEBUG ((DEBUG_INFO, "[0] - %02x\n", Version[0]));
  DEBUG ((DEBUG_INFO, "[1] - %02x\n", Version[1]));
  DEBUG ((DEBUG_INFO, "[2] - %02x\n", Version[2]));
  DEBUG ((DEBUG_INFO, "[3] - %02x\n", Version[3]));
  DEBUG ((DEBUG_INFO, "[4] - %02x\n", Version[4]));
  DEBUG ((DEBUG_INFO, "[5] - %02x\n", Version[5]));
  CopyMem (Resp.Description, Version, CharCount);
  CopyMem (Buffer, &Resp, sizeof (GET_VERSION_INFO_CONTENT_RESP));
  DEBUG ((DEBUG_INFO, "Version : %a\n", Resp.Description));

  return StmmHandlerSuccess;
}

/**
  Management sub event handle. The source buffer data comes from the content of
  the MM communicate protocol. For the protocol content, refer to Chapter 6.3.4
  of the Phytium MM Interface Specification.

  @param [in]      SubId      Sub event ID, refer to Chapter 6.3.4 of the Phytium
                              MM Interface Specification.
  @param [in,out]  Buffer     The content section in MM communicate buffer, refer
                              to Chapter 6.3.4 of the Phytium MM Interface
                              Specification.
  @param [in,out]  Length     Length of content.

  @retval    EFI_SUCCESS      Success.
  @retval    Other            Failed.

**/
UINT32
MmManagementHandle (
  IN       UINT16  SubId,
  IN  OUT  UINT8   *Buffer,
  IN  OUT  UINT32  *Length
  )
{
  UINT32                          Status;

  DEBUG ((DEBUG_INFO, "Check Step Sub ID : %02x\n", SubId));
  Status = -1;

  switch (SubId) {
  //
  //04-01
  //
  case MmVersionInfoGet:
    Status = GenerateVersionInfo (Buffer);
    break;
  default:
    break;
  }

  CopyMem (Buffer, &Status, 4);

  return EFI_SUCCESS;
}

/**
  Management event handle. The source buffer data comes from the content of the MM
  communicate protocol. For the protocol content, refer to Chapter 6.3.4 of the
  Phytium MM Interface Specification.

  @param[in]     DispatchHandle  The unique handle assigned to this handler by
                                 MmiHandlerRegister().
  @param[in]     RegisterContext Points to an optional handler context which was
                                 specified when the handler was registered.
  @param[in, out] CommBuffer     A pointer to a collection of data in memory that
                                 will be conveyed from a non-MM environment into
                                 an MM environment.
  @param[in, out] CommBufferSize The size of the CommBuffer.

  @retval       EFI_SUCCESS    Success.
  @retval       Other          Failed.
**/
EFI_STATUS
EFIAPI
PhytiumStmmManageHandler (
  IN     EFI_HANDLE   DispatchHandle,
  IN     CONST VOID   *RegisterContext,
  IN OUT VOID         *CommBuffer,
  IN OUT UINTN        *CommBufferSize
  )
{
  UINT32                  Status;
  UINT8                   *ContentBuffer;
  MM_COMM_CONTENT_HEADER  Header;

  Status = EFI_SUCCESS;
  if (*CommBufferSize < 8) {
    Status = StmmHandlerHeaderError;
    goto ProcExit;
  }
  CopyMem (&Header, (UINT8*) CommBuffer, sizeof (MM_COMM_CONTENT_HEADER));
  Status = gMmst->MmAllocatePool (
                    EfiRuntimeServicesData,
                    Header.Length,
                    (VOID **)&ContentBuffer
                    );
  ASSERT_EFI_ERROR (Status);
  CopyMem (ContentBuffer, CommBuffer + 8, Header.Length);
  switch (Header.MainId) {
  case MmManagement:
    MmManagementHandle (Header.SubId, ContentBuffer, &Header.Length);
    Status = EFI_SUCCESS;
    break;
  default:
    Status = StmmHandlerFuncIDUnsupported;
    break;
  }
  CopyMem ((UINT8 *)CommBuffer, &Header, sizeof (MM_COMM_CONTENT_HEADER));
  CopyMem ((UINT8 *)CommBuffer + 8, ContentBuffer, Header.Length);
  *CommBufferSize = Header.Length + 8;

ProcExit:
  if (ContentBuffer != NULL) {
    gMmst->MmFreePool (ContentBuffer);
  }
  DEBUG ((DEBUG_INFO, "%a, end Status : %r\n", __FUNCTION__, Status));
  return Status;
}

/**
  Phytium service MM driver entry point.

  @param[in] ImageHandle    A handle for the image that is initializing this
                            driver
  @param[in] MmSystemTable  A pointer to the MM system table

  @retval EFI_SUCCESS       Phytium stmm service successfully initialized.
**/
EFI_STATUS
EFIAPI
PhytiumStmmManageHandleInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_MM_SYSTEM_TABLE  *MmSystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  PhytiumStmmHandle;
  CHAR8       Buffer[100];
  UINTN       CharCount;
  EFI_TIME    Time;

  //
  //version and build time print
  //
  CONST CHAR16 *ReleaseString = (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVersionString);
  GetReleaseTime (&Time);
  CharCount = AsciiSPrint (
    Buffer,
    sizeof (Buffer),
    "STMM Version %s %t\n",
    ReleaseString,
    &Time
    );
  SerialPortWrite ((UINT8 *) Buffer, CharCount);
  DEBUG ((DEBUG_INFO, "[0] - %02x\n", Buffer[0]));
  DEBUG ((DEBUG_INFO, "[1] - %02x\n", Buffer[1]));
  DEBUG ((DEBUG_INFO, "[2] - %02x\n", Buffer[2]));
  DEBUG ((DEBUG_INFO, "[3] - %02x\n", Buffer[3]));
  DEBUG ((DEBUG_INFO, "[4] - %02x\n", Buffer[4]));
  DEBUG ((DEBUG_INFO, "[5] - %02x\n", Buffer[5]));

  PhytiumStmmHandle = NULL;
  Status = gMmst->MmiHandlerRegister (
                    PhytiumStmmManageHandler,
                    &gEfiPhytiumStmmManageProtocolGuid,
                    &PhytiumStmmHandle
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
