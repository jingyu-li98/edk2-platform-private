/** @file

  Copyright (c) 2011 - 2019, Intel Corporaton. All rights reserved.
  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#include "PhyDxeUtil.h"
#include "EmacDxeUtil.h"

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>

#include "RtlPhyDxe.h"

#define TIMEOUT   500

STATIC SOPHGO_MDIO_PROTOCOL *Mdio;

EFI_STATUS
EFIAPI
PhyDxeInitialization (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  EFI_STATUS   Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  // initialize the phyaddr
  PhyDriver->PhyAddr = 0;
  PhyDriver->PhyCurrentLink = LINK_DOWN;
  PhyDriver->PhyOldLink = LINK_DOWN;

  Status = PhyDetectDevice (PhyDriver, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  PhyConfig (PhyDriver, MacBaseAddress);

  return EFI_SUCCESS;
}


// PHY detect device
EFI_STATUS
EFIAPI
PhyDetectDevice (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  UINT32       PhyAddr;
  EFI_STATUS   Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  for (PhyAddr = 0; PhyAddr < 32; PhyAddr++) {
    Status = PhyReadId (PhyAddr, MacBaseAddress);
    if (EFI_ERROR(Status)) {
      continue;
    }

    PhyDriver->PhyAddr = PhyAddr;
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "SNP:PHY: Fail to detect Ethernet PHY!\r\n"));
  return EFI_NOT_FOUND;

}


EFI_STATUS
EFIAPI
Rtl8122fPhyConfig (
  IN  PHY_DRIVER   *PhyDriver,
  IN  UINTN        MacBaseAddress
  //IN PHY_DEVICE *PhyDev
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  Status = PhySoftReset (PhyDriver, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  // Configure TX/RX Skew
  PhyConfigSkew (PhyDriver, MacBaseAddress);

  // Read back and display Skew settings
  PhyDisplayConfigSkew (PhyDriver, MacBaseAddress);

  // Configure AN FLP Burst Trasmit timing interval
  PhyConfigFlpBurstTiming (PhyDriver, MacBaseAddress);
  PhyDisplayFlpBurstTiming (PhyDriver, MacBaseAddress);

  // Configure AN and Advertise
  PhyAutoNego (PhyDriver, MacBaseAddress);
  UINTN Reg;

  if (phydev->flags & PHY_RTL8211F_FORCE_EEE_RXC_ON) {
    UINT32 Reg;

    Reg = Mdio->Read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
    Reg &= ~MDIO_PCS_CTRL1_CLKSTOP_EN;
    Mdio->Write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, reg);
  }

  write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0xd08);
  Mdio->Read(phydev, MDIO_DEVAD_NONE, 0x11);

  //
  // enable TX-delay for rgmii-id and rgmii-txid, otherwise disable it */
  //
  if (PhyDev->Interface == PHY_INTERFACE_MODE_RGMII_ID ||
            PhyDev->Interface == PHY_INTERFACE_MODE_RGMII_TXID)
    Reg |= MIIM_RTL8211F_TX_DELAY;
  else
    Reg &= ~MIIM_RTL8211F_TX_DELAY;

  Mdio->Write(phydev, MDIO_DEVAD_NONE, 0x11, reg);

  //
  // enable RX-delay for rgmii-id and rgmii-rxid, otherwise disable it */
  //
  Reg = Mdio->Read(phydev, MDIO_DEVAD_NONE, 0x15);
  if (PhyDev->Interface == PHY_INTERFACE_MODE_RGMII_ID ||
            PhyDev->Interface == PHY_INTERFACE_MODE_RGMII_RXID)
    Reg |= MIIM_RTL8211F_RX_DELAY;
  else
    Reg &= ~MIIM_RTL8211F_RX_DELAY;

  Mdio->Write(phydev, MDIO_DEVAD_NONE, 0x15, reg);

  //
  // restore to default page 0
  //
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0x0);

  //
  // Set green LED for Link, yellow LED for Active
  //
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0xd04);
  Mdio->Write(phydev, MDIO_DEVAD_NONE, 0x10, 0x617f);
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0x0);

        genphy_config_aneg(phydev);

  DEBUG ((DEBUG_INFO, "disable CLKOUT\n"));
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0xa43);
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  0x19, Mdio->Read(phydev, MDIO_DEVAD_NONE, 0x19) & ~0x1);
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MIIM_RTL8211F_PAGE_SELECT, 0x0);
  Mdio->Write(phydev, MDIO_DEVAD_NONE,
                  MII_BMCR, Mdio->Read(phydev, MDIO_DEVAD_NONE, MII_BMCR) | BMCR_RESET);
