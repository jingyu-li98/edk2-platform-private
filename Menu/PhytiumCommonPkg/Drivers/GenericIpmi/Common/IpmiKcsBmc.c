/** @file
KCS operation interfaces.

Copyright (C) 2023, Phytium Technology Co., Ltd. All rights reserved.<BR>
Copyright 1999 - 2021 Intel Corporation. <BR>

SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <BaseIpmi.h>
#include "IpmiKcsBmc.h"

/**
  Read KCS Status Register.

  @param[in]         KcsIoAddr           The data is KCS IO Address.
  @param[out]        KcsState            A pointer to buffer storing KCS state.

**/
VOID
IpmiKcsGetState (
  IN UINT64                           KcsIoAddr,
  OUT VOID                            *KcsState
  )
{
  UINT8                               *KcsStatusReg;

  KcsStatusReg = (UINT8*)KcsState;
  *KcsStatusReg = MmioRead8 (KcsIoAddr + 1);
}

/**
  Read command to KCS Command Register.

  @param[in]         KcsIoAddr           The data is KCS IO Address.
  @param[out]        KcsData             Data to read to KCS.

**/
VOID
IpmiKcsGetData (
  IN UINT64                         KcsIoAddr,
  OUT VOID                          *KcsData
  )
{
  UINT8                             *KcsDataReg;

  KcsDataReg = (UINT8*)KcsData;
  *KcsDataReg = MmioRead8 (KcsIoAddr);
}

/**
  Erase BMC's output buffer.

  @param[in]         KcsIoAddr          The data is KCS IO Address.

**/
VOID
IpmiKcsEraseObf (
  IN UINT64                             KcsIoAddr
  )
{
  KCS_STATUS                            KcsReg;
  UINT8                                 Data;

  IpmiKcsGetState (KcsIoAddr, &KcsReg);

  if (KcsReg.Obf == 1) {
    IpmiKcsGetData (KcsIoAddr, &Data);
  }

}

/**
  Achieve KCS system interface state.

  @param[in]         KcsIoAddr          The data is KCS IO Address.

  @retval KCS Interface's State Bits.

**/
UINT8
IpmiKcsAchieveState (
  IN UINT64                             KcsIoAddr
  )
{
  UINT8                                 Context;

  IpmiKcsGetState (KcsIoAddr, &Context);

  return (Context >> 6);
}

/**
  KCS System Interface Write Processing.

  @param[in]         KcsIoAddr          The data is KCS IO Address.
  @param[in]         KcsTimePeriod      The data is KCS Timeout period.
  @param[in]         Request            IPMI_CMD_PACKET sturcture, include NetFnLun and Cmd.
  @param[in]         RequestData        Command data buffer to be written to BMC.
  @param[in]         RequestDataSize    Command data length of command data buffer.

  @retval EFI_SUCCESS                   Write Mode finish successfully.
  @retval EFI_ABORTED                   KCS Interface is idle.
  @retval EFI_TIMEOUT                   Output buffer is not full or iutput buffer is not empty
                                        in a given time.
  @retval EFI_DEVICE_ERROR              KCS Interface cannot enter idle state.

**/
EFI_STATUS
KcsSendToBmcMode (
  IN  UINT64                            KcsIoAddr,
  IN  UINT64                            KcsTimePeriod,
  IN  IPMI_CMD_PACKET                   *Request,
  IN  UINT8                             *RequestData,
  IN  UINT8                             RequestDataSize
  )
{
  KCS_STATUS                            KcsReg;
  UINT8                                 Command;
  UINT8                                 Count;
  UINT8                                 Length;
  UINT8                                 *DataArray;
  UINT32                                Timeout;
  UINT32                                Timedex;
  EFI_STATUS                            Status;

  Status = EFI_SUCCESS;
  Timeout = 0;
  Timedex = 0;
  DataArray = (UINT8*)Request;
  Length = sizeof (IPMI_CMD_PACKET) + RequestDataSize;

  //
  // Wait KCS system interface input buffer empty.
  //
  do {
     MicroSecondDelay(100);
     IpmiKcsGetState (KcsIoAddr,&KcsReg);
     if (Timeout >= KcsTimePeriod) {
       Status = EFI_DEVICE_ERROR;
       return Status;
     }
     Timeout++;
  } while (KcsReg.Ibf != 0);

  IpmiKcsEraseObf (KcsIoAddr);

  //
  // Send Data Phase.
  //
  for ( Count=0; Count <= Length; Count++) {
    if (Count == 0) {
        Command = KCS_CC_WRITE_START;
        MmioWrite8 (KcsIoAddr + 1, Command);
     } else {
         if (Count == Length ) {
         Command = KCS_CC_WRITE_END;
         MmioWrite8 (KcsIoAddr + 1, Command);
         } else {
              MmioWrite8 (KcsIoAddr, *DataArray);
              if(Count ==2) {
                DataArray = (UINT8*)RequestData;
              } else {
                ++DataArray;
              }
        }
    }

    //
    // Wait KCS system interface input buffer empty.
    //
    do {
      MicroSecondDelay(100);
      IpmiKcsGetState (KcsIoAddr,&KcsReg);
      if (Timedex >= KcsTimePeriod) {
         Status = EFI_DEVICE_ERROR;
        return Status;
      }
      Timedex++;
    } while (KcsReg.Ibf != 0);

    IpmiKcsEraseObf(KcsIoAddr);
 }

  MmioWrite8 (KcsIoAddr, *DataArray);

  return EFI_SUCCESS;
}

