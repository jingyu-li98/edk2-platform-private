/** @file
 * 
 *  SPI registers.
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __SPI_DW_H__
#define __SPI_DW_H__

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Include/Spi.h>

#define GENMASK(end, start)  (((1ULL << ((end) - (start) + 1)) - 1) << (start))

/* Synopsys DW SSI IP-core virtual IDs */
#define DW_PSSI_ID			0
#define DW_HSSI_ID			1

/* Synopsys DW SSI component versions (FourCC sequence) */
#define DW_HSSI_102A			0x3130322a

/* DW SPI controller capabilities */
#define DW_SPI_CAP_CS_OVERRIDE		BIT0
#define DW_SPI_CAP_DFS32		BIT1

/* Register offsets (Generic for both DWC APB SSI and DWC SSI IP-cores) */
#define DW_SPI_CTRLR0			0x00
#define DW_SPI_CTRLR1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFTLR			0x18
#define DW_SPI_RXFTLR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60
#define DW_SPI_RX_SAMPLE_DLY		0xf0
#define DW_SPI_CS_OVERRIDE		0xf4

/* Bit fields in CTRLR0 (DWC APB SSI) */
#define DW_PSSI_CTRLR0_DFS_MASK			GENMASK(3, 0)
#define DW_PSSI_CTRLR0_DFS32_MASK		GENMASK(20, 16)

#define DW_PSSI_CTRLR0_FRF_MASK			GENMASK(5, 4)
#define DW_SPI_CTRLR0_FRF_MOTO_SPI		0x0
#define DW_SPI_CTRLR0_FRF_TI_SSP		0x1
#define DW_SPI_CTRLR0_FRF_NS_MICROWIRE		0x2
#define DW_SPI_CTRLR0_FRF_RESV			0x3

#define DW_PSSI_CTRLR0_MODE_MASK		GENMASK(7, 6)
#define DW_PSSI_CTRLR0_SCPHA			BIT6
#define DW_PSSI_CTRLR0_SCPOL			BIT7

#define DW_PSSI_CTRLR0_TMOD_MASK		GENMASK(9, 8)
#define DW_SPI_CTRLR0_TMOD_TR			0x0	/* xmit & recv */
#define DW_SPI_CTRLR0_TMOD_TO			0x1	/* xmit only */
#define DW_SPI_CTRLR0_TMOD_RO			0x2	/* recv only */
#define DW_SPI_CTRLR0_TMOD_EPROMREAD		0x3	/* eeprom read mode */

#define DW_PSSI_CTRLR0_SLV_OE			BIT10
#define DW_PSSI_CTRLR0_SRL			BIT11
#define DW_PSSI_CTRLR0_CFS			BIT12

/* Bit fields in CTRLR1 */
#define DW_SPI_NDF_MASK				GENMASK(15, 0)

/* Bit fields in SR, 7 bits */
#define DW_SPI_SR_MASK				GENMASK(6, 0)
#define DW_SPI_SR_BUSY				BIT0
#define DW_SPI_SR_TF_NOT_FULL			BIT1
#define DW_SPI_SR_TF_EMPT			BIT2
#define DW_SPI_SR_RF_NOT_EMPT			BIT3
#define DW_SPI_SR_RF_FULL			BIT4
#define DW_SPI_SR_TX_ERR			BIT5
#define DW_SPI_SR_DCOL				BIT6

/* Bit fields in ISR, IMR, RISR, 7 bits */
#define DW_SPI_INT_MASK				GENMASK(5, 0)
#define DW_SPI_INT_TXEI				BIT0
#define DW_SPI_INT_TXOI				BIT1
#define DW_SPI_INT_RXUI				BIT2
#define DW_SPI_INT_RXOI				BIT3
#define DW_SPI_INT_RXFI				BIT4
#define DW_SPI_INT_MSTI				BIT5

/* Bit fields in DMACR */
#define DW_SPI_DMACR_RDMAE			BIT0
#define DW_SPI_DMACR_TDMAE			BIT1

/* Mem/DMA operations helpers */
#define DW_SPI_WAIT_RETRIES			5
#define DW_SPI_BUF_SIZE \
	(sizeof_field(struct spi_mem_op, cmd.opcode) + \
	 sizeof_field(struct spi_mem_op, addr.val) + 256)
#define DW_SPI_GET_BYTE(_val, _idx) \
	((_val) >> (BITS_PER_BYTE * (_idx)) & 0xff)


#define SPI_DW_SIGNATURE                      SIGNATURE_32 ('M', 'S', 'P', 'I')
#define SPI_MASTER_FROM_SPI_DW_PROTOCOL(a)    CR (a, SPI_DW, SpiDwProtocol, SPI_DW_SIGNATURE)

///
/// Global ID for the SPI NOR Flash Protocol
///
#define SOPHGO_SPI_DW_PROTOCOL_GUID  \
  { 0x13A97F2D, 0x0D8C, 0x4819,
    { 0x87, 0x6D, 0x09, 0x95, 0xBF, 0x62, 0xEF, 0xB0 } }

typedef struct {
  SOPHGO_SPI_DW_PROTOCOL     SpiDwProtocol;
  UINTN                      Signature;
  EFI_HANDLE                 Handle;
  EFI_LOCK                   Lock;
} SPI_MASTER;

EFI_STATUS
EFIAPI
SpifmcReadRegister (
  IN  SPI_NOR *Nor,
  IN  UINT8   Opcode,
  IN  UINTN   Length,
  OUT UINT8   *Buffer
  );

EFI_STATUS
EFIAPI
SpifmcWriteRegister (
  IN SPI_NOR      *Nor,
  IN UINT8        Opcode,
  IN CONST UINT8 *Buffer,
  IN UINTN        Length
  );

EFI_STATUS
EFIAPI
SpifmcRead (
  IN  SPI_NOR *Nor,
  IN  UINTN   From,
  IN  UINTN   Length,
  OUT UINT8   *Buffer
  );

EFI_STATUS
EFIAPI
SpifmcWrite (
  IN SPI_NOR     *Nor,
  IN UINTN       To,
  IN UINTN       Length,
  IN CONST UINT8 *Buffer
  );

EFI_STATUS
EFIAPI
SpifmcErase (
  IN SPI_NOR *Nor,
  IN UINTN   Offs
  );

SPI_NOR *
EFIAPI
SpiMasterSetupSlave (
  IN SOPHGO_SPI_MASTER_PROTOCOL *This,
  IN SPI_NOR                    *Nor
  );

EFI_STATUS
EFIAPI
SpiMasterFreeSlave (
  IN SPI_NOR *Nor
  );

EFI_STATUS
EFIAPI
SpifmcInit (
  IN SPI_NOR *Nor
  );

EFI_STATUS
EFIAPI
SpiMasterEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

#endif //__SPI_DW_H__
