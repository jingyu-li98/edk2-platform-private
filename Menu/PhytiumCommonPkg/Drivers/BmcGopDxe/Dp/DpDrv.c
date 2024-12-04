/* @file
  Copyright (c) 2022, Phytium Technology Co., Ltd. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include "DpPhy.h"
#include "DpDrv.h"
#include "DpFun.h"
#include "DpType.h"
#include "../PhyGopDxe.h"

UINT8  mTrainFailedTimes;
UINT8  mTrainDownLinkrateTimes;
BOOLEAN  mTrainDownLinkrate;

/**
  Enable or disable the dp timer.The timer block must be initialized with DptxTimerInit before calling this function.
  Disabling the timer pauses the countdown and preserves the value of the register.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  Enable       Switch of dp timer.1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
DptxTimerEnable (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  UINT32 TimerReg;
  TimerReg = ChannelRegRead (Private, Num, DPTX_TIMER, DpChOperate);
  if (Enable) {
    TimerReg = TimerReg | TR_DPTX_TIMER_FLAG_ENABLE;
  } else{
    TimerReg = TimerReg & (~TR_DPTX_TIMER_FLAG_ENABLE);
  }
  ChannelRegWrite (Private, Num, DPTX_TIMER, DpChOperate, TimerReg);
}

/**
  Set the current value of the Displayport TX timer to the specified value.This value must be specified in system ticks.
  The resolution of the timer is 1 usec.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  NewValue     The value to set the timer to in system ticks.The resolution of the timer is 1 usec.

  @retval     NULL
**/
VOID
DptxTimerSetValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            NewValue
  )
{
  UINT32 TimerReg;

  TimerReg = ChannelRegRead (Private, Num, DPTX_TIMER, DpChOperate);
  TimerReg = ((TimerReg & TR_DPTX_TIMER_FLAGS_ALL) | (TR_DPTX_TIMER_MAX_COUNT & NewValue));
  ChannelRegWrite (Private, Num, DPTX_TIMER, DpChOperate, TimerReg);
}

/**
  Return the current value of the Displayport TX timer. This is a 32-bit interger value equal to the Number of
  microseconds remaining in the timer. This function can be called in a loop, checking the return value to
  determine when the timer has expired.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     Current      value of the Displayport TX timer
**/
UINT32
DptxTimerGetValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  return (ChannelRegRead (Private, Num, DPTX_TIMER, DpChOperate) & TR_DPTX_TIMER_MAX_COUNT);
}

/**
  Set the flags for the Displayport TX timer. The available flags are as
  follows:
    TR_DPTX_TIMER_FLAG_ENABLE          - Enable the timer
    TR_DPTX_TIMER_FLAG_AUTORELOAD_EN   - Enable automatic reload of the initial value
    TR_DPTX_TIMER_FLAG_INTERRUPT_EN    - Enable generation of an interrupt upon expiration
    TR_DPTX_TIMER_FLAGS_ALL            - All flags logically OR'd together for convenience

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  flags        A value representing the flags to set/clear.

  @retval     NULL
**/
VOID
DptxTimerSetFlags (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            Flags
  )
{
  UINT32 TimerReg;
  TimerReg = ChannelRegRead (Private, Num, DPTX_TIMER, DpChOperate);
  //
  // Clear flags, masking off timer value, mask invalid flags
  //
  TimerReg = (TimerReg & TR_DPTX_TIMER_MAX_COUNT) | (Flags & TR_DPTX_TIMER_FLAGS_ALL);
  ChannelRegWrite (Private, Num, DPTX_TIMER, DpChOperate, TimerReg);
}

/**
  Initialize the timer block in the Displayport TX core. This diables the timer, sets the initial value to 0
  and clears all flags. This function must be called before using the timer block or the behavior will be unpredictable.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     NULL
**/
VOID
DptxTimerInit (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  )
{
  //
  // Disable the timer
  //
  DptxTimerEnable (Private, Num, 0);
  //
  // Zero the timer value
  //
  DptxTimerSetValue (Private, Num, 0);
  //
  // Clear autoreload, interrupt and enable flags
  //
  DptxTimerSetFlags (Private, Num, ~TR_DPTX_TIMER_FLAGS_ALL);
}

/**
  A convenience function that waits for the specified Number of microseconds before returning.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1
  @param[in]  Us           An integer value in microseconds to wait.

  @retval     NULL
**/
VOID
DptxTimerWait (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            Us
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  //
  // Disable the timer
  //
  DptxTimerEnable (Private, Num, 0);
  //
  // Set the timer
  //
  DptxTimerSetValue (Private, Num, Us);
  //
  // Enable the timer
  //
  DptxTimerEnable (Private, Num, 1);
  //
  // Wait for the timer to be zero
  //
  while(1) {
    if (DptxTimerGetValue (Private,Num) == 0) {
      break;
    }
  }
}

