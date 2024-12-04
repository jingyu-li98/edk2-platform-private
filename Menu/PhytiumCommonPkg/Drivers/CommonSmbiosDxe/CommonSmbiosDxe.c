/** @file
Phytium Platfomr Common Smbios driver
Note - Arm BBR ver 2.0 required and recommended SMBIOS structures:
  BIOS Information (Type 0)
  System Information (Type 1)
  Board Information (Type 2) - Recommended
  System Enclosure (Type 3)
  Processor Information (Type 4) - CPU Driver
  Cache Information (Type 7) - For cache that is external to processor
  Port Information (Type 8) - Recommended for platforms with physical ports
  System Slots (Type 9) - Required If system has slots
  OEM Strings (Type 11) - Recommended
  BIOS Language Information (Type 13) - Recommended
  Group Associations (Type 14) - Recommended (does not implement)
  System Event Log (Type 15) - Recommended (does not implement)
  Physical Memory Array (Type 16)
  Memory Device (Type 17) - For each socketed system-memory Device
  Memory Array Mapped Address (Type 19) - One per contiguous block per Physical Memroy Array
  System Boot Information (Type 32)
  IPMI Device Information (Type 38) - Required for platforms with IPMIv1.0 BMC Host Interface
  System Power Supplies (Type 39) - Recommended for server (does not implement)
  Onboard Devices Extended Information (Type 41) - Recommended
  Redfish Host Interface (Type 42) - Required for platforms supporting Redfish Host Interface (does not implement)

Copyright (c) 2017-2018, Andrey Warkentin <andrey.warkentin@gmail.com>
Copyright (c) 2013, Linaro.org
Copyright (c) 2012, Apple Inc. All rights reserved.<BR>
Copyright (c) Microsoft Corporation. All rights reserved.
Copyright (c) 2020, ARM Limited. All rights reserved.
Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Base.h>
#include <PiDxe.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>
#include <Guid/SmBios.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HobLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/PssiLib.h>
#include <Library/JsonLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimeBaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <PhytiumSmbiosHelper.h>

#define RDIMM_TYPE                1
#define UDIMM_TYPE                2
#define SODIMM_TYPE               3
#define LRDIMM_TYPE               4

extern
VOID
PlatSmbiosEntry(
  VOID
  );

VOID                        *mSmbiosInfo = NULL;
EDKII_JSON_VALUE            mJsonValue = NULL;
EDKII_JSON_OBJECT           mJsonObject = NULL;
CHAR8                       *UnknownString = "Unknown";
PHYTIUM_MEMORY_SMBIOS_INFO  *mMemorySmbiosInfo = NULL;

SPD_JEDEC_MANUFACTURER JEP106[] = {
  {0x8010, "NEC"},
  {0x802C, "Micron"},
  {0x803D, "Tektronix"},
  {0x8097, "Texas Instruments"},
  {0x80AD, "Hynix"},
  {0x80B3, "IDT"},
  {0x80CE, "Samsung"},
  {0x80C1, "Infineon"},
  {0x0194, "Smart"},
  {0x0198, "Kingston"},
  {0x029E, "Corsair"},
  {0x02C8, "Agilent"},
  {0x02FE, "Elpida"},
  {0x830B, "Nanya"},
  {0x0426, "MediaTek"},
  {0x0443, "Ramaxel"},
  {0x04B3, "Inphi"},
  {0x8551, "Qimonda"},
  {0x8557, "AENEON"},
  {0x8632, "LanQi"},
  {0x86F1, "innodisk"},
  {0x081A, "Unilc"},
  {0x8968, "Kimtigo"},
  {0x8A91, "CXMT"},
  {0x0BDC, "ASUS"}
  };

/**
 Convert a hex number to its ASCII code.

 @param [in]  Hex   Hex number to convert.
                    Must be 0 <= x < 16.

 @return The ASCII code corresponding to x.
         -1 if error.
**/
UINT8
EFIAPI
AsciiFromHex (
  IN  UINT8  Hex
  )
{
  if (Hex < 10) {
    return (UINT8)(Hex + '0');
  }

  if (Hex < 16) {
    return (UINT8)(Hex - 10 + 'A');
  }

  ASSERT (FALSE);
  return (UINT8)-1;
}

/**
 Get memory device manufacturer.

 @param [in]  MemModuleId    Module Manufacturer ID Code.
 @param [out] Manufacturer   Module Manufacturer string.

**/
VOID
GetManufacturer (
  IN  UINT16  MemModuleId,
  OUT CHAR8  *Manufacturer
)
{
  UINT32  Index;

  Index = 0;
  AsciiStrCpyS (Manufacturer, 8, "Unknown");

  for(Index = 0; Index < sizeof(JEP106)/sizeof(SPD_JEDEC_MANUFACTURER); Index++) {
    if(JEP106[Index].ManufacturerID == MemModuleId) {
      AsciiStrCpyS (Manufacturer, AsciiStrnSizeS (JEP106[Index].Name, 100), JEP106[Index].Name);
      break;
    }
  }
}

/***********************************************************************
        SMBIOS data definition  TYPE0  BIOS Information
***********************************************************************/
SMBIOS_TABLE_TYPE0 mBIOSInfoType0 = {
  { EFI_SMBIOS_TYPE_BIOS_INFORMATION, sizeof (SMBIOS_TABLE_TYPE0), 0 },
  1,                                                     // Vendor String
  2,                                                     // BiosVersion String
  (UINT16) (FixedPcdGet32 (PcdFdBaseAddress) / 0x10000), // BiosSegment
  3,                                                     // BiosReleaseDate String
  0,      // BiosSize (in 64KB)
  {                     // BiosCharacteristics
    0,    //  Reserved                          :2;  ///< Bits 0-1.
    0,    //  Unknown                           :1;
    0,    //  BiosCharacteristicsNotSupported   :1;
    0,    //  IsaIsSupported                    :1;
    0,    //  McaIsSupported                    :1;
    0,    //  EisaIsSupported                   :1;
    1,    //  PciIsSupported                    :1;
    0,    //  PcmciaIsSupported                 :1;
    0,    //  PlugAndPlayIsSupported            :1;
    0,    //  ApmIsSupported                    :1;
    1,    //  BiosIsUpgradable                  :1;
    0,    //  BiosShadowingAllowed              :1;
    0,    //  VlVesaIsSupported                 :1;
    0,    //  EscdSupportIsAvailable            :1;
    0,    //  BootFromCdIsSupported             :1;
    1,    //  SelectableBootIsSupported         :1;
    0,    //  RomBiosIsSocketed                 :1;
    0,    //  BootFromPcmciaIsSupported         :1;
    0,    //  EDDSpecificationIsSupported       :1;
    0,    //  JapaneseNecFloppyIsSupported      :1;
    0,    //  JapaneseToshibaFloppyIsSupported  :1;
    0,    //  Floppy525_360IsSupported          :1;
    0,    //  Floppy525_12IsSupported           :1;
    0,    //  Floppy35_720IsSupported           :1;
    0,    //  Floppy35_288IsSupported           :1;
    0,    //  PrintScreenIsSupported            :1;
    0,    //  Keyboard8042IsSupported           :1;
    0,    //  SerialIsSupported                 :1;
    0,    //  PrinterIsSupported                :1;
    0,    //  CgaMonoIsSupported                :1;
    0,    //  NecPc98                           :1;
    0     //  ReservedForVendor                 :32; ///< Bits 32-63. Bits 32-47 reserved for BIOS vendor
    ///< and bits 48-63 reserved for System Vendor.
  },
  {       // BIOSCharacteristicsExtensionBytes[]
    0x03, //  AcpiIsSupported                   :1;
          //  UsbLegacyIsSupported              :1;
          //  AgpIsSupported                    :1;
          //  I2OBootIsSupported                :1;
          //  Ls120BootIsSupported              :1;
          //  AtapiZipDriveBootIsSupported      :1;
          //  Boot1394IsSupported               :1;
          //  SmartBatteryIsSupported           :1;
          //  BIOSCharacteristicsExtensionBytes[1]
    0x0F, //  BiosBootSpecIsSupported              :1;
          //  FunctionKeyNetworkBootIsSupported    :1;
          //  TargetContentDistributionEnabled     :1;
          //  UefiSpecificationSupported           :1;
          //  VirtualMachineSupported              :1;
          //  ExtensionByte2Reserved               :3;
  },
  0,                       // SystemBiosMajorRelease
  0,                       // SystemBiosMinorRelease
  0,                       // EmbeddedControllerFirmwareMajorRelease
  0,                       // EmbeddedControllerFirmwareMinorRelease
  {0}                      //Extended BIOS ROM Size
};

