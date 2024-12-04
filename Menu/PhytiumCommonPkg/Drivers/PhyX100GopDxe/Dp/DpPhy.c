/* @file
** DpPhy.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/
#include "DpDrv.h"
#include "DpPhy.h"

/**
  This function is used to configure the PHY for the specified link rate. It is not requried by all PHY implementations. For those
  that require it, this function should be called when the link rate is changed.The parameter provided corresponds to the values
  defined in the DisplayPort specificaiton for the LINK_BW_SET register in the DPCD.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  linkRate     Main link bandwidth in Gbps unit, 1.62 , 2.16 , 2.43 , 2.7 , 3.24 , 4.32 , 5.4 , or 8.1.

  @retval     EFI_SUCCESS  Set register successfully.
**/
UINT32
DptxPhyUpdateLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32             LinkRate
  )
{
  UINT32  LinkRateHex;
  LinkRateHex = LinkRate / 27;
  SinkDpcdWrite (Private, Num, LINK_BW_SET, LinkRateHex);
  return EFI_SUCCESS;
}

/**
  This function performs any PHY-specific operations required to set the lane count in the PHY. It is not required by all PHY
  implementations. For those that requrie it, it should be called whenever the lane count is changed.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_count   The Number of lanes , 1 , 2 or 4.

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
  Set local sink device and all downstream sink devices to specified status.D0 normal opreation mode or D3 power down mode.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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
  Set voltage swing of a specified lane.Voltage swing has three levels.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_Num     The identifier of the lane you want to operate.

  @param[in]  vs           Voltage swing.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

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
  Set pre-emphasis level of a specified lane.Pre-emphasis level has three levels.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_Num     The identifier of the lane you want to operate.

  @param[in]  pe           Pre-emphasis level.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

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
  Get voltage swing of a specified lane.Voltage swing has three levels.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_Num     The identifier of the lane you want to operate.

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
  }
  else {
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE2_3, &Value);
  }
  Vswing = Value >> (4 * (LaneNum % 2));
  Vswing &= 0x03;
  return Vswing;
}

/**
  Get pre-emphasis level of a specified lane.Get pre-emphasis level has three levels.0 - level 0 , 1- level 1 , 2 - level 2 , 3 - level 3.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_Num     The identifier of the lane you want to operate.

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
  }
  else{
    SinkDpcdRead (Private, Num, ADJUST_REQUEST_LANE2_3, &Value);
  }
  PreEm = Value >> (4 * (LaneNum % 2) + 2);
  PreEm &= 0x03;
  return PreEm;
}

/**
  Read interval during Main-Link Training in microsecond unit.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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
  Get the status of a specified lane.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_Num     The identifier of the lane you want to operate.

  @retval     lane status  bit 0 dr_done
                           bit 1 channel_eq_done
                           bit 2 symboOUT UINT8            *TrainingStatus
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

/*
* 1- success
* 0 - failed
*/
UINT8
CheckTrainingStatus (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num,
  IN UINT8         LaneCount,
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
/*    for (Index = 0; Index < LaneCount; Index++) {
      if (((Value[Index] >> 2) & 0x01) == 0) {
        Status = 0;
        break;
      }
      else {
        Status = 1;
      }
    }
    break;*/
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

UINT32
DptxPhyGetLaneCount (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8          Num,
  OUT UINT8            *LaneCount
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

UINT32
DptxPhyGetLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num,
  OUT UINT32           *LinkRate
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
* 0 - unsupported
* 1 - supported
*
*/
UINT8
DptxPhyGetEnhancedFrameCap (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  )
{
  UINT8 Value = 0;
  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);
  Value = (Value >> 7) & 0x01;
  return Value;
}

//1- support  0 - unsupport
UINT8
DptxPhyTps4Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  )
{
  UINT8 Value = 0;
  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);
  return (Value >> 5) & 0x01;
}

//1- support  0 - unsupport
UINT8
DptxPhyTps3Supported (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8         Num
  )
{
  UINT8 Value;
  SinkDpcdRead (Private, Num, MAX_LANE_COUNT, &Value);
  return (Value >> 6) & 0x01;
}

UINT32
LinkPhyRead (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           PhyAddr
  )
{
  UINT32  Value;
  PhyWriteMmio (Private, Num, 0x5000, PhyAddr);
  PhyWriteMmio (Private, Num, 0x500C, 0x2);
  gBS->Stall (100);
  Value = PhyReadMmio (Private, Num, 0x5008);
  gBS->Stall (10);
  //DEBUG((DEBUG_INFO,"Link phy Read Addr : %x, Value : %x\n",PhyAddr,Value));
  return Value;
}

UINT32
LinkPhyWrite (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           PhyAddr,
  IN UINT32           Data
  )
{
  PhyWriteMmio (Private, Num, 0x5000, PhyAddr);
  PhyWriteMmio (Private, Num, 0x5004, Data);
  PhyWriteMmio (Private, Num, 0x500C, 0x1);
  //DEBUG((DEBUG_INFO,"Link phy Write Addr : %x, data : %x\n",PhyAddr,Data));
  gBS->Stall (10);
  return 0;
}

/*
*  1 - phy2 is dp
*  0 - phy2 is not dp
*/
UINT32
IsPhy2Dp (
  IN PHY_PRIVATE_DATA  *Private
)
{
  UINT32 Value;
  Value = LinkPhyRead (Private, 2, 0x100004);
  Value = (Value >> 2);
  Value &= 0x1;
  if(Value == 1)
    return 0;
  else
    return 1;
}

