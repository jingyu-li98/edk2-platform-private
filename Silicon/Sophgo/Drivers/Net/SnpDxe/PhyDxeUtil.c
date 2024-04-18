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

#include "PhyDxeUtil.h"
#include "EmacDxeUtil.h"

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>

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


/*
 * PHY detect device
 */
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
PhyRtl8211fConfig (
  IN  PHY_DRIVER   *PhyDriver,
  IN  UINTN        MacBaseAddress
  )
{
  UINT16      Reg;
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "SNP:PHY: %a ()\r\n", __func__));

  Status = PhySoftReset (PhyDriver, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  PhyWrite (PhyDriver->PhyAddr, MII_BMCR, BMCR_RESET, MacBaseAddress);

  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xd08, MacBaseAddress);

  //
  // enable TX-delay for phy-mode=rgmii-txid/rgimm-id, otherwise disable it
  //
  Status = PhyRead (PhyDriver->PhyAddr, 0x11, &Reg, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg |= MIIM_RTL8211F_TX_DELAY;

  PhyWrite (PhyDriver->PhyAddr, 0x11, Reg, MacBaseAddress);

  //
  // enable RX-delay for phy-mode=rgmii-id/rgmii-rxid, otherwise disable it
  //
  Status = PhyRead (PhyDriver->PhyAddr, 0x15, &Reg, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg &= ~MIIM_RTL8211F_RX_DELAY;

  PhyWrite (PhyDriver->PhyAddr, 0x15, Reg, MacBaseAddress);

  //
  // restore to default page 0
  //
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0, MacBaseAddress);

  //
  // Set green LED for Link, yellow LED for Active
  //
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xd04, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, 0x10, 0x617f, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0, MacBaseAddress);

  //
  // Configure AN and Advertise
  //
  PhyAutoNego (PhyDriver, MacBaseAddress);
#if 0
  DEBUG ((
    DEBUG_INFO,
    "disable CLKOUT\n"
    ));
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0xa43, MacBaseAddress);

  Status = PhyRead (PhyDriver->PhyAddr, 0x19, &Reg, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg &= ~0x1;
  PhyWrite (PhyDriver->PhyAddr, 0x19, Reg, MacBaseAddresses);

  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, 0x0, MacBaseAddress);

  Status = PhyRead (PhyDriver->PhyAddr, MII_BMCR, &Reg, MacBaseAddresses);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  Reg |= BMCR_RESET;
  PhyWrite (PhyDriver->PhyAddr, MII_BMCR, Reg);
#endif
  return EFI_SUCCESS;
}


// Perform PHY software reset
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

  // PHY Basic Control Register reset
  PhyWrite (PhyDriver->PhyAddr, MII_BMCR, PHYCTRL_RESET, MacBaseAddress);

  // Wait for completion
  TimeOut = 0;
  do {
    // Read MII_BMCR register from PHY
    Status = PhyRead (PhyDriver->PhyAddr, MII_BMCR, &Data32, MacBaseAddress);
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

/*
 * PHY read ID
 */
EFI_STATUS
EFIAPI
PhyReadId (
  IN UINT32   PhyAddr,
  IN UINTN    MacBaseAddress
  )
{
  UINT32        PhyId1;
  UINT32        PhyId2;
  UINT32        PhyId;
  EFI_STATUS    Status;

  //
  // Grab the bits from PHYIR1, and put them in the upper half
  //
  Status = PhyRead (PhyAddr, MII_PHYSID1, &PhyId1, MacBaseAddress);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  //
  // Grab the bits from PHYIR2, and put them in the upper half
  //
  Status = PhyRead (PhyAddr, MII_PHYSID2, &PhyId2, MacBaseAddress);
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
    PhyAddr
    ));

  return EFI_SUCCESS;
}

/*
 * Do auto-negotiation
 */