/**
  Wait util that an aux reply has been recieved.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval      status
              0 - indicates that a reply has been received
              1 - indicates that a reply timeout has occurred
**/
UINT32
WaitAuxReply (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  )
{
  UINT32 ReplyTimes;
  UINT32 Status;
  UINT8  AuxStatus;
  UINT8  InitStatus;

  Status = AUX_TIMEOUT;
  ReplyTimes = 5;

  ASSERT (Num < DPDC_PATH_NUM);
  while (ReplyTimes > 0) {
    gBS->Stall (10 * 1000);
    AuxStatus = ChannelRegRead (Private, Num, DPTX_AUX_TRANSACTION_STATUS, DpChOperate);
    ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
    InitStatus = ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate);
    if ((InitStatus & 0x08) != 0) {
      Status = AUX_TIMEOUT;
      break;
    } else if ((InitStatus & 0x04) != 0) {
      if((AuxStatus & 0x1) != 0) {
        Status = AUX_CONNECT;
        break;
      }
    }
    ReplyTimes--;
  }
  //ChannelRegRead(Private,Num,0x140,DpChOperate);
  return Status;
}

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
  OUT UINT8            *Data
  )
{
  UINT32 Status;

  ASSERT (Num < DPDC_PATH_NUM);
  ChannelRegRead (Private, Num, DPTX_INTERRUPT, DpChOperate);
  ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, DpcdAddr);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x900);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT){
    return Status;
  }
  gBS->Stall (100);
  ChannelRegRead (Private, Num, DPTX_RCV_REPLY_COUNT, DpChOperate);
  ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  *Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
#ifdef DPCD_DEBUG
  DEBUG ((DEBUG_INFO,"Dpcd Read addr : 0x%x , data : 0x%x\n", DpcdAddr, *Data));
#endif
  return AUX_CONNECT;
}

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
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            DpcdAddr,
  IN UINT8             WriteData
  )
{
  UINT32  Status;
  ChannelRegRead (Private, Num, DPTX_INTERRUPT, DpChOperate);
  ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, DpcdAddr);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  gBS->Stall (100);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x800);
  Status = WaitAuxReply (Private, Num);
#ifdef DPCD_DEBUG
  DEBUG ((DEBUG_INFO,"Dpcd Write addr : 0x%x , data : 0x%x , Status : %d\n",DpcdAddr, WriteData, Status));
#endif
  return Status;
}

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
  IN OUT UINT8         *Buffer
  )
{
  UINT32  Status;
  UINT8   Data;
  UINT8   Ii;
  UINT8   Jj;
  UINT8   WriteData;

  WriteData = 0;
  Data = 0;

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1400);
  Status = WaitAuxReply (Private,Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO,"--------------------"));


  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x400);
  Status = WaitAuxReply (Private,Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO,"--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  //DEBUG ((DEBUG_INFO,"--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
  //DEBUG ((DEBUG_INFO,"--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  //edid 0 128B
  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1400);
  Status = WaitAuxReply (Private, Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x400);
  Status = WaitAuxReply (Private, Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  for (Ii = 0; Ii < 8; Ii++) {
    while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
    ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
    ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x50f);
    Status = WaitAuxReply (Private, Num);
    if (Status == AUX_TIMEOUT) {
      return Status;
    }
    Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
    Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
    for (Jj = 0; Jj < 16; Jj++) {
      Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
      *(Buffer + Ii * 16 + Jj) = Data;
      DEBUG ((DEBUG_INFO, "edid[%d] : 0x%x\n", Ii * 16 + Jj, Data));
    }
    //DEBUG ((DEBUG_INFO, "--------------------"));
  }

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  //edid 1 128B
  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1400);
  Status = WaitAuxReply (Private, Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_FIFO, DpChOperate, WriteData);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x400);
  Status = WaitAuxReply (Private, Num);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  for (Ii = 0; Ii < 8; Ii++) {
    while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
    ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
    ChannelRegWrite (Private, Num, DPTX_CMD_REG, DpChOperate, 0x50f);
    Status = WaitAuxReply (Private, Num);
    if (Status == AUX_TIMEOUT) {
      return Status;
    }
    Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
    Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
    for (Jj = 0; Jj < 16; Jj++) {
      Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
      DEBUG ((DEBUG_INFO, "edid[%d] : 0x%x\n", Ii * 16 + Jj, Data));
      *(Buffer + 128 + Ii * 16 + Jj) = Data;
    }
    //DEBUG ((DEBUG_INFO, "--------------------"));
  }

  while (ChannelRegRead (Private, Num, DPTX_STATUS, DpChOperate) & 0x02);
  ChannelRegWrite (Private, Num, DPTX_ADDRESS_REG, DpChOperate, 0x50);
  ChannelRegWrite (Private, Num, DPTX_CMD_REG,  DpChOperate, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = ChannelRegRead (Private, Num, DPTX_RCV_REPLY_REG, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_DATA_COUNT, DpChOperate);
  Data = ChannelRegRead (Private, Num, DPTX_RCV_FIFO, DpChOperate);
  //DEBUG ((DEBUG_INFO, "--------------------"));
  return 0;
}

