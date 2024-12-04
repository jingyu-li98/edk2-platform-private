/* @file
** DpDrv.c
** Copyright (c) 2020 - 2022, Phytium Technology Co., Ltd. All rights reserved.
** SPDX-License-Identifier: BSD-2-Clause-Patent
*/

#include "DpDrv.h"
#include "DpFun.h"
#include "DpPhy.h"
#include "DpType.h"

#include "../PhyGopDxe.h"

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

/**
  Enable or disable the dp timer.The timer block must be initialized with DptxTimerInit before calling this function.
  Disabling the timer pauses the countdown and preserves the value of the register.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  enable       Switch of dp timer.1 - enable , 0 - disable.

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
  TimerReg = PhyReadMmio (Private, Num, DPTX_TIMER);
  if (Enable){
    TimerReg = TimerReg | TR_DPTX_TIMER_FLAG_ENABLE;
  }
  else{
    TimerReg = TimerReg & (~TR_DPTX_TIMER_FLAG_ENABLE);
  }
  PhyWriteMmio (Private, Num, DPTX_TIMER, TimerReg);
}

/**
  Set the current value of the Displayport TX timer to the specified value.This value must be specified in system ticks.
  The resolution of the timer is 1 usec.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  new_value    The value to set the timer to in system ticks.The resolution of the timer is 1 usec.

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
  TimerReg = PhyReadMmio (Private, Num, DPTX_TIMER);
  TimerReg = ((TimerReg & TR_DPTX_TIMER_FLAGS_ALL) | (TR_DPTX_TIMER_MAX_COUNT & NewValue));
  PhyWriteMmio (Private, Num, DPTX_TIMER, TimerReg);
}

/**
  Return the current value of the Displayport TX timer. This is a 32-bit interger value equal to the Number of
  microseconds remaining in the timer. This function can be called in a loop, checking the return value to
  determine when the timer has expired.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @retval     Current value of the Displayport TX timer
**/
UINT32
DptxTimerGetValue (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  return (PhyReadMmio (Private, Num, DPTX_TIMER) & TR_DPTX_TIMER_MAX_COUNT);
}

/**
  Set the flags for the Displayport TX timer. The available flags are as
  follows:
    TR_DPTX_TIMER_FLAG_ENABLE          - Enable the timer
    TR_DPTX_TIMER_FLAG_AUTORELOAD_EN   - Enable automatic reload of the initial value
    TR_DPTX_TIMER_FLAG_INTERRUPT_EN    - Enable generation of an interrupt upon expiration
    TR_DPTX_TIMER_FLAGS_ALL            - All flags logically OR'd together for convenience

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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
  TimerReg = PhyReadMmio (Private, Num, DPTX_TIMER);
  //
  // Clear flags, masking off timer value, mask invalid flags
  //
  TimerReg = (TimerReg & TR_DPTX_TIMER_MAX_COUNT) | (Flags & TR_DPTX_TIMER_FLAGS_ALL);
  PhyWriteMmio (Private, Num, DPTX_TIMER, TimerReg);
}

/**
  Initialize the timer block in the Displayport TX core. This diables the timer, sets the initial value to 0
  and clears all flags. This function must be called before using the timer block or the behavior will be unpredictable.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  us           An integer value in microseconds to wait.

  @retval     NULL
**/
VOID
DptxTimerWait (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            Us
  )
{
  ASSERT (Num < 3);
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
    if (DptxTimerGetValue (Private,Num) == 0)
      break;
  }
}

