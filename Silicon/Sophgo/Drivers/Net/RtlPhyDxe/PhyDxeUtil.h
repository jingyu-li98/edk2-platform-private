/** @file

  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __PHY_DXE_H__
#define __PHY_DXE_H__

typedef struct {
  UINT32 PhyAddr;
  UINT32 PhyCurrentLink;
  UINT32 PhyOldLink;
} PHY_DRIVER;

#define SPEED_1000                            1000
#define SPEED_100                             100
#define SPEED_10                              10

#define DUPLEX_FULL                           1
#define DUPLEX_HALF                           0

// PHY Super Special control/status
#define PHYSSCS_HCDSPEED_MASK                 (7 << 2)        // Speed indication
#define PHYSSCS_AUTODONE                      BIT12           // Auto-Negotiation Done

// Flags for PHY reset
#define PHY_RESET_PMT                         BIT0
#define PHY_RESET_BCR                         BIT1
#define PHY_RESET_CHECK_LINK                  BIT2

// Flags for auto negotiation
#define AUTO_NEGOTIATE_COLLISION_TEST         BIT0
#define AUTO_NEGOTIATE_ADVERTISE_ALL          BIT1

// HPS MII
#define MII_BUSY                              (1 << 0)
#define MII_WRITE                             (1 << 1)
#define MII_CLKRANGE_60_100M                  (0x0)
#define MII_CLKRANGE_100_150M                 (0x4)
#define MII_CLKRANGE_20_35M                   (0x8)
#define MII_CLKRANGE_35_60M                   (0xC)
#define MII_CLKRANGE_150_250M                 (0x10)
#define MII_CLKRANGE_250_300M                 (0x14)

#define MIIADDRSHIFT                          (11)
#define MIIREGSHIFT                           (6)
#define MII_REGMSK                            (0x1F << 6)
#define MII_ADDRMSK                           (0x1F << 11)

// Others
#define PHY_INVALID_ID                        0xFFFF
#define LINK_UP                               1
#define LINK_DOWN                             0
#define PHY_TIMEOUT                           200000

//
// Generic MII registers.
//
#define MII_BMCR                0x00    /* Basic mode control register */
#define MII_BMSR                0x01    /* Basic mode status register  */
#define MII_PHYSID1             0x02    /* PHYS ID 1                   */
#define MII_PHYSID2             0x03    /* PHYS ID 2                   */
#define MII_ADVERTISE           0x04    /* Advertisement control reg   */
#define MII_LPA                 0x05    /* Link partner ability reg    */
#define MII_EXPANSION           0x06    /* Expansion register          */
#define MII_CTRL1000            0x09    /* 1000BASE-T control          */
#define MII_STAT1000            0x0a    /* 1000BASE-T status           */
#define MII_MMD_CTRL            0x0d    /* MMD Access Control Register */
#define MII_MMD_DATA            0x0e    /* MMD Access Data Register */
#define MII_ESTATUS             0x0f    /* Extended Status             */
#define MII_DCOUNTER            0x12    /* Disconnect counter          */
#define MII_FCSCOUNTER          0x13    /* False carrier counter       */
#define MII_NWAYTEST            0x14    /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER         0x15    /* Receive error counter       */
#define MII_SREVISION           0x16    /* Silicon revision            */
#define MII_RESV1               0x17    /* Reserved...                 */
#define MII_LBRERROR            0x18    /* Lpback, rx, bypass error    */
#define MII_PHYADDR             0x19    /* PHY address                 */
#define MII_RESV2               0x1a    /* Reserved...                 */
#define MII_TPISTATUS           0x1b    /* TPI status for 10mbps       */
#define MII_NCONFIG             0x1c    /* Network interface config    */

//
// Basic mode control register.
//
#define BMCR_SPEED1000          BIT6    /* MSB of Speed (1000)         */
#define BMCR_CTST               BIT7    /* Collision test              */
#define BMCR_FULLDPLX           BIT8    /* Full duplex                 */
#define BMCR_ANRESTART          BIT9    /* Auto negotiation restart    */
#define BMCR_ISOLATE            BIT10   /* Isolate data paths from MII */
#define BMCR_PDOWN              BIT11   /* Enable low power state      */
#define BMCR_ANENABLE           BIT12   /* Enable auto negotiation     */
#define BMCR_SPEED100           BIT13   /* Select 100Mbps              */
#define BMCR_LOOPBACK           BIT14   /* TXD loopback bits           */
#define BMCR_RESET              BIT15   /* Reset to default state      */
#define BMCR_SPEED10            0x0000  /* Select 10Mbps               */

