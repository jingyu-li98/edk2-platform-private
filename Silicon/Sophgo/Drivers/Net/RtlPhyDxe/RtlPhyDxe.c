/** @file
 *
 *  STMMAC Ethernet Driver -- MDIO bus implementation
 *  Provides Bus interface for MII registers.
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include "RtlPhyDxe.h"
#include <Include/Mdio.h>
#include <Include/Phy.h>


#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

STATIC SOPHGO_MDIO_PROTOCOL *Mdio;
/*
 * PHY detect device
 */
EFI_STATUS
EFIAPI
PhyDetectDevice (
  IN PHY_DEVICE   *PhyDev
  )
{
  UINT32       PhyAddr;
  EFI_STATUS   Status;

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: %a ()\r\n",
    __func__
    ));

  for (PhyAddr = 0; PhyAddr < 32; PhyAddr++) {
    Status = PhyReadId (PhyDev);
    if (EFI_ERROR(Status)) {
      continue;
    }

    PhyDev->PhyAddr = PhyAddr;
    return EFI_SUCCESS;
  }

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: Fail to detect Ethernet PHY!\r\n"
    ));

  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PhyRtl8211fConfig (
  IN  PHY_DEVICE   *PhyDev
  )
{
  UINT32      Reg;
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  Status = PhySoftReset (PhyDev);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a[%d]: Software reset failed!\n",
      __func__,
      __LINE__
      ));
    return EFI_DEVICE_ERROR;
  }

  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_BMCR, BMCR_RESET);

  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xd08);

  //
  // enable TX-delay for phy-mode=rgmii-txid/rgimm-id, otherwise disable it
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, 0x11, &Reg);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg |= MIIM_RTL8211F_TX_DELAY;

  Mdio->Write (Mdio, PhyDev->PhyAddr, 0x11, Reg);

  //
  // enable RX-delay for phy-mode=rgmii-id/rgmii-rxid, otherwise disable it
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, 0x15, &Reg);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg &= ~MIIM_RTL8211F_RX_DELAY;

  Mdio->Write (Mdio, PhyDev->PhyAddr, 0x15, Reg);

  //
  // restore to default page 0
  //
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0);

  //
  // Set green LED for Link, yellow LED for Active
  //
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xd04);
  Mdio->Write (Mdio, PhyDev->PhyAddr, 0x10, 0x617f);
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0);

  //
  // Configure AN and Advertise
  //
  PhyAutoNego (PhyDev);
#if 0
  DEBUG ((
    DEBUG_INFO,
    "disable CLKOUT\n"
    ));
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xa43);

  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, 0x19, &Reg);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg &= ~0x1;
  Mdio->Write (Mdio, PhyDev->PhyAddr, 0x19, Reg);

  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0);

  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMCR, &Reg);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg |= BMCR_RESET;
  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_BMCR, Reg);
#endif
  return EFI_SUCCESS;
}

/*
 * Perform PHY software reset
 */
EFI_STATUS
EFIAPI
PhySoftReset (
  IN PHY_DEVICE   *PhyDev
  )
{
  UINT32        TimeOut;
  UINT32        Data32;
  EFI_STATUS    Status;

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: %a ()\r\n",
    __func__
    ));

  //
  // PHY Basic Control Register reset
  //
  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_BMCR, BMCR_RESET);

  //
  // Wait for completion
  //
  TimeOut = 0;
  do {
    //
    // Read MII_BMCR register from PHY
    //
    Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMCR, &Data32);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    //
    // Wait until PHYCTRL_RESET become zero
    //
    if ((Data32 & BMCR_RESET) == 0) {
      break;
    }
    gBS->Stall (1000);
  } while (TimeOut++ < PHY_TIMEOUT);

  if (TimeOut >= PHY_TIMEOUT) {
    DEBUG ((
      DEBUG_INFO,
      "SNP:PHY: ERROR! PhySoftReset timeout\n"
      ));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/*
 * PHY read ID
 */
EFI_STATUS
EFIAPI
PhyReadId (
  IN PHY_DEVICE *PhyDev
  )
{
  UINT32        PhyId1;
  UINT32        PhyId2;
  UINT32        PhyId;
  EFI_STATUS    Status;

  //
  // Grab the bits from PHYIR1, and put them in the upper half
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_PHYSID1, &PhyId1);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  //
  // Grab the bits from PHYIR2, and put them in the upper half
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_PHYSID2, &PhyId2);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  if (PhyId1 == PHY_INVALID_ID || PhyId2 == PHY_INVALID_ID) {
    return EFI_NOT_FOUND;
  }

  PhyId = (PhyId1 & 0xFFFF) << 16 | (PhyId2 & 0xFFFF);

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: Ethernet PHY detected. \
    PHY_ID1=0x%04X, PHY_ID2=0x%04X, PHY_ID=0X%04X, PHY_ADDR=0x%02X\r\n",
    PhyId1,
    PhyId2,
    PhyId,
    PhyDev->PhyAddr
    ));

  return EFI_SUCCESS;
}

/*
 * Do auto-negotiation
 */