/**
  Wait util that an aux reply has been recieved.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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
  ASSERT(Num < 3);
  while (ReplyTimes > 0) {
    gBS->Stall (2 * 1000);
    //InitStatus = PhyReadMmio(Private,Num,0x3130);
    AuxStatus = PhyReadMmio (Private, Num, 0x314c);
    PhyReadMmio (Private, Num, 0x138);
    InitStatus = PhyReadMmio (Private, Num, 0x3130);
    if ((InitStatus & 0x08) != 0) {
      Status = AUX_TIMEOUT;
      break;
    }
    else if ((InitStatus & 0x04) != 0) {
        if((AuxStatus & 0x1) != 0) {
            Status = AUX_CONNECT;
            break;
        }
    }
    ReplyTimes--;
  }
  //PhyReadMmio(Private,Num,0x3140);
  return Status;
}

/**
  Read phy register through aux channel.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  dpcdAddr     Dpcd regiter address.Twenty-bit address for the start of the AUX Channel burst

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
  ASSERT (Num < 3);
  //while(PhyReadMmio(Private,Num,0x3130)&0x02);
  PhyReadMmio (Private, Num, 0x3140);
  PhyReadMmio (Private, Num, 0x3130);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, DpcdAddr);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x900);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT){
    return Status;
  }
  gBS->Stall (100);
  PhyReadMmio (Private, Num, 0x13c);
  PhyReadMmio (Private, Num, 0x138);
  *Data = PhyReadMmio (Private, Num, DPTX_RCV_FIFO);
#ifdef DPCD_DEBUG
  DEBUG ((DEBUG_INFO,"Dpcd Read addr : 0x%x , data : 0x%x\n", DpcdAddr, *Data));
#endif
  return AUX_CONNECT;
}

/**
  Read phy register through aux channel.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  dpcdAddr     Dpcd regiter address.Twenty-bit address for the start of the AUX Channel burst

  @param[in]  writeData    The data that you want to write to the sink regiter through AUX channel.

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
  //while(PhyReadMmio(Private,Num,0x3130)&0x02);
  PhyReadMmio (Private, Num, 0x3140);
  PhyReadMmio (Private, Num, 0x3130);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, DpcdAddr);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  gBS->Stall (100);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x800);
  Status = WaitAuxReply (Private, Num);
#ifdef DPCD_DEBUG
  DEBUG ((DEBUG_INFO,"Dpcd Write addr : 0x%x , data : 0x%x , Status : %d\n",DpcdAddr, WriteData, Status));
#endif
  return Status;
}

#if 1
UINT32
AuxTest(
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT32             Num,
  IN UINT32             Times
)
{
  UINT32  i;
  UINT32  WStatus;
  UINT32  RStatus;
  UINT8   WData;
  UINT8   RData;
  DEBUG((DEBUG_INFO,"---------------------------"));
  DEBUG((DEBUG_INFO,"Aux Test Begin dp[%d]\n",Num));
  for(i=0;i<Times;i++){
#if 0
    WData = 0x01;
    WStatus = SinkDpcdWrite(Private, Num, 0x100, WData);
    gBS->Stall(1000);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    if(WData == RData)/*{}*/
      DEBUG((DEBUG_INFO,"times : %d ,Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i, RData, WData, WStatus, RStatus));
    else
      DEBUG((DEBUG_INFO,"times : %d ,Not Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));


    WData = 0x02;
    WStatus = SinkDpcdWrite(Private, Num, 0x100, WData);
    gBS->Stall(1000);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    if(WData == RData)/*{}*/
      DEBUG((DEBUG_INFO,"times : %d ,Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
    else
      DEBUG((DEBUG_INFO,"times : %d ,Not Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));


    WData = 0x3;
    WStatus = SinkDpcdWrite(Private, Num, 0x100, WData);
    gBS->Stall(1000);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    if(WData == RData)/*{}*/
      DEBUG((DEBUG_INFO,"times : %d ,Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
    else
      DEBUG((DEBUG_INFO,"times : %d ,Not Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));


    WData = 0x4;
    WStatus = SinkDpcdWrite(Private, Num, 0x100, WData);
    gBS->Stall(1000);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    if(WData == RData)/*{}*/
      DEBUG((DEBUG_INFO,"times : %d ,Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
    else
      DEBUG((DEBUG_INFO,"times : %d ,Not Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
#endif
#if 1
    WData = 0x55;
    WStatus = SinkDpcdWrite(Private, Num, 0x100, WData);
    RStatus = SinkDpcdRead(Private, Num, 0x100, &RData);
    if(WData == RData)/*{}*/
      DEBUG((DEBUG_INFO,"times : %d ,Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
    else
      DEBUG((DEBUG_INFO,"times : %d ,Not Equal -- RData : %02x , WData : %02x , WStatus : %d, RStatus : %d\n",i,  RData, WData, WStatus, RStatus));
    gBS->Stall(2*1000*1000);
#endif
  }
  DEBUG((DEBUG_INFO,"Aux Test Finish----------------------\n"));
  return 0;
}
#endif

UINT32
GetEdid(
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
  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1400);
  Status = WaitAuxReply (Private,Num);
  Data = PhyReadMmio (Private, Num, 0x3138);
  //DEBUG ((DEBUG_INFO,"--------------------"));


  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x400);
  Status = WaitAuxReply (Private,Num);
  Data = PhyReadMmio (Private, Num, 0x3138);
  //DEBUG ((DEBUG_INFO,"--------------------"));


  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  //DEBUG ((DEBUG_INFO,"--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  Data = PhyReadMmio (Private, Num, 0x3134);
  //DEBUG ((DEBUG_INFO,"--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  Data = PhyReadMmio (Private, Num, 0x3134);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  //edid 0 128B
  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1400);
  Status = WaitAuxReply (Private, Num);
  Data = PhyReadMmio (Private, Num,0x3138);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x400);
  Status = WaitAuxReply (Private, Num);
  Data = PhyReadMmio (Private, Num, 0x3138);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  for (Ii = 0; Ii < 8; Ii++) {
    while (PhyReadMmio (Private, Num, 0x130) & 0x02);
    PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
    PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x50f);
    Status = WaitAuxReply (Private, Num);
    if (Status == AUX_TIMEOUT) {
      return Status;
    }
    Data = PhyReadMmio (Private, Num, 0x3138);
    Data = PhyReadMmio (Private, Num, 0x3148);
    for (Jj = 0; Jj < 16; Jj++) {
      Data = PhyReadMmio (Private, Num, 0x3134);
      *(Buffer + Ii * 16 + Jj) = Data;
      DEBUG ((DEBUG_INFO, "edid[%d] : 0x%x\n", Ii * 16 + Jj, Data));
    }
    //DEBUG ((DEBUG_INFO, "--------------------"));
  }

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  Data = PhyReadMmio (Private, Num, 0x3134);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  //edid 1 128B
  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1400);
  Status = WaitAuxReply (Private, Num);
  Data = PhyReadMmio (Private, Num, 0x3138);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_FIFO, WriteData);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x400);
  Status = WaitAuxReply (Private, Num);
  Data = PhyReadMmio (Private, Num, 0x3138);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1500);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  //DEBUG ((DEBUG_INFO, "--------------------"));

  for (Ii = 0; Ii < 8; Ii++) {
    while (PhyReadMmio (Private, Num, 0x130) & 0x02);
    PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
    PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x50f);
    Status = WaitAuxReply (Private, Num);
    if (Status == AUX_TIMEOUT) {
      return Status;
    }
    Data = PhyReadMmio (Private, Num, 0x3138);
    Data = PhyReadMmio (Private, Num, 0x3148);
    for (Jj = 0; Jj < 16; Jj++) {
      Data = PhyReadMmio (Private, Num, 0x3134);
      DEBUG ((DEBUG_INFO, "edid[%d] : 0x%x\n", Ii * 16 + Jj, Data));
      *(Buffer + 128 + Ii * 16 + Jj) = Data;
    }
    //DEBUG ((DEBUG_INFO, "--------------------"));
  }

  while (PhyReadMmio (Private, Num, 0x130) & 0x02);
  PhyWriteMmio (Private, Num, DPTX_ADDRESS_REG, 0x50);
  PhyWriteMmio (Private, Num, DPTX_CMD_REG, 0x1100);
  Status = WaitAuxReply (Private, Num);
  if (Status == AUX_TIMEOUT) {
    return Status;
  }
  Data = PhyReadMmio (Private, Num, 0x3138);
  Data = PhyReadMmio (Private, Num, 0x3148);
  Data = PhyReadMmio (Private, Num, 0x3134);
  //DEBUG ((DEBUG_INFO, "--------------------"));
  return 0;
}

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
  Config Main Stream Attributes.It is must to reset reset phy link after main stream attributes configuration.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  DpConfig     Pointer to dp configuration structure include lane count , clock configuration , video mode.

  @param[in]  dp_sync      Pointer to dp timing parameter structure.

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
  PhyWriteMmio (Private, Num, 0x3084,0);
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_HTOTAL, DpSyncTemp->HTotal);        //hTotal
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_VTOTAL, DpSyncTemp->VTotal);        //vTotal
  Value = 0x0;
  if (DpSyncTemp->HPolarity == 0)       //bit 0
    Value &= 0xfffe;
  else
    Value |= 0x1;
  if (DpSyncTemp->VPolarity == 0)       //bit 1
    Value &= 0xfffd;
  else
    Value |= 0x2;
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_POLARITY, Value);
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_HSWIDTH, DpSyncTemp->HsWidth);  //hsWidth
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_VSWIDTH, DpSyncTemp->VsWidth);  //vsWidth
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_HRES, DpSyncTemp->HRes);        //hRes
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_VRES, DpSyncTemp->VRes);        //vRes
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_HSTART, DpSyncTemp->HStart);    //hStart
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_VSTART, DpSyncTemp->VStart);    //vStart
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_MISC0, SetMainStreamMisc0 (DpConfig->BitDepth, DpConfig->ComponentFormat, DpConfig->ClockMode));
  PhyWriteMmio (Private, Num, DPTX_MAIN_LINK_MISC1, 0x0);
  //
  //1BC : ((Hres * bits_per _pixel+7)/8 + lane_count-1)/lane_count
  //1AC : pixel_clock(MHz)*100
  //1B0 : link_sr = lane_count * link_rate * 100    for example:link_rate=2.7 (2.7G)
  //      vid_sr = (pixel_clock*bpc*3)/8            for example:pixel_clock = 25.2 (25.2MHz)
  //      data_per_tu = (vid_sr/link_sr)*TU_size
  //
  PhyWriteMmio (Private, Num, DPTX_M_VID, SetMVID (DpConfig->PixelClock));
  PhyWriteMmio (Private, Num, DPTX_TRANSFER_UNIT_SIZE, SetTransUintSRC0 (DpConfig->PixelClock, DpConfig->LinkRate, DpConfig->BitDepth, DpConfig->LaneCount, DpConfig->TuSize));
  PhyWriteMmio (Private, Num, DPTX_N_VID, SetNVID (DpConfig->LinkRate));
  PhyWriteMmio (Private, Num, DPTX_DATA_PER_LANE, SetUserDataCount (DpSyncTemp->HRes, DpConfig->BitDepth, DpConfig->LaneCount));
  Value = 0xC;
  if (DpSyncTemp->HUserPolarity == 0)       //bit 0
    Value &= 0xfffe;
  else
    Value |= 0x1;
  if (DpSyncTemp->VUserPolarity == 0)       //bit 1
    Value &= 0xfffd;
  else
    Value |= 0x2;
  PhyWriteMmio (Private, Num, DPTX_USER_POLARITY, Value);
  PhyWriteMmio (Private, Num, TR_INPUT_COURCE_ENABLE, 1);
  //MicroSecondDelay (1 * 1000);
  PhyWriteMmio (Private, Num, DPTX_ENABLE_MAIN_STREAM, 1);
  //MicroSecondDelay (1 * 1000);
  PhyWriteMmio (Private, Num, DPTX_LINK_SOFT_RESET, 1);
}

