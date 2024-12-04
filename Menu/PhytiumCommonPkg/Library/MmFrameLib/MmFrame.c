/** @file
Define phytium mm interface protocol according PHYTIUM MM SPEC.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmSpec.h>

/**
  Communication frame framing from UEFI to MM. Message must be large enough.

  @param[in]       Guid       MM event ID.
  @param[in]       Header     Event message header.
  @param[in]       Content    Event content.
  @param[in, out]  Message    Message after framing.
  @param[out]      OutLen     Length of message after framing.
**/
VOID
UefiToMmCommFraming (
  IN     EFI_GUID                Guid,
  IN     MM_COMM_CONTENT_HEADER  *Header,
  IN     UINT8                   *Content,
  IN OUT UINT8                   *Message,
  OUT    UINTN                   *OutLen
  )
{
  UINT32  Index;
  UINT64  InLen;

  DEBUG ((DEBUG_INFO, "Main ID : %x\n", Header->MainId));
  DEBUG ((DEBUG_INFO, "Sub ID : %x\n", Header->SubId));
  DEBUG ((DEBUG_INFO, "Length : %x\n", Header->Length));
  Index = 0;
  InLen = Header->Length + sizeof (MM_COMM_CONTENT_HEADER);
  CopyMem (Message + Index, &Guid, sizeof (EFI_GUID));
  Index += sizeof (EFI_GUID);
  CopyMem (Message + Index, &InLen, sizeof (UINT64));
  Index += sizeof (UINT64);
  CopyMem (Message + Index, (VOID *) Header, sizeof (MM_COMM_CONTENT_HEADER));
  Index += sizeof (MM_COMM_CONTENT_HEADER);
  CopyMem (Message + Index, Content, Header->Length);
  *OutLen = (UINTN) (Index + Header->Length);
}

/**
  Communication frame parse from MM to UEFI. Content must be large enough.

  @param[in]       Message    Message to parse.
  @param[out]      Header     Content header after parse.
  @param[out]      Content    content after parse.
**/
VOID
UefiToMmCommParse (
  IN   UINT8                   *Message,
  OUT  MM_COMM_CONTENT_HEADER  *Header,
  OUT  UINT8                   *Content
  )
{
  UINT32  Index;

  Index = 0;
  Index += sizeof (EFI_GUID) + sizeof (UINT64);
  CopyMem (Header, Message + Index, sizeof (MM_COMM_CONTENT_HEADER));

  Index += sizeof (MM_COMM_CONTENT_HEADER);
  CopyMem (Content, Message + Index, Header->Length);
}