/**
 Convert edid parameter to DC sync parameter.

 @param[in]  HPixel     Horizontal Addressable video in pixels.
 @param[in]  VPixel     Vertical Addressable video in lines.
 @param[in]  HBlanking  Horizontal Blanking in pixels.
 @param[in]  VBlanking  Vertical Blanking in lines.
 @param[in]  HSyncFront Horizontal Front Proch in pixels.
 @param[in]  VSyncFront Vertical Front Proch in lines.
 @param[in]  HSyncWidth Horizontal Sync Pulse Width in pixels.
 @param[in]  VSyncWidth Vertical Sync Pulse Width in lines.
 @param[in]  HPolarity  Horizontal Sync Polarity.
 @param[in]  VPolarity  Vertical Sync Polarity.
 @param[out] Sync       The struct of DC_SYNC.

 @retval    0        Success.
            other    Failed.
**/
VOID
TimingToDcSync (
  IN  UINT32   HPixel,
  IN  UINT32   VPixel,
  IN  UINT32   HBlanking,
  IN  UINT32   VBlanking,
  IN  UINT32   HSyncFront,
  IN  UINT32   VSyncFront,
  IN  UINT32   HSyncWidth,
  IN  UINT32   VSyncWidth,
  IN  UINT8    HPolarity,
  IN  UINT8    VPolarity,
  OUT DC_SYNC  *Sync
  )
{
  ZeroMem (Sync, sizeof (DP_SYNC));
  //
  //Hor Timing
  //
  Sync->HTotal = HPixel + HBlanking;
  Sync->HDisEnd = HPixel;
  Sync->HStart = HPixel + HSyncFront;
  Sync->HEnd = Sync->HStart + HSyncWidth;
  Sync->HPolarity = HPolarity;
  //
  //Ver Timing
  //
  Sync->VTotal = VPixel + VBlanking;
  Sync->VDisEnd = VPixel;
  Sync->VStart = VPixel + VSyncFront;
  Sync->VEnd = Sync->VStart + VSyncWidth;
  Sync->VPolarity = VPolarity;
}

/**
 Convert edid parameter to DP sync parameter.

 @param[in]  HPixel     Horizontal Addressable video in pixels.
 @param[in]  VPixel     Vertical Addressable video in lines.
 @param[in]  HBlanking  Horizontal Blanking in pixels.
 @param[in]  VBlanking  Vertical Blanking in lines.
 @param[in]  HSyncFront Horizontal Front Proch in pixels.
 @param[in]  VSyncFront Vertical Front Proch in lines.
 @param[in]  HSyncWidth Horizontal Sync Pulse Width in pixels.
 @param[in]  VSyncWidth Vertical Sync Pulse Width in lines.
 @param[in]  HPolarity  Horizontal Sync Polarity.
 @param[in]  VPolarity  Vertical Sync Polarity.
 @param[out] Sync       The struct of DP_SYNC.

 @retval    0        Success.
            other    Failed.
**/
VOID
TimingToDpSync (
  IN  UINT32   HPixel,
  IN  UINT32   VPixel,
  IN  UINT32   HBlanking,
  IN  UINT32   VBlanking,
  IN  UINT32   HSyncFront,
  IN  UINT32   VSyncFront,
  IN  UINT32   HSyncWidth,
  IN  UINT32   VSyncWidth,
  IN  UINT8    HPolarity,
  IN  UINT8    VPolarity,
  OUT DP_SYNC  *Sync
  )
{
  ZeroMem (Sync, sizeof (DP_SYNC));
  //
  //Hor Timing
  //
  Sync->HTotal = HPixel + HBlanking;
  Sync->HsWidth = HSyncWidth;
  Sync->HRes = HPixel;
  Sync->HStart = HBlanking - HSyncFront;
  Sync->HPolarity = (~ HPolarity) & 0x1;
  Sync->HUserPolarity = HPolarity;
  //
  //Ver Timing
  //
  Sync->VTotal = VPixel + VBlanking;
  Sync->VsWidth = VSyncWidth;
  Sync->VRes = VPixel;
  Sync->VStart = VBlanking - VSyncFront;
  Sync->VPolarity = (~ VPolarity) & 0x1;
  Sync->VUserPolarity = VPolarity;
}

