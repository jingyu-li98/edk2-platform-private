/** @file
  drivers/net/ethernet/stmicro/stmmac/dwmac4_lib.c
  drivers/net/ethernet/stmicro/stmmac/dwmac4_dma.c
  drivers/net/ethernet/stmicro/stmmac/dwmac4_descs.c
  drivers/net/ethernet/stmicro/stmmac/stmmac_main.c

  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "DwMac4DxeUtil.h"
#include "PhyDxeUtil.h"

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#define UPPER_32_BITS(n)      ((UINT32)((n) >> 32))
#define LOWER_32_BITS(n)      ((UINT32)((n) & 0xffffffff))

UINT32 RxChannelsCount = 8;
UINT32 TxChannelsCount = 8;
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
  DEBUG ((
    DEBUG_INFO,
    "SNP:MAC: %a ()\r\n",
    __func__
    ));

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
    "SNP:MAC: GMAC_ADDR_LOW(%d)  = 0x%08X \r\n",
    MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_LOW(RegN))),
    RegN
    ));
  DEBUG ((
    DEBUG_INFO,
    "SNP:MAC: GMAC_ADDR_HIGH(%d)  = 0x%08X \r\n",
    MmioRead32 ((UINTN)(MacBaseAddress + GMAC_ADDR_HIGH(RegN))),
    RegN
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

  DEBUG ((
    DEBUG_INFO,
    "SNP:MAC: %a ()\r\n",
    __func__
    ));

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
    "SNP:MAC: MAC Address = %02X:%02X:%02X:%02X:%02X:%02X\r\n",
    MacAddress->Addr[0],
    MacAddress->Addr[1],
    MacAddress->Addr[2],
    MacAddress->Addr[3],
    MacAddress->Addr[4],
    MacAddress->Addr[5]
  ));
}

/*
 * Enable disable MAC RX/TX.
 */
VOID
EFIAPI
StmmacSetMac (
  IN UINTN   MacBaseAddress,
  IN BOOLEAN Enable
  )
{
  UINT32 OldValue;
  UINT32 Value;

  OldValue = MmioRead32 ((UINTN)(MacBaseAddress + MAC_CTRL_REG));
  Value = OldVal;

  if (Enable) {
    Value |= MAC_ENABLE_RX | MAC_ENABLE_TX;
  } else {
     Value &= ~(MAC_ENABLE_TX | MAC_ENABLE_RX);
  }

  if (Value != OldValue) {
    MmioWrite32 ((UINTN)(MacBaseAddress + MAC_CTRL_REG), Value);
  }
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
/*
 * linux/drivers/net/ethernet/stmicro/stmmac/dwmac4_dma.c
 * const struct stmmac_dma_ops dwmac4_dma_ops = {
        .reset = dwmac4_dma_reset,
        .init = dwmac4_dma_init,
        .init_chan = dwmac4_dma_init_channel,
        .init_rx_chan = dwmac4_dma_init_rx_chan,
        .init_tx_chan = dwmac4_dma_init_tx_chan,
        .axi = dwmac4_dma_axi,
        .dump_regs = dwmac4_dump_dma_regs,
        .dma_rx_mode = dwmac4_dma_rx_chan_op_mode,
        .dma_tx_mode = dwmac4_dma_tx_chan_op_mode,
        .enable_dma_irq = dwmac4_enable_dma_irq,
        .disable_dma_irq = dwmac4_disable_dma_irq,
        .start_tx = dwmac4_dma_start_tx,
        .stop_tx = dwmac4_dma_stop_tx,
        .start_rx = dwmac4_dma_start_rx,
        .stop_rx = dwmac4_dma_stop_rx,
        .dma_interrupt = dwmac4_dma_interrupt,
        .get_hw_feature = dwmac4_get_hw_feature,
        .rx_watchdog = dwmac4_rx_watchdog,
        .set_rx_ring_len = dwmac4_set_rx_ring_len,
        .set_tx_ring_len = dwmac4_set_tx_ring_len,
        .set_rx_tail_ptr = dwmac4_set_rx_tail_ptr,
        .set_tx_tail_ptr = dwmac4_set_tx_tail_ptr,
        .enable_tso = dwmac4_enable_tso,
        .qmode = dwmac4_qmode,
        .set_bfsize = dwmac4_set_bfsize,
        .enable_sph = dwmac4_enable_sph,
 * };
 *
 */
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
    "dwmac4: Master AXI performs %s burst length\n",
    (Value & DMA_SYS_BUS_FB) ? "fixed" : "any"
    ));


  Value &= ~DMA_AXI_WR_OSR_LMT;
  Value |= (AxiWrOsrLmt & DMA_AXI_OSR_MAX) << DMA_AXI_WR_OSR_LMT_SHIFT;

  Value &= ~DMA_AXI_RD_OSR_LMT;
  Value |= (AxiRdOsrLmt & DMA_AXI_OSR_MAX) << DMA_AXI_RD_OSR_LMT_SHIFT;

  Value |= DMA_AXI_BLEN16 | DMA_AXI_BLEN8 | DMA_AXI_BLEN4;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE), Value);
}

