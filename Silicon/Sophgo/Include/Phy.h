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

typedef enum {
  PHY_CONNECTION_RGMII,
  PHY_CONNECTION_RGMII_ID,
  PHY_CONNECTION_RGMII_TXID,
  PHY_CONNECTION_RGMII_RXID,
  PHY_CONNECTION_SGMII,
  PHY_CONNECTION_RTBI,
  PHY_CONNECTION_XAUI,
  PHY_CONNECTION_RXAUI,
  PHY_CONNECTION_SFI
} PHY_CONNECTION;

typedef enum {
  NO_SPEED,
  SPEED_10,
  SPEED_100,
  SPEED_1000,
  SPEED_2500,
  SPEED_10000
} PHY_SPEED;

typedef struct {
  UINT32          Addr;
  UINT8           MdioIndex;
  BOOLEAN         LinkUp;
  BOOLEAN         FullDuplex;
  BOOLEAN         AutoNegotiation;
  PHY_SPEED       Speed;
  PHY_CONNECTION  Connection;
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
  IN UINT32                     PhyAddr,
  IN PHY_CONNECTION             PhyConnection,
  IN OUT PHY_DEVICE             **PhyDev
  );

struct _SOPHGO_PHY_PROTOCOL {
  SOPHGO_PHY_STATUS             Status;
  SOPHGO_PHY_INIT               Init;
};

extern EFI_GUID gSOPHGOPhyProtocolGuid;

#endif // __RTL_PHY_H__
