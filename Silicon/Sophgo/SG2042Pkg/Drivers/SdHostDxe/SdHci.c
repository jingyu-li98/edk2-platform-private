/** @file
  The implementation for handling SD card operations using the SD Host Controller Interface (SDHCI).

  Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
  Copyright (c) 2023, Academy of Intelligent Innovation, Shandong Universiy, China.P.R. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/MmcHost.h>

#include "SdHci.h"

#define SDCARD_INIT_FREQ  (200 * 1000)
#define SDCARD_TRAN_FREQ  (6 * 1000 * 1000)

/**
  Return the clock rate of SD card.

  @retval the clock rate of SD card.

**/
INT32
BmGetSdClk (
  VOID
  )
{
  return 100 * 1000 * 1000;
}

// VOID
// BmMMcHwReset (struct mmc_host *host)
// {
//   UINTN Base;

//   Base  = BmParams.RegBase;

//   // //
//   // // eMMC
//   // //
//   // mmio_write_32(REG_TOP_SOFT_RST0,
//   //   mmio_read_32(REG_TOP_SOFT_RST0) & (~BIT_TOP_SOFT_RST0_EMMC));
//   // gBS->Stall (10);
//   // mmio_write_32(REG_TOP_SOFT_RST0,
//   //   mmio_read_32(REG_TOP_SOFT_RST0) | BIT_TOP_SOFT_RST0_EMMC);
//   // gBS->Stall (10);

//   // // SD
//   // mmio_write_32(REG_TOP_SOFT_RST0,
//   //   mmio_read_32(REG_TOP_SOFT_RST0) & (~BIT_TOP_SOFT_RST0_SD));
//   // gBS->Stall (10);
//   // mmio_write_32(REG_TOP_SOFT_RST0,
//   //   mmio_read_32(REG_TOP_SOFT_RST0) | BIT_TOP_SOFT_RST0_SD);
//   // gBS->Stall (10);

//   // reset hw
//   MmioWrite8 (Base + SDHCI_SOFTWARE_RESET, 0x7);
//   while (MmioRead8 (Base + SDHCI_SOFTWARE_RESET))
//     ;
// }

