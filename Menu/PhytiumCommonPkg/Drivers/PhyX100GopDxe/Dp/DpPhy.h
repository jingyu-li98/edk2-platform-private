/* @file
** DpPhy.h
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#ifndef    _DP_PHY_H_
#define    _DP_PHY_H_
#include "PhyGopDxe.h"
/*
** DPCD Address
*/
#define    DPCD_REV                          0x00000
#define    MAX_LINK_RATE                     0x00001
#define    MAX_LANE_COUNT                    0x00002
#define    MAX_DOWNSPREAD                    0x00003
#define    NORP_DP_PWR_VOLTAGE_CAP           0x00004
#define    DOWN_STRAM_PORT_PRESENT           0x00005
#define    MAIN_LINK_CHANNEL_CODING          0x00006
#define    DOWN_STREAM_PORT_COUNT            0x00007
#define    RECEIVE_PORT0_CAP_0               0x00008
#define    RECEIVE_PORT0_CAP_1               0x00009
#define    RECEIVE_PORT1_CAP_0               0x0000A
#define    RECEIVE_PORT1_CAP_1               0x0000B
#define    I2C_SPEED_CONTROL_CAPABILITIES    0x0000C
#define    EDP_CONFIGURATION_CAP             0x0000D
#define    TRAINING_AUX_RD_INTERVAL          0x0000E
#define    ADAPTOR_CAP                       0x0000F

#define    MSTM_CAP                          0x00021
#define    NUMBER_OF_AUDIO_ENDPOINTS         0x00022
#define    AV_SYNC_DATA_BLOCK_DEC            0x00023
#define    AV_SYNC_DATA_BLOCK_DEC_LOW        0x00024
#define    AV_SYNC_DATA_BLOCK_DEC_HIGH       0x00025
#define    AV_SYNC_DATA_BLOCK_PP_LOW         0x00026
#define    AV_SYNC_DATA_BLOCK_PP_HIGH        0x00027
#define    AV_SYNC_DATA_BLOCK_INTER          0x00028
#define    AV_SYNC_DATA_BLOCK_PROG           0x00029
#define    AV_SYNC_DATA_BLOCK_REP_LOW        0x0002A
#define    AV_SYNC_DATA_BLOCK_DEL_LOW        0x0002B
#define    AV_SYNC_DATA_BLOCK_DEL_MID        0x0002C
#define    AV_SYNC_DATA_BLOCK_DEL_HIGH       0x0002D

#define    RX_GTC_VALUE_0                    0x00054
#define    RX_GTC_VALUE_1                    0x00055
#define    RX_GTC_VALUE_2                    0X00056
#define    RX_GTC_VALUE_3                    0X00057
#define    RX_GTC_MSTR_REQ                   0x00058
#define    RX_GTC_FREQ_LOCK_DONE             0x00059
#define    RX_GTC_PHASE_SKEW_OFFSET_0        0x0005A
#define    RX_GTC_PHASE_SKEW_OFFSET_1        0x0005B
#define    DSC_SUPPORT                       0x00060
#define    DSC_ALGORITHM_REVISION            0x00061
#define    DSC_RC_BUFFER_CLOCK_SIZE          0x00062
#define    DSC_RC_BUFFER_SIZE                0x00063
#define    DSC_SLICE_BAPABILITIES_1          0x00064
#define    DSC_LINE_BUFFER_BLT_DEPTH         0x00065
#define    DSC_BLOCK_PREDICTION_SUPPORT      0x00066
#define    MAX_BITS_PER_PIXEL_SUPPORTED      0x00067
#define    DSC_COLOR_FORMAT_CAPABILITIES     0x00069
#define    DSC_COLOR_DEPTH_CAPABILITIES      0x0006A
#define    PEAK_DSC_THROUGHPUT               0x0006B
#define    DSC_MAX_SLICE_WIDTH               0x0006C
#define    DSC_SLICE_CAPABILITIES_2          0x0006D
#define    MIN_BITS_PER_PIXEL_SUPPORTED      0x0006E

