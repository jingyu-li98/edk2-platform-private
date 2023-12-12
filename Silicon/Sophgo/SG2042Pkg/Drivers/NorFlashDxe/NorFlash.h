/** @file
 *
 *  The definitions of SPI NOR Flash commands and registers are from Linux Kernel.
 *
 *  Copyright (c) 2023, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#define SPI_NOR_MAX_ID_LEN      6
//
// Flash opcodes.
//
#define SPINOR_OP_WRDI          0x04    /* Write disable */
#define SPINOR_OP_WREN          0x06    /* Write enable */
#define SPINOR_OP_RDSR          0x05    /* Read status register */
#define SPINOR_OP_WRSR          0x01    /* Write status register 1 byte */
#define SPINOR_OP_READ          0x03    /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST     0x0b    /* Read data bytes (high frequency) */

#define SPINOR_OP_PP            0x02    /* Page program (up to 256 bytes) */
#define SPINOR_OP_SE            0xd8    /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID          0x9f    /* Read JEDEC ID */
#define SPINOR_OP_RDCR          0x35    /* Read configuration register */

//
// 4-byte address opcodes.
//
#define SPINOR_OP_READ_4B       0x13    /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B  0x0c    /* Read data bytes (high frequency) */
#define SPINOR_OP_PP_4B         0x12    /* Page program (up to 256 bytes) */
#define SPINOR_OP_SE_4B         0xdc    /* Sector erase (usually 64KiB) */
#define SPINOR_OP_EN4B          0xb7    /* Enter 4-byte mode */
#define SPINOR_OP_EX4B          0xe9    /* Exit 4-byte mode */

//
// Status Register bits.
//
#define SR_WIP                  BIT0  /* Write in progress */
#define SR_WEL                  BIT1  /* Write enable latch */

#define NOR_FLASH_SIGNATURE             SIGNATURE_32 ('F', 'S', 'P', 'I')
///
/// Global ID for the SPI NOR Flash Protocol
///
#define SOPHGO_NOR_FLASH_PROTOCOL_GUID  \
  { 0xE9A39038, 0x1965, 0x4404,          \
    { 0xA5, 0x2A, 0xB9, 0xA3, 0xA1, 0xAE, 0xC2, 0xE4 } }


EFI_STATUS
EFIAPI
SpiNorGetFlashId (
  IN SPI_NOR     *Nor,
  IN BOOLEAN     UseInRuntime
  );

EFI_STATUS
EFIAPI
SpiNorReadData (
  IN  SPI_NOR                        *Nor,
  IN  UINT32                         FlashOffset,
  IN  UINT32                         Length,
  OUT UINT8                          *Buffer
  );

EFI_STATUS
EFIAPI
SpiNorReadStatus (
  IN  SPI_NOR     *Nor,
  OUT UINT8       *Sr
  );

/*
 * Write the Status Register.
*/
EFI_STATUS
EFIAPI
SpiNorWriteStatus (
  IN SPI_NOR     *Nor,
  IN UINT8       *Sr,
  IN UINT32       Length
  );

EFI_STATUS
EFIAPI
SpiNorWriteData (
  IN SPI_NOR     *Nor,
  IN UINT32      FlashOffset,
  IN UINT32       Length,
  IN UINT8 *Buffer
  );

EFI_STATUS
EFIAPI
SpiNorErase (
  IN SPI_NOR                   *Nor,
  IN UINT32                    Length,
  IN UINT32                    Addr
  );