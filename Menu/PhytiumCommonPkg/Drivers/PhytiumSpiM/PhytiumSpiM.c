/** @file
  Phytium Spi Drivers.

  This driver implement Spi interface function.

  Copyright (C) 2016, Marvell International Ltd. All rights reserved.<BR>
  Copyright (C) 2023, Phytium Technology Co Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PhytiumSpiM.h"

/**
  To send or obtain data from a slave device.

  Enable or disable the chip selection based an the flag.After configuring the control register and
  baud rate,begin the poll mode transfer.

  @param[in] This           A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.
  @param[in] Slave          A pointer to the SPI Slave device.
  @param[in] DataByteCount  The number of bytes of the data.
  @param[in] DataOut        The data is written into slave devices.
  @param[out] DataIn        The data is read from slave devices.
  @param[in] Flag           A flag indicating the start at or end of a transmission.

  @retval EFI_SUCCESS       Transfer successfully.
  @retval EFI_TIMEOUT       Transfer failure,timeout.

**/
EFI_STATUS
EFIAPI
SpiTransfer (
  IN  PHYTIUM_SPI_MASTER_PROTOCOL  *This,
  IN  SPI_DEVICE                   *Slave,
  IN  UINTN                        DataByteCount,
  IN  VOID                         *DataOut,
  OUT VOID                         *DataIn,
  IN  UINTN                        Flag
  )
{
  SPI_MASTER  *SpiMaster;
  EFI_STATUS   Status;
  UINT8       *DataOutPtr;
  UINT8       *DataInPtr;

  DataOutPtr = NULL;
  DataInPtr  = NULL;

  DataOutPtr = (UINT8 *)DataOut;
  DataInPtr  = (UINT8 *)DataIn;

  SpiMaster = PHYTIUM_SPI_MASTER_FROM_SPI_MASTER_PROTOCOL (This);

  if (DataOutPtr && DataInPtr) {
    SpiMaster->Transfermode = CTRLR0_TMOD_TR;
  } else if (DataInPtr) {
      SpiMaster->Transfermode = CTRLR0_TMOD_RO;
  } else {
      //
      //In transmit only mode (CTRL0_TMOD_TO) input FIFO never gets
      //any data which breaks our logic in poll_transfer() above.
      //
      SpiMaster->Transfermode = CTRLR0_TMOD_TR;
  }

  SpiMaster->Length = DataByteCount;
  SpiMaster->Tx = (VOID *)DataOutPtr;
  SpiMaster->TxEnd = SpiMaster->Tx + SpiMaster->Length;
  SpiMaster->Rx = DataInPtr;
  SpiMaster->RxEnd = SpiMaster->Rx + SpiMaster->Length;

  Status = SpiBaseTransfer (SpiMaster, Slave, Flag);

  if (EFI_ERROR(Status)){
    return Status;
  }

  return EFI_SUCCESS;
}


/**
  Complete a read or write operation.

  @param[in] This       A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.
  @param[in] Slave      A pointer to the SPI Slave device.
  @param[in] Cmd        The command send to the slave device.
  @param[in] CmdSize    The number of bytes of the command.
  @param[in] DataOut    The data is written into slave devices.
  @param[out] DataIn    The data is read from slave devices.
  @param[in] DataSize   The number of bytes of the data.

  @retval EFI_SUCCESS    Transfer successfully.
  @retval EFI_TIMEOUT    Transfer failure,timeout.

**/
EFI_STATUS
EFIAPI
SpiReadWrite (
  IN  PHYTIUM_SPI_MASTER_PROTOCOL *This,
  IN  SPI_DEVICE *Slave,
  IN  UINT8 *Cmd,
  IN  UINTN CmdSize,
  IN  UINT8 *DataOut,
  OUT UINT8 *DataIn,
  IN  UINTN DataSize
  )
{
  EFI_STATUS Status;

  Status = SpiTransfer (This, Slave, CmdSize, Cmd, NULL, SPI_TRANSFER_BEGIN);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "Spi Transfer Cmd Error\n"));
    return EFI_DEVICE_ERROR;
  }

  Status = SpiTransfer (This, Slave, DataSize, DataOut, DataIn, SPI_TRANSFER_END);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "Spi Transfer Data Error\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Free memory allocated for the slave devices.

  @param[in] Slave    A pointer to the SPI Slave device.

  @retval EFI_SUCCES  Free the memory successfully.
  @retval other       Free the memory fail.

**/
EFI_STATUS
EFIAPI
SpiFreeSlave (
  IN SPI_DEVICE *Slave
  )
{
  FreePool (Slave);

  return EFI_SUCCESS;
}