/**
  Check the HPD status.True is HPD is asserted, false otherwise.The checking time interval is 500ms.Check five times.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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
  Status = DP_HPD_OFF;     //0-off,1-on
  CheckTimes = HPD_CHECK_TIMES;
  while (CheckTimes > 0) {
    Hpd = PhyReadMmio (Private, Num, DPTX_SINK_HPD_STATE);
    DEBUG ((DEBUG_INFO, "Hpd status checking! %d\n", CheckTimes));
    if ((Hpd & 0x01) == 0x01) {
      Status = DP_HPD_ON;
      break;
    }
    MicroSecondDelay ( HPD_CHECK_INTERVAL_MS * 1000);
    CheckTimes--;
  }
  return Status;
}

UINT32
GetHpdStatusOnce (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  UINT32 Hpd;
  UINT32 Status;
  Hpd = PhyReadMmio(Private,Num,DPTX_SINK_HPD_STATE);
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
  Initialize AUX channel include aux clock Initialization, dp timer Initialization and interrupt mask.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @retval     NULL
**/
UINT32
InitAux (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num
  )
{
  ASSERT (Num < 3);
  //disable dptx
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 0);
  //dptx clk devide   48MHz/48
  PhyWriteMmio (Private, Num, DPTX_CLK_DIVIDER, 48);
  //dptx timer init
  //DptxTimerInit(Private,Num);
  gBS->Stall (1 * 1000);
  //enable dptx
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 1);
  //mask all dptx interrupt
  //PhyWriteMmio (Private, Num, DPTX_INTERRUPT_MASK, 0x0);
  return EFI_SUCCESS;
}