#define    PANELSELFREF_CAPSUP_REV           0x00070
#define    PANEL_SELF_REFRESH_CAPABILITIES   0x00071
#define    DETAILED_CAPABILITIES_INFO        0x00080    //80-8F
#define    FEC_CAPABILITY                    0x00090

#define    LINK_BW_SET                       0x00100
#define    LANE_COUNT_SET                    0x00101
#define    TRAINING_PATTERN_SET              0x00102
#define    TRAINING_LANE0_SET                0x00103
#define    TRAINING_LANE1_SET                0x00104
#define    TRAINING_LANE2_SET                0x00105
#define    TRAINING_LANE3_SET                0x00106
#define    DOWNSPREAD_CTRL                   0x00107
#define    MAIN_LINK_CHANNEL_CODING_SET      0x00108
#define    I2C_SPEED_CONTROL_STATUS          0x00109
#define    EDP_CONFIGURATION_SET             0x0010A
#define    LINK_QUAL_LANE0_SET               0x0010B
#define    LINK_QUAL_LANE1_SET               0x0010C
#define    LINK_QUAL_LANE2_SET               0x0010D
#define    LINK_QUAL_LANE3_SET               0x0010E
#define    MSTM_CTRL                         0x00111
#define    AUDIO_DELAY_7_0                   0x00112
#define    AUDIO_DELAY_15_8                  0x00113
#define    AUDIO_DELAY_23_16                 0x00114
#define    LINK_RATE_SET_TX_GTC_CAPABILITY   0x00115
#define    UPSTREAM_DEVICE_DP_PWR_NEED       0x00118
#define    EXTENDED_DPRX_SLEEP_WAKE_TIMEOUT_GRANT    0x00119
#define    FEC_CONFIGURATION                 0x00120
#define    TX_GTC_VALUE_7_0                  0x00154
#define    TX_GTC_VALUE_15_8                 0x00155
#define    TX_GTC_VALUE_23_16                0x00156
#define    TX_GTC_VALUE_31_24                0x00157
#define    RX_GTC_VALUE_PHASE_SKEW_EN        0x00158
#define    TX_GTC_FREQ_LOCK_DONE             0x00159
#define    TX_GTC_PHASE_CKEW_OFFERT_7_0      0x0015A
#define    TX_GTC_PHASE_CKEW_OFFERT_15_8     0x0015B
#define    DSC_ENABLE                        0x00160
#define    ADAPTOR_CTRL                      0x001A0
#define    BRANCH_DEVICE_CTRL                0x001A1
#define    PAYLOAD_ALLOCATE_SET              0x001C0
#define    PAYLOAD_ALLOCATE_START_TIME_SLOT  0x001C1
#define    PAYLOAD_ALLOCATE_TIME_SLOT_COUNT  0x001C2