EFI_STATUS
EFIAPI
PhyAutoNego (
  IN PHY_DEVICE   *PhyDev
  )
{
  EFI_STATUS    Status;
  UINT32        PhyControl;
  UINT32        PhyStatus;
  UINT32        Features;

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: %a ()\r\n",
    __func__
    ));

  //
  // Read PHY Status
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &PhyStatus);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check PHY Status if auto-negotiation is supported
  //
  if (!(PhyStatus & BMSR_ANEGCAPABLE)) {
    DEBUG ((
      DEBUG_INFO,
      "SNP:PHY: Auto-negotiation is not supported.\n"
      ));
    return EFI_DEVICE_ERROR;
  }

  // 
  // Read PHY Auto-Nego Advertise capabilities register for 10/100 Base-T
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_ADVERTISE, &Features);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set Advertise capabilities for 10Base-T/10Base-T full-duplex/100Base-T/100Base-T full-duplex
  //
  Features |= (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);
  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_ADVERTISE, Features);

  //
  // Configure gigabit if it's supported
  // Read PHY Auto-Nego Advertise capabilities register for 1000 Base-T
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_CTRL1000, &Features);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set Advertise capabilities for 1000 Base-T/1000 Base-T full-duplex
  //
  Features |= (ADVERTISE_1000FULL | ADVERTISE_1000HALF);
  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_CTRL1000, Features);

  //
  // Enable and Restart Autonegotiation: read control register
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMCR, &PhyControl);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Enable & restart Auto-Negotiation
  //
  PhyControl |= (BMCR_ANENABLE | BMCR_ANRESTART);

  //
  // Don't isolate the PHY if we're negotiating
  //
  PhyControl &= ~(BMCR_ISOLATE);
  
  Mdio->Write (Mdio, PhyDev->PhyAddr, MII_BMCR, PhyControl);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Rtl8211fStartUp (
  IN PHY_DEVICE   *PhyDev
  )
{
  UINT32       Speed;
  UINT32       Duplex;
  EFI_STATUS   Status;

  //
  // Set the baseline so we only have to set them if they're different
  //
  Speed = SPEED_10;
  Duplex = DUPLEX_HALF;

  //
  // Read the Status (2x to make sure link is right)
  //
  Status = GenPhyUpdateLink (PhyDev);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Rtl8211xParseStatus (PhyDev, &Speed, &Duplex);
}

EFI_STATUS
EFIAPI
Rtl8211xParseStatus (
  IN PHY_DEVICE   *PhyDev,
  IN UINT32       *Speed,
  IN UINT32       *Duplex
  )
{
  UINT32     MiiReg;
  UINT32     Index;
  EFI_STATUS Status;

  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MIIM_RTL8211x_PHY_STATUS, &MiiReg);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!(MiiReg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
    Index = 0;

    //
    // In case of timeout ->link is cleared
    //
    PhyDev->CurrentLink = LINK_UP;
    DEBUG ((
      DEBUG_INFO,
      "Waiting for PHY realtime link\n"
      ));

    while (!(MiiReg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
      //
      // Timeout reached ?
      //
      if (Index > PHY_AUTONEGOTIATE_TIMEOUT) {
        DEBUG ((
          DEBUG_WARN,
          "TIMEOUT!\n"
          ));
        PhyDev->CurrentLink = LINK_DOWN;
        break;
      }
    }

    if ((Index ++ % 1000) == 0) {
      DEBUG ((
        DEBUG_INFO,
        "done\n"
        ));
    }
    gBS->Stall (1000);   /* 1 ms */
    Mdio->Read (Mdio, PhyDev->PhyAddr, MIIM_RTL8211x_PHY_STATUS, &MiiReg);

    DEBUG ((
      DEBUG_INFO,
      "done\n"
      ));
    gBS->Stall (500000); /* another 500 ms (results in faster booting) */
  } else {
    if (MiiReg & MIIM_RTL8211x_PHYSTAT_LINK) {
       PhyDev->CurrentLink = LINK_UP;
    } else {
       PhyDev->CurrentLink = LINK_DOWN;
    }
  }

  if (MiiReg & MIIM_RTL8211x_PHYSTAT_DUPLEX)
    *Duplex = DUPLEX_FULL;
  else
    *Duplex = DUPLEX_HALF;

  switch (MiiReg & MIIM_RTL8211x_PHYSTAT_SPEED) {
  case MIIM_RTL8211x_PHYSTAT_GBIT:
    *Speed = SPEED_1000;
    break;
  case MIIM_RTL8211x_PHYSTAT_100:
    *Speed = SPEED_100;
    break;
  default:
    *Speed = SPEED_10;
  }

  PhyDisplayAbility (*Speed, *Duplex);

  return EFI_SUCCESS;
}

/*
 * Updata Link Status.
 */