/**
  Configurate link rate and lane count of dptx.Lane count is one of 1 ,2 ,4.The unit of link rate is GHz.Link rate value is one of
  1.62 , 2.7 , 5.4 , 8.1.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  LinkRate     Main link bandwidth , 1.62 , 2.7 , 5.4 , or 8.1.

  @param[in]  LaneCount    The Number of lanes , 1 , 2 or 4.

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
  ASSERT (Num < 3);
  DEBUG ((DEBUG_INFO, "sink link rate : %d\n", LinkRate));
  LinkRateHex = LinkRate / 27;
  DEBUG ((DEBUG_INFO, "link rate hex : %x\n", LinkRateHex));
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 0);
  PhyWriteMmio (Private, Num, DPTX_LINK_BW_SET, LinkRateHex);
  PhyWriteMmio (Private, Num, DPTX_LANE_COUNT_SET, LaneCount);
  PhyWriteMmio (Private, Num, DPTX_LINK_SOFT_RESET, 0x01);
  MicroSecondDelay (1 * 1000);
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 1);
  return EFI_SUCCESS;
}

/**
  Configurate link rate and lane count of sink device.Lane count is one of 1 ,2 ,4.The unit of link rate is GHz.Link rate value is one of
  1.62 , 2.7 , 5.4 , 8.1.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  LinkRate     Main link bandwidth , 1.62 , 2.7 , 5.4 , or 8.1.

  @param[in]  LaneCount    The Number of lanes , 1 , 2 or 4.

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

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

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

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  enable       Switch of enhanced framing mode.1 - enable , 0 - disable.

  @retval     NULL
**/
VOID
EnableFramingMode (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN BOOLEAN           Enable
  )
{
  PhyWriteMmio (Private, Num, DPTX_ENHANCED_FRAME_EN, Enable);
}

