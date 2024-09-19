/** @file
  drivers/net/ethernet/stmicro/stmmac/dwmac4_lib.c
  drivers/net/ethernet/stmicro/stmmac/dwmac4_dma.c
  drivers/net/ethernet/stmicro/stmmac/dwmac4_descs.c
  drivers/net/ethernet/stmicro/stmmac/stmmac_main.c

  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/IoLib.h>
#include <Library/NetLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Include/Phy.h>
#include "DwMac4DxeUtil.h"

#define UPPER_32_BITS(n)      ((UINT32)((n) >> 32))
#define LOWER_32_BITS(n)      ((UINT32)((n) & 0xffffffff))

struct StmmacRxRouting {
  UINT32 RegMask;
  UINT32 RegShift;
};

UINT32 RxChannelsCount = 1;
UINT32 TxChannelsCount = 1;

/*
 * linux/drivers/net/ethernet/stmicro/stmmac/dwmac4_lib.c
 */
VOID
EFIAPI
StmmacSetUmacAddr (
  IN  EFI_MAC_ADDRESS   *MacAddress,
  IN  UINTN             MacBaseAddress,
  IN  UINTN             RegN
  )
{
  //
  // Note: This MAC_ADDR0 registers programming sequence cannot be swap:
  // Must program HIGH Offset first before LOW Offset
  // because synchronization is triggered when MAC Address0 Low Register are written.
  //
  MmioWrite32 (MacBaseAddress + GMAC_ADDR_HIGH(RegN),
               (UINT32)(MacAddress->Addr[4] & 0xFF) |
                      ((MacAddress->Addr[5] & 0xFF) << 8) |
		      GMAC_HI_REG_AE
                    );
  //
  // MacAddress->Addr[0,1,2] is the 3 bytes OUI
  //
  MmioWrite32 (MacBaseAddress + GMAC_ADDR_LOW(RegN),
                       (MacAddress->Addr[0] & 0xFF) |
                      ((MacAddress->Addr[1] & 0xFF) << 8) |
                      ((MacAddress->Addr[2] & 0xFF) << 16) |
                      ((MacAddress->Addr[3] & 0xFF) << 24)
                    );

  DEBUG ((
    DEBUG_INFO,
    "%a(): GMAC_ADDR_LOW(%d)  = 0x%08X \r\n",
    __func__,
    RegN,
    MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_LOW(RegN)))
    ));
  DEBUG ((
    DEBUG_INFO,
    "%a(): GMAC_ADDR_HIGH(%d)  = 0x%08X \r\n",
    __func__,
    RegN,
    MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_HIGH(RegN)))
    ));
}

VOID
EFIAPI
StmmacGetMacAddr (
  OUT  EFI_MAC_ADDRESS   *MacAddress,
  IN   UINTN             MacBaseAddress,
  IN   UINTN             RegN
  )
{
  UINT32 MacAddrHighValue;
  UINT32 MacAddrLowValue;

  //
  // Read the Mac Addr high register
  //
  MacAddrHighValue = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_HIGH(RegN))) & 0xFFFF;

  //
  // Read the Mac Addr low register
  //
  MacAddrLowValue = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_LOW(RegN)));

  SetMem (MacAddress, sizeof(*MacAddress), 0);
  MacAddress->Addr[0] = MacAddrLowValue & 0xFF;
  MacAddress->Addr[1] = (MacAddrLowValue >> 8) & 0xFF;
  MacAddress->Addr[2] = (MacAddrLowValue >> 16) & 0xFF;
  MacAddress->Addr[3] = (MacAddrLowValue >> 24) & 0xFF;
  MacAddress->Addr[4] = MacAddrHighValue & 0xFF;
  MacAddress->Addr[5] = (MacAddrHighValue >> 8) & 0xFF;

  DEBUG ((
    DEBUG_INFO,
    "%a(): MAC Address = %02X:%02X:%02X:%02X:%02X:%02X\r\n",
    __func__,
    MacAddress->Addr[0],
    MacAddress->Addr[1],
    MacAddress->Addr[2],
    MacAddress->Addr[3],
    MacAddress->Addr[4],
    MacAddress->Addr[5]
  ));
}

#if 0
VOID
DwMacDmaFlushTxFifo (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Csr6;
  Csr6 = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CONTROL));

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CONTROL), Csr6 | DMA_CONTROL_FTF);

  do {}
  while ((MmioRead32 ((UINTN)(MacBaseAddress + DMA_CONTROL)) & DMA_CONTROL_FTF));
}
#endif
VOID
DwMac4DmaAxi (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Value;
  UINT32 AxiWrOsrLmt;
  UINT32 AxiRdOsrLmt;

  AxiWrOsrLmt = 1;
  AxiRdOsrLmt = 1;
  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE));

  DEBUG ((
    DEBUG_INFO,
    "%a(): Master AXI performs %s burst length\n",
    __func__,
    (Value & DMA_SYS_BUS_FB) ? L"fixed" : L"any"
    ));

  Value &= ~DMA_AXI_WR_OSR_LMT;
  Value |= (AxiWrOsrLmt & DMA_AXI_OSR_MAX) << DMA_AXI_WR_OSR_LMT_SHIFT;

  Value &= ~DMA_AXI_RD_OSR_LMT;
  Value |= (AxiRdOsrLmt & DMA_AXI_OSR_MAX) << DMA_AXI_RD_OSR_LMT_SHIFT;

  Value |= DMA_AXI_BLEN16 | DMA_AXI_BLEN8 | DMA_AXI_BLEN4;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE), Value);
}

VOID
DwMac4SetRxTailPtr (
  IN UINTN  MacBaseAddress,
  IN UINT32 TailPtr,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_END_ADDR(Channel)), TailPtr);
}

VOID
DwMac4SetTxTailPtr (
  IN UINTN  MacBaseAddress,
  IN UINT32 TailPtr,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_END_ADDR(Channel)), TailPtr);
}

VOID
DwMac4DmaStartTx (
  IN  UINTN   MacBaseAddress,
  IN  UINT32  Channel
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)));
  Value |= DMA_CONTROL_ST;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)), Value);

  Value = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_CONFIG));
  Value |= GMAC_CONFIG_TE;

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_CONFIG), Value);
}

