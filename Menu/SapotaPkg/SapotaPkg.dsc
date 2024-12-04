## @file
# This package provides common open source Phytium Platform modules.
#
# Copyright (C) 2022-2024, Phytium Technology Co., Ltd. All rights reserved.<BR>
#
# SPDX-License-Identifier:BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = SapotaPkg
  PLATFORM_GUID                  = E9CC0982-1FA5-11ED-8638-0BE2D7512F50
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x0001001C
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Phytium/SapotaPkg/SapotaPkg.fdf
  PLATFORM_PACKAGE               = Platform/Phytium/SapotaPkg
  GENERAL_PACKAGE                = Silicon/Phytium/PhytiumCommonPkg
  SILICON_PACKAGE                = Silicon/Phytium/S5000CPkg
  CAPSULE_DIR                    = $(SILICON_PACKAGE)/Capsule

  DEFINE I2C_RTC_ENABLE          = TRUE #sunrui change I2C_RTC_ENABLE FALSE -> TRUE
  DYNAMIC_TABLES_FRAMEWORK       = TRUE
  DEFINE I2C_DDR_ENABLE          = FALSE
  DEFINE SPIM_DXE_ENABLE         = FALSE
  DEFINE I2C_DXE_ENABLE          = FALSE
  DEFINE NETWORK_ENABLE          = TRUE
  DEFINE NETWORK_IP6_ENABLE      = FALSE
  DEFINE INTEL_NETWORK_ENABLE    = FALSE
  DEFINE PXE_ENABLE              = TRUE
  DEFINE POST_CODE_ENABLE        = TRUE
  DEFINE APEI_ENABLE             = TRUE
  DEFINE CAPSULE_ENABLE          = TRUE
  DEFINE BIOS_PASSWORD           = TRUE
  DEFINE SECURE_BOOT_ENABLE      = TRUE
  DEFINE PRINT_SCREEN_SUPPORT    = TRUE

!include $(SILICON_PACKAGE)/S5000C.dsc.inc
!include $(GENERAL_PACKAGE)/PhytiumCommonPkg.dsc.inc
!if $(DYNAMIC_TABLES_FRAMEWORK) == TRUE
!include DynamicTablesPkg/DynamicTables.dsc.inc
!include Silicon/Phytium/PhytiumCommonPkg/ConfigurationManager/ConfigurationManager.dsc.inc
!endif

[LibraryClasses.common]
  # Phytium Platform library
  ArmPlatformLib|$(SILICON_PACKAGE)/Library/PlatformLib/PlatformLib.inf

!if $(CAPSULE_ENABLE)
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibFmp/DxeCapsuleLib.inf
  DisplayUpdateProgressLib|MdeModulePkg/Library/DisplayUpdateProgressLibText/DisplayUpdateProgressLibText.inf
  EdkiiSystemCapsuleLib|SignedCapsulePkg/Library/EdkiiSystemCapsuleLib/EdkiiSystemCapsuleLib.inf
  FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  IniParsingLib|SignedCapsulePkg/Library/IniParsingLib/IniParsingLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PlatformFlashAccessLib|$(GENERAL_PACKAGE)/Library/PlatformFlashAccessLib/PlatformFlashAccessLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