CHAR8 mBiosVendor[128]  = "EDK2";
CHAR8 mBiosVersion[128] = "EDK2-DEV";
CHAR8 mBiosReleaseDate[12] = "00/00/0000";

CHAR8 *mBIOSInfoType0Strings[] = {
  mBiosVendor,              // Vendor
  mBiosVersion,             // Version
  mBiosReleaseDate,         // Release Date
  NULL
};

typedef struct {
  CHAR8 MonthNameStr[4]; // example "Jan", Compiler build date, month
  CHAR8 DigitStr[3];     // example "01", Smbios date format, month
} MonthStringDig;


MonthStringDig MonthMatch[12] = {
  { "Jan", "01" },
  { "Feb", "02" },
  { "Mar", "03" },
  { "Apr", "04" },
  { "May", "05" },
  { "Jun", "06" },
  { "Jul", "07" },
  { "Aug", "08" },
  { "Sep", "09" },
  { "Oct", "10" },
  { "Nov", "11" },
  { "Dec", "12" }
};

/**
  Construct Build Date fo comform for SMBIOS spc.

  @param [out]  DateBuf  Coverted Date String.
**/
VOID
ConstructBuildDate (
  OUT CHAR8 *DateBuf
  )
{
  UINTN Index;

  // GCC __DATE__ format is "Feb  2 1996"
  // If the day of the month is less than 10, it is padded with a space on the left
  CHAR8 *BuildDate = __DATE__;

  // SMBIOS spec date string: MM/DD/YYYY
  CHAR8 SmbiosDateStr[sizeof (mBiosReleaseDate)] = { 0 };

  SmbiosDateStr[sizeof (mBiosReleaseDate) - 1] = '\0';

  SmbiosDateStr[2] = '/';
  SmbiosDateStr[5] = '/';

  // Month
  for (Index = 0; Index < sizeof (MonthMatch) / sizeof (MonthMatch[0]); Index++) {
    if (AsciiStrnCmp (&BuildDate[0], MonthMatch[Index].MonthNameStr, AsciiStrLen (MonthMatch[Index].MonthNameStr)) == 0) {
      CopyMem (&SmbiosDateStr[0], MonthMatch[Index].DigitStr, AsciiStrLen (MonthMatch[Index].DigitStr));
      break;
    }
  }

  // Day
  CopyMem (&SmbiosDateStr[3], &BuildDate[4], 2);
  if (BuildDate[4] == ' ') {
    // day is less then 10, SAPCE filed by compiler, SMBIOS requires 0
    SmbiosDateStr[3] = '0';
  }

  // Year
  CopyMem (&SmbiosDateStr[6], &BuildDate[7], 4);

  CopyMem (DateBuf, SmbiosDateStr, AsciiStrLen (mBiosReleaseDate));
}


/***********************************************************************
        SMBIOS data definition  TYPE1  System Information
************************************************************************/
SMBIOS_TABLE_TYPE1 mSysInfoType1 = {
  { EFI_SMBIOS_TYPE_SYSTEM_INFORMATION, sizeof (SMBIOS_TABLE_TYPE1), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  { 0x25EF0280, 0xEC82, 0x42B0, { 0x8F, 0xB6, 0x10, 0xAD, 0xCC, 0xC6, 0x7C, 0x02 } },
  SystemWakeupTypePowerSwitch,
  5,    // SKUNumber String
  6,    // Family String
};

CHAR8 mSysInfoManufName[128];
CHAR8 mSysInfoProductName[128];
CHAR8 mSysInfoVersionName[128];
CHAR8 mSysInfoSerial[sizeof (UINT64) * 2 + 1];
CHAR8 mSysInfoSKU[sizeof (UINT64) * 2 + 1];

CHAR8 *mSysInfoType1Strings[] = {
  mSysInfoManufName,
  mSysInfoProductName,
  mSysInfoVersionName,
  mSysInfoSerial,
  mSysInfoSKU,
  "Phytium",
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE2  Board Information
************************************************************************/
SMBIOS_TABLE_TYPE2 mBoardInfoType2 = {
  { EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION, sizeof (SMBIOS_TABLE_TYPE2), 0 },
  1,    // Manufacturer String
  2,    // ProductName String
  3,    // Version String
  4,    // SerialNumber String
  5,    // AssetTag String
  {     // FeatureFlag
    1,    //  Motherboard           :1;
    0,    //  RequiresDaughterCard  :1;
    0,    //  Removable             :1;
    0,    //  Replaceable           :1;
    0,    //  HotSwappable          :1;
    0,    //  Reserved              :3;
  },
  6,                        // LocationInChassis String
  0,                        // ChassisHandle;
  BaseBoardTypeMotherBoard, // BoardType;
  0,                        // NumberOfContainedObjectHandles;
  { 0 }                     // ContainedObjectHandles[1];
};

CHAR8 mBoardInfoManufName[128];
CHAR8 mBoardInfoProductName[128];
CHAR8 mBoardInfoVersionName[128];
CHAR8 mBoardInfoSerial[128];
CHAR8 mBoardInfoAssetTag[128];
CHAR8 mBoardInfoLocation[128];

CHAR8 *mBoardInfoType2Strings[] = {
  mBoardInfoManufName,
  mBoardInfoProductName,
  mBoardInfoVersionName,
  mBoardInfoSerial,
  mBoardInfoAssetTag,
  mBoardInfoLocation,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE3  Enclosure Information
************************************************************************/
SMBIOS_TABLE_TYPE3 mEnclosureInfoType3 = {
  { EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE, sizeof (SMBIOS_TABLE_TYPE3), 0 },
  1,                        // Manufacturer String
  MiscChassisTypeMainServerChassis,    // Type;
  2,                        // Version String
  3,                        // SerialNumber String
  4,                        // AssetTag String
  ChassisStateSafe,         // BootupState;
  ChassisStateSafe,         // PowerSupplyState;
  ChassisStateSafe,         // ThermalState;
  ChassisSecurityStatusNone,// SecurityStatus;
  { 0, 0, 0, 0 },           // OemDefined[4];
  0,    // Height;
  1,    // NumberofPowerCords;
  0,    // ContainedElementCount;
  0,    // ContainedElementRecordLength;
  { { 0 } },    // ContainedElements[1];
};

CHAR8 mEnclosureInfoManufName[128];
CHAR8 mEnclosureInfoVersionName[128];
CHAR8 mEnclosureInfoSerial[128];
CHAR8 mEnclosureInfoAssetTag[128];
CHAR8 mEnclosureInfoSKU[128];
CHAR8 mChassisAssetTag[128];

CHAR8 *mEnclosureInfoType3Strings[] = {
  mEnclosureInfoManufName,
  mEnclosureInfoVersionName,
  mEnclosureInfoSerial,
  mEnclosureInfoAssetTag,
  mEnclosureInfoSKU,
  NULL
};

#if 0
/***********************************************************************
        SMBIOS data definition  TYPE4  Processor Information
************************************************************************/
SMBIOS_TABLE_TYPE4 mProcessorInfoType4 = {
  { EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof (SMBIOS_TABLE_TYPE4), 0},
  1,                               // Socket String
  CentralProcessor,                // ProcessorType;          ///< The enumeration value from PROCESSOR_TYPE_DATA.
  ProcessorFamilyIndicatorFamily2, // ProcessorFamily;        ///< The enumeration value from PROCESSOR_FAMILY2_DATA.
  2,                               // ProcessorManufacture String;
  {                                // ProcessorId;
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00 }
  },
  3,                    // ProcessorVersion String;
  {                     // Voltage;
    1,  // ProcessorVoltageCapability5V        :1;
    1,  // ProcessorVoltageCapability3_3V      :1;
    1,  // ProcessorVoltageCapability2_9V      :1;
    0,  // ProcessorVoltageCapabilityReserved  :1; ///< Bit 3, must be zero.
    0,  // ProcessorVoltageReserved            :3; ///< Bits 4-6, must be zero.
    0   // ProcessorVoltageIndicateLegacy      :1;
  },
  0,                      // ExternalClock;
  0,                      // MaxSpeed;
  0,                      // CurrentSpeed;
  0x41,                   // Status;
  ProcessorUpgradeNone,   // ProcessorUpgrade;         ///< The enumeration value from PROCESSOR_UPGRADE.
  0xFFFF,                 // L1CacheHandle;
  0xFFFF,                 // L2CacheHandle;
  0xFFFF,                 // L3CacheHandle;
  0,                      // SerialNumber;
  0,                      // AssetTag;
  0,                      // PartNumber;
  4,                      // CoreCount;
  4,                      // EnabledCoreCount;
  4,                      // ThreadCount;
  0x6C,                   // ProcessorCharacteristics; ///< The enumeration value from PROCESSOR_CHARACTERISTIC_FLAGS
      // ProcessorReserved1              :1;
      // ProcessorUnknown                :1;
      // Processor64BitCapble            :1;
      // ProcessorMultiCore              :1;
      // ProcessorHardwareThread         :1;
      // ProcessorExecuteProtection      :1;
      // ProcessorEnhancedVirtualization :1;
      // ProcessorPowerPerformanceCtrl    :1;
      // Processor128bitCapble            :1;
      // ProcessorReserved2               :7;
  ProcessorFamilyARM,     // ARM Processor Family;
  0,                      // CoreCount2;
  0,                      // EnabledCoreCount2;
  0,                      // ThreadCount2;
};

CHAR8 mCpuName[128] = "Unknown ARM CPU";

CHAR8 *mProcessorInfoType4Strings[] = {
  "Socket",
  "Broadcom",
  mCpuName,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE7  Cache Information
************************************************************************/
SMBIOS_TABLE_TYPE7 mCacheInfoType7_L1I = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x380,                    // Cache Configuration
       //Cache Level        :3  (L1)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (Unknown)
       //Reserved           :6
#if (RPI_MODEL == 4)
  0x0030,                   // Maximum Size (RPi4: 48KB)
  0x0030,                   // Install Size (RPi4: 48KB)
#else
  0x0010,                   // Maximum Size (RPi3: 16KB)
  0x0010,                   // Install Size (RPi3: 16KB)
#endif
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorParity,         // Error Correction
  CacheTypeInstruction,     // System Cache Type
  CacheAssociativity2Way    // Associativity  (RPi4 L1 Instruction cache is 3-way set associative, but SMBIOS spec does not define that)
};
CHAR8  *mCacheInfoType7Strings_L1I[] = {
  "L1 Instruction",
  NULL
};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_L1D = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x180,                    // Cache Configuration
       //Cache Level        :3  (L1)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (WB)
       //Reserved           :6
#if (RPI_MODEL == 4)
  0x0020,                   // Maximum Size (RPi4: 32KB)
  0x0020,                   // Install Size (RPi4: 32KB)
#else
  0x0010,                   // Maximum Size (RPi3: 16KB)
  0x0010,                   // Install Size (RPi3: 16KB)
#endif
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorSingleBit,      // Error Correction
  CacheTypeData,            // System Cache Type
#if (RPI_MODEL == 4)
  CacheAssociativity2Way    // Associativity
#else
  CacheAssociativity4Way    // Associativity
#endif
};
CHAR8  *mCacheInfoType7Strings_L1D[] = {
  "L1 Data",
  NULL
};

SMBIOS_TABLE_TYPE7 mCacheInfoType7_L2 = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  1,                        // SocketDesignation String
  0x0181,                   // Cache Configuration
       //Cache Level        :3  (L2)
       //Cache Socketed     :1  (Not Socketed)
       //Reserved           :1
       //Location           :2  (Internal)
       //Enabled/Disabled   :1  (Enabled)
       //Operational Mode   :2  (WB)
       //Reserved           :6
#if (RPI_MODEL == 4)
  0x0400,                   // Maximum Size (RPi4: 1MB)
  0x0400,                   // Install Size (RPi4: 1MB)
#else
  0x0200,                   // Maximum Size (RPi3: 512KB)
  0x0200,                   // Install Size (RPi3: 512KB)
#endif
  {                         // Supported SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  {                         // Current SRAM Type
    0,  //Other             :1
    0,  //Unknown           :1
    0,  //NonBurst          :1
    1,  //Burst             :1
    0,  //PiplelineBurst    :1
    1,  //Synchronous       :1
    0,  //Asynchronous      :1
    0   //Reserved          :9
  },
  0,                        // Cache Speed unknown
  CacheErrorSingleBit,      // Error Correction Multi
  CacheTypeUnified,         // System Cache Type
  CacheAssociativity16Way   // Associativity
};
CHAR8  *mCacheInfoType7Strings_L2[] = {
  "L2",
  NULL
};
#endif