EFI_STATUS
EFIAPI
PhyAutoNego (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
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
  Status = PhyRead (PhyDriver->PhyAddr, MII_BMSR, &PhyStatus, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check PHY Status if auto-negotiation is supported
  //
  if (!(PhyStatus & BMSR_ANEGCAPABLE)) {
    DEBUG ((DEBUG_INFO, "SNP:PHY: Auto-negotiation is not supported.\n"));
    return EFI_DEVICE_ERROR;
  }

  // 
  // Read PHY Auto-Nego Advertise capabilities register for 10/100 Base-T
  //
  Status = PhyRead (PhyDriver->PhyAddr, MII_ADVERTISE, &Features, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set Advertise capabilities for 10Base-T/10Base-T full-duplex/100Base-T/100Base-T full-duplex
  //
  Features |= (ADVERTISE_10HALF | PHYANA_10FULL | PHYANA_100HALF | PHYANA_100FULL);
  PhyWrite (PhyDriver->PhyAddr, MII_ADVERTISE, Features, MacBaseAddress);

  //
  // Configure gigabit if it's supported
  // Read PHY Auto-Nego Advertise capabilities register for 1000 Base-T
  //
  Status = PhyRead (PhyDriver->PhyAddr, MII_CTRL1000, &Features, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set Advertise capabilities for 1000 Base-T/1000 Base-T full-duplex
  //
  Features |= (ADVERTISE_1000FULL | ADVERTISE_1000HALF);
  PhyWrite (PhyDriver->PhyAddr, MII_CTRL1000, Features, MacBaseAddress);

  //
  // Enable and Restart Autonegotiation: read control register
  //
  Status = PhyRead (PhyDriver->PhyAddr, MII_BMCR, &PhyControl, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* Enable & restart Auto-Negotiation */
  PhyControl |= (BMCR_ANENABLE | BMCR_ANRESTART);

  /* Don't isolate the PHY if we're negotiating */
  PhyControl &= ~(BMCR_ISOLATE);
  
  PhyWrite (PhyDriver->PhyAddr, MII_BMCR, PhyControl, MacBaseAddress);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Rtl8211fStartUp (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
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
  Status = GenPhyUpdateLink (PhyDriver, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Rtl8211xParseStatus (PhyDriver, MacBaseAddress, &Speed, &Duplex);
}

EFI_STATUS
EFIAPI
Rtl8211xParseStatus (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress,
  IN UINT32       *Speed,
  IN UINT32       *Duplex
  )
{
  UINT32     MiiReg;
  UINT32     Index;
  EFI_STATUS Status;

  Status = PhyRead (PhyDriver->PhyAddr, MIIM_RTL8211x_PHY_STATUS, &MiiReg, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!(MiiReg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
    Index = 0;

    /* in case of timeout ->link is cleared */
    PhyDriver->CurrentLink = LINK_UP;
    DEBUG ((
      DEBUG_INFO,
      "Waiting for PHY realtime link\n",
      ));

    while (!(MiiReg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
      /* Timeout reached ? */
      if (Index > PHY_AUTONEGOTIATE_TIMEOUT) {
        DEBUG ((
          DEBUG_WARN,
          "TIMEOUT!\n",
          ));
        PhyDriver->CurrentLink = LINK_DOWN;
        break;
       }
    }
    if ((Index ++ % 1000) == 0) {
      DEBUG ((
        DEBUG_INFO,
        "done\n",
        ));
     }
     gBS->Stall (1000);   /* 1 ms */
     PhyRead (PhyDriver->PhyAddr, MIIM_RTL8211x_PHY_STATUS, &MiiReg, MacBaseAddress);
  }
     DEBUG ((
       DEBUG_INFO,
       "done\n",
       ));
     gBS->Stall (500000); /* another 500 ms (results in faster booting) */
  } else {
     if (MiiReg & MIIM_RTL8211x_PHYSTAT_LINK) {
        PhyDriver->CurrentLink = LINK_UP;
     } else {
        PhyDriver->CurrentLink = LINK_DOWN;
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
EFIAPT
GenPhyUpdateLink (
  IN PHY_DRIVER   *PhyDriver,
  IN UINTN        MacBaseAddress
  )
{
  UINT32     MiiReg;
  UINT32     Index;
  EFI_STATUS Status;

  //
  // Wait if the link is up, and autonegotiation is in progress
  // (ie - we're capable and it's not done)
  //
  Status = PhyRead (PhyDriver->PhyAddr, MII_BMSR, &MiiReg, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // If we already saw the link up, and it hasn't gone down, then
  // we don't need to wait for autoneg again
  //
  if (PhyDriver->CurrentLink && MiiReg & BMSR_LSTATUS) {
    return EFI_SUCCESS;
  }

  if ((!(MiiReg & BMSR_ANEGCOMPLETE)) {
    Index = 0;

    DEBUG ((
      DEBUG_INFO,
      "%s Waiting for PHY auto negotiation to complete\n",
      phydev->dev->name
      );

    while (!(MiiReg & BMSR_ANEGCOMPLETE)) {
     //
     // Timeout reached ?
     //
     if (Index > (PHY_ANEG_TIMEOUT / 50)) {
       DEBUG ((
         DEBUG_INFO,
	 " TIMEOUT !\n"
	 );
       PhyDriver->CurrentLink = LinkDown;
       return EFI_TIMEOUT;
     }


     if ((Index ++ % 10) == 0) {
       DEBUG ((
         DEBUG_INFO,
	 " TIMEOUT !\n"
	 );
      }

      Status = PhyRead (PhyDriver->PhyAddr, MII_BMSR, &MiiReg, MacBaseAddress);
      gBS->Stall (50000);     /* 50 ms */
    }
       DEBUG ((
         DEBUG_INFO,
	 " done\n"
	 );
       PhyDriver->CurrentLink = LinkUp;
     } else {
       // 
       // Read the link a second time to clear the latched state */
       //
       Status = PhyRead (PhyDriver->PhyAddr, MII_BMSR, &MiiReg, MacBaseAddress);

       if (MiiReg & BMSR_LSTATUS) {
         PhyDriver->CurrentLink = LinkUp;
       } else {
         PhyDriver->CurrentLink = LinkDown;
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

STATIC
EFI_STATUS
MdioWaitReady (
  IN UINTN  MdioBase
  )
{
  UINT32  Timeout;
  UINT32  MdioReg;

  Timeout = STMMAC_MDIO_TIMEOUT;

  //
  // wait till the MII is not busy
  //
  do {
    //
    // read MII register
    //
    MdioReg = MmioRead32(MdioBase);
    if (Timeout-- == 100) {
      DEBUG ((
        DEBUG_ERROR,
        "MII busy Timeout\n"
        ));

      return EFI_TIMEOUT;
    }
  } while (MdioReg & MII_BUSY);

  return EFI_SUCCESS;
}

/*
 * Read from MII register (PHY Access).
 */
EFI_STATUS
EFIAPI
PhyRead (
  IN     UINT32   Addr,
  IN     UINT32   Reg,
  IN OUT UINT32   *Data,
  IN     UINTN    MacBaseAddress
  )
{
  UINT32        MiiConfig;
  UINT32        Count;

  //
  // Check it is a valid Reg
  //
  ASSERT (Reg < 31);

  MiiConfig = ((Addr << MIIADDRSHIFT) & MII_ADDRMSK) |
              ((Reg << MIIREGSHIFT) & MII_REGMSK)|
               MII_CLKRANGE_150_250M |
               MII_BUSY;

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  //
  // write this config to register
  //
  MmioWrite32 (MacBaseAddress + GMAC_MDIO_ADDR, MiiConfig);

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

   //
   // Read the data from the MII data register
   //
  *Data = MmioRead32 (MdioBase) & MII_DATA_MASK;
  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  return EFI_SUCCESS;
}

/*
 * Write to the MII registers (PHY access).
 */
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

  //
  // Check it is a valid Reg
  //
  ASSERT(Reg < 31);

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  MiiConfig = ((Addr << MIIADDRSHIFT) & MII_ADDRMSK) |
              ((Reg << MIIREGSHIFT) & MII_REGMSK)|
               MII_WRITE |
               MII_CLKRANGE_150_250M |
               MII_BUSY;
  //
  // Write the desired value to the register first
  //
  MmioWrite32 (MacBaseAddress + GMAC_MDIO_DATA, (Data & 0xFFFF));

  //
  // write this config to register
  //
  MmioWrite32 (MacBaseAddress + GMAC_MDIO_ADDR, MiiConfig);

  //
  // Wait until any existing MII operation is complete.
  //
  Status = MdioWaitReady (MdioBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "MdioDxe: MdioWaitReady error\n"
      ));

    return Status;
  }

  return EFI_SUCCESS;
}

/*
 * Read RTL8211F PHY Extended Registers.
 */
UINT32
EFIAPI
PhyRtl8211fExtendedRead (
  IN PHY_DRIVER   *PhyDriver,
  IN UINT32       DevAddr,
  IN UINT32       Regnum,
  IN UINTN        MacBaseAddress
  )
{
  UINT32         OldPage;
  UINT32         Value;
  EFI_STATUS     Status;

  Status = PhyRead (PhyDriver->PhyAddr, &OldPage, MIIM_RTL8211F_PAGE_SELECT);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, DevAddr, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, OldPage, MacBaseAddress);

  Status = PhyRead (PhyDriver->PhyAddr, &Value, RegNum);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  return Value;
}

/*
 * Write RTL8211F PHY Extended Registers.
 */
EFI_STATUS
EFIAPI
PhyRtl8211fExtendedWrite (
  IN PHY_DRIVER   *PhyDriver,
  IN UINT32       Mode,
  IN UINT32       DevAddr,
  IN UINT32       Regnum,
  IN UINT16       Val,
  IN UINTN        MacBaseAddress
  )
{
  UINT32 OldPage;

  Status = PhyRead (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, &OldPage, MacBaseAddress);
  if (EFI_ERROR (Status)) {
    return EFI_ERROR;
  }

  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, DevAddr, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, RegNum, Val, MacBaseAddress);
  PhyWrite (PhyDriver->PhyAddr, MIIM_RTL8211F_PAGE_SELECT, OldPage, MacBaseAddress);

  return EFI_SUCCESS;
}

