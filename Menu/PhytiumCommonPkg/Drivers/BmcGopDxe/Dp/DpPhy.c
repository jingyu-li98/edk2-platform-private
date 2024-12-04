/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include "DpPhy.h"
#include "DpDrv.h"

/**
  This function is used to configure the PHY for the specified link rate. It is
  not requried by all PHY implementations. For those that require it, this
  function should be called when the link rate is changed.The parameter provided
  corresponds to the values defined in the DisplayPort specificaiton for the
  LINK_BW_SET register in the DPCD.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  linkRate     Main link bandwidth in 10Mbps unit.

  @retval     EFI_SUCCESS  Set register successfully.
**/
UINT32
DptxPhyUpdateLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LinkRate
  )
{
  UINT32  LinkRateHex;

  LinkRateHex = LinkRate / 27;
  SinkDpcdWrite (Private, Num, LINK_BW_SET, LinkRateHex);
  return EFI_SUCCESS;
}

/**
  This function performs any PHY-specific operations required to set the lane
  count in the PHY. It is not required by all PHY implementations. For those
  that requrie it, it should be called whenever the lane count is changed.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[in]  LaneCount   The Number of lanes , 1.

  @retval     NULL
**/
VOID
DptxPhySetLaneCount (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LaneCount
  )
{
  SinkDpcdWrite (Private, Num, LANE_COUNT_SET, 0x80 + LaneCount);
}

/**
  Set local sink device and all downstream sink devices to specified status.
  D0 normal opreation mode or D3 power down mode.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  pwrMode      Power state. 1 - D0 , 2 - D3.

  @retval     NULL
**/
VOID
DptxPhySetPower (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN PHY_PWR_MODE     PwrMode
  )
{
  SinkDpcdWrite (Private, Num, SET_POWER_SET_DP_PWR_VALTAGE, PwrMode);
}

/**
  Set voltage swing of a specified lane.Voltage swing has three levels.
  0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[in]  LaneNum     The identifier of the lane you want to operate.
  @param[in]  Vs          Voltage swing.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @retval     NULL
**/
VOID
DptxPhySetVswing (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LaneNum,
  IN UINT8             Vs
  )
{
  UINT8 Value;

  Value = 0;

  SinkDpcdRead (Private, Num, TRAINING_LANE0_SET + LaneNum, &Value);
  Value &= 0xFC;
  Value |= Vs;
  SinkDpcdWrite (Private, Num, TRAINING_LANE0_SET + LaneNum, Value);
}

/**
  Set pre-emphasis level of a specified lane.Pre-emphasis level has three levels.
  0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[in]  LaneNum     The identifier of the lane you want to operate.
  @param[in]  Pe          Pre-emphasis level.
                          0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @retval     NULL
**/
VOID
DptxPhySetPreemphasis (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT32           LaneNum,
  IN UINT32           Pe
  )
{
  UINT8 Value;

  Value = 0;

  SinkDpcdRead (Private, Num, TRAINING_LANE0_SET + LaneNum, &Value);
  Value &= 0xE7;
  Value |= (Pe << 3);
  SinkDpcdWrite (Private, Num, TRAINING_LANE0_SET + LaneNum, Value);
}

/**
  Get voltage swing of a specified lane.Voltage swing has three levels.
  0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  LaneNum      The identifier of the lane you want to operate.

  @retval     0            level 0
  @retval     1            level 1
  @retval     2            level 2
  @retval     3            level 3
**/
UINT8
DptxSourceVswingForValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             LaneNum
  )
{
  UINT8 Value;
  UINT8 Vswing;

  Value = 0;

  if(LaneNum < 2) {
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE0_1, &Value);
  } else {
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE2_3, &Value);
  }
  Vswing = Value >> (4 * (LaneNum % 2));
  Vswing &= 0x03;
  return Vswing;
}

/**
  Get pre-emphasis level of a specified lane.Get pre-emphasis level has three levels.
  0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  LaneNum      The identifier of the lane you want to operate.

  @retval     0            level 0
  @retval     1            level 1
  @retval     2            level 2
  @retval     3            level 3
**/
UINT8
DptxSourcePreemphasisForValue (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            LaneNum
  )
{
  UINT8 Value;
  UINT8 PreEm;

  Value = 0;

  if(LaneNum < 2) {
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE0_1, &Value);
  } else{
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE2_3, &Value);
  }
  PreEm = Value >> (4 * (LaneNum % 2) + 2);
  PreEm &= 0x03;

  return PreEm;
}

