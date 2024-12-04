/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_DRV_H_
#define    _DP_DRV_H_

//
// TX Registers
//
#define DPTX_LINK_BW_SET                     0x0000
#define DPTX_LANE_COUNT_SET                  0x0004
#define DPTX_ENHANCED_FRAME_EN               0x0008
#define DPTX_TRAINING_PATTERN_SET            0x000c

#define DPTX_LINK_QUAL_PATTERN_SET           0x0010
#define DPTX_SCRAMBLING_DISABLE              0x0014
#define DPTX_DOWNSPREAD_CTRL                 0x0018
#define DPTX_ALT_SCRAMBLER_RESET             0x001c
#define DPTX_HBR2_SCRAMBLER_RESET            0x0020
#define DPTX_DISPLAYPORT_VERSION             0x0024
#define DPTX_LANE_REMAP                      0x002C

#define DPTX_ENABLE                          0x0080
#define DPTX_ENABLE_MAIN_STREAM              0x0084
#define DPTX_ENABLE_SEC_STREAM               0x0088
#define DPTX_SEC_DATA_WINDOW                 0x008C
#define DPTX_LINK_SOFT_RESET                 0x0090
#define TR_INPUT_COURCE_ENABLE               0x0094

#define DPTX_FORCE_SCRAMBLER_RESET           0x00C0
#define DPTX_SOURCE_CONTROL_STATUS           0x00C4
#define DPTX_DATA_CONTROL                    0x00C8

#define DPTX_CORE_CAPABILITY                 0x00F8
#define DPTX_CORE_ID                         0x00FC

#define DPTX_CMD_REG                         0x0100
#define DPTX_FIFO                            0x0104
#define DPTX_ADDRESS_REG                     0x0108
#define DPTX_CLK_DIVIDER                     0x010C
#define DPTX_AUX_REPLY_TIMEOUT_INTERVAL      0x0110
#define DPTX_SINK_HPD_STATE                  0x0128
#define DPTX_STATUS                          0x0130
#define DPTX_RCV_FIFO                        0x0134
#define DPTX_RCV_REPLY_REG                   0x0138
#define DPTX_RCV_REPLY_COUNT                 0x013C

#define DPTX_INTERRUPT                       0x0140
#define DPTX_INTERRUPT_MASK                  0x0144
#define DPTX_RCV_DATA_COUNT                  0x0148
#define DPTX_AUX_TRANSACTION_STATUS          0x014C
#define DPTX_TIMER                           0x0158

//
// Main Link registers
//
#define DPTX_MAIN_LINK_HTOTAL                0x0180
#define DPTX_MAIN_LINK_VTOTAL                0x0184
#define DPTX_MAIN_LINK_POLARITY              0x0188
#define DPTX_MAIN_LINK_HSWIDTH               0x018C
#define DPTX_MAIN_LINK_VSWIDTH               0x0190
#define DPTX_MAIN_LINK_HRES                  0x0194
#define DPTX_MAIN_LINK_VRES                  0x0198
#define DPTX_MAIN_LINK_HSTART                0x019C
#define DPTX_MAIN_LINK_VSTART                0x01A0
#define DPTX_MAIN_LINK_MISC0                 0x01A4
#define DPTX_MAIN_LINK_MISC1                 0x01A8
#define DPTX_M_VID                           0x01AC
#define DPTX_TRANSFER_UNIT_SIZE              0x01B0
#define DPTX_N_VID                           0x01B4
#define DPTX_USER_PIXEL_WIDTH                0x01B8
#define DPTX_DATA_PER_LANE                   0x01BC
#define DPTX_INTERLACED                      0x01C0
#define DPTX_USER_POLARITY                   0x01C4
#define DPTX_USER_CONTROL                    0x01C8

//
// eDP CRC registers
//
#define DPTX_EDP_CRC_ENABLE                  0x01D0
#define DPTX_EDP_CRC_RED                     0x01D4
#define DPTX_EDP_CRC_GREEN                   0x01D8
#define DPTX_EDP_CRC_BLUE                    0x01DC