VOID
StmmacSetRxTailPtr (
  IN UINTN  MacBaseAddress,
  IN UINT32 TailPtr,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_END_ADDR(Channel)), TailPtr);
}

VOID
StmmacSetTxTailPtr (
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

VOID
DwMac4SetTxRingLen (
  IN  UINTN   MacBaseAddress,
  IN  UINTN   Length,
  IN  UINT32  Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_RING_LEN(Channel)), Length);
}

VOID
DwMac4SetRxRingLen (
  IN  UINTN   MacBaseAddress,
  IN  UINTN   Length,
  IN  UINT32  Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_RING_LEN(Channel)), Length);
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
        "Bus software reset timeout\n"
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

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE), Value);
}

VOID
DwMac4DmaInitChannel (
  IN  STMMAC_DRIVER    *StmmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32 Value;

  //
  // common channel control register config
  //
  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Channel)));
  
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Channel)), Value);

  //
  // Mask interrupts by writing to CSR7
  //
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Channel), DMA_CHAN_INTR_DEFAULT_MASK);
}

VOID
StmmacDmaInitRxChan (
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32           Value;
  UINT32           Index;
  UINT32           RxPbl;
  DMA_DESCRIPTOR   *RxDescriptor;

  RxPbl = 32;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)));
  Value = Value | (RxPbl << DMA_BUS_MODE_RPBL_SHIFT);

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)), Value);

  for (Index = 0; Index < RX_DESC_NUM; Index++) {
    RxDescriptor = (VOID *)(UINTN)StmmacDriver->RxDescRingMap[Index].AddrMap;
    // RxDescriptor->DmaMacAddr = (UINT32)(UINTN)&StmmacDriver->RxBuffer[Index * ETH_BUFFER_SIZE];
    RxDescriptor->DmaMacAddr = StmmacDriver->RxBufNum[Index].AddrMap;

    if (Index < RX_DESC_NUM - 1) {
      RxDescriptor->DmaMacAddrNext = (UINTN)StmmacDriver->RxDescRingMap[Index + 1].AddrMap;
    }
    RxDescriptor->Des0 = LOWER_32_BITS(RxDescriptor->DmaMacAddr);
    RxDescriptor->Des1 = UPPER_32_BITS(RxDescriptor->DmaMacAddr);
    RxDescriptor->Des2 = 0;
    RxDescriptor->Des3 = RDES3_OWN | RDES3_BUFFER1_VALID_ADDR;
  }

  //
  // Correcting the last pointer of the chain
  //
  RxDescriptor->DmaMacAddrNext = (UINTN)StmmacDriver->RxDescRingMap[0].AddrMap;

  //
  // Write the address of tx descriptor list
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
StmmacDmaInitTxChan (
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  UINT32          Value;
  UINT32          Index;
  UINT32          TxPbl;
  DMA_DESCRIPTOR  *TxDescriptor;

  TxPbl = 32;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)));
  Value = Value | (TxPbl << DMA_BUS_MODE_PBL_SHIFT);

  //
  // Enable OSP to get best performance
  //
  Value |= DMA_CONTROL_OSP;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)), Value);

  for (Index = 0; Index < TX_DESC_NUM; Index++) {
    TxDescriptor = (VOID *)(UINTN)StmmacDriver->TxDescRingMap[Index].AddrMap;
    TxDescriptor->DmaMacAddr = (UINTN)&StmmacDriver->TxBuffer[Index * ETH_BUFFER_SIZE];

    if (Index < TX_DESC_NUM - 1) {
      TxDescriptor->DmaMacAddrNext = (UINTN)StmmacDriver->TxDescRingMap[Index + 1].AddrMap;
    }

    TxDescriptor->Des0 = LOWER_32_BITS(RxDescriptor->DmaMacAddr);
    TxDescriptor->Des1 = UPPER_32_BITS(RxDescriptor->DmaMacAddr);
    //TxDescriptor->Des2 = Length & TDES2_BUFFER1_SIZE_MASK;
    TxDescriptor->Des2 = 0;
    TxDescriptor->Des3 = TDES3_OWN | TDES3_PACKET_SIZE_MASK;
  }

  // 
  // Correcting the last pointer of the chain
  //
  TxDescriptor->DmaMacAddrNext = (UINTN)StmmacDriver->TxDescRingMap[0].AddrMap;

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
#if 0
VOID
DwMac4DmaRxChanOpMode (
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          DmaRxPhy,
  IN  UINT32         Mode,
  IN  UINTN          Channel,
  IN  UINT32         Fifosz,
  IN  UINT8          Qmode
  )
{
  UINT32 Rqs;
  UINT32 MtlRxOp;
  UINT32 Rfd;
  UINT32 Rfa;

  Rqs = Fifosz / 256 - 1;
  MtlRxOp = MmioRead32 ((UINTN)(MacBaseAddress + MTL_CHAN_RX_OP_MODE(Channel)));

  if (Mode == SF_DMA_MODE) {
    DEBUG ((
      DEBUG_WARNING,
      "GMAC: enable RX store and forward mode\n"
      ));
    MtlRxOp |= MTL_OP_MODE_RSF;
  } else {
    DEBUG ((
      DEBUG_WARNING,
      "GMAC: disable RX SF mode (threshold %d)\n",
      Mode
      ));
    MtlRxOp &= ~MTL_OP_MODE_RSF;
    MtlRxOp &= MTL_OP_MODE_RTC_MASK;
    if (Mode <= 32) {
      MtlRxOp |= MTL_OP_MODE_RTC_32;
    } else if (Mode <= 64) {
      MtlRxOp |= MTL_OP_MODE_RTC_64;
    } else if (mode <= 96) {
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
  if ((Fifosz >= 4096) && (Qmode != MTL_QUEUE_AVB)) {
    MtlRxOp |= MTL_OP_MODE_EHFC;

    //
    // Set Threshold for Activating Flow Control to min 2 frames,
    // i.e. 1500 * 2 = 3000 bytes.
    //
    // Set Threshold for Deactivating Flow Control to min 1 frame,
    // i.e. 1500 bytes.
    //
    switch (Fifosz) {
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
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  STMMAC_DRIVER  *StmmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          DmaRxPhy,
  IN  UINT32         Mode,
  IN  UINTN          Channel,
  IN  UINT32         Fifosz,
  IN  UINT8          Qmode
  )
{
 UINT32 MtlTxOp;
 UINT32 Tqs;

 MtlTxOp = MmioRead32 ((UINTN)(MacBaseAddress + MTL_CHAN_TX_OP_MODE(Channel)));
 Tqs = Fifosz / 256 - 1;

 if (Mode == SF_DMA_MODE) {
    DEBUG ((
      DEBUG_WARNING,
      "GMAC: enable TX store and forward mode\n"
      ));
    //
    // Transmit COE type 2 cannot be done in cut-through mode. */
    //
    MtlTxOp |= MTL_OP_MODE_TSF;
  } else {
    DEBUG ((
      DEBUG_WARNING,
      "GMAC: disabling TX SF (threshold %d)\n",
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

  MmioWrite32((UINTN)(MacBaseAddress + MTL_CHAN_TX_OP_MODE(Channel), MtlTxOp));
}
#endif
VOID
DwMac4SetTxRingLen (
  IN UINTN  MacBaseAddress,
  IN UINT32 Length,
  IN UINT32 Channel
  )
{
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_RING_LEN(Channel)), Length));
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
#if 0
VOID
DwMac4ProgMtlRxAlgorithms (
  IN struct mac_device_info *hw,
  IN UINT32 RxAlg
  )
{
  UINT32 Value;

  Value = MmioRead32 (UINTN (MacBaseAddress + MTL_OPERATION_MODE));
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

  MmioWrite32 (Value, UINTN (MacBaseAddress + MTL_OPERATION_MODE));
}

VOID
DwMac4ProgMtlTxAlgorithms (
  IN struct mac_device_info *hw,
  IN UINT32 TxAlg
  )
{
  UINT32 Value;

  Value = MmioRead32 (UINTN (MacBaseAddress + MTL_OPERATION_MODE));
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

  MmioWrite32 (Value, UINTN (MacBaseAddress + MTL_OPERATION_MODE));
}
#endif
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
  STMMAC_RX_QUEUE *RxQueue;
  STMMAC_TX_QUEUE *TxQueue;

  DEBUG ((
    DEBUG_INFO,
    "SNP:MAC: %a ()\r\n",
    __func__
    ));

  DmaCsrCh = MAX (RxChannelsCount, TxChannelsCount);

  Status = DwMac4DmaReset (priv, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to reset the dma\n"
      ));
    return Status;
  }

  //
  // DMA Configuration
  //
  DwMac4DmaInit (StmmacDriver, MacBaseAddress);

  //
  // Setup AXI
  //
  StmmacAxi (MacBaseAddress);

  //
  // DMA CSR Channel configuration
  //
  for (Channel = 0; Channel < DmaCsrCh; Channel++) {
    DwMac4DmaInitChannel (MacBaseAddress, Channel);
    // stmmac_disable_dma_irq(priv, priv->ioaddr, chan, 1, 1);
  }

  //
  // DMA RX Channel Configuration
  //
  for (Channel = 0; Channel < RxChannelsCount; Channel++) {
    RxQueue = &DmaConf.RxQueue[Channel];

    StmmacInitRxChan (MacBaseAddress, RxQueue->DmaRxPhy, Channel);

    RxQueue->RxTailAddr = RxQueue->DmaRxPhy + (RxQueue->BufferAllocNum *
                                      sizeof(DMA_DESCRIPTOR));
    StmmacSetRxTailPtr (MacBaseAddress, RxQueue->RxTailAddr, Channel);
  }

  //
  // DMA TX Channel Configuration
  //
  for (Channel = 0; Channel < TxChannelsCount; Channel++) {
    TxQueue = &DmaConf.TxQueue[Channel];

    StmmacInitTxChan (MacBaseAddress, TxQueue->DmaTxPhy, Channel);

    TxQueue>TxTailAddr = TxQueue->DmaTxPhy;

    StmmacSetTxTailPtr (MacBaseAddress, TxQueue->TxTailAddr, Channel);
  }

  return EFI_SUCCESS;
}

/*
 * linux/driver/net/etherent/stmicro/stmmac/dwmac4_descs.c
 * const struct stmmac_desc_ops dwmac4_desc_ops = {
        .tx_status = dwmac4_wrback_get_tx_status,
        .rx_status = dwmac4_wrback_get_rx_status,
        .get_tx_len = dwmac4_rd_get_tx_len,
        .get_tx_owner = dwmac4_get_tx_owner,
        .set_tx_owner = dwmac4_set_tx_owner,
        .set_rx_owner = dwmac4_set_rx_owner,
        .get_tx_ls = dwmac4_get_tx_ls,
        .get_rx_frame_len = dwmac4_wrback_get_rx_frame_len,
        .enable_tx_timestamp = dwmac4_rd_enable_tx_timestamp,
        .get_tx_timestamp_status = dwmac4_wrback_get_tx_timestamp_status,
        .get_rx_timestamp_status = dwmac4_wrback_get_rx_timestamp_status,
        .get_timestamp = dwmac4_get_timestamp,
        .set_tx_ic = dwmac4_rd_set_tx_ic,
        .prepare_tx_desc = dwmac4_rd_prepare_tx_desc,
        .prepare_tso_tx_desc = dwmac4_rd_prepare_tso_tx_desc,
        .release_tx_desc = dwmac4_release_tx_desc,
        .init_rx_desc = dwmac4_rd_init_rx_desc,
        .init_tx_desc = dwmac4_rd_init_tx_desc,
        .display_ring = dwmac4_display_ring,
        .set_mss = dwmac4_set_mss_ctxt,
        .set_addr = dwmac4_set_addr,
        .clear = dwmac4_clear,
        .set_sarc = dwmac4_set_sarc,
        .set_vlan_tag = dwmac4_set_vlan_tag,
        .set_vlan = dwmac4_set_vlan,
        .get_rx_header_len = dwmac4_get_rx_header_len,
        .set_sec_addr = dwmac4_set_sec_addr,
        .set_tbs = dwmac4_set_tbs,
 * };
 *
 */
#if 0
DwMac4RdPrepareTxDesc (
  IN DMA_DESC *p,
  IN UINT32   IsFs,
  IN UINT32   Length,
  IN BOOLEAN  CsumFlag,
  IN UINT32   Mode,
  IN UINT32   TxOwn,
  IN BOOLEAN  Ls,
  IN UINT32   TotPktLength)
{
  UINT32 Tdes3;

  Tdes3 = le32_to_cpu(p->Des3);

  p->Des2 |= cpu_to_le32(Length & TDES2_BUFFER1_SIZE_MASK);

  Tdes3 |= TotPktLength & TDES3_PACKET_SIZE_MASK;
  if (IsFs) {
    Tdes3 |= TDES3_FIRST_DESCRIPTOR;
  } else {
    Tdes3 &= ~TDES3_FIRST_DESCRIPTOR;
  }

  if (likely(CsumFlag)) {
    Tdes3 |= (TX_CIC_FULL << TDES3_CHECKSUM_INSERTION_SHIFT);
  } else {
    Tdes3 &= ~(TX_CIC_FULL << TDES3_CHECKSUM_INSERTION_SHIFT);
  }

  if (Ls) {
    Tdes3 |= TDES3_LAST_DESCRIPTOR;
  } else {
    Tdes3 &= ~TDES3_LAST_DESCRIPTOR;
  }

  //
  // Finally set the OWN bit. Later the DMA will start!
  //
  if (TxOwn) {
    Tdes3 |= TDES3_OWN;
  }

   if (IsFs && TxOwn) {
     //
     // When the own bit, for the first frame, has to be set, all
     // descriptors for the same frame has to be set before, to
     // avoid race condition.
     //
     dma_wmb();
   }

   p->Des3 = cpu_to_le32(Tdes3);
}
#endif

EFI_STATUS
TxDescsInit (
  IN eth_mac_handle_t handle
  )
{
  UINT32 Index;
  gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
  dw_gmac_priv *priv = mac_dev->priv;
  DW_GMAC_DMA_REGS *dma_reg = priv->dma_regs_p;
  DMA_MAC_DESC *DescTable = &priv->tx_mac_descrtable[0];
  INT8 *TxBuffs = &priv->txbuffs[0];
  DMA_MAC_DESC *Desc;

  for (Index = 0; Index < CONFIG_TX_DESCR_NUM; Index++) {
    Desc = &DescTable[Index];
    Desc->DmaMacAddr = (unsigned long)&TxBuffs[Index * CVI_CONFIG_ETH_BUFSIZE];
    Desc->DmaMacNext = (unsigned long)&DescTable[Index + 1];

#if defined(CONFIG_DW_ALTDESCRIPTOR)
    Desc->txrx_status &= ~(CVI_DESC_TXSTS_TXINT | CVI_DESC_TXSTS_TXLAST |
                             CVI_DESC_TXSTS_TXFIRST | CVI_DESC_TXSTS_TXCRCDIS |
                             CVI_DESC_TXSTS_TXCHECKINSCTRL |
                             CVI_DESC_TXSTS_TXRINGEND | CVI_DESC_TXSTS_TXPADDIS);

    Desc->txrx_status |= CVI_DESC_TXSTS_TXCHAIN;
    Desc->dmamac_cntl = 0;
    Desc->txrx_status &= ~(CVI_DESC_TXSTS_MSK | CVI_DESC_TXSTS_OWNBYDMA);
#else
    Desc->dmamac_cntl = CVI_DESC_TXCTRL_TXCHAIN;
    Desc->txrx_status = 0;
#endif
  }

  // 
  // Correcting the last pointer of the chain */
  //
  Desc->dmamac_next = (unsigned long)&DescTable[0];

  //
  // Flush all Tx buffer descriptors at once
  //
  soc_dcache_clean_invalid_range((unsigned long)priv->tx_mac_descrtable, sizeof(priv->tx_mac_descrtable));
  
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_BASE_ADDR_HI(0)), (UINTN)(&DescTable[0]));

  priv->tx_currdescnum = 0;
}