/**
  Read interval during Main-Link Training in microsecond unit.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     interval
**/
UINT32
DptxGetTraingRdInterval (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  )
{
  UINT8 Value;
  UINT32 RdInter;

  RdInter = 400;
  Value = 0;

  SinkDpcdRead (Private, Num, TRAINING_AUX_RD_INTERVAL, &Value);
  if(Value > 0) {
    Value *= (RdInter * 10);
  }

  return Value;
}

/**
  Get the status of all lanes.

  @param[in]  Private          Pointer to private data structure.
  @param[in]  Num              the index of dcdp , 0 or 1.
  @param[out] *TrainingStatus  4 bytes, each byte means to one lane status.
                               B0 - lane 0.
                               B1 - lane 1.
                               B2 - lane 2.
                               B3 - lane 3.
                               each byte:
                               bit 0 - CR_DONE.
                               bit 1 - CHANNEL_EQ_DONE
                               bit 2 - SYMBOL_LOCKED

  @retval     0    Success.
*/
UINT8
DptxGetTrainingStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT UINT8            *TrainingStatus
  )
{
  UINT8 Value[2];

  Value[0] = 0;
  Value[1] = 1;

  SinkDpcdRead (Private, Num, LANE0_1_STATUS, &Value[0]);
  SinkDpcdRead (Private, Num, LANE2_3_STATUS, &Value[1]);
  TrainingStatus[0] = (Value[0 ]& 0x07);
  TrainingStatus[1] = ((Value[0] >> 4) & 0x07);
  TrainingStatus[2] = (Value[1] & 0x07);
  TrainingStatus[3] = ((Value[1] >> 4) & 0x07);
  SinkDpcdRead (Private, Num, 0x204, &Value[0]);
  SinkDpcdRead (Private, Num, 0x205, &Value[1]);

  return 0;
}

/**
  Check the overall training status of the specified nameber of lanes and Tps type.

  @param[in]  Private          Pointer to private data structure.
  @param[in]  Num              the index of dcdp , 0 or 1.
  @param[in]  LaneCount        lane count, 1.
  @param[in]  Tpsn             Tps type.
  @param[in]  *Value           Training status of DptxGetTrainingStatus.

  @revtal    1    Success.
             0    Failed.
**/
UINT8
CheckTrainingStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             LaneCount,
  IN UINT8             Tpsn,
  IN UINT8             *Value
  )
{
  UINT8  Status;
  UINT8  Index;

  Status = 0;

  switch (Tpsn) {
  case LINK_TRAINING_TPS1:
    for (Index = 0; Index < LaneCount; Index++) {
      if ((Value[Index] & 0x01) == 0) {
        Status = 0;
        break;
      }
      else {
        Status = 1;
      }
    }
    break;
  case LINK_TRAINING_TPS2:
  case LINK_TRAINING_TPS3:
  case LINK_TRAINING_TPS4:
    for (Index=0; Index < LaneCount; Index++) {
      if (((Value[Index] >> 1) & 0x03) != 0x03) {
        Status = 0;
        break;
      }
      else{
        Status = 1;
      }
    }
    break;
  default:
    break;
  }
  return Status;
}

/*
  Get sink supported max lane count.

  @param[in]  Private          Pointer to private data structure.
  @param[in]  Num              the index of dcdp , 0 or 1.
  @param[out] *LaneCount       Sink supported lane count.

  @revtal    0    Success.
             1    Failed.
*/
UINT32
DptxPhyGetLaneCount (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT UINT8             *LaneCount
  )
{
  UINT32 Status;

  Status = AUX_TIMEOUT;

  Status = SinkDpcdRead (Private, Num, MAX_LANE_COUNT, LaneCount);
  if (Status == AUX_CONNECT) {
    *LaneCount = (*LaneCount) & 0x1F;
    DEBUG ((DEBUG_INFO, "phy max lane count : %d", *LaneCount));
  }
  return Status;
}

