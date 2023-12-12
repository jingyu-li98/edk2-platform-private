/** @file
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Include/Spi.h>
#include <Include/SpiNorFlash.h>
#include "NorFlash.h"
#include <Library/DebugLib.h>

#include <Library/UefiBootServicesTableLib.h>


#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Include/Uefi.h>

NOR_FLASH_INSTANCE  *mNorFlashInstance;
SOPHGO_SPI_MASTER_PROTOCOL *SpiMasterProtocol;

/*
 * Set write enable latch with Write Enable command.
 */
STATIC
EFI_STATUS
SpiNorWriteEnable (
  IN SPI_NOR  *Nor
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WREN, NULL, 0);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Spi error while write enable\n", __func__));
  }

  return Status;
}

/*
 * Send Write Disable instruction to the chip.
 */
STATIC
EFI_STATUS
SpiNorWriteDisable (
  IN SPI_NOR  *Nor
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WRDI, NULL, 0);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Spi error while write disable\n", __func__));
  }

  return Status;
}

EFI_STATUS
EFIAPI
SpiNorGetFlashId (
  IN SPI_NOR     *Nor,
  IN BOOLEAN     UseInRuntime
  )
{
  EFI_STATUS Status;
  UINT8 Id[NOR_FLASH_MAX_ID_LEN];

  Status = SpiMasterProtocol->ReadRegister (Nor, SPINOR_OP_RDID, SPI_NOR_MAX_ID_LEN, Id);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ReadId: Spi error while reading id\n"));
    return Status;
  }

  Status = NorFlashGetInfo (Id, &Nor->Info, UseInRuntime);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "%a: Unrecognized JEDEC Id bytes: 0x%02x%02x%02x\n",
      __func__,
      Id[0],
      Id[1],
      Id[2]));
    return Status;
  }

  NorFlashPrintInfo (Nor->Info);

  return EFI_SUCCESS;
}

/*
 * Read data from flash memory
 */
EFI_STATUS
EFIAPI
SpiNorReadData (
  IN  SPI_NOR                        *Nor,
  IN  UINT32                         FlashOffset,
  IN  UINT32                          Length,
  OUT UINT8                          *Buffer
  )
{
  EFI_STATUS Status;

  if (Length == 0) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]:Length is Zero!\n", __func__, __LINE__));
    return EFI_INVALID_PARAMETER;
  }

  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]:Buffer is NULL!\n", __func__, __LINE__));
    return EFI_BAD_BUFFER_SIZE;
  }

  //
  // read data from flash memory
  //
  Status = SpiMasterProtocol->Read (Nor, FlashOffset, Length, Buffer);

  return EFI_SUCCESS;
}

/*
 * Read the Status Register.
 */
EFI_STATUS
EFIAPI
SpiNorReadStatus (
  IN SPI_NOR     *Nor,
  IN UINT8       *Sr
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->ReadRegister (Nor, SPINOR_OP_RDSR, 1, Sr);

  DEBUG ((DEBUG_VERBOSE, "read status low: 0x%x\n", Sr[0]));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Spi error while reading SR\n", __func__));
  }

  return Status;
}

/*
 * Read the Configuration Register using the SPINOR_OP_RDCR (35h) command.
 */
EFI_STATUS
EFIAPI
SpiNorReadConfig (
  IN SPI_NOR     *Nor,
  IN OUT UINT8   *Cr
  )
{
  EFI_STATUS Status;

  Status = SpiMasterProtocol->ReadRegister (Nor, SPINOR_OP_RDCR, 1, Cr);

  DEBUG ((DEBUG_VERBOSE, "read status high: 0x%x\n", Cr[0]));
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Spi error while reading CR\n", __func__));
  }

  return Status;
}

/*
 * Wait for a predefined amount of time for the flash to be ready,
 * or timeout occurs.
 */