/**
  KCS System Interface Receive Processing.

  @param[in]         KcsIoAddr          The data is KCS IO Address.
  @param[in]         KcsTimePeriod      The data is KCS Timeout period.
  @param[out]        Response           IPMI_CMD_PACKET sturcture.
  @param[out]        Context            This transaction response Cmd.
  @param[out]        ResponseData       Data buffer to put the data read from BMC (from completion code).
  @param[out]        ResponseDataSize   Length of Data readed from BMC.

  @retval EFI_SUCCESS                   Receive Mode finish successfully.
  @retval EFI_ABORTED                   KCS Interface is idle.
  @retval EFI_TIMEOUT                   Output buffer is not full or iutput buffer is not empty
                                        in a given time.
  @retval EFI_DEVICE_ERROR              KCS Interface cannot enter idle state.

**/
EFI_STATUS
KcsReceiveBmcMode (
  IN  UINT64                            KcsIoAddr,
  IN  UINT64                            KcsTimePeriod,
  OUT IPMI_CMD_PACKET                   *Response,
  OUT UINT8                             *Context,
  OUT UINT8                             *ResponseData,
  OUT UINT8                             *ResponseDataSize
  )
{
  KCS_STATUS                            KcsReg;
  EFI_STATUS                            Status;
  UINT8                                 *DataArray;
  UINT8                                 Count;
  UINT8                                 ControlCodes;
  UINT32                                Timeout;
  UINT32                                Timeobf;
  UINT32                                Timedex;
  UINT8                                 KcsData;

  Timeout = 0;
  Timeobf = 0;
  Timedex = 0;
  KcsData = 0;
  *ResponseDataSize = 0;
  DataArray = (UINT8*)Response;
  ControlCodes = KCS_CC_READ;
  Count = 0;
  Status = EFI_SUCCESS;

  while (TRUE) {
    //
    // Wait KCS system interface input buffer empty.
    //
     do {
       MicroSecondDelay(100);
       IpmiKcsGetState (KcsIoAddr,&KcsReg);
       if (Timeout >= KcsTimePeriod) {
         Status = EFI_DEVICE_ERROR;
         return Status;
       }
        Timeout++;
     } while (KcsReg.Ibf != 0);

     Status= IpmiKcsAchieveState (KcsIoAddr);
      if (Status != KCS_READ_STATE) {
         break;
      }

      //
      // Wait KCS system interface output buffer full.
      //
      do {
        MicroSecondDelay(100);
        IpmiKcsGetState (KcsIoAddr, &KcsReg);
        if (Timeobf >= KcsTimePeriod) {
         Status = EFI_DEVICE_ERROR;
         return Status;
        }
        Timeobf++;
      } while (!KcsReg.Obf);

      //
      // Receive Data form BMC.
      //
      IpmiKcsGetData (KcsIoAddr, DataArray);

      switch (Count) {
         case 0:
            ++DataArray;
            break;
         case 1:
            DataArray = Context;
            break;
         case 2:
            DataArray = (UINT8*)ResponseData;
         default:
            ++DataArray;
            ++(*ResponseDataSize);
            break;
      }
      ++Count;
      MmioWrite8 (KcsIoAddr, ControlCodes);
  }

  if (IpmiKcsAchieveState (KcsIoAddr) != KCS_IDLE_STATE) {
       return EFI_DEVICE_ERROR;
   }

   do {
        MicroSecondDelay(100);
        IpmiKcsGetState (KcsIoAddr, &KcsReg);
        if (Timedex >= KcsTimePeriod) {
         Status = EFI_DEVICE_ERROR;
         return Status;
        }
        Timedex++;
      } while (!KcsReg.Obf);

   IpmiKcsGetData (KcsIoAddr,&KcsData);

  return EFI_SUCCESS;
}

/**
  Return system interface type that BMC currently use.

  @retval  IpmiKcs  Current system interface type is KCS.
  @retval  IpmiSmic Current system interface type is SMIC.
  @retval  IpmiBt   Current system interface type is BT.
  @retval  IpmiSsif Current system interface type is SSIF.
  @retval  IpmiUnknow  Current system interface type is Unknow.

**/
IPMI_INTERFACE_TYPE
EFIAPI
IpmiGetInterfaceType (
  VOID
  )
{
  return IpmiKcs;
}