#endif

        return 0;

  return EFI_SUCCESS;
}


EFI_STATUS
RtlPhyInit (
  IN CONST SOPHGO_PHY_PROTOCOL  *Snp,
  IN UINT32                     PhyIndex,
  IN PHY_CONNECTION             PhyConnection,
  IN OUT PHY_DEVICE             **OutPhyDev
  )
{
  UINT8       *DeviceIds;
  UINT8       MdioIndex;
  UINT8       PhyId;
  EFI_STATUS  Status;
  PHY_DEVICE  *PhyDev;

  Status = gBS->LocateProtocol (
      &gSophgoMdioProtocolGuid,
      NULL,
      (VOID **) &Mdio
      );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MdioIndex = Phy2MdioController[PhyIndex];

  //
  // Verify correctness of PHY <-> MDIO assignment
  //
  if ((MdioDeviceTable[MdioIndex] == 0) ||
      (MdioIndex >= Mdio->ControllerCount)) {
    DEBUG ((
      DEBUG_ERROR,
      "RtlPhyDxe: Incorrect Mdio controller assignment for PHY#%d",
      PhyIndex
      ));

    return EFI_INVALID_PARAMETER;
  }

  DeviceIds = PcdGetPtr (PcdPhyDeviceIds);
  PhyId = DeviceIds[PhyIndex];
  if (PhyId >= MV_PHY_DEVICE_ID_MAX) {
    DEBUG ((
      DEBUG_ERROR,
      "%a, Incorrect PHY ID (0x%x) for PHY#%d\n",
      __func__,
      PhyId,
      PhyIndex
      ));

    return EFI_INVALID_PARAMETER;
  }

  //
  // perform setup common for all PHYs
  //
  PhyDev = AllocateZeroPool (sizeof (PHY_DEVICE));
  PhyDev->Addr = PhySmiAddresses[PhyIndex];
  PhyDev->Connection = PhyConnection;
  PhyDev->MdioIndex = MdioIndex;
  DEBUG ((
    DEBUG_INFO,
    "MvPhyDxe: MdioIndex is %d, PhyAddr is %d, connection %d\n",
    PhyDev->MdioIndex,
    PhyDev->Addr,
    PhyConnection
    ));

  *OutPhyDev = PhyDev;

  return MvPhyDevices[PhyId].DevInit (Snp, PhyDev);
}
/*
  Perform PHY software reset

  */
EFI_STATUS
EFIAPI
PhySoftReset (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  UINT32        TimeOut;
  UINT32        Data32;
  EFI_STATUS    Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  //
  // PHY Basic Control Register reset
  //
  PhyWrite (PhyDriver->PhyAddr, MII_BMCR, BMCR_RESET, MacBaseAddress);

  //
  // Wait for completion
  //
  TimeOut = 0;
  do {
    // Read PHY_BASIC_CTRL register from PHY
    Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_CTRL, &Data32, MacBaseAddress);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    // Wait until PHYCTRL_RESET become zero
    if ((Data32 & PHYCTRL_RESET) == 0) {
      break;
    }
    MicroSecondDelay(1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: ERROR! PhySoftReset timeout\n"));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}


// PHY read ID
EFI_STATUS
EFIAPI
PhyReadId (
  IN UINT32   PhyAddr,
  IN UINTN    MacBaseAddress
  )
{
  UINT32        PhyId1;
  UINT32        PhyId2;
  EFI_STATUS    Status;

  Status = PhyRead (PhyAddr, PHY_ID1, &PhyId1, MacBaseAddress);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  Status = PhyRead (PhyAddr, PHY_ID2, &PhyId2, MacBaseAddress);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  if (PhyId1 == PHY_INVALID_ID || PhyId2 == PHY_INVALID_ID) {
    return EFI_NOT_FOUND;
  }

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: Ethernet PHY detected. \
    PHY_ID1=0x%04X, PHY_ID2=0x%04X, PHY_ADDR=0x%02X\r\n",
    PhyId1,
    PhyId2,
    PhyAddr
    ));

  return EFI_SUCCESS;
}