EFI_STATUS
SpiNorWaitTillReady (
  IN SPI_NOR *Nor
  )
{
  UINT32 Wait;

  Wait = 0;

  while (1) {
    gBS->Stall (100000);

    //
    // Query the Status Register to see if the flash is ready for new commands.
    //
    SpiNorReadStatus (Nor, Nor->BounceBuf);

    if (!(Nor->BounceBuf[0] & SR_WIP) || Wait > 30) {
      return EFI_SUCCESS;
    }

    Wait ++;
  }

  DEBUG ((DEBUG_ERROR, "%a: flash operation timed out\n", __func__));

  return EFI_TIMEOUT;
}

/*
 * Write the Status Register.
*/
EFI_STATUS
EFIAPI
SpiNorWriteStatus (
  IN SPI_NOR     *Nor,
  IN UINT8       *Sr,
  IN UINT32      Length
  )
{
  EFI_STATUS Status;

  Status = SpiNorWriteEnable (Nor);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_WRSR, Sr, Length);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Spi error while write SR\n", __func__));
  }

  Status = SpiNorWaitTillReady (Nor);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: flash while write SR\n", __func__));
  }

  //
  // Write disable
  //
  SpiNorWriteDisable (Nor);

  return Status;
}

/*
write data to flash memory
*/
EFI_STATUS
EFIAPI
SpiNorWriteData (
  IN SPI_NOR     *Nor,
  IN UINT32      FlashOffset,
  IN UINT32       Length,
  IN UINT8 *Buffer
  )
{
  EFI_STATUS Status;
  UINTN Index;
  UINT32 Addr;
  UINTN PageOffset;
  UINTN PageRemain;

  if (Length == 0) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]:Length is Zero!\n", __func__, __LINE__));
    return EFI_INVALID_PARAMETER;
  }

  if (Buffer == NULL) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]:Buffer is NULL!\n", __func__, __LINE__));
    return EFI_BAD_BUFFER_SIZE;
  }

  for (Index = 0; Index < Length; Index += PageRemain) {
    Addr = FlashOffset + Index;

    PageOffset = IS_POW2 (Nor->Info->PageSize) ? (Addr & (Nor->Info->PageSize - 1)) : (Addr % Nor->Info->PageSize);

    //
    // the size of data remaining on the first page
    //
    PageRemain = MIN (Nor->Info->PageSize - PageOffset, Length - Index);

    Status = SpiNorWriteEnable (Nor);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = SpiMasterProtocol->Write (Nor, Addr, PageRemain, Buffer + Index);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = SpiNorWaitTillReady (Nor);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: flash while write SR\n", __func__));
      return Status;
    }

    Index += PageRemain;
  }

  return EFI_SUCCESS;
}

/*
 * Erase an address range on the nor chip. The address range may extend
 * one or more erase sectors. Return an error if there is a problem erasing.
 */
EFI_STATUS
EFIAPI
SpiNorErase (
  IN SPI_NOR                   *Nor,
  IN UINT32                    Length,
  IN UINT32                    Addr
  )
{
  EFI_STATUS Status;

  if (Length == 0) {
    DEBUG ((DEBUG_ERROR, "[%a]:[%dL]:Length is Zero!\n", __func__, __LINE__));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Write enable
  //
  SpiNorWriteEnable (Nor);

  //
  // Erase Sector
  //
  Status = SpiMasterProtocol->Erase (Nor, Addr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: flash while write SR\n", __func__));
  }

  Status = SpiNorWaitTillReady (Nor);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: flash while write SR\n", __func__));
  }

  //
  // Write disable
  //
  SpiNorWriteDisable (Nor);

  return EFI_SUCCESS;
}

/*
 * Configure the SPI memory:
 * - select op codes for (Fast) Read, Page Program and Sector Erase.
 * - set the number of dummy cycles (mode cycles + wait states).
 * - set the SPI protocols for register and memory accesses.
 * - set the number of address bytes.
 * Send all the required SPI flash commands to initialize device
 */
