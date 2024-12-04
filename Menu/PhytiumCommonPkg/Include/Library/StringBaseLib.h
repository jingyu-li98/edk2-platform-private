/** @file
String Library Header File.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 2011 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef  STRING_BASE_LIB_H_
#define  STRING_BASE_LIB_H_

/**
  Parses and converts Unicode string to IP v4 address.
  The value will 64-bit.
  The string must be four decimal values 0-255 separated by dots.
  The string is parsed and format verified.

  @param[out]  Ip4Addr   pointer to the variable to store the value to.
  @param[in]  String    string that contains the value to parse and convert.


  @retval     EFI_SUCCESS                   Success.
  @retval     EFI_INVALID_PARAMETER         String contains invalid IP v4 format.
**/
EFI_STATUS
StrStringToIp4Adr (
  OUT EFI_IPv4_ADDRESS   *Ip4Addr,
  IN  CHAR16             *String
  );

/**
  Converts IP Address Buffer to String.

  @param[in,out]  Buffer   pointer to the variable to store the value to.
  @param[in]      Ipadd    string that contains the value to parse and convert.

**/
VOID
IpAddtoStr (
  IN OUT CHAR16        *Buffer,
  IN     UINT8         *Ipadd
  );

/**
  Set hii strings.

  @param[in]  HiiHandle    Hii handle.
  @param[in]  StrRef       String ID.
  @param[in]  sFormat      Format string.
  @param  ...              Variable argument list whose contents are accessed
                           based on the format string specified by Format.

  @retval     StringId.
**/
EFI_STRING_ID
BmcInfoSetHiiString (
  IN EFI_HII_HANDLE   HiiHandle,
  IN EFI_STRING_ID    StrRef,
  IN CHAR16           *sFormat,
  ...
  );

/**
  Parses and converts Unicode string to decimal value.
  The returned value is 64-bit.
  The string is expected in decimal format,
  the string is parsed and format verified.
  This function is missing from the libs. One day it maybe
  replaced with a lib function when it'll become available.

  @param[out]  Val      pointer to the variable to store the value to.
  @param[in]   String   string that contains the value to parse and convert.
  @param[out]  EndIdx   index on which the parsing stopped. It points to the
                        first character that was not part of the returned Val.
                        It's valid only if the function returns success.
                        It's optional and it could be NULL.

  @retval     EFI_SUCCESS                   Success.
  @retval     EFI_INVALID_PARAMETER         String is in unexpected format.
**/
EFI_STATUS
EfiStringToValue (
  OUT UINT64        *Val,
  IN  CHAR16        *String,
  OUT UINT8         *EndIdx OPTIONAL
  );

#endif