/**
 Covert edid detailed timimg parameter to DTD_TABLE.

 @param[in]  Buffer  One edid detailed time parameter.
 @param[out] Table   The struct of DTD_TABLE.

 @retval    0        Success.
            other    Failed.
**/
VOID
ParseEdidDtdTable (
  IN  UINT8      *Buffer,
  OUT DTD_TABLE  *Table
  )
{
  UINT32  HorPixel;
  UINT32  VerPixel;
  UINT32  HorBlanking;
  UINT32  VerBlanking;
  UINT32  HorSyncFront;
  UINT32  VerSyncFront;
  UINT32  HorSyncWidth;
  UINT32  VerSyncWidth;
  UINT8   HorPolarity;
  UINT8   VerPolarity;

  //
  //clear
  //
  ZeroMem (Table, sizeof (DTD_TABLE));
  //
  //pixel clock
  //
  Table->PixelClock = (((UINT32)Buffer[1] << 8) + Buffer[0]) * 10;
  //
  //hor pixel and ver pixel
  //
  HorPixel = ((UINT32)((Buffer[4] >> 4) & 0xF) << 8) + Buffer[2];
  VerPixel = ((UINT32)((Buffer[7] >> 4) & 0xF) << 8) + Buffer[5];
  Table->HorPixel = HorPixel;
  Table->VerPixel = VerPixel;
  //
  //hor blanking and ver blanking
  //
  HorBlanking = ((UINT32)(Buffer[4] & 0xF) << 8) + Buffer[3];
  VerBlanking = ((UINT32)(Buffer[7] & 0xF) << 8) + Buffer[6];
  //
  //hor sync front and ver sync front
  //
  HorSyncFront = ((UINT32)((Buffer[11] >> 6) & 0x3) << 8) + Buffer[8];
  VerSyncFront = ((UINT32)((Buffer[11] >> 2) & 0x3) << 4) + ((Buffer[10] >> 4) & 0xF);
  //
  //hor sync width and ver sync width
  //
  HorSyncWidth = ((UINT32)((Buffer[11] >> 4) & 0x3) << 8) + Buffer[9];
  VerSyncWidth = ((UINT32)((Buffer[11] >> 0) & 0x3) << 4) + (Buffer[10] & 0xF);
  //
  //hor sync polarity and ver sync polarity
  //
  HorPolarity = ((Buffer[17] >> 1) & 0x1);
  VerPolarity = ((Buffer[17] >> 2) & 0x1);

  DEBUG ((DEBUG_INFO, "DTD Table:\n"));
  DEBUG ((DEBUG_INFO, "HorPixel : %d\n",    HorPixel));
  DEBUG ((DEBUG_INFO, "VerPixel : %d\n",    VerPixel));
  DEBUG ((DEBUG_INFO, "HorBlanking : %d\n",  HorBlanking));
  DEBUG ((DEBUG_INFO, "VerBlanking : %d\n",  VerBlanking));
  DEBUG ((DEBUG_INFO, "HorSyncFront : %d\n", HorSyncFront));
  DEBUG ((DEBUG_INFO, "VerSyncFront : %d\n", VerSyncFront));
  DEBUG ((DEBUG_INFO, "HorSyncWidth : %d\n", HorSyncWidth));
  DEBUG ((DEBUG_INFO, "VerSyncWidth : %d\n", VerSyncWidth));
  DEBUG ((DEBUG_INFO, "HorPolarity : %d\n",  HorPolarity));
  DEBUG ((DEBUG_INFO, "VerPolarity : %d\n",  VerPolarity));
  //
  //parse dc sync table
  //
  TimingToDcSync (
              HorPixel,
              VerPixel,
              HorBlanking,
              VerBlanking,
              HorSyncFront,
              VerSyncFront,
              HorSyncWidth,
              VerSyncWidth,
              HorPolarity,
              VerPolarity,
              &Table->DcSync
              );
  //
  //parse dp sync table
  //
  TimingToDpSync (
              HorPixel,
              VerPixel,
              HorBlanking,
              VerBlanking,
              HorSyncFront,
              VerSyncFront,
              HorSyncWidth,
              VerSyncWidth,
              HorPolarity,
              VerPolarity,
              &Table->DpSync
              );
}

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
  )
{
  EFI_STATUS  Status;
  UINT32      Index;
  UINT32      Num;
  UINT8       CheckSum;

  Status = EFI_SUCCESS;
  //
  //clear dtd list
  //
  ZeroMem (List, sizeof (DTD_LIST));
  //
  //check sum
  //
  CheckSum = CalculateSum8 (Buffer, 128);
  if (CheckSum != 0) {
    DEBUG ((DEBUG_ERROR, "Edid checksum error, result : %x\n", CheckSum));
    Status = EFI_NOT_FOUND;
    goto Exit;
  }
  //
  //judge edid header
  //
  if ((Buffer[0] != 0) || (Buffer[7] != 0)) {
    DEBUG ((DEBUG_ERROR, "Edid header check failed!\n"));
    Status = EFI_NOT_FOUND;
    goto Exit;
  }
  for (Index = 1; Index < 7; Index++) {
    if (Buffer[Index] != 0xFF) {
      DEBUG ((DEBUG_ERROR, "Edid header check failed!\n"));
      Status = EFI_NOT_FOUND;
      goto Exit;
    }
  }
  //
  //parse dtd
  //
  List->DtdListNum = 0;
  for (Num = 0; Num < DTD_LIST_MAX_NUM; Num++) {
    Index = 54 + Num * 18;
    if ((Buffer[Index + 0] == 0) && (Buffer[Index + 1] == 0)
            && (Buffer[Index + 2] == 0) && (Buffer[Index + 4] == 0)) {
      continue;
    }
    List->DtdListNum++;
    ParseEdidDtdTable (&Buffer[Index], &List->DtdTable[Num]);
  }

Exit:
  return Status;
}

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
  )
{
  UINT32  Index;
  EFI_STATUS  Status;

  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < List->DtdListNum; Index++) {
      if ((GopMode->Width == List->DtdTable[Index].HorPixel)
              && (GopMode->Height == List->DtdTable[Index].VerPixel)) {
        CopyMem (Sync, &List->DtdTable[Index].DpSync, sizeof (DP_SYNC));
        *Clock = List->DtdTable[Index].PixelClock;
        Status = EFI_SUCCESS;
        break;
      }
  }
  return Status;
}

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
  )
{
  UINT32  Index;
  EFI_STATUS  Status;

  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < List->DtdListNum; Index++) {
      if ((GopMode->Width == List->DtdTable[Index].HorPixel)
              && (GopMode->Height == List->DtdTable[Index].VerPixel)) {
        CopyMem (Sync, &List->DtdTable[Index].DcSync, sizeof (DC_SYNC));
        Status = EFI_SUCCESS;
        break;
      }
  }
  return Status;
}

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
ConfigMainStreamAttr (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN DP_CONFIG         *DpConfig,
  IN DP_SYNC           *DpSyncTemp
  )
{
  UINT16 Value;

  ChannelRegWrite (Private, Num, DPTX_ENABLE_MAIN_STREAM, DpChOperate, 0);
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_HTOTAL,  DpChOperate, DpSyncTemp->HTotal);        //hTotal
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_VTOTAL,  DpChOperate, DpSyncTemp->VTotal);        //vTotal
  Value = 0x0;
  if (DpSyncTemp->HPolarity == 0) {
    Value &= 0xfffe;
  } else {
    Value |= 0x1;
  }
  if (DpSyncTemp->VPolarity == 0) {
    Value &= 0xfffd;
  } else {
    Value |= 0x2;
  }
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_POLARITY,  DpChOperate, Value);
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_HSWIDTH,  DpChOperate, DpSyncTemp->HsWidth);  //hsWidth
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_VSWIDTH,  DpChOperate, DpSyncTemp->VsWidth);  //vsWidth
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_HRES,  DpChOperate, DpSyncTemp->HRes);        //hRes
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_VRES,  DpChOperate, DpSyncTemp->VRes);        //vRes
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_HSTART,  DpChOperate, DpSyncTemp->HStart);    //hStart
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_VSTART,  DpChOperate, DpSyncTemp->VStart);    //vStart
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_MISC0,  DpChOperate, SetMainStreamMisc0 (DpConfig->BitDepth, DpConfig->ComponentFormat, DpConfig->ClockMode));
  ChannelRegWrite (Private, Num, DPTX_MAIN_LINK_MISC1,  DpChOperate, 0x0);
  //
  //1BC : ((Hres * bits_per _pixel+7)/8 + lane_count-1)/lane_count
  //1AC : pixel_clock(MHz)*100
  //1B0 : link_sr = lane_count * link_rate * 100    for example:link_rate=2.7 (2.7G)
  //      vid_sr = (pixel_clock*bpc*3)/8            for example:pixel_clock = 25.2 (25.2MHz)
  //      data_per_tu = (vid_sr/link_sr)*TU_size
  //
  ChannelRegWrite (Private, Num, DPTX_M_VID,  DpChOperate, SetMVID (DpConfig->PixelClock));
  ChannelRegWrite (Private, Num, DPTX_TRANSFER_UNIT_SIZE,  DpChOperate, SetTransUintSRC0 (DpConfig->PixelClock, DpConfig->LinkRate, DpConfig->BitDepth, DpConfig->LaneCount, DpConfig->TuSize));
  ChannelRegWrite (Private, Num, DPTX_N_VID,  DpChOperate, SetNVID (DpConfig->LinkRate));
  ChannelRegWrite (Private, Num, DPTX_DATA_PER_LANE,  DpChOperate, SetUserDataCount (DpSyncTemp->HRes, DpConfig->BitDepth, DpConfig->LaneCount));
  Value = 0xC;
  if (DpSyncTemp->HUserPolarity == 0) {
    Value &= 0xfffe;
  } else {
    Value |= 0x1;
  }
  if (DpSyncTemp->VUserPolarity == 0) {
    Value &= 0xfffd;
  } else {
    Value |= 0x2;
  }
  ChannelRegWrite (Private, Num, DPTX_USER_POLARITY,  DpChOperate, Value);
  ChannelRegWrite (Private, Num, TR_INPUT_COURCE_ENABLE,  DpChOperate, 1);
  ChannelRegWrite (Private, Num, DPTX_ENABLE_MAIN_STREAM,  DpChOperate, 1);
  ChannelRegWrite (Private, Num, DPTX_LINK_SOFT_RESET,  DpChOperate, 1);
}

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
  )
{
  UINT32  Hpd;
  UINT32  Status;
  UINT32  CheckTimes;

  Hpd = 0;
  //
  //0 - off, 1 - on
  //
  Status = DP_HPD_OFF;
  CheckTimes = HPD_CHECK_TIMES;

  while (CheckTimes > 0) {
    MicroSecondDelay ( HPD_CHECK_INTERVAL_MS * 1000);
    Hpd = ChannelRegRead (Private, Num, DPTX_SINK_HPD_STATE, DpChOperate);
    DEBUG ((DEBUG_INFO, "Hpd status checking! %d\n", CheckTimes));
    if ((Hpd & 0x01) == 0x01) {
      Status = DP_HPD_ON;
      break;
    }
    CheckTimes--;
  }
  return Status;
}

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
  )
{
  UINT32 Hpd;
  UINT32 Status;

  Hpd = ChannelRegRead (Private, Num, DPTX_SINK_HPD_STATE, DpChOperate);
  Hpd &= 0x1;
  if (Hpd == 0x1) {
    Status = DP_HPD_ON;
  }
  else {
    Status = DP_HPD_OFF;
  }
  return Status;
}

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
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  //
  //disable dptx
  //
  ChannelRegWrite (Private, Num, DPTX_ENABLE, DpChOperate, 0);
  //
  //dptx clk devide   100MHz / 100
  //
  ChannelRegWrite (Private, Num, DPTX_CLK_DIVIDER, DpChOperate, 100);
  //dptx timer init
  //DptxTimerInit(Private,Num);
  gBS->Stall (1 * 1000);
  //
  //enable dptx
  //
  ChannelRegWrite (Private, Num, DPTX_ENABLE, DpChOperate, 1);
  //mask all dptx interrupt
  //ChannelRegWrite (Private, Num, DPTX_INTERRUPT_MASK, 0x0);
  return EFI_SUCCESS;
}

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
  )
{
  UINT32  LinkRateHex;

  ASSERT (Num < DPDC_PATH_NUM);
  DEBUG ((DEBUG_INFO, "sink link rate : %d\n", LinkRate));
  LinkRateHex = LinkRate / 27;
  DEBUG ((DEBUG_INFO, "link rate hex : %x\n", LinkRateHex));
  ChannelRegWrite (Private, Num, DPTX_ENABLE, DpChOperate, 0);
  ChannelRegWrite (Private, Num, DPTX_LINK_BW_SET, DpChOperate, LinkRateHex);
  ChannelRegWrite (Private, Num, DPTX_LANE_COUNT_SET, DpChOperate, LaneCount);
  ChannelRegWrite (Private, Num, DPTX_LINK_SOFT_RESET, DpChOperate, 0x01);
  ChannelRegWrite (Private, Num, DPTX_ENABLE, DpChOperate, 1);

  return EFI_SUCCESS;
}

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
  )
{
  DptxPhyUpdateLinkRate (Private, Num, LinkRate);
  DptxPhySetLaneCount (Private, Num, LaneCount);

  return EFI_SUCCESS;
}

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
  )
{
  DptxPhySetPower (Private, Num, D0_MODE);
}