EFI_STATUS
EFIAPI
SpiNorInit (
  IN SOPHGO_NOR_FLASH_PROTOCOL *This,
  IN SPI_NOR                   *Nor
  )
{
  // UINT8      *Sr;
  EFI_STATUS Status;

  Nor->AddrNbytes= (Nor->Info->Flags & NOR_FLASH_4B_ADDR) ? 4 : 3;

  Status = SpiNorWriteEnable (Nor);
    if (EFI_ERROR (Status)) {
      DEBUG((DEBUG_ERROR, "SpiNor: Error while setting write_enable\n"));
      return Status;
  }

  Nor->ReadOpcode = SPINOR_OP_READ;
  Nor->ProgramOpcode = SPINOR_OP_PP;
  Nor->EraseOpcode = SPINOR_OP_SE;

  if (Nor->AddrNbytes == 4) {
    //
    // Enter 4-byte mode
    //
    Status = SpiMasterProtocol->WriteRegister (Nor, SPINOR_OP_EN4B, NULL, 0);
    if (EFI_ERROR (Status)) {
      DEBUG((DEBUG_ERROR, "SpiNor: Error while setting 4B address\n"));
      return Status;
    }

    Nor->ReadOpcode    = SPINOR_OP_READ_4B;
    Nor->ProgramOpcode = SPINOR_OP_PP_4B;
    Nor->EraseOpcode   = SPINOR_OP_SE_4B;
  }

  // //
  // // Initialize flash status register
  // //
  // Sr[0] = 0x0;
  // Status = SpiNorWriteStatus (Nor, Sr, 1);
  // if (EFI_ERROR (Status)) {
  //   DEBUG((DEBUG_ERROR, "SpiFlash: Error while writing status register\n"));
  //   return Status;
  // }

  return EFI_SUCCESS;
}

EFI_STATUS
SpiNorInitProtocol (
  IN SOPHGO_NOR_FLASH_PROTOCOL *NorFlashProtocol
  )
{
  // FlashProtocol->Init            = SpiNorInit;
  NorFlashProtocol->GetFlashid      = SpiNorGetFlashId;
  NorFlashProtocol->ReadData        = SpiNorReadData;
  NorFlashProtocol->WriteData       = SpiNorWriteData;
  NorFlashProtocol->ReadStatus      = SpiNorReadStatus;
  NorFlashProtocol->WriteStatus     = SpiNorWriteStatus;
  NorFlashProtocol->Erase           = SpiNorErase;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SpiNorEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                &gSophgoSpiMasterProtocolGuid,
                NULL,
                (VOID **)&SpiMasterProtocol
                );

  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "SpiNor: Cannot locate SPI Master protocol\n"));
    return EFI_DEVICE_ERROR;
  }

  mNorFlashInstance = AllocateRuntimeZeroPool (sizeof (NOR_FLASH_INSTANCE));

  if (mNorFlashInstance == NULL) {
    DEBUG((DEBUG_ERROR, "SpiNor: Cannot allocate memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  SpiNorInitProtocol (&mNorFlashInstance->NorFlashProtocol);

  mNorFlashInstance->Signature = NOR_FLASH_SIGNATURE;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mNorFlashInstance->Handle),
                  &gSophgoNorFlashProtocolGuid,
                  &(mNorFlashInstance->NorFlashProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "SpiNor: Cannot install SPI flash protocol\n"));
    goto ErrorInstallProto;
  }

  // //
  // // Register for the virtual address change event
  // //
  // Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL,
  //                 TPL_NOTIFY,
  //                 NorFlashVirtualNotifyEvent,
  //                 NULL,
  //                 &gEfiEventVirtualAddressChangeGuid,
  //                 &mNorFlashVirtualAddrChangeEvent);
  // if (EFI_ERROR (Status)) {
  //   DEBUG ((DEBUG_ERROR, "%a: Failed to register VA change event\n", __func__));
  //   goto ErrorCreateEvent;
  // }

  return EFI_SUCCESS;

// ErrorCreateEvent:
//   gBS->UninstallMultipleProtocolInterfaces (&mNorFlashInstance->Handle,
//     &gSophgoNorFlashProtocolGuid,
//     NULL);

ErrorInstallProto:
  FreePool (mNorFlashInstance);

  return EFI_SUCCESS;
}