!else
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
!endif


  #
  # I2c Lib
  #
  I2cLib|$(GENERAL_PACKAGE)/Library/I2cLib/I2cLib.inf
  I2cRuntimeLib|$(GENERAL_PACKAGE)/Library/I2cLib/I2cRuntimeLib.inf
  PhytiumPowerControlLib|$(SILICON_PACKAGE)/Library/PhytiumPowerControlLib/PhytiumPowerControlLib.inf
  #
  # RTC Driver
  #
  !if $(I2C_RTC_ENABLE) == TRUE
    RealTimeClockLib|$(GENERAL_PACKAGE)/Library/I2cRtcLib/I2cRtcLib.inf
  !else
    RealTimeClockLib|EmbeddedPkg/Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
  !endif
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf

  #
  # PL011 UART Driver and Dependency Libraries
  #
  SerialPortLib|ArmPlatformPkg/Library/PL011SerialPortLib/PL011SerialPortLib.inf
  PL011UartClockLib|ArmPlatformPkg/Library/PL011UartClockLib/PL011UartClockLib.inf
  PL011UartLib|ArmPlatformPkg/Library/PL011UartLib/PL011UartLib.inf

  #
  # ACPI Dependency Libraries
  #
  AcpiHelperLib|DynamicTablesPkg/Library/Common/AcpiHelperLib/AcpiHelperLib.inf
  AcpiLib|EmbeddedPkg/Library/AcpiLib/AcpiLib.inf
  DynamicAmlLib|DynamicTablesPkg/Library/Common/AmlLib/AmlLib.inf
  AcpiHelperLib|DynamicTablesPkg/Library/Common/AcpiHelperLib/AcpiHelperLib.inf

  #
  # Pssi Library
  #
  PssiLib|$(GENERAL_PACKAGE)/Library/PssiLib/PssiLib.inf

  SpiLib|$(GENERAL_PACKAGE)/Library/SpiLib/SpiLib.inf
  ScmiLib|$(GENERAL_PACKAGE)/Library/ScmiLib/ScmiLib.inf
  ScmiProtocolLib|$(GENERAL_PACKAGE)/Library/ScmiProtocolLib/ScmiProtocolLib.inf
  #
  # Iort Library
  #
  !if $(DYNAMIC_TABLES_FRAMEWORK) == TRUE
  IortTableLib|$(SILICON_PACKAGE)/Library/IortTableLib/IortTableLib.inf
  IortGeneratorLib|$(GENERAL_PACKAGE)/Library/IortGeneratorLib/IortGeneratorLib.inf
  !endif
  DynamicAmlLib|DynamicTablesPkg/Library/Common/AmlLib/AmlLib.inf

  GpioLib|$(GENERAL_PACKAGE)/Library/GpioLib/Gpio.inf
  #
  #I3cLib
  #
  I3cLib|$(GENERAL_PACKAGE)/Library/I3cLib/I3cLib.inf
  CrcLib|$(GENERAL_PACKAGE)/Library/BaseCrcLib/BaseCrcLib.inf
  #
  #IPMI
  #
  IpmiCommandLib|$(GENERAL_PACKAGE)/Library/IpmiCommandLib/IpmiCommandLib.inf
  IpmiDeviceInfoToBmcLib|$(GENERAL_PACKAGE)/Library/IpmiDeviceInfoToBmc/IpmiDeviceInfoToBmc.inf
  BmcConfigBootLib|$(GENERAL_PACKAGE)/Library/BmcConfigBootLib/BmcConfigBootLib.inf

  #
  #Smbios
  #
  JsonLib|RedfishPkg/Library/JsonLib/JsonLib.inf
  Ucs2Utf8Lib|RedfishPkg/Library/BaseUcs2Utf8Lib/BaseUcs2Utf8Lib.inf
  RedfishCrtLib|RedfishPkg/PrivateLibrary/RedfishCrtLib/RedfishCrtLib.inf
  BaseSortLib|MdeModulePkg/Library/BaseSortLib/BaseSortLib.inf
  PlatformSmbiosLib|$(SILICON_PACKAGE)/Library/PlatformSmbiosLib/PlatformSmbiosLib.inf


  #Networking Requirements
  NetLib|NetworkPkg/Library/DxeNetLib/DxeNetLib.inf
  DpcLib|NetworkPkg/Library/DxeDpcLib/DxeDpcLib.inf
  UdpIoLib|NetworkPkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  IpIoLib|NetworkPkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  #
  #MM
  #
  MmFrameLib|$(GENERAL_PACKAGE)/Library/MmFrameLib/MmFrameLib.inf
  MmInterfaceLib|$(GENERAL_PACKAGE)/Library/MmInterfaceLib/MmInterfaceLib.inf

  #BMC
  BmcBaseLib|$(SILICON_PACKAGE)/Library/BmcBaseLib/BmcBaseLib.inf
  #X100Lib
  X100ConfigLib|$(GENERAL_PACKAGE)/Library/X100ConfigLib/X100ConfigLib.inf

  PostCodeLib|$(GENERAL_PACKAGE)/Library/PostCodeLib/PostCodeLib.inf
  StringBaseLib|$(GENERAL_PACKAGE)/Library/StringBaseLib/StringBaseLib.inf

# SWJ ScreenPrint {
!if $(PRINT_SCREEN_SUPPORT) == TRUE
   ScreenPrintLib|$(GENERAL_PACKAGE)/Library/ScreenPrintLib/ScreenPrintLib.inf
!endif
# SWJ ScreenPrint }
   EncryptionLib|$(GENERAL_PACKAGE)/Library/EncryptionLib/EncryptionLib.inf