VOID
DwMac4DmaStopTx (
  IN  UINTN   MacBaseAddress,
  IN  UINT32  Channel
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)));
  Value &= ~DMA_CONTROL_ST;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)), Value);
}

VOID
DwMac4DmaStartRx (
  IN  UINTN   MacBaseAddress,
  IN  UINT32  Channel
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)));
  Value |= DMA_CONTROL_SR;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)), Value);

  Value = MmioRead32((UINTN)(MacBaseAddress + GMAC_CONFIG));
  Value |= GMAC_CONFIG_RE;

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_CONFIG), Value);
}

VOID
DwMac4DmaStopRx (
  IN  UINTN   MacBaseAddress,
  IN  UINT32  Channel
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)));
  Value &= ~DMA_CONTROL_SR;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)), Value);
}

/*
 * Start all RX and TX DMA channels
 */
VOID
EFIAPI
StmmacStartAllDma (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Channel;

  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    DwMac4DmaStartRx (MacBaseAddress, Channel);
  }

  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DwMac4DmaStartTx (MacBaseAddress, Channel);
  }
}

/*
 * Stop all RX and TX DMA channels
 */
VOID
EFIAPI
StmmacStopAllDma (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Channel;

  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    DwMac4DmaStopRx (MacBaseAddress, Channel);
  }

  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DwMac4DmaStopTx (MacBaseAddress, Channel);
  }
}

EFI_STATUS
EFIAPI
DwMac4DmaReset (
  IN  UINTN         MacBaseAddress
  )
{
  UINT32     Value;
  UINT32     Timeout;

  Timeout = 1000000;
  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE));

  //
  // DMA SW reset
  //
  Value |= DMA_BUS_MODE_SFT_RESET;
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE), Value);

  //
  // wait till the bus software reset
  //
  do {
    Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE));
    if (Timeout-- == 10000) {
      DEBUG ((
        DEBUG_ERROR,
        "%a(): Bus software reset timeout\n",
	__func__
        ));

      return EFI_TIMEOUT;
    }
  } while (Value & DMA_BUS_MODE_SFT_RESET);

  return EFI_SUCCESS;
}

VOID
DwMac4DmaInit (
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE));

  Value |= DMA_SYS_BUS_AAL;
  Value |= DMA_SYS_BUS_EAME;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE), Value);

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE));

  //
  // Only DWMAC core version 5.20 onwards supports HW descriptor prefetch.
  //
  if (MmioRead32((UINTN)(MacBaseAddress + GMAC4_VERSION)) >= 0x52) {
	  Value |= DMA_BUS_MODE_DCHE;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE), Value);
}

VOID
DwMac4InitChannel (
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32  Value;
  BOOLEAN Pblx8;

  Pblx8 = TRUE;

  //
  // Common channel control register config
  //
  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Channel)));

  if (Pblx8) {
    Value = Value | DMA_BUS_MODE_PBL;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Channel)), Value);

  //
  // Mask interrupts by writing to CSR7
  //
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Channel)), DMA_CHAN_INTR_DEFAULT_MASK);
}

VOID
DwMac4DmaInitRxChan (
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32           Value;
  UINT32           RxPbl;

  RxPbl = 32;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)));
  Value = Value | (RxPbl << DMA_BUS_MODE_RPBL_SHIFT);

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)), Value);
}

VOID
DwMac4DmaInitTxChan (
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32          Value;
  UINT32          TxPbl;

  TxPbl = 32;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)));
  Value = Value | (TxPbl << DMA_BUS_MODE_PBL_SHIFT);

  //
  // Enable OSP to get best performance
  //
  Value |= DMA_CONTROL_OSP;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)), Value);
}

VOID
DwMac4SetupRxDescriptor (
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress
  )
{
  UINT32           Index;
  DMA_DESCRIPTOR   *RxDescriptor;
  UINT32           Channel;

  Channel = 0;

  for (Index = 0; Index < RX_DESC_NUM; Index++) {
    RxDescriptor = (VOID *)(UINTN)StmmacDriver->RxDescRingMap[Index].AddrMap;
    RxDescriptor->DmaMacAddr = StmmacDriver->RxBufNum[Index].AddrMap;

    if (Index < RX_DESC_NUM - 1) {
      RxDescriptor->DmaMacNext = (UINTN)StmmacDriver->RxDescRingMap[Index + 1].AddrMap;
    }

    RxDescriptor->Des0 = LOWER_32_BITS(RxDescriptor->DmaMacAddr);
    RxDescriptor->Des1 = UPPER_32_BITS(RxDescriptor->DmaMacAddr);
    RxDescriptor->Des2 = 0;
    RxDescriptor->Des3 = RDES3_OWN | RDES3_BUFFER1_VALID_ADDR;
  }

  //
  // Correcting the last pointer of the chain
  //
  RxDescriptor->DmaMacNext = (UINTN)StmmacDriver->RxDescRingMap[0].AddrMap;

  //
  // Write the address of Rx descriptor list
  //
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_BASE_ADDR_HI(Channel)),
		    UPPER_32_BITS((UINTN)StmmacDriver->RxDescRingMap[0].AddrMap));

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_BASE_ADDR(Channel)),
		    LOWER_32_BITS((UINTN)StmmacDriver->RxDescRingMap[0].AddrMap));

  //
  // Initialize the descriptor number
  //
  StmmacDriver->RxCurrentDescriptorNum = 0;
  StmmacDriver->RxNextDescriptorNum = 0;
}

