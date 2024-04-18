/** @file

  Copyright (c) 2011 - 2019, Intel Corporaton. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

  The original software modules are licensed as follows:

  Copyright (c) 2012 - 2014, ARM Limited. All rights reserved.
  Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "EmacDxeUtil.h"
#include "PhyDxeUtil.h"

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#define UPPER_32_BITS(n)      ((UINT32)((n) >> 32))
#define LOWER_32_BITS(n)      ((UINT32)((n) & 0xffffffff))

VOID
EFIAPI
DwMac4SetMacAddr (
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
DwMac4GetMacAddr (
  OUT  EFI_MAC_ADDRESS   *MacAddress,
  IN   UINTN             MacBaseAddress,
  IN   UINTN             RegN
  )
{
  UINT32          MacAddrHighValue;
  UINT32          MacAddrLowValue;

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
EmacDxeInitialization (
  IN  EMAC_DRIVER   *EmacDriver,
  IN  UINTN         MacBaseAddress
  )
{
  DEBUG ((DEBUG_INFO, "SNP:MAC: %a ()\r\n", __func__));

  // Init EMAC DMA
  EmacDmaInit (EmacDriver, MacBaseAddress);

  return EFI_SUCCESS;
}

int
Dwmac4DmaReset (
  IN  UINTN         MacBaseAddress
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE));

  //
  // DMA SW reset
  //
  Value |= DMA_BUS_MODE_SFT_RESET;
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE), Value);

        return readl_poll_timeout(ioaddr + DMA_BUS_MODE, value,
                                 !(value & DMA_BUS_MODE_SFT_RESET),
                                 10000, 1000000);
}

VOID
DwMac4DmaInit (
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  EMAC_DRIVER    *EmacDriver,
  IN  UINTN          MacBaseAddress
  )
{
  UINT32 Value;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE));

  // 
  // Set the Fixed burst mode
  //
  if (DmaCfg->FixedBurst)
    Value |= DMA_SYS_BUS_FB;

  //
  // Mixed Burst has no effect when fb is set
  //
  if (DmaCfg->MixedBurst)
    Value |= DMA_SYS_BUS_MB;

  if (DmaCfg->Aal)
    Value |= DMA_SYS_BUS_AAL;

  if (DmaCfg->Eame)
    Value |= DMA_SYS_BUS_EAME;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_SYS_BUS_MODE), Value);

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE));

  if (DmaCfg->MultiMsiEn) {
    Value &= ~DMA_BUS_MODE_INTM_MASK;
    Value |= (DMA_BUS_MODE_INTM_MODE1 << DMA_BUS_MODE_INTM_SHIFT);
  }

  if (DmaCfg->Dche)
    Value |= DMA_BUS_MODE_DCHE;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_BUS_MODE), Value);

}

VOID
DwMac4DmaInitChannel (
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  EMAC_DRIVER    *EmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          Channel
  )
{
  CONST struct Dwmac4Addrs *Dwmac4Addrs = priv->plat->Dwmac4Addrs;
  UINT32 Value;

  //
  // common channel control register config
  //
  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Dwmac4Addrs, Channel)));
  if (DmaCfg->Pblx8) {
   Value = Value | DMA_BUS_MODE_PBL;
  }
  
  MmioWrite32 (((UINTN)(MacBaseAddress + DMA_CHAN_CONTROL(Dwmac4Addrs, Channel)), Value);

  //
  // Mask interrupts by writing to CSR7
  //
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_INTR_ENA(Dwmac4Addrs, Channel), DMA_CHAN_INTR_DEFAULT_MASK);
}

VOID
DwMac4DmaInitRxChan (
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  EMAC_DRIVER    *EmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          DmaRxPhy,
  IN  UINTN          Channel
  )
{
  UINT32 Value;
  UINT32 RxPbl;

  RxPbl = DmaCfg->RxPbl ? : DmaCfg->Pbl;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)));
  Value = Value | (RxPbl << DMA_BUS_MODE_RPBL_SHIFT);
  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_CONTROL(Channel)), Value));

  if (IS_ENABLED(CONFIG_ARCH_DMA_ADDR_T_64BIT) && likely(DmaCfg->Eame)) {
    MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_BASE_ADDR_HI(Channel)),
		    UPPER_32_BITS(DmaRxPhy));
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_RX_BASE_ADDR(Channel)),
		    LOWER_32_BITS(DmaRxPhy));
}

VOID
DwMac4DmaInitTxChan (
  IN  STMMAC_DMA_CFG *DmaCfg,
  IN  EMAC_DRIVER    *EmacDriver,
  IN  UINTN          MacBaseAddress,
  IN  UINTN          DmaRxPhy,
  IN  UINTN          Channel
  )
{
  UINT32 Value;
  UINT32 TxPbl;

  TxPbl = DmaCfg->TxPbl ? : DmaCfg->Pbl;

  Value = MmioRead32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)));
  Value = Value | (TxPbl << DMA_BUS_MODE_PBL_SHIFT);

  //
  // Enable OSP to get best performance
  //
  Value |= DMA_CONTROL_OSP;

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_CONTROL(Channel)), Value);

  if (IS_ENABLED(CONFIG_ARCH_DMA_ADDR_T_64BIT) && likely(DmaCfg->Eame)) {
    MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_BASE_ADDR_HI(Channel)),
		    UPPER_32_BITS(DmaTxPhy));
  }

  MmioWrite32 ((UINTN)(MacBaseAddress + DMA_CHAN_TX_BASE_ADDR(Channel)),
		    LOWER_32_BITS(DmaTxPhy));
}

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

EFI_STATUS
EFIAPI
EmacSetupTxdesc (
  IN  EMAC_DRIVER   *EmacDriver,
  IN  UINTN         MacBaseAddress
 )
{
  INTN                       Index;
  DESIGNWARE_HW_DESCRIPTOR   *TxDescriptor;

  for (Index = 0; Index < CONFIG_TX_DESCR_NUM; Index++) {
    TxDescriptor = (VOID *)(UINTN)EmacDriver->TxdescRingMap[Index].AddrMap;
    TxDescriptor->Addr = (UINT32)(UINTN)&EmacDriver->TxBuffer[Index * CONFIG_ETH_BUFSIZE];
    if (Index < 9) {
      TxDescriptor->AddrNext = (UINT32)(UINTN)EmacDriver->TxdescRingMap[Index + 1].AddrMap;
    }
    TxDescriptor->Tdes0 = TDES0_TXCHAIN;
    TxDescriptor->Tdes1 = 0;
  }

  // Correcting the last pointer of the chain
  TxDescriptor->AddrNext = (UINT32)(UINTN)EmacDriver->TxdescRingMap[0].AddrMap;

  // Write the address of tx descriptor list
  MmioWrite32 (MacBaseAddress +
              DW_EMAC_DMAGRP_TRANSMIT_DESCRIPTOR_LIST_ADDRESS_OFST,
              (UINT32)(UINTN)EmacDriver->TxdescRingMap[0].AddrMap);

  // Initialize the descriptor number
  EmacDriver->TxCurrentDescriptorNum = 0;
  EmacDriver->TxNextDescriptorNum = 0;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
EmacSetupRxdesc (
  IN  EMAC_DRIVER   *EmacDriver,
  IN  UINTN         MacBaseAddress
  )
{
  INTN                        Index;
  DESIGNWARE_HW_DESCRIPTOR    *RxDescriptor;

  for (Index = 0; Index < CONFIG_RX_DESCR_NUM; Index++) {
    RxDescriptor = (VOID *)(UINTN)EmacDriver->RxdescRingMap[Index].AddrMap;
    RxDescriptor->Addr = EmacDriver->RxBufNum[Index].AddrMap;
    if (Index < 9) {
      RxDescriptor->AddrNext = (UINT32)(UINTN)EmacDriver->RxdescRingMap[Index + 1].AddrMap;
    }
    RxDescriptor->Tdes0 = RDES0_OWN;
    RxDescriptor->Tdes1 = RDES1_CHAINED | RX_MAX_PACKET;
  }

  // Correcting the last pointer of the chain
  RxDescriptor->AddrNext = (UINT32)(UINTN)EmacDriver->RxdescRingMap[0].AddrMap;

  // Write the address of tx descriptor list
  MmioWrite32(MacBaseAddress +
              DW_EMAC_DMAGRP_RECEIVE_DESCRIPTOR_LIST_ADDRESS_OFST,
              (UINT32)(UINTN)EmacDriver->RxdescRingMap[0].AddrMap);

  // Initialize the descriptor number
  EmacDriver->RxCurrentDescriptorNum = 0;
  EmacDriver->RxNextDescriptorNum = 0;

  return EFI_SUCCESS;
}

VOID
DwMac4SetFilter(struct mac_device_info *hw,
                              struct net_device *dev)
  IN  UINTN   MacBaseAddress
  )
{
  UINT32 Value;
        int numhashregs = (hw->multicast_filter_bins >> 5);
        int mcbitslog2 = hw->mcast_bits_log2;
        unsigned int value;
        u32 mc_filter[8];
        int i;

        memset(mc_filter, 0, sizeof(mc_filter));

  Value = MmioRead32 (UINTN (MacBaseAddress + GMAC_PACKET_FILTER));
  Value &= ~GMAC_PACKET_FILTER_HMC;
  Value &= ~GMAC_PACKET_FILTER_HPF;
  Value &= ~GMAC_PACKET_FILTER_PCF;
  Value &= ~GMAC_PACKET_FILTER_PM;
  Value &= ~GMAC_PACKET_FILTER_PR;
  Value &= ~GMAC_PACKET_FILTER_RA;
  if (dev->flags & IFF_PROMISC) {
    /* VLAN Tag Filter Fail Packets Queuing */
    if (hw->vlan_fail_q_en) {
      Value = MmioRead32 ((UINTN)(MacBaseAddress + GMAC_RXQ_CTRL4));
      Value &= ~GMAC_RXQCTRL_VFFQ_MASK;
      Value |= GMAC_RXQCTRL_VFFQE |
               (hw->vlan_fail_q << GMAC_RXQCTRL_VFFQ_SHIFT);
      MmioWrite32 (Value, (UINTN)(MacBaseAddress + GMAC_RXQ_CTRL4));
      Value = GMAC_PACKET_FILTER_PR | GMAC_PACKET_FILTER_RA;
    } else {
      Value = GMAC_PACKET_FILTER_PR | GMAC_PACKET_FILTER_PCF;
    }

  } else if ((dev->flags & IFF_ALLMULTI) ||
             (netdev_mc_count(dev) > hw->multicast_filter_bins)) {
    /* Pass all multi */
    Value |= GMAC_PACKET_FILTER_PM;
    /* Set all the bits of the HASH tab */
    memset(mc_filter, 0xff, sizeof(mc_filter));
  } else if (!netdev_mc_empty(dev) && (dev->flags & IFF_MULTICAST)) {
    struct netdev_hw_addr *ha;

    /* Hash filter for multicast */
    Value |= GMAC_PACKET_FILTER_HMC;

    netdev_for_each_mc_addr(ha, dev) {
      //
      // The upper n bits of the calculated CRC are used to
      // index the contents of the hash table. The number of
      // bits used depends on the hardware configuration
      // selected at core configuration time.
      //
      u32 bit_nr = bitrev32(~crc32_le(~0, ha->addr,
                                        ETH_ALEN)) >> (32 - mcbitslog2);
      //
      // The most significant bit determines the register to use (H/L)
      // while the other 5 bits determine the bit within the register.
      //
      mc_filter[bit_nr >> 5] |= (1 << (bit_nr & 0x1f));
     }
   }

   for (i = 0; i < numhashregs; i++)
     writel(mc_filter[i], ioaddr + GMAC_HASH_TAB(i));

     Value |= GMAC_PACKET_FILTER_HPF;

     //
     // Handle multiple unicast addresses
     //
     if (netdev_uc_count(dev) > hw->unicast_filter_entries) {
       //
       // Switch to promiscuous mode if more than 128 addrs are required
       //
       Value |= GMAC_PACKET_FILTER_PR;
     } else {
       struct netdev_hw_addr *ha;
       int reg = 1;

       netdev_for_each_uc_addr(ha, dev) {
         dwmac4_set_umac_addr(hw, ha->addr, reg);
         reg++;
       }

       while (reg < GMAC_MAX_PERFECT_ADDRESSES) {
         MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_ADDR_HIGH(reg)), 0);
         MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_ADDR_LOW(reg)), 0);
        reg++;
       }
     }

        /* VLAN filtering */
        if (dev->flags & IFF_PROMISC && !hw->vlan_fail_q_en)
                value &= ~GMAC_PACKET_FILTER_VTFE;
        else if (dev->features & NETIF_F_HW_VLAN_CTAG_FILTER)
                value |= GMAC_PACKET_FILTER_VTFE;

  MmioWrite32 ((UINTN)(MacBaseAddress + GMAC_PACKET_FILTER), Value);
}