UINT32
LinkPhyAllInit (
  IN PHY_PRIVATE_DATA  *Private
  )
{
  UINT32  Value;
  UINT32  LinkRate[3];
  LinkRate[0] = 540;
  LinkRate[1] = 540;
  LinkRate[2] = 540;
  //dp0 & dp1
  LinkPhyWrite (Private, 0, 0x40104, 0x0);
  LinkPhyWrite (Private, 0, 0x40088, 0xffff);
  LinkPhyWrite (Private, 0, 0x40010, 0x00ff);
  //test
  LinkPhyWrite (Private, 0, 0x40044, 0x00ff);
  //test
  LinkPhyWrite (Private, 0, 0x40020, 0x55555555);
  LinkPhyWrite (Private, 0, 0x40014, 0x01010101);
  LinkPhyWrite (Private, 0, 0x40018, 0x01010101);
  LinkPhyWrite (Private, 0, 0x40108 ,0x3 );
  LinkPhyWrite (Private, 0, 0x40100 ,0x1 );
  LinkPhyWrite (Private, 0, 0x30038 ,0x2 );
  switch (LinkRate[0]) {
  case 162:
    LinkPhyWrite (Private, 0, 0x684, 0xf01);
    break;
  case 270:
    LinkPhyWrite (Private, 0, 0x684, 0x701);
    break;
  case 540:
    LinkPhyWrite (Private, 0, 0x684, 0x301);
    break;
  case 810:
    LinkPhyWrite (Private, 0, 0x684, 0x200);
    break;
  default:
    break;
  }
  switch (LinkRate[1]) {
  case 162:
    LinkPhyWrite (Private, 1, 0x704, 0xf01);
    break;
  case 270:
    LinkPhyWrite (Private, 1, 0x704, 0x701);
    break;
  case 540:
    LinkPhyWrite (Private, 1, 0x704, 0x301);
    break;
  case 810:
    LinkPhyWrite (Private, 1, 0x704, 0x200);
    break;
  default:
    break;
  }

  LinkPhyWrite (Private, 0, 0x18398, 0x0);
  LinkPhyWrite (Private, 0, 0x1A398, 0x1);

  switch (LinkRate[0]) {
  case 162:
    LinkPhyWrite (Private, 0, 0x1839c, 0x2);
    break;
  case 270:
    LinkPhyWrite (Private, 0, 0x1839c, 0x1);
    break;
  case 540:
    LinkPhyWrite (Private, 0, 0x1839c, 0x0);
    break;
  case 810:
    LinkPhyWrite (Private, 0, 0x1839c, 0x0);
    break;
  default:
    LinkPhyWrite (Private, 0, 0x1839c, 0x1);
    break;
  }
  switch (LinkRate[1]) {
  case 162:
    LinkPhyWrite (Private, 1, 0x1A39c, 0x2);
    break;
  case 270:
    LinkPhyWrite (Private, 1, 0x1A39c, 0x1);
    break;
  case 540:
    LinkPhyWrite (Private, 1, 0x1A39c, 0x0);
    break;
  case 810:
    LinkPhyWrite (Private, 1, 0x1A39c, 0x0);
    break;
  default:
    LinkPhyWrite (Private, 1, 0x1A39c, 0x1);
    break;
  }
  LinkPhyWrite (Private, 0, 0x18394, 0x1);
  LinkPhyWrite (Private, 1, 0x1A394, 0x9);

  LinkPhyWrite (Private, 0, 0x250, 0x4);
  LinkPhyWrite (Private, 0, 0x218, 0xc5e);
  LinkPhyWrite (Private, 0, 0x208, 0x3);
  LinkPhyWrite (Private, 1, 0x350, 0x4);
  LinkPhyWrite (Private, 1, 0x318, 0xc5e);
  LinkPhyWrite (Private, 1, 0x308, 0x3);

  switch (LinkRate[0]) {
  case 162:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x061);
    LinkPhyWrite (Private, 0, 0x244, 0x3333);
    LinkPhyWrite (Private, 0, 0x248, 0x0);
    LinkPhyWrite (Private, 0, 0x24c, 0x42);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  case 270:
  case 540:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x06c);
    LinkPhyWrite (Private, 0, 0x244, 0x0);
    LinkPhyWrite (Private, 0, 0x248, 0x0);
    LinkPhyWrite (Private, 0, 0x24c, 0x48);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x051);
    LinkPhyWrite (Private, 0, 0x244, 0x0);
    LinkPhyWrite (Private, 0, 0x248, 0x036);
    LinkPhyWrite (Private, 0, 0x24c, 0x42);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  default:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x06c);
    LinkPhyWrite (Private, 0, 0x244, 0x0);
    LinkPhyWrite (Private, 0, 0x248, 0x0);
    LinkPhyWrite (Private, 0, 0x24c, 0x48);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  }
  LinkPhyWrite (Private, 0, 0x18400, 0xfb);
  LinkPhyWrite (Private, 0, 0x18408, 0x4aa);
  LinkPhyWrite (Private, 0, 0x1840c, 0x4aa);
  LinkPhyWrite (Private, 0, 0x28000, 0x0);
  LinkPhyWrite (Private, 0, 0x28008, 0x0);
  LinkPhyWrite (Private, 0, 0x2800c, 0x0);
  LinkPhyWrite (Private, 0, 0x28018, 0x0);
  LinkPhyWrite (Private, 0, 0x183a8, 0xf);
  LinkPhyWrite (Private, 0, 0x28420, 0x0);
  LinkPhyWrite (Private, 0, 0x28440, 0x0);
  LinkPhyWrite (Private, 0, 0x28460, 0x0);
  //change vol
  //LinkPhyWrite(Private,0,0x784,0xee00);
  //LinkPhyWrite(Private,0,0x44c,0x1a);
  //LinkPhyWrite(Private,0,0x40c,0xd);
  //LinkPhyWrite(Private,0,0x42c,0xd);
  //LinkPhyWrite(Private,0,0x18318,0xa3);
  //wsing level
  LinkPhyWrite (Private, 0, 0x1879c, 0x1);
  LinkPhyWrite (Private, 0, 0x18100, 0x08a4);
  LinkPhyWrite (Private, 0, 0x18318, 0x0003);
  LinkPhyWrite (Private, 0, 0x18140, 0x0000);
  LinkPhyWrite (Private, 0, 0x18130, 0x001c);
  LinkPhyWrite (Private, 0, 0x1879c, 0x0);
  //dp1 phy vco
  switch (LinkRate[1]) {
  case 162:
    LinkPhyWrite (Private, 1, 0x710, 0x509);
    LinkPhyWrite (Private, 1, 0x714, 0xf00);
    LinkPhyWrite (Private, 1, 0x718, 0xf08);
    LinkPhyWrite (Private, 1, 0x340, 0x061);
    LinkPhyWrite (Private, 1, 0x344, 0x3333);
    LinkPhyWrite (Private, 1, 0x348, 0x0);
    LinkPhyWrite (Private, 1, 0x34c, 0x42);
    LinkPhyWrite (Private, 1, 0x700, 0x02);
    LinkPhyWrite (Private, 1, 0x320, 0xc5e);
    LinkPhyWrite (Private, 1, 0x370, 0xc7);
    LinkPhyWrite (Private, 1, 0x378, 0xc7);
    LinkPhyWrite (Private, 1, 0x37c, 0x05);
    break;
  case 270:
  case 540:
    LinkPhyWrite (Private, 1, 0x710, 0x509);
    LinkPhyWrite (Private, 1, 0x714, 0xf00);
    LinkPhyWrite (Private, 1, 0x718, 0xf08);
    LinkPhyWrite (Private, 1, 0x340, 0x06c);
    LinkPhyWrite (Private, 1, 0x344, 0x0);
    LinkPhyWrite (Private, 1, 0x348, 0x0);
    LinkPhyWrite (Private, 1, 0x34c, 0x48);
    LinkPhyWrite (Private, 1, 0x700, 0x02);
    LinkPhyWrite (Private, 1, 0x320, 0xc5e);
    LinkPhyWrite (Private, 1, 0x370, 0xc7);
    LinkPhyWrite (Private, 1, 0x378, 0xc7);
    LinkPhyWrite (Private, 1, 0x37c, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, 1, 0x710, 0x509);
    LinkPhyWrite (Private, 1, 0x714, 0xf00);
    LinkPhyWrite (Private, 1, 0x718, 0xf08);
    LinkPhyWrite (Private, 1, 0x340, 0x051);
    LinkPhyWrite (Private, 1, 0x344, 0x0);
    LinkPhyWrite (Private, 1, 0x348, 0x036);
    LinkPhyWrite (Private, 1, 0x34c, 0x42);
    LinkPhyWrite (Private, 1, 0x700, 0x02);
    LinkPhyWrite (Private, 1, 0x320, 0xc5e);
    LinkPhyWrite (Private, 1, 0x370, 0xc7);
    LinkPhyWrite (Private, 1, 0x378, 0xc7);
    LinkPhyWrite (Private, 1, 0x37c, 0x05);
    break;
  default:
    LinkPhyWrite (Private, 1, 0x710, 0x509);
    LinkPhyWrite (Private, 1, 0x714, 0xf00);
    LinkPhyWrite (Private, 1, 0x718, 0xf08);
    LinkPhyWrite (Private, 1, 0x340, 0x06c);
    LinkPhyWrite (Private, 1, 0x344, 0x0);
    LinkPhyWrite (Private, 1, 0x348, 0x0);
    LinkPhyWrite (Private, 1, 0x34c, 0x48);
    LinkPhyWrite (Private, 1, 0x700, 0x02);
    LinkPhyWrite (Private, 1, 0x320, 0xc5e);
    LinkPhyWrite (Private, 1, 0x370, 0xc7);
    LinkPhyWrite (Private, 1, 0x378, 0xc7);
    LinkPhyWrite (Private, 1, 0x37c, 0x05);
    break;
  }
  LinkPhyWrite (Private, 1, 0x1A400, 0xfb);
  LinkPhyWrite (Private, 1, 0x1A408, 0x4aa);
  LinkPhyWrite (Private, 1, 0x1A40c, 0x4aa);
  LinkPhyWrite (Private, 1, 0x2A000, 0x0);
  LinkPhyWrite (Private, 1, 0x2A008, 0x0);
  LinkPhyWrite (Private, 1, 0x2A00c, 0x0);
  LinkPhyWrite (Private, 1, 0x2A018, 0x0);
  LinkPhyWrite (Private, 1, 0x1A3a8, 0xf);
  LinkPhyWrite (Private, 1, 0x2A420, 0x0);
  LinkPhyWrite (Private, 1, 0x2A440, 0x0);
  LinkPhyWrite (Private, 1, 0x2A460, 0x0);
  //change vol
  //LinkPhyWrite(Private,1,0x784,0xee00);
  //LinkPhyWrite(Private,1,0x44c,0x1a);
  //LinkPhyWrite(Private,1,0x40c,0xd);
  //LinkPhyWrite(Private,1,0x42c,0xd);
  //LinkPhyWrite(Private,1,0x1A318,0xa3);
  //wsing level
  LinkPhyWrite (Private, 1, 0x1A79c, 0x1);
  LinkPhyWrite (Private, 1, 0x1A100, 0x08a4);
  LinkPhyWrite (Private, 1, 0x1A318, 0x0003);
  LinkPhyWrite (Private, 1, 0x1A140, 0x0000);
  LinkPhyWrite (Private, 1, 0x1A130, 0x001c);
  LinkPhyWrite (Private, 1, 0x1A79c, 0x0);
  LinkPhyWrite (Private, 0, 0x40104, 0x1);
  //dp2
  if (IsPhy2Dp (Private) == 0x1) {
  //dp/sata
    Value = LinkPhyRead (Private, 2, 0x100070);
    Value |= 0x10;
  LinkPhyWrite(Private,2,0x100070,Value);
  Value = LinkPhyRead(Private,2,0x1000A0);
  Value &= 0xFFFFF0FF;
  Value |= (0x0B << 8);
  LinkPhyWrite(Private,2,0x1000A0,0xB04);
    Value = LinkPhyRead (Private, 2, 0x100104);
    Value |= 0x4;
  LinkPhyWrite(Private,2,0x100104, Value);
  LinkPhyWrite(Private,2,0xB0038 ,0x0 );
  switch(LinkRate[2]){
  case 162:
    LinkPhyWrite (Private, 2, 0x80684, 0xf01);
    LinkPhyWrite (Private, 2, 0x80704, 0xf01);
    break;
  case 270:
    LinkPhyWrite (Private, 2, 0x80684, 0x701);
    LinkPhyWrite (Private, 2, 0x80704, 0x701);
    break;
  case 540:
    LinkPhyWrite (Private, 2, 0x80684, 0x301);
    LinkPhyWrite (Private, 2, 0x80704, 0x301);
    break;
  case 810:
    LinkPhyWrite (Private, 2, 0x80684, 0x200);
    LinkPhyWrite (Private, 2, 0x80704, 0x200);
    break;
  default:
    LinkPhyWrite (Private, 2, 0x80684, 0x701);
    LinkPhyWrite (Private, 2, 0x80704, 0x701);
    break;
  }
  LinkPhyWrite (Private, 2, 0x90398, 0x0);
  switch (LinkRate[2]) {
  case 162:
    LinkPhyWrite (Private, 2, 0x9039c, 0x2);
    break;
  case 270:
    LinkPhyWrite (Private, 2, 0x9039c, 0x1);
    break;
  case 540:
    LinkPhyWrite (Private, 2, 0x9039c, 0x0);
    break;
  case 810:
    LinkPhyWrite (Private, 2, 0x9039c, 0x0);
    break;
  default:
    LinkPhyWrite (Private, 2, 0x9039c, 0x1);
    break;
  }
  LinkPhyWrite (Private, 2, 0x90394, 0x1);
  LinkPhyWrite (Private, 2, 0x80250, 0x4);
  LinkPhyWrite (Private, 2, 0x80218, 0xc5e);
  LinkPhyWrite (Private, 2, 0x80208, 0x3);
  switch (LinkRate[2]) {
  case 162:
    LinkPhyWrite (Private, 2, 0x80690, 0x509);
    LinkPhyWrite (Private, 2, 0x80694, 0xf00);
    LinkPhyWrite (Private, 2, 0x80698, 0xf08);
    LinkPhyWrite (Private, 2, 0x80240, 0x061);
    LinkPhyWrite (Private, 2, 0x80244, 0x3333);
    LinkPhyWrite (Private, 2, 0x80248, 0x0);
    LinkPhyWrite (Private, 2, 0x8024c, 0x42);
    LinkPhyWrite (Private, 2, 0x80680, 0x02);
    LinkPhyWrite (Private, 2, 0x80220, 0xc56);
    LinkPhyWrite (Private, 2, 0x80270, 0xc7);
    LinkPhyWrite (Private, 2, 0x80278, 0xc7);
    LinkPhyWrite (Private, 2, 0x8027c, 0x05);
    break;
  case 270:
  case 540:
    LinkPhyWrite (Private, 2, 0x80690, 0x509);
    LinkPhyWrite (Private, 2, 0x80694, 0xf00);
    LinkPhyWrite (Private, 2, 0x80698, 0xf08);
    LinkPhyWrite (Private, 2, 0x80240, 0x06c);
    LinkPhyWrite (Private, 2, 0x80244, 0x0);
    LinkPhyWrite (Private, 2, 0x80248, 0x0);
    LinkPhyWrite (Private, 2, 0x8024c, 0x48);
    LinkPhyWrite (Private, 2, 0x80680, 0x02);
    LinkPhyWrite (Private, 2, 0x80220, 0xc56);
    LinkPhyWrite (Private, 2, 0x80270, 0xc7);
    LinkPhyWrite (Private, 2, 0x80278, 0xc7);
    LinkPhyWrite (Private, 2, 0x8027c, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, 2, 0x80690, 0x509);
    LinkPhyWrite (Private, 2, 0x80694, 0xf00);
    LinkPhyWrite (Private, 2, 0x80698, 0xf08);
    LinkPhyWrite (Private, 2, 0x80240, 0x061);
    LinkPhyWrite (Private, 2, 0x80244, 0x3333);
    LinkPhyWrite (Private, 2, 0x80248, 0x0);
    LinkPhyWrite (Private, 2, 0x8024c, 0x42);
    LinkPhyWrite (Private, 2, 0x80680, 0x02);
    LinkPhyWrite (Private, 2, 0x80220, 0xc56);
    LinkPhyWrite (Private, 2, 0x80270, 0xc7);
    LinkPhyWrite (Private, 2, 0x80278, 0xc7);
    LinkPhyWrite (Private, 2, 0x8027c, 0x05);
    break;
  default:
    LinkPhyWrite (Private, 2, 0x80690, 0x509);
    LinkPhyWrite (Private, 2, 0x80694, 0xf00);
    LinkPhyWrite (Private, 2, 0x80698, 0xf08);
    LinkPhyWrite (Private, 2, 0x80240, 0x06c);
    LinkPhyWrite (Private, 2, 0x80244, 0x0);
    LinkPhyWrite (Private, 2, 0x80248, 0x0);
    LinkPhyWrite (Private, 2, 0x8024c, 0x48);
    LinkPhyWrite (Private, 2, 0x80680, 0x02);
    LinkPhyWrite (Private, 2, 0x80220, 0xc56);
    LinkPhyWrite (Private, 2, 0x80270, 0xc7);
    LinkPhyWrite (Private, 2, 0x80278, 0xc7);
    LinkPhyWrite (Private, 2, 0x8027c, 0x05);
    break;
  }
  LinkPhyWrite(Private,2,0x258 ,0x0b );
  LinkPhyWrite(Private,2,0x90400 ,0xfb );
  LinkPhyWrite(Private,2,0x90408 ,0x4aa );
  LinkPhyWrite(Private,2,0x9040c ,0x4aa );
  LinkPhyWrite(Private,2,0xa0000,0x0 );
  LinkPhyWrite(Private,2,0xa0008 ,0x0 );
  LinkPhyWrite(Private,2,0xa000c ,0x0 );
  LinkPhyWrite(Private,2,0xa0018 ,0x0 );
  LinkPhyWrite(Private,2,0x903a8 ,0xf );
  LinkPhyWrite(Private,2,0xa0420 ,0x0 );
  LinkPhyWrite(Private,2,0xa0440 ,0x0 );
  LinkPhyWrite(Private,2,0xa0460 ,0x0 );
    Value = LinkPhyRead(Private, 2, 0x100100);
    Value |= 0x4;
  LinkPhyWrite(Private,2,0x100100 ,Value );
  }
  return 0;
}