VOID
DwMac4SetupTxDescriptor (
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress
  )
{
  UINT32          Index;
  DMA_DESCRIPTOR  *TxDescriptor;
  UINT32          Channel;

  Channel = 0;

  for (Index = 0; Index < TX_DESC_NUM; Index++) {
    TxDescriptor = (VOID *)(UINTN)StmmacDriver->TxDescRingMap[Index].AddrMap;
    TxDescriptor->DmaMacAddr = (UINTN)&StmmacDriver->TxBuffer[Index * ETH_BUFFER_SIZE];

    if (Index < TX_DESC_NUM - 1) {
      TxDescriptor->DmaMacNext = (UINTN)StmmacDriver->TxDescRingMap[Index + 1].AddrMap;
    }

    TxDescriptor->Des0 = LOWER_32_BITS(TxDescriptor->DmaMacAddr);
    TxDescriptor->Des1 = UPPER_32_BITS(TxDescriptor->DmaMacAddr);
    //TxDescriptor->Des2 = Length & TDES2_BUFFER1_SIZE_MASK;
    TxDescriptor->Des2 = 0;
    TxDescriptor->Des3 = TDES3_OWN | TDES3_PACKET_SIZE_MASK;
  }

  //
  // Correcting the last pointer of the chain
  //
  TxDescriptor->DmaMacNext = (UINTN)StmmacDriver->TxDescRingMap[0].AddrMap;

  //
  // Write the address of tx descriptor list
  //
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_BASE_ADDR_HI(Channel)),
		    UPPER_32_BITS((UINTN)StmmacDriver->TxDescRingMap[0].AddrMap));

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_BASE_ADDR(Channel)),
		    LOWER_32_BITS((UINTN)StmmacDriver->TxDescRingMap[0].AddrMap));

  //
  // Initialize the descriptor number
  //
  StmmacDriver->TxCurrentDescriptorNum = 0;
  StmmacDriver->TxNextDescriptorNum = 0;
}

VOID
DwMac4DmaRxChanOpMode (
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel,
  IN  UINT32         FifoSize,
  IN  UINT32         Mode,
  IN  UINT8          Qmode
  )
{
  UINT32 Rqs;
  UINT32 MtlRxOp;
  UINT32 Rfd;
  UINT32 Rfa;

  Rqs = FifoSize / 256 - 1;
  MtlRxOp = MmioRead32 ((UINTN)(MacBaseAddress + MTL_CHAN_RX_OP_MODE(Channel)));

  if (Mode == SF_DMA_MODE) {
    DEBUG ((
      DEBUG_INFO,
      "%a(): enable RX store and forward mode\n",
      __func__
      ));
    MtlRxOp |= MTL_OP_MODE_RSF;
  } else {
    DEBUG ((
      DEBUG_INFO,
      "%a(): disable RX SF mode (threshold %d)\n",
      __func__,
      Mode
      ));

    MtlRxOp &= ~MTL_OP_MODE_RSF;
    MtlRxOp &= MTL_OP_MODE_RTC_MASK;

    if (Mode <= 32) {
      MtlRxOp |= MTL_OP_MODE_RTC_32;
    } else if (Mode <= 64) {
      MtlRxOp |= MTL_OP_MODE_RTC_64;
    } else if (Mode <= 96) {
      MtlRxOp |= MTL_OP_MODE_RTC_96;
    } else {
      MtlRxOp |= MTL_OP_MODE_RTC_128;
    }
  }

  MtlRxOp &= ~MTL_OP_MODE_RQS_MASK;
  MtlRxOp |= Rqs << MTL_OP_MODE_RQS_SHIFT;

  //
  // Enable flow control only if each channel gets 4 KiB or more FIFO and
  // only if channel is not an AVB channel.
  //
  if ((FifoSize >= 4096) && (Qmode != MTL_QUEUE_AVB)) {
    MtlRxOp |= MTL_OP_MODE_EHFC;

    //
    // Set Threshold for Activating Flow Control to min 2 frames,
    // i.e. 1500 * 2 = 3000 bytes.
    //
    // Set Threshold for Deactivating Flow Control to min 1 frame,
    // i.e. 1500 bytes.
    //
    switch (FifoSize) {
    case 4096:
      //
      // This violates the above formula because of FIFO size
      // limit therefore overflow may occur in spite of this.
      //
      Rfd = 0x03; /* Full-2.5K */
      Rfa = 0x01; /* Full-1.5K */
      break;

    default:
      Rfd = 0x07; /* Full-4.5K */
      Rfa = 0x04; /* Full-3K */
      break;
    }

    MtlRxOp &= ~MTL_OP_MODE_RFD_MASK;
    MtlRxOp |= Rfd << MTL_OP_MODE_RFD_SHIFT;

    MtlRxOp &= ~MTL_OP_MODE_RFA_MASK;
    MtlRxOp |= Rfa << MTL_OP_MODE_RFA_SHIFT;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + MTL_CHAN_RX_OP_MODE(Channel)), MtlRxOp);
}

VOID
DwMac4DmaTxChanOpMode (
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel,
  IN  UINT32         FifoSize,
  IN  UINT32         Mode,
  IN  UINT8          Qmode
  )
{
 UINT32 MtlTxOp;
 UINT32 Tqs;

 MtlTxOp = MmioRead32 ((UINTN)(MacBaseAddress + MTL_CHAN_TX_OP_MODE(Channel)));
 Tqs = FifoSize / 256 - 1;

 if (Mode == SF_DMA_MODE) {
    DEBUG ((
      DEBUG_INFO,
      "%a(): enable TX store and forward mode\n",
      __func__
      ));
    //
    // Transmit COE type 2 cannot be done in cut-through mode.
    //
    MtlTxOp |= MTL_OP_MODE_TSF;
  } else {
    DEBUG ((
      DEBUG_INFO,
      "%a(): disabling TX SF (threshold %d)\n",
      __func__,
      Mode
      ));
    MtlTxOp &= ~MTL_OP_MODE_TSF;
    MtlTxOp &= MTL_OP_MODE_TTC_MASK;

    //
    // Set the transmit threshold
    //
    if (Mode <= 32) {
      MtlTxOp |= MTL_OP_MODE_TTC_32;
    } else if (Mode <= 64) {
      MtlTxOp |= MTL_OP_MODE_TTC_64;
    } else if (Mode <= 96) {
      MtlTxOp |= MTL_OP_MODE_TTC_96;
    } else if (Mode <= 128) {
      MtlTxOp |= MTL_OP_MODE_TTC_128;
    } else if (Mode <= 192) {
      MtlTxOp |= MTL_OP_MODE_TTC_192;
    } else if (Mode <= 256) {
      MtlTxOp |= MTL_OP_MODE_TTC_256;
    } else if (Mode <= 384) {
      MtlTxOp |= MTL_OP_MODE_TTC_384;
    } else {
      MtlTxOp |= MTL_OP_MODE_TTC_512;
    }
  }

  //
  // For an IP with DWC_EQOS_NUM_TXQ == 1, the fields TXQEN and TQS are RO
  // with reset values: TXQEN on, TQS == DWC_EQOS_TXFIFO_SIZE.
  // For an IP with DWC_EQOS_NUM_TXQ > 1, the fields TXQEN and TQS are R/W
  // with reset values: TXQEN off, TQS 256 bytes.
  //
  // TXQEN must be written for multi-channel operation and TQS must
  // reflect the available fifo size per queue (total fifo size / number
  // of enabled queues).
  //
  MtlTxOp &= ~MTL_OP_MODE_TXQEN_MASK;
  if (Qmode != MTL_QUEUE_AVB) {
    MtlTxOp |= MTL_OP_MODE_TXQEN;
  } else {
    MtlTxOp |= MTL_OP_MODE_TXQEN_AV;
  }

  MtlTxOp &= ~MTL_OP_MODE_TQS_MASK;
  MtlTxOp |= Tqs << MTL_OP_MODE_TQS_SHIFT;

  MmioWrite32((UINTN)(MacBaseAddress + MTL_CHAN_TX_OP_MODE(Channel)), MtlTxOp);
}

