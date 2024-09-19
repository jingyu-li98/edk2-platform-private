/** @file

  Copyright (c) 2024, SOPHGO Inc. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


#ifndef __PHY_DXE_H__
#define __PHY_DXE_H__
#include <Include/Phy.h>

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
#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000          0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Isolate data paths from MII */
#define BMCR_PDOWN              0x0800  /* Enable low power state      */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset to default state      */
#define BMCR_SPEED10            0x0000  /* Select 10Mbps               */

//
// Basic mode status register.
//
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_RFAULT             0x0010  /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       0x0020  /* Auto-negotiation complete   */
#define BMSR_RESV               0x00c0  /* Unused...                   */
#define BMSR_ESTATEN            0x0100  /* Extended Status in R15      */
#define BMSR_100HALF2           0x0200  /* Can do 100BASE-T2 HDX       */
#define BMSR_100FULL2           0x0400  /* Can do 100BASE-T2 FDX       */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           0x8000  /* Can do 100mbps, 4k packets  */

//
// Advertisement control register.
//
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

#define ADVERTISE_FULL          (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                                 ADVERTISE_CSMA)
#define ADVERTISE_ALL           (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                                 ADVERTISE_100HALF | ADVERTISE_100FULL)

//
// Link partner ability register.
//
#define LPA_SLCT                0x001f  /* Same as advertise selector  */
#define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
#define LPA_1000XFULL           0x0020  /* Can do 1000BASE-X full-duplex */
#define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
#define LPA_1000XHALF           0x0040  /* Can do 1000BASE-X half-duplex */
#define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE          0x0080  /* Can do 1000BASE-X pause     */
#define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM     0x0100  /* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4            0x0200  /* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP           0x0400  /* Can pause                   */
#define LPA_PAUSE_ASYM          0x0800  /* Can pause asymetrically     */
#define LPA_RESV                0x1000  /* Unused...                   */
#define LPA_RFAULT              0x2000  /* Link partner faulted        */
#define LPA_LPACK               0x4000  /* Link partner acked us       */
#define LPA_NPAGE               0x8000  /* Next page bit               */

#define LPA_DUPLEX              (LPA_10FULL | LPA_100FULL)
#define LPA_100                 (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

//
// Expansion register for auto-negotiation.
//
#define EXPANSION_NWAY          0x0001  /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          0x0002  /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   0x0004  /* This enables npage words    */
#define EXPANSION_NPCAPABLE     0x0008  /* Link partner supports npage */
#define EXPANSION_MFAULTS       0x0010  /* Multiple faults detected    */
#define EXPANSION_RESV          0xffe0  /* Unused...                   */

#define ESTATUS_1000_XFULL      0x8000  /* Can do 1000BX Full */
#define ESTATUS_1000_XHALF      0x4000  /* Can do 1000BX Half */
#define ESTATUS_1000_TFULL      0x2000  /* Can do 1000BT Full          */
#define ESTATUS_1000_THALF      0x1000  /* Can do 1000BT Half          */

//
// N-way test register.
//
#define NWAYTEST_RESV1          0x00ff  /* Unused...                   */
#define NWAYTEST_LOOPBACK       0x0100  /* Enable loopback for N-way   */
#define NWAYTEST_RESV2          0xfe00  /* Unused...                   */

//
// 1000BASE-T Control register
//
#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */
#define CTL1000_AS_MASTER       0x0800
#define CTL1000_ENABLE_MASTER   0x1000

//
// 1000BASE-T Status register
//
#define LPA_1000LOCALRXOK       0x2000  /* Link partner local receiver status */
#define LPA_1000REMRXOK         0x1000  /* Link partner remote receiver status */
#define LPA_1000FULL            0x0800  /* Link partner 1000BASE-T full duplex */
#define LPA_1000HALF            0x0400  /* Link partner 1000BASE-T half duplex */

//
// Flow control flags
//
#define FLOW_CTRL_TX            0x01
#define FLOW_CTRL_RX            0x02

//
// MMD Access Control register fields
//
#define MII_MMD_CTRL_DEVAD_MASK 0x1f    /* Mask MMD DEVAD*/
#define MII_MMD_CTRL_ADDR       0x0000  /* Address */
#define MII_MMD_CTRL_NOINCR     0x4000  /* no post increment */
#define MII_MMD_CTRL_INCR_RDWT  0x8000  /* post increment on reads & writes */
#define MII_MMD_CTRL_INCR_ON_WT 0xC000  /* post increment on writes only */

/* ***************** */
#define PHY_RTL8211x_FORCE_MASTER BIT(1)
#define PHY_RTL8211F_FORCE_EEE_RXC_ON BIT(3)
#define PHY_RTL8201F_S700_RMII_TIMINGS BIT(4)

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