UINT32
LinkPhyChangeRate (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           LinkRate,
  IN UINT32           Num
  )
{
  UINT32 Value;
  if (Num == 0) {
    //power state A3
    LinkPhyWrite (Private, 0, 0x40014, 0x08080808);
    //disable pll clock
  Value = LinkPhyRead (Private, 0, 0x40010);
  Value &= 0xFFFFFFF0;
  LinkPhyWrite (Private, 0, 0x40010, Value);
    Value = LinkPhyRead (Private, 0, 0x38004);
  //disable pll , pll_raw_ctrl
  Value = LinkPhyRead (Private, 0, 0x3800C);
  Value &= 0xFFFFFFFE;
  LinkPhyWrite (Private, 0, 0x3800C, Value);
  //wait disable pll success
  while (1) {
      Value = LinkPhyRead (Private, 0, 0x38004);
    if (((Value >> 2) & 0x00000001) == 0x01)
    break;
  }
  switch (LinkRate) {
    case 162:
      LinkPhyWrite (Private, 0, 0x684, 0xf01);
      break;
    case 270:
      LinkPhyWrite (Private, 0, 0x684, 0x701);
      break;
    case 540:
      LinkPhyWrite (Private, 0, 0x684, 0x301);
      break;
    case 810:
      LinkPhyWrite (Private, 0, 0x684, 0x200);
      break;
    default:
      break;
  }
  LinkPhyWrite (Private, 0, 0x18398, 0x0);
  switch (LinkRate) {
    case 162:
      LinkPhyWrite (Private, 0, 0x1839c, 0x2);
      break;
    case 270:
      LinkPhyWrite (Private, 0, 0x1839c, 0x1);
      break;
    case 540:
      LinkPhyWrite (Private, 0, 0x1839c, 0x0);
      break;
    case 810:
      LinkPhyWrite (Private, 0, 0x1839c, 0x0);
      break;
    default:
      break;
  }
    LinkPhyWrite (Private, 0, 0x18394, 0x1);
  //link rate
  LinkPhyWrite (Private, 0, 0x250, 0x4);
  LinkPhyWrite (Private, 0, 0x218, 0xc5e);
  LinkPhyWrite (Private, 0, 0x208, 0x3);
  switch (LinkRate) {
  case 162:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x061);
    LinkPhyWrite (Private, 0, 0x244, 0x3333);
    LinkPhyWrite (Private, 0, 0x248, 0x0);
    LinkPhyWrite (Private, 0, 0x24c, 0x42);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  case 270:
  case 540:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x06c);
    LinkPhyWrite (Private, 0, 0x244, 0x0);
    LinkPhyWrite (Private, 0, 0x248, 0x0);
    LinkPhyWrite (Private, 0, 0x24c, 0x48);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  case 810:
    LinkPhyWrite (Private, 0, 0x690, 0x509);
    LinkPhyWrite (Private, 0, 0x694, 0xf00);
    LinkPhyWrite (Private, 0, 0x698, 0xf08);
    LinkPhyWrite (Private, 0, 0x240, 0x051);
    LinkPhyWrite (Private, 0, 0x244, 0x0);
    LinkPhyWrite (Private, 0, 0x248, 0x036);
    LinkPhyWrite (Private, 0, 0x24c, 0x42);
    LinkPhyWrite (Private, 0, 0x680, 0x02);
    LinkPhyWrite (Private, 0, 0x220, 0xc5e);
    LinkPhyWrite (Private, 0, 0x270, 0xc7);
    LinkPhyWrite (Private, 0, 0x278, 0xc7);
    LinkPhyWrite (Private, 0, 0x27c, 0x05);
    break;
  default:
    break;
  }
  LinkPhyWrite (Private, 0, 0x18400, 0xfb);
  LinkPhyWrite (Private, 0, 0x18408, 0x4aa);
  LinkPhyWrite (Private, 0, 0x1840c, 0x4aa);
  LinkPhyWrite (Private, 0, 0x28000, 0x0);
  LinkPhyWrite (Private, 0, 0x28008, 0x0);
  LinkPhyWrite (Private, 0, 0x2800c, 0x0);
  LinkPhyWrite (Private, 0, 0x28018, 0x0);
  LinkPhyWrite (Private, 0, 0x183a8, 0xf);
  LinkPhyWrite (Private, 0, 0x28420, 0x0);
  LinkPhyWrite (Private, 0, 0x28440, 0x0);
  LinkPhyWrite (Private, 0, 0x28460, 0x0);
    //enable pll , pll_raw_ctrl
  Value = LinkPhyRead (Private, 0, 0x3800C);
  Value |= 0x00000001;
  LinkPhyWrite (Private, 0, 0x3800C, Value);
  //enable pll clock
  Value = LinkPhyRead (Private, 0, 0x40010);
  Value |= 0x0000000F;
  LinkPhyWrite (Private, 0, 0x40010, Value);
  LinkPhyWrite (Private, 0, 0x40014, 0x01010101);
  }
  else if (Num == 1) {
  LinkPhyWrite (Private, 1, 0x40018, 0x08080808);
  //disable pllen
  Value = LinkPhyRead (Private, 1, 0x40010);
  Value &= 0xFFFFFF0F;
  LinkPhyWrite(Private,1,0x40010,Value);
  //disable pll , pll_raw_ctrl
  Value = LinkPhyRead (Private, 1, 0x3800C);
  Value &= 0xFFFFFFFD;
  LinkPhyWrite (Private, 1, 0x3800C, Value);
  //wait disable pll success
  while (1) {
      Value = LinkPhyRead (Private, 1, 0x38004);
    if (((Value >> 3) & 0x00000001) == 0x01)
    break;
    }
    switch (LinkRate) {
    case 162:
    LinkPhyWrite (Private, 1, 0x704, 0xf01);
    break;
    case 270:
    LinkPhyWrite (Private, 1, 0x704, 0x701);
    break;
    case 540:
      LinkPhyWrite (Private, 1, 0x704, 0x301);
      break;
    case 810:
      LinkPhyWrite (Private, 1, 0x704, 0x200);
      break;
    default:
      break;
    }
    LinkPhyWrite (Private, 1, 0x1A398, 0x1);
    switch (LinkRate) {
    case 162:
      LinkPhyWrite (Private, 1, 0x1A39c, 0x2);
      break;
    case 270:
      LinkPhyWrite (Private, 1, 0x1A39c, 0x1);
      break;
    case 540:
      LinkPhyWrite (Private, 1, 0x1A39c, 0x0);
      break;
    case 810:
      LinkPhyWrite (Private, 1, 0x1A39c, 0x0);
      break;
    default:
      break;
    }
      LinkPhyWrite (Private, 1, 0x1A394, 0x9);
    LinkPhyWrite (Private, 1, 0x350, 0x4);
    LinkPhyWrite (Private, 1, 0x318, 0xc5e);
    LinkPhyWrite (Private, 1, 0x308, 0x3);
    switch (LinkRate) {
    case 162:
      LinkPhyWrite (Private, 1, 0xE90, 0x509);
      LinkPhyWrite (Private, 1, 0xE94, 0xf00);
      LinkPhyWrite (Private, 1, 0xE98, 0xf08);
      LinkPhyWrite (Private, 1, 0x340, 0x061);
      LinkPhyWrite (Private, 1, 0x344, 0x3333);
      LinkPhyWrite (Private, 1, 0x348, 0x0);
      LinkPhyWrite (Private, 1, 0x34c, 0x42);
      LinkPhyWrite (Private, 1, 0xE80, 0x02);
      LinkPhyWrite (Private, 1, 0x320, 0xc5e);
      LinkPhyWrite (Private, 1, 0x370, 0xc7);
      LinkPhyWrite (Private, 1, 0x378, 0xc7);
      LinkPhyWrite (Private, 1, 0x37c, 0x05);
    break;
    case 270:
    case 540:
      LinkPhyWrite (Private, 1, 0xE90, 0x509);
      LinkPhyWrite (Private, 1, 0xE94, 0xf00);
      LinkPhyWrite (Private, 1, 0xE98, 0xf08);
      LinkPhyWrite (Private, 1, 0x340, 0x06c);
      LinkPhyWrite (Private, 1, 0x344, 0x0);
      LinkPhyWrite (Private, 1, 0x348, 0x0);
      LinkPhyWrite (Private, 1, 0x34c, 0x48);
      LinkPhyWrite (Private, 1, 0xE80, 0x02);
      LinkPhyWrite (Private, 1, 0x320, 0xc5e);
      LinkPhyWrite (Private, 1, 0x370, 0xc7);
      LinkPhyWrite (Private, 1, 0x378, 0xc7);
      LinkPhyWrite (Private, 1, 0x37c, 0x05);
      break;
    case 810:
      LinkPhyWrite (Private, 1, 0xE90, 0x509);
      LinkPhyWrite (Private, 1, 0xE94, 0xf00);
      LinkPhyWrite (Private, 1, 0xE98, 0xf08);
      LinkPhyWrite (Private, 1, 0x340, 0x051);
      LinkPhyWrite (Private, 1, 0x344, 0x0);
      LinkPhyWrite (Private, 1, 0x348, 0x036);
      LinkPhyWrite (Private, 1, 0x34c, 0x42);
      LinkPhyWrite (Private, 1, 0xE80, 0x02);
      LinkPhyWrite (Private, 1, 0x320, 0xc5e);
      LinkPhyWrite (Private, 1, 0x370, 0xc7);
      LinkPhyWrite (Private, 1, 0x378, 0xc7);
      LinkPhyWrite (Private, 1, 0x37c, 0x05);
      break;
    default:
      break;
    }
    LinkPhyWrite (Private, 1, 0x1A400, 0xfb);
    LinkPhyWrite (Private, 1, 0x1A408, 0x4aa);
    LinkPhyWrite (Private, 1, 0x1A40c, 0x4aa);
    LinkPhyWrite (Private, 1, 0x2A000, 0x0);
    LinkPhyWrite (Private, 1, 0x2A008, 0x0);
    LinkPhyWrite (Private, 1, 0x2A00c, 0x0);
    LinkPhyWrite (Private, 1, 0x2A018, 0x0);
    LinkPhyWrite (Private, 1, 0x1A3a8, 0xf);
    LinkPhyWrite (Private, 1, 0x2A420, 0x0);
    LinkPhyWrite (Private, 1, 0x2A440, 0x0);
    LinkPhyWrite (Private, 1, 0x2A460, 0x0);
    //enable pll , pll_raw_ctrl
    Value = LinkPhyRead (Private, 1, 0x3800C);
    Value |= 0x00000002;
    LinkPhyWrite (Private, 1, 0x3800C, Value);
    //enable pll clock
    Value = LinkPhyRead (Private, 1, 0x40010);
    Value |= 0x000000F0;
    LinkPhyWrite (Private, 1, 0x40010, Value);
      LinkPhyWrite (Private, 1, 0x40018, 0x01010101);
  }
  else if (Num == 2) {
      if (IsPhy2Dp (Private) == 1) {
        //set power state A3
        Value = LinkPhyRead (Private, 2, 0x1000A0);
        Value &= 0xFFFFFF03;
        Value |= (0x08 << 2);
        LinkPhyWrite (Private, 2, 0x1000A0, Value);
        //disable pllen
        Value = LinkPhyRead (Private, 2, 0x1000A0);
        Value &= 0xFFFFFEFF;
        LinkPhyWrite (Private, 2, 0x1000A0, Value);
        //disable pll , pll_raw_ctrl
        Value = LinkPhyRead (Private, 2, 0xB800C);
        Value &= 0xFFFFFFFC;
        LinkPhyWrite (Private, 2, 0xB800C, Value);
        //wait disable pll success
        while (1) {
          Value = LinkPhyRead (Private, 2, 0xB8004);
          if (((Value >> 2) & 0x00000001) == 0x01)
            break;
        }
        switch (LinkRate) {
        case 162:
          LinkPhyWrite (Private, 2, 0x80684, 0xf01);
          LinkPhyWrite (Private, 2, 0x80704, 0xf01);
          break;
        case 270:
          LinkPhyWrite (Private, 2, 0x80684, 0x701);
          LinkPhyWrite (Private, 2, 0x80704, 0x701);
          break;
        case 540:
          LinkPhyWrite (Private, 2, 0x80684, 0x301);
          LinkPhyWrite (Private, 2, 0x80704, 0x301);
          break;
        case 810:
          LinkPhyWrite (Private, 2, 0x80684, 0x200);
          LinkPhyWrite (Private, 2, 0x80704, 0x200);
          break;
        default:
          break;
        }
        LinkPhyWrite (Private, 2, 0x90398, 0x0);
        switch (LinkRate) {
        case 162:
          LinkPhyWrite (Private, 2, 0x9039c, 0x2);
          break;
        case 270:
          LinkPhyWrite (Private, 2, 0x9039c, 0x1);
          break;
        case 540:
          LinkPhyWrite (Private, 2, 0x9039c, 0x0);
          break;
        case 810:
          LinkPhyWrite (Private, 2, 0x9039c, 0x0);
          break;
        default:
          break;
        }
        LinkPhyWrite (Private, 2, 0x90394, 0x1);
      LinkPhyWrite (Private, 2, 0x80250, 0x4);
      LinkPhyWrite (Private, 2, 0x80218, 0xc5e);
      LinkPhyWrite (Private, 2, 0x80208, 0x3);
      switch (LinkRate) {
      case 162:
        LinkPhyWrite (Private, 2, 0x80690, 0x509);
        LinkPhyWrite (Private, 2, 0x80694, 0xf00);
        LinkPhyWrite (Private, 2, 0x80698, 0xf08);
        LinkPhyWrite (Private, 2, 0x80240, 0x061);
        LinkPhyWrite (Private, 2, 0x80244, 0x3333);
        LinkPhyWrite (Private, 2, 0x80248, 0x0);
        LinkPhyWrite (Private, 2, 0x8024c, 0x42);
        LinkPhyWrite (Private, 2, 0x80680, 0x02);
        LinkPhyWrite (Private, 2, 0x80220, 0xc5e);
        LinkPhyWrite (Private, 2, 0x80270, 0xc7);
        LinkPhyWrite (Private, 2, 0x80278, 0xc7);
        LinkPhyWrite (Private, 2, 0x8027c, 0x05);
      break;
      case 270:
      case 540:
        LinkPhyWrite (Private, 2, 0x80690, 0x509);
        LinkPhyWrite (Private, 2, 0x80694, 0xf00);
        LinkPhyWrite (Private, 2, 0x80698, 0xf08);
        LinkPhyWrite (Private, 2, 0x80240, 0x06c);
        LinkPhyWrite (Private, 2, 0x80244, 0x0);
        LinkPhyWrite (Private, 2, 0x80248, 0x0);
        LinkPhyWrite (Private, 2, 0x8024c, 0x48);
        LinkPhyWrite (Private, 2, 0x80680, 0x02);
        LinkPhyWrite (Private, 2, 0x80220, 0xc56);
        LinkPhyWrite (Private, 2, 0x80270, 0xc7);
        LinkPhyWrite (Private, 2, 0x80278, 0xc7);
        LinkPhyWrite (Private, 2, 0x8027c, 0x05);
        break;
      case 810:
        LinkPhyWrite (Private, 2, 0x80690, 0x509);
        LinkPhyWrite (Private, 2, 0x80694, 0xf00);
        LinkPhyWrite (Private, 2, 0x80698, 0xf08);
        LinkPhyWrite (Private, 2, 0x80240, 0x051);
        LinkPhyWrite (Private, 2, 0x80244, 0x0);
        LinkPhyWrite (Private, 2, 0x80248, 0x036);
        LinkPhyWrite (Private, 2, 0x8024c, 0x42);
        LinkPhyWrite (Private, 2, 0x80680, 0x02);
        LinkPhyWrite (Private, 2, 0x80220, 0xc5e);
        LinkPhyWrite (Private, 2, 0x80270, 0xc7);
        LinkPhyWrite (Private, 2, 0x80278, 0xc7);
        LinkPhyWrite (Private, 2, 0x8027c, 0x05);
        break;
      default:
        break;
      }
      LinkPhyWrite (Private, 2, 0x90400, 0xfb);
      LinkPhyWrite (Private, 2, 0x90408, 0x4aa);
      LinkPhyWrite (Private, 2, 0x9040c, 0x4aa);
      LinkPhyWrite (Private, 2, 0xA0000, 0x0);
      LinkPhyWrite (Private, 2, 0xA0008, 0x0);
      LinkPhyWrite (Private, 2, 0xA000c, 0x0);
      LinkPhyWrite (Private, 2, 0xA0018, 0x0);
      LinkPhyWrite (Private, 2, 0x903a8, 0xf);
      LinkPhyWrite (Private, 2, 0xA0420, 0x0);
      LinkPhyWrite (Private, 2, 0xA0440, 0x0);
      LinkPhyWrite (Private, 2, 0xA0460, 0x0);
      //enable pll , pll_raw_ctrl
      Value = LinkPhyRead (Private, 2, 0xB800C);
      Value |= 0x00000003;
      LinkPhyWrite (Private, 2, 0xB800C, Value);
      //enable pll clock
      Value = LinkPhyRead (Private, 2, 0x1000A0);
        Value &= 0xFFFFFEFF;
      Value |= 0x100;
      LinkPhyWrite (Private, 2, 0x1000A0, Value);
      Value = LinkPhyRead (Private, 2, 0x1000A0);
        Value &= 0xFFFFFF03;
      Value |= 0x04;
      LinkPhyWrite (Private, 2, 0x1000A0, Value);
      }
    }
  return 0;
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