EFI_STATUS
EFIAPI
GenPhyUpdateLink (
  IN PHY_DEVICE   *PhyDev
  )
{
  UINT32     MiiReg;
  UINT32     Index;
  EFI_STATUS Status;

  //
  // Wait if the link is up, and autonegotiation is in progress
  // (ie - we're capable and it's not done)
  //
  Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &MiiReg);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // If we already saw the link up, and it hasn't gone down, then
  // we don't need to wait for autoneg again
  //
  if (PhyDev->CurrentLink && MiiReg & BMSR_LSTATUS) {
    return EFI_SUCCESS;
  }

  if (!(MiiReg & BMSR_ANEGCOMPLETE)) {
    Index = 0;

    DEBUG ((
      DEBUG_INFO,
      "%s Waiting for PHY auto negotiation to complete\n",
      __func__
      ));

    while (!(MiiReg & BMSR_ANEGCOMPLETE)) {
     //
     // Timeout reached ?
     //
     if (Index > (PHY_ANEG_TIMEOUT / 50)) {
       DEBUG ((
         DEBUG_INFO,
	 " TIMEOUT !\n"
	 ));
       PhyDev->CurrentLink = LINK_DOWN;
       return EFI_TIMEOUT;
     }


     if ((Index ++ % 10) == 0) {
       DEBUG ((
         DEBUG_INFO,
	 " TIMEOUT !\n"
	 ));
      }

      Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &MiiReg);
      gBS->Stall (50000);     /* 50 ms */
    }
       DEBUG ((
         DEBUG_INFO,
	 " done\n"
	 ));
       PhyDev->CurrentLink = LINK_UP;
     } else {
       // 
       // Read the link a second time to clear the latched state
       //
       Status = Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &MiiReg);

       if (MiiReg & BMSR_LSTATUS) {
         PhyDev->CurrentLink = LINK_UP;
       } else {
         PhyDev->CurrentLink = LINK_DOWN;
       }
  }

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

/*
 * Read RTL8211F PHY Extended Registers.
 */
UINT32
EFIAPI
PhyRtl8211fExtendedRead (
  IN PHY_DEVICE   *PhyDev,
  IN UINT32       DevAddr,
  IN UINT32       RegNum
  )
{
  UINT32         OldPage;
  UINT32         Value;

  OldPage = 0;
  Value = 0;

  Mdio->Read (Mdio, PhyDev->PhyAddr, OldPage, (UINT32 *)MIIM_RTL8211F_PAGE_SELECT);

  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, DevAddr);
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, OldPage);

  Mdio->Read (Mdio, PhyDev->PhyAddr, Value, &RegNum);

  return Value;
}

/*
 * Write RTL8211F PHY Extended Registers.
 */
EFI_STATUS
EFIAPI
PhyRtl8211fExtendedWrite (
  IN PHY_DEVICE   *PhyDev,
  IN UINT32       Mode,
  IN UINT32       DevAddr,
  IN UINT32       RegNum,
  IN UINT16       Val
  )
{
  UINT32 OldPage;

  Mdio->Read (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, &OldPage);

  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, DevAddr);
  Mdio->Write (Mdio, PhyDev->PhyAddr, RegNum, Val);
  Mdio->Write (Mdio, PhyDev->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, OldPage);

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl8211fPhyInit (
  IN CONST SOPHGO_PHY_PROTOCOL *Snp,
  IN PHY_DEVICE                *PhyDev
  )
{
  EFI_STATUS Status;

  DEBUG ((
    DEBUG_INFO,
    "SNP:PHY: %a ()\r\n",
    __func__
    ));

  //
  // Initialize the phyaddr
  //
  PhyDev->PhyAddr = 0;
  PhyDev->CurrentLink = LINK_DOWN;
  PhyDev->PhyOldLink = LINK_DOWN;

  Status = PhyDetectDevice (PhyDev);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  PhyRtl8211fConfig (PhyDev);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "RTL8211F PHY config failed: %r\n",
      Status
      ));
    return Status;
  }

  Rtl8211fStartUp (PhyDev);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "RTL8211F PHY startup failed: %r\n",
      Status
      ));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl8211fPhyStatus (
  IN CONST SOPHGO_PHY_PROTOCOL *This,
  IN PHY_DEVICE                 *PhyDev
  )
{
  UINT32 Data;

  Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &Data);
  Mdio->Read (Mdio, PhyDev->PhyAddr, MII_BMSR, &Data);

  if ((Data & BMSR_LSTATUS) == 0) {
    PhyDev->LinkUp = FALSE;
  } else {
    PhyDev->LinkUp = TRUE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Rtl8211fPhyDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  SOPHGO_PHY_PROTOCOL *Phy;
  EFI_STATUS          Status;
  EFI_HANDLE          Handle = NULL;

  Phy = AllocateZeroPool (sizeof (SOPHGO_PHY_PROTOCOL));
  Phy->Status = Rtl8211fPhyStatus;
  Phy->Init = (SOPHGO_PHY_INIT)Rtl8211fPhyInit;

  Status = gBS->InstallMultipleProtocolInterfaces (
		  &Handle,
		  &gSophgoPhyProtocolGuid,
		  Phy,
		  NULL
		  );

  if (EFI_ERROR(Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to install interfaces.\n"
      ));

    return Status;
  }

  DEBUG ((
    DEBUG_INFO,
    "Succesfully installed protocol interfaces.\n"
    ));

  return EFI_SUCCESS;
}
