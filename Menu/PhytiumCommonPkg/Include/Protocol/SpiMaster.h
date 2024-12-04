/** @file
  SPI operation interfaces.

  Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef SPI_H_
#define SPI_H_

extern EFI_GUID gPhytiumSpiMasterProtocolGuid;

typedef struct _PHYTIUM_SPI_MASTER_PROTOCOL PHYTIUM_SPI_MASTER_PROTOCOL;

#define PHYTIUM_SPI_MASTER_SIGNATURE            SIGNATURE_32 ('P', 'S', 'P', 'I')

#define CTRLR0_TMOD_TR          0x0     /* xmit & recv */
#define CTRLR0_TMOD_TO          0x1     /* xmit only */
#define CTRLR0_TMOD_RO          0x2     /* recv only */
#define CTRLR0_TMOD_EPROMREAD   0x3 /* eeprom read mode */

typedef enum {
  SpiMode0, // CPOL = 0 & CPHA = 0
  SpiMode1, // CPOL = 0 & CPHA = 1
  SpiMode2, // CPOL = 1 & CPHA = 0
  SpiMode3  // CPOL = 1 & CPHA = 1
} SPI_MODE;


typedef struct {
  UINT64    HostRegisterBaseAddress;
  UINT32    AddrSize;
  UINT32    XferSpeed;
  SPI_MODE  Mode;
  UINT8     Cs;
} SPI_DEVICE;

typedef
EFI_STATUS
(EFIAPI *PHYTIUM_SPI_TRANSFER) (
  IN PHYTIUM_SPI_MASTER_PROTOCOL  *This,
  IN SPI_DEVICE                   *Slave,
  IN UINTN                        DataByteCount,
  IN VOID                         *DataOut,
  IN VOID                         *DataIn,
  IN UINTN                        Flag
  );

typedef
EFI_STATUS
(EFIAPI * PHYTIUM_SPI_READ_WRITE) (
  IN  PHYTIUM_SPI_MASTER_PROTOCOL  *This,
  IN  SPI_DEVICE                   *Slave,
  IN  UINT8                        *Cmd,
  IN  UINTN                        CmdSize,
  IN  UINT8                        *DataOut,
  OUT UINT8                        *DataIn,
  IN  UINTN                        DataSize
  );

typedef
SPI_DEVICE *
(EFIAPI *PHYTIUM_SPI_SETUP_DEVICE) (
  IN PHYTIUM_SPI_MASTER_PROTOCOL  *This,
  IN SPI_DEVICE                   *Slave,
  IN UINT32                       XferSpeed,
  IN SPI_MODE                     Mode,
  IN UINT8                        Cs
  );

typedef
EFI_STATUS
(EFIAPI *PHYTIUM_SPI_FREE_DEVICE) (
  IN SPI_DEVICE  *Slave
  );


struct _PHYTIUM_SPI_MASTER_PROTOCOL {
  PHYTIUM_SPI_READ_WRITE    ReadWrite;
  PHYTIUM_SPI_TRANSFER      Transfer;
  PHYTIUM_SPI_SETUP_DEVICE  SetupDevice;
  PHYTIUM_SPI_FREE_DEVICE   FreeDevice;
};

typedef
struct SPI_MASTER{
  UINT32      Signature;
  EFI_HANDLE  Controller;
  UINTN       ControllerAddress;
  UINT8       ControllerNumber;
  UINTN       CoreClock;
  INTN        MaxFreq;
  UINTN       CurrentClock;
  UINT8       Transfermode;

  UINT32      FifoLen;/* depth of the FIFO buffer */
  UINT32      MaxXfer;/* Maximum transfer size (in bits) */
  INT32       BitsPerData;
  INT32       Length;
  CONST VOID  *Tx;
  CONST VOID  *TxEnd;
  VOID        *Rx;
  VOID        *RxEnd;
  PHYTIUM_SPI_MASTER_PROTOCOL SpiMasterProtocol;
} SPI_MASTER;

#define SPI_TRANSFER_BEGIN              0x01  // Assert CS before transfer
#define SPI_TRANSFER_END                0x02  // Deassert CS after transfers


#define PHYTIUM_SPI_MASTER_FROM_SPI_MASTER_PROTOCOL(a)  CR (a, SPI_MASTER, SpiMasterProtocol, PHYTIUM_SPI_MASTER_SIGNATURE)


#endif // __PHYTIUM_SPI_MASTER_PROTOCOL_H__