EFI_STATUS
EFIAPI
EmacRxFilters (
  IN  UINT32            ReceiveFilterSetting,
  IN  BOOLEAN           Reset,
  IN  UINTN             NumMfilter          OPTIONAL,
  IN  EFI_MAC_ADDRESS   *Mfilter            OPTIONAL,
  IN  UINTN             MacBaseAddress
  )
{
  UINT32  MacFilter;
  UINT32  Crc;
  UINT32  Count;
  UINT32  HashReg;
  UINT32  HashBit;
  UINT32  Reg;
  UINT32  Val;

  // If reset then clear the filter registers
  if (Reset) {
    for (Count = 0; Count < NumMfilter; Count++)
    {
      MmioWrite32 (MacBaseAddress + HASH_TABLE_REG(Count), 0x00000000);
    }
  }

  // Set MacFilter to the reset value of the  DW_EMAC_GMACGRP_MAC_FRAME_FILTER register.
  MacFilter =  DW_EMAC_GMACGRP_MAC_FRAME_FILTER_RESET;

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    MacFilter |=  DW_EMAC_GMACGRP_MAC_FRAME_FILTER_HMC_SET_MSK;

    // Set the hash tables
    if ((NumMfilter > 0) && (!Reset)) {
      // Go through each filter address and set appropriate bits on hash table
      for (Count = 0; Count < NumMfilter; Count++) {
        // Generate a 32-bit CRC
        Crc = GenEtherCrc32 (&Mfilter[Count], 6);
        // reserve CRC + take upper 8 bit = take lower 8 bit and reverse it
        Val = BitReverse(Crc & 0xff);
        // The most significant bits determines the register to be used (Hash Table Register X),
        // and the least significant five bits determine the bit within the register.
        // For example, a hash value of 8b'10111111 selects Bit 31 of the Hash Table Register 5.
        HashReg = (Val >> 5);
        HashBit = (Val & 0x1f);

        Reg = MmioRead32 (MacBaseAddress + HASH_TABLE_REG(HashReg));
        // set 1 to HashBit of HashReg
        // for example, set 1 to bit 31 to Reg 5 as in above example
        Reg |= (1 << HashBit);
        MmioWrite32(MacBaseAddress + HASH_TABLE_REG(HashReg), Reg);
      }
    }
  }

  if ((ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) == 0) {
    MacFilter |= DW_EMAC_GMACGRP_MAC_FRAME_FILTER_DBF_SET_MSK;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    MacFilter |= DW_EMAC_GMACGRP_MAC_FRAME_FILTER_PR_SET_MSK;
  }

  if (ReceiveFilterSetting & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST) {
    MacFilter |= ( DW_EMAC_GMACGRP_MAC_FRAME_FILTER_PM_SET_MSK);
  }

  // Set MacFilter to EMAC register
  MmioWrite32 (MacBaseAddress +  DW_EMAC_GMACGRP_MAC_FRAME_FILTER_OFST, MacFilter);
  return EFI_SUCCESS;
}


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
  Remainder = 0xFFFFFFFF;    // 0xFFFFFFFF is standard seed for Ethernet

  // Convert Mac Address to array of bytes
  Ptr = (UINT8 *)Mac;

  // Generate the Crc bit-by-bit (LSB first)
  while (AddrLen--) {
    Remainder ^= *Ptr++;
    for (Iter = 0; Iter < 8; Iter++) {
      // Check if exponent is set
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


VOID
EFIAPI
EmacStopTxRx (
   IN  UINTN   MacBaseAddress
  )
{
  DEBUG ((DEBUG_INFO, "SNP:MAC: %a ()\r\n", __func__));

  // Stop DMA TX
  MmioAnd32 (MacBaseAddress +
             DW_EMAC_DMAGRP_OPERATION_MODE_OFST,
             DW_EMAC_DMAGRP_OPERATION_MODE_ST_CLR_MSK);

  // Flush TX
  MmioOr32 (MacBaseAddress +
            DW_EMAC_DMAGRP_OPERATION_MODE_OFST,
            DW_EMAC_DMAGRP_OPERATION_MODE_FTF_SET_MSK);

  // Stop transmitters
  MmioAnd32 (MacBaseAddress +
             DW_EMAC_GMACGRP_MAC_CONFIGURATION_OFST,
             DW_EMAC_GMACGRP_MAC_CONFIGURATION_RE_CLR_MSK &
             DW_EMAC_GMACGRP_MAC_CONFIGURATION_TE_CLR_MSK);

  // Stop DMA RX
  MmioAnd32 (MacBaseAddress +
             DW_EMAC_DMAGRP_OPERATION_MODE_OFST,
             DW_EMAC_DMAGRP_OPERATION_MODE_SR_CLR_MSK);

}


EFI_STATUS
EFIAPI
EmacDmaStart (
  IN  UINTN   MacBaseAddress
  )
{
  // Start the transmission
  MmioWrite32(MacBaseAddress +
              DW_EMAC_DMAGRP_TRANSMIT_POLL_DEMAND_OFST,
              0x1);
  return EFI_SUCCESS;
}


VOID
EFIAPI
EmacGetDmaStatus (
  OUT  UINT32   *IrqStat  OPTIONAL,
  IN   UINTN    MacBaseAddress
  )
{
  UINT32  DmaStatus;
  UINT32  ErrorBit;
  UINT32  Mask = 0;

  if (IrqStat != NULL) {
    *IrqStat = 0;
  }

  DmaStatus = MmioRead32 (MacBaseAddress +
                           DW_EMAC_DMAGRP_STATUS_OFST);
  if (DmaStatus & DW_EMAC_DMAGRP_STATUS_NIS_SET_MSK) {
    Mask |= DW_EMAC_DMAGRP_STATUS_NIS_SET_MSK;
    // Rx interrupt
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_RI_SET_MSK) {
      if (IrqStat != NULL) {
        *IrqStat |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
        Mask |= DW_EMAC_DMAGRP_STATUS_RI_SET_MSK;
      }
    }

    // Tx interrupt
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_TI_SET_MSK) {
      if (IrqStat != NULL) {
        *IrqStat |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
        Mask |= DW_EMAC_DMAGRP_STATUS_TI_SET_MSK;
      }
    }

    // Tx Buffer
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_TU_SET_MSK){
      Mask |= DW_EMAC_DMAGRP_STATUS_TU_SET_MSK;
      }
    // Early receive interrupt
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_ERI_SET_MSK) {
      Mask |= DW_EMAC_DMAGRP_STATUS_ERI_SET_MSK;
    }
  }
  if (DmaStatus & DW_EMAC_DMAGRP_STATUS_AIS_SET_MSK) {
    Mask |= DW_EMAC_DMAGRP_STATUS_AIS_SET_MSK;
    // Transmit process stop
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_TPS_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Transmit process stop\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_TPS_SET_MSK;
    }
    // Transmit jabber timeout
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_TJT_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Transmit jabber timeout\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_TJT_SET_MSK;
    }
    // Receive FIFO overflow
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_OVF_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Receive FIFO overflow\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_OVF_SET_MSK;
    }
    // Transmit FIFO underflow
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_UNF_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Receive FIFO underflow\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_UNF_SET_MSK;
    }
    // Receive buffer unavailable
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_RU_SET_MSK) {
      Mask |= DW_EMAC_DMAGRP_STATUS_RU_SET_MSK;
    }

    // Receive process stop
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_RPS_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Receive process stop\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_RPS_SET_MSK;
    }
    // Receive watchdog timeout
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_RWT_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Receive watchdog timeout\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_RWT_SET_MSK;
    }
    // Early transmit interrupt
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_ETI_SET_MSK) {
      Mask |= DW_EMAC_DMAGRP_STATUS_ETI_SET_MSK;
    }
    // Fatal bus error
    if (DmaStatus & DW_EMAC_DMAGRP_STATUS_FBI_SET_MSK) {
      DEBUG ((DEBUG_INFO, "SNP:MAC: Fatal bus error:\n"));
      Mask |= DW_EMAC_DMAGRP_STATUS_FBI_SET_MSK;

      ErrorBit = DW_EMAC_DMAGRP_STATUS_EB_GET (DmaStatus);
      switch (ErrorBit) {
        case RX_DMA_WRITE_DATA_TRANSFER_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Rx Dma write data transfer error\n"));
          break;
        case TX_DMA_READ_DATA_TRANSFER_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Tx Dma read data transfer error\n"));
          break;
        case RX_DMA_DESCRIPTOR_WRITE_ACCESS_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Rx Dma descriptor write access error\n"));
          break;
        case RX_DMA_DESCRIPTOR_READ_ACCESS_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Rx Dma descriptor read access error\n"));
          break;
        case TX_DMA_DESCRIPTOR_WRITE_ACCESS_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Tx Dma descriptor write access error\n"));
          break;
        case TX_DMA_DESCRIPTOR_READ_ACCESS_ERROR:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Tx Dma descriptor read access error\n"));
          break;
        default:
          DEBUG ((DEBUG_INFO, "SNP:MAC: Undefined error\n"));
          break;
      }
    }
  }
  MmioOr32 (MacBaseAddress +
            DW_EMAC_DMAGRP_STATUS_OFST,
            Mask);
}