VOID
EFIAPI
PhyConfigSkew (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_CONTROL_PAD_SKEW_REG,
                        PHY_KSZ9031RN_CONTROL_PAD_SKEW_VALUE,
                        MacBaseAddress);
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_CLK_PAD_SKEW_REG,
                        PHY_KSZ9031RN_CLK_PAD_SKEW_VALUE,
                        MacBaseAddress);
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_DEV_ADDR,
                        PHY_KSZ9031RN_RX_DATA_PAD_SKEW_REG,
                        PHY_KSZ9031RN_RX_DATA_PAD_SKEW_VALUE,
                        MacBaseAddress);
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_DEV_ADDR,
                        PHY_KSZ9031RN_TX_DATA_PAD_SKEW_REG,
                        PHY_KSZ9031RN_TX_DATA_PAD_SKEW_VALUE,
                        MacBaseAddress);
}


VOID
EFIAPI
PhyDisplayConfigSkew (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  // Display skew configuration
  DEBUG ((DEBUG_INFO, "SNP:PHY: Control Signal Pad Skew = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC,
          PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_CONTROL_PAD_SKEW_REG, MacBaseAddress)));

  DEBUG ((DEBUG_INFO, "SNP:PHY: RGMII Clock Pad Skew    = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC,
          PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_CLK_PAD_SKEW_REG, MacBaseAddress)));

  DEBUG ((DEBUG_INFO, "SNP:PHY: RGMII RX Data Pad Skew  = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC,
          PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_RX_DATA_PAD_SKEW_REG, MacBaseAddress)));

  DEBUG ((DEBUG_INFO, "SNP:PHY: RGMII TX Data Pad Skew  = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC,
          PHY_KSZ9031RN_DEV_ADDR, PHY_KSZ9031RN_TX_DATA_PAD_SKEW_REG, MacBaseAddress)));
}

VOID
EFIAPI
PhyConfigFlpBurstTiming (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_MMD_DEV_ADDR_00,
                        PHY_KSZ9031RN_MMD_D0_FLP_LO_REG,
                        PHY_KSZ9031RN_MMD_D0_FLP_16MS_LO,
                        MacBaseAddress);
  Phy9031ExtendedWrite (PhyDriver,
                        PHY_KSZ9031_MOD_DATA_NO_POST_INC,
                        PHY_KSZ9031RN_MMD_DEV_ADDR_00,
                        PHY_KSZ9031RN_MMD_D0_FLP_HI_REG,
                        PHY_KSZ9031RN_MMD_D0_FLP_16MS_HI,
                        MacBaseAddress);
}

VOID
EFIAPI
PhyDisplayFlpBurstTiming (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  // Display Auto-Negotiation FLP burst transmit timing
  DEBUG ((DEBUG_INFO, "SNP:PHY: AN FLP Burst Transmit - LO = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC,
          PHY_KSZ9031RN_MMD_DEV_ADDR_00, PHY_KSZ9031RN_MMD_D0_FLP_LO_REG, MacBaseAddress)));
  DEBUG ((DEBUG_INFO, "SNP:PHY: AN FLP Burst Transmit - HI = 0x%04X\r\n",
          Phy9031ExtendedRead (PhyDriver, PHY_KSZ9031_MOD_DATA_NO_POST_INC, PHY_KSZ9031RN_MMD_DEV_ADDR_00,
          PHY_KSZ9031RN_MMD_D0_FLP_HI_REG, MacBaseAddress)));
}

