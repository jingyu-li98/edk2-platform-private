/*
Initialization oft the main interface.

Copyright (c) 2023, Phytium Technology Co., Ltd. All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include "SetupFormInit.h"
#include "InformationNv.h"

#include <Protocol/DiskInfo.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/Atapi.h>
#include <Library/DevicePathLib.h>
#include <Library/ArmSmcLib.h>
#include <Protocol/Cpu.h>
#include <Protocol/PciIo.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HiiLib.h>
#include <Library/DebugLib.h>
#include <Library/TimeBaseLib.h>
//#include "FrontPage.h"
#include <Guid/MdeModuleHii.h>
#include <Library/EcLib.h>
#include <Library/MmInterface.h>
#include <Library/MmSpec.h>


EFI_GUID   gInformtionSetFormsetGuid = INFORMATION_SET_FORMSET_GUID;
#define _MYSTR(a)         #a
#define MYTKN2STR(a)      _MYSTR(a)

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
  Set hii strings.

  @param[in]  HiiHandle    Hii handle.
  @param[in]  StrRef       String ID.
  @param[in]  sFormat      Format string.
  @param[in]  ...          Variable argument list whose contents are accessed
                           based on the format string specified by Format.

  @retval     StringId.
**/
STATIC
EFI_STRING_ID
SetHiiString (
  IN EFI_HII_HANDLE   HiiHandle,
  IN EFI_STRING_ID    StrRef,
  IN CHAR16           *SFormat, ...
  )
{
  STATIC CHAR16 Str[1024];
  VA_LIST       Marker;
  EFI_STRING_ID StringId;

  VA_START (Marker, SFormat);
  UnicodeVSPrint (Str, sizeof (Str),  SFormat, Marker);
  VA_END (Marker);

  StringId = HiiSetString (HiiHandle, StrRef, Str, NULL);

  return StringId;
}

/**
  Get Hii String.

  @param[in]  Token      String ID.
  @param[in]  HiiHandle  Hii Handle.

  @retval     String.
**/
CHAR16 *
GetToken (
  IN  EFI_STRING_ID   Token,
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STRING  String;

  if (HiiHandle == NULL) {
    return NULL;
  }

  String = HiiGetString (HiiHandle, Token, NULL);
  if (String == NULL) {
    String = AllocateCopyPool (sizeof (L"!"), L"!");
    ASSERT (String != NULL);
  }

  return (CHAR16 *) String;
}

/**
  Get srr
**/
STATIC
UINT16
SmbiosGetStrOffset (
  IN CONST SMBIOS_STRUCTURE *Hdr,
  IN       UINT8            Index
  )
{
  CONST UINT8 *PData8;
  UINT8       Ii;

  if (Index == 0) {
    return 0;
  }

  PData8  = (UINT8*)Hdr;
  PData8 += Hdr->Length;

  Ii = 1;
  while (Ii != Index){
    while (*PData8 != 0) {
      PData8++;
    }

    if (PData8[1] == 0){     // if next byte of a string end is NULL, type end.
      break;
    }

    PData8++;
    Ii++;
  }
  if (Ii == Index) {
    return (UINT16) (PData8 - (UINT8*) Hdr);
  } else {
    return 0;
  }
}

STATIC
CHAR8 *
SmbiosGetStringInTypeByIndex(
  IN SMBIOS_STRUCTURE_POINTER Hdr,
  IN SMBIOS_TABLE_STRING      StrIndex
  )
{
  CHAR8  *Str8;
  if (StrIndex == 0) {
    return "";
  }
  Str8 = (CHAR8*) (Hdr.Raw + SmbiosGetStrOffset (Hdr.Hdr, StrIndex));
  return Str8;
}

