/* @file
** DpDrv.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_DRV_H_
#define    _DP_DRV_H_

#include "../PhyGopDxe.h"

#define DPTX_BASE_ADDRESS                    0x3000
//
// TX Registers
//
#define DPTX_LINK_BW_SET                     (DPTX_BASE_ADDRESS + 0x0000)
#define DPTX_LANE_COUNT_SET                  (DPTX_BASE_ADDRESS + 0x0004)
#define DPTX_ENHANCED_FRAME_EN               (DPTX_BASE_ADDRESS + 0x0008)
#define DPTX_TRAINING_PATTERN_SET            (DPTX_BASE_ADDRESS + 0x000c)

#define DPTX_LINK_QUAL_PATTERN_SET           (DPTX_BASE_ADDRESS + 0x0010)
#define DPTX_SCRAMBLING_DISABLE              (DPTX_BASE_ADDRESS + 0x0014)
#define DPTX_DOWNSPREAD_CTRL                 (DPTX_BASE_ADDRESS + 0x0018)
#define DPTX_ALT_SCRAMBLER_RESET             (DPTX_BASE_ADDRESS + 0x001c)
#define DPTX_HBR2_SCRAMBLER_RESET            (DPTX_BASE_ADDRESS + 0x0020)
#define DPTX_DISPLAYPORT_VERSION             (DPTX_BASE_ADDRESS + 0x0024)
#define DPTX_LANE_REMAP                      (DPTX_BASE_ADDRESS + 0x002C)

#define DPTX_ENABLE                          (DPTX_BASE_ADDRESS + 0x0080)
#define DPTX_ENABLE_MAIN_STREAM              (DPTX_BASE_ADDRESS + 0x0084)
#define DPTX_ENABLE_SEC_STREAM               (DPTX_BASE_ADDRESS + 0x0088)
#define DPTX_SEC_DATA_WINDOW                 (DPTX_BASE_ADDRESS + 0x008C)
#define DPTX_LINK_SOFT_RESET                 (DPTX_BASE_ADDRESS + 0x0090)
#define TR_INPUT_COURCE_ENABLE               (DPTX_BASE_ADDRESS + 0x0094)

#define DPTX_FORCE_SCRAMBLER_RESET           (DPTX_BASE_ADDRESS + 0x00C0)
#define DPTX_SOURCE_CONTROL_STATUS           (DPTX_BASE_ADDRESS + 0x00C4)
#define DPTX_DATA_CONTROL                    (DPTX_BASE_ADDRESS + 0x00C8)

#define DPTX_CORE_CAPABILITY                 (DPTX_BASE_ADDRESS + 0x00F8)
#define DPTX_CORE_ID                         (DPTX_BASE_ADDRESS + 0x00FC)

#define DPTX_CMD_REG                         (DPTX_BASE_ADDRESS + 0x0100)
#define DPTX_FIFO                            (DPTX_BASE_ADDRESS + 0x0104)
#define DPTX_ADDRESS_REG                     (DPTX_BASE_ADDRESS + 0x0108)
#define DPTX_CLK_DIVIDER                     (DPTX_BASE_ADDRESS + 0x010C)
#define DPTX_AUX_REPLY_TIMEOUT_INTERVAL      (DPTX_BASE_ADDRESS + 0x0110)
#define DPTX_SINK_HPD_STATE                  (DPTX_BASE_ADDRESS + 0x0128)
#define DPTX_STATUS                          (DPTX_BASE_ADDRESS + 0x0130)
#define DPTX_RCV_FIFO                        (DPTX_BASE_ADDRESS + 0x0134)
#define DPTX_RCV_REPLY_REG                   (DPTX_BASE_ADDRESS + 0x0138)
#define DPTX_RCV_REPLY_COUNT                 (DPTX_BASE_ADDRESS + 0x013C)

#define DPTX_INTERRUPT                       (DPTX_BASE_ADDRESS + 0x0140)
#define DPTX_INTERRUPT_MASK                  (DPTX_BASE_ADDRESS + 0x0144)
#define DPTX_RCV_DATA_COUNT                  (DPTX_BASE_ADDRESS + 0x0148)
#define DPTX_AUX_TRANSACTION_STATUS          (DPTX_BASE_ADDRESS + 0x014C)
#define DPTX_TIMER                           (DPTX_BASE_ADDRESS + 0x0158)

//
// Main Link registers
//
#define DPTX_MAIN_LINK_HTOTAL                (DPTX_BASE_ADDRESS + 0x0180)
#define DPTX_MAIN_LINK_VTOTAL                (DPTX_BASE_ADDRESS + 0x0184)
#define DPTX_MAIN_LINK_POLARITY              (DPTX_BASE_ADDRESS + 0x0188)
#define DPTX_MAIN_LINK_HSWIDTH               (DPTX_BASE_ADDRESS + 0x018C)
#define DPTX_MAIN_LINK_VSWIDTH               (DPTX_BASE_ADDRESS + 0x0190)
#define DPTX_MAIN_LINK_HRES                  (DPTX_BASE_ADDRESS + 0x0194)
#define DPTX_MAIN_LINK_VRES                  (DPTX_BASE_ADDRESS + 0x0198)
#define DPTX_MAIN_LINK_HSTART                (DPTX_BASE_ADDRESS + 0x019C)
#define DPTX_MAIN_LINK_VSTART                (DPTX_BASE_ADDRESS + 0x01A0)
#define DPTX_MAIN_LINK_MISC0                 (DPTX_BASE_ADDRESS + 0x01A4)
#define DPTX_MAIN_LINK_MISC1                 (DPTX_BASE_ADDRESS + 0x01A8)
#define DPTX_M_VID                           (DPTX_BASE_ADDRESS + 0x01AC)
#define DPTX_TRANSFER_UNIT_SIZE              (DPTX_BASE_ADDRESS + 0x01B0)
#define DPTX_N_VID                           (DPTX_BASE_ADDRESS + 0x01B4)
#define DPTX_USER_PIXEL_WIDTH                (DPTX_BASE_ADDRESS + 0x01B8)
#define DPTX_DATA_PER_LANE                   (DPTX_BASE_ADDRESS + 0x01BC)
#define DPTX_INTERLACED                      (DPTX_BASE_ADDRESS + 0x01C0)
#define DPTX_USER_POLARITY                   (DPTX_BASE_ADDRESS + 0x01C4)
#define DPTX_USER_CONTROL                    (DPTX_BASE_ADDRESS + 0x01C8)

//
// eDP CRC registers
//
#define DPTX_EDP_CRC_ENABLE                  (DPTX_BASE_ADDRESS + 0x01D0)
#define DPTX_EDP_CRC_RED                     (DPTX_BASE_ADDRESS + 0x01D4)
#define DPTX_EDP_CRC_GREEN                   (DPTX_BASE_ADDRESS + 0x01D8)
#define DPTX_EDP_CRC_BLUE                    (DPTX_BASE_ADDRESS + 0x01DC)

//
// PHY registers
//
#define DPTX_PHY_RESET                       (DPTX_BASE_ADDRESS + 0x0200)
#define DPTX_PHY_PREEMPHASIS_LANE_0          (DPTX_BASE_ADDRESS + 0x0210)
#define DPTX_PHY_PREEMPHASIS_LANE_1          (DPTX_BASE_ADDRESS + 0x0214)
#define DPTX_PHY_PREEMPHASIS_LANE_2          (DPTX_BASE_ADDRESS + 0x0218)
#define DPTX_PHY_PREEMPHASIS_LANE_3          (DPTX_BASE_ADDRESS + 0x021C)
#define DPTX_PHY_VOLTAGE_DIFF_LANE_0         (DPTX_BASE_ADDRESS + 0x0220)
#define DPTX_PHY_VOLTAGE_DIFF_LANE_1         (DPTX_BASE_ADDRESS + 0x0224)
#define DPTX_PHY_VOLTAGE_DIFF_LANE_2         (DPTX_BASE_ADDRESS + 0x0228)
#define DPTX_PHY_VOLTAGE_DIFF_LANE_3         (DPTX_BASE_ADDRESS + 0x022C)
#define DPTX_PHY_TRANSMIT_PRBS7              (DPTX_BASE_ADDRESS + 0x0230)
#define DPTX_PHY_CLOCK_FEEDBACK_SETTING      (DPTX_BASE_ADDRESS + 0x0234)
#define DPTX_PHY_POWER_DOWN                  (DPTX_BASE_ADDRESS + 0x0238)
#define DPTX_PHY_POSTEMPHASIS_LANE_0         (DPTX_BASE_ADDRESS + 0x0240)
#define DPTX_PHY_POSTEMPHASIS_LANE_1         (DPTX_BASE_ADDRESS + 0x0244)
#define DPTX_PHY_POSTEMPHASIS_LANE_2         (DPTX_BASE_ADDRESS + 0x0248)
#define DPTX_PHY_POSTEMPHASIS_LANE_3         (DPTX_BASE_ADDRESS + 0x024C)
#define DPTX_PHY_PLL_SETTINGS                (DPTX_BASE_ADDRESS + 0x0250)
#define DPTX_PHY_ELEC_IDLE                   (DPTX_BASE_ADDRESS + 0x0254)
#define DPTX_PHY_SSC_ENABLE                  (DPTX_BASE_ADDRESS + 0x0258)
#define DPTX_PHY_REPEATER_ENABLE             (DPTX_BASE_ADDRESS + 0x025C)
#define DPTX_PHY_STATUS                      (DPTX_BASE_ADDRESS + 0x0280)

//
// Secondary channel registers
//
#define DPTX_SEC_AUDIO_ENABLE                (DPTX_BASE_ADDRESS + 0x0300)
#define DPTX_SEC_INPUT_SELECT                (DPTX_BASE_ADDRESS + 0x0304)
#define DPTX_SEC_CHANNEL_COUNT               (DPTX_BASE_ADDRESS + 0x0308)
#define DPTX_SEC_DIRECT_CLKDIV               (DPTX_BASE_ADDRESS + 0x030C)
#define DPTX_SEC_INFOFRAME_ENABLE            (DPTX_BASE_ADDRESS + 0x0310)
#define DPTX_SEC_INFOFRAME_RATE              (DPTX_BASE_ADDRESS + 0x0314)
#define DPTX_SEC_MAUD                        (DPTX_BASE_ADDRESS + 0x0318)
#define DPTX_SEC_NAUD                        (DPTX_BASE_ADDRESS + 0x031C)
#define DPTX_SEC_AUDIO_CLOCK_MODE            (DPTX_BASE_ADDRESS + 0x0320)
#define DPTX_SEC_3D_VSC_DATA                 (DPTX_BASE_ADDRESS + 0x0324)
#define DPTX_SEC_AUDIO_FIFO                  (DPTX_BASE_ADDRESS + 0x0328)
#define DPTX_SEC_AUDIO_FIFO_DEPTH            (DPTX_BASE_ADDRESS + 0x032C)
#define DPTX_SEC_AUDIO_FIFO_READY            (DPTX_BASE_ADDRESS + 0x0330)
#define DPTX_SEC_INFOFRAME_SELECT            (DPTX_BASE_ADDRESS + 0x0334)
#define DPTX_SEC_INFOFRAME_DATA              (DPTX_BASE_ADDRESS + 0x0338)
#define DPTX_SEC_TIMESTAMP_INTERVAL          (DPTX_BASE_ADDRESS + 0x033C)
#define DPTX_SEC_CS_SOURCE_FORMAT            (DPTX_BASE_ADDRESS + 0x0340)
#define DPTX_SEC_CS_CATEGORY_CODE            (DPTX_BASE_ADDRESS + 0x0344)
#define DPTX_SEC_CS_LENGTH_ORIG_FREQ         (DPTX_BASE_ADDRESS + 0x0348)
#define DPTX_SEC_CS_FREQ_CLOCK_ACCURACY      (DPTX_BASE_ADDRESS + 0x034C)
#define DPTX_SEC_CS_COPYRIGHT                (DPTX_BASE_ADDRESS + 0x0350)
#define DPTX_SEC_GTC_COUNT_CONFIG            (DPTX_BASE_ADDRESS + 0x0354)
#define DPTX_SEC_GTC_COMMAND_EDGE            (DPTX_BASE_ADDRESS + 0x0358)
#define DPTX_SEC_AUDIO_CHANNEL_MAP           (DPTX_BASE_ADDRESS + 0x035C)
#define DPTX_SEC_PSR_3D_ENABLE               (DPTX_BASE_ADDRESS + 0x0360)
#define DPTX_SEC_PSR_CONFIG                  (DPTX_BASE_ADDRESS + 0x0364)
#define DPTX_SEC_PSR_STATE                   (DPTX_BASE_ADDRESS + 0x0368)
#define DPTX_SEC_AUDIO_OVERFLOW              (DPTX_BASE_ADDRESS + 0x0370)
#define DPTX_SEC_PACKET_COUNT                (DPTX_BASE_ADDRESS + 0x0374)

//
// HDCP 1.3 Registers
//
#define DPTX_HDCP13_ENABLE                   (DPTX_BASE_ADDRESS + 0x0400)
#define DPTX_HDCP13_MODE                     (DPTX_BASE_ADDRESS + 0x0404)
#define DPTX_HDCP13_KS_0                     (DPTX_BASE_ADDRESS + 0x0408)
#define DPTX_HDCP13_KS_1                     (DPTX_BASE_ADDRESS + 0x040C)
#define DPTX_HDCP13_KM_LOW                   (DPTX_BASE_ADDRESS + 0x0410)
#define DPTX_HDCP13_KM_HIGH                  (DPTX_BASE_ADDRESS + 0x0414)
#define DPTX_HDCP13_AN_LOW                   (DPTX_BASE_ADDRESS + 0x0418)
#define DPTX_HDCP13_AN_HIGH                  (DPTX_BASE_ADDRESS + 0x041C)
#define DPTX_HDCP13_RSVD_0                   (DPTX_BASE_ADDRESS + 0x0420)
#define DPTX_HDCP13_RSVD_1                   (DPTX_BASE_ADDRESS + 0x0424)
#define DPTX_HDCP13_R0                       (DPTX_BASE_ADDRESS + 0x0428)
#define DPTX_HDCP13_USE_HARDWARE_KM          (DPTX_BASE_ADDRESS + 0x042C)
#define DPTX_HDCP13_BKSV_0                   (DPTX_BASE_ADDRESS + 0x0430)
#define DPTX_HDCP13_BKSV_1                   (DPTX_BASE_ADDRESS + 0x0434)
#define DPTX_HDCP13_AKSV_0                   (DPTX_BASE_ADDRESS + 0x0438)
#define DPTX_HDCP13_AKSV_1                   (DPTX_BASE_ADDRESS + 0x043C)
#define DPTX_HDCP13_REPEATER                 (DPTX_BASE_ADDRESS + 0x0450)
#define DPTX_HDCP13_STREAM_CIPHER_ENABLE     (DPTX_BASE_ADDRESS + 0x0454)
#define DPTX_HDCP13_M0_LOWER                 (DPTX_BASE_ADDRESS + 0x0458)
#define DPTX_HDCP13_M0_UPPER                 (DPTX_BASE_ADDRESS + 0x045C)
#define DPTX_HDCP13_RNG_STORE_AN             (DPTX_BASE_ADDRESS + 0x0480)
#define DPTX_HDCP13_RNG_LOWER                (DPTX_BASE_ADDRESS + 0x0484)
#define DPTX_HDCP13_RNG_UPPER                (DPTX_BASE_ADDRESS + 0x0488)

//
// HDCP 2.2 Registers
//
#define DPTX_HDCP22_ENABLE                   (DPTX_BASE_ADDRESS + 0x0400) //  HDCP enable
#define DPTX_HDCP22_MODE                     (DPTX_BASE_ADDRESS + 0x0404) //  HDCP mode select (set to a value of 2 for HDCP 2.2)
#define DPTX_HDCP22_KS_0                     (DPTX_BASE_ADDRESS + 0x0408) //  Ks bits 31:0
#define DPTX_HDCP22_KS_1                     (DPTX_BASE_ADDRESS + 0x040c) //  Ks bits 63:32
#define DPTX_HDCP22_KS_2                     (DPTX_BASE_ADDRESS + 0x0410) //  Ks bits 95:64
#define DPTX_HDCP22_KS_3                     (DPTX_BASE_ADDRESS + 0x0414) //  Ks bits 127:96
#define DPTX_HDCP22_RTX_0                    (DPTX_BASE_ADDRESS + 0x0418) //  Rtx bits 31:0, session random number
#define DPTX_HDCP22_RTX_1                    (DPTX_BASE_ADDRESS + 0x041c) //  Rtx bits 63:32, session random number
#define DPTX_HDCP22_RRX_0                    (DPTX_BASE_ADDRESS + 0x0430) //  Rrx bits 31:0
#define DPTX_HDCP22_RRX_1                    (DPTX_BASE_ADDRESS + 0x0434) //  Rrx bits 63:32
#define DPTX_HDCP22_DKEY_2                   (DPTX_BASE_ADDRESS + 0x0438) //  Dkey bits 95:64
#define DPTX_HDCP22_DKEY_3                   (DPTX_BASE_ADDRESS + 0x043c) //  Dkey bits 127:96
#define DPTX_HDCP22_LC128_0                  (DPTX_BASE_ADDRESS + 0x0440) //  Global constant, lc128 bits 31:0
#define DPTX_HDCP22_LC128_1                  (DPTX_BASE_ADDRESS + 0x0444) //  Global constant, lc128 bits 63:32
#define DPTX_HDCP22_LC128_2                  (DPTX_BASE_ADDRESS + 0x0448) //  Global constant, lc128 bits 95:64
#define DPTX_HDCP22_LC128_3                  (DPTX_BASE_ADDRESS + 0x044c) //  Global constant, lc128 bits 127:64
#define DPTX_HDCP22_REPEATER                 (DPTX_BASE_ADDRESS + 0x0450) //  Repeater global enable / disable
#define DPTX_HDCP22_STREAM_CYPHER_ENABLE     (DPTX_BASE_ADDRESS + 0x0454) //  Stream cipher enable
#define DPTX_HDCP22_DKEY_0                   (DPTX_BASE_ADDRESS + 0x0458) //  Dkey bits 31:0
#define DPTX_HDCP22_DKEY_1                   (DPTX_BASE_ADDRESS + 0x045c) //  Dkey bits 63:32
#define DPTX_HDCP22_AES_INPUT_SELECT         (DPTX_BASE_ADDRESS + 0x0460) //  AES input select
#define DPTX_HDCP22_AES_CTR_DISABLE          (DPTX_BASE_ADDRESS + 0x0464) //  AES counter disable
#define DPTX_HDCP22_AES_CTR_INC              (DPTX_BASE_ADDRESS + 0x0468) //  AES counter increment
#define DPTX_HDCP22_ENCRYPTION_CTL_0         (DPTX_BASE_ADDRESS + 0x046c) //  HDCP Encryption Control Field, bits 31:0
#define DPTX_HDCP22_ENCRYPTION_CTL_1         (DPTX_BASE_ADDRESS + 0x0470) //  HDCP Encryption Control Field, bits 63:32
#define DPTX_HDCP22_AES_CTR_RESET            (DPTX_BASE_ADDRESS + 0x0474) //  AES counter reset
#define DPTX_HDCP22_RN_0                     (DPTX_BASE_ADDRESS + 0x0478) //  Rn bits 31:0
#define DPTX_HDCP22_RN_1                     (DPTX_BASE_ADDRESS + 0x047c) //  Rn bits 63:32

//
// MST control registers
//
#define DPTX_MST_ENABLE                      (DPTX_BASE_ADDRESS + 0x0500)
#define DPTX_MST_PID_TABLE_INDEX             (DPTX_BASE_ADDRESS + 0x0504)
#define DPTX_MST_PID_TABLE_DATA              (DPTX_BASE_ADDRESS + 0x0508)
#define DPTX_MST_SST_SOURCE                  (DPTX_BASE_ADDRESS + 0x050C)
#define DPTX_MST_ACT_TRIGGER                 (DPTX_BASE_ADDRESS + 0x0510)
#define DPTX_MST_SOURCE_ACTIVE               (DPTX_BASE_ADDRESS + 0x0520)
#define DPTX_MST_LINK_FRAME_COUNT            (DPTX_BASE_ADDRESS + 0x0524)

//
// TX AUX status constants - DPTX_STATUS
//
#define DPTX_AUX_HPD                         0x01 // Bit 0
#define DPTX_AUX_REQUEST_IN_PROGRESS         0x02 // Bit 1
#define DPTX_AUX_REPLY_RECEIVED              0x04 // Bit 2
#define DPTX_AUX_REPLY_TIMEOUT               0x08 // Bit 3

//
// AUX transaction sattus - DPTX_AUX_TRANSACTION_STATUS
//
#define DPTX_AUX_STATUS_REPLY_RECEIVED       0x01 // Bit 0
#define DPTX_AUX_STATUS_REPLY_IN_PROGRESS    0x02 // Bit 1
#define DPTX_AUX_STATUS_REQUEST_IN_PROGRESS  0x04 // Bit 2
#define DPTX_AUX_STATUS_REPLY_ERROR          0x08 // Bit 3

//
// Timer block
//
#define DPTX_TIMER_PWM_CONTROL               (DPTX_TIMER + 0x0004)
#define DPTX_TIMER_INTR_STATUS               (DPTX_TIMER + 0x0008)
#define DPTX_TIMER_MAX_COUNT                 0x1FFFFF     // 20 bits

//
// Timer Register
// bit 31     = Timer enable
// bit 30     = Auto-reload
// bit 29     = Interrupt enable
// bit 28 : 0 = Timer value
//
#define DPTX_TIMER_FLAG_ENABLE               0x80000000
#define DPTX_TIMER_FLAG_AUTORELOAD_EN        0x40000000
#define DPTX_TIMER_FLAG_INTERRUPT_EN         0x20000000
#define DPTX_TIMER_FLAGS_ALL                 (DPTX_TIMER_FLAG_ENABLE | DPTX_TIMER_FLAG_AUTORELOAD_EN | DPTX_TIMER_FLAG_INTERRUPT_EN)

//
// AUX CH Requests
//
#define TR_DP_CMD_REQUEST_READ                      0x0900
#define TR_DP_CMD_REQUEST_WRITE                     0x0800
#define TR_DP_CMD_REQUEST_I2C_READ                  0x0100
#define TR_DP_CMD_REQUEST_I2C_WRITE                 0x0000
#define TR_DP_CMD_REQUEST_I2C_WRITE_STATUS          0x0200
#define TR_DP_CMD_REQUEST_I2C_READ_MOT              0x0500
#define TR_DP_CMD_REQUEST_I2C_WRITE_MOT             0x0400
#define TR_DP_CMD_REQUEST_I2C_WRITE_STATUS_MOT      0x0600

//
// AUX Channel Replies
//
#define TR_DP_CMD_REPLY_ACK                         0x00
#define TR_DP_CMD_REPLY_NACK                        0x01
#define TR_DP_CMD_REPLY_DEFER                       0x02
#define TR_DP_CMD_REPLY_UNKNOWN                     0xAA
#define TR_DP_CMD_REPLY_INVALID                     0x55
#define TR_DP_CMD_REPLY_I2C_ACK                     0x00
#define TR_DP_CMD_REPLY_I2C_NACK                    0x04
#define TR_DP_CMD_REPLY_I2C_DEFER                   0x08
#define TR_DP_CMD_ADDRESS_ONLY                      0x1000

//
// Return codes from link rate set functions
//
#define TR_DP_LINK_RATE_OK                          0x00
#define TR_DP_LINK_RATE_NOT_SUPPORTED               0x01

//
// MST Constants for the TX core
//
#define DPTX_MST_ENABLE_BIT                         0x01

#define HPD_CHECK_TIMES                             5
#define HPD_CHECK_INTERVAL_MS                       10

//
// Timer block
//
#define TR_DPTX_TIMER_PWM_CONTROL                  (DPTX_TIMER + 0x0004)
#define TR_DPTX_TIMER_INTR_STATUS                  (DPTX_TIMER + 0x0008)
#define TR_DPTX_TIMER_MAX_COUNT                     0x1FFFFF     // 20 bits

#define TR_DPTX_TIMER_FLAG_ENABLE                   0x80000000
#define TR_DPTX_TIMER_FLAG_AUTORELOAD_EN            0x40000000
#define TR_DPTX_TIMER_FLAG_INTERRUPT_EN             0x20000000
#define TR_DPTX_TIMER_FLAGS_ALL                     (TR_DPTX_TIMER_FLAG_ENABLE | TR_DPTX_TIMER_FLAG_AUTORELOAD_EN | TR_DPTX_TIMER_FLAG_INTERRUPT_EN)


#define SINK_DISCONNECT                             1
#define SINK_CONNECT                                0

#define DP_HPD_ON                                   1
#define DP_HPD_OFF                                  0

#define AUX_TIMEOUT                                 1
#define AUX_CONNECT                                 0

UINT32
SinkDpcdRead (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            DpcdAddr,
  OUT UINT8           *Data
  );


UINT32
SinkDpcdWrite (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT32           DpcdAddr,
  IN UINT8            WriteData
  );

VOID
CoreIDGet (
  PHY_PRIVATE_DATA  *Private,
  UINT8             Num,
  UINT16            *Coreid,
  UINT16            *Corerev
  );

UINT32
CheckHpdStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

UINT32
GetHpdStatusOnce (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );


UINT32
InitAux (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

UINT32
ConfigDptx (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate,
  IN UINT32            LaneCount
  );

UINT32
ConfigSink (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate,
  IN UINT32            LaneCount
  );

VOID
WakeUpSink (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  );

UINT32
FastTraining (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LaneCount
  );

VOID
DptxSetTraningPattern (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN TRAINING_PATTERN  TrainPattern
  );

VOID
DptxDisableScrambling (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            Disable
  );

VOID
DptxScramblingRest (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  );

VOID
ConfigMainStreamAttr(
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN DP_CONFIG        *DpConfig,
  IN DP_SYNC          *DpSync
  );

VOID
EnableFramingMode (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN BOOLEAN          Enable
  );

UINT32
DpConnect (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8         Num
  );


UINT32
GetEdid (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN OUT  UINT8        *Buffer
  );

EFI_STATUS
ParseEdidDtdList (
  IN  UINT8      *Buffer,
  OUT DTD_LIST   *List
  );

EFI_STATUS
FindDpSyncFromDtd (
  IN  DTD_LIST      *List,
  IN  PHY_GOP_MODE  *GopMode,
  OUT DP_SYNC       *Sync,
  OUT UINT32        *Clock
  );

EFI_STATUS
FindDcSyncFromDtd (
  IN  DTD_LIST      *List,
  IN  PHY_GOP_MODE  *GopMode,
  OUT DC_SYNC       *Sync
  );

UINT32
AuxTest (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32             Num,
  IN UINT32             Times
  );

UINT32
DpStartLinkTrain (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num,
  IN UINT8            MaxLaneCount,
  IN UINT32           MaxLinkRate
  );
#endif