VOID
EFIAPI
EmacGetStatistic (
  OUT  EFI_NETWORK_STATISTICS   *Statistic,
  IN   UINTN                     MacBaseAddress
  )
{
  EFI_NETWORK_STATISTICS   *Stats;

  DEBUG ((DEBUG_INFO, "SNP:MAC: %a ()\r\n", __func__));

  // Allocate Resources
  Stats = AllocateZeroPool (sizeof (EFI_NETWORK_STATISTICS));
  if (Stats == NULL) {
    return;
  }

  Stats->RxTotalFrames     = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXFRAMECOUNT_GB_OFST);
  Stats->RxUndersizeFrames = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXUNDERSIZE_G_OFST);
  Stats->RxOversizeFrames  = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXOVERSIZE_G_OFST);
  Stats->RxUnicastFrames   = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXUNICASTFRAMES_G_OFST);
  Stats->RxBroadcastFrames = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXBROADCASTFRAMES_G_OFST);
  Stats->RxMulticastFrames = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXMULTICASTFRAMES_G_OFST);
  Stats->RxCrcErrorFrames  = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXCRCERROR_OFST);
  Stats->RxTotalBytes      = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_RXOCTETCOUNT_GB_OFST);
  Stats->RxGoodFrames      = Stats->RxUnicastFrames + Stats->RxBroadcastFrames + Stats->RxMulticastFrames;

  Stats->TxTotalFrames     = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXFRAMECOUNT_GB_OFST);
  Stats->TxGoodFrames      = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXFRAMECOUNT_G_OFST);
  Stats->TxOversizeFrames  = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXOVERSIZE_G_OFST);
  Stats->TxUnicastFrames   = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXUNICASTFRAMES_GB_OFST);
  Stats->TxBroadcastFrames = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXBROADCASTFRAMES_G_OFST);
  Stats->TxMulticastFrames = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXMULTICASTFRAMES_G_OFST);
  Stats->TxTotalBytes      = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXOCTETCOUNT_GB_OFST);
  Stats->Collisions        = MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXLATECOL_OFST) +
                             MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_TXEXESSCOL_OFST);

  // Fill in the statistics
  CopyMem (Statistic, Stats, sizeof (EFI_NETWORK_STATISTICS));
}


VOID
EFIAPI
EmacConfigAdjust (
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