/**
  Get the unique identification code of the core and the current revision level.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[out] coreid       Pointer to Core ID information.

  @param[out] corerev      Pointer to Core revision level information.

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
  Value = PhyReadMmio (Private, Num, DPTX_CORE_ID);
  *Coreid = (UINT16) (Value >> 16);
  *Corerev = (UINT16) Value;
  DEBUG ((DEBUG_INFO, "Core ID : %04x , Core Rev : %04x\n", *Coreid, *Corerev));
}

#if 0
/**
  Sets the current link rate for the TX core. This programs the LINK_BW register in the TX core and calls
  the PHY's link rate update function to set the proper clock speeds for the main link.  PHY reset is sent
  to the transmitter to allow the internal PLLs to lock to the new clock rate.Speed is 1.62 ,
  2.7 , 5.4 , or 8.1 in GHz unit.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  speed        Main link bandwidth , 1.62 , 2.7 , 5.4 , or 8.1.

  @retval     EFI_SUCCESS  Configurate successfully.

  @retval     other        Configurate failed.
**/
UINT32
DptxSetLinkRate (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            Speed
  )
{
  UINT32 Status;
  Status = 0;
  ASSERT(Num < 3);
  //
  // Disable the core before continuing
  //
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 0);
  //
  // Set the link rate in the core (for completeness)
  //
  PhyWriteMmio (Private, Num, DPTX_LINK_BW_SET, Speed);
  //
  // Set the link rate in the PHY
  //
  status = DptxPhyUpdateLinkRate (Private, Num, Speed);
  //
  // Reset the link
  //
  PhyWriteMmio (Private, Num, DPTX_LINK_SOFT_RESET, 0x01);
  //DptxTimerWait (Private, Num, 2);
  MicroSecondDelay (2 * 1000);
  //
  // Reenable the core for use
  //
  PhyWriteMmio (Private, Num, DPTX_ENABLE, 1);
  return status;
}