//
// PHY registers
//
#define DPTX_PHY_RESET                       0x0200
#define DPTX_PHY_PREEMPHASIS_LANE_0          0x0210
#define DPTX_PHY_PREEMPHASIS_LANE_1          0x0214
#define DPTX_PHY_PREEMPHASIS_LANE_2          0x0218
#define DPTX_PHY_PREEMPHASIS_LANE_3          0x021C
#define DPTX_PHY_VOLTAGE_DIFF_LANE_0         0x0220
#define DPTX_PHY_VOLTAGE_DIFF_LANE_1         0x0224
#define DPTX_PHY_VOLTAGE_DIFF_LANE_2         0x0228
#define DPTX_PHY_VOLTAGE_DIFF_LANE_3         0x022C
#define DPTX_PHY_TRANSMIT_PRBS7              0x0230
#define DPTX_PHY_CLOCK_FEEDBACK_SETTING      0x0234
#define DPTX_PHY_POWER_DOWN                  0x0238
#define DPTX_PHY_POSTEMPHASIS_LANE_0         0x0240
#define DPTX_PHY_POSTEMPHASIS_LANE_1         0x0244
#define DPTX_PHY_POSTEMPHASIS_LANE_2         0x0248
#define DPTX_PHY_POSTEMPHASIS_LANE_3         0x024C
#define DPTX_PHY_PLL_SETTINGS                0x0250
#define DPTX_PHY_ELEC_IDLE                   0x0254
#define DPTX_PHY_SSC_ENABLE                  0x0258
#define DPTX_PHY_REPEATER_ENABLE             0x025C
#define DPTX_PHY_STATUS                      0x0280

//
// Secondary channel registers
//
#define DPTX_SEC_AUDIO_ENABLE                0x0300
#define DPTX_SEC_INPUT_SELECT                0x0304
#define DPTX_SEC_CHANNEL_COUNT               0x0308
#define DPTX_SEC_DIRECT_CLKDIV               0x030C
#define DPTX_SEC_INFOFRAME_ENABLE            0x0310
#define DPTX_SEC_INFOFRAME_RATE              0x0314
#define DPTX_SEC_MAUD                        0x0318
#define DPTX_SEC_NAUD                        0x031C
#define DPTX_SEC_AUDIO_CLOCK_MODE            0x0320
#define DPTX_SEC_3D_VSC_DATA                 0x0324
#define DPTX_SEC_AUDIO_FIFO                  0x0328
#define DPTX_SEC_AUDIO_FIFO_DEPTH            0x032C
#define DPTX_SEC_AUDIO_FIFO_READY            0x0330
#define DPTX_SEC_INFOFRAME_SELECT            0x0334
#define DPTX_SEC_INFOFRAME_DATA              0x0338
#define DPTX_SEC_TIMESTAMP_INTERVAL          0x033C
#define DPTX_SEC_CS_SOURCE_FORMAT            0x0340
#define DPTX_SEC_CS_CATEGORY_CODE            0x0344
#define DPTX_SEC_CS_LENGTH_ORIG_FREQ         0x0348
#define DPTX_SEC_CS_FREQ_CLOCK_ACCURACY      0x034C
#define DPTX_SEC_CS_COPYRIGHT                0x0350
#define DPTX_SEC_GTC_COUNT_CONFIG            0x0354
#define DPTX_SEC_GTC_COMMAND_EDGE            0x0358
#define DPTX_SEC_AUDIO_CHANNEL_MAP           0x035C
#define DPTX_SEC_PSR_3D_ENABLE               0x0360
#define DPTX_SEC_PSR_CONFIG                  0x0364
#define DPTX_SEC_PSR_STATE                   0x0368
#define DPTX_SEC_AUDIO_OVERFLOW              0x0370
#define DPTX_SEC_PACKET_COUNT                0x0374