VOID
DwMac4ProgMtlRxAlgorithms (
  IN UINTN   MacBaseAddress,
  IN UINT32  RxAlg
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + MTL_OPERATION_MODE));
  Value &= ~MTL_OPERATION_RAA;

  switch (RxAlg) {
  case MTL_RX_ALGORITHM_SP:
    Value |= MTL_OPERATION_RAA_SP;
    break;
  case MTL_RX_ALGORITHM_WSP:
    Value |= MTL_OPERATION_RAA_WSP;
    break;
  default:
    break;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + MTL_OPERATION_MODE), Value);
}

VOID
DwMac4ProgMtlTxAlgorithms (
  IN UINTN   MacBaseAddress,
  IN UINT32  TxAlg
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + MTL_OPERATION_MODE));
  Value &= ~MTL_OPERATION_SCHALG_MASK;

  switch (TxAlg) {
  case MTL_TX_ALGORITHM_WRR:
    Value |= MTL_OPERATION_SCHALG_WRR;
    break;
  case MTL_TX_ALGORITHM_WFQ:
    Value |= MTL_OPERATION_SCHALG_WFQ;
    break;
  case MTL_TX_ALGORITHM_DWRR:
    Value |= MTL_OPERATION_SCHALG_DWRR;
    break;
  case MTL_TX_ALGORITHM_SP:
    Value |= MTL_OPERATION_SCHALG_SP;
    break;
  default:
    break;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + MTL_OPERATION_MODE), Value);
}

VOID
DwMac4SetTxRingLen (
  IN UINTN  MacBaseAddress,
  IN UINT32 Length,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_RING_LEN(Channel)), Length);
}

VOID
DwMac4SetRxRingLen (
  IN UINTN  MacBaseAddress,
  IN UINT32 Length,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_RING_LEN(Channel)), Length);
}

VOID
EFIAPI
DwMac4CoreInit (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_CONFIG));
  Value |= GMAC_CORE_INIT;

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_CONFIG), Value);

  //
  // Enable GMAC interrupts
  //
  Value = GMAC_INT_DEFAULT_ENABLE;

  //
  // Enable FPE interrupt
  //
  if ((GMAC_HW_FEAT_FPESEL & MmioRead32 ((UINTN)(MacBaseAddress + GMAC_HW_FEATURE3))) >> 26) {
    Value |= GMAC_INT_FPE_EN;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_INT_EN), Value);
}

VOID
DwMac4EnableDmaInterrupt (
  IN UINTN   MacBaseAddress,
  IN UINTN   Channel,
  IN BOOLEAN Rx,
  IN BOOLEAN Tx
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Channel)));

  if (Rx) {
    Value |= DMA_CHAN_INTR_DEFAULT_RX;
  }

  if (Tx) {
    Value |= DMA_CHAN_INTR_DEFAULT_TX;
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Channel)), Value);
}

/*
 * DMA init.
 * Description:
 * It inits the DMA invoking the specific MAC/GMAC callback.
 * Some DMA parameters can be passed from the platform;
 * in case of these are not passed a default is kept for the MAC or GMAC.
 */