/**
  Enable or disable the enhanced framing mode of DPTX.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.
  @param[in]  Enable       Switch of enhanced framing mode.1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
EnableFramingMode (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  ChannelRegWrite (Private, Num, DPTX_ENHANCED_FRAME_EN, DpChOperate, Enable);
}

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
  IN  PHY_PRIVATE_DATA  *Private,
  IN  UINT8             Num,
  OUT UINT16            *Coreid,
  OUT UINT16            *Corerev
  )
{
  UINT32 Value;

  Value = ChannelRegRead (Private, Num, DPTX_CORE_ID, DpChOperate);
  *Coreid = (UINT16) (Value >> 16);
  *Corerev = (UINT16) Value;
  DEBUG ((DEBUG_INFO, "Core ID : %04x , Core Rev : %04x\n", *Coreid, *Corerev));
}

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
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, TrainPattern);
}

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
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  ChannelRegWrite (Private, Num, DPTX_SCRAMBLING_DISABLE, DpChOperate, Disable);
}

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
  )
{
  ASSERT (Num < DPDC_PATH_NUM);
  ChannelRegWrite (Private, Num, DPTX_FORCE_SCRAMBLER_RESET, DpChOperate, 1);
}

/**
  Force the transmitter core to use the alternate scrambler reset value.

  @param[in]  Private      Pointer to private data structure.
  @param[in]  Num          the index of dcdp , 0 or 1.

  @retval     LINK_TRAINING_TPS2   TPS2
              LINK_TRAINING_TPS3   TPS3
              LINK_TRAINING_TPS4   TPS4
**/
UINT32
DpTraingPattern (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  )
{
  if (DptxPhyTps4Supported (Private, Num)) {
    return LINK_TRAINING_TPS4;
  }
  if (DptxPhyTps3Supported (Private, Num)){
    return LINK_TRAINING_TPS3;
  }
  return LINK_TRAINING_TPS2;
}

