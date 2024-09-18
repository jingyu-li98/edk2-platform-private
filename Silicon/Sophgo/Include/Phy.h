/** @file
 *
 *  Copyright (c) 2024, SOPHGO Inc. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __RTL_PHY_H__
#define __RTL_PHY_H__

#define SOPHGO_PHY_PROTOCOL_GUID { 0x9A94120C, 0xE250, 0x48B5, { 0x8B, 0x53, 0x1B, 0x7E, 0x2F, 0x64, 0x13, 0x3F } }

typedef struct _SOPHGO_PHY_PROTOCOL SOPHGO_PHY_PROTOCOL;

/**
 * enum PHY_INTERFACE - Interface Mode definitions
 *
 * @PHY_INTERFACE_MODE_NA: Not Applicable - don't touch
 * @PHY_INTERFACE_MODE_INTERNAL: No interface, MAC and PHY combined
 * @PHY_INTERFACE_MODE_MII: Media-independent interface
 * @PHY_INTERFACE_MODE_GMII: Gigabit media-independent interface
 * @PHY_INTERFACE_MODE_SGMII: Serial gigabit media-independent interface
 * @PHY_INTERFACE_MODE_TBI: Ten Bit Interface
 * @PHY_INTERFACE_MODE_REVMII: Reverse Media Independent Interface
 * @PHY_INTERFACE_MODE_RMII: Reduced Media Independent Interface
 * @PHY_INTERFACE_MODE_REVRMII: Reduced Media Independent Interface in PHY role
 * @PHY_INTERFACE_MODE_RGMII: Reduced gigabit media-independent interface
 * @PHY_INTERFACE_MODE_RGMII_ID: RGMII with Internal RX+TX delay
 * @PHY_INTERFACE_MODE_RGMII_RXID: RGMII with Internal RX delay
 * @PHY_INTERFACE_MODE_RGMII_TXID: RGMII with Internal RX delay
 * @PHY_INTERFACE_MODE_RTBI: Reduced TBI
 * @PHY_INTERFACE_MODE_SMII: Serial MII
 * @PHY_INTERFACE_MODE_XGMII: 10 gigabit media-independent interface
 * @PHY_INTERFACE_MODE_XLGMII:40 gigabit media-independent interface
 * @PHY_INTERFACE_MODE_MOCA: Multimedia over Coax
 * @PHY_INTERFACE_MODE_PSGMII: Penta SGMII
 * @PHY_INTERFACE_MODE_QSGMII: Quad SGMII
 * @PHY_INTERFACE_MODE_TRGMII: Turbo RGMII
 * @PHY_INTERFACE_MODE_100BASEX: 100 BaseX
 * @PHY_INTERFACE_MODE_1000BASEX: 1000 BaseX
 * @PHY_INTERFACE_MODE_2500BASEX: 2500 BaseX
 * @PHY_INTERFACE_MODE_5GBASER: 5G BaseR
 * @PHY_INTERFACE_MODE_RXAUI: Reduced XAUI
 * @PHY_INTERFACE_MODE_XAUI: 10 Gigabit Attachment Unit Interface
 * @PHY_INTERFACE_MODE_10GBASER: 10G BaseR
 * @PHY_INTERFACE_MODE_25GBASER: 25G BaseR
 * @PHY_INTERFACE_MODE_USXGMII:  Universal Serial 10GE MII
 * @PHY_INTERFACE_MODE_10GKR: 10GBASE-KR - with Clause 73 AN
 * @PHY_INTERFACE_MODE_QUSGMII: Quad Universal SGMII
 * @PHY_INTERFACE_MODE_1000BASEKX: 1000Base-KX - with Clause 73 AN
 * @PHY_INTERFACE_MODE_10G_QXGMII: 10G-QXGMII - 4 ports over 10G USXGMII
 * @PHY_INTERFACE_MODE_MAX: Book keeping
 *
 * Describes the interface between the MAC and PHY.
 */