/*
  Get sink supported max lane count.

  @param[in]  Private          Pointer to private data structure.
  @param[in]  Num              the index of dcdp , 0 or 1.
  @param[out] *LinkRate        Sink supported link rate in 10MHz unit.

  @revtal    0    Success.
             1    Failed.
*/
UINT32
DptxPhyGetLinkRate (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT UINT32            *LinkRate
  )
{
  UINT8 Value;
  UINT32 Status;

  Status = AUX_TIMEOUT;
  Status = SinkDpcdRead (Private ,Num, MAX_LINK_RATE, &Value);
  if (Status == AUX_CONNECT) {
    switch(Value) {
    case 0x06:
      *LinkRate = 162;
      break;
    case 0x0A:
      *LinkRate = 270;
      break;
    case 0x14:
      *LinkRate = 540;
      break;
    case 0x1E:
      *LinkRate = 810;
      break;
    default:
      *LinkRate = 270;
      break;
    }
    DEBUG ((DEBUG_INFO, "phy max link rate : 0x%02x\n", Value));
  }
  return Status;
}

/*
  Get sink ENHANCED_FRAME_CAP.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.

  @revtal    0    Unsupported.
             1    Supported.
*/
UINT8
DptxPhyGetEnhancedFrameCap (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT8 Value;

  Value = 0;
  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);
  Value = (Value >> 7) & 0x01;

  return Value;
}

/**
  Get sink TPS4 support.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.

  @revtal    0    Unsupported.
             1    Supported.
**/
UINT8
DptxPhyTps4Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT8 Value;
  Value = 0;

  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);

  return (Value >> 5) & 0x01;
}

/**
  Get sink TPS3 support.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.

  @revtal    0    Unsupported.
             1    Supported.
**/
UINT8
DptxPhyTps3Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT8 Value;

  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);

  return (Value >> 6) & 0x01;
}

/*
  Dp phy register read.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.
  @param[in]  PhyAddr  Register offset to read.

  @revtal     Value    Register value.
*/
UINT32
LinkPhyRead (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           PhyAddr
  )
{
  UINT32  Value;

  Value = ChannelRegRead (Private, Num, PhyAddr, DpPhyOperate);
  //DEBUG((DEBUG_INFO,"Link phy Read Addr : %x, Value : %x\n",PhyAddr,Value));
  gBS->Stall (1 * 1000);
  return Value;
}

/*
  Dp phy register write.

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.
  @param[in]  PhyAddr  Register offset to read.
  @param[in]  Data     Data to write.

  @revtal     0        Success.
*/
UINT32
LinkPhyWrite (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           PhyAddr,
  IN UINT32           Data
  )
{
  ChannelRegWrite (Private, Num, PhyAddr, DpPhyOperate, Data);
  //DEBUG((DEBUG_INFO,"Link phy Write Addr : %x, data : %x\n",PhyAddr,Data));
  gBS->Stall (1 * 1000);
  return 0;
}