/*
  Dp link training clock recovery.

  @param[in]      Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.
  @param[in,out]  *LaneCountNow    in - training start lanecount.
                                   out - lanecount when cr training successfully.
  @param[in,out]  *LaneRateNow     in - training start linkrate.
                                   out - linkrate when cr training successfully.
  @param[out]     *VsWing          in - training start voltage swing.
                                   out - voltage swing when cr training successfully.
  @param[out]     *PreEmphasis     in - training start pre-emphasis level.
                                   out - pre-emphasis levelwhen cr training successfully.

  @retval         0    CR training successfully.
                  -1   CR training failed.
*/
UINT32
DpLinkTrainingClockRecovery (
  IN      PHY_PRIVATE_DATA *Private,
  IN      UINT8            Num,
  IN OUT  UINT8            *LaneCountNow,
  IN OUT  UINT32           *LinkRateNow,
  IN OUT  UINT8            *VsWing,
  IN OUT  UINT8            *PreEmphasis
  )
{
  UINT8   EnhanceFrameCap;
  UINT32  Index;
  UINT32  IntervalTime;
  UINT32  VoltageTries;
  UINT32  MaxVswingTries;
  UINT8   TrainStatus;
  UINT8   Voltage[2];
  UINT8   Value[4];
  UINT32  Tps1Tries;
  //
  //config dptx lane count and link rate
  //
  EnhanceFrameCap = DptxPhyGetEnhancedFrameCap (Private, Num);
  EnableFramingMode (Private, Num, EnhanceFrameCap);
  ConfigDptx (Private, Num, *LinkRateNow, *LaneCountNow);
  //
  //config sink lane count and link rate
  //
  ConfigSink (Private, Num, *LinkRateNow, *LaneCountNow);
  if (Private->DownSpreadEnable[Num] == 1) {
    SinkDpcdWrite (Private, Num, DOWNSPREAD_CTRL, 0x10);
  } else {
    SinkDpcdWrite (Private, Num, DOWNSPREAD_CTRL, 0x0);
  }
  SinkDpcdWrite (Private, Num, MAIN_LINK_CHANNEL_CODING_SET, 0x1);
  //
  //get sink enhanced frame capacity , and config dptx enhanced frame capacity
  //EnhanceFrameCap = DptxPhyGetEnhancedFrameCap(Private,Num);
  //EnableFramingMode(Private,Num,EnhanceFrameCap);
  //
  ZeroMem (VsWing, 4);
  ZeroMem (PreEmphasis, 4);

  IntervalTime = 500;
  ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, 1);
  ChannelRegWrite (Private, Num, DPTX_SCRAMBLING_DISABLE, DpChOperate, 1);
  //DEBUG((DEBUG_INFO,"training pattern 1,clock recovery"));
  //DptxRegReadCheck(DpConfig,TR_DPTX_TRAINING_PATTERN_SET);
  //
  //set sink TPS1
  //
  DptxSetTrainingPar(Private,Num,VsWing,PreEmphasis,*LaneCountNow);
  //////gBS->Stall (IntervalTime);
  SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS1);

  VoltageTries = 1;
  Tps1Tries = 0;
  MaxVswingTries = 0;

  while(1) {
    gBS->Stall (IntervalTime);
    DptxGetTrainingStatus (Private, Num, Value);
    TrainStatus = CheckTrainingStatus (Private, Num, *LaneCountNow, LINK_TRAINING_TPS1, Value);
    //DEBUG((DEBUG_INFO,"training status : %d\n",TrainStatus));
    if (TrainStatus == 1) {
      return 0;   //success - 0
    }
    if (VoltageTries >= 5){
      DEBUG ((DEBUG_ERROR, "Same voltage tried 5 times\n"));
      return -1;
    }
    if (Tps1Tries >= 10) {
      return -1;
    }
    if (MaxVswingTries == 1) {
      DEBUG ((DEBUG_ERROR, "max valtage swing reached\n"));
      return -1;
    }
    Voltage[0] = VsWing[0];
    Voltage[1] = PreEmphasis[0];

    for (Index = 0; Index < *LaneCountNow; Index++) {
      VsWing[Index] = DptxSourceVswingForValue (Private, Num, Index);
      PreEmphasis[Index] = DptxSourcePreemphasisForValue (Private, Num, Index);
    }
    //////gBS->Stall (IntervalTime);
    DptxSetTrainingPar (Private, Num, VsWing, PreEmphasis, *LaneCountNow);
    Tps1Tries++;
    if (VsWing[0] == Voltage[0]) {
      VoltageTries++;
    }
    if (Voltage[0] >= 3) {
      MaxVswingTries++;
    }
    //DEBUG ((DEBUG_INFO, "train again : lane : %d , rate : %d, vs : %d , pe : %d,VoltageTries : %d, MaxVswingTries: %d\n",
      //*LaneCountNow, (UINT32) ((*LinkRateNow) * 100), VsWing[0], PreEmphasis[0], VoltageTries, MaxVswingTries));
  }
}