/**
  Set the Number of lanes for the transmitter link. A reset applied if needed for the PHY.
  Valid values for the lane count are 1, 2 and 4.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  lane_count   The Number of lane , 1 , 2 or 4.

  @retval     NULL
**/
VOID
DptxSetLaneCount (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT32            LaneCount
  )
{
  UINT32 PowerState;
  UINT32 CurrentLaneCount;
  PowerState = 0x00;
  CurrentLaneCount = PhyReadMmio (Private, Num, DPTX_LANE_COUNT_SET);
  //
  // reconfigure the core and PHY only on a change in the Number of lanes
  //
  if (CurrentLaneCount != LaneCount) {
    PhyWriteMmio (Private, Num, DPTX_LANE_COUNT_SET, LaneCount);
    if (LaneCount == 2){
      power_state = 0x0c;
    }
    if (LaneCount == 1)
      power_state = 0x0e;
    PhyWriteMmio(Private,Num,DPTX_PHY_POWER_DOWN, power_state);
    DptxPhySetLaneCount (Private, Num, LaneCount);
    //
    // reset the entire PHY state on a lane change
    //
    //dptx_phy_reset(DpConfig,0xff);
  }
}
#endif

/**
  This is a pass-through function that sets the voltage swing levels for the PHY. The PHY driver sets the registers
  in the TX core as appropriate for the selected voltage swing levels. The registers programmed in the TX core are
  only required for certain PHYs and are contained in the PHY drivers where necessary.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  vs0          voltage swing levels for lane 0.

  @param[in]  vs1          voltage swing levels for lane 1.

  @param[in]  vs2          voltage swing levels for lane 2.

  @param[in]  vs3          voltage swing levels for lane 3.

  @retval     NULL
**/
VOID
DptxSetVswing (
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Vs0,
  IN UINT8             Vs1,
  IN UINT8             Vs2,
  IN UINT8             Vs3
  )
{
  DptxPhySetVswing (Private, Num, 0, Vs0);
  DptxPhySetVswing (Private, Num, 1, Vs1);
  DptxPhySetVswing (Private, Num, 2, Vs2);
  DptxPhySetVswing (Private, Num, 3, Vs3);
}

/**
  This is a pass-through function that sets the pre-emphasis levels for the PHY. The PHY driver sets the registers
  in the TX core as appropriate for the selected pre-emphasis levels. The registers programmed in the TX core are
  only required for certain PHYs and are contained in the PHY drivers where necessary.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  pe0          pre-emphasis levels for lane 0.

  @param[in]  pe1          pre-emphasis levels for lane 1.

  @param[in]  pe2          pre-emphasis levels for lane 2.

  @param[in]  pe3          pre-emphasis levels for lane 3.

  @retval     NULL
**/
VOID
DptxSetPreemphasis(
  IN PHY_PRIVATE_DATA  *Private,
  IN UINT8             Num,
  IN UINT8             Pe0,
  IN UINT8             Pe1,
  IN UINT8             Pe2,
  IN UINT8             Pe3
  )
{
  DptxPhySetPreemphasis (Private, Num, 0, Pe0);
  DptxPhySetPreemphasis (Private, Num, 1, Pe1);
  DptxPhySetPreemphasis (Private, Num, 2, Pe2);
  DptxPhySetPreemphasis (Private, Num, 3, Pe3);
}