/***********************************************************************
        SMBIOS data definition  TYPE8  Port Connector Information
************************************************************************/
SMBIOS_TABLE_TYPE8  mPortConnectorType8 = {
  { EFI_SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION, sizeof (SMBIOS_TABLE_TYPE8), 0 },
  1,                     //InternalReferenceDesignator
  PortConnectorTypeNone, //InternalConnectorType
  2,                     //ExternalReferenceDesignator
  PortConnectorTypeNone, //ExternalConnectorType
  PortTypeNone           //PortType
};

CHAR8 mPortInternalDesignation[128];
CHAR8 mPortExternalDesignation[128];
CHAR8 *mPortInfoType8Strings[] = {
  mPortInternalDesignation,
  mPortExternalDesignation,
  NULL
};


/***********************************************************************
        SMBIOS data definition  TYPE9  System Slot Information
************************************************************************/
SMBIOS_TABLE_TYPE9  mSysSlotInfoType9 = {
  { EFI_SMBIOS_TYPE_SYSTEM_SLOTS, sizeof (SMBIOS_TABLE_TYPE9), 0 },
  1,                      // SlotDesignation String
  SlotTypeOther,          // SlotType;                 ///< The enumeration value from MISC_SLOT_TYPE.
  SlotDataBusWidthOther,  // SlotDataBusWidth;         ///< The enumeration value from MISC_SLOT_DATA_BUS_WIDTH.
  SlotUsageAvailable,     // CurrentUsage;             ///< The enumeration value from MISC_SLOT_USAGE.
  SlotLengthOther,        // SlotLength;               ///< The enumeration value from MISC_SLOT_LENGTH.
  0,    // SlotID;
  {    // SlotCharacteristics1;
    1,  // CharacteristicsUnknown  :1;
    0,  // Provides50Volts         :1;
    0,  // Provides33Volts         :1;
    0,  // SharedSlot              :1;
    0,  // PcCard16Supported       :1;
    0,  // CardBusSupported        :1;
    0,  // ZoomVideoSupported      :1;
    0,  // ModemRingResumeSupported:1;
  },
  {     // SlotCharacteristics2;
    0,  // PmeSignalSupported      :1;
    0,  // HotPlugDevicesSupported :1;
    0,  // SmbusSignalSupported    :1;
    0,  // Reserved                :5;  ///< Set to 0.
  },
  0xFFFF, // SegmentGroupNum;
  0xFF,   // BusNum;
  0xFF,   // DevFuncNum;
};