/*
  Dp link training Channel Equalization.

  @param[in]      Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.
  @param[in,out]  *LaneCountNow    in - training start lanecount.
                                   out - lanecount when eq training successfully.
  @param[in,out]  *LaneRateNow     in - training start linkrate.
                                   out - linkrate when cr training successfully.
  @param[out]     *VsWing          in - training start voltage swing.
                                   out - voltage swing when eq training successfully.
  @param[out]     *PreEmphasis     in - training start pre-emphasis level.
                                   out - pre-emphasis levelwhen eq training successfully.

  @retval         0    EQ training successfully.
                  -1   EQ training failed.
*/
UINT32
DpLinkTrainingChannelEqual (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN OUT UINT8        *LaneCountNow,
  IN OUT UINT32       *LinkRateNow,
  IN OUT UINT8        *VsWing,
  IN OUT UINT8        *PreEmphasis
  )
{
  UINT8   TrainStatus;
  UINT8   TrainingPattern;
  UINT32  Tries;
  UINT8   Value[4];

  DptxSetTrainingPar (Private, Num, VsWing, PreEmphasis, *LaneCountNow);
  gBS->Stall (20 * 1000);
  TrainingPattern = DpTraingPattern (Private, Num);
  //DEBUG((DEBUG_INFO,"tps support : %x\n",TrainingPattern));
  //TrainingPattern = LINK_TRAINING_TPS2;
  switch (TrainingPattern) {
  case LINK_TRAINING_TPS4:
    ChannelRegWrite (Private, Num, DPTX_SCRAMBLING_DISABLE, DpChOperate, 0);
    ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, 4);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, LINK_TRAINING_TPS4);
    break;
  case LINK_TRAINING_TPS3:
    ChannelRegWrite (Private, Num, DPTX_SCRAMBLING_DISABLE, DpChOperate, 1);
    ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, 3);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS3);
    break;
  case LINK_TRAINING_TPS2:
    ChannelRegWrite (Private, Num, DPTX_SCRAMBLING_DISABLE, DpChOperate, 1);
    ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, 2);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS2);
    break;
  default:
     break;
  }

  for (Tries = 0; Tries < 5; Tries++) {
    gBS->Stall (20 * 1000);
    DptxGetTrainingStatus (Private, Num, Value);
    TrainStatus = CheckTrainingStatus (Private, Num, *LaneCountNow, LINK_TRAINING_TPS1,Value);
    if (TrainStatus == 0) {
      return -1;
    }
    TrainStatus = CheckTrainingStatus (Private, Num, *LaneCountNow, TrainingPattern, Value);
    if (TrainStatus == 1) {
      return 0;
    }
    DptxPhyGetAdjustRequest (Private, Num, VsWing, PreEmphasis);
    DptxSetTrainingPar (Private, Num, VsWing, PreEmphasis, *LaneCountNow);
  }
  return -1;
}