/**
  sets the output of the transmitter core to the specified training pattern. When set, all other main link information
  such as video and audio data are blocked in favor of the training pattern.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  trainPattern link training pattern.
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
  ASSERT (Num < 3);
  PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, TrainPattern);
}

/**
  Enable or disable the internal scrambling function of the DisplayPort transmitter.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @param[in]  disable      Switch of internal scrambling function. 1 - disable , 0 - enable.

  @retval     NULL
**/
VOID
DptxDisableScrambling (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num,
  IN UINT8            Disable
  )
{
  ASSERT (Num < 3);
  PhyWriteMmio (Private, Num, DPTX_SCRAMBLING_DISABLE, Disable);
}

/**
  Force the transmitter core to use the alternate scrambler reset value.

  @param[in]  Private      Pointer to private data structure.

  @param[in]  Num          the index of dcdp , 0 , 1 or 2.

  @retval     NULL
**/
VOID
DptxScramblingRest (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8            Num
  )
{
  ASSERT (Num < 3);
  PhyWriteMmio (Private, Num, DPTX_FORCE_SCRAMBLER_RESET, 1);
}

UINT32
DpTraingPattern (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num
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

UINT32
DpLinkTrainingClockRecovery (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num,
  IN UINT8            *LaneCountNow,
  IN UINT32           *LinkRateNow,
  UINT8               *VsWing,
  UINT8               *PreEmphasis
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
  }
  else {
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
  MicroSecondDelay (IntervalTime);
  MicroSecondDelay (IntervalTime);
  PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, 1);
  PhyWriteMmio (Private, Num, DPTX_SCRAMBLING_DISABLE, 1);
  //DEBUG((DEBUG_INFO,"training pattern 1,clock recovery"));
  //DptxRegReadCheck(DpConfig,TR_DPTX_TRAINING_PATTERN_SET);
  //set sink TPS1
  DptxSetTrainingPar(Private,Num,VsWing,PreEmphasis,*LaneCountNow);
  gBS->Stall (IntervalTime);
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
    gBS->Stall (IntervalTime);
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

UINT32
DpLinkTrainingChannelEqual (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num,
  IN UINT8            *LaneCountNow,
  IN UINT32           *LinkRateNow,
  UINT8               *VsWing,
  UINT8               *PreEmphasis
  )
{
  //UINT32  Index;
  UINT8   TrainStatus;
  UINT8   TrainingPattern;
  UINT32  Tries;
  UINT8   Value[4];

  DptxSetTrainingPar (Private, Num, VsWing, PreEmphasis, *LaneCountNow);
  gBS->Stall (20*1000);
  TrainingPattern = DpTraingPattern (Private, Num);
  //DEBUG((DEBUG_INFO,"tps support : %x\n",TrainingPattern));
  //TrainingPattern = LINK_TRAINING_TPS2;
  switch (TrainingPattern) {
  case LINK_TRAINING_TPS4:
    PhyWriteMmio (Private, Num, DPTX_SCRAMBLING_DISABLE, 0);
    PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, 4);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, LINK_TRAINING_TPS4);
    break;
  case LINK_TRAINING_TPS3:
    PhyWriteMmio (Private, Num, DPTX_SCRAMBLING_DISABLE, 1);
    PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, 3);
    SinkDpcdWrite (Private, Num, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS3);
    break;
  case LINK_TRAINING_TPS2:
    PhyWriteMmio (Private, Num, DPTX_SCRAMBLING_DISABLE, 1);
    PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, 2);
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
      return -1;   //success - 0
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