#define    SINK_COUNT                        0x00200
#define    DEVICE_SERVICE_IRQ_VECTOR         0x00201
#define    LANE0_1_STATUS                    0x00202
#define    LANE2_3_STATUS                    0x00203
#define    LANE_ALIGN_STATUS_UPDATED         0x00204
#define    SINK_STATUS                       0x00205
#define    ADJUST_REQUEST_LANE0_1            0x00206
#define    ADJUST_REQUEST_LANE2_3            0x00207
#define    TRAINING_SCORE_LANE0              0x00208
#define    TRAINING_SCORE_LANE1              0x00209
#define    TRAINING_SCORE_LANE2              0x0020A
#define    TRAINING_SCORE_LANE3              0x0020B
#define    SYMBOL_ERROR_COUNT_LANE0_LOW      0x00210
#define    SYMBOL_ERROR_COUNT_LANE0_HIGH     0x00211
#define    SYMBOL_ERROR_COUNT_LANE1_LOW      0x00212
#define    SYMBOL_ERROR_COUNT_LANE1_HIGH     0x00213
#define    SYMBOL_ERROR_COUNT_LANE2_LOW      0x00214
#define    SYMBOL_ERROR_COUNT_LANE2_HIGH     0x00215
#define    SYMBOL_ERROR_COUNT_LANE3_LOW      0x00216
#define    SYMBOL_ERROR_COUNT_LANE3_HIGH     0x00217
#define    TEST_REQUEST                      0x00208
#define    TEST_LINK_RATE                    0x00219
#define    TEST_LANE_COUNT                   0x00220
#define    TEST_PATTERN                      0x00221
#define    TEST_H_TOTAL_HIGH                 0x00222
#define    TEST_H_TOTAL_LOW                  0x00223
#define    TEST_V_TOTAL_HIGH                 0x00224
#define    TEST_V_TOTAL_LOW                  0x00225
#define    TEST_H_START_HIGH                 0x00226
#define    TEST_H_START_LOW                  0x00227
#define    TEST_V_START_HIGH                 0x00228
#define    TEST_V_START_LOW                  0x00229
#define    TEST_HSYNC_HIGH                   0x0022A
#define    TEST_HSYNC_LOW                    0x0022B
#define    TEST_VSYNC_HIGH                   0x0022C
#define    TEST_VSYNC_LOW                    0x0022D
#define    TEST_H_WIDTH_HIGH                 0x0022E
#define    TEST_H_WIDTH_LOW                  0x0022F
#define    TEST_V_HEIGHT_HIGH                0x00230
#define    TEST_V_HEIGHT_LOW                 0x00231
#define    TEST_MISC_LOW                     0x00232
#define    TEST_MISC_HIGH                    0x00233
#define    TEST_REFRESH_RATE_NUMERATOR       0x00234
#define    TEST_CRC_R_CR_LOW                 0x00240
#define    TEST_CRC_R_CR_HIGH                0x00241
#define    TEST_CRC_G_Y_LOW                  0x00242
#define    TEST_CRC_G_Y_HIGH                 0x00243
#define    TEST_CRC_B_CB_LOW                 0x00244
#define    TEST_CRC_B_CB_HIGH                0x00245
#define    TEST_CRC_COUNT                    0x00246
#define    PHY_TEST_PATTERN                  0x00248
#define    HBR2_COMPLIANCE_SCRAMBLER_RESET_7_0    0x0024A
#define    HBR2_COMPLIANCE_SCRAMBLER_RESET_15_8   0x0024B
#define    TEST_80BIT_CUSTOM_PATTERN_7_0     0x00250
#define    TEST_80BIT_CUSTOM_PATTERN_15_8    0x00251
#define    TEST_80BIT_CUSTOM_PATTERN_23_16   0x00252
#define    TEST_80BIT_CUSTOM_PATTERN_31_24   0x00253
#define    TEST_80BIT_CUSTOM_PATTERN_39_32   0x00254
#define    TEST_80BIT_CUSTOM_PATTERN_47_40   0x00255
#define    TEST_80BIT_CUSTOM_PATTERN_55_48   0x00256
#define    TEST_80BIT_CUSTOM_PATTERN_63_56   0x00257
#define    TEST_80BIT_CUSTOM_PATTERN_71_64   0x00258
#define    TEST_80BIT_CUSTOM_PATTERN_79_72   0x00259
#define    CONTINUOUS_80BIT_PATTERN_FROM_DPRX_AUX_CH_CAP    0x0025A
#define    CONTINUOUS_80BIT_PATTERN_FROM_DPRX_AUX_CH_CTRL   0x0025B
#define    TEST_RESPONSE                     0x00260
#define    TEST_EDID_CHECHSUM                0x00261
#define    TEST_SINK                         0x00270
#define    TEST_AUDIO_MODE                   0x00271
#define    TEST_AUDIO_PATTERN_TYPE           0x00272
#define    TEST_AUDIO_PERIOD_CH_1            0x00273
#define    TEST_AUDIO_PERIOD_CH_2            0x00274
#define    TEST_AUDIO_PERIOD_CH_3            0x00275
#define    TEST_AUDIO_PERIOD_CH_4            0x00276
#define    TEST_AUDIO_PERIOD_CH_5            0x00277
#define    TEST_AUDIO_PERIOD_CH_6            0x00278
#define    TEST_AUDIO_PERIOD_CH_7            0x00279
#define    TEST_AUDIO_PERIOD_CH_8            0x0027A
#define    FEC_STATUS                        0x00280
#define    FEC_ERROR_COUNT0                  0x00281
#define    FEC_ERROR_COUNT1                  0x00282
#define    PAYLOAD_TABLE_UPDATE_STATUS       0x002C0