[LibraryClasses.common.PEIM]
  IpmiBaseLib|$(GENERAL_PACKAGE)/Library/PeiIpmiBaseLib/PeiIpmiBaseLib.inf

[LibraryClasses.common.DXE_DRIVER]
  PlatformBootManagerLib|$(PLATFORM_PACKAGE)/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  IpmiBaseLib|$(GENERAL_PACKAGE)/Library/IpmiBaseLib/IpmiBaseLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
!if $(CAPSULE_ENABLE)
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibFmp/DxeRuntimeCapsuleLib.inf
!else
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
!endif

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFixedAtBuild.common]
  #
  # The FIRMWARE_VER string will auto created after platform build.
  # if there is no git repository, FIRMWARE_VER will be undefined.
  #
  #!ifdef $(FIRMWARE_VER)
    gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"UEFI-V1.4-025" #sunrui change $(FIRMWARE_VER)->UEFI-V1.4
  #!else
  #  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"Development Build Phytium S5000C"
  #!endif

  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"UEFI-V1.4-025"
  #gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"SapotaPkg Platform"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Chaoyue"

  #
  # The offset of the same controller in two adjacent DIE is 16TB
  # DIE index left shift 44 BIT
  #
  gPhytiumPlatformTokenSpaceGuid.PcdDieOffset | 44

  gArmTokenSpaceGuid.PcdVFPEnabled|1
  gArmTokenSpaceGuid.PcdArmPrimaryCoreMask|0x303
  gArmTokenSpaceGuid.PcdArmPrimaryCore|0x0
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|48

  #
  # Arm Cores and Clusters
  # Set the value according to the actual situation.
  #
  gArmPlatformTokenSpaceGuid.PcdCoreCount|1
  gArmPlatformTokenSpaceGuid.PcdClusterCount|2

  #
  # NV Storage PCDs
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64|0x7a0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase64|0x7b0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase64|0x7c0000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize|0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|FALSE

  #
  # Size of the region used by UEFI in permanent memory (Reserved 64MB)
  #
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x04000000

  #
  # PL011 - Serial Terminal
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|0x20001000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultReceiveFifoDepth|0
  gArmPlatformTokenSpaceGuid.PL011UartClkInHz|48000000
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
  gArmPlatformTokenSpaceGuid.PL011UartInterrupt|47


  #
  # Debug Serial
  #
  gArmPlatformTokenSpaceGuid.PcdSerialDbgRegisterBase|0x20000000
  gArmPlatformTokenSpaceGuid.PcdSerialDbgUartBaudRate|115200
  gArmPlatformTokenSpaceGuid.PcdSerialDbgUartClkInHz|48000000
    #
    # UART2 Serial
   #
   gArmPlatformTokenSpaceGuid.PcdSerial2RegisterBase|0x20002000
   gArmPlatformTokenSpaceGuid.PcdSerial2UartBaudRate|115200
   gArmPlatformTokenSpaceGuid.PcdSerial2UartClkInHz|48000000
  #
  # ARM General Interrupt Controller
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x22000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x220c0000


  #
  # GPIO
  #
  gPhytiumPlatformTokenSpaceGuid.PcdGpioControllerBase | 0x2000A000
  gPhytiumPlatformTokenSpaceGuid.PcdGpioMaxIndexNumber | 1
  gPhytiumPlatformTokenSpaceGuid.PcdGpioMaxPortNumber | 32

  #
  # SCMI
  #
  gPhytiumPlatformTokenSpaceGuid.PcdMhuBaseAddress|0x38001120
  gPhytiumPlatformTokenSpaceGuid.PcdMhuShareMemoryBase|0x38004C00
  gPhytiumPlatformTokenSpaceGuid.PcdMhuConfigBaseAddress|0x38001500

  #
  # Rtc Info
  #
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerBaseAddress|0x20003000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerSlaveAddress|0x32 #sunrui change 0x68-> 0x32
  gPhytiumPlatformTokenSpaceGuid.PcdRtcI2cControllerSpeed|400000
  gPhytiumPlatformTokenSpaceGuid.PcdRtcBaseAddress|0x2003E000

  #
  # System IO space
  #
  gPhytiumPlatformTokenSpaceGuid.PcdSystemIoBase|0x2000000
  gPhytiumPlatformTokenSpaceGuid.PcdSystemIoSize|0x3E000000
  gEfiMdePkgTokenSpaceGuid.PcdPciIoTranslation|0x50000000

  #
  # System Memory (2GB ~ 4GB - 64MB), the top 64MB is reserved for
  # PBF(the processor basic firmware, Mainly deals the initialization
  # of the chip).
  #
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x70000000
  #
  # Pci Config Size
  #
  gPhytiumPlatformTokenSpaceGuid.PcdPciConfigSize|0x10000000

  #
  # Stack Size
  #
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x1A0C0000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x8000

  #
  # SBSA Watchdog
  # WDT0 refresh base 0x1A102000, size is 0x1000
  # WDT0 control base 0x1A103000, size is 0x1000
  # WDT0 interrupt number is 52
  #
  gArmTokenSpaceGuid.PcdGenericWatchdogControlBase|0x1A103000
  gArmTokenSpaceGuid.PcdGenericWatchdogRefreshBase|0x1A102000
  gArmTokenSpaceGuid.PcdGenericWatchdogEl2IntrNum|50

  #
  # SPI Flash Control Register Base Address and Size
  #
  gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashBase|0
  gPhytiumPlatformTokenSpaceGuid.PcdSpiFlashSize|0x1000000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerBase|0x1A100000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerSize|0x1000

  #
  # DDR Info
  #
  gPhytiumPlatformTokenSpaceGuid.PcdSpdI2cControllerBaseAddress | 0x20005000
  gPhytiumPlatformTokenSpaceGuid.PcdSpdI2cControllerSpeed | 400000
  gPhytiumPlatformTokenSpaceGuid.PcdDdrI2cAddress             | {0x50}

  #
  # DDR Info - I3C Bus
  #
  gPhytiumPlatformTokenSpaceGuid.PcdDdrChannelCount | 2
  gPhytiumPlatformTokenSpaceGuid.PcdDdrDimmCount | 1
  gPhytiumPlatformTokenSpaceGuid.PcdDdrDimmNumberArray64C | {1, 0, 6, 7, 2, 3, 5, 4, 9, 8, 14, 15, 10, 11, 13, 12}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrSpdHubAddressArray64C | {0x51, 0x50, 0x56, 0x57, 0x52, 0x53, 0x55, 0x54, 0x51, 0x50, 0x56, 0x57, 0x52, 0x53, 0x55, 0x54}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrPmicAddressArray64C | {0x49, 0x48, 0x4e, 0x4f, 0x4a, 0x4b, 0x4d, 0x4c, 0x49, 0x48, 0x4e, 0x4f, 0x4a, 0x4b, 0x4d, 0x4c}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrDimmNumberArray32C | {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrSpdHubAddressArray32C | {0x50, 0x51, 0x52, 0x53, 0x50, 0x51, 0x52, 0x53, 0x50, 0x51, 0x52, 0x53, 0x50, 0x51, 0x52, 0x53}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrPmicAddressArray32C | {0x48, 0x49, 0x4a, 0x4b ,0x48, 0x49, 0x4a, 0x4b ,0x48, 0x49, 0x4a, 0x4b ,0x48, 0x49, 0x4a, 0x4b}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrDimmNumberArray16C | {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrSpdHubAddressArray16C | {0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50}
  gPhytiumPlatformTokenSpaceGuid.PcdDdrPmicAddressArray16C | {0x49, 0x48, 0x49, 0x48, 0x49, 0x48, 0x49, 0x48, 0x49, 0x48, 0x49, 0x48, 0x49, 0x48, 0x49, 0x48}
  gPhytiumPlatformTokenSpaceGuid.PcdSpdMatserI3cBaseAddress | 0x21000000
  gPhytiumPlatformTokenSpaceGuid.PcdSpdMatserI2cBaseAddress | 0x20005000


  #
  # I2C Info
  #
  gPhytiumPlatformTokenSpaceGuid.PcdI2cClkFreq| 48000000
  ##The followed PCD define the value filled in IC_XX_SCL_HCNT and IC_XX_SCL_LCNT
  gPhytiumPlatformTokenSpaceGuid.PcdI2cSsSclLcnt|0xf0
  gPhytiumPlatformTokenSpaceGuid.PcdI2cSsSclHcnt|0xf0
  gPhytiumPlatformTokenSpaceGuid.PcdI2cFsSclLcnt|0x3c
  gPhytiumPlatformTokenSpaceGuid.PcdI2cFsSclHcnt|0x3c
  gPhytiumPlatformTokenSpaceGuid.PcdI2cHsSclLcnt|0x7
  gPhytiumPlatformTokenSpaceGuid.PcdI2cHsSclHcnt|0x7
  ##The followed PCD define the value filled in IC_FS_SPKLEN and IC_HS_SPKLEN
  ##Use the default value now
  gPhytiumPlatformTokenSpaceGuid.PcdI2cHsSpkLen|5
  gPhytiumPlatformTokenSpaceGuid.PcdI2cFsSpkLen|2

  ##PcdI2cSlaveAddress and PcdI2cSlaveAddress need one to one corresondence
  gPhytiumPlatformTokenSpaceGuid.PcdI2cSlaveAddress|{ 0x68 }
  gPhytiumPlatformTokenSpaceGuid.PcdI2cSlaveBusesNumber|{ 0x1 }
  ##PcdDevicesAddress and PcdI2cDevicesBusesNumber need one to one corresondence
  gPhytiumPlatformTokenSpaceGuid.PcdDevicesAddress|{ 0x68 }
  gPhytiumPlatformTokenSpaceGuid.PcdI2cDevicesBusesNumber|{ 0x1 }
  gPhytiumPlatformTokenSpaceGuid.PcdI2cBusSpeed|400000
  gPhytiumPlatformTokenSpaceGuid.PcdI2cRuntimeBaseAddress|0x20004000
  gPhytiumPlatformTokenSpaceGuid.PcdI2cControllerInformation.I2cControllerNumber | 3
  gPhytiumPlatformTokenSpaceGuid.PcdI2cControllerInformation.I2cControllerAddress[0] | 0x20003000
  gPhytiumPlatformTokenSpaceGuid.PcdI2cControllerInformation.I2cControllerAddress[1] | 0x20004000
  gPhytiumPlatformTokenSpaceGuid.PcdI2cControllerInformation.I2cControllerAddress[2] | 0x20005000

  #
  #MM Communication Buffer
  #
  gArmTokenSpaceGuid.PcdMmBufferBase|0xFC400000
  gArmTokenSpaceGuid.PcdMmBufferSize|0x100000

  # Spi Controller Base Address
  #
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerClockFrequency|500000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerMaxFrequency|250000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerInformation.SpiControllerNumber | 2
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerInformation.SpiControllerAddress[0] | 0x20006000
  gPhytiumPlatformTokenSpaceGuid.PcdSpiControllerInformation.SpiControllerAddress[1] | 0x20007000

  #
  # Kcs Base Address
  #
  gPhytiumPlatformTokenSpaceGuid.PcdKcsBaseAddress|0x8000000

  #
  # POST Code
  # PcdPostCodeOutputPort | POST_CODE_KCS_DEBUG     BIT0
  # PcdPostCodeOutputPort | POST_CODE_SERIAL_DEBUG  BIT1
  #
!if $(POST_CODE_ENABLE) == TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdPostCodeOutputPort | 3
!endif
  #
  # default optvalue
  #
  gPhytiumPlatformTokenSpaceGuid.PcdDefaultPowerDownValue|85
  gPhytiumPlatformTokenSpaceGuid.PcdDefaultCrtLower|5

  #
  # Ras Strategy
  #
  gPhytiumPlatformTokenSpaceGuid.PcdRasBmcCperEnable|TRUE
  gPhytiumPlatformTokenSpaceGuid.PcdRasFatalErrorRecord|0x0
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemFatalUceIsoStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemFatalUceIsoStrategyResetClean|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemFatalUceIsoStrategyMethod|0x2
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemFatalUceIsoStrategyRecord|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemFatalUceIsoStrategyCount|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemNonFatalUceIsoStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemNonFatalUceIsoStrategyResetClean|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemNonFatalUceIsoStrategyMethod|0x2
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemNonFatalUceIsoStrategyRecord|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemNonFatalUceIsoStrategyCount|0x5
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeIsoStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeIsoStrategyResetClean|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeIsoStrategyMethod|0x2
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeIsoStrategyRecord|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeIsoStrategyCount|0xA
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuFatalIsoStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuFatalIsoStrategyResetClean|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuFatalIsoStrategyMethod|0x2
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuFatalIsoStrategyRecord|0x1
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeReportStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemCeReportStrategyCount|0x32
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuCeReportStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuCeReportStrategyCount|0x32
  gPhytiumPlatformTokenSpaceGuid.PcdRasPcieCeReportStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasPcieCeReportStrategyCount|0x32
  gPhytiumPlatformTokenSpaceGuid.PcdRasFatalErrorFlashAddress|0x7D0000
  gPhytiumPlatformTokenSpaceGuid.PcdRasCpuRecordFlashAddress|0x7E0000
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryRecordFlashAddress|0x7F0000
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyEnable|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyResetClean|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyType|0
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryIsoRecord|0x1

[PcdsFeatureFlag.common]
  gEmbeddedTokenSpaceGuid.PcdCacheEnable|TRUE

  #
  # Capsule Update Reset Enable
  #
!if $(CAPSULE_ENABLE)
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE
!endif

[PcdsDynamicDefault.common.DEFAULT]
  ## This PCD defines the video horizontal resolution.
  ##  This PCD could be set to 0 then video resolution could be at highest resolution.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|800
  ## This PCD defines the video vertical resolution.
  ##  This PCD could be set to 0 then video resolution could be at highest resolution.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|600

  #  This PCD could be set to 0 then video resolution could be at highest resolution.
  ## This PCD defines the Console output row and the default value is 80 according to UEFI spec.
  ##This PCD could be set to 0 then console output could be at max column and max row.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|128
  ## This PCD defines the Console output column and the default value is 25 according to UEFI spec.
  ##  This PCD could be set to 0 then console output could be at max column and max row.
  #gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40
  #
  ## Specify the video horizontal resolution of text setup.
  ## @Prompt Video Horizontal Resolution of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|800

  ## Specify the video vertical resolution of text setup.
  ## @Prompt Video Vertical Resolution of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|600
  # @Prompt Video Horizontal Resolution of Text Setup

  ## Specify the console output column of text setup.
  ## @Prompt Console Output Column of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
  ## Specify the console output row of text setup.
  ## @Prompt Console Output Row of Text Setup
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40

  
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|128
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|40

  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
  #gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40
  #
  #设置BIOS SETUP设置页面分辨率，及调试显示的行数，可控制显示内容的大小
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1024
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|768
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|128
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|40

  ## The number of seconds that the firmware will wait before initiating the original default boot selection.
  #  A value of 0 indicates that the default boot selection is to be initiated immediately on boot.
  #  The value of 0xFFFF then firmware will wait for user input before booting.
  # @Prompt Boot Timeout (s)
  # Please changed with the PcdPlatformBootTimeOutDefault bellow
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|3
  # @Prompt Boot Timeout Default use for F9 LoadDefault
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOutDefault|3

  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyEnableConfig|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyResetCleanConfig|FALSE
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryAddressIsolationStrategyTypeConfig|0
  gPhytiumPlatformTokenSpaceGuid.PcdRasMemoryIsoRecordConfig|0x1

!if $(CAPSULE_ENABLE)
[PcdsDynamicExDefault.common.DEFAULT]
  gEfiMdeModulePkgTokenSpaceGuid.PcdS3BootScriptTablePrivateDataPtr|0x0
  gEfiSignedCapsulePkgTokenSpaceGuid.PcdEdkiiSystemFirmwareImageDescriptor|{0x0}|VOID*|0x100
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxSizeNonPopulateCapsule|0xE00000
  gEfiMdeModulePkgTokenSpaceGuid.PcdSystemFmpCapsuleImageTypeIdGuid|{0x42, 0x90, 0xD8, 0x26, 0xC3, 0x2C, 0xEC, 0x11, 0xA5, 0xC0, 0xDB, 0x1A, 0x3C, 0xB7, 0xE4, 0x95}
  gEfiSignedCapsulePkgTokenSpaceGuid.PcdEdkiiSystemFirmwareFileGuid|{0x6A, 0x5B, 0xBD, 0x4E, 0xC3, 0x2C, 0xEC, 0x11, 0xB7, 0x3F, 0x17, 0xCB, 0x46, 0x12, 0x63, 0x96}
!endif

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
  #
  # PEI Phase modules
  #
  ArmPlatformPkg/PrePeiCore/PrePeiCoreUniCore.inf
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

!if $(CAPSULE_ENABLE)
  MdeModulePkg/Universal/CapsulePei/CapsulePei.inf
  $(CAPSULE_DIR)/SystemFirmwareDescriptor/SystemFirmwareDescriptor.inf
!endif

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/CpuPei/CpuPei.inf
  UefiCpuPkg/CpuIoPei/CpuIoPei.inf
  MdeModulePkg/Universal/FaultTolerantWritePei/FaultTolerantWritePei.inf
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf
  ArmPlatformPkg/MemoryInitPei/MemoryInitPeim.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/Pei/ReportStatusCodeRouterPei.inf
  MdeModulePkg/Universal/StatusCodeHandler/Pei/StatusCodeHandlerPei.inf
  $(GENERAL_PACKAGE)/Drivers/GenericIpmi/Pei/PeiGenericIpmi.inf

  #
  # PCD database
  #
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
  ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  ShellPkg/Application/Shell/Shell.inf {
    <LibraryClasses>
      ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
      NULL|ShellPkg/Library/UefiShellLevel2CommandsLib/UefiShellLevel2CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel1CommandsLib/UefiShellLevel1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellLevel3CommandsLib/UefiShellLevel3CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellDriver1CommandsLib/UefiShellDriver1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellAcpiViewCommandLib/UefiShellAcpiViewCommandLib.inf
      NULL|ShellPkg/Library/UefiShellDebug1CommandsLib/UefiShellDebug1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellInstall1CommandsLib/UefiShellInstall1CommandsLib.inf
      NULL|ShellPkg/Library/UefiShellNetwork1CommandsLib/UefiShellNetwork1CommandsLib.inf
      NULL|$(GENERAL_PACKAGE)/Library/UefiShellOemCommandLib/UefiShellOemCommandsLib.inf
      HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
      BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf
      OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  }

  #
  # Dxe core entry
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf

  #
  # DXE driver
  #
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf

  #
  # Common Arm Timer and Gic Components
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf

  #
  # security system
  #
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
  }

  #
  # FV Filesystem
  #
  MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf

  #
  # Common Console Components
  #
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  $(GENERAL_PACKAGE)/Console/ConSplitterDxe/ConSplitterDxe.inf
  $(GENERAL_PACKAGE)/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  $(GENERAL_PACKAGE)/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf

  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf

  #
  # Hii database init
  #
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf

  #
  # Generic Watchdog Timer
  #
  ArmPkg/Drivers/GenericWatchdogDxe/GenericWatchdogDxe.inf

  #
  # Flash Fvb protocol
  #
  $(GENERAL_PACKAGE)/Drivers/FlashFvbDxe/FlashFvbDxe.inf


  #
  # I2C Driver
  #
!if $(I2C_DXE_ENABLE) == TRUE
  $(GENERAL_PACKAGE)/Drivers/I2c/PhytiumI2cDxe/PhytiumI2cDxe.inf
  MdeModulePkg/Bus/I2c/I2cDxe/I2cDxe.inf
  $(GENERAL_PACKAGE)/Drivers/I2c/PhytiumI2cDevicesIo/PhytiumI2cDevicesIo.inf
!endif

  #
  # Spi Dxe
  #
!if $(SPIM_DXE_ENABLE) == TRUE
  $(GENERAL_PACKAGE)/Drivers/PhytiumSpiM/PhytiumSpiM.inf
!endif

  #
  # IDE/AHCI Support
  #
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf

  MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf

!if $(PRINT_SCREEN_SUPPORT) == TRUE
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxeScreen.inf
!else
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
!endif

  MdeModulePkg/Bus/Usb/UsbMouseDxe/UsbMouseDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf


  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf

  #
  # NVME Support
  #
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # SMBIOS
  #
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  $(GENERAL_PACKAGE)/Drivers/CommonSmbiosDxe/CommonSmbiosDxe.inf

  #
  # IPMI driver
  #
  $(GENERAL_PACKAGE)/Drivers/GenericIpmi/Dxe/DxeIpmiInit.inf

!if $(NETWORK_ENABLE) == TRUE
  NetworkPkg/SnpDxe/SnpDxe.inf
  NetworkPkg/DpcDxe/DpcDxe.inf
  NetworkPkg/MnpDxe/MnpDxe.inf
  NetworkPkg/ArpDxe/ArpDxe.inf
  NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
  NetworkPkg/Ip4Dxe/Ip4Dxe.inf
  NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf
  NetworkPkg/Udp4Dxe/Udp4Dxe.inf
  #NetworkPkg/VlanConfigDxe/VlanConfigDxe.inf
!endif

!if $(NETWORK_IP6_ENABLE) == TRUE
  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
  NetworkPkg/TcpDxe/TcpDxe.inf
!endif

#
#Platform
#
$(PLATFORM_PACKAGE)/Drivers/PlatformDxe/PlatformDxe.inf

!if $(PXE_ENABLE) == TRUE
  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf
!endif
  #
  #MM Communicate Buffer
  #
!if $(APEI_ENABLE) == TRUE
  ArmPkg/Drivers/MmCommunicationDxe/MmCommunication.inf
  $(SILICON_PACKAGE)/Drivers/Apei/Apei.inf
!endif

  #
  # ACPI
  #
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  $(PLATFORM_PACKAGE)/AcpiTables/AcpiTables.inf
  $(GENERAL_PACKAGE)/Drivers/AcpiPlatformDxe/AcpiPlatformDxe.inf
  Silicon/Phytium/S5000CPkg/Drivers/AcpiUpdateDxe/AcpiUpdateDxe.inf

  #
  # Phytium Bmc Gop
  #
  $(GENERAL_PACKAGE)/Drivers/BmcGopDxe/PhyGopDxe.inf

  #
  # Phytium X100 Gop
  #
  $(GENERAL_PACKAGE)/Drivers/PhyX100GopDxe/PhyGopDxe.inf

  #
  # Firmware Capsule Update Libraries
  #
!if $(CAPSULE_ENABLE)
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/EsrtDxe/EsrtDxe.inf
  SignedCapsulePkg/Universal/SystemFirmwareUpdate/SystemFirmwareReportDxe.inf {
    <LibraryClasses>
      FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  }
  SignedCapsulePkg/Universal/SystemFirmwareUpdate/SystemFirmwareUpdateDxe.inf {
    <LibraryClasses>
      FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  }
  MdeModulePkg/Application/CapsuleApp/CapsuleApp.inf
!endif
  #
  #Bios Update Spi NorFlash
  #
  $(GENERAL_PACKAGE)/App/UpdateBiosSpiNorflashApp/UpdateBiosApp.inf
  #sunrui - Andvanced config Feature 
  $(SILICON_PACKAGE)/Drivers/FeatureUiDxe/FeatureUiDxe.inf
  #
  #Common Ras Strategy Config UI
  #sunrui - Ras configuration
  #$(GENERAL_PACKAGE)/Drivers/RasConfigUiDxe/RasConfig.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf {
    <LibraryClasses>
    !if $(CAPSULE_ENABLE)
      FmpAuthenticationLib|SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
    !else
      FmpAuthenticationLib|MdeModulePkg/Library/FmpAuthenticationLibNull/FmpAuthenticationLibNull.inf
    !endif
  }
  MdeModulePkg/Application/BootManagerMenuApp/BootManagerMenuApp.inf
  $(GENERAL_PACKAGE)/Setup/DisplayEngineDxe/DisplayEngineDxe.inf
  $(GENERAL_PACKAGE)/Setup/SetupBrowserDxe/SetupBrowserDxe.inf
  #$(GENERAL_PACKAGE)/Setup/DriverSampleDxe/DriverSampleDxe.inf
  $(GENERAL_PACKAGE)/Setup/Information/Information.inf
  $(GENERAL_PACKAGE)/Setup/ShowTimeDxe/ShowTime.inf
  $(GENERAL_PACKAGE)/Setup/BmcInfoUiDxe/BmcInfoUiDxe.inf
  $(GENERAL_PACKAGE)/Setup/FirmwareManagerUiDxe/FirmwareManagerUiDxe.inf
  !if $(BIOS_PASSWORD)
      $(GENERAL_PACKAGE)/Setup/PasswordConfigDxe/PasswordConfigUiDxe.inf
  !endif
  $(GENERAL_PACKAGE)/Setup/Security/Security.inf
  $(GENERAL_PACKAGE)/Setup/UiApp/UiApp.inf {
  #MdeModulePkg/Application/UiApp/UiApp.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/BootManagerUiLib/BootManagerUiLib.inf
      NULL|MdeModulePkg/Library/DeviceManagerUiLib/DeviceManagerUiLib.inf
      NULL|$(GENERAL_PACKAGE)/Setup/Library/BootMaintenanceManagerUiLib/BootMaintenanceManagerUiLib.inf
      NULL|$(GENERAL_PACKAGE)/Setup/Library/AdvancedConfigUiLib/AdvancedConfigUiLib.inf
  }
  
  #
  #Application
  #
  MdeModulePkg/Application/HelloWorld/HelloWorld.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D ENABLE_MD5_DEPRECATED_INTERFACES

!if $(PRINT_SCREEN_SUPPORT) == TRUE
  GCC:*_*_*_CC_FLAGS                  = -D PRINT_SCREEN_SUPPORT
  GCC:*_*_*_VFRPP_FLAGS               = -D PRINT_SCREEN_SUPPORT
!endif