/*
  Reduce the link rate and lane to the next supported gear.

  @param[in]      Private          Pointer to private data structure.
  @param[in]      Num              the index of dcdp , 0 or 1.
  @param[in,out]  *LaneCountNow    in - lane count before deceleration.
                                   out - lane count after deceleration.
  @param[in,out]  *LaneRateNow     in - link rate before deceleration.
                                   out - link rate after deceleration.
  @param[in]      MaxLaneCount     Supported max lane count.
  @param[in]      MaxLinkRate      Supported max link rate.

  @retval         0    deceleration success.
                  -1   deceleration failed, already lowest gear.
*/
UINT32
DpTrainingFailedDown (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN OUT UINT8        *LaneCountNow,
  IN OUT UINT32       *LinkRateNow,
  IN UINT8            MaxLaneCount,
  IN UINT32           MaxLinkRate
  )
{
  UINT8  LaneCount;
  UINT32 LinkRate;
  UINT32 Tlink;

  LaneCount = *LaneCountNow;
  LinkRate = *LinkRateNow;
  //DEBUG((DEBUG_INFO," failure -- lane count : %d, link rate : %d\n",*LaneCountNow,(UINT32)((*LinkRateNow)*100)));
  //DEBUG((DEBUG_INFO," failure -- lane count : %d, link rate : %d\n",LaneCount,(UINT32)(LinkRate*100)));
  if (mTrainFailedTimes < RETRAINING_TIMES_SAMESTEP) {
    mTrainFailedTimes++;
    return 0;
  } else {
    mTrainFailedTimes = 0;
  }
  Tlink = LinkRate;
  if (Tlink == 162) {
    if (LaneCount == 1) {
      return -1;
    } else if (LaneCount == 2) {
      *LinkRateNow = MaxLinkRate;
      *LaneCountNow = 1;
    } else if (LaneCount == 4) {
      *LinkRateNow = MaxLinkRate;
      *LaneCountNow = 2;
    }
  }
  else if (Tlink == 270) {
    *LinkRateNow = 162;
  } else if (Tlink == 540) {
    *LinkRateNow = 270;
  } else if (Tlink == 810) {
    *LinkRateNow = 540;
  }
  return 0;
}

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
  )
{
  UINT32 Status;
  UINT8  LaneCount;
  UINT32 LinkRate;
  UINT8  VsWing[4];
  UINT8  PreEmphasis[4];

  mTrainFailedTimes = 0;
  LaneCount = MaxLaneCount;
  LinkRate = MaxLinkRate;
  mTrainDownLinkrate = 0;
Again:
  ChannelRegWrite (Private, Num, DPTX_TRAINING_PATTERN_SET, DpChOperate, 0);
  SinkDpcdWrite (Private, Num, 0x102, 0x0);
  LinkPhyChangeRate (Private, LinkRate, Num);
  gBS->Stall (1000);
  Status = DpLinkTrainingClockRecovery (Private, Num, &LaneCount, &LinkRate, VsWing, PreEmphasis);
  if (Status != 0) {
    goto Failure;
  }
  Status = DpLinkTrainingChannelEqual (Private, Num, &LaneCount, &LinkRate, VsWing, PreEmphasis);
  if (Status != 0) {
    goto Failure;
  }
  Private->DpConfig[Num].LinkRate = LinkRate;
  Private->DpConfig[Num].LaneCount = LaneCount;
  //DEBUG ((DEBUG_INFO, "sink connect link rate : %d, lane count : %d\n", (UINT32) (LinkRate * 100), LaneCount));
  Private->DpIsTrained[Num] = 1;
  return SINK_CONNECT;
Failure:
  Status = DpTrainingFailedDown (Private, Num, &LaneCount, &LinkRate, MaxLaneCount, MaxLinkRate);
  DEBUG ((DEBUG_INFO, "Lane:%d,Max Lane:%d,LinkRate:%d,MaxLinkRate:%d\n", LaneCount, LinkRate, MaxLaneCount, MaxLinkRate));
  if (Status == 0) {
    if ((LaneCount != MaxLaneCount) || (LinkRate != MaxLinkRate)) {
      mTrainDownLinkrate = 1;
    }
    goto Again;
  }
  Private->DpIsTrained[Num] = 0;
  return SINK_DISCONNECT;
}

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
)
{
  UINT32  Index;
  UINT8   Value;
  UINT32  Status;
  UINT8   LaneCount;
  UINT32  LinkRate;

  DEBUG ((DEBUG_INFO, "%a(), %d\n", __FUNCTION__, __LINE__));
  Status = DptxPhyGetLaneCount (Private, Num, &LaneCount);
  if (Status == AUX_TIMEOUT) {
    return SINK_DISCONNECT;
  }
  Status = DptxPhyGetLinkRate (Private, Num, &LinkRate);
  if (Status == AUX_TIMEOUT) {
    return SINK_DISCONNECT;
  }
  DEBUG ((DEBUG_INFO, "max lane count support: %d\n", LaneCount));
  DEBUG ((DEBUG_INFO, "max link rate support: %d\n", (UINT32) (LinkRate * 100)));
  gBS->Stall(1 * 1000);
  for (Index = 0; Index < 16; Index++) {
    SinkDpcdRead (Private, Num, Index, &Value);
    //DEBUG ((DEBUG_INFO,"dpcd %03x : %x\n", Index, Value));
  }
  SinkDpcdRead (Private, Num, SINK_COUNT, &Value);
  if (LaneCount > 1) {
    LaneCount = 1;
  }
  Status = DpStartLinkTrain (Private, Num, LaneCount, LinkRate);
  SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x0);
  if (mTrainDownLinkrate != 0) {
    Status = DpStartLinkTrain (Private, Num, LaneCount, LinkRate);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x0);
  }
  return Status;
}