EFI_STATUS
EFIAPI
StmmacInitDmaEngine (
  IN STMMAC_DRIVER   *StmmacDriver,
  IN UINTN           MacBaseAddress
  )
{
  UINT32          DmaCsrCh;
  UINT32          Channel;
  EFI_STATUS      Status;

  DEBUG ((
    DEBUG_INFO,
    "%a(): MacBaseAddress=0x%lx\r\n",
    __func__,
    MacBaseAddress
    ));

  DmaCsrCh = MAX (RxChannelsCount, TxChannelsCount);

  //
  // Step 1. Provide a software reset. This resets all of the MAC internal
  // registers and logic (bit-0 of DMA_Mode).
  // Step 2. Wait for the completion of the reset process (poll bit 0 of the
  // DMA_Mode, which is only cleared after the reset operation is completed).
  //
  Status = DwMac4DmaReset (MacBaseAddress);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a(): Reset the dma failed!\n",
      __func__
      ));
    return Status;
  }

  //
  // Step 3. Program the following fields to initialize the DMA_SysBus_Mode
  // register:
  // (1) AAL.
  // (2) Fixed burst or undefined burst.
  // (3) Burst mode values in case of AHB bus interface, OSR_LMT in case of
  //     AXI bus interface.
  // (4) If fixed length value is enabled, select the maximum burst length
  //     possible on the AXI Bus (bits [7:1]).
  //
  DwMac4DmaInit (StmmacDriver, MacBaseAddress);

  DwMac4DmaAxi (MacBaseAddress);

  //
  // Step 4. Create a descriptor list for transmit and receive.
  // In addition, ensure that the descriptors are owned by DMA
  // (set bit 31 of descriptor TDES3/RDES3).
  //
  // DMA CSR Channel configuration
  //
  DwMac4SetupTxDescriptor (StmmacDriver, MacBaseAddress);
  DwMac4SetupRxDescriptor (StmmacDriver, MacBaseAddress);

  //
  // Step 5. Program the Transmit and Receive Ring length registers
  // (DMA_CH(#i)_TxDesc_Ring_Length (for i = 0; i <= DWC_EQOS_NUM_DMA_TX_CH-1)
  // and DMA_CH(#i)_RxDesc_Ring_Length (for i = 0; i <= DWC_EQOS_NUM_DMA_RX_CH-1)).
  // The ring length programmed must be at least 4.
  //
  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DwMac4SetTxRingLen (MacBaseAddress, TX_DESC_NUM - 1, Channel);
  }

  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    DwMac4SetRxRingLen (MacBaseAddress, RX_DESC_NUM - 1, Channel);
  }

  //
  // Step 6. Initialize receive and transmit descriptor list address with
  // the base address of the transmit and receive descriptor
  // (DMA_CH(#i)_TxDesc_List_Address (for i = 0; i <= DWC_EQOS_NUM_DMA_TX_CH-1),
  // DMA_CH(#i)_RxDesc_List_Address (for i = 0; i <= DWC_EQOS_NUM_DMA_RX_CH-1)).
  // Also, program transmit and receive tail pointer registers indicating to
  // the DMA about the available descriptors
  // (DMA_CH(#i)_TxDesc_Tail_Pointer (for i = 0; i <= DWC_EQOS_NUM_DMA_TX_CH-1)
  // and DMA_CH(#i)_RxDesc_Tail_Pointer (for i = 0; i <= DWC_EQOS_NUM_DMA_RX_CH-1)).
  //
  // Step 7. Program the settings of the following registers for the parameters
  // like maximum burst-length (PBL) initiated by DMA, descriptor skip lengths,
  // OSP in case of Tx DMA, RBSZ in case of Rx DMA, and so on.
  //
  for (Channel = -1; Channel < DmaCsrCh; Channel++) {
    DwMac4InitChannel (MacBaseAddress, Channel);
  }

  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    DwMac4DmaInitRxChan (MacBaseAddress, Channel);

    DwMac4SetRxTailPtr (MacBaseAddress, (UINTN)&StmmacDriver->RxBuffer[(RX_DESC_NUM - 1) * ETH_BUFFER_SIZE], Channel);
  }

  //
  // DMA TX Channel Configuration
  //
  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DwMac4DmaInitTxChan (MacBaseAddress, Channel);

    DwMac4SetTxTailPtr (MacBaseAddress, (UINTN)&StmmacDriver->TxBuffer[(TX_DESC_NUM - 1) * ETH_BUFFER_SIZE], Channel);
  }

  //
  // Step 8. Enable the interrupts by programming the
  // DMA_CH(#i)_Interrupt_Enable (for i = 0; i <= DWC_EQOS_NUM_DMA_TX_CH-1) register.
  //
  DwMac4CoreInit (MacBaseAddress);
  for (Channel = 0; Channel < DmaCsrCh; Channel++) {
    DwMac4EnableDmaInterrupt (MacBaseAddress, Channel, TRUE, TRUE);
  }

  //
  // Step 9. Start the Receive and Transmit DMAs by setting SR (bit 0) of the
  // DMA_CH(#i)_RX_Control (for i = 0; i <= DWC_EQOS_NUM_DMA_RX_CH-1) and
  // ST (bit 0) of the DMA_CH(#i)_TX_Control (for i = 0; i <= DWC_EQOS_NUM_DMA_TX_CH-1) register.
  //
  for (Channel = 0; Channel < DmaCsrCh; Channel++) {
    DwMac4DmaStartRx (MacBaseAddress, Channel);
  }

  for (Channel = 0; Channel < DmaCsrCh; Channel++) {
    DwMac4DmaStartTx (MacBaseAddress, Channel);
  }

  //
  // Step 10. Repeat steps 4 to 9 for all the Tx DMA and Rx DMA channels
  // selected in the hardware.
  //

  return EFI_SUCCESS;
}

VOID
DwMac4SetMtlTxQueueWeight (
  IN UINTN  MacBaseAddress,
  IN UINT32 Weight,
  IN UINT32 Queue
  )
{
  UINT32 Value;
  Value = MmioRead32 ((UINTN)(MacBaseAddress + MTL_TXQ_WEIGHT_BASE_ADDR + (Queue * MTL_TXQ_WEIGHT_BASE_OFFSET)));

  Value &= ~MTL_TXQ_WEIGHT_ISCQW_MASK;
  Value |= Weight & MTL_TXQ_WEIGHT_ISCQW_MASK;

  MmioWrite32 ((UINTN)(MacBaseAddress + MTL_TXQ_WEIGHT_BASE_ADDR + (Queue * MTL_TXQ_WEIGHT_BASE_OFFSET)), Value);
}

VOID
DwMac4MapMtlDma (
  IN UINTN  MacBaseAddress,
  IN UINT32 Queue,
  IN UINT32 Channel
  )
{
  UINT32 Value;

  if (Queue < 4) {
    Value = MmioRead32 ((UINTN)(MacBaseAddress + MTL_RXQ_DMA_MAP0));
    Value &= ~MTL_RXQ_DMA_QXMDMACH_MASK(Queue);
    Value |= MTL_RXQ_DMA_QXMDMACH(Channel, Queue);
    MmioWrite32 ((UINTN)(MacBaseAddress + MTL_RXQ_DMA_MAP0), Value);
  } else {
    Value = MmioRead32 ((UINTN)(MacBaseAddress + MTL_RXQ_DMA_MAP1));
    Value &= ~MTL_RXQ_DMA_QXMDMACH_MASK(Queue - 4);
    Value |= MTL_RXQ_DMA_QXMDMACH(Channel, Queue - 4);
    MmioWrite32 ((UINTN)(MacBaseAddress + MTL_RXQ_DMA_MAP1), Value);
  }
}

VOID
DwMac4RxQueueEnable (
  IN UINTN  MacBaseAddress,
  IN UINT8  Mode,
  IN UINT32 Queue
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_RXQ_CTRL0));

  Value &= GMAC_RX_QUEUE_CLEAR(Queue);
  if (Mode == MTL_QUEUE_AVB) {
    Value |= GMAC_RX_AV_QUEUE_ENABLE(Queue);
  } else if (Mode == MTL_QUEUE_DCB) {
    Value |= GMAC_RX_DCB_QUEUE_ENABLE(Queue);
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_RXQ_CTRL0), Value);
}

/**
 *  stmmac_mtl_configuration - Configure MTL
 *  @priv: driver private structure
 *  Description: It is used for configurring MTL
 */