UINT32
LinkPhyChangeVsWing (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT32           Num,
  IN UINT32           VsWing,
  IN UINT32           PreEmphasis
  )
{
  if (Num == 0) {
    LinkPhyWrite (Private, 0, 0x1879c, 0x1);
    LinkPhyWrite (Private, 0, 0x18100, 0x08a4);
    LinkPhyWrite (Private, 0, 0x18318, 0x0003);
    LinkPhyWrite (Private, 0, 0x18140, MgnfsTable[VsWing][PreEmphasis]);
    LinkPhyWrite (Private, 0, 0x18130, CpostTable[VsWing][PreEmphasis]);
    LinkPhyWrite (Private, 0, 0x1879c, 0x0);
  }
  else if (Num == 1) {
    LinkPhyWrite (Private, 1, 0x1A79c, 0x1);
    LinkPhyWrite (Private, 1, 0x1A100, 0x08a4);
    LinkPhyWrite (Private, 1, 0x1A318, 0x0003);
    LinkPhyWrite (Private, 1, 0x1A140, MgnfsTable[VsWing][PreEmphasis]);
    LinkPhyWrite (Private, 1, 0x1A130, CpostTable[VsWing][PreEmphasis]);
    LinkPhyWrite (Private, 1, 0x1A79c, 0x0);
  }
  else if (Num == 2) {
    if (IsPhy2Dp (Private) == 1) {
      LinkPhyWrite (Private, 2, 0x9879c, 0x1);
      LinkPhyWrite (Private, 2, 0x98100, 0x08a4);
      LinkPhyWrite (Private, 2, 0x98318, 0x0003);
      LinkPhyWrite (Private, 2, 0x98140, MgnfsTable[VsWing][PreEmphasis]);
      LinkPhyWrite (Private, 2, 0x98130, CpostTable[VsWing][PreEmphasis]);
      LinkPhyWrite (Private, 2, 0x9879c, 0x0);
    }
  }
  gBS->Stall (20*1000);
  return 0;
}

UINT32
DptxPhyGetAdjustRequest (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  OUT UINT8            *Swing,
  OUT UINT8            *PreEmphasis
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
  switch (LaneCount){
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

/*
 * 1 - need to retrain
 * 0 - not need to retrain
*/
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
  if ((UINT32)(Private->DpConfig[Num].LinkRate * 100) == 0) {
    return 1;
  }
  DEBUG((DEBUG_INFO, "Sink Need Retrain? lanecount : %d, linkrate : %d\n",
    Private->DpConfig[Num].LaneCount, (UINT32)(Private->DpConfig[Num].LinkRate * 100)));
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