// Do auto-negotiation
EFI_STATUS
EFIAPI
PhyAutoNego (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  UINT32        PhyControl;
  UINT32        PhyStatus;
  UINT32        Features;
  EFI_STATUS    Status;

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: %a ()\r\n",
    __func__
    ));

  //
  // Read PHY Status
  //
  Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_STATUS, &PhyStatus, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Check PHY Status if auto-negotiation is supported
  if ((PhyStatus & PHYSTS_AUTO_CAP) == 0) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: Auto-negotiation is not supported.\n"));
    return EFI_DEVICE_ERROR;
  }

  // Read PHY Auto-Nego Advertise capabilities register for 10/100 Base-T
  Status = PhyRead (PhyDriver->PhyAddr, PHY_AUTO_NEG_ADVERT, &Features, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Set Advertise capabilities for 10Base-T/10Base-T full-duplex/100Base-T/100Base-T full-duplex
  Features |= (PHYANA_10BASET | PHYANA_10BASETFD | PHYANA_100BASETX | PHYANA_100BASETXFD);
  PhyWrite (PhyDriver->PhyAddr, PHY_AUTO_NEG_ADVERT, Features, MacBaseAddress);

  // Read PHY Auto-Nego Advertise capabilities register for 1000 Base-T
  Status = PhyRead (PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL, &Features, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Set Advertise capabilities for 1000 Base-T/1000 Base-T full-duplex
  Features |= (PHYADVERTISE_1000FULL | PHYADVERTISE_1000HALF);
  PhyWrite (PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL, Features, MacBaseAddress);

  // Read control register
  Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_CTRL, &PhyControl, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  // Enable Auto-Negotiation
  PhyControl |= PHYCTRL_AUTO_EN;
  // Restart auto-negotiation
  PhyControl |= PHYCTRL_RST_AUTO;
  // Write this configuration
  PhyWrite (PhyDriver->PhyAddr, PHY_BASIC_CTRL, PhyControl, MacBaseAddress);

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PhyLinkAdjustEmacConfig (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  UINT32       Speed;
  UINT32       Duplex;
  EFI_STATUS   Status;

  Status = EFI_SUCCESS;
  Speed = SPEED_10;
  Duplex = DUPLEX_HALF;

  Status = PhyCheckLinkStatus (PhyDriver, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    PhyDriver->PhyCurrentLink = LINK_DOWN;
  } else {
    PhyDriver->PhyCurrentLink = LINK_UP;
  }

  if (PhyDriver->PhyOldLink != PhyDriver->PhyCurrentLink) {
    if (PhyDriver->PhyCurrentLink == LINK_UP) {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Link is up - Network Cable is Plugged\r\n"));
      PhyReadCapability (PhyDriver, &Speed, &Duplex, MacBaseAddress);
      EmacConfigAdjust (Speed, Duplex, MacBaseAddress);
      Status = EFI_SUCCESS;
    } else {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Link is Down - Network Cable is Unplugged?\r\n"));
      Status = EFI_NOT_READY;
    }
  } else if (PhyDriver->PhyCurrentLink == LINK_DOWN) {
    Status = EFI_NOT_READY;
  }

  PhyDriver->PhyOldLink = PhyDriver->PhyCurrentLink;

  return Status;
}


EFI_STATUS
EFIAPI
PhyCheckLinkStatus (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  EFI_STATUS    Status;
  UINT32        Data32;
  UINTN         TimeOut;
  UINT32        PhyBasicStatus;

  // Get the PHY Status
  Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_STATUS, &PhyBasicStatus, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // if Link is already up then dont need to proceed anymore
  if (PhyBasicStatus & PHYSTS_LINK_STS) {
    return EFI_SUCCESS;
  }

  // Wait until it is up or until Time Out
  TimeOut = 0;
  do {
    // Read PHY_BASIC_STATUS register from PHY
    Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_STATUS, &Data32, MacBaseAddress);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    // Wait until PHYSTS_LINK_STS become one
    if (Data32 & PHYSTS_LINK_STS) {
      // Link is up
      break;
    }
    MicroSecondDelay (1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    // Link is down
    return EFI_TIMEOUT;
  }

  // Wait until autonego process has completed
  TimeOut = 0;
  do {
    // Read PHY_BASIC_STATUS register from PHY
    Status = PhyRead (PhyDriver->PhyAddr, PHY_BASIC_STATUS, &Data32, MacBaseAddress);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    // Wait until PHYSTS_AUTO_COMP become one
    if (Data32 & PHYSTS_AUTO_COMP) {
      DEBUG ((DEBUG_INFO, "SNP:PHY: Auto Negotiation completed\r\n"));
      break;
    }
    MicroSecondDelay (1);
  } while (TimeOut++ < PHY_TIMEOUT);
  if (TimeOut >= PHY_TIMEOUT) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: Error! Auto Negotiation timeout\n"));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PhyReadCapability (
  IN PHY_DRIVER   *PhyDriver,
  IN UINT32       *Speed,
  IN UINT32       *Duplex,
  IN UINTN        MacBaseAddress
  )
{
  EFI_STATUS    Status;
  UINT32        PartnerAbilityGb;
  UINT32        AdvertisingGb;
  UINT32        CommonAbilityGb;
  UINT32        PartnerAbility;
  UINT32        Advertising;
  UINT32        CommonAbility;

  // For 1000 Base-T

  Status = PhyRead (PhyDriver->PhyAddr, PHY_1000BASE_T_STATUS, &PartnerAbilityGb, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PhyRead (PhyDriver->PhyAddr, PHY_1000BASE_T_CONTROL, &AdvertisingGb, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CommonAbilityGb = PartnerAbilityGb & (AdvertisingGb << 2);

  // For 10/100 Base-T

  Status = PhyRead (PhyDriver->PhyAddr, PHY_AUTO_NEG_LINK_ABILITY, &PartnerAbility, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PhyRead (PhyDriver->PhyAddr, PHY_AUTO_NEG_EXP, &Advertising, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CommonAbility = PartnerAbility & Advertising;

  // Determine the Speed and Duplex
  if (PartnerAbilityGb & (PHYLPA_1000FULL | PHYLPA_1000HALF)) {
    *Speed = SPEED_1000;
    if (CommonAbilityGb & PHYLPA_1000FULL) {
      *Duplex = DUPLEX_FULL;
    }
  } else if (CommonAbility & (PHYLPA_100FULL | PHYLPA_100HALF)) {
    *Speed = SPEED_100;
    if (CommonAbility & PHYLPA_100FULL) {
      *Duplex = DUPLEX_FULL;
    } else if (CommonAbility & PHYLPA_10FULL) {
      *Duplex = DUPLEX_FULL;
    }
  }

  PhyDisplayAbility (*Speed, *Duplex);

  return EFI_SUCCESS;
}


VOID
EFIAPI
PhyDisplayAbility (
  IN UINT32   Speed,
  IN UINT32   Duplex
  )
{

  DEBUG ((DEBUG_INFO, "SNP:PHY: "));
  switch (Speed) {
    case SPEED_1000:
      DEBUG ((DEBUG_INFO, "1 Gbps - "));
      break;
    case SPEED_100:
      DEBUG ((DEBUG_INFO, "100 Mbps - "));
      break;
    case SPEED_10:
      DEBUG ((DEBUG_INFO, "10 Mbps - "));
      break;
    default:
      DEBUG ((DEBUG_INFO, "Invalid link speed"));
      break;
    }

  switch (Duplex) {
    case DUPLEX_FULL:
      DEBUG ((DEBUG_INFO, "Full Duplex\n"));
      break;
    case DUPLEX_HALF:
      DEBUG ((DEBUG_INFO, "Half Duplex\n"));
      break;
    default:
      DEBUG ((DEBUG_INFO, "Invalid duplex mode\n"));
      break;
    }
}


// Function to read from MII register (PHY Access)
EFI_STATUS
EFIAPI
PhyRead (
  IN  UINT32   Addr,
  IN  UINT32   Reg,
  OUT UINT32   *Data,
  IN  UINTN    MacBaseAddress
  )
{
  UINT32        MiiConfig;
  UINT32        Count;

  // Check it is a valid Reg
  ASSERT (Reg < 31);

  MiiConfig = ((Addr << MIIADDRSHIFT) & MII_ADDRMSK) |
              ((Reg << MIIREGSHIFT) & MII_REGMSK)|
               MII_CLKRANGE_150_250M |
               MII_BUSY;

  // write this config to register
  MmioWrite32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_ADDRESS_OFST, MiiConfig);

  // Wait for busy bit to clear
  Count = 0;
  while (Count < 10000) {
    if (!(DW_EMAC_GMACGRP_GMII_ADDRESS_GB_GET (MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_ADDRESS_OFST)))) {
      *Data = DW_EMAC_GMACGRP_GMII_DATA_GD_GET (MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_DATA_OFST));
      return EFI_SUCCESS;
    }
    MemoryFence ();
    Count++;
  };
  DEBUG ((DEBUG_INFO, "SNP:PHY: MDIO busy bit timeout\r\n"));
  return EFI_TIMEOUT;
}


// Function to write to the MII register (PHY Access)
EFI_STATUS
EFIAPI
PhyWrite (
  IN UINT32   Addr,
  IN UINT32   Reg,
  IN UINT32   Data,
  IN UINTN    MacBaseAddress
  )
{
  UINT32   MiiConfig;
  UINT32   Count;

  // Check it is a valid Reg
  ASSERT(Reg < 31);

  MiiConfig = ((Addr << MIIADDRSHIFT) & MII_ADDRMSK) |
              ((Reg << MIIREGSHIFT) & MII_REGMSK)|
               MII_WRITE |
               MII_CLKRANGE_150_250M |
               MII_BUSY;
  // Write the desired value to the register first
  MmioWrite32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_DATA_OFST, (Data & 0xFFFF));

  // write this config to register
  MmioWrite32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_ADDRESS_OFST, MiiConfig);

  // Wait for busy bit to clear
  Count = 0;
  while (Count < 1000) {
    if (!(DW_EMAC_GMACGRP_GMII_ADDRESS_GB_GET (MmioRead32 (MacBaseAddress + DW_EMAC_GMACGRP_GMII_ADDRESS_OFST)))) {
      return EFI_SUCCESS;
    }
    MemoryFence ();
    Count++;
  };

  return EFI_TIMEOUT;
}


EFI_STATUS
EFIAPI
Phy9031ExtendedWrite (
  IN PHY_DRIVER   *PhyDriver,
  IN UINT32       Mode,
  IN UINT32       DevAddr,
  IN UINT32       Regnum,
  IN UINT16       Val,
  IN UINTN        MacBaseAddress
  )
{
  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_CTRL_REG, DevAddr, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_REGDATA_REG, Regnum, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_CTRL_REG, (Mode << 14) | DevAddr, MacBaseAddress);
  return PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_REGDATA_REG, Val, MacBaseAddress);
}


UINT32
EFIAPI
Phy9031ExtendedRead (
  IN PHY_DRIVER   *PhyDriver,
  IN UINT32       Mode,
  IN UINT32       DevAddr,
  IN UINT32       Regnum,
  IN UINTN        MacBaseAddress
  )
{
  EFI_STATUS    Status;
  UINT32        Data32;

  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_CTRL_REG, DevAddr, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_REGDATA_REG, Regnum, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_CTRL_REG, (Mode << 14) | DevAddr, MacBaseAddress);

  Status = PhyRead (PhyDriver->PhyAddr, PHY_KSZ9031RN_MMD_REGDATA_REG, &Data32, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  return Data32;
}