/**
  Build and initialize the spi slave device.

  @param[in] This       A pointer to the PHYTIUM_SPI_MASTER_PROTOCOL instance.
  @param[in] Slave      A pointer to the SPI Slave device.
  @param[in] XferSpeed  The speed at which the SPI Slave device operate.
  @param[in] TMode      The transfer mode need for slave device which is set in controller.
  @param[in] Cs         Chip selecet for slave devices.

  @retval Slave         Spi decives instance.

**/
SPI_DEVICE *
EFIAPI
SpiSetupSlave (
  IN PHYTIUM_SPI_MASTER_PROTOCOL *This,
  IN SPI_DEVICE *Slave,
  IN UINT32      XferSpeed,
  IN SPI_MODE    Mode,
  IN UINT8       Cs
  )
{
  SPI_MASTER *SpiMaster;

  SpiMaster = PHYTIUM_SPI_MASTER_FROM_SPI_MASTER_PROTOCOL (This);

  if (Slave == NULL) {
    Slave = AllocateZeroPool (sizeof(SPI_DEVICE));
    if (Slave == NULL) {
      DEBUG((DEBUG_ERROR, "Cannot allocate memory\n"));
      return NULL;
    }

    Slave->Cs   = Cs;
    Slave->Mode = Mode;
    Slave->XferSpeed = XferSpeed;
    Slave->HostRegisterBaseAddress = SpiMaster->ControllerAddress;
  }

  return Slave;
}

/**
  Initialize ths spi controller.

  Initialize some basic controller parameters,such as baud rate,controller number,
  data bit number.The devive path is create and bound to PHYTIUM_SPI_MASTER_PROTOCOL
  for the controller.

  @param[in] Bus            Bus index of controller.
  @param[in] BaseAddress    Spi controller address.

  @retval EFI_SUCCESS              Initialize the controller successfully.
  @retval EFI_OUT_OF_RESOURCES     The request could not be completed due to a lack of resources.
  @retval Others                   Initialize the controller failure.

**/
EFI_STATUS
EFIAPI
SpiControllerInit (
  IN UINTN                 Bus,
  IN EFI_PHYSICAL_ADDRESS  BaseAddress
  )
{
  EFI_STATUS  Status;
  SPI_MASTER  *SpiMaster;

  SpiMaster = AllocateZeroPool (sizeof (SPI_MASTER));
  if (SpiMaster == NULL) {
    DEBUG((DEBUG_ERROR, "PhytiumSpiDxe: Spi master context allocation failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  SpiMaster->Signature = PHYTIUM_SPI_MASTER_SIGNATURE;
  SpiMaster->ControllerAddress = BaseAddress;
  SpiMaster->ControllerNumber  = Bus;
  SpiMaster->BitsPerData       = 8;//8 bits 1 byte
  SpiMaster->CoreClock       = PcdGet32(PcdSpiControllerClockFrequency);
  SpiMaster->MaxFreq         = PcdGet32(PcdSpiControllerMaxFrequency);
  SpiMaster->SpiMasterProtocol.SetupDevice = SpiSetupSlave;
  SpiMaster->SpiMasterProtocol.FreeDevice  = SpiFreeSlave;
  SpiMaster->SpiMasterProtocol.ReadWrite   = SpiReadWrite;
  SpiMaster->SpiMasterProtocol.Transfer    = SpiTransfer;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &SpiMaster->Controller,
                  &gPhytiumSpiMasterProtocolGuid,
                  &SpiMaster->SpiMasterProtocol,
                  NULL
                  );

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "PhytiumSpiDxe: Installing protocol interfaces failed!\n"));
    goto fail;
  }

  DEBUG((DEBUG_INFO, "Succesfully installed controller %d at 0x%llx\n", Bus,SpiMaster));

  return Status;

fail:
  FreePool(SpiMaster);
  return Status;
}


/**
  This function is the entrypoint of the spi driver.Get the controller address and init.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
SpiDxeEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  SPI_BUS_INFO  *SpiBusInfo;
  EFI_STATUS    Status;
  UINT64        SpiControllerAddress;
  UINTN         ControllerCount;
  UINTN         Index;

  SpiBusInfo   = PcdGetPtr (PcdSpiControllerInformation);
  ControllerCount = SpiBusInfo->SpiControllerNumber;

  /* Initialize enabled chips */
  for (Index = 0; Index < ControllerCount; Index++) {
    SpiControllerAddress = SpiBusInfo->SpiControllerAddress[Index];
    Status = SpiControllerInit(
             Index,
             SpiControllerAddress
             );
  if (EFI_ERROR(Status))
    return Status;
  }

  return EFI_SUCCESS;
}
