/** @file
  The Header of Protocol For SPI.

  Copyright (C) 2020, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef SPI_H_
#define SPI_H_

extern EFI_GUID gSpiMasterProtocolGuid;
typedef struct _EFI_SPI_DRV_PROTOCOL EFI_SPI_DRV_PROTOCOL;

#define REG_WP_REG_ENABLE               0x1
#define REG_WP_REG_DISABLE              0x0
#define REG_WP_REG_LEVEL_HIGH           0x1
#define REG_WP_REG_LEVEL_LOW            0x0
#define REG_WP_REG_HOLD_TIME_DEFAULT    0x50
#define REG_WP_REG_SET_UP_TIME_DEFAULT  0xF

#define FLASH_WREN              0x6
#define FLASH_WRSR              0x1
#define FLASH_RDSR              0x5
#define FLASH_STAREG_SPR0       0x80
#define FLASH_STAREG_BP_OFFSET  2

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_INIT_INTERFACE)  (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_SET_CONFIG_INTERFACE)(
  IN  UINT8  CmdId,
  IN  UINT32 Config,
  IN  UINTN  RegAddr
  );

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_GET_CONFIG_INTERFACE)(
  IN  UINT8  CmdId,
  OUT UINT32 *Config,
  IN  UINTN  RegAddr
  );

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_CONFIG_MODE_INTERFACE)(
  IN  UINT32 Config
  );

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_WP_CONFIG_INTERFACE)(
  IN  UINT32 Enable,
  IN  UINT32 WpLevel,
  IN  UINT32 WpHold,
  IN  UINT32 WpSetUp
);

typedef
VOID
(EFIAPI *SPI_WP_DELAY_CONFIG_INTERFACE)(
  IN  UINT16  Delay
);

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_STATUS_REIGSTER_CONFIG_INTERFACE)(
  IN  UINT32 ConfigBit
);


typedef
EFI_STATUS
(EFIAPI *SPI_DRV_SET_REGISTER_INTERFACE)(
  IN  UINT32 Value,
  IN  UINTN  RegAddr
);

typedef
EFI_STATUS
(EFIAPI *SPI_DRV_GET_REGISTER_INTERFACE)(
  IN  UINT32 *Value,
  IN  UINTN  RegAddr
);

struct _EFI_SPI_DRV_PROTOCOL{
  UINT16                                    WpDelay;
  SPI_DRV_INIT_INTERFACE                    SpiInit;
  SPI_DRV_SET_CONFIG_INTERFACE              SpiSetConfig;
  SPI_DRV_GET_CONFIG_INTERFACE              SpiGetConfig;
  SPI_DRV_CONFIG_MODE_INTERFACE             SpiSetMode;
  SPI_DRV_WP_CONFIG_INTERFACE               SpiWpConfig;
  SPI_WP_DELAY_CONFIG_INTERFACE             SpiWpDelayConfig;
  SPI_DRV_SET_REGISTER_INTERFACE            SpiSetRegister;
  SPI_DRV_GET_REGISTER_INTERFACE            SpiGetRegister;
  SPI_DRV_STATUS_REIGSTER_CONFIG_INTERFACE  SpiFlashStatusRegisterConfig;
};

#endif // SPI_H_