//
// HDCP 1.3 Registers
//
#define DPTX_HDCP13_ENABLE                   0x0400
#define DPTX_HDCP13_MODE                     0x0404
#define DPTX_HDCP13_KS_0                     0x0408
#define DPTX_HDCP13_KS_1                     0x040C
#define DPTX_HDCP13_KM_LOW                   0x0410
#define DPTX_HDCP13_KM_HIGH                  0x0414
#define DPTX_HDCP13_AN_LOW                   0x0418
#define DPTX_HDCP13_AN_HIGH                  0x041C
#define DPTX_HDCP13_RSVD_0                   0x0420
#define DPTX_HDCP13_RSVD_1                   0x0424
#define DPTX_HDCP13_R0                       0x0428
#define DPTX_HDCP13_USE_HARDWARE_KM          0x042C
#define DPTX_HDCP13_BKSV_0                   0x0430
#define DPTX_HDCP13_BKSV_1                   0x0434
#define DPTX_HDCP13_AKSV_0                   0x0438
#define DPTX_HDCP13_AKSV_1                   0x043C
#define DPTX_HDCP13_REPEATER                 0x0450
#define DPTX_HDCP13_STREAM_CIPHER_ENABLE     0x0454
#define DPTX_HDCP13_M0_LOWER                 0x0458
#define DPTX_HDCP13_M0_UPPER                 0x045C
#define DPTX_HDCP13_RNG_STORE_AN             0x0480
#define DPTX_HDCP13_RNG_LOWER                0x0484
#define DPTX_HDCP13_RNG_UPPER                0x0488

//
// HDCP 2.2 Registers
//
#define DPTX_HDCP22_ENABLE                   0x0400 //  HDCP enable
#define DPTX_HDCP22_MODE                     0x0404 //  HDCP mode select (set to a value of 2 for HDCP 2.2)
#define DPTX_HDCP22_KS_0                     0x0408 //  Ks bits 31:0
#define DPTX_HDCP22_KS_1                     0x040c //  Ks bits 63:32
#define DPTX_HDCP22_KS_2                     0x0410 //  Ks bits 95:64
#define DPTX_HDCP22_KS_3                     0x0414 //  Ks bits 127:96
#define DPTX_HDCP22_RTX_0                    0x0418 //  Rtx bits 31:0, session random number
#define DPTX_HDCP22_RTX_1                    0x041c //  Rtx bits 63:32, session random number
#define DPTX_HDCP22_RRX_0                    0x0430 //  Rrx bits 31:0
#define DPTX_HDCP22_RRX_1                    0x0434 //  Rrx bits 63:32
#define DPTX_HDCP22_DKEY_2                   0x0438 //  Dkey bits 95:64
#define DPTX_HDCP22_DKEY_3                   0x043c //  Dkey bits 127:96
#define DPTX_HDCP22_LC128_0                  0x0440 //  Global constant, lc128 bits 31:0
#define DPTX_HDCP22_LC128_1                  0x0444 //  Global constant, lc128 bits 63:32
#define DPTX_HDCP22_LC128_2                  0x0448 //  Global constant, lc128 bits 95:64
#define DPTX_HDCP22_LC128_3                  0x044c //  Global constant, lc128 bits 127:64
#define DPTX_HDCP22_REPEATER                 0x0450 //  Repeater global enable / disable
#define DPTX_HDCP22_STREAM_CYPHER_ENABLE     0x0454 //  Stream cipher enable
#define DPTX_HDCP22_DKEY_0                   0x0458 //  Dkey bits 31:0
#define DPTX_HDCP22_DKEY_1                   0x045c //  Dkey bits 63:32
#define DPTX_HDCP22_AES_INPUT_SELECT         0x0460 //  AES input select
#define DPTX_HDCP22_AES_CTR_DISABLE          0x0464 //  AES counter disable
#define DPTX_HDCP22_AES_CTR_INC              0x0468 //  AES counter increment
#define DPTX_HDCP22_ENCRYPTION_CTL_0         0x046c //  HDCP Encryption Control Field, bits 31:0
#define DPTX_HDCP22_ENCRYPTION_CTL_1         0x0470 //  HDCP Encryption Control Field, bits 63:32
#define DPTX_HDCP22_AES_CTR_RESET            0x0474 //  AES counter reset
#define DPTX_HDCP22_RN_0                     0x0478 //  Rn bits 31:0
#define DPTX_HDCP22_RN_1                     0x047c //  Rn bits 63:32