CHAR8 mSlotInfoDesignation[128];
CHAR8 *mSysSlotInfoType9Strings[] = {
  mSlotInfoDesignation,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE 11  OEM Strings
************************************************************************/
SMBIOS_TABLE_TYPE11 mOemStringsType11 = {
  { EFI_SMBIOS_TYPE_OEM_STRINGS, sizeof (SMBIOS_TABLE_TYPE11), 0 },
  1     // StringCount
};
CHAR8 *mOemStringsType11Strings[] = {
  "http://phytium.com.cn",
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE 13  BIOS Language Information
************************************************************************/
SMBIOS_TABLE_TYPE13 mBiosLanguagaeType13 = {
  { EFI_SMBIOS_TYPE_BIOS_LANGUAGE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE13), 0 },
  1, // InstallableLanguages
  0, // Flags
  {0},
  1  // CurrentLanguages
};
CHAR8 *mBiosLanguageType13Strings[] = {
  "en|US|iso8859-1",
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE16  Physical Memory ArrayInformation
************************************************************************/
SMBIOS_TABLE_TYPE16 mPhyMemArrayInfoType16 = {
  { EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, sizeof (SMBIOS_TABLE_TYPE16), 0 },
  MemoryArrayLocationSystemBoard, // Location;                       ///< The enumeration value from MEMORY_ARRAY_LOCATION.
  MemoryArrayUseSystemMemory,     // Use;                            ///< The enumeration value from MEMORY_ARRAY_USE.
  MemoryErrorCorrectionUnknown,   // MemoryErrorCorrection;          ///< The enumeration value from MEMORY_ERROR_CORRECTION.
  0x00000000,                     // MaximumCapacity;
  0xFFFE,                         // MemoryErrorInformationHandle;
  0,                              // NumberOfMemoryDevices;
  0x00000000ULL,                  // ExtendedMaximumCapacity;
};
CHAR8 *mPhyMemArrayInfoType16Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE17  Memory Device Information
************************************************************************/
SMBIOS_TABLE_TYPE17 mMemDevInfoType17 = {
  { EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof (SMBIOS_TABLE_TYPE17), 0 },
  0,                    // MemoryArrayHandle; // Should match SMBIOS_TABLE_TYPE16.Handle, initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  0xFFFE,               // MemoryErrorInformationHandle; (not provided)
  0xFFFF,               // TotalWidth; (unknown)
  0xFFFF,               // DataWidth; (unknown)
  0xFFFF,               // Size; // When bit 15 is 0: Size in MB
                        // When bit 15 is 1: Size in KB, and continues in ExtendedSize
                        // initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  MemoryFormFactorUnknown, // FormFactor;                     ///< The enumeration value from MEMORY_FORM_FACTOR.
  0,                    // DeviceSet;
  1,                    // DeviceLocator String
  2,                    // BankLocator String
  MemoryTypeUnknown,    // MemoryType;                     ///< The enumeration value from MEMORY_DEVICE_TYPE.
  {                     // TypeDetail;
    0,  // Reserved        :1;
    0,  // Other           :1;
    0,  // Unknown         :1;
    0,  // FastPaged       :1;
    0,  // StaticColumn    :1;
    0,  // PseudoStatic    :1;
    0,  // Rambus          :1;
    1,  // Synchronous     :1;
    0,  // Cmos            :1;
    0,  // Edo             :1;
    0,  // WindowDram      :1;
    0,  // CacheDram       :1;
    0,  // Nonvolatile     :1;
    0,  // Registered      :1;
    0,  // Unbuffered      :1;
    0,  // Reserved1       :1;
  },
  0,                    // Speed; (unknown)
  3,                    // Manufacturer String
  4,                    // SerialNumber String
  5,                    // AssetTag String
  6,                    // PartNumber String
  0,                    // Attributes; (unknown rank)
  0,                    // ExtendedSize; (since Size < 32GB-1)
  0,                    // ConfiguredMemoryClockSpeed; (unknown)
  0,                    // MinimumVoltage; (unknown)
  0,                    // MaximumVoltage; (unknown)
  0,                    // ConfiguredVoltage; (unknown)
  MemoryTechnologyUnknown, // MemoryTechnology                 ///< The enumeration value from MEMORY_DEVICE_TECHNOLOGY
  {{                    // MemoryOperatingModeCapability
    0,  // Reserved                        :1;
    0,  // Other                           :1;
    0,  // Unknown                         :1;
    1,  // VolatileMemory                  :1;
    0,  // ByteAccessiblePersistentMemory  :1;
    0,  // BlockAccessiblePersistentMemory :1;
    0   // Reserved                        :10;
  }},
  7,                    // FirwareVersion
  0,                    // ModuleManufacturerID (unknown)
  0,                    // ModuleProductID (unknown)
  0,                    // MemorySubsystemControllerManufacturerID (unknown)
  0,                    // MemorySubsystemControllerProductID (unknown)
  0,                    // NonVolatileSize
  0xFFFFFFFFFFFFFFFFULL,// VolatileSize // initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  0,                    // CacheSize
  0,                    // LogicalSize (since MemoryType is not MemoryTypeLogicalNonVolatileDevice)
  0,                    // ExtendedSpeed,
  0                     // ExtendedConfiguredMemorySpeed
};

CHAR8 mMemDevInfoDeviceLocator[128];
CHAR8 mMemDevInfoManufacturer[128];
CHAR8 mMemDevInfoSerialNumber[128];
CHAR8 mMemDevInfoAssetTag[128];
CHAR8 mMemDevInfoPartNumber[128];
CHAR8 *mMemDevInfoType17Strings[] = {
  mMemDevInfoDeviceLocator,
  "BANK 0",
  mMemDevInfoManufacturer,
  mMemDevInfoSerialNumber,
  mMemDevInfoAssetTag,
  mMemDevInfoPartNumber,
  "Not set",
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE19  Memory Array Mapped Address Information
************************************************************************/
SMBIOS_TABLE_TYPE19 mMemArrMapInfoType19 = {
  { EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, sizeof (SMBIOS_TABLE_TYPE19), 0 },
  0x00000000, // StartingAddress;
  0x00000000, // EndingAddress;
  0,          // MemoryArrayHandle; // Should match SMBIOS_TABLE_TYPE16.Handle, initialized at runtime, refer to PhyMemArrayInfoUpdateSmbiosType16()
  1,          // PartitionWidth;
  0,          // ExtendedStartingAddress;  // not used
  0,          // ExtendedEndingAddress;    // not used
};
CHAR8 *mMemArrMapInfoType19Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE32  Boot Information
************************************************************************/
SMBIOS_TABLE_TYPE32 mBootInfoType32 = {
  { EFI_SMBIOS_TYPE_SYSTEM_BOOT_INFORMATION, sizeof (SMBIOS_TABLE_TYPE32), 0 },
  { 0, 0, 0, 0, 0, 0 },         // Reserved[6];
  BootInformationStatusNoError  // BootStatus
};

CHAR8 *mBootInfoType32Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE38  IPMI Device Information
************************************************************************/
SMBIOS_TABLE_TYPE38 mIpmiDeviceInfoType38 = {
  { EFI_SMBIOS_TYPE_IPMI_DEVICE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE38), 0 },
  IPMIDeviceInfoInterfaceTypeUnknown, // InterfaceType
  0,  // I2CSlaveAddress
  0,  // NVStorageDeviceAddress
  0,  // BaseAddress
  0,  // BaseAddressModifier_InterruptInfo
  0  // InterruptNumber
};
CHAR8 *mIpmiDeviceInfoType38Strings[] = {
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE41  On Board Device Information
************************************************************************/
SMBIOS_TABLE_TYPE41 mOnBoardDevInfoType41 = {
  { EFI_SMBIOS_TYPE_ONBOARD_DEVICES_EXTENDED_INFORMATION, sizeof (SMBIOS_TABLE_TYPE41), 0 },
  1,                                // ReferenceDesignation
  OnBoardDeviceExtendedTypeUnknown, // DeviceType
  0,                                // DeviceTypeInstance
  0,                                // SegmentGroupNum
  0,                                // BusNum
  0                                 // DevFuncNum
};

CHAR8 mOnBoardDevDesignation[128];
CHAR8 *mOnBoardDevInfoType41Strings[] = {
  mOnBoardDevDesignation,
  NULL
};

/***********************************************************************
        SMBIOS data definition  TYPE45  Firmware Inventory Information
************************************************************************/
SMBIOS_TABLE_TYPE45 mFirmwareInventoryInfoType45 = {
  { SMBIOS_TYPE_FIRMWARE_INVENTORY_INFORMATION, sizeof (SMBIOS_TABLE_TYPE45), 0 },
  1,                                // FirmwareComponentName
  2,                                // FirmwareVersion
  VersionFormatTypeMajorMinor,      // The enumeration value from FIRMWARE_INVENTORY_VERSION_FORMAT_TYPE.
  3,                                // FirmwareId
  InventoryFirmwareIdFormatTypeOem, // The enumeration value from FIRMWARE_INVENTORY_FIRMWARE_ID_FORMAT_TYPE.
  4,                                // ReleaseDate
  5,                                // Manufacturer
  6,                                // LowestSupportedVersion
  0xFFFFFFFFFFFFFFFFULL,            // ImageSize
  {0},                              // Characteristics
  FirmwareInventoryStateUnknown,    // State
  0                                 // AssociatedComponentCount
};

CHAR8 mFirmwareVersion[12];
CHAR8 mReleaseDate[32];
CHAR8 *mFirmwareInventoryInfoType45Strings[] = {
  "PBF Firmware",
  mFirmwareVersion,
  "Phytium",
  mReleaseDate,
  "PHYTIUM LTD",
  "Not Set",
  NULL
};

/**
   Create SMBIOS record.

   Converts a fixed SMBIOS structure and an array of pointers to strings into
   an SMBIOS record where the strings are cat'ed on the end of the fixed record
   and terminated via a double NULL and add to SMBIOS table.

   SMBIOS_TABLE_TYPE32 gSmbiosType12 = {
   { EFI_SMBIOS_TYPE_SYSTEM_CONFIGURATION_OPTIONS, sizeof (SMBIOS_TABLE_TYPE12), 0 },
   1 // StringCount
   };

   CHAR8 *gSmbiosType12Strings[] = {
   "Not Found",
   NULL
   };

   ...

   LogSmbiosData (
   (EFI_SMBIOS_TABLE_HEADER*)&gSmbiosType12,
   gSmbiosType12Strings
   );

   @param  Template    Fixed SMBIOS structure, required.
   @param  StringPack  Array of strings to convert to an SMBIOS string pack.
   NULL is OK.
   @param  DataSmbiosHandle  The new SMBIOS record handle.
   NULL is OK.
**/
EFI_STATUS
EFIAPI
LogSmbiosData (
  IN  EFI_SMBIOS_TABLE_HEADER *Template,
  IN  CHAR8                   **StringPack,
  OUT EFI_SMBIOS_HANDLE       *DataSmbiosHandle
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *Smbios;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINTN                     Index;
  UINTN                     StringSize;
  UINTN                     Size;
  CHAR8                     *Str;

  //
  // Locate Smbios protocol.
  //
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&Smbios);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Calculate the size of the fixed record and optional string pack

  Size = Template->Length;
  if (StringPack == NULL) {
    // At least a double null is required
    Size += 2;
  } else {
    for (Index = 0; StringPack[Index] != NULL; Index++) {
      StringSize = AsciiStrSize (StringPack[Index]);
      Size += StringSize;
    }
    if (StringPack[0] == NULL) {
      // At least a double null is required
      Size += 1;
    }

    // Don't forget the terminating double null
    Size += 1;
  }

  // Copy over Template
  Record = (EFI_SMBIOS_TABLE_HEADER*)AllocateZeroPool (Size);
  if (Record == NULL) {
    DEBUG((DEBUG_ERROR, "LogSmbiosData Out of Resources\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, Template, Template->Length);

  // Append string pack
  Str = ((CHAR8*)Record) + Record->Length;

  for (Index = 0; StringPack[Index] != NULL; Index++) {
    StringSize = AsciiStrSize (StringPack[Index]);
    CopyMem (Str, StringPack[Index], StringSize);
    Str += StringSize;
  }

  *Str = 0;
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (
                     Smbios,
                     gImageHandle,
                     &SmbiosHandle,
                     Record
                   );

  if ((Status == EFI_SUCCESS) && (DataSmbiosHandle != NULL)) {
    *DataSmbiosHandle = SmbiosHandle;
  }

  ASSERT_EFI_ERROR (Status);
  FreePool (Record);
  return Status;
}

/**
 Get smbios information about memmory device.

**/
VOID
GetSmbiosMemoryInfo (
  VOID
  )
{
  EFI_HOB_GUID_TYPE   *GuidHob;

  GuidHob = GetFirstGuidHob (&gPlatformMemorySmbiosInfoGuid);
  if (GuidHob == NULL) {
    DEBUG ((DEBUG_ERROR, "Couldn't get the Memory Smbios Hob\n"));
    return;
  }
  mMemorySmbiosInfo  = (PHYTIUM_MEMORY_SMBIOS_INFO *) GET_GUID_HOB_DATA(GuidHob);
}

/**
 Get smbios information form Json file.

 @retval EFI_SUCCESS                    Success.
 @retval EFI_NOT_FOUND                  The required object information is not found.
**/
EFI_STATUS
GetSmbiosInfoFromJson ()
{
  EFI_STATUS        Status;
  EDKII_JSON_ERROR  Error;
  UINTN             SmbiosInforSize;

  Status = GetSectionFromAnyFv (
             &gEfiSmbiosStringTxtGuid,
             EFI_SECTION_RAW,
             0,
             &mSmbiosInfo,
             &SmbiosInforSize
             );
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "Not find json data in Fv\n"));
    return Status;
  }

  mJsonValue = JsonLoadBuffer (mSmbiosInfo, SmbiosInforSize, EDKII_JSON_DISABLE_EOF_CHECK|EDKII_JSON_ALLOW_NUL, &Error);
  if (mJsonValue == NULL) {
    DEBUG((DEBUG_ERROR, "Fail to load JSON payload\n"));
    return EFI_NOT_FOUND;
  }

  mJsonObject = JsonValueGetObject (mJsonValue);
  if (mJsonObject == NULL) {
    DEBUG((DEBUG_INFO, "Json Get Object Fail\n"));
    return EFI_NOT_FOUND;
  }

  return Status;
}

/**
 Get Ascii String by Json key.

 @param [in]  Key   The key of the JSON value to be retrieved.

 @return The ASCII code corresponding to the key.
**/
CONST CHAR8 *
GetAsciiString (
  IN CHAR8 *Key
  )
{
  EDKII_JSON_VALUE   JsonValue;

  JsonValue = JsonObjectGetValue (mJsonObject, Key);
  if (!JsonValueGetAsciiString (JsonValue)) {
    return UnknownString;
  } else {
    return JsonValueGetAsciiString (JsonValue);
  }
}

/**
 Get Integer by Json key.

 @param [in]  Key   The key of the JSON value to be retrieved.

 @return The Integer corresponding to the key.
**/
INT64
GetInteger (
  IN CHAR8 *Key
  )
{
  EDKII_JSON_VALUE   JsonValue;

  JsonValue = JsonObjectGetValue (mJsonObject, Key);
  if (JsonValue == NULL) {
    DEBUG ((DEBUG_ERROR, "Get Json Integer Fail.\n"));
    return 0;
  }
  return JsonValueGetInteger (JsonValue);
}

/**
 Convert INT32 to hex string.

 @param [in, out] TargetStringSz    The pointer to Target string.
 @param [in]      TargetStringSize  Size of Target String.
 @param [in]      Value             The value to be convert.

**/
VOID
I32ToHexString (
  IN OUT CHAR8* TargetStringSz,
  IN UINT32 TargetStringSize,
  IN UINT32 Value
  )
{
  static CHAR8 ItoH[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
  UINT8 StringInx;
  INT8 NibbleInx;

  ZeroMem ((void*)TargetStringSz, TargetStringSize);

  //
  // Convert each nibble to hex string, starting from
  // the highest non-zero nibble.
  //
  StringInx = 0;
  for (NibbleInx = sizeof (UINT32) * 2; NibbleInx > 0; --NibbleInx) {
    UINT32 NibbleMask = (((UINT32)0xF) << ((NibbleInx - 1) * 4));
    UINT8 Nibble = (UINT8)((Value & NibbleMask) >> ((NibbleInx - 1) * 4));

    ASSERT (Nibble <= 0xF);

    if (StringInx < (TargetStringSize - 1)) {
      TargetStringSz[StringInx++] = ItoH[Nibble];
    } else {
      break;
    }
  }
}

/**
 Get memory devices total size,uint is GBytes.

 @return Total size.
**/
UINT64
GetMemoryTotalSize (
  VOID
  )
{
  UINT8 DimmCount;
  UINT8 Index;
  UINT64 TotalSize;

  TotalSize = 0;
  DimmCount = mMemorySmbiosInfo->MaxDimmCount;
  for (Index = 0; Index < DimmCount; Index++) {
    TotalSize += mMemorySmbiosInfo->MemoryDevice[Index].Size;
  }

  return TotalSize;
}

/***********************************************************************
        SMBIOS data update  TYPE0  BIOS Information.
************************************************************************/
VOID
BIOSInfoUpdateSmbiosType0 (
  VOID
  )
{
  UINT32 BiosRealSize;

  BiosRealSize = FixedPcdGet32 (PcdFdSize);
  if (BiosRealSize >= 0x1000000) {
    mBIOSInfoType0.BiosSize = 0xFF;
    mBIOSInfoType0.ExtendedBiosSize.Size = (UINT16)(BiosRealSize / 0x100000);
    mBIOSInfoType0.ExtendedBiosSize.Unit = 0;
  } else {
    mBIOSInfoType0.BiosSize = (UINT8)(BiosRealSize / 0x10000 - 1);
    mBIOSInfoType0.ExtendedBiosSize.Size = 0;
    mBIOSInfoType0.ExtendedBiosSize.Unit = 0;
  }
  UnicodeStrToAsciiStrS ((CHAR16*)PcdGetPtr (PcdFirmwareVendor),
    mBiosVendor, sizeof (mBiosVendor));
  UnicodeStrToAsciiStrS ((CHAR16*)PcdGetPtr (PcdFirmwareVersionString),
    mBiosVersion, sizeof (mBiosVersion));
  ConstructBuildDate (mBiosReleaseDate);

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBIOSInfoType0, mBIOSInfoType0Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE1  System Information.
************************************************************************/
VOID
SysInfoUpdateSmbiosType1 (
  VOID
  )
{
  AsciiStrCpyS (mSysInfoManufName, sizeof (mSysInfoManufName),
    GetAsciiString ("SysInfoManufName"));
  AsciiStrCpyS (mSysInfoProductName, sizeof (mSysInfoProductName),
    GetAsciiString ("SysInfoProductName"));
  AsciiStrCpyS (mSysInfoVersionName, sizeof (mSysInfoVersionName),
    GetAsciiString ("SysInfoVersionName"));
  AsciiStrCpyS (mSysInfoSerial, sizeof (mSysInfoSerial),
    GetAsciiString ("SysInfoSerial"));
  AsciiStrCpyS (mSysInfoSKU, sizeof (mSysInfoSKU),
    GetAsciiString ("SysInfoSKU"));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mSysInfoType1, mSysInfoType1Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE2  Board Information.
************************************************************************/
VOID
BoardInfoUpdateSmbiosType2 (
  VOID
  )
{
  AsciiStrCpyS (mBoardInfoManufName, sizeof (mBoardInfoManufName),
    GetAsciiString ("BoardInfoManufName"));
  AsciiStrCpyS (mBoardInfoProductName, sizeof (mBoardInfoProductName),
    GetAsciiString ("BoardInfoProductName"));
  AsciiStrCpyS (mBoardInfoVersionName, sizeof (mBoardInfoVersionName),
    GetAsciiString ("BoardInfoVersionName"));
  AsciiStrCpyS (mBoardInfoSerial, sizeof (mBoardInfoSerial),
    GetAsciiString ("BoardInfoSerial"));
  AsciiStrCpyS (mBoardInfoAssetTag, sizeof (mBoardInfoAssetTag),
    GetAsciiString ("BoardInfoAssetTag"));
  AsciiStrCpyS (mBoardInfoLocation, sizeof (mBoardInfoLocation),
    GetAsciiString ("BoardInfoLocation"));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBoardInfoType2, mBoardInfoType2Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE3  Enclosure Information.
************************************************************************/
VOID
EnclosureInfoUpdateSmbiosType3 (
  VOID
  )
{
  EFI_SMBIOS_HANDLE SmbiosHandle;

  AsciiStrCpyS (mEnclosureInfoManufName, sizeof (mEnclosureInfoManufName),
    GetAsciiString ("EnclosureInfoManufName"));
  AsciiStrCpyS (mEnclosureInfoVersionName, sizeof (mEnclosureInfoVersionName),
    GetAsciiString ("EnclosureInfoVersionName"));
  AsciiStrCpyS (mEnclosureInfoSerial, sizeof (mEnclosureInfoSerial),
    GetAsciiString ("EnclosureInfoSerial"));
  AsciiStrCpyS (mEnclosureInfoAssetTag, sizeof (mEnclosureInfoAssetTag),
    GetAsciiString ("EnclosureInfoAssetTag"));
  AsciiStrCpyS (mEnclosureInfoSKU, sizeof (mEnclosureInfoSKU),
    GetAsciiString ("EnclosureInfoSKU"));


  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mEnclosureInfoType3, mEnclosureInfoType3Strings, &SmbiosHandle);

  // Set Type2 ChassisHandle to point to the newly added Type3 handle
  mBoardInfoType2.ChassisHandle = (UINT16) SmbiosHandle;
}

/***********************************************************************
        SMBIOS data update  TYPE8  Port Connector Information.
************************************************************************/
VOID
PortConnectorInfoUpdateSmbiosType8 (
  VOID
  )
{
  INT64 PortCount;
  CHAR8 Strings[128];
  UINT8 Index;

  PortCount = GetInteger ("PortCount");
  for (Index = 0; Index < PortCount; Index++) {
    //InternalReferenceDesignator
    CopyMem (Strings, "PortxInternalDesignation", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    AsciiStrCpyS (mPortInternalDesignation, sizeof (mPortInternalDesignation),
      GetAsciiString (Strings));

    //InternalConnectorType
    CopyMem (Strings, "PortxInternalConnectorType", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mPortConnectorType8.InternalConnectorType = GetInteger (Strings);

    //ExternalReferenceDesignator
    CopyMem (Strings, "PortxExternalDesignation", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    AsciiStrCpyS (mPortExternalDesignation, sizeof (mPortExternalDesignation),
      GetAsciiString (Strings));

    //ExternalConnectorType
    CopyMem (Strings, "PortxExternalConnectorType", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mPortConnectorType8.ExternalConnectorType = GetInteger (Strings);

    //PortType
    CopyMem (Strings, "PortxType", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mPortConnectorType8.PortType = GetInteger (Strings);

    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mPortConnectorType8, mPortInfoType8Strings, NULL);
  }
}

/***********************************************************************
        SMBIOS data update  TYPE9  System Slot Information.
************************************************************************/
VOID
SysSlotInfoUpdateSmbiosType9 (
  VOID
  )
{
  INT64 SlotCount;
  CHAR8 Strings[128];
  UINT8 Index;

  SlotCount = GetInteger ("SlotCount");

  for (Index = 0; Index < SlotCount; Index++) {
    //SlotDesignation
    CopyMem (Strings, "SlotxDesignation", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    AsciiStrCpyS (mSlotInfoDesignation, sizeof (mSlotInfoDesignation),
      GetAsciiString (Strings));
    //SlotType
    CopyMem (Strings, "SlotxType", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotType = GetInteger (Strings);
    //SlotDataBusWidth
    CopyMem (Strings, "SlotxDataBusWidth", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotDataBusWidth = GetInteger (Strings);
    //CurrentUsage
    CopyMem (Strings, "SlotxCurrentUsage", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.CurrentUsage = GetInteger (Strings);
    //SlotLength
    CopyMem (Strings, "SlotxLength", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotLength = GetInteger (Strings);
    //SlotID
    CopyMem (Strings, "SlotxID", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotID = GetInteger (Strings);
    //SegmentGroupNum
    CopyMem (Strings, "SlotxSegmentGroupNum", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SegmentGroupNum = GetInteger (Strings);
    //BusNum
    CopyMem (Strings, "SlotxBusNum", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.BusNum = GetInteger (Strings);
    //DevFuncNum
    CopyMem (Strings, "SlotxDevFuncNum", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.DevFuncNum = GetInteger (Strings);
    //DataBusWidth
    CopyMem (Strings, "SlotxElectricalDataBusWidth", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.DataBusWidth = GetInteger (Strings);
    //SlotInformation
    CopyMem (Strings, "SlotxInformation", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotInformation = GetInteger (Strings);
    //SlotPhysicalWidth
    CopyMem (Strings, "SlotxPhysicalWidth", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotPhysicalWidth = GetInteger (Strings);
    //SlotPitch
    CopyMem (Strings, "SlotxPitch", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotPitch = GetInteger (Strings);
    //SlotHeight
    CopyMem (Strings, "SlotxHeight", sizeof (Strings));
    Strings[4] = AsciiFromHex (Index);
    mSysSlotInfoType9.SlotHeight = GetInteger (Strings);

    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mSysSlotInfoType9, mSysSlotInfoType9Strings, NULL);
  }
}

/***********************************************************************
        SMBIOS data update  TYPE11  OEM Strings.
************************************************************************/
VOID
OemStringsUpdateSmbiosType11 (
  VOID
  )
{
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mOemStringsType11, mOemStringsType11Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE13  BIOS Language Information.
************************************************************************/
VOID
BiosLanguageUpdateSmbiosType13 (
  VOID
  )
{
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBiosLanguagaeType13, mBiosLanguageType13Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE16  Physical Memory Array Information.
************************************************************************/
VOID
PhyMemArrayInfoUpdateSmbiosType16 (
  VOID
  )
{
  EFI_SMBIOS_HANDLE           MemArraySmbiosHandle;
  UINT64                      MaximumCapacity;

  MaximumCapacity = GetInteger ("MemoryMaximumCapacity") * 1024 * 1024;
  if (MaximumCapacity >= 0x80000000) {
    mPhyMemArrayInfoType16.MaximumCapacity = 0x80000000;
    mPhyMemArrayInfoType16.ExtendedMaximumCapacity = MaximumCapacity << 10;
  } else {
    mPhyMemArrayInfoType16.MaximumCapacity = (UINT32)MaximumCapacity;
    mPhyMemArrayInfoType16.ExtendedMaximumCapacity = 0;
  }

  mPhyMemArrayInfoType16.MemoryErrorCorrection = mMemorySmbiosInfo->EccType;
  mPhyMemArrayInfoType16.NumberOfMemoryDevices = mMemorySmbiosInfo->MaxDimmCount;

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mPhyMemArrayInfoType16, mPhyMemArrayInfoType16Strings, &MemArraySmbiosHandle);

  //
  // Update the memory device information and memory array map with the newly added type 16 handle
  //
  mMemDevInfoType17.MemoryArrayHandle = MemArraySmbiosHandle;
  mMemArrMapInfoType19.MemoryArrayHandle = MemArraySmbiosHandle;
}

/***********************************************************************
        SMBIOS data update  TYPE17  Memory Device Information.
************************************************************************/
VOID
MemDevInfoUpdateSmbiosType17 (
  VOID
  )
{
  UINT8  DimmCount;
  UINT8  DimmIndex;
  UINT8  DimmType;
  UINT8  DeviceLocator;
  UINT16 Manufacturer;
  UINT32 SerialNumber;
  UINT32 DimmSize;

  DimmCount = mMemorySmbiosInfo->MaxDimmCount;
  DEBUG ((DEBUG_INFO, "SmbiosType17 DIMM Count: %d\n", DimmCount));
  for (DimmIndex = 0; DimmIndex < DimmCount; DimmIndex++) {
    //Size uint:GByte
    DimmSize = mMemorySmbiosInfo->MemoryDevice[DimmIndex].Size;
    DeviceLocator = mMemorySmbiosInfo->MemoryDevice[DimmIndex].DeviceLocator;
    DEBUG ((DEBUG_INFO, "SmbiosType17 DIMM Index: %d, DIMM Size: %d\n", DimmCount, DimmSize));
    if (DimmSize == 0) {
      mMemDevInfoType17.TotalWidth = 0xFFFF;
      mMemDevInfoType17.DataWidth = 0xFFFF;
      mMemDevInfoType17.Size = 0x0;
      mMemDevInfoType17.FormFactor = MemoryFormFactorUnknown;
      mMemDevInfoType17.MemoryType = MemoryTypeUnknown;
      mMemDevInfoType17.TypeDetail.Synchronous = 0;
      mMemDevInfoType17.TypeDetail.Registered = 0;
      mMemDevInfoType17.TypeDetail.Unbuffered = 0;
      mMemDevInfoType17.TypeDetail.LrDimm = 0;
      mMemDevInfoType17.MinimumVoltage = 0;
      mMemDevInfoType17.MaximumVoltage = 0;
      mMemDevInfoType17.ConfiguredVoltage = 0;
      AsciiSPrint(mMemDevInfoDeviceLocator, SMBIOS_STRING_MAX_LENGTH, "DIMM %d", DeviceLocator);
      mMemDevInfoType17.Speed = 0x0;
      AsciiStrCpyS (mMemDevInfoManufacturer, sizeof (mMemDevInfoAssetTag), "Unknown");
      AsciiStrCpyS (mMemDevInfoSerialNumber, sizeof (mMemDevInfoSerialNumber), "Unknown");
      AsciiStrCpyS (mMemDevInfoAssetTag, sizeof (mMemDevInfoAssetTag), "Unkonwn");
      AsciiStrCpyS (mMemDevInfoPartNumber, sizeof (mMemDevInfoPartNumber), "Unkonwn");
      mMemDevInfoType17.Attributes = 0x0;
      mMemDevInfoType17.ExtendedSize = 0x0;
      mMemDevInfoType17.ConfiguredMemoryClockSpeed = 0x0;
      mMemDevInfoType17.MinimumVoltage = 0x0;
      mMemDevInfoType17.MaximumVoltage = 0x0;
      mMemDevInfoType17.ConfiguredVoltage = 0x0;
      mMemDevInfoType17.MemoryTechnology = MemoryTechnologyUnknown;
      mMemDevInfoType17.MemoryOperatingModeCapability.Uint16 = 0x4;
      mMemDevInfoType17.ModuleManufacturerID = 0x0;
      mMemDevInfoType17.ModuleProductID = 0x0;
      mMemDevInfoType17.MemorySubsystemControllerManufacturerID = 0x0;
      mMemDevInfoType17.MemorySubsystemControllerProductID = 0x0;
      mMemDevInfoType17.NonVolatileSize = 0;
      mMemDevInfoType17.VolatileSize = 0x0;
      mMemDevInfoType17.CacheSize = 0x0;
      mMemDevInfoType17.LogicalSize = 0x0;
    } else {
      DimmSize = DimmSize * 1024;  //convert to MByte
      if (DimmSize > 32767) {
        mMemDevInfoType17.Size = 0x7FFF;
        mMemDevInfoType17.ExtendedSize = DimmSize;
      } else {
        mMemDevInfoType17.Size = DimmSize;
        mMemDevInfoType17.ExtendedSize = 0;
      }
      //Total Width
      mMemDevInfoType17.TotalWidth = mMemorySmbiosInfo->MemoryDevice[DimmIndex].TotalWidth;
      //Data Width
      mMemDevInfoType17.DataWidth = mMemorySmbiosInfo->MemoryDevice[DimmIndex].DataWidth;
      //FormFactor
      DimmType = mMemorySmbiosInfo->MemoryDevice[DimmIndex].DimmType;
      if (DimmType == SODIMM_TYPE) {
        mMemDevInfoType17.FormFactor = MemoryFormFactorSodimm;
      } else {
        mMemDevInfoType17.FormFactor = MemoryFormFactorDimm;
      }
      //DeviceLocator
      AsciiSPrint(mMemDevInfoDeviceLocator, SMBIOS_STRING_MAX_LENGTH, "DIMM %d", DeviceLocator);
      //MemoryType
      mMemDevInfoType17.MemoryType = mMemorySmbiosInfo->MemoryDevice[DimmIndex].MemoryType;

      //Type Detail
      switch (DimmType) {
        case RDIMM_TYPE:
          mMemDevInfoType17.TypeDetail.Registered = 1;
          break;

        case SODIMM_TYPE:
        case UDIMM_TYPE:
          mMemDevInfoType17.TypeDetail.Unbuffered= 1;
          break;

        case LRDIMM_TYPE:
          mMemDevInfoType17.TypeDetail.LrDimm = 1;
          break;
      }

      //Speed
      mMemDevInfoType17.Speed = mMemorySmbiosInfo->MemoryDevice[DimmIndex].MaxSpeed;
      //Manufacturer
      Manufacturer = mMemorySmbiosInfo->MemoryDevice[DimmIndex].Manufacturer;
      GetManufacturer (Manufacturer, mMemDevInfoManufacturer);
      //SerialNumber
      SerialNumber = mMemorySmbiosInfo->MemoryDevice[DimmIndex].SerialNumber;
      I32ToHexString (mMemDevInfoSerialNumber, 8, SerialNumber);
      //AssetTag
      AsciiStrCpyS (mMemDevInfoAssetTag, sizeof (mMemDevInfoAssetTag),
        GetAsciiString ("MemDevInfoAssetTag"));
      //PartNumber
      AsciiStrCpyS (mMemDevInfoPartNumber, sizeof (mMemDevInfoPartNumber), mMemorySmbiosInfo->MemoryDevice[DimmIndex].PartNumber);
      //Attributes
      mMemDevInfoType17.Attributes = mMemorySmbiosInfo->MemoryDevice[DimmIndex].RankNumber;
      //ConfiguredMemoryClockSpeed
      mMemDevInfoType17.ConfiguredMemoryClockSpeed = mMemorySmbiosInfo->MemoryDevice[DimmIndex].ConfiguredMemoryClockSpeed;
      switch (mMemDevInfoType17.MemoryType) {
        case MemoryTypeDdr4:
          mMemDevInfoType17.MinimumVoltage = 1200;
          mMemDevInfoType17.MaximumVoltage = 1200;
          mMemDevInfoType17.ConfiguredVoltage = 1200;
          break;

        case MemoryTypeDdr5:
          mMemDevInfoType17.MinimumVoltage = 1100;
          mMemDevInfoType17.MaximumVoltage = 1100;
          mMemDevInfoType17.ConfiguredVoltage = 1100;
          break;

        default:
          mMemDevInfoType17.MinimumVoltage = 0;
          mMemDevInfoType17.MaximumVoltage = 0;
          mMemDevInfoType17.ConfiguredVoltage = 0;
      }

      //MemoryTechnology
      mMemDevInfoType17.MemoryTechnology = mMemorySmbiosInfo->MemoryDevice[DimmIndex].MemoryTechnology;
      //ModuleManufacturerID
      mMemDevInfoType17.ModuleManufacturerID = mMemorySmbiosInfo->MemoryDevice[DimmIndex].ModuleManufacturerID;
      //ModuleProductID
      mMemDevInfoType17.ModuleProductID = 0x0;
      //MemorySubsystemControllerManufacturerID
      mMemDevInfoType17.MemorySubsystemControllerManufacturerID = 0x0;
      //MemorySubsystemControllerProductID
      mMemDevInfoType17.MemorySubsystemControllerProductID = 0x0;
      //NonVolatileSize
      mMemDevInfoType17.NonVolatileSize = 0;
      //VolatileSize
      mMemDevInfoType17.VolatileSize = (UINT64) LShiftU64 (DimmSize, 20);
      //CacheSize
      mMemDevInfoType17.CacheSize = 0;
      //LogicalSize
      mMemDevInfoType17.LogicalSize = 0;
    }

    DEBUG ((DEBUG_INFO, "SmbiosType17 LogSmbiosData DimmIndex: %d\n", DimmIndex));
    DEBUG ((DEBUG_INFO, "TotalWidth: %d\n", mMemDevInfoType17.TotalWidth));
    DEBUG ((DEBUG_INFO, "DataWidth: %d\n", mMemDevInfoType17.DataWidth));
    DEBUG ((DEBUG_INFO, "Size: %d\n", mMemDevInfoType17.Size));
    DEBUG ((DEBUG_INFO, "FormFactor: %d\n", mMemDevInfoType17.FormFactor));
    DEBUG ((DEBUG_INFO, "DeviceSet: %d\n", mMemDevInfoType17.DeviceSet));
    DEBUG ((DEBUG_INFO, "DeviceLocator: %d\n", mMemDevInfoType17.DeviceLocator));
    DEBUG ((DEBUG_INFO, "BankLocator: %d\n", mMemDevInfoType17.BankLocator));
    DEBUG ((DEBUG_INFO, "MemoryType: %d\n", mMemDevInfoType17.MemoryType));
    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mMemDevInfoType17, mMemDevInfoType17Strings, NULL);
  }
}

/***********************************************************************
        SMBIOS data update  TYPE19  Memory Array Map Information.
************************************************************************/
VOID
MemArrMapInfoUpdateSmbiosType19 (
  VOID
  )
{
  UINT64 TotalSize;

  TotalSize = GetMemoryTotalSize ();
  mMemArrMapInfoType19.StartingAddress = 0;

  mMemArrMapInfoType19.EndingAddress = (UINT32) LShiftU64 (TotalSize, 20);
  mMemArrMapInfoType19.EndingAddress -= 1;
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mMemArrMapInfoType19, mMemArrMapInfoType19Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE32  Boot Information.
************************************************************************/
VOID
BootInfoUpdateSmbiosType32 (
  VOID
  )
{
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mBootInfoType32, mBootInfoType32Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE38  IPMI Device Information.
************************************************************************/
VOID
IpmiDevInfoUpdateSmbiosType38 (
  VOID
  )
{
  UINT8 IpmiExist;

  IpmiExist = GetInteger ("IpmiExist");
  if (IpmiExist) {
    mIpmiDeviceInfoType38.InterfaceType = GetInteger ("IpmiInterfaceType");
    mIpmiDeviceInfoType38.IPMISpecificationRevision = GetInteger ("IpmiRevision");
    mIpmiDeviceInfoType38.I2CSlaveAddress = GetInteger ("I2CSlaveAddress");
    mIpmiDeviceInfoType38.NVStorageDeviceAddress = GetInteger ("NVStorageDeviceAddress");
    mIpmiDeviceInfoType38.BaseAddress = GetInteger ("BaseAddress");
    mIpmiDeviceInfoType38.BaseAddressModifier_InterruptInfo = GetInteger ("IpmiInterruptInfo");
    mIpmiDeviceInfoType38.InterruptNumber = GetInteger ("InterruptNumber");
  }
  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mIpmiDeviceInfoType38, mIpmiDeviceInfoType38Strings, NULL);
}

/***********************************************************************
        SMBIOS data update  TYPE41  On Board  Device Information.
************************************************************************/
VOID
OnBoardDevInfoUpdateSmbiosType41 (
  VOID
  )
{
  INT64 OnBoardDevCount;
  CHAR8 Strings[128];
  UINT8 Index;
  UINT8 DeviceStatus;
  UINT8 DeviceType;

  OnBoardDevCount = GetInteger ("OnBoardDevCount");
  for (Index = 0; Index < OnBoardDevCount; Index++) {
    //ReferenceDesignator
    CopyMem (Strings, "OnBoardDevxDesignation", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    AsciiStrCpyS (mOnBoardDevDesignation, sizeof (mOnBoardDevDesignation),
      GetAsciiString (Strings));

    //Device Status
    CopyMem (Strings, "OnBoardDevxStatus", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    DeviceStatus = GetInteger (Strings);
    //Device Type
    CopyMem (Strings, "OnBoardDevxType", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    DeviceType = GetInteger (Strings);
    mOnBoardDevInfoType41.DeviceType = (DeviceStatus << 7) | DeviceType;

    //DeviceTypeInstance
    CopyMem (Strings, "OnBoardDevxInstance", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    mOnBoardDevInfoType41.DeviceTypeInstance = GetInteger (Strings);

    //SegmentGroupNum
    CopyMem (Strings, "OnBoardDevxSeg", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    mOnBoardDevInfoType41.SegmentGroupNum = GetInteger (Strings);

    //BusNum
    CopyMem (Strings, "OnBoardDevxBusNum", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    mOnBoardDevInfoType41.BusNum = GetInteger (Strings);

    //DevFuncNum
    CopyMem (Strings, "OnBoardDevxDevFuncNum", sizeof (Strings));
    Strings[10] = AsciiFromHex (Index);
    mOnBoardDevInfoType41.DevFuncNum = GetInteger (Strings);

    LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mOnBoardDevInfoType41, mOnBoardDevInfoType41Strings, NULL);
  }
}

/***********************************************************************
        SMBIOS data update  TYPE45  Firmware Inventory Information.
************************************************************************/
VOID
FirmwareInventoryInformationType45 (
  VOID
  )
{
  UINT16            MajorVer;
  UINT16            MinorVer;
  UINTN             Size;
  UINT8             *PhyPbfBuff;

  MajorVer = 0;
  MinorVer = 0;
  ZeroMem (mReleaseDate, sizeof (mReleaseDate));
  ZeroMem (mFirmwareVersion, sizeof (mFirmwareVersion));

  // Pbf version
  PssiGetPbfVersion (&MajorVer,&MinorVer);
  AsciiSPrint(mFirmwareVersion,sizeof(mFirmwareVersion),"%d.%d",MajorVer,MinorVer);

  // Pbf version description
  PhyPbfBuff = AllocatePages (1);
  Size = 0x20;
  PssiGetPbfVersionDescription (PhyPbfBuff,&Size);

  CopyMem (mReleaseDate, (PhyPbfBuff + PhyPbfBuff[0]), sizeof (mReleaseDate));

  LogSmbiosData ((EFI_SMBIOS_TABLE_HEADER*)&mFirmwareInventoryInfoType45, mFirmwareInventoryInfoType45Strings, NULL);
  if (PhyPbfBuff != NULL) {
   FreePages (PhyPbfBuff, 1);
  }
}


/**
   Installs SMBIOS information for Phytium platforms.

   @param ImageHandle     Module's image handle
   @param SystemTable     Pointer of EFI_SYSTEM_TABLE

   @retval EFI_SUCCESS    Smbios data successfully installed
   @retval Other          Smbios data was not installed

**/
EFI_STATUS
EFIAPI
CommonSmbiosEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  GetSmbiosInfoFromJson ();

  GetSmbiosMemoryInfo ();

  BIOSInfoUpdateSmbiosType0 ();

  SysInfoUpdateSmbiosType1 ();

  EnclosureInfoUpdateSmbiosType3 (); // Add Type 3 first to get chassis handle for use in Type 2

  BoardInfoUpdateSmbiosType2 ();

  PlatSmbiosEntry ();

  PortConnectorInfoUpdateSmbiosType8 ();

  SysSlotInfoUpdateSmbiosType9 ();

  OemStringsUpdateSmbiosType11 ();

  BiosLanguageUpdateSmbiosType13 ();

  PhyMemArrayInfoUpdateSmbiosType16 ();

  MemDevInfoUpdateSmbiosType17 ();

  MemArrMapInfoUpdateSmbiosType19 ();

  BootInfoUpdateSmbiosType32 ();

  IpmiDevInfoUpdateSmbiosType38 ();

  OnBoardDevInfoUpdateSmbiosType41 ();

  FirmwareInventoryInformationType45 ();

  return EFI_SUCCESS;
}