//
// Basic mode status register.
//
#define BMSR_ERCAP              BIT0   /* Ext-reg capability          */
#define BMSR_JCD                BIT1   /* Jabber detected             */
#define BMSR_LSTATUS            BIT2   /* Link status                 */
#define BMSR_ANEGCAPABLE        BIT3   /* Able to do auto-negotiation */
#define BMSR_RFAULT             BIT4   /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       BIT5   /* Auto-negotiation complete   */
#define BMSR_ESTATEN            BIT8   /* Extended Status in R15      */
#define BMSR_100HALF2           BIT9   /* Can do 100BASE-T2 HDX       */
#define BMSR_100FULL2           BIT10  /* Can do 100BASE-T2 FDX       */
#define BMSR_10HALF             BIT11  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             BIT12  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            BIT13  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            BIT14  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           BIT15  /* Can do 100mbps, 4k packets  */

//
// Advertisement control register.
//
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        BIT5    /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     BIT5    /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        BIT6    /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     BIT6    /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       BIT7    /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    BIT7    /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       BIT8    /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM BIT8    /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      BIT9    /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     BIT10   /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    BIT11   /* Try for asymetric pause     */
#define ADVERTISE_RESV          BIT12   /* Unused...                   */
#define ADVERTISE_RFAULT        BIT13   /* Say we can detect faults    */
#define ADVERTISE_LPACK         BIT14   /* Ack link partners response  */
#define ADVERTISE_NPAGE         BIT15   /* Next page bit               */

#define ADVERTISE_FULL          (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                                  ADVERTISE_CSMA)
#define ADVERTISE_ALL           (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                                  ADVERTISE_100HALF | ADVERTISE_100FULL)

//
// Link partner ability register.
//
#define LPA_SLCT                0x001f  /* Same as advertise selector  */
#define LPA_10HALF              BIT5    /* Can do 10mbps half-duplex   */
#define LPA_1000XFULL           BIT5    /* Can do 1000BASE-X full-duplex */
#define LPA_10FULL              BIT6    /* Can do 10mbps full-duplex   */
#define LPA_1000XHALF           BIT6    /* Can do 1000BASE-X half-duplex */
#define LPA_100HALF             BIT7    /* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE          BIT7    /* Can do 1000BASE-X pause     */
#define LPA_100FULL             BIT8    /* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM     BIT8    /* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4            BIT9    /* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP           BIT10   /* Can pause                   */
#define LPA_PAUSE_ASYM          BIT11   /* Can pause asymetrically     */
#define LPA_RESV                BIT12   /* Unused...                   */
#define LPA_RFAULT              BIT13   /* Link partner faulted        */
#define LPA_LPACK               BIT14   /* Link partner acked us       */
#define LPA_NPAGE               BIT15   /* Next page bit               */

#define LPA_DUPLEX              (LPA_10FULL | LPA_100FULL)
#define LPA_100                 (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

//
// Expansion register for auto-negotiation.
//
#define EXPANSION_NWAY          BIT0   /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          BIT1   /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   BIT2   /* This enables npage words    */
#define EXPANSION_NPCAPABLE     BIT3   /* Link partner supports npage */
#define EXPANSION_MFAULTS       BIT4   /* Multiple faults detected    */

#define ESTATUS_1000_XFULL      BIT15  /* Can do 1000BaseX Full       */
#define ESTATUS_1000_XHALF      BIT14  /* Can do 1000BaseX Half       */
#define ESTATUS_1000_TFULL      BIT13  /* Can do 1000BT Full          */
#define ESTATUS_1000_THALF      BIT12  /* Can do 1000BT Half          */

//
// N-way test register.
//
#define NWAYTEST_LOOPBACK       BIT8   /* Enable loopback for N-way   */

//
// RTL8221 PHY registers.
//
#define RTL821x_PHYSR                           0x11
#define RTL821x_PHYSR_DUPLEX                    BIT(13)
#define RTL821x_PHYSR_SPEED                     GENMASK(15, 14)

#define RTL821x_INER                            0x12
#define RTL8211B_INER_INIT                      0x6400
#define RTL8211E_INER_LINK_STATUS               BIT(10)
#define RTL8211F_INER_LINK_STATUS               BIT(4)

#define RTL821x_INSR                            0x13

#define RTL821x_EXT_PAGE_SELECT                 0x1e
#define RTL821x_PAGE_SELECT                     0x1f