typedef enum {
  PHY_INTERFACE_MODE_NA,
  PHY_INTERFACE_MODE_INTERNAL,
  PHY_INTERFACE_MODE_MII,
  PHY_INTERFACE_MODE_GMII,
  PHY_INTERFACE_MODE_SGMII,
  PHY_INTERFACE_MODE_TBI,
  PHY_INTERFACE_MODE_REVMII,
  PHY_INTERFACE_MODE_RMII,
  PHY_INTERFACE_MODE_REVRMII,
  PHY_INTERFACE_MODE_RGMII,
  PHY_INTERFACE_MODE_RGMII_ID,
  PHY_INTERFACE_MODE_RGMII_RXID,
  PHY_INTERFACE_MODE_RGMII_TXID,
  PHY_INTERFACE_MODE_RTBI,
  PHY_INTERFACE_MODE_SMII,
  PHY_INTERFACE_MODE_XGMII,
  PHY_INTERFACE_MODE_XLGMII,
  PHY_INTERFACE_MODE_MOCA,
  PHY_INTERFACE_MODE_PSGMII,
  PHY_INTERFACE_MODE_QSGMII,
  PHY_INTERFACE_MODE_TRGMII,
  PHY_INTERFACE_MODE_100BASEX,
  PHY_INTERFACE_MODE_1000BASEX,
  PHY_INTERFACE_MODE_2500BASEX,
  PHY_INTERFACE_MODE_5GBASER,
  PHY_INTERFACE_MODE_RXAUI,
  PHY_INTERFACE_MODE_XAUI,
  /* 10GBASE-R, XFI, SFI - single lane 10G Serdes */
  PHY_INTERFACE_MODE_10GBASER,
  PHY_INTERFACE_MODE_25GBASER,
  PHY_INTERFACE_MODE_USXGMII,
  /* 10GBASE-KR - with Clause 73 AN */
  PHY_INTERFACE_MODE_10GKR,
  PHY_INTERFACE_MODE_QUSGMII,
  PHY_INTERFACE_MODE_1000BASEKX,
  PHY_INTERFACE_MODE_10G_QXGMII,
  PHY_INTERFACE_MODE_MAX,
} PHY_INTERFACE;

typedef enum {
  NO_SPEED,
  SPEED_10,
  SPEED_100,
  SPEED_1000,
  SPEED_2500,
  SPEED_10000
} PHY_SPEED;

/*
 * struct phy_device - An instance of a PHY
 * @state: State of the PHY for management purposes
 * @Speed: Current link speed
 * @Duplex: Current duplex
 * @Interface: enum PHY_INTERFACE value
 */
typedef struct {
  UINT32          PhyAddr;
  UINT8           MdioIndex;
  UINT32          CurrentLink;
  UINT32          PhyOldLink;
  BOOLEAN         LinkUp;
  BOOLEAN         FullDuplex;
  UINT32          Duplex;
  BOOLEAN         AutoNegotiation;
  PHY_SPEED       Speed;
  PHY_INTERFACE   Interface;
  UINT32          RxInternalDelayPs;
  UINT32          TxInternalDelayPs;
  //PHY_STATE       State;
} PHY_DEVICE;

/*
 * Before calling SOPHGO_PHY_STATUS driver should request PHY_DEVICE structure by
 * calling SOPHGO_PHY_INIT. Pointer to that needs to be provided as an argument to
 * SOPHGO_PHY_STATUS.
 */
typedef
EFI_STATUS
(EFIAPI *SOPHGO_PHY_STATUS) (
  IN CONST SOPHGO_PHY_PROTOCOL  *This,
  IN OUT PHY_DEVICE             *PhyDev
  );

/*
 * SOPHGO_PHY_INIT allocates PhyDev and provides driver with pointer via **PhyDev.
 * After it becomes unnecessary, PhyDev should be freed by a driver (or it will
 * get freed at ExitBootServices).
 */
typedef
EFI_STATUS
(EFIAPI *SOPHGO_PHY_INIT) (
  IN CONST SOPHGO_PHY_PROTOCOL  *This,
  IN PHY_INTERFACE              PhyInterface,
  IN PHY_DEVICE                 *PhyDev
  );

struct _SOPHGO_PHY_PROTOCOL {
  SOPHGO_PHY_STATUS             Status;
  SOPHGO_PHY_INIT               Init;
};

extern EFI_GUID gSophgoPhyProtocolGuid;

#endif // __RTL_PHY_H__