/**
  Display phy initialization

  @param[in]  Private  Pointer to private data structure.
  @param[in]  Num      the index of dcdp , 0 or 1.
  @param[in]  LinkRate Initialized link rate, in 10KHz unit.

  @retval     Null.
**/
VOID
LinkPhyInit (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32            Num,
  IN UINT32            LinkRate
  )
{
  //
  //reset,reg 40250
  //
  LinkPhyWrite (Private, Num, REG_PHY_RESET_DEASSERT, 0x0);
  //
  //dp mode, reg 40034
  //
  LinkPhyWrite (Private, Num, REG_MODE, 0x3);
  //
  //Pll Enable, reg 40214
  //
  LinkPhyWrite (Private, Num, REG_PLLCLK_EN, 0x1);
  //
  //link reset, reg 40258
  //
  LinkPhyWrite (Private, Num, REG_PHY_RESET_LINK_DEASSERT, 0x1);
  //
  //Data width 20 bits, reg 4021c
  //
  LinkPhyWrite (Private, Num, REG_PMA_DATA_WIDTH, 0x5);
  LinkPhyWrite (Private, Num, REG_SGMII_DPSEL_INIT, 0x1);
  //
  //link reset, reg 40258
  //
  LinkPhyWrite (Private, Num, REG_PHY_RESET_LINK_DEASSERT, 0x1);
  //
  //APB Reset, reg 40250
  //
  LinkPhyWrite (Private, Num, REG_APB_RESET_DEASSERT, 0x1);

  //
  //Phy interal register
  //
  //
  //Config single link
  //
  LinkPhyWrite (Private, Num, PHY_PLL_CFG, 0);
  //
  //Reference PLL frequence
  //
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
    break;
  case 270:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x701);
    break;
  case 540:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x301);
    break;
  case 810:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x200);
    break;
  default:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
    break;
  }
  //
  //Select Analog High Speed Clock
  //
  LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_SEL, 0x0);
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x2);
    break;
  case 270:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x1);
    break;
  case 540:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x0);
    break;
  case 810:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x0);
    break;
  default:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x2);
    break;
  }
  //
  //Select Digital PLL and Data Speed
  //
  LinkPhyWrite (Private, Num, XCVR_DIAG_PLLDRC_CTRL, 0x1);
  //
  //Config PLL for all VCO
  //
  LinkPhyWrite (Private, Num, CMN_PLL0_DSM_DIAG_M0, 0x4);
  LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
  LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_TCTRL, 0x3);
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x061);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x3333);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
  break;
  case 270:
  case 540:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x06c);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x48);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x051);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x036);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  default:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x061);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x3333);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  }
  //
  //Power Control
  //
  LinkPhyWrite (Private, Num, TX_PSC_A0 , 0xfb);
  LinkPhyWrite (Private, Num, TX_PSC_A2, 0x4aa);
  LinkPhyWrite (Private, Num, TX_PSC_A3, 0x4aa);
  LinkPhyWrite (Private, Num, RX_PSC_A0, 0x0);
  LinkPhyWrite (Private, Num, RX_PSC_A2, 0x0);
  LinkPhyWrite (Private, Num, RX_PSC_A3, 0x0);
  LinkPhyWrite (Private, Num, RX_PSC_CAL, 0x0);
  //
  //Transceiver control and diagnosis
  //
  LinkPhyWrite (Private, Num, XCVR_DIAG_BIDI_CTRL, 0xf);
  //
  //Receiver equalization engine
  //
  LinkPhyWrite (Private, Num, RX_REE_GCSM1_CTRL, 0x0);
  LinkPhyWrite (Private, Num, RX_REE_GCSM2_CTRL, 0x0);
  LinkPhyWrite (Private, Num, RX_REE_PERGCSM_CTRL, 0x0);
  //
  //Voltage swing and pre-emphasis
  //
  LinkPhyWrite (Private, Num, TX_DIAG_ACYA, 0x1);
  LinkPhyWrite (Private, Num, TX_TXCC_CTRL, 0x08a4);
  LinkPhyWrite (Private, Num, DRV_DIAG_TX_DRV, 0x0003);
  //LinkPhyWrite (Private, Num, TX_TXCC_MGNFS_MULT_000, 0x0000);
  //LinkPhyWrite (Private, Num, TX_TXCC_CPOST_MULT_00, 0x001c);
  LinkPhyWrite (Private, Num, TX_TXCC_MGNFS_MULT_000, 0x002a);
  LinkPhyWrite (Private, Num, TX_TXCC_CPOST_MULT_00, 0x0000);
  LinkPhyWrite (Private, Num, TX_DIAG_ACYA, 0x0);
  //
  //Release reset signal
  //
  LinkPhyWrite (Private, Num, REG_PHY_RESET_DEASSERT, 0x1);
}