//
// RTL8211x 1000BASE-T Control Register
//
#define MIIM_RTL8211x_CTRL1000T_MSCE BIT(12);
#define MIIM_RTL8211x_CTRL1000T_MASTER BIT(11);

//
// RTL8211x PHY Status Register
//
#define MIIM_RTL8211x_PHY_STATUS       0x11
#define MIIM_RTL8211x_PHYSTAT_SPEED    0xc000
#define MIIM_RTL8211x_PHYSTAT_GBIT     0x8000
#define MIIM_RTL8211x_PHYSTAT_100      0x4000
#define MIIM_RTL8211x_PHYSTAT_DUPLEX   0x2000
#define MIIM_RTL8211x_PHYSTAT_SPDDONE  0x0800
#define MIIM_RTL8211x_PHYSTAT_LINK     0x0400

//
// RTL8211x PHY Interrupt Enable Register
//
#define MIIM_RTL8211x_PHY_INER         0x12
#define MIIM_RTL8211x_PHY_INTR_ENA     0x9f01
#define MIIM_RTL8211x_PHY_INTR_DIS     0x0000

//
// RTL8211x PHY Interrupt Status Register
//
#define MIIM_RTL8211x_PHY_INSR         0x13

//
// RTL8211F PHY Status Register
//
#define MIIM_RTL8211F_PHY_STATUS       0x1a
#define MIIM_RTL8211F_AUTONEG_ENABLE   0x1000
#define MIIM_RTL8211F_PHYSTAT_SPEED    0x0030
#define MIIM_RTL8211F_PHYSTAT_GBIT     0x0020
#define MIIM_RTL8211F_PHYSTAT_100      0x0010
#define MIIM_RTL8211F_PHYSTAT_DUPLEX   0x0008
#define MIIM_RTL8211F_PHYSTAT_SPDDONE  0x0800
#define MIIM_RTL8211F_PHYSTAT_LINK     0x0004

#define MIIM_RTL8211E_CONFREG           0x1c
#define MIIM_RTL8211E_CTRL_DELAY        BIT(13)
#define MIIM_RTL8211E_TX_DELAY          BIT(12)
#define MIIM_RTL8211E_RX_DELAY          BIT(11)

#define MIIM_RTL8211E_EXT_PAGE_SELECT  0x1e

#define MIIM_RTL8211F_PAGE_SELECT      0x1f
#define MIIM_RTL8211F_TX_DELAY          0x100
#define MIIM_RTL8211F_RX_DELAY          0x8
#define MIIM_RTL8211F_LCR               0x10

#define RTL8201F_RMSR                   0x10

#define RMSR_RX_TIMING_SHIFT            BIT(2)
#define RMSR_RX_TIMING_MASK             GENMASK(7, 4)
#define RMSR_RX_TIMING_VAL              0x4
#define RMSR_TX_TIMING_SHIFT            BIT(3)
#define RMSR_TX_TIMING_MASK             GENMASK(11, 8)
#define RMSR_TX_TIMING_VAL              0x5

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
/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half          (1 << 0)
#define SUPPORTED_10baseT_Full          (1 << 1)
#define SUPPORTED_100baseT_Half         (1 << 2)
#define SUPPORTED_100baseT_Full         (1 << 3)
#define SUPPORTED_1000baseT_Half        (1 << 4)
#define SUPPORTED_1000baseT_Full        (1 << 5)
#define SUPPORTED_Autoneg               (1 << 6)
#define SUPPORTED_TP                    (1 << 7)
#define SUPPORTED_AUI                   (1 << 8)
#define SUPPORTED_MII                   (1 << 9)
#define SUPPORTED_FIBRE                 (1 << 10)
#define SUPPORTED_BNC                   (1 << 11)
#define SUPPORTED_10000baseT_Full       (1 << 12)
#define SUPPORTED_Pause                 (1 << 13)
#define SUPPORTED_Asym_Pause            (1 << 14)
#define SUPPORTED_2500baseX_Full        (1 << 15)
#define SUPPORTED_Backplane             (1 << 16)
#define SUPPORTED_1000baseKX_Full       (1 << 17)
#define SUPPORTED_10000baseKX4_Full     (1 << 18)
#define SUPPORTED_10000baseKR_Full      (1 << 19)
#define SUPPORTED_10000baseR_FEC        (1 << 20)
#define SUPPORTED_1000baseX_Half        (1 << 21)
#define SUPPORTED_1000baseX_Full        (1 << 22)