/**
  Get bios information and set formset.

  @param[in]  Handle    Hii handle.
  @param[in]  Smbios    A pointer to EFI_SMBIOS_PROTOCOL.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetBiosInfo(
  IN EFI_HII_HANDLE       Handle,
  IN EFI_SMBIOS_PROTOCOL  *Smbios
  )
{
  EFI_STATUS  Status;
  CHAR8       *BiosVen;

  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TYPE           SmbiosType;
  EFI_SMBIOS_TABLE_HEADER   *SmbiosHdr;
  SMBIOS_STRUCTURE_POINTER  P;

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  SmbiosType = EFI_SMBIOS_TYPE_BIOS_INFORMATION;
  Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  P.Hdr = SmbiosHdr;

  //bios vendor
  BiosVen = SmbiosGetStringInTypeByIndex (P, P.Type0->Vendor);
  SetHiiString (
    Handle,
    STRING_TOKEN(STR_BIOS_VENDOR_VALUE),
    L"%a",
    BiosVen
  );

  return EFI_SUCCESS;
}

/**
  Get vios release time.

  @param[out]    Time    Bios release time.
**/
STATIC
VOID
GetReleaseTime (
  OUT EFI_TIME *Time
  )
{
  CONST CHAR8  *ReleaseDate = __DATE__;
  CONST CHAR8  *ReleaseTime = __TIME__;
  UINTN        Index;

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
  Get bios version and build time. Then set them to formset.

  @param[in]  Handle    Hii handle.
**/
VOID
GetBiosVersionBuildTime (
  IN EFI_HII_HANDLE    Handle
  )
{
  CHAR8       BiosVerStr[100];
  CHAR8       BiosDate[100];
  EFI_TIME    Time = {0};

  //bios version
  CONST CHAR16 *BiosVer = (CHAR16 *) FixedPcdGetPtr (PcdFirmwareVersionString);
  AsciiSPrint (
    BiosVerStr,
    sizeof (BiosVerStr),
    "%s",
    BiosVer
    );
  DEBUG ((DEBUG_INFO, "BIOS VERSION : %s\n", BiosVer));
  SetHiiString (
    Handle,
    STRING_TOKEN(STR_BIOS_VERSION_VALUE),
    L"%a",
    BiosVerStr
  );
  //bios date
  GetReleaseTime (&Time);
  AsciiSPrint (
    BiosDate,
    sizeof (BiosDate),
    "%t",
    &Time
    );
  SetHiiString (
    Handle,
    STRING_TOKEN(STR_BIOS_BUILD_TIME_VALUE),
    L"%a",
    BiosDate
  );
}

/**
  Get PBF version and set formset.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetPBFInfo(
  IN EFI_HII_HANDLE    Handle
  )
{
  ARM_SMC_ARGS              ArmSmcArgs;
  UINT16                    MajorVer;
  UINT16                    MinorVer;

  ArmSmcArgs.Arg0 = 0x82000001;
  ArmCallSmc (&ArmSmcArgs);
  // bit[16:31] Major version, BIT[0:15] Minor version
  MajorVer = (UINT16) (ArmSmcArgs.Arg0 >> 16);
  MinorVer = (UINT16) (ArmSmcArgs.Arg0);
  SetHiiString (
    Handle,
    STRING_TOKEN(STR_PBF_VERSION_VALUE),
    L"%d.%d",
    MajorVer,
    MinorVer
  );

  return EFI_SUCCESS;
}

/**
  Get and update dynamic information, such as X100 information and Ec.

  @param[in]  Handle    Hii hanlde.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetDynamicInfo (
  IN EFI_HII_HANDLE  Handle
  )
{
  VOID                           *StartOpCodeHandle;
  VOID                           *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL             *StartLabel;
  EFI_IFR_GUID_LABEL             *EndLabel;
  UINT16                         StrRef;
  EFI_STATUS                     Status;
  UINTN                          NumOfPciHandles;
  EFI_HANDLE                     *HandleBuffer;
  EFI_PCI_IO_PROTOCOL            *PciIo;
  UINT32                         Index;
  UINT16                         DeviceID;
  UINT16                         VendorID;
  UINT8                          SeVersion;
  UINT8                          ThirdVersion;
  UINT32                         Utc;
  EFI_TIME                       BuildTime;
  GET_VERSION_INFO_CONTENT_RESP  MmVersion;

  NumOfPciHandles = 0;
  StrRef = 0;
  SeVersion = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumOfPciHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_DYNAMIC;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_END;

  //
  //Ec Version: todo
  //
  //get x100 se soft version
  for (Index = 0; Index < NumOfPciHandles; Index ++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x0,
                   1,
                   &VendorID
                   );
      PciIo->Pci.Read (
                   PciIo,
                   EfiPciIoWidthUint16,
                   0x2,
                   1,
                   &DeviceID
                   );
      if ((DeviceID == 0xdc20) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x2f,
                     1,
                     &SeVersion
                     );
        DEBUG ((DEBUG_INFO, "X100 SE Soft Version : 0x%02x\n", SeVersion));
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint8,
                     0x2e,
                     1,
                     &ThirdVersion
                     );
        DEBUG ((DEBUG_INFO, "X100 SE Soft Tag Version : 0x%02x\n", ThirdVersion));
        StrRef = SetHiiString (
                   Handle,
                   0,
                   L"V%d.%d.%d",
                   (SeVersion >> 4) & 0xF,
                   (SeVersion >> 0) & 0xF,
                   ThirdVersion
                   );

        HiiCreateTextOpCode (
          StartOpCodeHandle,
          STRING_TOKEN (STR_X100_SE_VERSION),
          STRING_TOKEN (STR_EMPTY),
          StrRef
        );
      }
      if ((DeviceID == 0xdc35) && (VendorID == 0x1db7)) {
        PciIo->Pci.Read (
                     PciIo,
                     EfiPciIoWidthUint32,
                     0x2c,
                     1,
                     &Utc
                     );
        DEBUG ((DEBUG_INFO, "X100 SE build time utc : 0x%08x\n", Utc));
        Utc += 8 * 60 *60;
        EpochToEfiTime ((UINTN)Utc, &BuildTime);
        StrRef = SetHiiString (
                   Handle,
                   0,
                   L"%02d/%02d/%04d %02d:%02d:%02d",
                   BuildTime.Month,
                   BuildTime.Day,
                   BuildTime.Year,
                   BuildTime.Hour,
                   BuildTime.Minute,
                   BuildTime.Second
                   );
        DEBUG ((DEBUG_INFO, "SE Build Time : Year : %d\nMonth : %d\nDay : %d\nHour : %d\nMinute : %d\nSecond : %d\n",
                    BuildTime.Year, BuildTime.Month, BuildTime.Day, BuildTime.Hour, BuildTime.Minute, BuildTime.Second));
        HiiCreateTextOpCode (
          StartOpCodeHandle,
          STRING_TOKEN (STR_X100_SE_BUILDTIME),
          STRING_TOKEN (STR_EMPTY),
          StrRef
        );
      }
    }
  }
  //
  //STMM Version
  //
  ZeroMem (&MmVersion ,sizeof (GET_VERSION_INFO_CONTENT_RESP));
  Status = GetVersionInfoFromMm (&MmVersion);
  if (EFI_ERROR (Status) || (MmVersion.Result != StmmHandlerSuccess)) {
    DEBUG ((DEBUG_ERROR, "Get Mm Version %r, %d\n", Status, MmVersion.Result));
  } else {
    //StrRef = SetHiiString (
    //           Handle,
    //           0,
    //           L"%d.%02d",
    //           MmVersion.MainVersion,
    //           MmVersion.SubVersion
    //           );
    //DEBUG ((DEBUG_INFO, "Mm Version : %d.%d, StrRef = %x\n", MmVersion.MainVersion, MmVersion.SubVersion, StrRef));
    //UINT8 *Temp;
    //Temp = HiiCreateTextOpCode (
    //  StartOpCodeHandle,
    //  STRING_TOKEN (STR_MM_VERSION),
    //  STRING_TOKEN (STR_EMPTY),
    //  StrRef
    //);
    StrRef = SetHiiString (
               Handle,
               0,
               L"%a",
               MmVersion.Description
               );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      STRING_TOKEN (STR_MM_VERSION),
      STRING_TOKEN (STR_EMPTY),
      StrRef
    );
  }

  if(HandleBuffer != NULL){
    FreePool(HandleBuffer);
  }
  //Status = HiiUpdateForm (Handle, &gFrontPageFormSetGuid, FRONT_PAGE_FORM_ID, StartOpCodeHandle, EndOpCodeHandle);
  //Status = HiiUpdateForm (Handle, NULL, FRONT_PAGE_FORM_ID, StartOpCodeHandle, EndOpCodeHandle);
  Status = HiiUpdateForm (Handle, &gInformtionSetFormsetGuid, INFO_PAGE_FORM_ID, StartOpCodeHandle, EndOpCodeHandle);
  Status = HiiUpdateForm (Handle, NULL, INFO_PAGE_FORM_ID, StartOpCodeHandle, EndOpCodeHandle);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  return EFI_SUCCESS;
}

/**
  Get baseboard information from smbios and set formset.

  @param[in]  Handle    Hii handle.
  @param[in]  Smbios    A pointer to EFI_SMBIOS_PROTOCOL.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetBaseboradInfo (
  IN EFI_HII_HANDLE         Handle,
  IN EFI_SMBIOS_PROTOCOL    *Smbios
  )
{
  EFI_STATUS                Status;
  CHAR8                     *BaseboardProduct;
  CHAR8                     *BaseboardMfy;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TYPE           SmbiosType;
  EFI_SMBIOS_TABLE_HEADER   *SmbiosHdr;
  SMBIOS_STRUCTURE_POINTER  P;

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  SmbiosType = SMBIOS_TYPE_BASEBOARD_INFORMATION;
  Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  P.Hdr = SmbiosHdr;

  //baseboard Product
  BaseboardProduct = SmbiosGetStringInTypeByIndex (P, P.Type2->ProductName);
  SetHiiString (
    Handle,
    STRING_TOKEN (STR_BASEBOARD_PRODUCT_VALUE),
    L"%a",
    BaseboardProduct
  );

  //bios version
  BaseboardMfy = SmbiosGetStringInTypeByIndex (P, P.Type2->Manufacturer);
  SetHiiString (
    Handle,
    STRING_TOKEN (STR_BASEBOARD_MANUFACTORY_VALUE),
    L"%a",
    BaseboardMfy
  );

  return EFI_SUCCESS;
}

/**
  Get processor information from smbios and set formset.

  @param[in]  Handle    Hii handle.
  @param[in]  Smbios    A pointer to EFI_SMBIOS_PROTOCOL.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetProcessorInfo (
  IN EFI_HII_HANDLE         Handle,
  IN EFI_SMBIOS_PROTOCOL    *Smbios
  )
{
  EFI_STATUS                Status;
  CHAR8                     *ProcessorName;
  UINT16                    ProcessorFreq;
  UINT32                    CoreCounts;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TYPE           SmbiosType;
  EFI_SMBIOS_TABLE_HEADER   *SmbiosHdr;
  SMBIOS_STRUCTURE_POINTER  P;

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  SmbiosType = SMBIOS_TYPE_PROCESSOR_INFORMATION;
  Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  P.Hdr = SmbiosHdr;

  //Processor Version(Name)
  ProcessorName = SmbiosGetStringInTypeByIndex (P, P.Type4->ProcessorVersion);
  SetHiiString (
    Handle,
    STRING_TOKEN (STR_PROCESSOR_MODEL_VALUE),
    L"%a",
    ProcessorName
  );

  //processor freq
  ProcessorFreq = P.Type4->CurrentSpeed;
  SetHiiString (
    Handle,
      STRING_TOKEN (STR_PROCESSOR_FREQUENCY_VALUE),
    L"%d MHz",
    ProcessorFreq
    );

  //core counts
  CoreCounts = P.Type4->CoreCount;
  while (1) {
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    P.Hdr = SmbiosHdr;
    CoreCounts += P.Type4->CoreCount;
  }
  SetHiiString (
    Handle,
      STRING_TOKEN (STR_PROCESSOR_CORE_COUNTS_VALUE),
    L"%d",
    CoreCounts
    );

  return EFI_SUCCESS;
}

/**
  Get memory information from smbios and set formset.

  @param[in]  Handle    Hii handle.
  @param[in]  Smbios    A pointer to EFI_SMBIOS_PROTOCOL.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
GetMemoryInfo (
  IN EFI_HII_HANDLE         Handle,
  IN EFI_SMBIOS_PROTOCOL    *Smbios
  )
{
  EFI_STATUS                Status;
  UINT32                    MemDevNum;
  UINT32                    MemDevTotal;
  UINT32                    MemSpeed;
  UINT32                    MemTotalSize;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TYPE           SmbiosType;
  EFI_SMBIOS_TABLE_HEADER   *SmbiosHdr;
  SMBIOS_STRUCTURE_POINTER  P;

  //Memory Device Number
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  SmbiosType = SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY;
  Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  P.Hdr = SmbiosHdr;
  MemDevTotal = P.Type16->NumberOfMemoryDevices;
  while (1) {
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    P.Hdr = SmbiosHdr;
    MemDevTotal += P.Type16->NumberOfMemoryDevices;
  }

  //Memory Total Size and Memory Speed
  MemTotalSize = 0;
  MemSpeed = 0;
  MemDevNum = 0;
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  SmbiosType = SMBIOS_TYPE_MEMORY_DEVICE;
  while (1) {
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, &SmbiosType, &SmbiosHdr, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    P.Hdr = SmbiosHdr;
    //Mem Speed
    if (P.Type17->Size != 0) {
      MemSpeed = P.Type17->ConfiguredMemoryClockSpeed;
      MemDevNum++;
    }
    if (P.Type17->Size == 0x7FFF) {
      MemTotalSize += P.Type17->ExtendedSize;
    }
    else {
      MemTotalSize += P.Type17->Size;
    }
  }

  SetHiiString (
    Handle,
    STRING_TOKEN (STR_MEMORY_NUMBER_VALUE),
    L"%d",
    MemDevNum
  );

  SetHiiString (
    Handle,
    STRING_TOKEN (STR_MEMORY_SPEED_VALUE),
    L"%d MHz",
    MemSpeed
  );
//zhjmxl memery size > 16G ,use GB 
 if(MemTotalSize < 16385)
 {
  SetHiiString (
    Handle,
    STRING_TOKEN (STR_MEMORY_TOTAL_SIZE_VALUE),
    L"%d MB",
    MemTotalSize
  );
 }
 else
 {
  SetHiiString (
    Handle,
    STRING_TOKEN (STR_MEMORY_TOTAL_SIZE_VALUE),
    L"%d GB",
    (MemTotalSize/1024)
  );
 }

  return EFI_SUCCESS;
}

/**
  Main Form Init.

  @param[in]  Handle    Hii handle.

  @retval     EFI_SUCCESS    Success.
  @retval     Other          Failed.
**/
EFI_STATUS
MainFormInit (
  IN EFI_HII_HANDLE    Handle
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  Status = EFI_SUCCESS;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);
  ASSERT_EFI_ERROR (Status);

  GetBiosInfo (Handle, Smbios);
  GetBiosVersionBuildTime (Handle);
  GetPBFInfo (Handle);
  GetDynamicInfo (Handle);
  GetBaseboradInfo (Handle, Smbios);
  GetProcessorInfo (Handle, Smbios);
  GetMemoryInfo (Handle, Smbios);

  return Status;
}