/*
 * linux/drivers/net/ethernet/stmicro/stmmac/dwmac4_core.c
 * const struct stmmac_ops dwmac4_ops = {
        .core_init = dwmac4_core_init,
        .set_mac = stmmac_set_mac,
        .rx_ipc = dwmac4_rx_ipc_enable,
        .rx_queue_enable = dwmac4_rx_queue_enable,
        .rx_queue_prio = dwmac4_rx_queue_priority,
        .tx_queue_prio = dwmac4_tx_queue_priority,
        .rx_queue_routing = dwmac4_rx_queue_routing,
        .prog_mtl_rx_algorithms = dwmac4_prog_mtl_rx_algorithms,
        .prog_mtl_tx_algorithms = dwmac4_prog_mtl_tx_algorithms,
        .set_mtl_tx_queue_weight = dwmac4_set_mtl_tx_queue_weight,
        .map_mtl_to_dma = dwmac4_map_mtl_dma,
        .config_cbs = dwmac4_config_cbs,
        .dump_regs = dwmac4_dump_regs,
        .host_irq_status = dwmac4_irq_status,
        .host_mtl_irq_status = dwmac4_irq_mtl_status,
        .flow_ctrl = dwmac4_flow_ctrl,
        .pmt = dwmac4_pmt,
        .set_umac_addr = dwmac4_set_umac_addr,
        .get_umac_addr = dwmac4_get_umac_addr,
        .set_eee_mode = dwmac4_set_eee_mode,
        .reset_eee_mode = dwmac4_reset_eee_mode,
        .set_eee_lpi_entry_timer = dwmac4_set_eee_lpi_entry_timer,
        .set_eee_timer = dwmac4_set_eee_timer,
        .set_eee_pls = dwmac4_set_eee_pls,
        .pcs_ctrl_ane = dwmac4_ctrl_ane,
        .pcs_rane = dwmac4_rane,
        .pcs_get_adv_lp = dwmac4_get_adv_lp,
        .debug = dwmac4_debug,
        .set_filter = dwmac4_set_filter,
        .set_mac_loopback = dwmac4_set_mac_loopback,
        .update_vlan_hash = dwmac4_update_vlan_hash,
        .sarc_configure = dwmac4_sarc_configure,
        .enable_vlan = dwmac4_enable_vlan,
        .set_arp_offload = dwmac4_set_arp_offload,
        .config_l3_filter = dwmac4_config_l3_filter,
        .config_l4_filter = dwmac4_config_l4_filter,
        .add_hw_vlan_rx_fltr = dwmac4_add_hw_vlan_rx_fltr,
        .del_hw_vlan_rx_fltr = dwmac4_del_hw_vlan_rx_fltr,
        .restore_hw_vlan_rx_fltr = dwmac4_restore_hw_vlan_rx_fltr,
 * };
 */