#if 1
/**
  SD card sends command with response block data.

  @param    Cmd   Command sent by SD card.

  @retval EFI_SUCCESS             The command with response block data was sent successfully.
  @retval EFI_DEVICE_ERROR        There was an error during the command transmission or response handling.
  @retval EFI_TIMEOUT             The command transmission or response handling timed out.

**/
STATIC
EFI_STATUS
SdSendCmdWithData (
  IN MMC_CMD     Cmd,
  IN UINT32      Argument
  )
{
  UINTN   Base;
  UINT32  Mode;
  UINT32  State;
  UINT32  Flags;
  UINT32  Timeout;

  Base  = BmParams.RegBase;
  Mode  = 0;
  Flags = 0;

  //
  // Make sure Cmd line is clear
  //
  while (1) {
    if (!(MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
    break;
  }

  switch (Cmd) {
    case MMC_CMD17:
    case MMC_CMD18:
    case MMC_ACMD51:
      Mode = SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_MULTI | SDHCI_TRNS_READ;
      break;
    case MMC_CMD24:
    case MMC_CMD25:
      Mode = (SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_MULTI) & ~SDHCI_TRNS_READ;
      break;
    default:
      ASSERT(0);
  }

  MmioWrite16 (Base + SDHCI_TRANSFER_MODE, Mode);
  MmioWrite32 (Base + SDHCI_ARGUMENT, Argument);

  //
  // Set cmd flags
  //
  if (Cmd == MMC_CMD0) {
    Flags |= SDHCI_CMD_RESP_NONE;
  } else if (Cmd & MMC_CMD_LONG_RESPONSE) {
    Flags |= SDHCI_CMD_RESP_LONG;
  } else {
    Flags |= SDHCI_CMD_RESP_SHORT;
  }
  Flags |= SDHCI_CMD_CRC;
  Flags |= SDHCI_CMD_INDEX;
  Flags |= SDHCI_CMD_DATA;

  //
  // Issue the Cmd
  //
  MmioWrite16 (Base + SDHCI_COMMAND, SDHCI_MAKE_CMD (MMC_GET_INDX (Cmd), Flags));
  // DEBUG ((DEBUG_WARN, "%a[%d] Cmd | (Flags & 0xff)=0x%lx\t Flags=0x%lx\n", __func__, __LINE__, SDHCI_MAKE_CMD (MMC_GET_INDX (Cmd), Flags), Flags));
  //
  // Check Cmd complete if necessary
  //
  if ((MmioRead16 (Base + SDHCI_TRANSFER_MODE) & SDHCI_TRNS_RESP_INT) == 0) {
    Timeout = 100000;
    while (1) {
      State = MmioRead16 (Base + SDHCI_INT_STATUS);
      if (State & SDHCI_INT_ERROR) {
        DEBUG ((DEBUG_ERROR, "%a: interrupt error: 0x%x 0x%x\n", __func__,  MmioRead16 (Base + SDHCI_INT_STATUS),
                                MmioRead16 (Base + SDHCI_ERR_INT_STATUS)));
        return EFI_DEVICE_ERROR;
      }
      if (State & SDHCI_INT_CMD_COMPLETE) {
        MmioWrite16 (Base + SDHCI_INT_STATUS, State | SDHCI_INT_CMD_COMPLETE);
        break;
      }

      gBS->Stall (1);
      if (!Timeout--) {
        DEBUG ((DEBUG_ERROR, "%a: Timeout!\n", __func__));
        return EFI_TIMEOUT;
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  SD card sends command without response block data.

  @param    Cmd   Command sent by SD card.

  @retval EFI_SUCCESS             The command without response block data was sent successfully.
  @retval EFI_DEVICE_ERROR        There was an error during the command transmission or response handling.
  @retval EFI_TIMEOUT             The command transmission or response handling timed out.

**/
STATIC
EFI_STATUS
SdSendCmdWithoutData (
  IN MMC_CMD    Cmd,
  IN UINT32     Argument
  )
{
  UINTN   Base;
  UINT32  State;
  UINT32  Flags;
  UINT32  Timeout;

  Base    = BmParams.RegBase;
  Flags   = 0x0;
  Timeout = 10000;

  //
  // Make sure Cmd line is clear
  //
  while (1) {
    if (!(MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
      break;
  }

  //
  // set Cmd Flags
  //
  if (Cmd == MMC_CMD0)
    Flags |= SDHCI_CMD_RESP_NONE;
  else if (Cmd == MMC_CMD1)
    Flags |= SDHCI_CMD_RESP_SHORT;
  else if (Cmd == MMC_ACMD41)
    Flags |= SDHCI_CMD_RESP_SHORT;
  else if (Cmd & MMC_CMD_LONG_RESPONSE) {
    Flags |= SDHCI_CMD_RESP_LONG;
    Flags |= SDHCI_CMD_CRC;
  } else {
    Flags |= SDHCI_CMD_RESP_SHORT;
    Flags |= SDHCI_CMD_CRC;
    Flags |= SDHCI_CMD_INDEX;
  }

  //
  // Make sure dat line is clear if necessary
  //
  if (Flags != SDHCI_CMD_RESP_NONE) {
    while (1) {
      if (!(MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT_DAT))
        break;
    }
  }

  //
  // Issue the Cmd
  //
  MmioWrite32 (Base + SDHCI_ARGUMENT, Argument);
  MmioWrite16 (Base + SDHCI_COMMAND, SDHCI_MAKE_CMD (MMC_GET_INDX (Cmd), Flags));

  // DEBUG ((DEBUG_WARN, "%a[%d] Cmd | (Flags & 0xff)=0x%lx\t Flags=0x%lx\n", __func__, __LINE__, SDHCI_MAKE_CMD (MMC_GET_INDX (Cmd), Flags), Flags));

  //
  // Check Cmd complete
  //
  Timeout = 100000;
  while (1) {
    State = MmioRead16 (Base + SDHCI_INT_STATUS);
    if (State & SDHCI_INT_ERROR) {
      DEBUG ((DEBUG_ERROR, "%a: interrupt error: 0x%x 0x%x\n", __func__,  MmioRead16 (Base + SDHCI_INT_STATUS),
                              MmioRead16 (Base + SDHCI_ERR_INT_STATUS)));
      return EFI_DEVICE_ERROR;
    }
    if (State & SDHCI_INT_CMD_COMPLETE) {
      MmioWrite16 (Base + SDHCI_INT_STATUS, State | SDHCI_INT_CMD_COMPLETE);
      break;
    }

    gBS->Stall (1);
    if (!Timeout--) {
      DEBUG ((DEBUG_ERROR, "%a: Timeout!\n", __func__));
      return EFI_TIMEOUT;
    }
  }

  return EFI_SUCCESS;
}
#endif
/**
  SD card sends command.

  @param[in]  MMcCmd    Command ID.
  @param[in]  Argument  Command argument.

  @retval  EFI_SUCCESS             The command was sent successfully.
  @retval  EFI_DEVICE_ERROR        There was an error during the command transmission or response handling.
  @retval  EFI_TIMEOUT             The command transmission or response handling timed out.

**/
EFI_STATUS
EFIAPI
BmSdSendCmd (
  IN  MMC_CMD MMcCmd,
  IN  UINT32  Argument
  )
{
  EFI_STATUS  Status;

  switch (MMcCmd) {
    case MMC_CMD17:
    case MMC_CMD18:
    case MMC_CMD24:
    case MMC_CMD25:
    case MMC_ACMD51:
      Status = SdSendCmdWithData(MMcCmd, Argument);
      break;
    default:
      Status = SdSendCmdWithoutData(MMcCmd, Argument);
  }

  return Status;
  // UINTN   Base;
  // UINT32  State;
  // UINT32  Flags;
  // UINT32  Timeout;

  // Base    = BmParams.RegBase;
  // Flags   = 0x0;
  // Timeout = 10000;

  // //
  // // Make sure Cmd line is clear
  // //
  // while (1) {
  //   if (!(MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
  //     break;
  // }

  // //
  // // set Cmd Flags
  // //
  // if (MMcCmd == MMC_CMD0)
  //   Flags |= SDHCI_CMD_RESP_NONE;
  // else if (MMcCmd == MMC_CMD1)
  //   Flags |= SDHCI_CMD_RESP_SHORT;
  // else if (MMcCmd == MMC_ACMD41)
  //   Flags |= SDHCI_CMD_RESP_SHORT;
  // else if (MMcCmd & MMC_CMD_LONG_RESPONSE) {
  //   Flags |= SDHCI_CMD_RESP_LONG;
  //   Flags |= SDHCI_CMD_CRC;
  // } else {
  //   Flags |= SDHCI_CMD_RESP_SHORT;
  //   Flags |= SDHCI_CMD_CRC;
  //   Flags |= SDHCI_CMD_INDEX;
  // }

  // //
  // // Make sure dat line is clear if necessary
  // //
  // if (Flags != SDHCI_CMD_RESP_NONE) {
  //   while (1) {
  //     if (!(MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT_DAT))
  //       break;
  //   }
  // }

  // //
  // // Issue the Cmd
  // //
  // MmioWrite32 (Base + SDHCI_ARGUMENT, Argument);
  // MmioWrite16 (Base + SDHCI_COMMAND, SDHCI_MAKE_CMD (MMC_GET_INDX (MMcCmd), Flags));

  // // DEBUG ((DEBUG_WARN, "%a[%d] Cmd | (Flags & 0xff)=0x%lx\t Flags=0x%lx\n", __func__, __LINE__, SDHCI_MAKE_CMD (MMC_GET_INDX (Cmd), Flags), Flags));

  // //
  // // Check Cmd complete
  // //
  // Timeout = 100000;
  // while (1) {
  //   State = MmioRead16 (Base + SDHCI_INT_STATUS);
  //   if (State & SDHCI_INT_ERROR) {
  //     DEBUG ((DEBUG_ERROR, "%a: interrupt error: 0x%x 0x%x\n", __func__,  MmioRead16 (Base + SDHCI_INT_STATUS),
  //                             MmioRead16 (Base + SDHCI_ERR_INT_STATUS)));
  //     return EFI_DEVICE_ERROR;
  //   }
  //   if (State & SDHCI_INT_CMD_COMPLETE) {
  //     MmioWrite16 (Base + SDHCI_INT_STATUS, State | SDHCI_INT_CMD_COMPLETE);
  //     break;
  //   }

  //   gBS->Stall (1);
  //   if (!Timeout--) {
  //     DEBUG ((DEBUG_ERROR, "%a: Timeout!\n", __func__));
  //     return EFI_TIMEOUT;
  //   }
  // }

  // return EFI_SUCCESS;
}

/**
  Set clock frequency of SD card.

  @param[in] Clk       The clock frequency of SD card.

**/
VOID
SdSetClk (
  IN INT32 Clk
  )
{
  INT32  I;
  INT32  Div;
  UINTN  Base;

  ASSERT (Clk > 0);

  if (BmParams.ClkRate <= Clk) {
    Div = 0;
  } else {
    for (Div = 0x1; Div < 0xFF; Div++) {
      if (BmParams.ClkRate / (2 * Div) <= Clk)
        break;
    }
  }
  ASSERT (Div <= 0xFF);

  Base = BmParams.RegBase;
  if (MmioRead16 (Base + SDHCI_HOST_CONTROL2) & (1 << 15)) {
    DEBUG ((DEBUG_INFO, "Use SDCLK Preset Value\n"));
  } else {
    DEBUG ((DEBUG_INFO, "Set SDCLK by driver. Div=0x%x(%d)\n", Div, Div));
    MmioWrite16 (Base + SDHCI_CLK_CTRL,
            MmioRead16 (Base + SDHCI_CLK_CTRL) & ~0x9); // disable INTERNAL_CLK_EN and PLL_ENABLE
    MmioWrite16 (Base + SDHCI_CLK_CTRL,
            (MmioRead16 (Base + SDHCI_CLK_CTRL) & 0xDF) | Div << 8); // set Clk Div
    MmioWrite16 (Base + SDHCI_CLK_CTRL,
            MmioRead16 (Base + SDHCI_CLK_CTRL) | 0x1); // set INTERNAL_CLK_EN

    for (I = 0; I <= 150000; I += 100) {
      if (MmioRead16 (Base + SDHCI_CLK_CTRL) & 0x2)
        break;
      gBS->Stall (100);
    }

    if (I > 150000) {
      DEBUG ((DEBUG_ERROR, "%a: SD INTERNAL_CLK_EN setting FAILED!\n", __func__));
      ASSERT(0);
    }

    MmioWrite16 (Base + SDHCI_CLK_CTRL, MmioRead16 (Base + SDHCI_CLK_CTRL) | 0x8); // set PLL_ENABLE

    //
    // Wait max 150 ms
    //
    for (I = 0; I <= 150000; I += 100) {
      if (MmioRead16 (Base + SDHCI_CLK_CTRL) & SDHCI_CLK_INT_STABLE)
        return;
      gBS->Stall (100);
    }
  }

  DEBUG ((DEBUG_INFO, "%a: SD PLL setting FAILED!\n", __func__));
}

/**
  Change clock frequency of SD card.

  @param[in] Clk       The clock frequency of SD card.

**/
VOID
SdChangeClk (
  IN INT32 Clk
  )
{
  INT32  I;
  INT32  Div;
  UINTN  Base;

  ASSERT (Clk > 0);

  if (BmParams.ClkRate <= Clk) {
    Div = 0;
  } else {
    for (Div = 0x1; Div < 0xFF; Div++) {
      if (BmParams.ClkRate / (2 * Div) <= Clk)
        break;
    }
  }
  ASSERT (Div <= 0xFF);

  Base = BmParams.RegBase;

  MmioWrite16 (Base + SDHCI_CLK_CTRL,
          MmioRead16 (Base + SDHCI_CLK_CTRL) & ~(0x1 << 2)); // stop SD clock

  MmioWrite16 (Base + SDHCI_CLK_CTRL,
          MmioRead16 (Base + SDHCI_CLK_CTRL) & ~0x8); // disable  PLL_ENABLE

  if (MmioRead16 (Base + SDHCI_HOST_CONTROL2) & (1 << 15)) {
    MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
            MmioRead16 (Base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL
  } else {
    MmioWrite16 (Base + SDHCI_CLK_CTRL,
            (MmioRead16 (Base + SDHCI_CLK_CTRL) & 0xDF) | Div << 8); // set Clk Div
    MmioWrite16 (Base + SDHCI_CLK_CTRL,
            MmioRead16 (Base + SDHCI_CLK_CTRL) & ~(0x1 << 5)); // CLK_GEN_SELECT
  }

  MmioWrite16 (Base + SDHCI_CLK_CTRL,
          MmioRead16 (Base + SDHCI_CLK_CTRL) | 0xc); // enable  PLL_ENABLE

  for (I = 0; I <= 150000; I += 100) {
    if (MmioRead16 (Base + SDHCI_CLK_CTRL) & 0x2)
      return;
    gBS->Stall (100);
  }

  DEBUG ((DEBUG_INFO, "%a: SD PLL setting FAILED!\n", __func__));
}

/**
  Detect the status of the SD card.

  @return The status of the SD card:
          - SDCARD_STATUS_INSERTED:      The SD card is inserted.
          - SDCARD_STATUS_NOT_INSERTED:  The SD card is not inserted.
          - SDCARD_STATUS_UNKNOWN:       The status of the SD card is unknown.

**/
INT32
BmSdCardDetect (
  VOID
  )
{
  UINTN  Base;
  UINTN  Reg;

  Base = BmParams.RegBase;

  if (BmParams.CardIn != SDCARD_STATUS_UNKNOWN)
    return BmParams.CardIn;

  Reg = MmioRead32 (Base + SDHCI_PSTATE);

  if (Reg & SDHCI_CARD_INSERTED)
    BmParams.CardIn = SDCARD_STATUS_INSERTED;
  else
    BmParams.CardIn = SDCARD_STATUS_NOT_INSERTED;

  return BmParams.CardIn;
}

/**
  SD card hardware initialization.

**/
STATIC
VOID
SdHwInit (
  VOID
  )
{
  UINTN  Base;

  Base                = BmParams.RegBase;
  BmParams.VendorBase = Base + (MmioRead16 (Base + P_VENDOR_SPECIFIC_AREA) & ((1 << 12) - 1));

  // deasset reset of phy
  MmioWrite32 (Base + SDHCI_P_PHY_CNFG, MmioRead32 (Base + SDHCI_P_PHY_CNFG) | (1 << PHY_CNFG_PHY_RSTN));

  // reset data & Cmd
  MmioWrite8 (Base + SDHCI_SOFTWARE_RESET, 0x6);

  // init common parameters
  MmioWrite8 (Base + SDHCI_PWR_CONTROL, (0x7 << 1));


  MmioWrite8 (Base + SDHCI_TOUT_CTRL, 0xe);  // for TMCLK 50Khz
  MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
          MmioRead16 (Base + SDHCI_HOST_CONTROL2) | 1 << 11);  // set cmd23 support
  MmioWrite16 (Base + SDHCI_CLK_CTRL, MmioRead16 (Base + SDHCI_CLK_CTRL) & ~(0x1 << 5));  // divided clock Mode

  // set host version 4 parameters
  MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
          MmioRead16 (Base + SDHCI_HOST_CONTROL2) | (1 << 12)); // set HOST_VER4_ENABLE
  if (MmioRead32 (Base + SDHCI_CAPABILITIES1) & (0x1 << 27)) {
    MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
            MmioRead16 (Base + SDHCI_HOST_CONTROL2) | 0x1 << 13); // set 64bit addressing
  }

  // if support asynchronous int
  if (MmioRead32 (Base + SDHCI_CAPABILITIES1) & (0x1 << 29))
    MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
            MmioRead16 (Base + SDHCI_HOST_CONTROL2) | (0x1 << 14)); // enable async int
  // give some time to power down card
  gBS->Stall (20000);

  MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
          MmioRead16 (Base + SDHCI_HOST_CONTROL2) & ~(0x1 << 8)); // clr UHS2_IF_ENABLE
  MmioWrite8 (Base + SDHCI_PWR_CONTROL,
          MmioRead8 (Base + SDHCI_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
  MmioWrite16 (Base + SDHCI_HOST_CONTROL2,
          MmioRead16 (Base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL

  SdSetClk (SDCARD_INIT_FREQ);
  gBS->Stall (50000);

  // SdSetClk (BmParams.ClkRate);

  MmioWrite16 (Base + SDHCI_CLK_CTRL,
          MmioRead16 (Base + SDHCI_CLK_CTRL) | (0x1 << 2)); // supply SD clock
  gBS->Stall (400); // wait for voltage ramp up time at least 74 cycle, 400us is 80 cycles for 200Khz

  MmioWrite16 (Base + SDHCI_INT_STATUS, MmioRead16 (Base + SDHCI_INT_STATUS) | (0x1 << 6));

  // we enable all interrupt Status here for testing
  MmioWrite16 (Base + SDHCI_INT_STATUS_EN, MmioRead16 (Base + SDHCI_INT_STATUS_EN) | 0xFFFF);
  MmioWrite16 (Base + SDHCI_ERR_INT_STATUS_EN, MmioRead16 (Base + SDHCI_ERR_INT_STATUS_EN) | 0xFFFF);

  DEBUG ((DEBUG_WARN, "SD init done\n"));
}

/**
  Set the input/output settings for the SD card.

  @param[in] Clk     The clock frequency for the SD card.
  @param[in] Width   The bus width for data transfer.

  @retval EFI_SUCCESS             The input/output settings were set successfully.
  @retval EFI_UNSUPPORTED         The specified bus width is not supported.

**/
EFI_STATUS
BmSdSetIos (
  IN UINT32 Clk,
  IN UINT32 Width
  )
{
  DEBUG ((DEBUG_WARN, "%a[%d] width=%d (0-1, 1-4)\n", __func__, __LINE__, Width));
  switch (Width) {
    case MMC_BUS_WIDTH_1:
      MmioWrite8 (BmParams.RegBase + SDHCI_HOST_CONTROL,
              MmioRead8 (BmParams.RegBase + SDHCI_HOST_CONTROL) &
              ~SDHCI_DAT_XFER_WIDTH);
      break;
    case MMC_BUS_WIDTH_4:
      MmioWrite8 (BmParams.RegBase + SDHCI_HOST_CONTROL,
              MmioRead8 (BmParams.RegBase + SDHCI_HOST_CONTROL) |
              SDHCI_DAT_XFER_WIDTH);
      break;
    default:
      return EFI_UNSUPPORTED;
  }

  if (Clk) {
    SdChangeClk (Clk);
  }

  return EFI_SUCCESS;
}

/**
  Prepare the SD card for data transfer.
  Set the number and size of data blocks before sending IO commands to the SD card.

  @param[in]  Lba       Logical Block Address.
  @param[in]  Buf       Buffer Address.
  @param[in]  Size      Size of Data Blocks.

  @retval EFI_SUCCESS             The SD card was prepared successfully.
  @retval Other                   An error occurred during the preparation of the SD card.

**/
EFI_STATUS
BmSdPrepare (
  IN INT32 Lba,
  IN UINTN Buf,
  IN UINTN Size
  )
{
  UINTN   LoadAddr;
  UINTN   Base;
  UINT32  BlockCnt;
  UINT32  BlockSize;

  LoadAddr = Buf;

  if (Size >= MMC_BLOCK_SIZE) {
    // CMD17, 18, 24, 25
    // ASSERT (((LoadAddr & MMC_BLOCK_MASK) == 0) && ((Size % MMC_BLOCK_SIZE) == 0));
    BlockSize = MMC_BLOCK_SIZE;
    BlockCnt  = Size / MMC_BLOCK_SIZE;
  } else {
    // ACMD51
    ASSERT (((LoadAddr & 8) == 0) && ((Size % 8) == 0));
    BlockSize = 8;
    BlockCnt  = Size / 8;
  }

  Base = BmParams.RegBase;

  MmioWrite16 (Base + SDHCI_BLOCK_SIZE, BlockSize);
  MmioWrite16 (Base + SDHCI_BLOCK_COUNT, BlockCnt);

  return EFI_SUCCESS;
}

EFI_STATUS
BmResonse (
  IN MMC_RESPONSE_TYPE        Type,
  IN UINT32*                  Buffer
  ) {
  UINTN   Base;

  Base    = BmParams.RegBase;
  // if (Buffer == NULL) {
  //   DEBUG ((DEBUG_ERROR, "SdHost: SdReceiveResponse(): Input Buffer is NULL\n"));
  //   return EFI_INVALID_PARAMETER;
  // }

  if ((Type == MMC_RESPONSE_TYPE_R1) ||
    (Type == MMC_RESPONSE_TYPE_R1b) ||
    (Type == MMC_RESPONSE_TYPE_R3) ||
    (Type == MMC_RESPONSE_TYPE_R6) ||
    (Type == MMC_RESPONSE_TYPE_R7)) {
    Buffer[0] = MmioRead32 (Base + SDHCI_RESPONSE_01);
    DEBUG ((DEBUG_INFO, "SdHost: SdReceiveResponse(Type: %x), Buffer[0]: %08x\n",
      Type, Buffer[0]));

  } else if (Type == MMC_RESPONSE_TYPE_R2) {
    Buffer[0] = MmioRead32 (Base + SDHCI_RESPONSE_01);
    Buffer[1] = MmioRead32 (Base + SDHCI_RESPONSE_23);
    Buffer[2] = MmioRead32 (Base + SDHCI_RESPONSE_45);
    Buffer[3] = MmioRead32 (Base + SDHCI_RESPONSE_67);

    DEBUG ((DEBUG_INFO,
      "SdHost: SdReceiveResponse(Type: %x), Buffer[0-3]: %08x, %08x, %08x, %08x\n",
      Type, Buffer[0], Buffer[1], Buffer[2], Buffer[3]));
  }

  return EFI_SUCCESS;
}

/**
  SD card sends command to read data blocks.

  @param[in]  Lba       Logical Block Address.
  @param[in]  Buf       Buffer Address.
  @param[in]  Size      Size of Data Blocks.

  @retval  EFI_SUCCESS             The command to read data blocks was sent successfully.
  @retval  EFI_TIMEOUT             The command transmission or data transfer timed out.

**/
EFI_STATUS
BmSdRead (
  IN INT32   Lba,
  IN UINT32* Buf,
  IN UINTN   Size
  )
{
  UINT32  Timeout;
  UINTN   Base;
  UINT32  *Data;
  UINT32  BlockSize;
  UINT32  BlockCnt;
  UINT32  Status;

  Timeout   = 0;
  Base      = BmParams.RegBase;
  Data      = Buf;
  BlockSize = 0;
  BlockCnt  = 0;
  Status    = 0;

    BlockSize = MmioRead16 (Base + SDHCI_BLOCK_SIZE);
    DEBUG ((DEBUG_WARN, "%a[%d] BlockSize=0x%lx\n", __func__, __LINE__, BlockSize));
    BlockCnt  = Size / BlockSize;
    BlockSize /= 32;
    DEBUG ((DEBUG_WARN, "%a[%d] BlockSize=0x%lx\tBlockCnt=0x%x\n", __func__, __LINE__, BlockSize, BlockCnt));
    for (INT32 I = 0; I < BlockCnt; ) {
      // Status = MmioRead16 (Base + SDHCI_INT_STATUS);
      // if ((Status & SDHCI_INT_BUF_RD_READY) &&
      //     (MmioRead32 (Base + SDHCI_PSTATE) & SDHCI_BUF_RD_ENABLE)) {
      //   MmioWrite16 (Base + SDHCI_INT_STATUS, SDHCI_INT_BUF_RD_READY);
        for (INT32 j = 0; j < BlockSize; j++) {
          *(Data++) = MmioRead32 (Base + SDHCI_BUF_DATA_R);
          DEBUG ((DEBUG_WARN, "%a[%d] Data[%d]=0x%lx\n", __func__, __LINE__, j, MmioRead32 (Base + SDHCI_BUF_DATA_R)));
        }


        // Timeout = 0;
        I++;
      // } else {
      //   gBS->Stall (1);
      //   Timeout++;
      // }

      // if (Timeout >= 10000) {
      //   DEBUG ((DEBUG_INFO, "%a[%d]: sdhci read data Timeout\n", __func__, __LINE__));
      //   goto Timeout;
      // }
    }

    // Timeout = 0;
    // while (1) {
    //   Status = MmioRead16 (Base + SDHCI_INT_STATUS);
    //   if (Status & SDHCI_INT_XFER_COMPLETE) {
    //     MmioWrite16 (Base + SDHCI_INT_STATUS,
    //             Status | SDHCI_INT_XFER_COMPLETE);

    //     return EFI_SUCCESS;
    //   } else {
    //     gBS->Stall (1);
    //     Timeout++;
    //   }

    //   if (Timeout >= 10000) {
    //     DEBUG ((DEBUG_INFO, "%a:wait xfer complete Timeout\n", __func__));
    //     goto Timeout;
    //   }
    return EFI_SUCCESS;

// Timeout:
//   return EFI_TIMEOUT;

}

/**
  SD card sends commands to write data blocks.

  @param[in]  Lba       Logical Block Address.
  @param[in]  Buf       Buffer Address.
  @param[in]  Size      Size of Data Blocks.

  @retval  EFI_SUCCESS             The command to write data blocks was sent successfully.
  @retval  EFI_TIMEOUT             The command transmission or data transfer timed out.

**/
EFI_STATUS
BmSdWrite (
  IN INT32   Lba,
  IN UINT32* Buf,
  IN UINTN   Size
  )
{
  UINT32  Timeout;
  UINTN   Base;
  UINT32  *Data;
  UINT32  BlockSize;
  UINT32  BlockCnt;
  UINT32  Status;

  Timeout   = 0;
  Base      = BmParams.RegBase;
  Data      = Buf;
  BlockSize = 0;
  BlockCnt  = 0;
  Status    = 0;

  if (BmParams.Flags & SD_USE_PIO) {
    BlockSize = MmioRead16 (Base + SDHCI_BLOCK_SIZE);
    BlockCnt = Size / BlockSize;
    BlockSize /= 4;

    for (INT32 j = 0; j < BlockSize; j++) {
      MmioWrite32 (Base + SDHCI_BUF_DATA_R, *(Data++));
    }

    for (INT32 I = 0; I < BlockCnt-1; ) {
      Status = MmioRead16 (Base + SDHCI_INT_STATUS);
      if ((Status & SDHCI_INT_BUF_WR_READY) &&
          (MmioRead32 (Base + SDHCI_PSTATE) &
          SDHCI_BUF_WR_ENABLE)) {
        MmioWrite16 (Base + SDHCI_INT_STATUS, SDHCI_INT_BUF_WR_READY);
        for (INT32 j = 0; j < BlockSize; j++) {
          MmioWrite32 (Base + SDHCI_BUF_DATA_R, *(Data++));
        }

        Timeout = 0;
        I++;
      } else {
        gBS->Stall (1);
        Timeout++;
      }

      if (Timeout >= 10000000) {
        DEBUG ((DEBUG_INFO, "%a:sdhci write data Timeout\n", __func__));
        goto Timeout;
      }
    }

    Timeout = 0;
    while (1) {
      Status = MmioRead16 (Base + SDHCI_INT_STATUS);
      if (Status & SDHCI_INT_XFER_COMPLETE) {
        MmioWrite16 (Base + SDHCI_INT_STATUS,
                Status | SDHCI_INT_XFER_COMPLETE);

        return EFI_SUCCESS;
      } else {
        gBS->Stall (1);
        Timeout++;
      }

      if (Timeout >= 10000) {
        DEBUG ((DEBUG_INFO, "%a:wait xfer complete Timeout\n", __func__));
        goto Timeout;
      }
    }
  } else
    return EFI_SUCCESS;

Timeout:
  return EFI_TIMEOUT;
}

/**
  Initialize the SD PHY.

  This function performs the initialization of the SD PHY hardware.

**/
VOID
SdPhyInit (
  VOID
  )
{
  UINTN Base;
  INT32 RetryCount;

  Base       = BmParams.RegBase;
  RetryCount = 100;

  //
  // reset hardware
  //
  MmioWrite8 (Base + SDHCI_SOFTWARE_RESET, 0x7);
  while (MmioRead8 (Base + SDHCI_SOFTWARE_RESET)) {
    if (RetryCount-- > 0)
      gBS->Stall (10000);
    else
      break;
  }

  //
  // Wait for the PHY power on ready
  //
  RetryCount = 100;
  while (!(MmioRead32 (Base + SDHCI_P_PHY_CNFG) & (1 << PHY_CNFG_PHY_PWRGOOD))) {
    if (RetryCount-- > 0)
      gBS->Stall (10000);
    else
      break;
  }

  // Asset reset of phy
  MmioAnd32 (Base + SDHCI_P_PHY_CNFG, ~(1 << PHY_CNFG_PHY_RSTN));

  // Set PAD_SN PAD_SP
  MmioWrite32 (Base + SDHCI_P_PHY_CNFG,
          (1 << PHY_CNFG_PHY_PWRGOOD) | (0x9 << PHY_CNFG_PAD_SP) | (0x8 << PHY_CNFG_PAD_SN));

  // Set CMDPAD
  MmioWrite16 (Base + SDHCI_P_CMDPAD_CNFG,
          (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
          (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

  // Set DATAPAD
  MmioWrite16 (Base + SDHCI_P_DATPAD_CNFG,
          (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
          (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

  // Set CLKPAD
  MmioWrite16 (Base + SDHCI_P_CLKPAD_CNFG,
          (0x2 << PAD_CNFG_RXSEL) | (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

  // Set STB_PAD
  MmioWrite16 (Base + SDHCI_P_STBPAD_CNFG,
          (0x2 << PAD_CNFG_RXSEL) | (0x2 << PAD_CNFG_WEAKPULL_EN) |
          (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

  // Set RSTPAD
  MmioWrite16 (Base + SDHCI_P_RSTNPAD_CNFG,
          (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
          (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

  // Set SDCLKDL_CNFG, EXTDLY_EN = 1, fix delay
  MmioWrite8 (Base + SDHCI_P_SDCLKDL_CNFG, (1 << SDCLKDL_CNFG_EXTDLY_EN));

  // Set SMPLDL_CNFG, Bypass
  MmioWrite8 (Base + SDHCI_P_SMPLDL_CNFG, (1 << SMPLDL_CNFG_BYPASS_EN));

  // Set ATDL_CNFG, tuning Clk not use for init
  MmioWrite8 (Base + SDHCI_P_ATDL_CNFG, (2 << ATDL_CNFG_INPSEL_CNFG));

  return;
}

/**
  Initialize the SD card.

  This function performs the initialization of the SD card hardware and settings.

  @param[in] Flags     Initialization flags.

  @retval EFI_SUCCESS  The SD card was initialized successfully.

**/
EFI_STATUS
SdInit (
  IN UINT32 Flags
)
{
  BmParams.ClkRate = BmGetSdClk ();

  DEBUG ((DEBUG_INFO, "SD initializing %dHz\n", BmParams.ClkRate));

  BmParams.Flags = Flags;

  SdPhyInit ();

  SdHwInit ();

  return EFI_SUCCESS;
}