VOID
StmmacMtlConfiguration (
  IN UINTN MacBaseAddress
)
{
  UINT32 HwCap;
  UINT32 TxFifoSize;
  UINT32 RxFifoSize;
  UINT32 TxQueuesToUse;
  UINT32 RxQueuesToUse;
  UINT32 TxAlgorithm;
  UINT32 RxAlgorithm;
  UINT32 Queue;
  UINT32 Weight;
  UINT32 Channel;
  UINT8  Mode;

  TxQueuesToUse = 1;
  RxQueuesToUse = 1;
  TxAlgorithm = MTL_TX_ALGORITHM_SP;
  RxAlgorithm = MTL_RX_ALGORITHM_WSP;
  HwCap = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_HW_FEATURE0));

  //
  // RX and TX FIFO sizes are encoded as log2(n / 128). Undo that by
  // shifting and store the sizes in bytes.
  //
  TxFifoSize = 128 << ((HwCap & GMAC_HW_TXFIFOSIZE) >> 6);
  RxFifoSize = 128 << ((HwCap & GMAC_HW_RXFIFOSIZE) >> 0);

  //
  // Adjust for real per queue fifo size
  //
  TxFifoSize /= TxQueuesToUse;
  RxFifoSize /= RxQueuesToUse;

  if (TxQueuesToUse > 1) {
    for (Queue = 0; Queue < TxQueuesToUse; Queue++) {
      Weight = 0x10 + Queue;
      DwMac4SetMtlTxQueueWeight (MacBaseAddress, Weight, Queue);
    }
  }

  //
  // Configure MTL RX algorithms
  //
  if (RxQueuesToUse > 1) {
    DwMac4ProgMtlRxAlgorithms (MacBaseAddress, RxAlgorithm);
  }

  //
  // Configure MTL TX algorithms
  //
  if (TxQueuesToUse > 1) {
    DwMac4ProgMtlTxAlgorithms (MacBaseAddress, TxAlgorithm);
  }

  //
  // Map RX MTL to DMA channels
  //
  for (Queue = 0; Queue < RxQueuesToUse; Queue++) {
    Channel = Queue;
    DwMac4MapMtlDma (MacBaseAddress, Queue, Channel);
  }

  //
  // Enable MAC RX Queues
  //
  for (Queue = 0; Queue < RxQueuesToUse; Queue++) {
    Mode = MTL_QUEUE_DCB;
    DwMac4RxQueueEnable (MacBaseAddress, Mode, Queue);
  }

  //
  // Step 3. Program the following fields to initialize the mode of operation
  //         in the MTL_TxQ0_Operation_Mode register.
  // a. Transmit Store And Forward (TSF) or Transmit Threshold Control (TTC) in case of threshold mode
  // b. Transmit Queue Enable (TXQEN) to value 2‘b10 to enable Transmit Queue0
  // c. Transmit Queue Size (TQS)
  //
  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DwMac4DmaTxChanOpMode(MacBaseAddress, Channel, TxFifoSize, SF_DMA_MODE, MTL_QUEUE_DCB);
  }

  //
  // Step 4. Program the following fields to initialize the mode of operation
  //         in the MTL_RxQ0_Operation_Mode register:
  // a. Receive Store and Forward (RSF) or RTC in case of Threshold mode
  // b. Flow Control Activation and De-activation thresholds for MTL Receive FIFO (RFA and RFD)
  // c. Error Packet and undersized good Packet forwarding enable (FEP and FUP)
  // d. Receive Queue Size (RQS)
  //
  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    DwMac4DmaRxChanOpMode(MacBaseAddress, Channel, RxFifoSize, SF_DMA_MODE, MTL_QUEUE_DCB);
  }
}

EFI_STATUS
EFIAPI
StmmacSetFilters (
  IN  UINT32            ReceiveFilterSetting,
  IN  BOOLEAN           Reset,
  IN  UINTN             NumMfilter          OPTIONAL,
  IN  EFI_MAC_ADDRESS   *Mfilter            OPTIONAL,
  IN  UINTN             MacBaseAddress
  )
{
  UINT32  MacFilter;
  UINT32  Crc;
  UINT32  Index;
  UINT32  HashReg;
  UINT32  HashBit;
  UINT32  Register;
  UINT32  Value;

  //
  // If reset then clear the filter registers
  //
  if (Reset) {
    for (Index = 0; Index < NumMfilter; Index++) {
      MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_HASH_TAB(Index)), 0x0);
    }
  }

  //
  // Set MacFilter to the reset value of the GMAC_PACKET_FILTER register.
  //
  MacFilter = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_PACKET_FILTER));
  MacFilter &= ~GMAC_PACKET_FILTER_HMC;
  MacFilter &= ~GMAC_PACKET_FILTER_HPF;
  MacFilter &= ~GMAC_PACKET_FILTER_PCF;
  MacFilter &= ~GMAC_PACKET_FILTER_PM;
  MacFilter &= ~GMAC_PACKET_FILTER_PR;
  MacFilter &= ~GMAC_PACKET_FILTER_RA;

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    MacFilter |=  GMAC_PACKET_FILTER_HMC;

    //
    // Set the hash tables.
    //
    if ((NumMfilter > 0) && (!Reset)) {
      //
      // Go through each filter address and set appropriate bits on hash table.
      //
      for (Index = 0; Index < NumMfilter; Index++) {
        //
	// Generate a 32-bit CRC.
	//
        Crc = GenEtherCrc32 (&Mfilter[Index], NET_ETHER_ADDR_LEN);
        //
	// Reserve CRC + take upper 8 bit = take lower 8 bit and reverse it.
	//
        Value = BitReverse (Crc & 0xff);
	//
        // The most significant bits determines the register to be used (Hash Table Register X),
        // and the least significant five bits determine the bit within the register.
        // For example, a hash value of 8b'10111111 selects Bit 31 of the Hash Table Register 5.
	//
        HashReg = (Value >> 5);
        HashBit = (Value & 0x1f);

        Register = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_HASH_TAB(HashReg)));
	//
        // Set 1 to HashBit of HashReg.
        // For example, set 1 to bit 31 to Reg 5 as in above example.
	//
        Register |= (1 << HashBit);
        MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_HASH_TAB(HashReg)), Register);
      }
    }
  }

  if ((ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) == 0) {
    MacFilter |= GMAC_PACKET_FILTER_DBF;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    MacFilter |= GMAC_PACKET_FILTER_PR;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST) {
    MacFilter |= GMAC_PACKET_FILTER_PM;
  }

  //
  // Set MacFilter to GMAC_PACKET_FILTER register.
  //
  MmioWrite32((UINTN)(MacBaseAddress + GMAC_PACKET_FILTER), MacFilter);

  return EFI_SUCCESS;
}