VOID
EFIAPI
StmmacCoreInit (
  IN UINTN MacBaseAddress
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_CONFIG));
  Value |= GMAC_CORE_INIT;

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_CONFIG), Value);
}

EFI_STATUS
EFIAPI
StmmacSetFilter (
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
    // Set the hash tables
    //
    if ((NumMfilter > 0) && (!Reset)) {
      //
      // Go through each filter address and set appropriate bits on hash table
      //
      for (Index = 0; Index < NumMfilter; Index++) {
        //
	// Generate a 32-bit CRC
	//
        Crc = GenEtherCrc32 (&Mfilter[Index], NET_ETHER_ADDR_LEN);
        //
	// reserve CRC + take upper 8 bit = take lower 8 bit and reverse it
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
        // set 1 to HashBit of HashReg
        // for example, set 1 to bit 31 to Reg 5 as in above example
	//
        Register |= (1 << HashBit);
        MmioWrite32 ((UINTN)(MacBaseAddress + HASH_TABLE_REG(HashReg)), Register);
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
  // Set MacFilter to GMAC_PACKET_FILTER register
  //
  MmioWrite32_((UINTN)(MacBaseAddress + GMAC_PACKET_FILTER), MacFilter);

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
  // Generate the Crc bit-by-bit (LSB first)
  //
  while (AddrLen--) {
    Remainder ^= *Ptr++;
    for (Iter = 0; Iter < 8; Iter++) {
      //
      // Check if exponent is set
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
    // TX/RX NORMAL interrupts
    //
    if (DmaStatus & DMA_CHAN_STATUS_NIS) {
      Mask |= DMA_CHAN_STATUS_NIS;
      //
      // Rx interrupt
      //
      if (DmaStatus & DMA_CHAN_STATUS_RI) {
        if (IrqStat != NULL) {
          *IrqStat |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
          Mask |= DMA_CHAN_STATUS_RI;
        }
      }

      //
      // Tx interrupt
      //
      if (DmaStatus & DMA_CHAN_STATUS_TI) {
        if (IrqStat != NULL) {
          *IrqStat |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
          Mask |= DMA_CHAN_STATUS_TI;
        }
      }

      //
      // Tx Buffer
      //
      if (DmaStatus & DMA_CHAN_STATUS_TBU) {
        Mask |= DMA_CHAN_STATUS_TBU;
      }
      //
      // Early receive interrupt
      //
      if (DmaStatus & DMA_CHAN_STATUS_ERI) {
        Mask |= DMA_CHAN_STATUS_ERI;
      }
    }

    //
    // ABNORMAL interrupts
    //
    if (DmaStatus & DMA_CHAN_STATUS_AIS) {
      Mask |= DMA_CHAN_STATUS_AIS;
      //
      // Transmit process stopped
      //
      if (DmaStatus & DMA_CHAN_STATUS_TPS) {
        DEBUG ((
          DEBUG_INFO,
	  "SNP:MAC: Transmit process stopped\n"
	  ));
        Mask |= DMA_CHAN_STATUS_TPS;
      }
    
      //
      // Receive buffer unavailable
      //
      if (DmaStatus & DMA_CHAN_STATUS_RBU) {
        Mask |= DMA_CHAN_STATUS_RBU;
      }

      //
      // Receive process stopped
      //
      if (DmaStatus & DMA_CHAN_STATUS_RPS) {
        DEBUG ((
          DEBUG_INFO,
	  "SNP:MAC: Receive process stop\n"
	  ));
        Mask |= DMA_CHAN_STATUS_RPS;
      }

      //
      // Receive watchdog timeout
      //
      if (DmaStatus & DMA_CHAN_STATUS_RWT) {
        DEBUG ((
	  DEBUG_INFO,
          "SNP:MAC: Receive watchdog timeout\n"
	  ));
        Mask |= DMA_CHAN_STATUS_RWT;
      }

      //
      // Early transmit interrupt
      //
      if (DmaStatus & DMA_CHAN_STATUS_ETI) {
        Mask |= DMA_CHAN_STATUS_ETI;
      }

      //
      // Fatal bus error
      //
      if (DmaStatus & DMA_CHAN_STATUS_FBE) {
        DEBUG ((
          DEBUG_INFO,
	  "SNP:MAC: Fatal bus error:\n"
	  ));
        Mask |= DMA_CHAN_STATUS_FBE;

        ErrorBit = DmaStatus & DMA_CHAN_STATUS_TEB >> DMA_CHAN_STATUS_TEB_SHIFT;
        switch (ErrorBit) {
        case DMA_TX_WRITE_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	   "SNP:MAC: Tx DMA write buffer error\n"
	   ));
          break;
        case DMA_TX_WRITE_DESCRIPTOR_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Tx DMA write descriptor error\n"));
          break;
        case DMA_TX_READ_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Tx DMA read buffer error\n"
	    ));
          break;
        case DMA_TX_READ_DESCRIPTOR_ERROR:
          DEBUG ((
            DEBUG_INFO,
	    "SNP:MAC: Tx DMA read descriptor error\n"));
          break;
        default:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Undefined error\n"
	    ));
          break;
        }

        ErrorBit = DmaStatus & DMA_CHAN_STATUS_REB >> DMA_CHAN_STATUS_REB_SHIFT;
        switch (ErrorBit) {
        case DMA_RX_WRITE_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Rx DMA write buffer error\n"
	    ));
          break;
        case DMA_RX_WRITE_DESCRIPTOR_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Rx DMA write descriptor error\n"));
          break;
        case DMA_RX_READ_DATA_BUFFER_ERROR:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Rx DMA read buffer error\n"
	    ));
          break;
        case DMA_RX_READ_DESCRIPTOR_ERROR:
          DEBUG ((
            DEBUG_INFO,
	    "SNP:MAC: Rx DMA read descriptor error\n"));
          break;
        default:
          DEBUG ((
	    DEBUG_INFO,
	    "SNP:MAC: Undefined error\n"
	    ));
          break;
        }
      }
    }
    MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_STATUS(Channel), Mask & IntrEnable));
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
    "SNP:MAC: %a ()\r\n",
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

#if 0
VOID
EFIAPI
StmmacConfigAdjust (
  IN  UINT32   Speed,
  IN  UINT32   Duplex,
  IN  UINTN    MacBaseAddress
  )
{
  UINT32   Config;

  Config = 0;
  if (Speed != SPEED_1000) {
   Config |= DW_EMAC_GMACGRP_MAC_CONFIGURATION_PS_SET_MSK;
  }
  if (Speed == SPEED_100) {
    Config |= DW_EMAC_GMACGRP_MAC_CONFIGURATION_FES_SET_MSK;
  }
  if (Duplex == DUPLEX_FULL) {
    Config |= DW_EMAC_GMACGRP_MAC_CONFIGURATION_DM_SET_MSK;
  }
  MmioOr32 (MacBaseAddress +
            DW_EMAC_GMACGRP_MAC_CONFIGURATION_OFST,
            DW_EMAC_GMACGRP_MAC_CONFIGURATION_BE_SET_MSK |
            DW_EMAC_GMACGRP_MAC_CONFIGURATION_DO_SET_MSK |
            Config);

}
#endif