//
// MST control registers
//
#define DPTX_MST_ENABLE                      0x0500
#define DPTX_MST_PID_TABLE_INDEX             0x0504
#define DPTX_MST_PID_TABLE_DATA              0x0508
#define DPTX_MST_SST_SOURCE                  0x050C
#define DPTX_MST_ACT_TRIGGER                 0x0510
#define DPTX_MST_SOURCE_ACTIVE               0x0520
#define DPTX_MST_LINK_FRAME_COUNT            0x0524

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

#define HPD_CHECK_TIMES                             1
#define HPD_CHECK_INTERVAL_MS                       1

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

#define RETRAINING_TIMES_SAMESTEP                   1
#define RETRAINING_DOWNLINKRATE                     1

/**
  Read phy register through aux channel.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 , 1 or 2.
  @param[in]  DpcdAddr     Dpcd regiter address.Twenty-bit address for the start of the AUX Channel burst
  @param[out] *Data        Pointer to data of reading from dpcd.

  @retval     Dpcd Read Status.
              0            Read successfully.
              1            AUX  Timeout.
**/
UINT32
SinkDpcdRead (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            DpcdAddr,
  OUT UINT8           *Data
  );

/**
  Read phy register through aux channel.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  DpcdAddr     Dpcd regiter address.Twenty-bit address for the start of the AUX Channel burst
  @param[in]  WriteData    The data that you want to write to the sink regiter through AUX channel.

  @retval     Dpcd Read Status.
              0            Read successfully.
              1            AUX  Timeout.
**/
UINT32
SinkDpcdWrite (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT32           DpcdAddr,
  IN UINT8            WriteData
  );

/**
  Get the unique identification code of the core and the current revision level.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[out] Coreid       Pointer to Core ID information.
  @param[out] Corerev      Pointer to Core revision level information.

  @retval     NULL
**/
VOID
CoreIDGet (
  PHY_PRIVATE_DATA  *Private,
  UINT8             Num,
  UINT16            *Coreid,
  UINT16            *Corerev
  );

/**
  Check the HPD status.True is HPD is asserted, false otherwise. The checking
  time interval and times is decided by HPD_CHECK_TIMES and HPC_CHECK_INTERVAL_MS.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     0            HPD is not asserted.
  @retval     1            HPD is asserted.
**/
UINT32
CheckHpdStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