/*
 * Create Ethernet CRC
 * INFO USED:
 * 1. http://en.wikipedia.org/wiki/Cyclic_redundancy_check
 * 2. http://en.wikipedia.org/wiki/Computation_of_CRC
 */
UINT32
EFIAPI
GenEtherCrc32 (
  IN  EFI_MAC_ADDRESS   *Mac,
  IN  UINT32            AddrLen
  )
{
  INT32   Iter;
  UINT32  Remainder;
  UINT8   *Ptr;

  Iter = 0;
  Remainder = 0xFFFFFFFF; // 0xFFFFFFFF is standard seed for Ethernet

  //
  // Convert Mac Address to array of bytes
  //
  Ptr = (UINT8 *)Mac;

  //
  // Generate the Crc bit-by-bit (LSB first).
  //
  while (AddrLen--) {
    Remainder ^= *Ptr++;
    for (Iter = 0; Iter < 8; Iter++) {
      //
      // Check if exponent is set.
      //
      if (Remainder & 1) {
        Remainder = (Remainder >> 1) ^ CRC_POLYNOMIAL;
      } else {
        Remainder = (Remainder >> 1) ^ 0;
      }
    }
  }

  return (~Remainder);
}

STATIC CONST UINT8 NibbleTab[] = {
    /* 0x0 0000 -> 0000 */  0x0,
    /* 0x1 0001 -> 1000 */  0x8,
    /* 0x2 0010 -> 0100 */  0x4,
    /* 0x3 0011 -> 1100 */  0xc,
    /* 0x4 0100 -> 0010 */  0x2,
    /* 0x5 0101 -> 1010 */  0xa,
    /* 0x6 0110 -> 0110 */  0x6,
    /* 0x7 0111 -> 1110 */  0xe,
    /* 0x8 1000 -> 0001 */  0x1,
    /* 0x9 1001 -> 1001 */  0x9,
    /* 0xa 1010 -> 0101 */  0x5,
    /* 0xb 1011 -> 1101 */  0xd,
    /* 0xc 1100 -> 0011 */  0x3,
    /* 0xd 1101 -> 1011 */  0xb,
    /* 0xe 1110 -> 0111 */  0x7,
    /* 0xf 1111 -> 1111 */  0xf
};

UINT8
EFIAPI
BitReverse (
  UINT8   Value
  )
{
  return (NibbleTab[Value & 0xf] << 4) | NibbleTab[Value >> 4];
}

/*
 * Get DMA Interrupt Stauts.
 * dwmac4_dma_interrupt
 */
VOID
EFIAPI
StmmacGetDmaStatus (
  OUT  UINT32   *IrqStat  OPTIONAL,
  IN   UINTN    MacBaseAddress
  )
{
  UINT32  DmaStatus;
  UINT32  IntrEnable;
  UINT32  ErrorBit;
  UINT32  Mask;
  UINT32  Channel;

  ErrorBit = 0;
  Mask = 0;

  if (IrqStat != NULL) {
    *IrqStat = 0;
  }

  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    DmaStatus = MmioRead32 ((UINTN)MacBaseAddress + DMA_CHAN_STATUS(Channel));
    IntrEnable = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Channel)));

    //
    // TX/RX NORMAL interrupts.
    //
    if (DmaStatus & DMA_CHAN_STATUS_NIS) {
      Mask |= DMA_CHAN_STATUS_NIS;
      //
      // Rx interrupt.
      //
      if (DmaStatus & DMA_CHAN_STATUS_RI) {
        if (IrqStat != NULL) {
          *IrqStat |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
          Mask |= DMA_CHAN_STATUS_RI;
        }
      }

      //
      // Tx interrupt.
      //
      if (DmaStatus & DMA_CHAN_STATUS_TI) {
        if (IrqStat != NULL) {
          *IrqStat |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
          Mask |= DMA_CHAN_STATUS_TI;
        }
      }

      //
      // Tx Buffer.
      //
      if (DmaStatus & DMA_CHAN_STATUS_TBU) {
        Mask |= DMA_CHAN_STATUS_TBU;
      }
      //
      // Early receive interrupt.
      //
      if (DmaStatus & DMA_CHAN_STATUS_ERI) {
        Mask |= DMA_CHAN_STATUS_ERI;
      }
    }

    //
    // ABNORMAL interrupts.
    //
    if (DmaStatus & DMA_CHAN_STATUS_AIS) {
      Mask |= DMA_CHAN_STATUS_AIS;
      //
      // Transmit process stopped.
      //
      if (DmaStatus & DMA_CHAN_STATUS_TPS) {
        DEBUG ((
          DEBUG_INFO,
	  "%a(): Transmit process stopped\n",
	  __func__
	  ));
        Mask |= DMA_CHAN_STATUS_TPS;
      }

      //
      // Receive buffer unavailable.
      //
      if (DmaStatus & DMA_CHAN_STATUS_RBU) {
        Mask |= DMA_CHAN_STATUS_RBU;
      }

      //
      // Receive process stopped.
      //
      if (DmaStatus & DMA_CHAN_STATUS_RPS) {
        DEBUG ((
          DEBUG_INFO,
	  "%a(): Receive process stop\n",
	  __func__
	  ));
        Mask |= DMA_CHAN_STATUS_RPS;
      }

      //
      // Receive watchdog timeout
      //
      if (DmaStatus & DMA_CHAN_STATUS_RWT) {
        DEBUG ((
	  DEBUG_INFO,
          "%a(): Receive watchdog timeout\n",
	  __func__
	  ));
        Mask |= DMA_CHAN_STATUS_RWT;
      }

      //
      // Early transmit interrupt.
      //
      if (DmaStatus & DMA_CHAN_STATUS_ETI) {
        Mask |= DMA_CHAN_STATUS_ETI;
      }

      //
      // Fatal bus error.
      //
      if (DmaStatus & DMA_CHAN_STATUS_FBE) {
        DEBUG ((
          DEBUG_INFO,
	  "%a(): Fatal bus error:\n",
	  __func__
	  ));
        Mask |= DMA_CHAN_STATUS_FBE;

        ErrorBit = DmaStatus & DMA_CHAN_STATUS_TEB >> DMA_CHAN_STATUS_TEB_SHIFT;
        switch (ErrorBit) {
        case DMA_TX_WRITE_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Tx DMA write buffer error\n",
	    __func__
	   ));
          break;
        case DMA_TX_WRITE_DESCRIPTOR_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Tx DMA write descriptor error\n",
	    __func__
	    ));
          break;
        case DMA_TX_READ_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Tx DMA read buffer error\n",
	    __func__
	    ));
          break;
        case DMA_TX_READ_DESCRIPTOR_ERROR:
          DEBUG ((
            DEBUG_INFO,
	    "%a(): Tx DMA read descriptor error\n",
	    __func__
	    ));
          break;
        default:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Undefined error\n",
	    __func__
	    ));
          break;
        }

        ErrorBit = DmaStatus & DMA_CHAN_STATUS_REB >> DMA_CHAN_STATUS_REB_SHIFT;
        switch (ErrorBit) {
        case DMA_RX_WRITE_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Rx DMA write buffer error\n",
	    __func__
	    ));
          break;
        case DMA_RX_WRITE_DESCRIPTOR_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Rx DMA write descriptor error\n",
	    __func__
	    ));
          break;
        case DMA_RX_READ_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Rx DMA read buffer error\n",
	    __func__
	    ));
          break;
        case DMA_RX_READ_DESCRIPTOR_ERROR:
          DEBUG ((
            DEBUG_INFO,
	    "%a(): Rx DMA read descriptor error\n",
	    __func__
	    ));
          break;
        default:
          DEBUG ((
	    DEBUG_INFO,
	    "%a(): Undefined error\n",
	    __func__
	    ));
          break;
        }
      }
    }
    MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_STATUS(Channel)), Mask & IntrEnable);
  }
}