/* Indicates what features are advertised by the interface. */
#define ADVERTISED_10baseT_Half         (1 << 0)
#define ADVERTISED_10baseT_Full         (1 << 1)
#define ADVERTISED_100baseT_Half        (1 << 2)
#define ADVERTISED_100baseT_Full        (1 << 3)
#define ADVERTISED_1000baseT_Half       (1 << 4)
#define ADVERTISED_1000baseT_Full       (1 << 5)
#define ADVERTISED_Autoneg              (1 << 6)
#define ADVERTISED_TP                   (1 << 7)
#define ADVERTISED_AUI                  (1 << 8)
#define ADVERTISED_MII                  (1 << 9)
#define ADVERTISED_FIBRE                (1 << 10)
#define ADVERTISED_BNC                  (1 << 11)
#define ADVERTISED_10000baseT_Full      (1 << 12)
#define ADVERTISED_Pause                (1 << 13)
#define ADVERTISED_Asym_Pause           (1 << 14)
#define ADVERTISED_2500baseX_Full       (1 << 15)
#define ADVERTISED_Backplane            (1 << 16)
#define ADVERTISED_1000baseKX_Full      (1 << 17)
#define ADVERTISED_10000baseKX4_Full    (1 << 18)
#define ADVERTISED_10000baseKR_Full     (1 << 19)
#define ADVERTISED_10000baseR_FEC       (1 << 20)
#define ADVERTISED_1000baseX_Half       (1 << 21)
#define ADVERTISED_1000baseX_Full       (1 << 22)

/* The following are all involved in forcing a particular link
 * mode for the device for setting things.  When getting the
 * devices settings, these indicate the current mode and whether
 * it was foced up into this mode or autonegotiated.
 */


#define PHY_MAX_ADDR 32

#define PHY_FLAG_BROKEN_RESET   (1 << 0) /* soft reset not supported */

#define PHY_DEFAULT_FEATURES    (SUPPORTED_Autoneg | \
                                 SUPPORTED_TP | \
                                 SUPPORTED_MII)

#define PHY_10BT_FEATURES       (SUPPORTED_10baseT_Half | \
                                 SUPPORTED_10baseT_Full)

#define PHY_100BT_FEATURES      (SUPPORTED_100baseT_Half | \
                                 SUPPORTED_100baseT_Full)

#define PHY_1000BT_FEATURES     (SUPPORTED_1000baseT_Half | \
                                 SUPPORTED_1000baseT_Full)

#define PHY_BASIC_FEATURES      (PHY_10BT_FEATURES | \
                                 PHY_100BT_FEATURES | \
                                 PHY_DEFAULT_FEATURES)

#define PHY_100BT1_FEATURES     (SUPPORTED_TP | \
                                 SUPPORTED_MII | \
                                 SUPPORTED_100baseT_Full)

#define PHY_GBIT_FEATURES       (PHY_BASIC_FEATURES | \
                                 PHY_1000BT_FEATURES)

#define PHY_10G_FEATURES        (PHY_GBIT_FEATURES | \
                                SUPPORTED_10000baseT_Full)

//#ifndef PHY_ANEG_TIMEOUT
#define PHY_ANEG_TIMEOUT        4000

//
// Stmmac MDIO
//
#define MIIADDRSHIFT            (11)
#define MIIREGSHIFT             (6)
#define MII_REGMSK              (0x1F << 6)
#define MII_ADDRMSK             (0x1F << 11)

EFI_STATUS
EFIAPI
PhyDetectDevice (
  IN  PHY_DEVICE     *PhyDev
  );

EFI_STATUS
EFIAPI
PhyRtl8211fConfig (
  IN  PHY_DEVICE     *PhyDev
  );

EFI_STATUS
EFIAPI
PhySoftReset (
  IN  PHY_DEVICE     *PhyDev
  );

EFI_STATUS
EFIAPI
PhyReadId (
  IN  PHY_DEVICE     *PhyDev
  );

EFI_STATUS
EFIAPI
PhyAutoNego (
  IN  PHY_DEVICE     *PhyDev
  );

EFI_STATUS
EFIAPI
Rtl8211fStartUp (
  IN PHY_DEVICE      *PhyDev
  );

EFI_STATUS
EFIAPI
Rtl8211xParseStatus (
  IN PHY_DEVICE      *PhyDev,
  IN UINT32          *Speed,
  IN UINT32          *Duplex
  );

EFI_STATUS
EFIAPI
GenPhyUpdateLink (
  IN PHY_DEVICE      *PhyDev
  );

VOID
EFIAPI
PhyDisplayAbility (
  IN  UINT32         Speed,
  IN  UINT32         Duplex
  );

UINT32
EFIAPI
PhyRtl8211fExtendedRead (
  IN PHY_DEVICE      *PhyDev,
  IN UINT32          DevAddr,
  IN UINT32          RegNum
  );

EFI_STATUS
EFIAPI
PhyRtl8211fExtendedWrite (
  IN PHY_DEVICE      *PhyDev,
  IN UINT32          Mode,
  IN UINT32          DevAddr,
  IN UINT32          RegNum,
  IN UINT16          Val
  );

#endif /* __PHY_DXE_H__ */