/**
  Check the HPD status once.True is HPD is asserted, false otherwise.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     0            HPD is not asserted.
  @retval     1            HPD is asserted.
**/
UINT32
GetHpdStatusOnce (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

/**
  Initialize AUX channel include aux clock Initialization, dp timer Initialization
  and interrupt mask.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     NULL
**/
UINT32
InitAux (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

/**
  Configurate link rate and lane count of dptx.Lane count is one of 1 ,2 ,4.
  The unit of link rate is GHz.Link rate value is one of 1.62 , 2.7 , 5.4 , 8.1.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  LinkRate     Main link bandwidth.
                           162 - 1.62G
                           270 - 2.7G
                           540 - 5.4G
                           810 - 8.1G
  @param[in]  LaneCount    The Number of lanes , 1.

  @retval     EFI_SUCCESS  Confituration successfully.
**/
UINT32
ConfigDptx (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate,
  IN UINT32            LaneCount
  );

/**
  Configurate link rate and lane count of sink device.Lane count is one of
  1 ,2 ,4.The unit of link rate is GHz.Link rate value is one of 1.62 , 2.7 , 5.4 , 8.1.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  LinkRate     Main link bandwidth , 1.62 , 2.7 , 5.4 , or 8.1.
                           162 - 1.62G
                           270 - 2.7G
                           540 - 5.4G
                           810 - 8.1G
  @param[in]  LaneCount    The Number of lanes , 1.

  @retval     EFI_SUCCESS  Confituration successfully.
**/
UINT32
ConfigSink (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate,
  IN UINT32            LaneCount
  );

/**
  Set sink device to D0(normal operation mode).

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     NULL.
**/
VOID
WakeUpSink (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  );

/**
  sets the output of the transmitter core to the specified training pattern.
  When set, all other main link information such as video and audio data are
  blocked in favor of the training pattern.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  TrainPattern link training pattern.
                           0 - Training off
                           1 - Training pattern 1, clock recovery
                           2 - Training pattern 2, inter-lane alignment and symbol recovery
                           3 - Training pattern 3, inter-lane alignment and symbol recovery
                           4 - Training pattern 4, inter-lane alignment and symbol recovery

  @retval     NULL
**/
VOID
DptxSetTraningPattern (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN TRAINING_PATTERN  TrainPattern
  );

/**
  Enable or disable the internal scrambling function of the DisplayPort transmitter.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  Disable      Switch of internal scrambling function. 1 - disable , 0 - enable.

  @retval     NULL
**/
VOID
DptxDisableScrambling (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            Disable
  );

/**
  Force the transmitter core to use the alternate scrambler reset value.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     NULL
**/
VOID
DptxScramblingRest (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  );

/**
  Config Main Stream Attributes.It is must to reset reset phy link after main
  stream attributes configuration.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  DpConfig     Pointer to dp configuration structure include lane
                           count , clock configuration , video mode.
  @param[in]  DpSyncTemp   Pointer to dp timing parameter structure.

  @retval     NULL
**/
VOID
ConfigMainStreamAttr(
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN DP_CONFIG        *DpConfig,
  IN DP_SYNC          *DpSync
  );

/**
  Enable or disable the enhanced framing mode of DPTX.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  Enable       Switch of enhanced framing mode.1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
EnableFramingMode (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN BOOLEAN          Enable
  );

/*
  Dp connect to sink.

  @param[in]      Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.

  @retval         SINK_CONNECT     Dp training success.
                  SINK_DISCONNECT  Dp training failed.
*/
UINT32
DpConnect (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN     UINT8             Num
  );

/**
 Get edid information form sink.

 @param[in]      Private  Pointer to private data structure.
 @param[in]      Num      the index of dcdp , 0 or 1.
 @param[in,out]  Buffer   Edid information.

 @retval    0        Success.
            other    Failed.
**/
UINT32
GetEdid (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN OUT  UINT8        *Buffer
  );

/**
 Parse edid data and fill the DTD_LIST.

 @param[in]  Buffer  Edid data buffer.
 @param[out] Table   The struct of DTD_LIST.

 @retval    0        Success.
            other    Failed.
**/
EFI_STATUS
ParseEdidDtdList (
  IN  UINT8      *Buffer,
  OUT DTD_LIST   *List
  );

/**
 Traverse the DTD_LIST, and convert the first parameters that meet the resolution,
 to DP sync parameter.

 @param[in]  DTD_LIST  The struct of DTD_LIST.
 @param[in]  GopMode   Gop mode to match.
 @param[out] Sync      Dp sync matched.
 @param[out] Clock     Pixel clock matched.

 @retval    EFI_SUCCESS   Match successfully.
            EFI_NOT_FOUND Not Matched.
**/
EFI_STATUS
FindDpSyncFromDtd (
  IN  DTD_LIST      *List,
  IN  PHY_GOP_MODE  *GopMode,
  OUT DP_SYNC       *Sync,
  OUT UINT32        *Clock
  );

/**
 Traverse the DTD_LIST, and convert the first parameters that meet the resolution,
 to DC sync parameter.

 @param[in]  DTD_LIST  The struct of DTD_LIST.
 @param[in]  GopMode   Gop mode to match.
 @param[out] Sync      Dc sync matched.

 @retval    EFI_SUCCESS   Match successfully.
            EFI_NOT_FOUND Not Matched.
**/
EFI_STATUS
FindDcSyncFromDtd (
  IN  DTD_LIST      *List,
  IN  PHY_GOP_MODE  *GopMode,
  OUT DC_SYNC       *Sync
  );

/*
  Dp link training.

  @param[in]      Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.
  @param[in]      MaxLaneCount     Supported max lane count.
  @param[in]      MaxLinkRate      Supported max link rate.

  @retval         SINK_CONNECT     Dp training success.
                  SINK_DISCONNECT  Dp training failed.
*/
UINT32
DpStartLinkTrain (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            MaxLaneCount,
  IN UINT32           MaxLinkRate
  );
#endif