/*
 * MMC: MAC Management Counters
 * drivers/net/ethernet/stmicro/stmmac/mmc_core.c
 */
VOID
EFIAPI
StmmacGetStatistic (
  OUT  EFI_NETWORK_STATISTICS   *Statistic,
  IN   UINTN                     MacBaseAddress
  )
{
  EFI_NETWORK_STATISTICS   *Stats;

  DEBUG ((
    DEBUG_INFO,
    "%a()\r\n",
    __func__
    ));

  //
  // Allocate Resources
  //
  Stats = AllocateZeroPool (sizeof (EFI_NETWORK_STATISTICS));
  if (Stats == NULL) {
    return;
  }

  Stats->RxTotalFrames     = MmioRead32 (MacBaseAddress + MMC_RX_FRAMECOUNT_GB);
  Stats->RxUndersizeFrames = MmioRead32 (MacBaseAddress + MMC_RX_UNDERSIZE_G);
  Stats->RxOversizeFrames  = MmioRead32 (MacBaseAddress + MMC_RX_OVERSIZE_G);
  Stats->RxUnicastFrames   = MmioRead32 (MacBaseAddress + MMC_RX_UNICAST_G);
  Stats->RxBroadcastFrames = MmioRead32 (MacBaseAddress + MMC_RX_BROADCASTFRAME_G);
  Stats->RxMulticastFrames = MmioRead32 (MacBaseAddress + MMC_RX_MULTICASTFRAME_G);
  Stats->RxCrcErrorFrames  = MmioRead32 (MacBaseAddress + MMC_RX_CRC_ERROR);
  Stats->RxTotalBytes      = MmioRead32 (MacBaseAddress + MMC_RX_OCTETCOUNT_GB);
  Stats->RxGoodFrames      = Stats->RxUnicastFrames +
	                     Stats->RxBroadcastFrames +
			     Stats->RxMulticastFrames;

  Stats->TxTotalFrames     = MmioRead32 (MacBaseAddress + MMC_TX_FRAMECOUNT_GB);
  Stats->TxGoodFrames      = MmioRead32 (MacBaseAddress + MMC_TX_FRAMECOUNT_G);
  Stats->TxOversizeFrames  = MmioRead32 (MacBaseAddress + MMC_TX_OVERSIZE_G);
  Stats->TxUnicastFrames   = MmioRead32 (MacBaseAddress + MMC_TX_UNICAST_GB);
  Stats->TxBroadcastFrames = MmioRead32 (MacBaseAddress + MMC_TX_BROADCASTFRAME_G);
  Stats->TxMulticastFrames = MmioRead32 (MacBaseAddress + MMC_TX_MULTICASTFRAME_G);
  Stats->TxTotalBytes      = MmioRead32 (MacBaseAddress + MMC_TX_OCTETCOUNT_GB);
  Stats->Collisions        = MmioRead32 (MacBaseAddress + MMC_TX_LATECOL) +
                             MmioRead32 (MacBaseAddress + MMC_TX_EXESSCOL);

  //
  // Fill in the statistics
  //
  CopyMem (Statistic, Stats, sizeof (EFI_NETWORK_STATISTICS));
}

VOID
EFIAPI
StmmacMacLinkUp (
  IN  UINT32   Speed,
  IN  UINT32   Duplex,
  IN  UINTN    MacBaseAddress
  )
{
  UINT32 OldValue;
  UINT32 Value;

  OldValue = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_CONFIG));

  Value = OldValue & ~(GMAC_CONFIG_FES | GMAC_CONFIG_PS);

  switch (Speed) {
  case SPEED_10:
    Value |= GMAC_CONFIG_PS;
    break;
  case SPEED_100:
    Value |= GMAC_CONFIG_FES | GMAC_CONFIG_PS;
    break;
  case SPEED_1000:
    Value |= 0;
    break;
  case SPEED_2500:
    Value |= GMAC_CONFIG_FES;
    break;
  default:
    break;
  }

  if (Duplex == DUPLEX_FULL) {
    Value |= GMAC_CONFIG_DM;
  }

  if (Value != OldValue) {
    MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_CONFIG), Value);
  }
}