UINT32
DpTrainingFailedDown (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num,
  UINT8               *LaneCountNow,
  UINT32              *LinkRateNow,
  UINT8               MaxLaneCount,
  UINT32              MaxLinkRate
  )
{
  UINT8  LaneCount;
  UINT32 LinkRate;
  UINT32 Tlink;
  LaneCount = *LaneCountNow;
  LinkRate = *LinkRateNow;
  //DEBUG((DEBUG_INFO," failure -- lane count : %d, link rate : %d\n",*LaneCountNow,(UINT32)((*LinkRateNow)*100)));
  //DEBUG((DEBUG_INFO," failure -- lane count : %d, link rate : %d\n",LaneCount,(UINT32)(LinkRate*100)));
  Tlink = LinkRate;
  if (Tlink == 162) {
    //DEBUG((DEBUG_INFO,"1.62 1.62\n"));
    if (LaneCount == 1) {
      return -1;
    }
    else if (LaneCount == 2) {
      *LinkRateNow = MaxLinkRate;
      *LaneCountNow = 1;
    }
    else if (LaneCount == 4) {
      *LinkRateNow = MaxLinkRate;
      *LaneCountNow = 2;
    }
  }
  else if (Tlink == 270) {
    *LinkRateNow = 162;
  }
  else if (Tlink == 540) {
    *LinkRateNow = 270;
  }
  else if (Tlink == 810) {
    *LinkRateNow = 540;
  }
  return 0;
}

UINT32
DpStartLinkTrain (
  IN PHY_PRIVATE_DATA *Private,
  IN UINT8        Num,
  UINT8               MaxLaneCount,
  UINT32              MaxLinkRate
  )
{
  UINT32 Status;
  UINT8  LaneCount;
  UINT32 LinkRate;
  LaneCount = MaxLaneCount;
  LinkRate = MaxLinkRate;
  UINT8   VsWing[4];
  UINT8   PreEmphasis[4];
Again:
  PhyWriteMmio (Private, Num, DPTX_TRAINING_PATTERN_SET, 0);
  SinkDpcdWrite (Private, Num, 0x102, 0x0);
  LinkPhyChangeRate (Private, LinkRate, Num);
  gBS->Stall(1000);
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
  if (Status == 0) {
    goto Again;
  }
  Private->DpIsTrained[Num] = 0;
  return SINK_DISCONNECT;
}

UINT32
DpConnect (
  IN OUT PHY_PRIVATE_DATA  *Private,
  IN   UINT8         Num
)
{
  UINT32  Index;
  UINT8   Value;
  UINT32  Status;
  UINT8   LaneCount;
  UINT32  LinkRate;
  Status = DptxPhyGetLaneCount (Private, Num, &LaneCount);
  if (Status == AUX_TIMEOUT) {
    return SINK_DISCONNECT;
  }
  Status = DptxPhyGetLinkRate (Private, Num, &LinkRate);
  if (Status == AUX_TIMEOUT) {
    return SINK_DISCONNECT;
  }
  //DEBUG ((DEBUG_INFO, "max lane count support: %d\n", LaneCount));
  //DEBUG ((DEBUG_INFO, "max link rate support: %d\n", (UINT32) (LinkRate * 100)));
  gBS->Stall(10 * 1000);
  for (Index = 0; Index < 16; Index++) {
    SinkDpcdRead (Private, Num, Index, &Value);
    //DEBUG ((DEBUG_INFO,"dpcd %03x : %x\n", Index, Value));
  }
  SinkDpcdRead (Private, Num, 0x200, &Value);
  if (Num == 2) {
    LaneCount = 1;
  }
  //if(LinkRate >= 540)
   //LinkRate = 540;
  Status = DpStartLinkTrain (Private, Num, LaneCount, LinkRate);
  SinkDpcdWrite (Private, Num, 0x102, 0x0);
  return Status;
}