#define RTL8211F_PHYCR1                         0x18
#define RTL8211F_PHYCR2                         0x19
#define RTL8211F_INSR                           0x1d

#define RTL8211F_TX_DELAY                       BIT(8)
#define RTL8211F_RX_DELAY                       BIT(3)

#define RTL8211F_ALDPS_PLL_OFF                  BIT(1)
#define RTL8211F_ALDPS_ENABLE                   BIT(2)
#define RTL8211F_ALDPS_XTAL_OFF                 BIT(12)

#define RTL8211E_CTRL_DELAY                     BIT(13)
#define RTL8211E_TX_DELAY                       BIT(12)
#define RTL8211E_RX_DELAY                       BIT(11)

#define RTL8211F_CLKOUT_EN                      BIT(0)

#define RTL8201F_ISR                            0x1e
#define RTL8201F_ISR_ANERR                      BIT(15)
#define RTL8201F_ISR_DUPLEX                     BIT(13)
#define RTL8201F_ISR_LINK                       BIT(11)
#define RTL8201F_ISR_MASK                       (RTL8201F_ISR_ANERR | \
                                                 RTL8201F_ISR_DUPLEX | \
                                                 RTL8201F_ISR_LINK)
#define RTL8201F_IER                            0x13

#define RTL8366RB_POWER_SAVE                    0x15
#define RTL8366RB_POWER_SAVE_ON                 BIT(12)

#define RTL_SUPPORTS_5000FULL                   BIT(14)
#define RTL_SUPPORTS_2500FULL                   BIT(13)
#define RTL_SUPPORTS_10000FULL                  BIT(0)
#define RTL_ADV_2500FULL                        BIT(7)
#define RTL_LPADV_10000FULL                     BIT(11)
#define RTL_LPADV_5000FULL                      BIT(6)
#define RTL_LPADV_2500FULL                      BIT(5)

#define RTL9000A_GINMR                          0x14
#define RTL9000A_GINMR_LINK_STATUS              BIT(4)

#define RTLGEN_SPEED_MASK                       0x0630

#define RTL_GENERIC_PHYID                       0x001cc800
#define RTL_8211FVD_PHYID                       0x001cc878

EFI_STATUS
EFIAPI
PhyDxeInitialization (
  IN  PHY_DRIVER     *PhyDriver,
  IN  UINTN          MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyDetectDevice (
  IN  PHY_DRIVER     *PhyDriver,
  IN  UINTN          MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyConfig (
  IN  PHY_DRIVER     *PhyDriver,
  IN  UINTN          MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhySoftReset (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyReadId (
  IN  UINT32        PhyAddr,
  IN  UINTN         MacBaseAddress
  );

VOID
EFIAPI
PhyConfigSkew (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

VOID
EFIAPI
PhyDisplayConfigSkew (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

VOID
EFIAPI
PhyConfigFlpBurstTiming (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

VOID
EFIAPI
PhyDisplayFlpBurstTiming (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyAutoNego (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyLinkAdjustEmacConfig (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyCheckLinkStatus (
  IN  PHY_DRIVER    *PhyDriver,
  IN  UINTN         MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyReadCapability (
  IN  PHY_DRIVER   *PhyDriver,
  IN  UINT32       *Speed,
  IN  UINT32       *Duplex,
  IN  UINTN        MacBaseAddress
  );

VOID
EFIAPI
PhyDisplayAbility (
  IN  UINT32       Speed,
  IN  UINT32       Duplex
  );

EFI_STATUS
EFIAPI
PhyRead (
  IN  UINT32       Addr,
  IN  UINT32       Reg,
  OUT UINT32       *Data,
  IN  UINTN        MacBaseAddress
  );

EFI_STATUS
EFIAPI
PhyWrite (
  IN  UINT32       Addr,
  IN  UINT32       Reg,
  IN  UINT32       Data,
  IN  UINTN        MacBaseAddress
  );

EFI_STATUS
EFIAPI
Phy9031ExtendedWrite (
  IN  PHY_DRIVER   *PhyDriver,
  IN  UINT32       Mode,
  IN  UINT32       DevAddr,
  IN  UINT32       Regnum,
  IN  UINT16       Val,
  IN  UINTN        MacBaseAddress
  );

UINT32
EFIAPI
Phy9031ExtendedRead (
  IN  PHY_DRIVER   *PhyDriver,
  IN  UINT32       Mode,
  IN  UINT32       DevAddr,
  IN  UINT32       Regnum,
  IN  UINTN        MacBaseAddress
  );

#endif /* __PHY_DXE_H__ */