/**
 DP phy date rate change.

 @param[in]  Private  Pointer to private data structure.
 @param[in]  LinkRate Initialized link rate, in 10KHz unit.
 @param[in]  Num      the index of dcdp , 0 or 1.

 @retval     Null.
**/
VOID
LinkPhyChangeRate (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           LinkRate,
  IN UINT32           Num
  )
{
  UINT32 Value;

  //
  //Set power state to A3
  //
  LinkPhyWrite (Private, Num, REG_PMA_POWER_STATE_REQ, 0x8);
  //
  //Disable PLL clock
  //
  LinkPhyWrite (Private, Num, REG_PLLCLK_EN, 0x0);
  //
  //Disable PLL, pll_raw_ctrl
  //
  LinkPhyWrite (Private, Num, PHY_PMA_PLL_RAW_CTRL, 0);
  //
  //Wait disable PLL success
  //
  while (1) {
    Value = LinkPhyRead (Private, Num, PHY_PMA_CMN_CTRL2);
    if (((Value >> 2) & 0x00000001) == 0x01) {
      break;
    }
  }
  //
  //Reference PLL frequence
  //
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
    break;
  case 270:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x701);
    break;
  case 540:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x301);
    break;
  case 810:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x200);
    break;
  default:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
    break;
  }
  //
  //Select Analog High Speed Clock
  //
  LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_SEL, 0x0);
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x2);
    break;
  case 270:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x1);
    break;
  case 540:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x0);
    break;
  case 810:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x0);
    break;
  default:
    LinkPhyWrite (Private, Num, XCVR_DIAG_HSCLK_DIV, 0x2);
    break;
  }
  //
  //Select Digital PLL and Data Speed
  //
  LinkPhyWrite (Private, Num, XCVR_DIAG_PLLDRC_CTRL, 0x1);
  //
  //Config PLL for all VCO
  //
  LinkPhyWrite (Private, Num, CMN_PLL0_DSM_DIAG_M0, 0x4);
  LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
  LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_TCTRL, 0x3);
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x061);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x3333);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
  break;
  case 270:
  case 540:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x06c);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x48);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x051);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x036);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  default:
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
    LinkPhyWrite (Private, Num, CMN_PLL0_INTDIV_M0, 0x061);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVL_M0, 0x3333);
    LinkPhyWrite (Private, Num, CMN_PLL0_FRACDIVH_M0, 0x0);
    LinkPhyWrite (Private, Num, CMN_PLL0_HIGH_THR_M0, 0x42);
    LinkPhyWrite (Private, Num, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
    LinkPhyWrite (Private, Num, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
    LinkPhyWrite (Private, Num, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
    break;
  }
  //
  //Enable pll , pll_raw_ctrl
  //
  LinkPhyWrite (Private, Num, PHY_PMA_PLL_RAW_CTRL, 1);
  //
  //Enable pll clock and Set power state to A1
  //
  LinkPhyWrite (Private, Num, REG_PLLCLK_EN, 0x1);
  LinkPhyWrite (Private, Num, REG_PMA_POWER_STATE_REQ, 0x1);
}

#if 0
UINT32  MgnfsTable[4][4] = {{0x002A,0x001F,0x0012,0x0000},
                           {0x001F,0x0013,0x0000,0x0000},
                           {0x0013,0x0000,0x0000,0x0000},
                           {0x0000,0x0000,0x0000,0x0000}};
#endif

UINT32  MgnfsTable[4][4] = {{0x0026, 0x0013, 0x0012, 0x0000},
                            {0x0013, 0x0006, 0x0000, 0x0000},
                            {0x0006, 0x0000, 0x0000, 0x0000},
                            {0x0000, 0x0000, 0x0000, 0x0000}};

UINT32  CpostTable[4][4] = {{0x0000, 0x0014, 0x0020, 0x002A},
                            {0x0000, 0x0012, 0x001F, 0x0000},
                            {0x0000, 0x0013, 0x0000, 0x0000},
                            {0x0000, 0x0000, 0x0000, 0x0000}};

/**
  Voltage swing and pre-emphasis training.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[in]  VsWing      Swing level, 0 - 3.
  @param[in]  PreEmphasis Pre-emphasis level, 0 - 3.

  @revtal     NULL.
**/
VOID
LinkPhyChangeVsWing (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           VsWing,
  IN UINT32           PreEmphasis
  )
{
  LinkPhyWrite (Private, Num, TX_DIAG_ACYA, 0x1);
  LinkPhyWrite (Private, Num, TX_TXCC_CTRL, 0x08a4);
  LinkPhyWrite (Private, Num, DRV_DIAG_TX_DRV, 0x0003);
  LinkPhyWrite (Private, Num, TX_TXCC_MGNFS_MULT_000, MgnfsTable[VsWing][PreEmphasis]);
  LinkPhyWrite (Private, Num, TX_TXCC_CPOST_MULT_00, CpostTable[VsWing][PreEmphasis]);
  LinkPhyWrite (Private, Num, TX_DIAG_ACYA, 0x0);
  gBS->Stall (1 * 1000);
}

/**
  Get Swing and pre-emphasis level form sink.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[out] Swing       Swing level of sink, max 4 lane.
  @param[out] PreEmphasis Pre-emphasis level of sink, max 4 lane.

  @retval     0    SUCCESS.
**/
UINT32
DptxPhyGetAdjustRequest (
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT UINT8             *Swing,
  OUT UINT8             *PreEmphasis
  )
{
  UINT8  Value[2];
  SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE0_1, &Value[0]);
  SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE2_3, &Value[1]);
  Swing[0] = Value[0] & 0x03;
  Swing[1] = (Value[0] >> 4) & 0x03;
  Swing[2] = Value[1] & 0x03;
  Swing[3] = (Value[1] >> 4) & 0x03;
  PreEmphasis[0] = (Value[0] >> 2) & 0x03;
  PreEmphasis[1] = (Value[0] >> 6) & 0x03;
  PreEmphasis[2] = (Value[1] >> 2) & 0x03;
  PreEmphasis[3] = (Value[1] >> 6) & 0x03;
  return 0;
}

