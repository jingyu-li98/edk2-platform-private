/** @file
Definition of String operation function.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>

#include <Protocol/HiiString.h>

#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/StringBaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>


#define IS_DIGIT(Ch)  (((Ch) >= L"0"[0]) && ((Ch) <= L"9"[0]))
//
// Length of temp string buffer to store value string.
//
#define CHARACTER_NUMBER_FOR_VALUE  30

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
  )
{
  STATIC CHAR16 s[1024];
  VA_LIST  Marker;
  EFI_STRING_ID    StringId;

  VA_START (Marker, sFormat);
  UnicodeVSPrint (s, sizeof (s),  sFormat, Marker);
  VA_END (Marker);

  StringId = HiiSetString (HiiHandle, StrRef, s, NULL);

  return StringId;
}

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
  )
{
  UINT8   Index;
  UINT64  TempVal;

  TempVal = 0;
  //
  // Iterate upto 20 digits, only so many could fit in the UINT64
  //
  for (Index = 0; Index <= 20; Index++) {
    //
    // test if the next character is not a digit
    //
    if (!IS_DIGIT (String[Index])) {
      //
      // If here, there is no more digits,
      // return with success if there was at least one to process
      //
      if (Index == 0) {
        break;
      }

      *Val = TempVal;

      if (EndIdx != NULL) {
        *EndIdx = Index;
      }

      return EFI_SUCCESS;
    }
    //
    // If here, there is a digit to process
    //
    TempVal = MultU64x32 (TempVal, 10);
    TempVal += String[Index] - L"0"[0];
  }
  //
  // if here, there was some sort of format error
  //
  return EFI_INVALID_PARAMETER;
}

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
  )
{
  EFI_STATUS        Status;

  EFI_IPv4_ADDRESS  RetVal;
  UINT64            TempVal;
  UINT8             Idx;
  UINT8             Index;

  //
  // Iterate the decimal values separated by dots
  //
  for (Index = 0; Index < 4; Index++) {
    //
    // get the value of a decimal
    //
    Status = EfiStringToValue (&TempVal, String, &Idx);
    if ((EFI_ERROR (Status)) || (TempVal > 255)) {
      break;
    }

    RetVal.Addr[Index] = (UINT8) TempVal;
    String += Idx;

    //
    // test if it is the last decimal value
    //
    if (Index == 3) {
      if (String[0] != L'\0') {
        //
        // the string must end with string termination character
        //
        break;
      }

      *Ip4Addr = RetVal;
      return EFI_SUCCESS;
    }
    //
    // Test for presence of a dot, it is required between the values
    //
    if (String++[0] != L'.') {
      break;
    }
  }

  return EFI_INVALID_PARAMETER;
}

/**
  Converts IP Address Buffer to String.

  @param[in,out]  Buffer   pointer to the variable to store the value to.
  @param[in]      Ipadd    string that contains the value to parse and convert.

**/
VOID
IpAddtoStr (
  IN OUT CHAR16        *Buffer,
  IN     UINT8         *Ipadd
  )
{
  UnicodeSPrint (Buffer, 2 * 16, L"%d.%d.%d.%d", Ipadd[0], Ipadd[1], Ipadd[2], Ipadd[3]);
}