#define    IEEE_OUI                          0x00300
#define    DEVICE_ID_STRING                  0x00303
#define    HARDWARE_REVISION                 0x00309
#define    FIRMWARE_SOFTWARE_MAJOR_REV       0x0030A
#define    FIRMWARE_SOFTWARE_MINOR_REV       0x0030B

#define    SET_POWER_SET_DP_PWR_VALTAGE      0x00600

#define    SINK_COUNT_ESI                    0x02002
#define    DEVICE_SERVICE_IRQ_VECTOR_ESI0    0x02003
#define    DEVICE_SERVICE_IRQ_VECTOR_ESI1    0x02004
#define    LINK_SERVICE_IRQ_VECTOR_ESI0      0x02005
#define    LANE0_1_STATUS_ESI                0x0200C
#define    LANE2_3_STATUS_ESI                0x0200D
#define    LANE_ALIGN_STATUS_UPDATED_ESI     0x0200E
#define    SINK_STATUS_ESI                   0x0200F

#define    LINK_TRAINING_TPS1                0x01
#define    LINK_TRAINING_TPS2                0x02
#define    LINK_TRAINING_TPS3                0x03
#define    LINK_TRAINING_TPS4                0x07

UINT8
TrainingLaneSe (
  IN UINT8 Adjust
  );

UINT8
IsAdjReqChanged (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );

UINT32
DptxPhyUpdateLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate
  );

VOID
DptxPhySetLaneCount (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT32           LaneCount
  );

VOID
DptxPhySetPower (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN PHY_PWR_MODE     PwrMode
  );

VOID
DptxPhySetVswing (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LaneNum,
  IN UINT8             Vs
  );

VOID
DptxPhySetPreemphasis (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT32           LaneNum,
  IN UINT32           Pe
  );

UINT8
DptxSourceVswingForValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             LaneNum
  );

UINT8
DptxSourcePreemphasisForValue (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            LaneNum
  );

UINT32
DptxGetTraingRdInterval (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  );

UINT8
DptxGetTrainingStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT UINT8            *TrainingStatus
  );

UINT32
DptxPhyGetLaneCount (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num,
  OUT UINT8        *LaneCount
  );


UINT32
DptxPhyGetLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num,
  OUT UINT32       *LinkRate
  );


UINT8
DptxPhyGetEnhancedFrameCap (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  );

UINT8
DptxPhyTps4Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  );

UINT8
DptxPhyTps3Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  );

UINT8
CheckTrainingStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num,
  IN UINT8         LaneCount,
  IN UINT8             Tpsn,
  IN UINT8             *Value
  );


UINT32
LinkPhyInit (
  IN PHY_PRIVATE_DATA *Private,
  UINT32              LaneCount,
  UINT32              LinkRate,
  UINT32              Num
  );

UINT32
LinkPhyChangeRate (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           LinkRate,
  IN UINT32           Num
  );

UINT32
LinkPhyAllInit (
  IN PHY_PRIVATE_DATA *Private
  );


UINT32
LinkPhyChangeVsWing (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           VsWing,
  IN UINT32           PreEmphasis,
  IN UINT32           Num
  );

UINT32
DptxSetTrainingPar (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             *Swing,
  IN UINT8             *PreEmphasis,
  IN UINT8             LaneCount
  );


UINT32
DptxPhyGetAdjustRequest (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT UINT8            *Swing,
  OUT UINT8            *PreEmphasis
  );

UINT32
IsPhy2Dp (
  IN PHY_PRIVATE_DATA  *Private
  );

UINT32
SinkNeedReTrain (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  );
#endif