/**
  Set Swing and pre-emphasis level to sink.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.
  @param[in]  Swing       Swing level of sink, max 4 lane.
  @param[in]  PreEmphasis Pre-emphasis level of sink, max 4 lane.
  @param[in]  LaneCount   Current lane count, 1 , 2 or 4

  @retval     0    SUCCESS.
**/
UINT32
DptxSetTrainingPar (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             *Swing,
  IN UINT8             *PreEmphasis,
  IN UINT8             LaneCount
  )
{
  UINT8  Value;

  Value = 0;

  LinkPhyChangeVsWing (Private, Num, Swing[0], PreEmphasis[0]);
  switch (LaneCount) {
  case 1:
    Value = 0;
    Value |= Swing[0] & 0x03;
    Value |= (PreEmphasis[0] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE0_SET, Value);
    break;
  case 2:
    Value = 0;
    Value |= Swing[0] & 0x03;
    Value |= (PreEmphasis[0] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE0_SET, Value);
    Value = 0;
    Value |= Swing[1] & 0x03;
    Value |= (PreEmphasis[1] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE1_SET, Value);
    break;
  case 4:
    Value = 0;
    Value |= Swing[0] & 0x03;
    Value |= (PreEmphasis[0] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE0_SET, Value);
    Value = 0;
    Value |= Swing[1] & 0x03;
    Value |= (PreEmphasis[1] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE1_SET, Value);
    Value = 0;
    Value |= Swing[2] & 0x03;
    Value |= (PreEmphasis[2] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE2_SET, Value);
    Value = 0;
    Value |= Swing[3] & 0x03;
    Value |= (PreEmphasis[3] & 0x03) << 3;
    SinkDpcdWrite (Private, Num, TRAINING_LANE3_SET, Value);
    break;
  default:
    break;
  }
  return 0;
}

/**
  Determine whether retraining is needed at present.

  @param[in]  Private     Pointer to private data structure.
  @param[in]  Num         the index of dcdp , 0 or 1.

  @retval     1    Need retraining.
              0    Not need retraining.
**/
UINT32
SinkNeedReTrain (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Index;
  UINT32 Status;
  UINT8  Value[4];
  UINT32  LaneCount;

  Status = 0;
  if (Private->DpConfig[Num].LaneCount == 0) {
    return 1;
  }
  if ((UINT32)(Private->DpConfig[Num].LinkRate) == 0) {
    return 1;
  }
  DEBUG((DEBUG_INFO, "Sink Need Retrain? lanecount : %d, linkrate : %d\n",
    Private->DpConfig[Num].LaneCount, (UINT32)Private->DpConfig[Num].LinkRate));
  DptxGetTrainingStatus (Private, Num, Value);
  LaneCount = Private->DpConfig[Num].LaneCount;
  for (Index = 0; Index < LaneCount; Index++) {
    if ((Value[Index] & 0x07) != 0x07) {
      DEBUG ((DEBUG_INFO, "sink need retrain!!\n"));
      Status = 1;
      break;
    }
    else {
      DEBUG ((DEBUG_INFO, "sink need not retrain!!\n"));
      Status = 0;
    }
  }
  return Status;
}
